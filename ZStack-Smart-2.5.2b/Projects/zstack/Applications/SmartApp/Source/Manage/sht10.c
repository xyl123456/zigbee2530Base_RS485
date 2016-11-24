#include "sht10.h"
struct SHT11_DATA_STAUCT  SHT11;
static unsigned char flag = 0;

void SHT11_Delay(unsigned char us)
{
	while(us--)
	{
		asm("NOP");
		asm("NOP");
		asm("NOP");
		asm("NOP");
		asm("NOP");
		asm("NOP");
		asm("NOP");
		asm("NOP");
	}
}

void SHT11_Delay_Ms(unsigned char ms)
{
	unsigned char i;
	for(i=0;i<ms;i++)
	{
	SHT11_Delay(250);
	SHT11_Delay(250);
	SHT11_Delay(250);
	SHT11_Delay(250);
	}
}

/********************
*数据线依次输入1  0  表示启动
时钟的时序为时钟线拉低再拉高
*********************/
void SHT11_Start(void)
{
	SHT11_DATA_OUT;//数据为输出模式
/*-------------------1--------------*/
	SHT11_DATA  = 1;//数据为1

	SHT11_SCL_RESET;//scl=0
	SHT11_Delay(5);
	SHT11_SCL_SET;//scl =1
	
	SHT11_Delay(1);
/*-------------------0--------------*/
	SHT11_DATA =0;//数据为0
	SHT11_SCL_RESET;//scl=0
	SHT11_Delay(5);
	SHT11_SCL_SET;//scl=1

	SHT11_DATA = 1;//SDA=1
	SHT11_SCL_RESET;//SCL=0
	
}

void SHT11_Reset(void)
{
	unsigned char i;
	SHT11_DATA_OUT;//输出模式
	SHT11_DATA=1;
	asm("NOP");
	SHT11_SCL_RESET;
	asm("NOP");
	for(i=0;i<9;i++)
	{
		SHT11_SCL_SET;
		asm("NOP");
		SHT11_SCL_RESET;
		asm("NOP");
	}
	SHT11_Start();//启动SHT11
	
}

unsigned char SHT11_Write_Byte(unsigned char byte)
{
		unsigned char i;
		unsigned char status =1;//状态
		SHT11_DATA_OUT;//数据输出模式
		for(i=0;i<8;i++)
		{
		//一个字节由高位到低位写入SHT11
		if((byte & 0x80)==0x80) 
			SHT11_DATA =1;
		else
			SHT11_DATA =0;
		//拉高时钟线,延时5US,脉冲写入
		SHT11_SCL_SET;
		SHT11_Delay(5);
		SHT11_SCL_RESET;
		byte <<=1;
		}
		SHT11_DATA =1;//释放数据线
		SHT11_DATA_IN;//输入模式，未接收准备
		SHT11_SCL_SET;//scl=1
		status = SHT11_DATA;//接收应答
		//默认状态
		SHT11_SCL_RESET;//拉低
		if(status == 0)
			{
			return SUCCESS_1;
			}
		else
			{
			return ERROR_1;
			}		
}

char SHT11_Read_Byte(unsigned char ack_status)
{
	unsigned char i;
	unsigned char byte =0;
	SHT11_DATA_IN;//输入模式
	for(i=0;i<8;i++)
	{
	byte <<=1;//往高位移1位
	SHT11_SCL_SET;//scl=1
	SHT11_Delay(5);

	if(SHT11_DATA)
		{
		byte |=0x01;
		}
	SHT11_SCL_RESET;//SCL=0
	}

	SHT11_DATA_OUT;//输出模式
	SHT11_DATA = !ack_status;//是否要应答
	//拉高时钟等待应答
	SHT11_SCL_SET;
	SHT11_Delay(5);
	SHT11_SCL_RESET;

	SHT11_DATA =1;//释放数据线
	return byte;
}

