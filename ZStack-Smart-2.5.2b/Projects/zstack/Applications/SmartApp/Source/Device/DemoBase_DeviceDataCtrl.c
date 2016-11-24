/**************************************************************************************************
  Filename:       DemoBase_DeviceDataCtrl.c
  Revised:        $Date: 2014-12-01 14:27:34 -0800 (Thu, 01 dec 2014) $
  Revision:       $Revision: 29218 $

  Description:    This file contains interface of demo base device data operations
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-07-31
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CommonApp.h"
#include "hal_drivers.h" 
#include "i2c-ctrl.h"
#include "ht-ctrl.h"
#include "rfid/RC522_RegCtrl.h"
#include "rfid/RC522_Mifare.h"
#include "rfid/RC522_ErrCode.h"
#include "rfid/Mifare_Util.h"
#include "mincode.h"

/*********************************************************************
 * MACROS
 */
#define DEMOBASE_DEVICE_IIC_ADDR     ((1<<8)+1)	//IIC存储ID 地址(可自定义)
#define DEMOBASE_SETID_HEAD		"SETID"
#define DEMOBASE_GETID_HEAD		"GETID"

#define LED_DATA_SIZE			4
#define PLC_DATA_SIZE			6
#define RFID_DATA_SIZE			19
#define HUMITURE_DATA_SIZE 		7
#define LIGHTDETECT_DATA_SIZE 	5
#define AIRDETECT_DATA_SIZE 	5
#define LIGHTCTRL_DATA_SIZE		4

#define DEMOBASE_DEVICE_LED				"01"
#define DEMOBASE_DEVICE_PLC				"02"
#define DEMOBASE_DEVICE_RFID			"03"
#define DEMOBASE_DEVICE_HUMITURE		"04"
#define DEMOBASE_DEVICE_LIGHTDETECT		"05"
#define DEMOBASE_DEVICE_AIRDETECT		"06"
#define DEMOBASE_DEVICE_LIGHTCTRL		"07"

#define DEMOBASE_DEVICE_NOTIDENTIFY		"FF"

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern byte CommonApp_TaskID;

extern uint8 *optData;
extern uint8 optDataLen;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
 
/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 queryStates = 0;
static uint8 isBee = 0;
static uint8 ids[8] = {0};
static uint8 idlen = 0;

static uint8 temp;
static uint8 humi;

static uint8 lightGrade = 2;

static uint8 airStates = 1;

static uint8 lightCtrlStates = 0;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void led_init(void);
static void plc_init(void);
static void rfid_init(void);
static void humiture_init(void);
static void lightdetect_init(void);
static void airdetect_init(void);
static void lightctrl_init(void);

static int8 set_led_data(uint8 *data, uint8 dataLen);
static int8 set_plc_data(uint8 *data, uint8 dataLen);
static int8 set_rfid_data(uint8 *data, uint8 dataLen);
static int8 set_humiture_data(uint8 *data, uint8 dataLen);
static int8 set_lightdetect_data(uint8 *data, uint8 dataLen);
static int8 set_airdetect_data(uint8 *data, uint8 dataLen);
static int8 set_lightctrl_data(uint8 *data, uint8 dataLen);

static int8 get_led_data(uint8 *data, uint8 *dataLen);
static int8 get_plc_data(uint8 *data, uint8 *dataLen);
static int8 get_rfid_data(uint8 *data, uint8 *dataLen);
static int8 get_humiture_data(uint8 *data, uint8 *dataLen);
static int8 get_lightdetect_data(uint8 *data, uint8 *dataLen);
static int8 get_airdetect_data(uint8 *data, uint8 *dataLen);
static int8 get_lightctrl_data(uint8 *data, uint8 *dataLen);

static void DemoBaseRFID_QueryCB( void *params, uint16 *duration, uint8 *count);
static void DemoBaseRFID_StopBeeCB( void *params, uint16 *duration, uint8 *count);
static void DemoBaseHumiture_StatusCB( void *params, uint16 *duration, uint8 *count);
static void DemoBaseLightDetect_StatusCB( void *params, uint16 *duration, uint8 *count);
static void DemoBaseAirDetect_StatusCB( void *params, uint16 *duration, uint8 *count);
static uint16 getAD(void);
static void InitTimer1(void);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

