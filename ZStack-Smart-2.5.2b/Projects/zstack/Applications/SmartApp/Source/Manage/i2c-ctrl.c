#include "i2c-ctrl.h"

void iDelay(unsigned long n)
{
  volatile unsigned long i;
  for(i = n;i > 0;i--)
    asm("NOP");
}

void Start_I2c(void)
{
	P0DIR |= 0xC0;   /* P0_6, P0_7输出 */
	//SCL=0;
	SDA=1;   /*发送起始条件的数据信号*/
	iDelay(50);
	SCL=1;
	iDelay(200);    /*起始条件建立时间大于4.7us,延时*/ 
	SDA=0;   /*发送起始信号*/
	iDelay(200);    /* 起始条件锁定时间大于4μs*/    
	SCL=0;   /*钳住I2C总线，准备发送或接收数据 */
	iDelay(50);
}

void Stop_I2c(void)
{
	P0DIR |= 0x40;   /* P0_6输出 */
	//SCL=0;
	//iDelay(200);
	SDA=0;  /*发送结束条件的数据信号*/
	iDelay(200);   /*发送结束条件的时钟信号*/
	SCL=1;  /*结束条件建立时间大于4μs*/
	iDelay(200);
	SDA=1;  /*发送I2C总线结束信号*/
	iDelay(50);
}


/* 应答子函数 */
void Ack_I2c(uint8 a)
{
	P0DIR |= 0x40;   /* P0_6输出 */
	if(a==0)SDA=0;     /*在此发出应答或非应答信号 */
	else SDA=1;
	iDelay(50);      
	SCL=1;
	iDelay(200);               /*时钟低电平周期大于4μs*/

	SCL=0;                /*清时钟线，钳住I2C总线以便继续接收*/
	iDelay(50);    
}


/* 字节数据传送函数 */
void  SendByte(uint8 c)
{
	uint8 i;

	P0DIR |= 0x40;   /* P0_6输出 */
	for (i=0x80;i>0;i/=2)             //shift bit for masking
	{  
		if (i & c) SDA=1;
		else SDA=0; 

		iDelay(50);
		SCL=1;               /*置时钟线为高，通知被控器开始接收数据位*/
		iDelay(200);               /*保证时钟高电平周期大于4μs*/     
		SCL=0; 
	}  

	iDelay(50);
	SDA=1;               /*8位发送完后释放数据线，准备接收应答位*/
	iDelay(50);   
	SCL=1;
	iDelay(50);
	P0DIR &= (~0x40);/* P0_6输入 */
	if(SDA == 1)ack=0;     
	else ack=1;  

	SCL=0;
	iDelay(50);
}


/* 字节数据接收函数  */
uint8  RecvByte(void)
{
	uint8 retc;
	uint8 i;

	P0DIR |= 0x40;   /* P0_6输出 */
	retc=0; 
	SDA=1; 
	//LY_GPIO_Ctrl(LY_DEV_LED2,LY_LED_ON);
	P0DIR &= (~0x40);/* P0_6输入 *//*置数据线为输入方式*/
	iDelay(200);
	for (i=0x80;i>0;i/=2)             
	{           
		SCL=1;       /*置时钟线为高使数据线上数据有效*/
		iDelay(100);
		//retc=retc<<1;
		if (SDA==1)   
			retc=(retc | i);
		SCL=0;       /*置时钟线为低，准备接收数据位*/
		iDelay(200);         /*时钟低电平周期大于4.7μs*/  
	}
	SCL=0;    
	iDelay(50);
	return(retc);
}


/* 向无子地址器件读字节数据函数  */
uint8 IRecvByte(uint16 sla)
{
   uint8 c;
   
   uint8 slaveHighAddr = (sla>>8);
   uint8 slaveLowAddr = (sla&0xff);
   
   Start_I2c();                /*启动总线*/
   SendByte(IIC_WRITE_SLAVE_ADDR);  /*发送器件地址*/
     if(ack==0)return(0);
   SendByte(slaveHighAddr);  /*发送器件16位子地址高位字节*/
     if(ack==0)return(0);
   SendByte(slaveLowAddr);  /*发送器件16位子地址低位字节*/
     if(ack==0)return(0);
   Start_I2c();   
   SendByte(IIC_READ_SLAVE_ADDR); /*发送器件地址*/
     if(ack==0)return(0); 
   c = RecvByte();               /*读取数据*/
     Ack_I2c(1);               /*发送非就答位*/
  Stop_I2c();                  /*结束总线*/ 

  return c;
}


/* 向无子地址器件发送字节数据函数   */
uint8 ISendByte(uint16 sla, uint8 c)
{
   uint8 slaveHighAddr = (sla>>8);
   uint8 slaveLowAddr = (sla&0xff);
   Start_I2c();               /*启动总线*/
   SendByte(IIC_WRITE_SLAVE_ADDR);  /*发送器件地址*/
     if(ack==0)return(0);
   SendByte(slaveHighAddr);  /*发送器件16位子地址高位字节*/
     if(ack==0)return(0);
   SendByte(slaveLowAddr);  /*发送器件16位子地址低位字节*/
      if(ack==0)return(0);
   SendByte(c);               /*发送数据*/
     if(ack==0)return(0);
   Stop_I2c();                 /*结束总线*/ 
   
   return(1);
}



