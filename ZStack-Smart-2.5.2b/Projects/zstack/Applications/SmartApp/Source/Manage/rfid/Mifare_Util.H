//////////////////////////////////////////////////////////////////////////////
//
//                  ELEKTOR RFID READER for MIFARE and ISO14443-A
//
//                      Copyright (c) 2006 Gerhard H. Schalk
//         
//////////////////////////////////////////////////////////////////////////////
#ifndef __MIFARE_UTIL_H_INCLUDED
#define __MIFARE_UTIL_H_INCLUDED
    
    #define ATQA_Philips_Mifare_UL  0x4400  // ATQA Byte 0 | ATQA Byte 1
    #define ATQA_Philips_Mifare_1K  0x0400  // ATQA Byte 0 | ATQA Byte 1
    #define ATQA_Philips_Mifare_4K  0x0200  // ATQA Byte 0 | ATQA Byte 1
    #define ATQA_Philips_DesFire    0x4403  // ATQA Byte 0 | ATQA Byte 1
    
    #define SAK_Philips_Mifare_UL   0x00
    #define SAK_Philips_Mifare_1K   0x08
    #define SAK_Philips_Mifare_4K   0x18
    #define SAK_Philips_DesFire     0x20
    
    enum MifareICType
    {
        Unknown,
        Mifare_UL,
        Mifare_1K,
        Mifare_4K,
        Mifare_DesFire
    };
    
    
    
    short ActivateCard(unsigned char req_code, unsigned char *atq, 
                       unsigned char *uid, unsigned char *uid_len, 
                       unsigned char *sak );
    
    enum MifareICType MIFARE_TypeIdentification(unsigned char *baATQ, unsigned char bSAK);
    void PrintMifareULPage(unsigned char *DataBuffer);

#endif