void HalDeviceInit (void)
{

}

void HalStatesInit(devStates_t status)
{
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);
	switch(i2cAddr)
	{
	case 1: led_init(); break;
	case 2: plc_init(); break;
	case 3: rfid_init(); break;
	case 4: humiture_init(); break;
	case 5: lightdetect_init(); break;
	case 6: airdetect_init(); break;
	case 7: lightctrl_init(); break;
	}
}

int8 set_device_data(uint8 const *data, uint8 dataLen)
{
	uint8 i2cAddr;
	
	if(dataLen >= 7 && osal_memcmp(optData, DEMOBASE_SETID_HEAD, 5))
	{
		i2cAddr = atox((uint8 *)(data+5), 2);
		if(ISendByte(DEMOBASE_DEVICE_IIC_ADDR, i2cAddr) > 0)
		{
			osal_memcpy(optData, DEMOBASE_GETID_HEAD, 5);
			i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);
			incode_xtocs(optData+5, &i2cAddr, 1);
		}
		else
		{
			osal_memcpy(optData+5, "ER", 2);
		}
		
		return 0;
	}
	
	i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);
	switch(i2cAddr)
	{
	case 1: return set_led_data((uint8 *)data, dataLen);
	case 2: return set_plc_data((uint8 *)data, dataLen);
	case 3: return set_rfid_data((uint8 *)data, dataLen);
	case 4: return set_humiture_data((uint8 *)data, dataLen);
	case 5: return set_lightdetect_data((uint8 *)data, dataLen);
	case 6: return set_airdetect_data((uint8 *)data, dataLen);
	case 7: return set_lightctrl_data((uint8 *)data, dataLen);
	}
	
	return 0;
}


int8 get_device_data(uint8 *data, uint8 *dataLen)
{
	if(osal_memcmp(optData, DEMOBASE_SETID_HEAD, 5)
		|| osal_memcmp(optData, DEMOBASE_GETID_HEAD, 5))
	{
		osal_memcpy(data, optData, 7);
		*dataLen = 7;
		
		return 0;
	}
	
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);
	switch(i2cAddr)
	{
	case 1: return get_led_data(data, dataLen);
	case 2: return get_plc_data(data, dataLen);
	case 3: return get_rfid_data(data, dataLen);
	case 4: return get_humiture_data(data, dataLen);
	case 5: return get_lightdetect_data(data, dataLen);
	case 6: return get_airdetect_data(data, dataLen);
	case 7: return get_lightctrl_data(data, dataLen);
	}
	
	return 0;
}

void led_init(void)
{
	HAL_INIT_BASELED();
}

void plc_init(void)
{
	HAL_INIT_PLCIN1();
	HAL_INIT_PLCIN2();
	HAL_INIT_PLCOUT1();
	HAL_INIT_PLCOUT2();
}

void rfid_init(void)
{
	HAL_INIT_RFIDBEE();

	update_user_event(CommonApp_TaskID, 
							DEMOBASE_RFID_QUERY_EVT, 
							DemoBaseRFID_QueryCB, 
	  						500, 
	  						TIMER_LOOP_EXECUTION|TIMER_EVENT_RESIDENTS, 
	  						NULL);

}

void DemoBaseRFID_QueryCB( void *params, uint16 *duration, uint8 *count)
{
	unsigned char baATQ[2],bSAK;
	//enum MifareICType eMifareType;
	short status;

	Rc522RFReset(5);  //RF - Reset 
	status = ActivateCard(ISO14443_3_REQA, baATQ, ids, &idlen, &bSAK);
	if(status !=  STATUS_SUCCESS || idlen > RFID_DATA_SIZE-3)
	{
		idlen = 0;
		isBee = 0;
		return;
	}

	if(!isBee)
	{
		isBee = 1;

		if(queryStates)
		{
			if(optData != NULL && optDataLen < RFID_DATA_SIZE)
			{
				osal_mem_free(optData);
				optData = NULL;
			}

			if(optData == NULL)
			{
				optData = osal_mem_alloc(RFID_DATA_SIZE);
				optDataLen = RFID_DATA_SIZE;
			}

			osal_memcpy(optData, DEMOBASE_DEVICE_RFID, 2);
			optData[2] = 'S';
			incode_xtocs(optData+3, ids, idlen);
			
			Update_Refresh(optData, (idlen*2)+3);
		}
		
		HAL_TURN_ON_RFIDBEE();
		update_user_event(CommonApp_TaskID, 
								DEMOBASE_RFID_BEE_EVT, 
								DemoBaseRFID_StopBeeCB, 
	  							500, 
	  							TIMER_ONE_EXECUTION|TIMER_EVENT_RESIDENTS, 
	  							NULL);
	}
}

