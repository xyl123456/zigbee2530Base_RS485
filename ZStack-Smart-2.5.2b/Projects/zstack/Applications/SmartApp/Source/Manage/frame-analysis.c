
/**************************************************************************************************
  Filename:       frame-analysis.c
  Revised:        $Date: 2014-04-25 17:16:57 -0800 (Wed, 25 Apr 2014) $
  Revision:       $Revision: 29217 $

  Description:    Analysis frame format.
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-07-31
**************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "serial-comm.h"
#include "frame-analysis.h"

#include "hal_adc.h"
#include "hal_led.h"
#include "hal_timer.h"
#include "OSAL_Nv.h"
#include "sht10.h"
#include "bh1750.h"
#include "list.h"
#include "CommonApp.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */  
static UC_t ucFrame;
static UO_t uoFrame;
static UH_t uhFrame;
static UR_t urFrame;
static DE_t deFrame;

static uint8 pFrameBuffer[FRAME_BUFFER_SIZE] = {0};
static uint8 pFrameLen = 0;

static uint8 aDataBuffer[FRAME_DATA_SIZE] = {0};
static uint8 aDataLen = 0;

const uint8 f_tail[4] = {0x3A, 0x4F, 0x0D, 0x0A}; 

uint8 Address_dev;//设备地址
uint8 Device_state[4]={0};//开关状态
/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * FUNCTION DECLARATION
 */

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SSAFrame_Analysis()
 *
 * @brief   analysis frame
 *
 * @param none
 *
 * @return  none
 */
void *SSAFrame_Analysis(frHeadType_t hType, uint8 SrcBuf[], uint8 SrcLen)
{ 
 	switch(hType)
	{
	case HEAD_UC: 
		if(SrcLen>=FR_UC_DATA_FIX_LEN && !memcmp(SrcBuf, FR_HEAD_UC, 3)
			&& SrcBuf[3] == FR_DEV_COORD && !memcmp(SrcBuf+34, f_tail, 4))
		{
			memcpy(ucFrame.head, SrcBuf, 3);
			ucFrame.type = SrcBuf[3];
			memcpy(ucFrame.ed_type, SrcBuf+4, 2);
			memcpy(ucFrame.short_addr, SrcBuf+6, 4);
			memcpy(ucFrame.ext_addr, SrcBuf+10, 16);
			memcpy(ucFrame.panid, SrcBuf+26, 4);
			memcpy(ucFrame.channel, SrcBuf+30, 4);
			memcpy(ucFrame.tail, SrcBuf+34, 4);

			return (void *)&ucFrame;
		}
		else { goto  FR_Analysis_err;}
		
	case HEAD_UO: 
		if(SrcLen>=FR_UO_DATA_FIX_LEN && !memcmp(SrcBuf, FR_HEAD_UO, 3)
			&& !memcmp(SrcBuf+SrcLen-4, f_tail, 4))
		{
			memcpy(uoFrame.head, SrcBuf, 3);
			uoFrame.type = SrcBuf[3];
			memcpy(uoFrame.ed_type, SrcBuf+4, 2);
			memcpy(uoFrame.short_addr, SrcBuf+6, 4);
			memcpy(uoFrame.ext_addr, SrcBuf+10, 16);
			
			memset(aDataBuffer, 0, sizeof(aDataBuffer));
			memcpy(aDataBuffer, SrcBuf+26, SrcLen-FR_UO_DATA_FIX_LEN);
			aDataLen = SrcLen-FR_UO_DATA_FIX_LEN;
			uoFrame.data_len = aDataLen;
			uoFrame.data = aDataBuffer;
			
			memcpy(uoFrame.tail, SrcBuf+SrcLen-4, 4);

			return (void *)&uoFrame;
		}
		else { goto  FR_Analysis_err;}
		
	case HEAD_UH: 
		if(SrcLen>=FR_UH_DATA_FIX_LEN && !memcmp(SrcBuf, FR_HEAD_UH, 3)
			&& !memcmp(SrcBuf+7, f_tail, 4))
		{
			memcpy(uhFrame.head, SrcBuf, 3);
			memcpy(uhFrame.short_addr, SrcBuf+3, 4);
			memcpy(uhFrame.tail, SrcBuf+7, 4);;

			return (void *)&uhFrame;
		}
		else { goto  FR_Analysis_err;}
		
	case HEAD_UR: 
		if(SrcLen>=FR_HR_DATA_FIX_LEN && !memcmp(SrcBuf, FR_HEAD_UR, 3)
			&& !memcmp(SrcBuf+SrcLen-4, f_tail, 4))
		{
			memcpy(urFrame.head, SrcBuf, 3);
			urFrame.type = SrcBuf[3];
			memcpy(urFrame.ed_type, SrcBuf+4, 2);
			memcpy(urFrame.short_addr, SrcBuf+6, 4);
			
			memset(aDataBuffer, 0, sizeof(aDataBuffer));
			memcpy(aDataBuffer, SrcBuf+10, SrcLen-FR_HR_DATA_FIX_LEN);
			aDataLen = SrcLen-FR_HR_DATA_FIX_LEN;
			urFrame.data_len = aDataLen;
			urFrame.data = aDataBuffer;
			
			memcpy(urFrame.tail, SrcBuf+SrcLen-4, 4);

			return (void *)&urFrame;
		}
		else { goto  FR_Analysis_err;}
	
	case HEAD_DE: 
		if(SrcLen>=FR_DE_DATA_FIX_LEN && !memcmp(SrcBuf, FR_HEAD_DE, 2)
			&& !memcmp(SrcBuf+SrcLen-4, f_tail, 4))
		{
			memcpy(deFrame.head, SrcBuf, 2);
			memcpy(deFrame.cmd, SrcBuf+2, 4);
			memcpy(deFrame.short_addr, SrcBuf+6, 4);
			
			memset(aDataBuffer, 0, sizeof(aDataBuffer));
			memcpy(aDataBuffer, SrcBuf+10, SrcLen-FR_DE_DATA_FIX_LEN);
			aDataLen = SrcLen-FR_DE_DATA_FIX_LEN;
			deFrame.data_len = aDataLen;
			deFrame.data = aDataBuffer;
			
			memcpy(deFrame.tail, SrcBuf+SrcLen-4, 4);

			return (void *)&deFrame;
		}
		else { goto  FR_Analysis_err;}

	default: goto  FR_Analysis_err;
	}

FR_Analysis_err:
	return NULL;
}


