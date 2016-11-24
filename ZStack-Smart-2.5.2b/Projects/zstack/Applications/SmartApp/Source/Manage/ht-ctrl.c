#include "ht-ctrl.h"
#include <iocc2530.h>
#include <string.h>


#define	DATA   	P0_6
#define	SCK   	P0_7

#define noACK 1
#define ACK   0
                            //adr  command  r/w
#define STATUS_REG_W 0x06   //000   0011    0
#define STATUS_REG_R 0x07   //000   0011    1
#define MEASURE_TEMP 0x03   //000   0001    1
#define MEASURE_HUMI 0x05   //000   0010    1
#define RESET        0x1e   //000   1111    0

char s_write_byte(unsigned  value);
char s_read_byte(uint8 ack);
void s_transstart(void);
char s_softreset(void);
char s_write_statusreg(uint8 *p_value);
char s_read_statusreg(uint8 *p_value, uint8 *p_checksum);


static void tDelay(uint8 n)
{
  uint8 i;
  for(i = 0;i < n;i++)
    asm("NOP");
}


//----------------------------------------------------------------------------------
// writes a byte on the Sensibus and checks the acknowledge 
char s_write_byte(unsigned  value)
{ 
  uint8 i,error=0;  
  
  P0DIR |= 0xC0;
  for (i=0x80;i>0;i/=2)             //shift bit for masking
  { if (i & value) DATA=1;          //masking value with i , write to SENSI-BUS
    else DATA=0;                        
    SCK=1;                          //clk for SENSI-BUS
    tDelay(6); 	
    SCK=0;
  }
  DATA=1;                           //release DATA-line
  P0DIR &= (~0x40);/* P0_6ÊäÈë */
  SCK=1;                            //clk #9 for ack 
  error=DATA;                       //check ack (DATA will be pulled down by SHT11)
  SCK=0;        
  return error;                     //error=1 in case of no acknowledge
}


//----------------------------------------------------------------------------------
// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1" 
char s_read_byte(uint8 ack)
{ 
  uint8 i,val=0;
  DATA=1;
  
  P0DIR |= 0xC0;
  for (i=0x80;i>0;i/=2)             //shift bit for masking
  { SCK=1;                          //clk for SENSI-BUS
    if (DATA) val=(val | i);        //read bit  
    SCK=0;  					 
  }
  DATA=!ack;                        //in case of "ack==1" pull down DATA-Line
  SCK=1;                            //clk #9 for ack
   tDelay(6);
  SCK=0;						    
  DATA=1;                           //release DATA-line
  return val;
}

//----------------------------------------------------------------------------------
// generates a transmission start 
//       _____         ________
// DATA:      |_______|
//           ___     ___
// SCK : ___|   |___|   |______
void s_transstart(void)
{  
   P0DIR |= 0xC0;
   DATA=1; SCK=0;                   //Initial state
   tDelay(6);
   SCK=1;
   tDelay(6);
   DATA=0;
   tDelay(6);
   SCK=0;  
   tDelay(16);
   SCK=1;
   tDelay(6);
   DATA=1;		   
   tDelay(6);
   SCK=0;		   
}

//----------------------------------------------------------------------------------
// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
void s_connectionreset(void)
{  
  uint8 i; 
  P0DIR |= 0xC0;
  DATA=1; SCK=0;                    //Initial state
  for(i=0;i<9;i++)                  //9 SCK cycles
  { SCK=1;
    tDelay(6);
    SCK=0;
  }
  s_transstart();                   //transmission start
}

//----------------------------------------------------------------------------------
// resets the sensor by a softreset 
char s_softreset(void)
{ 
  uint8 error=0;  
  s_connectionreset();              //reset communication
  error+=s_write_byte(RESET);       //send RESET-command to sensor
  return error;                     //error=1 in case of no response form the sensor
}

//----------------------------------------------------------------------------------
// reads the status register with checksum (8-bit)
char s_read_statusreg(uint8 *p_value, uint8 *p_checksum)
{ 
  uint8 error=0;
  s_transstart();                   //transmission start
  error=s_write_byte(STATUS_REG_R); //send command to sensor
  *p_value=s_read_byte(ACK);        //read status register (8-bit)
  *p_checksum=s_read_byte(noACK);   //read checksum (8-bit)  
  return error;                     //error=1 in case of no response form the sensor
}

//----------------------------------------------------------------------------------
// writes the status register with checksum (8-bit)
char s_write_statusreg(uint8 *p_value)
{ 
  uint8 error=0;
  s_transstart();                   //transmission start
  error+=s_write_byte(STATUS_REG_W);//send command to sensor
  error+=s_write_byte(*p_value);    //send value of status register
  return error;                     //error>=1 in case of no response form the sensor
}
 							   
//----------------------------------------------------------------------------------
// makes a measurement (humidity/temperature) with checksum
char s_measure(uint8 *p_value, uint8 *p_checksum, uint8 mode)
{ 
  uint8 error=0;
  uint32 i;

  s_transstart();                   //transmission start
  switch(mode){                     //send command to sensor
    case TEMP	: error+=s_write_byte(MEASURE_TEMP); break;
    case HUMI	: error+=s_write_byte(MEASURE_HUMI); break;
    default		: break;	 
  }
  for (i=0;i<1000000;i++) if(DATA==0) break; //wait until sensor has finished the measurement
  if(DATA) error+=1;                // or timeout (~2 sec.) is reached
  *(p_value+1)  =s_read_byte(ACK);    //read the first byte (MSB)
  *(p_value)=s_read_byte(ACK);    //read the second byte (LSB)
  *p_checksum =s_read_byte(noACK);  //read checksum
  return error;
}



//----------------------------------------------------------------------------------------
// calculates temperature [°C] and humidity [%RH] 
// input :  humi [Ticks] (12 bit) 
//          temp [Ticks] (14 bit)
// output:  humi [%RH]
//          temp [°C]
void calc_sht11(float *p_humidity ,float *p_temperature)
{ 
  const float C1=-2.0468;           // for 12 Bit
  const float C2=+0.0367;           // for 12 Bit
  const float C3=-0.0000016;        // for 12 Bit
  const float T1=+0.01;             // for 14 Bit @ 5V
  const float T2=+0.00008;          // for 14 Bit @ 5V	

  float rh=*p_humidity;             // rh:      Humidity [Ticks] 12 Bit 
  float t=*p_temperature;           // t:       Temperature [Ticks] 14 Bit
  float rh_lin;                     // rh_lin:  Humidity linear
  float rh_true;                    // rh_true: Temperature compensated humidity
  float t_C;                        // t_C   :  Temperature [°C]

  t_C=t*0.01 - 40;                  //calc. temperature from ticks to [°C]
  rh_lin=C3*rh*rh + C2*rh + C1;     //calc. humidity from ticks to [%RH]
  rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;   //calc. temperature compensated humidity [%RH]
  if(rh_true>100)rh_true=100;       //cut if the value is outside of
  if(rh_true<0.1)rh_true=0.1;       //the physical possible range

  *p_temperature=t_C;               //return temperature [°C]
  *p_humidity=rh_true;              //return humidity[%RH]
}