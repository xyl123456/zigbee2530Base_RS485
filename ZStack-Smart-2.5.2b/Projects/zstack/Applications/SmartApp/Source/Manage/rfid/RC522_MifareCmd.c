/* /////////////////////////////////////////////////////////////////////////////////////////////////
//                     Copyright (c) Philips Semiconductors
//
//         All rights are reserved. Reproduction in whole or in part is
//        prohibited without the written consent of the copyright owner.
//    Philips reserves the right to make changes without notice at any time.
//   Philips makes no warranty, expressed, implied or statutory, including but
//   not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party patent,
//   copyright or trademark. Philips must not be liable for any loss or damage
//                            arising from its use.
///////////////////////////////////////////////////////////////////////////////////////////////// */

/*! \file Mifare.c
 *
 * Project: Mifare reader with RC522
 *
 * Workfile: Mifare.c
 * $Author: Bob Jiang
 * $Revision: 1.0 $
 * $Date: Wed Aug 17 2005 $
 *
 * Comment:
 *  All the ISO14443-3 protocol and mifare command set are all implemented here.
 *  All the founctions in this file is totally independent of hardware.
 *  The source can be ported to other platforms very easily.
 *
 *  The interrupt pin of the reader IC is not conntected and no interrupt needed.
 *  All protocol relevant timing constraints are generated
 *  by the internal timer of the reader module.
 *
 *  Therefore the function M522PcdCmd is very important for understanding
 *  of the communication between reader and card.
 *
 *
 * History:
 *
 *
*/
//#include <REG936.H>
#include <string.h>
#include "RC522_Reg.h"
#include "RC522_RegCtrl.h"
#include "RC522_OpCtrl.h"
#include "RC522_ErrCode.h"
#include "RC522_Mifare.h"
#include "mfrc522.h"
#include "I2C.h"

#define ResetInfo(info)    \
            info.cmd            = 0;\
            info.status         = STATUS_SUCCESS;\
            info.nBytesSent     = 0;\
            info.nBytesToSend   = 0;\
            info.nBytesReceived = 0;\
            info.nBitsReceived  = 0;\
            info.collPos        = 0;

MfCmdInfo MInfo;
volatile MfCmdInfo *MpIsrInfo = 0;
unsigned char SerBuffer[64];


/*************************************************
Function:       SetPowerDown
Description:
     set the rc522 enter or exit power down mode
Parameter:
     ucFlag     0   --  exit power down mode
                !0  --  enter power down mode
Return:
     short      status of implement
**************************************************/
#if 0 // This function is currently not used
void SetPowerDown(unsigned char ucFlag)
{
    unsigned char RegVal;
/*
    Note: The bit Power Down can not be set when the SoftReset command has been activated.
*/
    if(ucFlag)
    {
        RegVal = RcGetReg(RC522_REG_COMMAND);  //enter power down mode
        RegVal |= 0x10;
        RcSetReg(RC522_REG_COMMAND, RegVal);
    }
    else
    {
        RegVal = RcGetReg(RC522_REG_COMMAND);  //disable power down mode
        RegVal &= (~0x10);
        RcSetReg(RC522_REG_COMMAND, RegVal);
    }
}
#endif
/*************************************************
Function:       SetTimeOut
Description:
     Adjusts the timeout in 100us steps
Parameter:
     uiMicroSeconds   the time to set the timer(100us as a step)
Return:
     short      status of implement
**************************************************/
short SetTimeOut(unsigned int uiMicroSeconds)
{
    unsigned int RegVal;
    unsigned char TmpVal;
    RegVal = uiMicroSeconds / 100;

    /*
    NOTE: The supported hardware range is bigger, since the prescaler here
          is always set to 100 us.
    */
    if(RegVal >= 0xfff)
    {
        return STATUS_INVALID_PARAMETER;
    }
    RcModifyReg(RC522_REG_TMODE, 1, RC522_BIT_TAUTO);

    RcSetReg(RC522_REG_TPRESCALER, 0xa6);

    TmpVal = RcGetReg(RC522_REG_TMODE);
    TmpVal &= 0xf0;
    TmpVal |= 0x02;
    RcSetReg(RC522_REG_TMODE, TmpVal);//82

    RcSetReg(RC522_REG_TRELOADLO, ((unsigned char)(RegVal&0xff)));
    RcSetReg(RC522_REG_TRELOADHI, ((unsigned char)((RegVal>>8)&0xff)));
    return STATUS_SUCCESS;
}
  