/*********************************************************************
 * @fn      SSAFrame_Package()
 *
 * @brief   packet frame
 *
 * @param none
 *
 * @return  none
 */
int8 SSAFrame_Package(frHeadType_t hType, void *data, uint8 **DstBuf, uint16 *DstLen)
{
	if(data == NULL)
		goto  FR_Package_err;
	
	switch(hType)
	{
	case HEAD_UC: 
	{
		UC_t *p_ucFrame = (UC_t *)data;
		memset(pFrameBuffer, 0, sizeof(pFrameBuffer));
		memcpy(pFrameBuffer, p_ucFrame->head, 3);
		pFrameBuffer[3] = p_ucFrame->type;
		memcpy(pFrameBuffer+4, p_ucFrame->ed_type, 2);
		memcpy(pFrameBuffer+6, p_ucFrame->short_addr, 4);
		memcpy(pFrameBuffer+10, p_ucFrame->ext_addr, 16);
		memcpy(pFrameBuffer+26, p_ucFrame->panid, 4);
		memcpy(pFrameBuffer+30, p_ucFrame->channel, 4);
		memcpy(pFrameBuffer+34, p_ucFrame->data, p_ucFrame->data_len);
		memcpy(pFrameBuffer+34+p_ucFrame->data_len, p_ucFrame->tail, 4);

		pFrameLen = FR_UC_DATA_FIX_LEN+p_ucFrame->data_len;

		*DstBuf = pFrameBuffer;
		*DstLen = pFrameLen;
	}
	break;
		
	case HEAD_UO: 
	{
		UO_t *p_uoFrame = (UO_t *)data;
		memset(pFrameBuffer, 0, sizeof(pFrameBuffer));
		memcpy(pFrameBuffer, p_uoFrame->head, 3);
		pFrameBuffer[3] = p_uoFrame->type;
		memcpy(pFrameBuffer+4, p_uoFrame->ed_type, 2);
		memcpy(pFrameBuffer+6, p_uoFrame->short_addr, 4);
		memcpy(pFrameBuffer+10, p_uoFrame->ext_addr, 16);
		memcpy(pFrameBuffer+26, p_uoFrame->data, p_uoFrame->data_len);
		memcpy(pFrameBuffer+26+p_uoFrame->data_len, p_uoFrame->tail, 4);

		pFrameLen = FR_UO_DATA_FIX_LEN+p_uoFrame->data_len;

		*DstBuf = pFrameBuffer;
		*DstLen = pFrameLen;
	}
	break;
		
	case HEAD_UH: 
	{
		UH_t *p_uhFrame = (UH_t *)data;
		memset(pFrameBuffer, 0, sizeof(pFrameBuffer));
		memcpy(pFrameBuffer, p_uhFrame->head, 3);
		memcpy(pFrameBuffer+3, p_uhFrame->short_addr, 4);
		memcpy(pFrameBuffer+7, p_uhFrame->tail, 4);

		pFrameLen = FR_UH_DATA_FIX_LEN;

		*DstBuf = pFrameBuffer;
		*DstLen = pFrameLen;
	}
	break;
		
	case HEAD_UR: 
	{
		UR_t *p_urFrame = (UR_t *)data;
		memset(pFrameBuffer, 0, sizeof(pFrameBuffer));
		memcpy(pFrameBuffer, p_urFrame->head, 3);
		pFrameBuffer[3] = p_urFrame->type;
		memcpy(pFrameBuffer+4, p_urFrame->ed_type, 2);
		memcpy(pFrameBuffer+6, p_urFrame->short_addr, 4);
		memcpy(pFrameBuffer+10, p_urFrame->data, p_urFrame->data_len);
		memcpy(pFrameBuffer+10+p_urFrame->data_len, p_urFrame->tail, 4);

		pFrameLen = FR_HR_DATA_FIX_LEN+p_urFrame->data_len;

		*DstBuf = pFrameBuffer;
		*DstLen = pFrameLen;
	}
	break;
	
	case HEAD_DE: 
	{
		DE_t *p_deFrame = (DE_t *)data;
		memset(pFrameBuffer, 0, sizeof(pFrameBuffer));
		memcpy(pFrameBuffer, p_deFrame->head, 2);
		memcpy(pFrameBuffer+3, p_deFrame->cmd, 4);
		memcpy(pFrameBuffer+6, p_deFrame->short_addr, 4);
		memcpy(pFrameBuffer+10, p_deFrame->data, p_deFrame->data_len);
		memcpy(pFrameBuffer+10+p_deFrame->data_len, p_deFrame->tail, 4);

		pFrameLen = FR_DE_DATA_FIX_LEN+p_deFrame->data_len;

		*DstBuf = pFrameBuffer;
		*DstLen = pFrameLen;
	}
	break;

	default: goto  FR_Package_err;
	}

	return 0;

FR_Package_err:
	return -1;
}

