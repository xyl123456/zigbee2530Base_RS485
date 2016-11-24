#ifndef __BH1750_H
#define __BH1750_H

#include <ioCC2530.h>
#include <ZComDef.h>

#define LUX_DATA_CHAR //输出定义字符转换
#define BH1750_Addr 0x46    //ADDR接地时的地址
#define BH1750_ON    0x01    //通电
#define BH1750_CON  0x10   //连续分辨率模式
#define BH1750_ONE   0x20  //一次分辨率模式
#define BH1750_RSET  0x07  //重置数组寄存器

#define st(x)      do { x } while (__LINE__ == -1)
#define HAL_IO_SET(port, pin, val)        HAL_IO_SET_PREP(port, pin, val)
#define HAL_IO_SET_PREP(port, pin, val)   st( P##port##_##pin## = val; )
#define HAL_IO_GET(port, pin)   HAL_IO_GET_PREP( port,pin)
#define HAL_IO_GET_PREP(port, pin)   ( P##port##_##pin)


//I2C接口是P0_6和P0_7
//时钟接口是P0_7,数据接口是P0_6
#define LIGHT_SCK_0()         HAL_IO_SET(0,7,0)
#define LIGHT_SCK_1()         HAL_IO_SET(0,7,1)
#define LIGHT_DTA_0()         HAL_IO_SET(0,6,0)
#define LIGHT_DTA_1()         HAL_IO_SET(0,6,1)

#define LIGHT_DTA()          HAL_IO_GET(0,6)
#define LIGHT_SCK()          HAL_IO_GET(0,7)

#define SDA_W() (P0DIR |=(1 << 6)  )
#define SDA_R() (P0DIR &=~(1 << 6) )

                       
#define LIGHT_INIT()                           \
do{                                            \
	P0SEL &= ~0x80;                        \
	P0DIR |=0x80;                           \
	P0_7 = 1;                                  \
	                                 \
	P0SEL &= ~0x40;                        \
	P0DIR |= 0x40;                         \
	P0_6 = 1; 	                             \
}while(0)
	
//extern unsigned short get_light(void);

extern uint16 Light(void);
extern void conversion(unsigned int temp_data,uint8 buf[]);
unsigned short get_light(void);

#endif // __BH1750_H