/*************************************************
Function:       Rc522Init
Description:
     initialize rc522 as a mifare reader
Parameter:
     NONE
Return:
     NONE
**************************************************/
void Rc522Init(void)
{
    unsigned char RegVal;

    RcSetReg(RC522_REG_COMMAND, 0x0F); /*reset the RC522*/

    RcSetReg(RC522_REG_TXASK, 0x40); /*force to 100% ASK*/

    /* disable Crypto1 bit*/
    RcModifyReg(RC522_REG_STATUS2, 0, RC522_BIT_CRYPTO1ON);

    /* do not touch bits: InvMod in register TxMode */
    RegVal = RcGetReg(RC522_REG_TXMODE);
    RegVal = (unsigned char)(RegVal & RC522_BIT_INVMOD);
    RegVal = (unsigned char)(RegVal | RC522_BIT_CRCEN | (RCO_VAL_RF106K << RC522_SPEED_SHL_VALUE));
    /* TxCRCEn = 1; TxSpeed = x; InvMod, TXMix = 0; TxFraming = 0 */
    RcSetReg(RC522_REG_TXMODE, RegVal);

    /* do not touch bits: RxNoErr in register RxMode */
    RegVal = RcGetReg(RC522_REG_RXMODE);
    RegVal = (unsigned char)(RegVal & RC522_BIT_RXNOERR);
    RegVal = (unsigned char)(RegVal | RC522_BIT_CRCEN | (RCO_VAL_RF106K << RC522_SPEED_SHL_VALUE));
     /* RxCRCEn = 1; RxSpeed = x; RxNoErr, RxMultiple = 0; TxFraming = 0 */
    RcSetReg(RC522_REG_RXMODE, RegVal);

    /* ADDIQ = 10b; FixIQ = 1; RFU = 0; TauRcv = 11b; TauSync = 01b */
    RcSetReg(RC522_REG_DEMOD, 0x6D);
    //RegVal = RcGetReg(RC522_REG_DEMOD);

    /* RxGain = 4*/
    RcSetReg(RC522_REG_RFCFG, 0x48);
    //RegVal = RcGetReg(RC522_REG_RFCFG);///test88888

    /* do settings common for all functions */
    RcSetReg(RC522_REG_RXTRESHOLD, 0x55);    /* MinLevel = 5; CollLevel = 5 */
   
    
    RcSetReg(RC522_REG_MODWIDTH, 0x26);      /* Modwidth = 0x26 */
    RcSetReg(RC522_REG_GSN, 0xF0 | 0x04);     /* CWGsN = 0xF; ModGsN = 0x4 */

    /* Set the timer to auto mode, 5ms using operation control commands before HF is switched on to
     * guarantee Iso14443-3 compliance of Polling procedure
     */
    SetTimeOut(5000);

    /* Activate the field  */
    RcModifyReg(RC522_REG_TXCONTROL, 1, RC522_BIT_TX2RFEN | RC522_BIT_TX1RFEN);

    /* start timer manually to check the initial waiting time */
    RcModifyReg(RC522_REG_CONTROL, 1, RC522_BIT_TSTARTNOW);

    /*
     * After switching on the timer wait until the timer interrupt occures, so that
     * the field is on and the 5ms delay have been passed.
     */
    do {
        RegVal = RcGetReg(RC522_REG_COMMIRQ);
    }
    while(!(RegVal & RC522_BIT_TIMERI));


    /* Clear the status flag afterwards */
    RcSetReg(RC522_REG_COMMIRQ, RC522_BIT_TIMERI);

    /*
     * Reset timer 1 ms using operation control commands (AutoMode and Prescaler are the same)
     * set reload value
     */
    SetTimeOut(5000);

    RcSetReg(RC522_REG_WATERLEVEL, 0x1A);
    RcSetReg(RC522_REG_TXSEL, 0x10);
    
    RcSetReg(RC522_REG_RXSEL, 0x87);        /* Default 0x84  7..Rx Waite (7 x 9,4us) */

    /* Activate receiver for communication
       The RcvOff bit and the PowerDown bit are cleared, the command is not changed. */
    RcSetReg(RC522_REG_COMMAND, RC522_CMD_IDLE);

    /* Set timeout for REQA, ANTICOLL, SELECT to 200us */
    SetTimeOut(2000);
    RcSetReg(RC522_REG_ANALOGTEST,0xCD);

} 


/*************************************************
Function:       RF Reset
Description:
     halt the current selected card
Parameter:
     NONE
Return:
     NONE
**************************************************/

