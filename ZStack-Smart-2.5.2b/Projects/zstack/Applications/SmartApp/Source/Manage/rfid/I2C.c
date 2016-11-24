//////////////////////////////////////////////////////////////////////////////
//
//                  ELEKTOR RFID READER for MIFARE and ISO14443-A
//
//                      Copyright (c) 2006 Gerhard H. Schalk
//
//////////////////////////////////////////////////////////////////////////////
//#include <REG936.H> 
//#include <intrins.h> 
#include <iocc2530.h>
#include "I2C.h"

//------------------------------------------------------------------------
// Function: i2c_Start
//------------------------------------------------------------------------
// Description:
// Generates a I2C start condition
//------------------------------------------------------------------------

void Delay(unsigned int n)
{
  unsigned int i,tt;
  for(i = 0;i < n;i++)
    for(tt = 0;tt < 5;tt++)
      asm("NOP");
}

void i2c_Start (void)
{
    // Set SDA and SCL to HIGH (I2C Bus not busy)
    SDA = HIGH;
    SCL = HIGH;	
    Delay(30);
    // Start Condition
    SDA = LOW;
    Delay(30);
    SCL = LOW;
    Delay(30);

}

//------------------------------------------------------------------------
// Function: i2c_Stop
//------------------------------------------------------------------------
// Description:
// Generates a I2C stop condition
//------------------------------------------------------------------------
void i2c_Stop (void)
{
    SCL = LOW;
    SDA = LOW;

    // Stop Condition
    //_nop_();
    //_nop_();
    Delay(30);
    SCL = HIGH;
   // _nop_();
   //_nop_();
    Delay(30);
    SDA = HIGH;
    //_nop_();
    //_nop_();
    Delay(30);
}

//-------------------------------------------------------------------------
//  Funktion: char i2c_MasterTransmit (unsigned char output_data)
//  Input:    I2C Output Data
//  Return:   0..ACK / 1..NACK .. vom Slave
//  Schreibt Daten in einen I2C Slave
//-------------------------------------------------------------------------


//------------------------------------------------------------------------
// Function: i2c_MasterTransmit
//------------------------------------------------------------------------
// Description:
// Transmits a byte (slave address or data byte) from the I2C master 
// to the I2C Slave.
//------------------------------------------------------------------------
// Parameters:
// IN output_data:  Data byte to transmit.
// Return:   0..Slave ACK / 1.. Slave NACK
//------------------------------------------------------------------------
unsigned char i2c_MasterTransmit (unsigned char output_data)
{
    unsigned char i, ack_Status;
    unsigned char mask = 0x80;

    // Sends 8 Data Bit via I2C (MSB first)
    for(i = 0; i < 8; i++) 
    {
        SDA = (output_data & mask) ? 1 : 0;  // SDA = Data Bit
        mask  >>= 1;                                   // Shift Data Byte
        //_nop_();
        //_nop_();
        Delay(30);
        SCL = HIGH;                                    // Generate I2C clock
        //_nop_();
       // _nop_();
        Delay(30);
        SCL = LOW;
    }
    
    // Read Acknowledge from I2C Slave
    SDA =   HIGH;           // SDA in Input Mode 
    //_nop_();
    //_nop_();
    Delay(30);
    SCL = HIGH;             // Generate I2C clock
    ack_Status =   SDA;     // Read Slave ACK/NACK
    //_nop_();
    //_nop_();
    Delay(30);
    SCL = LOW;
    //_nop_();
    //_nop_();
    Delay(30);
    return ack_Status; // Return Acknowledge Status 0..ACK (OK), 1..NAK
}

//------------------------------------------------------------------------
// Function: i2c_MasterTransmit
//------------------------------------------------------------------------
// Description:
// The I2C Master receives a data byte from the I2C Slave.
//------------------------------------------------------------------------
// Parameters:
// IN output_data:  I2C ACK=1 or NACK=0 Byte 
//                  NACK .. last byte to receive
// Return:          Slave Data Byte
//------------------------------------------------------------------------
unsigned char i2c_MasterReceive (unsigned char ACKValue)
{
    unsigned char i;
    unsigned char input_data = 0;
    unsigned char mask = 0x80;
    
    // Reads 8 Data Bit via I2C (MSB first)
    for(i = 0; i < 8; i++)
    {
        SCL = HIGH;           
        if(SDA==HIGH)           // Read and store Data Bit 
            input_data |= mask;
        mask >>= 1;
        SCL = LOW;            
        Delay(30);
    }

    // Send Acknowledge to I2C Slave
    SDA = ACKValue;          // Send ACK
    SCL = HIGH;              
    Delay(30);
    SCL = LOW;               
    SDA = HIGH;              // SDA in Input Mode 
    Delay(30);
    return input_data;
}