#ifdef RS485_DEV
void rs485_state(uint8 *data, uint16 length)
{
#ifdef SHT11_DEV
        //温湿度传感器
        SHT11_Finish();//获取数据并且装入结构体
        Res_Inquire();
#endif
#ifdef IOCTL_DEV
        //控制设备
        Res_Ioctrl();
#endif
#ifdef QTJC_DEV
        //气体类检测
       QT_ResInquire();
#endif
#ifdef LUX_DEV
       //光照强度计数
       LUX_ResInquire();
#endif      
}

void Res_Inquire(void)
{
  uint8 send_buf[9];
  int crcdata;
  send_buf[0]=Address_dev;//设备地址
  send_buf[1]=0x03;
  send_buf[2]=0x04;
  send_buf[3]=SHT11.Temp_byte[0];//温度值
  send_buf[4]=SHT11.Temp_byte[1];
  send_buf[5]=SHT11.Humi_byte[0];//湿度值
  send_buf[6]=SHT11.Humi_byte[1];
  crcdata=crc16(send_buf, 7);
  send_buf[7]=crcdata&0x00ff;
  send_buf[8]=crcdata>>8;
#ifdef UART_DEBUG
  Data_TxHandler(send_buf, 9);
#endif
  CommonApp_SendTheMessage(0x0000, send_buf, 9);
}

void Res_Ioctrl(void)
{
  uint8 send_buf[9];
  int crcdata;
  send_buf[0]=Address_dev;//设备地址
  send_buf[1]=0x03;
  send_buf[2]=0x04;
  send_buf[3]=Device_state[0];
  send_buf[4]=Device_state[1];
  send_buf[5]=Device_state[2];
  send_buf[6]=Device_state[3];
  crcdata=crc16(send_buf, 7);
  send_buf[7]=crcdata&0x00ff;
  send_buf[8]=crcdata>>8;
#ifdef UART_DEBUG
  Data_TxHandler(send_buf, 9);
#endif
  CommonApp_SendTheMessage(0x0000, send_buf, 9);
}