void Rc522RFReset(unsigned char ms)
{
  //unsigned int j;
  //RcSetReg(RC522_REG_COMMAND, 0x0f); // RC522 Soft Reset
	//Timer1_Delay_ms(ms);
  //      for (j=0;j<(100*ms);j++)
  //         Delay(200);
	Rc522Init();
}


/*************************************************
Function:       M522PcdCmd
Description:
     implement a command
Parameter:
     cmd            command code
     ExchangeBuf    saved the data will be send to card and the data responed from the card
     info           some information for the command
Return:
     short      status of implement
**************************************************/
short  M522PcdCmd(unsigned char cmd, unsigned char *ExchangeBuf, MfCmdInfo  *info)
{
    short          status    = STATUS_SUCCESS;

    unsigned char  commIrqEn   = 0;
    unsigned char  divIrqEn    = 0;
    unsigned char  waitForComm = RC522_BIT_ERRI | RC522_BIT_TXI;
    unsigned char  waitForDiv  = 0;
    unsigned char  doReceive   = 0;
    unsigned char  i;
    unsigned char  getRegVal,setRegVal;

    //unsigned char  nbytes, nbits;
    unsigned int counter;

    /*remove all Interrupt request flags that are used during function,
    keep all other like they are*/
    RcSetReg(RC522_REG_COMMIRQ, waitForComm);
    RcSetReg(RC522_REG_DIVIRQ, waitForDiv);
    RcSetReg(RC522_REG_FIFOLEVEL, RC522_BIT_FLUSHBUFFER);

    /*disable command or set to transceive*/
    getRegVal = RcGetReg(RC522_REG_COMMAND);
    if(cmd == RC522_CMD_TRANSCEIVE)
    {
        /*re-init the transceive command*/
        setRegVal = (getRegVal & ~RC522_MASK_COMMAND) | RC522_CMD_TRANSCEIVE;
        RcSetReg(RC522_REG_COMMAND, setRegVal);
    }
    else
    {
        /*clear current command*/
        setRegVal = (getRegVal & ~RC522_MASK_COMMAND);
        RcSetReg(RC522_REG_COMMAND, setRegVal);
    }
    MpIsrInfo = info;
    switch(cmd)
    {
       case RC522_CMD_IDLE:         /* values are 00, so return immediately after all bytes written to FIFO */
            waitForComm = 0;
            waitForDiv  = 0;
            break;
        case RC522_CMD_CALCCRC:      /* values are 00, so return immediately after all bytes written to FIFO */
            waitForComm = 0;
            waitForDiv  = 0;
            break;
        case RC522_CMD_TRANSMIT:
            commIrqEn = RC522_BIT_TXI | RC522_BIT_TIMERI;
            waitForComm = RC522_BIT_TXI;
            break;
        case RC522_CMD_RECEIVE:
            commIrqEn = RC522_BIT_RXI | RC522_BIT_TIMERI | RC522_BIT_ERRI;
            waitForComm = RC522_BIT_RXI | RC522_BIT_TIMERI | RC522_BIT_ERRI;
            doReceive = 1;
            break;
        case RC522_CMD_TRANSCEIVE:
            commIrqEn = RC522_BIT_RXI | RC522_BIT_TIMERI | RC522_BIT_ERRI;
            waitForComm = RC522_BIT_RXI | RC522_BIT_TIMERI | RC522_BIT_ERRI;
            doReceive = 1;
            break;
        case RC522_CMD_AUTHENT:
            commIrqEn = RC522_BIT_IDLEI | RC522_BIT_TIMERI | RC522_BIT_ERRI;
            waitForComm = RC522_BIT_IDLEI | RC522_BIT_TIMERI | RC522_BIT_ERRI;
            break;
        case RC522_CMD_SOFTRESET:    /* values are 0x00 for IrqEn and for waitFor, nothing to do */
            waitForComm = 0;
            waitForDiv  = 0;
            break;
        default:
            status = STATUS_UNSUPPORTED_COMMAND;
    }
    if(status == STATUS_SUCCESS)
    {
        /* activate necessary communication Irq's */
        getRegVal = RcGetReg(RC522_REG_COMMIEN);
        RcSetReg(RC522_REG_COMMIEN, getRegVal | commIrqEn);

        /* activate necessary other Irq's */
        getRegVal = RcGetReg(RC522_REG_DIVIEN);
        RcSetReg(RC522_REG_DIVIEN, getRegVal | divIrqEn);

        /*write data to FIFO*/
        for(i=0; i<MpIsrInfo->nBytesToSend; i++)
        {
            RcSetReg(RC522_REG_FIFODATA, ExchangeBuf[i]);
        }

        /*do seperate action if command to be executed is transceive*/
        if(cmd == RC522_CMD_TRANSCEIVE)
        {
            /*TRx is always an endless loop, Initiator and Target must set STARTSEND.*/
            RcModifyReg(RC522_REG_BITFRAMING, 1, RC522_BIT_STARTSEND);
        }
        else
        {
            getRegVal = RcGetReg(RC522_REG_COMMAND);
            RcSetReg(RC522_REG_COMMAND, (getRegVal & ~RC522_MASK_COMMAND) | cmd);
        }

        /*polling mode*/
        getRegVal = 0;
        setRegVal = 0;
        counter = 0; /*Just for debug*/
        while(!(waitForComm ? (waitForComm & setRegVal) : 1) ||
              !(waitForDiv ? (waitForDiv & getRegVal) :1))
        {
            setRegVal = RcGetReg(RC522_REG_COMMIRQ);
            getRegVal = RcGetReg(RC522_REG_DIVIRQ);
            counter ++;
            if(counter > 0x0100)
                break;
        }
        /*store IRQ bits for clearance afterwards*/
        waitForComm = (unsigned char)(waitForComm & setRegVal);
        waitForDiv  = (unsigned char)(waitForDiv & getRegVal);

        /*set status to Timer Interrupt occurence*/
        if (setRegVal & RC522_BIT_TIMERI)
        {
            status = STATUS_IO_TIMEOUT;
        }
    }

    /*disable all interrupt sources*/
    RcModifyReg(RC522_REG_COMMIEN, 0, commIrqEn);

    RcModifyReg(RC522_REG_DIVIEN, 0, divIrqEn);

    if(doReceive && (status == STATUS_SUCCESS))
    {
        /*read number of bytes received (used for error check and correct transaction*/
        MpIsrInfo->nBytesReceived = RcGetReg(RC522_REG_FIFOLEVEL);
        getRegVal = RcGetReg(RC522_REG_CONTROL);
        MpIsrInfo->nBitsReceived = (unsigned char)(getRegVal & 0x07);

        getRegVal = RcGetReg(RC522_REG_ERROR);
        /*set status information if error occured*/
        if(getRegVal)
        {
            if(getRegVal & RC522_BIT_COLLERR)
                status = STATUS_COLLISION_ERROR;         /* Collision Error */
            else if(getRegVal & RC522_BIT_PARITYERR)
                status = STATUS_PARITY_ERROR;            /* Parity Error */

            if(getRegVal & RC522_BIT_PROTERR)
                status = STATUS_PROTOCOL_ERROR;          /* Protocoll Error */
            else if(getRegVal & RC522_BIT_BUFFEROVFL)
                status = STATUS_BUFFER_OVERFLOW;         /* BufferOverflow Error */
            else if(getRegVal & RC522_BIT_CRCERR)
            {   /* CRC Error */
                if(MpIsrInfo->nBytesReceived == 0x01 &&
                    (MpIsrInfo->nBitsReceived == 0x04 ||
                     MpIsrInfo->nBitsReceived == 0x00))
                {   /* CRC Error and only one byte received might be a Mifare (N)ACK */
                    ExchangeBuf[0] = RcGetReg(RC522_REG_FIFODATA);
                    MpIsrInfo->nBytesReceived = 1;
                    status = STATUS_ACK_SUPPOSED;        /* (N)ACK supposed */
                }
                else
                    status = STATUS_CRC_ERROR;           /* CRC Error */    ///////////////
            }
            else if(getRegVal & RC522_BIT_TEMPERR)
                status = STATUS_RC522_TEMP_ERROR;       /* Temperature Error */
            if(getRegVal & RC522_BIT_WRERR)
                status = STATUS_FIFO_WRITE_ERROR;        /* Error Writing to FIFO */
            if(status == STATUS_SUCCESS)
                status = STATUS_ERROR_NY_IMPLEMENTED;    /* Error not yet implemented, shall never occur! */

            /* if an error occured, clear error register before IRQ register */
            RcSetReg(RC522_REG_ERROR, 0);
        }

        /*read data from FIFO and set response parameter*/
        if(status != STATUS_ACK_SUPPOSED)
        {
            for(i=0; i<MpIsrInfo->nBytesReceived; i++)
            {
                ExchangeBuf[i] = RcGetReg(RC522_REG_FIFODATA);
            }
            /*in case of incomplete last byte reduce number of complete bytes by 1*/
            if(MpIsrInfo->nBitsReceived && MpIsrInfo->nBytesReceived)
                MpIsrInfo->nBytesReceived --;
        }
    }
    RcSetReg(RC522_REG_COMMIRQ, waitForComm);
    RcSetReg(RC522_REG_DIVIRQ, waitForDiv);
    RcSetReg(RC522_REG_FIFOLEVEL, RC522_BIT_FLUSHBUFFER);
    RcSetReg(RC522_REG_COMMIRQ, RC522_BIT_TIMERI);
    RcSetReg(RC522_REG_BITFRAMING, 0);
    return status;
}

