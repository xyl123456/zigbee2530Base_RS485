#ifndef __I2C_CTRL_H__
#define __I2C_CTRL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_board.h"


#define SCL    P0_7
#define SDA    P0_6

#define IIC_READ_SLAVE_ADDR    161
#define IIC_WRITE_SLAVE_ADDR   160

static uint8 ack = 0;


void iDelay(unsigned long n);
void Start_I2c(void);
void Stop_I2c(void);
void Ack_I2c(uint8 a);
void  SendByte(uint8 c);

uint8  RecvByte(void);



extern uint8 IRecvByte(uint16 sla);
extern uint8 ISendByte(uint16 sla, uint8 c);

#ifdef __cplusplus
}
#endif

#endif