void QT_ResInquire(void)
{
  //气体检测
  uint8 send_buf[7];
  int crcdata;
  send_buf[0]=Address_dev;//设备地址
  send_buf[1]=0x03;
  send_buf[2]=0x02;
  uint16 QT_Tmp = HalAdcRead (HAL_ADC_CHANNEL_1, HAL_ADC_RESOLUTION_12);
  send_buf[3]=QT_Tmp>>8;     //高位在前
  send_buf[4]=QT_Tmp&0x00ff; //低位在后
  //矫正输出
  crcdata=crc16(send_buf, 5);
  send_buf[5]=crcdata&0x00ff;
  send_buf[6]=crcdata>>8;
#ifdef UART_DEBUG
  Data_TxHandler(send_buf, 7);
#endif
  CommonApp_SendTheMessage(0x0000, send_buf, 7);
}

void LUX_ResInquire(void)
{
  //光照查询
  uint16 LUX_value=Light();
#ifdef DE_BUG
  uint8 LUX_Buf[5];
  conversion(LUX_value,LUX_Buf);
  HalUARTWrite(0,LUX_Buf,5);
#endif
   uint8 send_buf[7];
   int crcdata;
   send_buf[0]=Address_dev;//设备地址
   send_buf[1]=0x03;
   send_buf[2]=0x02;
   send_buf[3]=LUX_value>>8;
   send_buf[4]=LUX_value&0x00ff;
   crcdata=crc16(send_buf, 5);
   send_buf[5]=crcdata&0x00ff;
   send_buf[6]=crcdata>>8;
#ifdef UART_DEBUG
   Data_TxHandler(send_buf, 7);
#endif
   CommonApp_SendTheMessage(0x0000, send_buf, 7);
}

void rs485_control(uint8 *data, uint16 length)
{
  unsigned  char buf_tmp[8];
	osal_memcpy(buf_tmp,data,length);
        int i;
        for(i=0;i<4;i++)
        {
          if(buf_tmp[2+i]==0x01)
          {
            HalGpioSet( 1<<i, HAL_GPIO_MODE_ON );
            Device_state[i]=0x01;
          }
          else
          {
            HalGpioSet( 1<<i, HAL_GPIO_MODE_OFF );
            Device_state[i]=0x00;
          }
        }
        Res_Ioctrl();
 
}
void rs485_changeAddr(uint8 *data, uint16 length)
{
        uint8 buf_tmp[8];
	osal_memcpy(buf_tmp,data,length);
   	Address_dev=buf_tmp[5];
	osal_nv_item_init(ZCD_DEV_ADDRESS, 1, NULL);
	osal_nv_write(ZCD_DEV_ADDRESS, 0, 1, &Address_dev);
        rs485_address();
}
void rs485_address(void)
{
        uint8 send_buf[6];
  	unsigned int Crc_data=0;
	send_buf[0]=Address_dev;
	send_buf[1]=0x25;
	send_buf[2]=0x01;
	send_buf[3]=0x02;
	Crc_data=crc16(send_buf, 4);
	send_buf[4]=Crc_data&0x00ff;
	send_buf[5]=Crc_data>>8;
#ifdef   UART_DEBUG     
        Data_TxHandler(send_buf, 6);
#endif
      CommonApp_SendTheMessage(0x0000, send_buf, 6); 
 
}

bool crc_confirm(uint8 SrcBuf[],uint8 SrcLen)
{
  uint8 crcbuf[2];
  unsigned int crc_tmp;
  crc_tmp=crc16(SrcBuf,SrcLen-2);
  crcbuf[0]=crc_tmp&0x00ff;//crc低8位
  crcbuf[1]=crc_tmp>>8;//crc高8位
  if((crcbuf[0]==SrcBuf[SrcLen-2])&&(crcbuf[1]==SrcBuf[SrcLen-1]))
  {
    return true;
  }
  else
    return false;
}

unsigned int crc16(uint8 buf[],uint8 len)
{
  unsigned int  i,j,c,crc;
	crc=0xFFFF;
	for(i=0;i<len;i++)
		{
		c=*(buf+i)&0xFF;
		crc^=c;
		for(j=0;j<8;j++)
			{
				if(crc & 0x0001)
					{
					crc>>=1;
					crc ^=0xA001;
					}
				else
					{
					crc >>=1;
					}
			}
		}
  return(crc);
}
#endif