/*************************************************
Function:       Request
Description:
     REQA, request to see if have a ISO14443A card in the field
Parameter:
     req_code   command code(ISO14443_3_REQALL or ISO14443_3_REQIDL)
     atq        the buffer to save the answer to request from the card
Return:
     short      status of implement
**************************************************/
short Request(unsigned char req_code, unsigned char *atq)
{
   char  status = STATUS_SUCCESS;

   /************* initialize *****************/
   RcModifyReg(RC522_REG_STATUS2, 0, RC522_BIT_CRYPTO1ON);  /* disable Crypto if activated before */
   RcSetReg(RC522_REG_COLL, RC522_BIT_VALUESAFTERCOLL);  	/* active values after coll */
   RcModifyReg(RC522_REG_TXMODE, 0, RC522_BIT_CRCEN);  		/* disable TxCRC and RxCRC */
   RcModifyReg(RC522_REG_RXMODE, 0, RC522_BIT_CRCEN);
   RcSetReg(RC522_REG_BITFRAMING, REQUEST_BITS);

   /* set necessary parameters for transmission */
   ResetInfo(MInfo);
   SerBuffer[0] = req_code;
   MInfo.nBytesToSend   = 1;

   /* Set timeout for REQA, ANTICOLL, SELECT*/
    SetTimeOut(400);

   status = M522PcdCmd(RC522_CMD_TRANSCEIVE,
                      SerBuffer,
                      &MInfo);
   if(status == STATUS_SUCCESS || status == STATUS_COLLISION_ERROR)
   {
       if(MInfo.nBytesReceived != ATQA_LENGTH || MInfo.nBitsReceived != 0x00)
       {
           status = STATUS_PROTOCOL_ERROR;
       }
       else
       {
           memcpy(atq,SerBuffer,2);
       }
   }
   else
   {   /* reset atqa parameter */
       atq[0] = 0x00;
       atq[1] = 0x00;
   }
   //RcSetReg(RC522_REG_BITFRAMING, 0);
   return status;
}

