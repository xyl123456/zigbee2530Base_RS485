#ifndef  __SHT10_H__
#define  __SHT10_H__
#include <ioCC2530.h>
#define SHT11_DATA_FLOAT  0x01 //开启精度
#define SHT11_DATA_BYTE   //设置数据格式是字节

#define SHT11_SCL      P0_7  //时钟线
#define SHT11_DATA   P0_6  //数据线

#define  SHT11_Init()   { P0SEL &= ~0xc0;P0DIR |=0xc0; }//io口初始化

#define SHT11_DATA_IN    P0DIR &= ~0x40;//数据为输入模式
#define SHT11_DATA_OUT    P0DIR |=  0x40;//数据为输出模式

#define SHT11_SCL_SET         SHT11_SCL=1
#define SHT11_SCL_RESET     SHT11_SCL=0

#define SHT11_TEMP    0x03    //测温度
#define SHT11_HUMI    0x05    //测湿度
#define SHT11_READ    0x07   //读状态寄存器
#define SHT11_WRITE  0x06    //写状态寄存器
#define SHT11_RESET   0x1e   //软件复位

#define MODE_TEMP    0    //温度模式
#define MODE_HUMI    1    //湿度模式

#define  SUCCESS_1    1
#define  ERROR_1      2
#define  noACK      0
#define  ACK        1

typedef struct SHT11_DATA_STAUCT
{
  unsigned char Temp_byte[2];//温度字符串字节表示
  unsigned char Humi_byte[2];//湿度字符串字节表示
#ifndef SHT11_DATA_FLOAT
  int Temp;//温度
  int Humi;//湿度
  unsigned char Temp_s[4];//温度字符串
  unsigned char Humi_s[4];//湿度字符串
#else
  float  Temp;
  float  Humi;
  unsigned char Temp_s[6];//温度字符串
  unsigned char Humi_s[6];//湿度字符串
  #endif
  unsigned char length_temp;
  unsigned char length_humi;
}SHT11_DATA_STRUCT;

extern SHT11_DATA_STRUCT SHT11;

void SHT11_Delay(unsigned char us);
void SHT11_Delay_Ms(unsigned char ms);
void SHT11_Start(void);
void SHT11_Reset(void);
unsigned char SHT11_Write_Byte(unsigned char byte);
char SHT11_Read_Byte(unsigned char ack_status);
unsigned char SHT11_Order(unsigned char mode);

void SHT11_Get_Data(void);
void SHT11_Real_Data(void);

extern void SHT11_Finish(void);
#endif
