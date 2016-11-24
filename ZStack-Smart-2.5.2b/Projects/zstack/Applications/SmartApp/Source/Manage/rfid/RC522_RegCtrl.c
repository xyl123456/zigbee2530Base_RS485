//////////////////////////////////////////////////////////////////////////////
//
//                  ELEKTOR RFID READER for MIFARE and ISO14443-A
//
//                      Copyright (c) 2006 Gerhard H. Schalk
//
//////////////////////////////////////////////////////////////////////////////
//#include <REG936.H> 
#include "RC522_ErrCode.h"
#include "I2C.h"
#include "RC522_RegCtrl.h"

/*************************************************
Function:       RcSetReg
Description:
     Write data to register of RC522
Parameter:
     RegAddr       The address of the regitster
     RegVal        The value to be writen
Return:
     None
**************************************************/
void RcSetReg(unsigned char RegAddr, unsigned char RegVal)
{    
    i2c_Start();                                   
    i2c_MasterTransmit(MFRC522_I2C_WRAdr); 
    i2c_MasterTransmit(RegAddr & 0x3F);   
    i2c_MasterTransmit(RegVal);
    i2c_Stop();   
}

/*************************************************
Function:       qRcGetReg
Description:
     Write data to register of RC522
Parameter:
     RegAddr       The address of the regitster to be readed
Return:
     The value of the specify register
**************************************************/
unsigned char RcGetReg(unsigned char RegAddr)
{
    unsigned char RegVal;

    unsigned char ackValueRD;
    
    i2c_Start();                                       
    ackValueRD = i2c_MasterTransmit(MFRC522_I2C_WRAdr);
    ackValueRD = i2c_MasterTransmit(RegAddr & 0x3F);    
    if(ackValueRD)          
    i2c_Stop();                                         
    
    i2c_Start();                                       
    ackValueRD = i2c_MasterTransmit(MFRC522_I2C_RDAdr); 
    RegVal = i2c_MasterReceive (NACK);                    
    i2c_Stop();                                         

    return RegVal;   

}

/*************************************************
Function:       RcModifyReg
Description:
     Change some bits of the register
Parameter:
     RegAddr       The address of the regitster
     ModifyVal        The value to change to, set or clr?
     MaskByte      Only the corresponding bit '1' is valid,
Return:
     None
**************************************************/

void RcModifyReg(unsigned char RegAddr, unsigned char ModifyVal, unsigned char MaskByte)
{
    unsigned char RegVal;
    RegVal = RcGetReg(RegAddr);
    if(ModifyVal)
    {
        RegVal |= MaskByte;
    }
    else
    {
        RegVal &= (~MaskByte);
    }
    RcSetReg(RegAddr, RegVal);
}