/*************************************************
Function:       CascAnticoll
Description:
     Functions to split anticollission and select internally.
     NOTE: this founction is used internal only, and cannot call by application program
Parameter:
     sel_code   command code
     bitcount   the bit counter of known UID
     snr        the UID have known
Return:
     short      status of implement
**************************************************/
short CascAnticoll(unsigned char sel_code,
                   unsigned char bitcount,
                   unsigned char *snr)
{
    short status = STATUS_SUCCESS;

    unsigned char  i;
    unsigned char  complete = 0; /* signs end of anticollission loop */
    unsigned char  rbits    = 0; /* number of total received bits */
    unsigned char  nbits    = 0; /* */
    unsigned char  nbytes   = 0; /* */
    unsigned char  byteOffset;   /* stores offset for ID copy if uncomplete last byte was sent */

    /* initialise relvant bytes in internal buffer */
    for(i=2;i<7;i++)
        SerBuffer[i] = 0x00;

    /* disable TxCRC and RxCRC */
    RcModifyReg(RC522_REG_TXMODE, 0, RC522_BIT_CRCEN);
    RcModifyReg(RC522_REG_RXMODE, 0, RC522_BIT_CRCEN);

    /* activate deletion of bits after coll */
    RcSetReg(RC522_REG_COLL, 0);

    /* init parameters for anticollision */
    while(!complete && (status == STATUS_SUCCESS))
    {
         /* if there is a communication problem on the RF interface, bcnt
            could be larger than 32 - folowing loops will be defective. */
        if(bitcount > SINGLE_UID_LENGTH)
        {
            status = STATUS_INVALID_PARAMETER;
            continue;
        }

        /* prepare data length */
        nbits = (unsigned char)(bitcount % BITS_PER_BYTE);
        nbytes = (unsigned char)(bitcount / BITS_PER_BYTE);
        if(nbits)
            nbytes++;

        /* prepare data buffer */
        SerBuffer[0] = sel_code;
        SerBuffer[1] = (unsigned char)(NVB_MIN_PARAMETER + ((bitcount / BITS_PER_BYTE) << UPPER_NIBBLE_SHIFT) + nbits);
        for(i=0;i<nbytes;i++)
            SerBuffer[2+i] = snr[i];   /* copy serial number to tranmit buffer */

        /* set TxLastBits and RxAlign to number of bits sent */
        RcSetReg(RC522_REG_BITFRAMING, (unsigned char)((nbits << UPPER_NIBBLE_SHIFT) | nbits));

        /* prepare data for common transceive */
        ResetInfo(MInfo);
        MInfo.nBytesToSend   = (unsigned char)(nbytes + 2);
        SetTimeOut(1000);
        //SetTimeOut(10000);
        status = M522PcdCmd(RC522_CMD_TRANSCEIVE, SerBuffer, &MInfo);

        if(status == STATUS_COLLISION_ERROR || status == STATUS_SUCCESS)
        {
            /* store number of received data bits and bytes internaly */
            rbits = (unsigned char)(MInfo.nBitsReceived + (MInfo.nBytesReceived << 3) - nbits);

            if((rbits + bitcount) > COMPLETE_UID_BITS)
            {
                status = STATUS_BITCOUNT_ERROR;
                continue;
            }

            /* increment number of bytes received if also some bits received */
            if(MInfo.nBitsReceived)
                MInfo.nBytesReceived++;

            /* reset offset for data copying */
            byteOffset = 0;
            /* if number of bits sent are not 0, write first received byte in last of sent */
            if(nbits)
            {   /* last byte transmitted and first byte received are the same */
                snr[nbytes - 1] |= SerBuffer[0];
                byteOffset++;
            }

            for(i=0;i<(4-nbytes);i++)
                snr[nbytes + i] = SerBuffer[i + byteOffset];

            if(status == STATUS_COLLISION_ERROR)
            {
                /* calculate new bitcount value */
                bitcount = (unsigned char)(bitcount + rbits);
                status = STATUS_SUCCESS;
            } else
            {
                if((snr[0] ^ snr[1] ^ snr[2] ^ snr[3]) != SerBuffer[i + byteOffset])
                {
                    status = STATUS_WRONG_UID_CHECKBYTE;
                    continue;
                }
                complete=1;
            }
        }
    }

    /* clear RxAlign and TxLastbits */
    RcSetReg(RC522_REG_BITFRAMING, 0);

    /* activate values after coll */
    RcSetReg(RC522_REG_COLL, RC522_BIT_VALUESAFTERCOLL);
    return status;
}