/*MODE 选择采集湿度还是温度
*/
unsigned char SHT11_Order(unsigned char mode)
{
	unsigned char status;//success or error
	unsigned char i;
	unsigned char count = 0;
	if(mode == MODE_TEMP)
		{
		status = SHT11_Write_Byte(SHT11_TEMP);//监测温度
		}
	else if(mode == MODE_HUMI)
		{
		status = SHT11_Write_Byte(SHT11_HUMI);//监测湿度
		}
	if(status == SUCCESS_1)
		{
		/*写入命令成功*/
		while(SHT11_DATA)
			{
			for(i=0;i<200;i++)
				{
				SHT11_Delay(250);
				SHT11_Delay(250);
				}//100ms
				count++;
			if(count >=20)	
				{
				status = ERROR_1;
				break;
				//超过2S未响应
				}
			}
		}
	else
		{
		SHT11_Reset();
		}
	return status;
}

void SHT11_Get_Data(void)
{
	unsigned int value =0;
	SHT11_Reset();//复位
	if(SHT11_Order(MODE_HUMI)==SUCCESS_1)
		{
		SHT11_Delay(250);
		*((unsigned char *)&value+1) = SHT11_Read_Byte(ACK);//高8位
		*((unsigned char *)&value) = SHT11_Read_Byte(ACK);//低8位
		SHT11_Read_Byte(noACK);//校验和
		}
	SHT11.Humi = value;//记录湿度
	value =0;
	
	SHT11_Reset();
	SHT11_Delay_Ms(200);
	if (SHT11_Order(MODE_TEMP)==SUCCESS_1)
		{
		SHT11_Delay(250);
		*((unsigned char *)&value+1) = SHT11_Read_Byte(ACK);//高8位
		*((unsigned char *)&value) = SHT11_Read_Byte(ACK);//低8位
		SHT11_Read_Byte(noACK);//校验和
		}
	SHT11.Temp = value;
}

void SHT11_Real_Data(void)
{
	SHT11_Get_Data();//获取到温度和湿度数据
	const float C1 = -2.0468;//
	const float C2 = +0.0367;
	const float C3 = -0.0000015955;
	const float T1 = +0.01;
	const float T2 = +0.00008;

	float rh = SHT11.Humi;//12位湿度
	float t = SHT11.Temp;//14为温度
	float rh_lin;//温度极限值
	float rh_true;//真实值
	float t_C;

	//t_C = t*0.01-40.1;//供电电压5V
        t_C = t*0.01-39.6;//供电电压3.3V
        if(t_C<15)
          t_C=0;
        else
          t_C=t_C;
	rh_lin = C3*rh*rh+C2*rh+C1;//相对湿度补偿
	rh_true=(t_C-25)*(T1+T2*rh)+rh_lin;//相对湿度补偿
	if(rh_true>100)
		rh_true=100;
	if(rh_true<0.1)
		rh_true=0.1;
	#ifndef SHT11_DATA_FLOAT
	SHT11.Temp = (int)t_C;//返回温度
	SHT11.Humi = (int)rh_true;//返回湿度
	#else
	SHT11.Temp = t_C;  //返回温度
	SHT11.Humi = rh_true;//返回湿度
	#endif
	
}