void DemoBaseRFID_StopBeeCB( void *params, uint16 *duration, uint8 *count)
{
	HAL_TURN_OFF_RFIDBEE();
}

void humiture_init(void)
{
	update_user_event(CommonApp_TaskID, 
  					DEMOBASE_HUMITURE_EVT, 
  					DemoBaseHumiture_StatusCB, 
  					2000, 
  					TIMER_LOOP_EXECUTION|TIMER_EVENT_RESIDENTS, 
  					NULL);
}

void DemoBaseHumiture_StatusCB( void *params, uint16 *duration, uint8 *count)
{
	HTValue humi_val, temp_val;
	uint8 error, checksum;

	s_connectionreset();

	error = 0;
	error += s_measure((uint8*) &humi_val.i, &checksum, HUMI);  //measure humidity
	error += s_measure((uint8*) &temp_val.i, &checksum, TEMP);  //measure temperature
	if(error != 0) 
	{	
		s_connectionreset();                 //in case of an error: connection reset
		return;
	}

	humi_val.f = (float)humi_val.i;                   //converts integer to float
	temp_val.f = (float)temp_val.i;                   //converts integer to float
	calc_sht11(&humi_val.f, &temp_val.f);            //calculate humidity, temperature

	if(temp != (uint8)temp_val.f 
		|| humi != (uint8)humi_val.f)
	{
		temp = (uint8)temp_val.f;
		humi = (uint8)humi_val.f;
		
		if(queryStates)
		{
			if(optData != NULL && optDataLen < HUMITURE_DATA_SIZE)
			{
				osal_mem_free(optData);
				optData = NULL;
				optDataLen = 0;
			}

			if(optData == NULL)
			{
				optData = osal_mem_alloc(HUMITURE_DATA_SIZE);
				optDataLen = HUMITURE_DATA_SIZE;
			}

			osal_memcpy(optData, DEMOBASE_DEVICE_HUMITURE, 2);
			optData[2] = 'S';
			incode_xtocs(optData+3, &temp, 1);
			incode_xtocs(optData+5, &humi, 1);
			
			Update_Refresh(optData, HUMITURE_DATA_SIZE);
		}
	}
}

void lightdetect_init(void)
{
	update_user_event(CommonApp_TaskID, 
  					DEMOBASE_LIGHTDETECT_EVT, 
  					DemoBaseLightDetect_StatusCB, 
  					2000, 
  					TIMER_LOOP_EXECUTION|TIMER_EVENT_RESIDENTS, 
  					NULL);
}

void DemoBaseLightDetect_StatusCB( void *params, uint16 *duration, uint8 *count)
{
	uint16 avgAD, i;
	uint8 currentGrade;
	
	for(i=0; i<64; i++)
	{
		avgAD += getAD();
		avgAD = avgAD/2;
	}
	avgAD &= 0x0FFF;

	if(avgAD > 962)	/* 弱光 */
	{
		currentGrade = 1;
	}
	else if(avgAD > 200) /* 正常*/
	{
		currentGrade = 2;
	}
	else /* 强光*/
	{
		currentGrade = 3;
	}
	
	if( lightGrade != currentGrade )
	{
		lightGrade = currentGrade;
		
		if(queryStates)
		{
			if(optData != NULL && optDataLen < LIGHTDETECT_DATA_SIZE)
			{
				osal_mem_free(optData);
				optData = NULL;
				optDataLen = 0;
			}

			if(optData == NULL)
			{
				optData = osal_mem_alloc(LIGHTDETECT_DATA_SIZE);
				optDataLen = LIGHTDETECT_DATA_SIZE;
			}

			osal_memcpy(optData, DEMOBASE_DEVICE_LIGHTDETECT, 2);
			optData[2] = 'S';
			incode_xtocs(optData+3, &lightGrade, 1);
			
			Update_Refresh(optData, LIGHTDETECT_DATA_SIZE);
		}
	}
}