/*************************************************
Function:       Select
Description:
     selecte a card to response the following command
     NOTE: this founction is used internal only, and cannot call by application program
Parameter:
     sel_code   command code
     snr        buffer to store the card UID
     sak        the byte to save the ACK from card
Return:
     short      status of implement
**************************************************/
short Select(unsigned char sel_code, unsigned char *snr, unsigned char *sak)
{
    short status = STATUS_SUCCESS;
    /* define local variables */
    unsigned char i;
    /* activate CRC */
    RcModifyReg(RC522_REG_TXMODE, 1, RC522_BIT_CRCEN);
    RcModifyReg(RC522_REG_RXMODE, 1, RC522_BIT_CRCEN);

    /* prepare data stream */
    SerBuffer[0] = sel_code;   /* command code */
    SerBuffer[1] = NVB_MAX_PARAMETER;       /* parameter */
    for(i=0;i<4;i++)
        SerBuffer[2+i] = snr[i];   /* serial numbner bytes 1 to 4 */
    SerBuffer[6] = (unsigned char)(snr[0] ^ snr[1] ^ snr[2] ^ snr[3]);   /* serial number check byte */

    /* prepare data for common transceive */
    ResetInfo(MInfo);
    MInfo.nBytesToSend   = 0x07;
    SetTimeOut(1000);
    //SetTimeOut(2000);
    status = M522PcdCmd(RC522_CMD_TRANSCEIVE, SerBuffer, &MInfo);

    if(status == STATUS_SUCCESS)
    {
        if(MInfo.nBytesReceived == SAK_LENGTH && MInfo.nBitsReceived == 0)
            *sak = SerBuffer[0];
        else
            status = STATUS_BITCOUNT_ERROR;
    }
    return status;
}