void SHT11_Finish(void)
{
#ifdef SHT11_DATA_FLOAT
	unsigned int temp;//中间变量
#endif
	if(flag == 0)
		{
                //执行一次初始化
		SHT11_Init();
		flag =1;
		}
	SHT11_Real_Data();//获取数据
	SHT11_Delay_Ms(100);
#ifndef SHT11_DATA_FLOAT
/*--------温度转换字符------------*/
if(SHT11.Temp/100 !=0)
	{
	SHT11.Temp_s[0]=SHT11.Temp/100+0x30;//百位
	SHT11.Temp_s[1]=SHT11.Temp%100/10+0x30;
	SHT11.Temp_s[2]=SHT11.Temp%10+0x30;
	SHT11.Temp_s[3]='\0';
	SHT11.length_temp=3;
	}
else
	{
	if(SHT11.Temp%100/10 !=0)//十位不为0
		{
		SHT11.Temp_s[0]=SHT11.Temp/10+0x30;//十位
		SHT11.Temp_s[1]=SHT11.Temp%10+0x30;//个位
		SHT11.Temp_s[2]='\0';
		SHT11.length_temp=2;
		}
	else
		{
		SHT11.Temp_s[0]=SHT11.Temp+0x30;//个位
		SHT11.Temp_s[1]='\0';
		SHT11.length_temp=1;
		}
	
	}
/*--------------湿度转换字符串-------------*/
	if(SHT11.Humi/100 !=0)
		{
			SHT11.Humi_s[0]=SHT11.Humi/100+0x30;//百位
			SHT11.Humi_s[1]=SHT11.Humi%100/10+0x30;
			SHT11.Humi_s[2]=SHT11.Humi%10+0x30;
			SHT11.Humi_s[3]='\0';
			SHT11.length_humi=3;
		}
	else
		{
			if(SHT11.Humi%100/10 !=0)
				{
				SHT11.Humi_s[0]=SHT11.Humi/10+0x30;
				SHT11.Humi_s[1]=SHT11.Humi%10+0x30;
				SHT11.Humi_s[3]='\0';
				SHT11.length_humi=2;
				}
			else
				{
				SHT11.Humi_s[0]=SHT11.Humi+0X30;
				SHT11.Humi_s[1]='\0';
				SHT11.length_humi=1;
				}
		}
#else

/*--------温度转换字符------------*/
	temp = (unsigned int)(SHT11.Temp*10);//记录小数点下一位
#ifdef SHT11_DATA_BYTE
        SHT11.Temp_byte[0]=(temp>>8)&0xff;
        SHT11.Temp_byte[1]=temp&0xff;
#else
	if(temp/1000 !=0)//百位不为0
		{
		SHT11.Temp_s[0]=temp/1000+0x30;//百位
		SHT11.Temp_s[1]=temp%1000/100+0x30;//十位
		SHT11.Temp_s[2]=temp%100/10+0x30;//个位
		SHT11.Temp_s[3]=0x2E;
		SHT11.Temp_s[4]=temp%10+0x30;//小数后一位
		SHT11.Temp_s[5]='\0';
		SHT11.length_temp=5;
		}
	else
	{
		if(temp/100 !=0)//十位不为0
		{
		SHT11.Temp_s[0]=temp/100+0x30;//十位
		SHT11.Temp_s[1]=temp%100/10+0x30;//个位
		SHT11.Temp_s[2]=0x2E;
		SHT11.Temp_s[3]=temp%10+0x30;//小数后一位
		SHT11.Temp_s[4]='\0';
		SHT11.length_temp=4;
		}
		else
		{
		SHT11.Temp_s[0]=temp/10+0x30;//个位
		SHT11.Temp_s[1]=0x2E;
		SHT11.Temp_s[2]=temp%10+0x30;//小数后一位
		SHT11.Temp_s[3]='\0';
		SHT11.length_temp=3;
		}
	}
#endif
/*--------------湿度转换字符串-------------*/
	temp=(unsigned int)(SHT11.Humi*10);
#ifdef SHT11_DATA_BYTE
        SHT11.Humi_byte[0]=(temp>>8)&0xff;
        SHT11.Humi_byte[1]=temp&0xff;
#else
	if(temp/1000 !=0)
		{
		SHT11.Humi_s[0]=temp/1000+0x30;//百位
		SHT11.Humi_s[1]=temp%1000/100+0x30;//十位
		SHT11.Humi_s[2]=temp%100/10+0x30;//个位
		SHT11.Humi_s[3]=0x2E;
		SHT11.Humi_s[4]=temp%10+0x30;//小数后一位
		SHT11.Humi_s[5]='\0';
		SHT11.length_humi=5;
		}
	else
		{
		if(temp/100 !=0)//十位不为0
			{
			SHT11.Humi_s[0]=temp/100+0x30;//十位
			SHT11.Humi_s[1]=temp%100/10+0x30;//个位
			SHT11.Humi_s[2]=0x2E;
			SHT11.Humi_s[3]=temp%10+0x30;//小数后一位
			SHT11.Humi_s[4]='\0';
			SHT11.length_humi=4;
			}
		else
			{
			SHT11.Humi_s[0]=temp/10+0x30;//个位
			SHT11.Humi_s[1]=0x2E;
			SHT11.Humi_s[2]=temp%10+0x30;//小数后一位
			SHT11.Humi_s[3]='\0';
			SHT11.length_humi=3;
			}
		}
#endif
#endif
}