uint16 getAD(void)
{
  uint16 value;
  ADCCON3 = 0xB1;
  
  while(!(ADCCON1 & 0x80));
  value = ADCL>>4;
  value |= (((uint16)ADCH)<<4);
  
  return value;
}

void airdetect_init(void)
{
	HAL_INIT_AIRDETECT();
	
	update_user_event(CommonApp_TaskID, 
  					DEMOBASE_AIRETECT_EVT, 
  					DemoBaseAirDetect_StatusCB, 
  					2000, 
  					TIMER_LOOP_EXECUTION|TIMER_EVENT_RESIDENTS, 
  					NULL);
}

void DemoBaseAirDetect_StatusCB( void *params, uint16 *duration, uint8 *count)
{
	uint8 currentStates = HAL_STATE_AIRDETECT();
	
	if( airStates != currentStates )
	{
		airStates = currentStates;
		
		if(queryStates)
		{
			if(optData != NULL && optDataLen < AIRDETECT_DATA_SIZE)
			{
				osal_mem_free(optData);
				optData = NULL;
				optDataLen = 0;
			}

			if(optData == NULL)
			{
				optData = osal_mem_alloc(AIRDETECT_DATA_SIZE);
				optDataLen = AIRDETECT_DATA_SIZE;
			}

			osal_memcpy(optData, DEMOBASE_DEVICE_AIRDETECT, 2);
			optData[2] = 'S';
			incode_xtocs(optData+3, &airStates, 1);
			
			Update_Refresh(optData, AIRDETECT_DATA_SIZE);
		}
	}
}

void lightctrl_init(void)
{
  	volatile int i;
	InitTimer1();
	T1CTL = 0x06;
  
    for(i = 32;i > 0;i--)
      asm("NOP");
	
	T1CTL = 0x04;
}

int8 set_led_data(uint8 *data, uint8 dataLen)
{
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);

	if(i2cAddr != incode_atox(data, 2) || dataLen < 4)
	{
		return 0;
	}
	
	HAL_SET_BASELED(incode_atox(data+2, 2));
	return 0;
}

int8 set_plc_data(uint8 *data, uint8 dataLen)
{
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);

	if(i2cAddr != incode_atox(data, 2) || dataLen < 4)
	{
		return 0;
	}
	
	if(data[2] == '0')
	{
		HAL_TURN_OFF_PLCOUT1();
	}
	else
	{
		HAL_TURN_ON_PLCOUT1();
	}

	if(data[3] == '0')
	{
		HAL_TURN_OFF_PLCOUT2();
	}
	else
	{
		HAL_TURN_ON_PLCOUT2();
	}
	
	return 0;
}

int8 set_rfid_data(uint8 *data, uint8 dataLen)
{
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);

	if(i2cAddr != incode_atox(data, 2) || dataLen < 3)
	{
		return 0;
	}

	if(data[2] == 'S')
	{
		queryStates = 1;
	}
	else if(data[2] == 'P')
	{
		queryStates = 0;
	}
	
	return 0;
}

int8 set_humiture_data(uint8 *data, uint8 dataLen)
{
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);

	if(i2cAddr != incode_atox(data, 2) || dataLen < 3)
	{
		return 0;
	}

	if(data[2] == 'S')
	{
		queryStates = 1;
	}
	else if(data[2] == 'P')
	{
		queryStates = 0;
	}
	
	return 0;
}

int8 set_lightdetect_data(uint8 *data, uint8 dataLen)
{
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);

	if(i2cAddr != incode_atox(data, 2) || dataLen < 3)
	{
		return 0;
	}

	if(data[2] == 'S')
	{
		queryStates = 1;
	}
	else if(data[2] == 'P')
	{
		queryStates = 0;
	}
	
	return 0;
}

