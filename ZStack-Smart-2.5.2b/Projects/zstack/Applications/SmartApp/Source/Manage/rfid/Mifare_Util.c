//////////////////////////////////////////////////////////////////////////////
//
//                  ELEKTOR RFID READER for MIFARE and ISO14443-A
//
//                      Copyright (c) 2006 Gerhard H. Schalk
//         
//////////////////////////////////////////////////////////////////////////////
#include "stdio.h"
#include "RC522_ErrCode.h"
#include "RC522_Mifare.h"
#include "Mifare_Util.h"

//------------------------------------------------------------------------
// Function: ActivateCard
//------------------------------------------------------------------------
// Description:
// This Function performs a REQA or WUPA, Anticollision, Select sequence
// to activate a single or double UID card.
//------------------------------------------------------------------------
// Parameters:
// IN:  req_code:  command code(ISO14443_3_REQA or ISO14443_3_WUPA)
// OUT: *atq:      2 byte buffer to save the card Answer to Request (ATQA)
// OUT: *uid:      8 byte buffer to save the card UID
// OUT: *uid_len:  1 byte to save Card UID Length;
// (uid_len = 4 for Single UID card or uid_len = 8 for Double UID card)
// OUT: *sak:      1 byte to save the card Select Acknowledge (SAK) info
// Return:         Error Code (RC522_ErrCode.H)
//------------------------------------------------------------------------
short ActivateCard(unsigned char req_code, unsigned char *atq, 
                   unsigned char *uid, unsigned char *uid_len, 
                   unsigned char *sak )
{
    short status;

    status = Request(req_code, atq);
    if(status != STATUS_SUCCESS)
        return status;

    status = CascAnticoll(0x93,0,&uid[0]);
    if(status != STATUS_SUCCESS)
        return status;   
    
    status = Select(0x93, &uid[0], sak);           
    if(status != STATUS_SUCCESS)
        return status;
    
    // Check if UID is complete
    if((*sak & 0x04) == 0x04)
    {   
        // UID not complete
        status = CascAnticoll(0x95,0,&uid[4]);
        if(status != STATUS_SUCCESS)
        return status;

        status = Select(0x95, &uid[4], sak);
        if(status != STATUS_SUCCESS)
            return status;

        *uid_len = 8;
        return STATUS_SUCCESS;   
    }
    else 
    {
        *uid_len = 4;
        return STATUS_SUCCESS;
    }
}

//------------------------------------------------------------------------
// Function: MIFARE_TypeIdentification
//------------------------------------------------------------------------
// Description:
// This function delivers the chip type information for all 
// Philips MIFARE ICs 
//------------------------------------------------------------------------
// Parameters:
// IN: *baATQ: 2 byte buffer; Card Answer to Request (ATQA)  
// IN: *sak:   1 byte; Card Select Acknowledge (SAK) 
// Return: Mifare Card type (Unknown, Mifare_UL, Mifare_1K, Mifare_4K or
//                           Mifare_DesFire)
//------------------------------------------------------------------------
enum MifareICType MIFARE_TypeIdentification(unsigned char *baATQ, unsigned char bSAK)
{
    unsigned int iATQA;
    
    iATQA = baATQ[0] *256 + baATQ[1];
    
    if((iATQA == ATQA_Philips_Mifare_UL) && (bSAK == SAK_Philips_Mifare_UL))
        return Mifare_UL;
    else if((iATQA == ATQA_Philips_Mifare_1K) && (bSAK == SAK_Philips_Mifare_1K))
        return Mifare_1K;
    else if((iATQA == ATQA_Philips_Mifare_4K) && (bSAK == SAK_Philips_Mifare_4K))
        return Mifare_4K;
    else if((iATQA == ATQA_Philips_DesFire) && (bSAK == SAK_Philips_DesFire))
        return Mifare_DesFire;	
    else 
        return Unknown;
}

//------------------------------------------------------------------------
// Function: PrintMifareULPage
//------------------------------------------------------------------------
// Description:
// This function formats 4 Data Bytes of the Mifare Ultra Light and sends
// the information via RS232/USB with the help of the printf function.
//------------------------------------------------------------------------
// Parameters:
// IN: *DataBuffer: 4 Byte DataBuffer 
//------------------------------------------------------------------------
/*
void PrintMifareULPage(unsigned char *DataBuffer)
{
    unsigned char i;
    char ASCIIOutput[4];

    // Print 4 Byte HEX Values
    printf("0x%b02X %b02X %b02X %b02X",DataBuffer[0],DataBuffer[1],DataBuffer[2],DataBuffer[3]);

    for(i=0; i<4; i++)
    {
        if(DataBuffer[i] < 32)
            ASCIIOutput[i] = '.';
        else
            ASCIIOutput[i] = DataBuffer[i]; 
    }
    
    // Print 4 Byte ASCII Values
    printf("    %c%c%c%c\r\n",ASCIIOutput[0],ASCIIOutput[1],ASCIIOutput[2],ASCIIOutput[3]);
}
*/