/*************************************************
Function:       HaltA
Description:
     halt the current selected card
Parameter:
     NONE
Return:
     short      status of implement
**************************************************/
short HaltA(void)
{
    short  status = STATUS_SUCCESS;
    /* initialise data buffer */
    SerBuffer[0] = HALTA_CMD;
    SerBuffer[1] = HALTA_PARAM;

    ResetInfo(MInfo);
    MInfo.nBytesToSend   = HALTA_CMD_LENGTH;
    SetTimeOut(1000);
    status = M522PcdCmd(RC522_CMD_TRANSCEIVE, SerBuffer, &MInfo);

    if(status == STATUS_IO_TIMEOUT)
        status = STATUS_SUCCESS;
    return status;
}

/*************************************************
Function:       Authentication
Description:
     authentication the password for a sector of mifare card
Parameter:
     auth_mode  specify key A or key B -- MIFARE_AUTHENT_A or MIFARE_AUTHENT_A
     key        the buffer stored the key(6 bytes)
     snr        the buffer stored the selected card's UID
     addr       the block address of a sector
Return:
     short      status of implement
**************************************************/
short Authentication(unsigned char auth_mode,
                     unsigned char *key,
                     unsigned char *snr,
                     unsigned char addr)
{
    short status;
    //unsigned char i = 0;
    unsigned char RegVal;

    ResetInfo(MInfo);

    SerBuffer[0] = auth_mode;      //key A or key B
    SerBuffer[1] = addr;           //address to authentication
    memcpy(SerBuffer+2,key,6);     //6 bytes key
    memcpy(SerBuffer+8,snr,4);     //4 bytes UID
    MInfo.nBytesToSend = 12;       //length
    SetTimeOut(10000);
    status = M522PcdCmd(RC522_CMD_AUTHENT, SerBuffer, &MInfo);
    if(status == STATUS_SUCCESS)
    {
        RegVal = RcGetReg(RC522_REG_STATUS2);
        if((RegVal & 0x0f) != 0x08)
            status = STATUS_AUTHENT_ERROR;
    }
    return status;
}
/*************************************************
Function:       Read
Description:
     read 16 bytes data from a block
Parameter:
     addr       the address of the block
     _data      the buffer to save the 16 bytes data
Return:
     short      status of implement
**************************************************/
short Read(unsigned char addr, unsigned char *_data)
{
   short status = STATUS_SUCCESS;
   //char tmp    = 0;

   ResetInfo(MInfo);
   SerBuffer[0] = MIFARE_READ;
   SerBuffer[1] = addr;
   MInfo.nBytesToSend   = 2;
   SetTimeOut(10000);
   status = M522PcdCmd(RC522_CMD_TRANSCEIVE,
                       SerBuffer,
                       &MInfo);

   if (status != STATUS_SUCCESS)
   {
      if (status != STATUS_IO_TIMEOUT )     // no timeout occured
      {
         if (MInfo.nBitsReceived == 4)
         {
             SerBuffer[0] &= 0x0f;
             if ((SerBuffer[0] & 0x0a) == 0)
             {
                status = STATUS_AUTHENT_ERROR;
             }
             else
             {
                status = STATUS_INVALID_FORMAT;
             }
          }
      }
      memset(_data,0,16);
   }
   else   // Response Processing
   {
      if (MInfo.nBytesReceived != 16)
      {
         status = STATUS_ACCESS_DENIED;
         memset(_data,0,16);
      }
      else
      {
         memcpy(_data,SerBuffer,16);
      }
   }
   return status;
}