int8 set_airdetect_data(uint8 *data, uint8 dataLen)
{
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);

	if(i2cAddr != incode_atox(data, 2) || dataLen < 3)
	{
		return 0;
	}

	if(data[2] == 'S')
	{
		queryStates = 1;
	}
	else if(data[2] == 'P')
	{
		queryStates = 0;
	}
	
	return 0;
}

int8 set_lightctrl_data(uint8 *data, uint8 dataLen)
{
	uint8 i2cAddr = IRecvByte(DEMOBASE_DEVICE_IIC_ADDR);

	if(i2cAddr != incode_atox(data, 2) || dataLen < 4)
	{
		return 0;
	}
	
	lightCtrlStates = incode_atox(data+2, 2);

	InitTimer1();

	T1CC1L = lightCtrlStates*12+4;
	T1CC1H = 0;
	
	if(lightCtrlStates != 0)
	{
		T1CTL = 0x06;
	}
	else
	{
		T1CTL = 0x04;
	}
	
	return 0;
}

void InitTimer1(void)
{
  P1SEL |= 0x02;
  P1DIR |= 0x02;
  PERCFG |= 0x40; //alt2
  CLKCONCMD |= 0x28;  //CLKCONCMD.TICKSPD=110, 1MHz
  /* 设置溢出值 */
  T1CC0H=0x00;
  T1CC0L=0xFF;
  T1CC1H = 0x00;
  T1CC1L = 5;//定时器1通道0捕获/比较值低位
  T1CCTL1 |= 0x1C;//在向上比较清除输出,比较模式 PWM
  //T1CCTL1  = 0x14; // 输出切换 50%占空比

  T1CTL    = 0x04 ;//32分频 模模式
  
  T1IE = 0;     //timer1 中断屏蔽
  EA = 1;     //IEN0.EA=1,使能全局中断
}
int8 get_led_data(uint8 *data, uint8 *dataLen)
{
	if(optData != NULL && optDataLen < LED_DATA_SIZE)
	{
		osal_mem_free(optData);
		optData = NULL;
	}

	if(optData == NULL)
	{
		optData = osal_mem_alloc(LED_DATA_SIZE);
		optDataLen = LED_DATA_SIZE;
	}

	osal_memcpy(optData, DEMOBASE_DEVICE_LED, 2);
	uint8 portVal = P1;
	incode_xtocs(optData+2, &portVal, 1);

	osal_memcpy(data, optData, optDataLen);
	*dataLen = optDataLen;

	return 0;
}

int8 get_plc_data(uint8 *data, uint8 *dataLen)
{
	if(optData != NULL && optDataLen < PLC_DATA_SIZE)
	{
		osal_mem_free(optData);
		optData = NULL;
	}

	if(optData == NULL)
	{
		optData = osal_mem_alloc(PLC_DATA_SIZE);
		optDataLen = PLC_DATA_SIZE;
	}

	osal_memcpy(optData, DEMOBASE_DEVICE_PLC, 2);

	if(HAL_STATE_PLCIN1())
	{
		optData[2] = '1';
	}
	else
	{
		optData[2] = '0';
	}

	if(HAL_STATE_PLCIN2())
	{
		optData[3] = '1';
	}
	else
	{
		optData[3] = '0';
	}

	if(HAL_STATE_PLCOUT1())
	{
		optData[4] = '1';
	}
	else
	{
		optData[4] = '0';
	}

	if(HAL_STATE_PLCOUT2())
	{
		optData[5] = '1';
	}
	else
	{
		optData[5] = '0';
	}

	osal_memcpy(data, optData, optDataLen);
	*dataLen = optDataLen;
	
	return 0;
}

int8 get_rfid_data(uint8 *data, uint8 *dataLen)
{
	if(optData != NULL && optDataLen < RFID_DATA_SIZE)
	{
		osal_mem_free(optData);
		optData = NULL;
	}

	if(optData == NULL)
	{
		optData = osal_mem_alloc(RFID_DATA_SIZE);
		optDataLen = RFID_DATA_SIZE;
	}

	osal_memcpy(optData, DEMOBASE_DEVICE_RFID, 2);

	if(queryStates)
	{
		optData[2] = 'S';
		incode_xtocs(optData+3, ids, idlen);
		
		osal_memcpy(data, optData, (idlen*2)+3);
		*dataLen = (idlen*2)+3;
	}
	else
	{
		optData[2] = 'P';
		osal_memcpy(data, optData, 3);
		*dataLen = 3;
	}

	return 0;
}

