//////////////////////////////////////////////////////////////////////////////
//
//                  ELEKTOR RFID READER for MIFARE and ISO14443-A
//
//                      Copyright (c) 2006 Gerhard H. Schalk
//         
//////////////////////////////////////////////////////////////////////////////
#ifndef __I2C_H_INCLUDED
#define __I2C_H_INCLUDED
    #define     HIGH    1
    #define     LOW     0

    #define     ON  1
    #define     OFF 0
    
    #define SCL    P0_7
    #define SDA    P0_6
    void Delay(unsigned int n);
    void i2c_Start (void);
    void i2c_Stop  (void);
    unsigned char i2c_MasterTransmit(unsigned char output_data);
    unsigned char i2c_MasterReceive(unsigned char ACKValue);
#endif
