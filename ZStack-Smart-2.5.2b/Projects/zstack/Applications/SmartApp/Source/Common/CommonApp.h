/**************************************************************************************************
  Filename:       CommonApp.h
  Revised:        $Date: 2014-04-16 14:41:24 -0800 (Wed, 16 Apr 2014) $
  Revision:       $Revision: 29217 $

  Description:    This file contains the Common Application definitions.
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-08-27
**************************************************************************************************/


#ifndef COMMONAPP_H
#define COMMONAPP_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"

#include "api_events.h"
#include "HeartBeat.h"
#include "incode.h"

#include "serial-comm.h"
#include "frame-analysis.h"

#include "bh1750.h"
#include "sht10.h"
#include "list.h"

/*********************************************************************
 * CONSTANTS
 */

/*通用设备描述信息*/
#define COMMONAPP_ENDPOINT			 0x10
#define COMMONAPP_PROFID             0x0F08
#define COMMONAPP_DEVICEID           0x0001
#define COMMONAPP_DEVICE_VERSION     1
#define COMMONAPP_FLAGS              0

#define COMMONAPP_CLUSTERID          1
#define COMMONAPP_MAX_INCLUSTERS     1
#define COMMONAPP_MAX_OUTCLUSTERS    1

#define COORDINATOR_ADDR		0x0000
#define BROADCAST_ADDR		0xFFFF

/*********************************************************************
 * MACROS
 */
#define RS485_HEART_EVT                 0x0008
//User events message
#define SERIAL_CMD_EVT				0x4000		//serial receive event by user defined
#define TRANSNODE_UPLOAD_EVT		0x2000
#define HEARTBERAT_EVT				0x0001		//heart beat event by user defined
#define CMD_PEROID_EVT				0x0002		//cmd period control event
#define PERMIT_JOIN_EVT				0x0004
#define POWERSETTING_COUNT_EVT		0x0008
#define DOORSENSOR_ISR_EVT			0x0010
#define DOORSENSOR_DETECT_EVT		0x0020
#define IRDETECT_ISR_EVT			0x0010
#define IRDETECT_DETECT_EVT			0x0020
#define IRDETECT_QUERY_EVT			0x0040
#define AIRCONTROLLER_QUERY_EVT		0x0080
#define HUMITURE_DETECT_EVT			0x0010
#define DEMOBASE_RFID_QUERY_EVT		0x0010
#define DEMOBASE_RFID_BEE_EVT		0x0020
#define DEMOBASE_HUMITURE_EVT		0x0040
#define DEMOBASE_LIGHTDETECT_EVT	0x0080
#define DEMOBASE_AIRETECT_EVT		0x0100
#define HUELIGHT_COUNT_EVT			0x0010

#define IRRELAY_LEARN_CMD	0x88
#define IRRELAY_SEND_CMD	0x86

//event peroid
  
#define RS485_SEND_realtime_TIMEOUT   30000     // Every 30 seconds
#ifndef POWER_SAVING
#define HEARTBEAT_TIMEOUT   	30000	//Every 30 Seconds
#define TRANSNODE_TIMEOUT   	30000	//Every 30 Seconds
#define CMD_PEROID_TIMEOUT		5000	//Every 5 Seconds
#define DOORSENSOR_TIMEOUT  	5000
#define IRDETECT_TIMEOUT		10000
#define IRDETECT_QUERY_TIMEOUT	500
#define AIRCONTROLLER_TIMEOUT	5000
#define PERMIT_JOIN_TIMEOUT		30		//30 Seconds, <=255s
#define HUMITUREDETECT_TIMEOUT  5000
#else
#define HEARTBEAT_TIMEOUT   	6000	//Every 36 Seconds
#define TRANSNODE_TIMEOUT   	6000	//Every 36 Seconds
#define CMD_PEROID_TIMEOUT		1000	//Every 6 Seconds
#define DOORSENSOR_TIMEOUT  	1000	//Every 6 Seconds
#define HUMITUREDETECT_TIMEOUT  3000

#define PERMIT_JOIN_TIMEOUT	6		//36 Seconds
#endif

#define POWERSETTING_TIMEOUT	3000
#define HUELIGHT_TIMEOUT		1000

#define PERMIT_JOIN_FORBID	0
#define PERMIT_JOIN_ALWAYS	0xFF

// Device Info Constants
#define ZB_INFO_DEV_STATE                 0
#define ZB_INFO_IEEE_ADDR                 1
#define ZB_INFO_SHORT_ADDR                2
#define ZB_INFO_PARENT_SHORT_ADDR         3
#define ZB_INFO_PARENT_IEEE_ADDR          4
#define ZB_INFO_CHANNEL                   5
#define ZB_INFO_PAN_ID                    6
#define ZB_INFO_EXT_PAN_ID                7