int8 get_humiture_data(uint8 *data, uint8 *dataLen)
{
	if(optData != NULL && optDataLen < HUMITURE_DATA_SIZE)
	{
		osal_mem_free(optData);
		optData = NULL;
		optDataLen = 0;
	}

	if(optData == NULL)
	{
		optData = osal_mem_alloc(HUMITURE_DATA_SIZE);
		optDataLen = HUMITURE_DATA_SIZE;
	}

	osal_memcpy(optData, DEMOBASE_DEVICE_HUMITURE, 2);

	if(queryStates)
	{
		optData[2] = 'S';
		incode_xtocs(optData+3, &temp, 1);
		incode_xtocs(optData+5, &humi, 1);

		osal_memcpy(data, optData, HUMITURE_DATA_SIZE);
		*dataLen = HUMITURE_DATA_SIZE;
	}
	else
	{
		optData[2] = 'P';
		
		osal_memcpy(data, optData, 3);
		*dataLen = 3;
	}
	
	return 0;
}

int8 get_lightdetect_data(uint8 *data, uint8 *dataLen)
{
	if(optData != NULL && optDataLen < LIGHTDETECT_DATA_SIZE)
	{
		osal_mem_free(optData);
		optData = NULL;
		optDataLen = 0;
	}

	if(optData == NULL)
	{
		optData = osal_mem_alloc(LIGHTDETECT_DATA_SIZE);
		optDataLen = LIGHTDETECT_DATA_SIZE;
	}

	osal_memcpy(optData, DEMOBASE_DEVICE_LIGHTDETECT, 2);

	if(queryStates)
	{
		optData[2] = 'S';
		incode_xtocs(optData+3, &lightGrade, 1);

		osal_memcpy(data, optData, LIGHTDETECT_DATA_SIZE);
		*dataLen = LIGHTDETECT_DATA_SIZE;
	}
	else
	{
		optData[2] = 'P';
		
		osal_memcpy(data, optData, 3);
		*dataLen = 3;
	}
	
	return 0;
}

int8 get_airdetect_data(uint8 *data, uint8 *dataLen)
{
	if(optData != NULL && optDataLen < AIRDETECT_DATA_SIZE)
	{
		osal_mem_free(optData);
		optData = NULL;
		optDataLen = 0;
	}

	if(optData == NULL)
	{
		optData = osal_mem_alloc(AIRDETECT_DATA_SIZE);
		optDataLen = AIRDETECT_DATA_SIZE;
	}

	osal_memcpy(optData, DEMOBASE_DEVICE_AIRDETECT, 2);

	if(queryStates)
	{
		optData[2] = 'S';
		incode_xtocs(optData+3, &airStates, 1);

		osal_memcpy(data, optData, AIRDETECT_DATA_SIZE);
		*dataLen = AIRDETECT_DATA_SIZE;
	}
	else
	{
		optData[2] = 'P';
		
		osal_memcpy(data, optData, 3);
		*dataLen = 3;
	}
	
	return 0;
}

int8 get_lightctrl_data(uint8 *data, uint8 *dataLen)
{
	if(optData != NULL && optDataLen < LIGHTCTRL_DATA_SIZE)
	{
		osal_mem_free(optData);
		optData = NULL;
	}

	if(optData == NULL)
	{
		optData = osal_mem_alloc(LIGHTCTRL_DATA_SIZE);
		optDataLen = LIGHTCTRL_DATA_SIZE;
	}

	osal_memcpy(optData, DEMOBASE_DEVICE_LIGHTCTRL, 2);
	incode_xtocs(optData+2, &lightCtrlStates, 1);

	osal_memcpy(data, optData, LIGHTCTRL_DATA_SIZE);
	*dataLen = LIGHTCTRL_DATA_SIZE;

	return 0;
}