/*************************************************
Function:       Write
Description:
     write 16 bytes data to a block
Parameter:
     addr       the address of the block
     _data      the data to write
Return:
     short      status of implement
**************************************************/
short Write( unsigned char addr, unsigned char *_data)
{
    short status = STATUS_SUCCESS;
    ResetInfo(MInfo);
    SerBuffer[0] = MIFARE_WRITE;
    SerBuffer[1] = addr;
    MInfo.nBytesToSend   = 2;
    SetTimeOut(20000);
    status = M522PcdCmd(RC522_CMD_TRANSCEIVE,
                        SerBuffer,
                        &MInfo);

    if (status != STATUS_IO_TIMEOUT)
    {
       if (MInfo.nBitsReceived != 4)
       {
          status = STATUS_BITCOUNT_ERROR;
       }
       else
       {
          SerBuffer[0] &= 0x0f;
          if ((SerBuffer[0] & 0x0a) == 0)
          {
             status = STATUS_AUTHENT_ERROR;
          }
          else
          {
             if (SerBuffer[0] == 0x0a)
             {
                status = STATUS_SUCCESS;
             }
             else
             {
                status = STATUS_INVALID_FORMAT;
             }
          }
       }
    }

    if ( status == STATUS_SUCCESS)
    {

       SetTimeOut(60000);

       ResetInfo(MInfo);
       memcpy(SerBuffer,_data,16);
       MInfo.nBytesToSend   = 16;
       status = M522PcdCmd(RC522_CMD_TRANSCEIVE,
                           SerBuffer,
                           &MInfo);

       if (status & 0x80)
       {
          status = STATUS_IO_TIMEOUT;
       }
       else
       {
          if (MInfo.nBitsReceived != 4)
          {
             status = STATUS_BITCOUNT_ERROR;
          }
          else
          {
             SerBuffer[0] &= 0x0f;
             if ((SerBuffer[0] & 0x0a) == 0)
             {
                status = STATUS_ACCESS_DENIED;
             }
             else
             {
                if (SerBuffer[0] == 0x0a)
                {
                   status = STATUS_SUCCESS;
                }
                else
                {
                   status = STATUS_INVALID_FORMAT;
                }
             }
          }
       }
    }
  return status;
}

/*************************************************
Function:       ValueOper
Description:
     block value operation function, increment or decrement the block value
     and transfer to a block
Parameter:
     OperMode   MIFARE_INCREMENT or MIFARE_DECREMENT
     addr       the address of the block
     value      the value to be increment or decrement
     trans_addr the address to save the resulet of increment or decrement
Return:
     short      status of implement
**************************************************/
short ValueOper(unsigned char OperMode, 
                    unsigned char addr,
                    unsigned char *value,
                    unsigned char trans_addr)
{
   short status = STATUS_SUCCESS;
   ResetInfo(MInfo);
   SerBuffer[0] = OperMode;
   SerBuffer[1] = addr;
   MInfo.nBytesToSend   = 2;
   SetTimeOut(20000);
   status = M522PcdCmd(RC522_CMD_TRANSCEIVE,
                       SerBuffer,
                       &MInfo);

   if (status != STATUS_IO_TIMEOUT)
   {
        if (MInfo.nBitsReceived != 4)
        {
           status = STATUS_BITCOUNT_ERROR;
        }
        else
        {
           SerBuffer[0] &= 0x0f;
           switch(SerBuffer[0])
           {
              case 0x00:
                 status = STATUS_AUTHENT_ERROR;
                 break;
              case 0x0a:
                 status = STATUS_SUCCESS;
                 break;
              case 0x01:
                 status = STATUS_INVALID_FORMAT;
                 break;
              default:
                 status = STATUS_OTHER_ERROR;
                 break;
           }
        }
     }

     if ( status == STATUS_SUCCESS)
     {
        SetTimeOut(10000);
        ResetInfo(MInfo);
        memcpy(SerBuffer,value,4);
        MInfo.nBytesToSend   = 4;
        status = M522PcdCmd(RC522_CMD_TRANSCEIVE,
                            SerBuffer,
                            &MInfo);

        if (status == STATUS_IO_TIMEOUT||(status == MIFARE_DECREMENT && OperMode == MIFARE_DECREMENT))
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_OTHER_ERROR;
        }
     }
     if ( status == STATUS_SUCCESS)
     {
        ResetInfo(MInfo);
        SerBuffer[0] = MIFARE_TRANSFER;
        SerBuffer[1] = trans_addr;
        MInfo.nBytesToSend   = 2;
        status = M522PcdCmd(RC522_CMD_TRANSCEIVE,
                            SerBuffer,
                            &MInfo);

        if (status & MIFARE_ACK_MASK)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = STATUS_OTHER_ERROR;
        }
     }
   return status;
}

/*************************************************
Function:       InitBlock
Description:
     initialize a block value
Parameter:
     addr       the address of the block
     value      the value to be initialized, 4 bytes buffer
Return:
     short      status of implement
**************************************************/
short InitBlock(unsigned char addr,unsigned char *value)
{
    unsigned char tmp[16],i;
    short status = STATUS_SUCCESS;
    for(i=0;i<4;i++)
    {
    	tmp[i]=value[i];
    	tmp[i+4]=255-value[i];
    	tmp[i+8]=value[i];
    }
    tmp[12]=addr;
    tmp[13]=255-addr;
    tmp[14]=tmp[12];
    tmp[15]=tmp[13];
    status=Write(addr,tmp);
    return status;
}