#if (DEVICE_TYPE_ID==0xF0 || DEVICE_TYPE_ID==14)
#define AIRCONTROLLER_IR_SEND_MED			"SEN"
#define AIRCONTROLLER_IR_LEARN_MED			"LEA"
#define AIRCONTROLLER_PM25_READVAL_MED		"REV"
#define AIRCONTROLLER_PM25_READMODE_MED		"REM"
#define AIRCONTROLLER_PM25_READHOLD_MED		"REH"
#define AIRCONTROLLER_PM25_SETMODE_MED		"WRM"
#define AIRCONTROLLER_PM25_SETHOLD_MED		"WRH"
#define AIRCONTROLLER_GET_DATA_MED			"GDT"
#define AIRCONTROLLER_OVER_THRESHOLD_MED	"OTD"

#define AIRCONTROL_PM25_THRESMODE_UP		1
#define AIRCONTROL_PM25_THRESMODE_DOWN		2
#define AIRCONTROL_PM25_THRESMODE_UNABLE	3

#define AIRCONTROL_PM25_DEFAULT_TRESHOLD	115
#endif

#define EXT_ADDR_SIZE		16

/*********************************************************************
 * TYPEDEFS
 */
typedef struct DATA_CMD
{
   uint8 Head; //52 
   uint8 CMD;//80
   uint8 PM25[2]; //
   uint8 PM10[2]; //
   uint8 data[2];//保留
   uint8 Check_sum;//
   uint8 Tail;//53
}DATA_CMD_T;

#if (DEVICE_TYPE_ID==0xF0 || DEVICE_TYPE_ID==14)
typedef enum
{
	AIRCONTROL_IR_SEND,
	AIRCONTROL_IR_LEARN,
	AIRCONTROL_PM25_READVAL,
	AIRCONTROL_PM25_READMODE,
	AIRCONTROL_PM25_READHOLD,
	AIRCONTROL_PM25_SETMODE,
	AIRCONTROL_PM25_SETHOLD,
	AIRCONTROL_GET_DATA,
	AIRCONTROL_NONE,
}AirController_Method_t;

typedef struct AirController_Opt
{
	uint8 PM25_thresmode;	//0, unable; 1, up; 2, down;
	uint8 PM25_threstrigger;
	uint16 PM25_threshold;
	uint16 PM25_val;
}AirController_Opt_t;

typedef int8(*PM25_Threshold_CallBack)(void);
#endif

/*********************************************************************
 * FUNCTIONS
 */
#if(DEVICE_TYPE_ID!=0)
extern void HalStatesInit(devStates_t status);
#endif

#if (DEVICE_TYPE_ID==0xF0 || DEVICE_TYPE_ID==14)
extern void SetPM25Val(uint16 val);
extern uint16 GetPM25Val(void);
extern void SetPM25ThresCallBack(uint8 mode, 
				uint16 threshold, PM25_Threshold_CallBack func);
extern void PM25_Threshold_Handler(void);
extern void SetThresHold(uint8 mode, uint16 hold);
#endif

/*
 * Task Initialization for the Common Application
 */
extern void CommonApp_Init( byte task_id );

/*
 * Task Event Processor for the Common Application
 */
extern UINT16 CommonApp_ProcessEvent( byte task_id, UINT16 events );

/*
 * Task Send the Message for the Common Application
 */
extern void CommonApp_SendTheMessage(uint16 dstNwkAddr, uint8 *data, uint8 length);

/*
 * Task Initialization for the Common Application Confirm
 */
extern void CommonApp_InitConfirm( byte task_id );
extern void CommonApp_MessageMSGCB( afIncomingMSGPacket_t *pckt );
extern void CommonApp_ProcessZDOStates(devStates_t status);
extern void CommonApp_HandleCombineKeys(uint16 keys, uint8 keyCounts);
#ifdef KEY_PUSH_PORT_1_BUTTON
extern void DeviceCtrl_HandlePort1Keys(uint16 keys, uint8 keyCounts);
#endif

#if(DEVICE_TYPE_ID!=0)
extern int8 set_device_data(uint8 const *data, uint8 dataLen);
extern int8 get_device_data(uint8 *data, uint8 *dataLen);

extern int8 CommonDevice_SetData(uint8 const *data, uint8 dataLen);
extern int8 CommonDevice_GetData(uint8 *data, uint8 *dataLen);

extern void CommonApp_CmdPeroidCB( void *params, uint16 *duration, uint8 *count);
extern int8 DataCmd_Ctrl(uint8 *data, uint8 length);
#endif
extern void Update_Refresh(uint8 *data, uint8 length);
extern void PermitJoin_Refresh(uint8 *data, uint8 length);
ZStatus_t CommonApp_PermitJoiningRequest( byte PermitDuration );


/*********************************************************************
 * IMPLEMENTS
 */
extern void CommonApp_GetDeviceInfo ( uint8 param, void *pValue );

#ifdef SSA_CONNECTOR
extern void ConnectorApp_TxHandler(uint8 txBuf[], uint8 txLen);
#endif

extern uint8 list_len;
extern void CommonApp_GetDevDataSend(uint8 *buf, uint16 len);
extern void RS485_GetDevDataSend(uint8 *buf, uint16 len);
extern void CommonApp_RS485SendMessage(uint8 *data, uint8 length);

extern uint16 nwkAddr;
extern uint8 nwkAddr_buf[3];
extern uint8 value_addr_L;
extern uint8 value_addr_H;

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* COMMONAPP_H */

