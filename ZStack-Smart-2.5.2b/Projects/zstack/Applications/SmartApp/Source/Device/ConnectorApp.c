/**************************************************************************************************
  Filename:       ConnectorApp.c
  Revised:        $Date: 2014-04-16 13:53:12 -0800 (Wed, 16 Apr 2014) $
  Revision:       $Revision: 29217 $

  Description:    Connector Application
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-08-07
**************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include "OSAL_Nv.h"
#include "ZComDef.h"
#include "OnBoard.h"

#include "CommonApp.h"

/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_uart.h"


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

/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern const cId_t CommonApp_InClusterList[];
extern const cId_t CommonApp_OutClusterList[];
extern const SimpleDescriptionFormat_t CommonApp_SimpleDesc;

extern endPointDesc_t CommonApp_epDesc;

extern byte CommonApp_TaskID;
extern devStates_t CommonApp_NwkState;
extern byte CommonApp_TransID;

extern afAddrType_t CommonApp_DstAddr;

//network address
extern uint16 nwkAddr;
//mac address
extern ZLongAddr_t macAddr;

extern uint8 SHORT_ADDR_G[4];

extern uint8 EXT_ADDR_G[16];

extern const uint8 f_tail[4];

extern bool isPermitJoining;

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 *fBuf;		//pointer data buffer
static uint16 fLen;		//buffer data length

uint8 list_len=0;               //定义list的长度

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */
#ifndef ZDO_COORDINATOR
static void ConnectorApp_HeartBeatEvent(void);
#endif

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      CommonApp_InitConfirm
 *
 * @brief   Initialization function for the Connector App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */

void CommonApp_InitConfirm( uint8 task_id )
{
  CommonApp_PermitJoiningRequest(PERMIT_JOIN_FORBID);
#ifdef RS485_DEV
  //初始化LIST,分配NV空间
  createNodelist();
#endif 
#if(HAL_UART==TRUE)
  SerialTx_Handler(SERIAL_COM_PORT, ConnectorApp_TxHandler);
#endif
}


/*********************************************************************
 * @fn      CommonApp_MessageMSGCB
 *
 * @brief   Data message processor callback.  This function processes
 *          any incoming data - probably from other devices.  So, based
 *          on cluster ID, perform the intended action.
 *
 * @param   none
 *
 * @return  none
 */

void CommonApp_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
  switch ( pkt->clusterId )
  {
    case COMMONAPP_CLUSTERID:
#ifndef RS485_DEV
      CommonApp_GetDevDataSend(pkt->cmd.Data, pkt->cmd.DataLength);
#else
      RS485_GetDevDataSend(pkt->cmd.Data, pkt->cmd.DataLength);
#endif
      break; 
  }
}


/*********************************************************************
 * @fn      CommonApp_ProcessZDOStates
 *
 * @brief   Process when network change
 *
 * @param   network status
 *
 * @return  none
 */
void CommonApp_ProcessZDOStates(devStates_t status)
{
  if(status == DEV_ZB_COORD || status == DEV_ROUTER)
  {
	nwkAddr = NLME_GetShortAddr();
	incode_2_to_16(SHORT_ADDR_G, (uint8 *)&nwkAddr, 2);
	memcpy(macAddr, NLME_GetExtAddr(), sizeof(ZLongAddr_t));
    incode_2_to_16(EXT_ADDR_G, macAddr, 8);

#ifdef ZDO_COORDINATOR
	UC_t mFrame;

	memcpy(mFrame.head, FR_HEAD_UC, 3);
	mFrame.type = FR_DEV_COORD;
	memcpy(mFrame.ed_type, FR_APP_DEV, 2);
	memcpy(mFrame.short_addr, SHORT_ADDR_G, 4);
	memcpy(mFrame.ext_addr, EXT_ADDR_G, 16);
	incode_2_to_16(mFrame.panid, (uint8 *)&_NIB.nwkPanId, 2);
	uint16 channel = _NIB.nwkLogicalChannel;
	incode_2_to_16(mFrame.channel, (uint8 *)&channel, 2);
    mFrame.data = NULL;
	mFrame.data_len = 0;
	memcpy(mFrame.tail, f_tail, 4);

	if(!SSAFrame_Package(HEAD_UC, &mFrame, &fBuf, &fLen))
	{
	  CommonApp_GetDevDataSend(fBuf, fLen);
	}
#else
	UO_t mFrame;
	
	memcpy(mFrame.head, FR_HEAD_UO, 3);
	mFrame.type = FR_DEV_ROUTER;
	memcpy(mFrame.ed_type, FR_APP_DEV, 2);
	memcpy(mFrame.short_addr, SHORT_ADDR_G, 4);
	memcpy(mFrame.ext_addr, EXT_ADDR_G, 16);
	mFrame.data = NULL;
	mFrame.data_len = 0;
	memcpy(mFrame.tail, f_tail, 4);

	if(!SSAFrame_Package(HEAD_UO, &mFrame, &fBuf, &fLen))
	{
		CommonApp_GetDevDataSend(fBuf, fLen);
		CommonApp_SendTheMessage(COORDINATOR_ADDR, fBuf, fLen);
	}

	ConnectorApp_HeartBeatEvent();
#endif
  }
}


#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
void CommonApp_HandleCombineKeys(uint16 keys, uint8 keyCounts)
{
  if(
#ifdef KEY_PUSH_PORT_1_BUTTON
    keys & HAL_KEY_PORT_1_SWITCH_3
#else
	keys & HAL_KEY_SW_6
#endif
  )
  {
	switch( keyCounts)
	{	
	case 0: //Long click listen
	//Reset factory mode
#if defined(HOLD_INIT_AUTHENTICATION)
		HalLedBlink ( HAL_LED_4, 0, 50, 100 );
#if !defined(ZDO_COORDINATOR)
		if(devState != DEV_HOLD)
		{
    		devStates_t tStates;
  			if (ZSUCCESS == osal_nv_item_init( 
  				ZCD_NV_NWK_HOLD_STARTUP, sizeof(tStates),  &tStates))
  			{
  				tStates = DEV_HOLD;
    			osal_nv_write(
					ZCD_NV_NWK_HOLD_STARTUP, 0, sizeof(tStates),  &tStates);
  			}
		}
#else
  		devState = DEV_INIT;
#endif

		zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
		WatchDogEnable( WDTIMX );
#endif
		break;

	case 3:
		//permit\forbid add znet
		if(devState == DEV_HOLD)
		{
			ZDOInitDevice( 0 );
		}
		else
		{
			if(isPermitJoining)
			{
				CommonApp_PermitJoiningRequest(PERMIT_JOIN_FORBID);
			}
			else
			{
				CommonApp_PermitJoiningRequest(PERMIT_JOIN_TIMEOUT);
			}
		}
		break;

	default: break;
	}
  }
}
#else
void CommonApp_HandleCombineKeys(uint16 keys, uint8 keyCounts)
{
  uint8 *keysID = get_keys_id();
  uint8 *keysPush = get_keys_push();

  if (keysPush[0] == HAL_KEY_LONG_PUSH)
  {
  	if(osal_memcmp(keysID, "a", keyCounts) && keyCounts == 1)
    {
		//permit\forbid add znet
		if(devState == DEV_HOLD)
		{
			ZDOInitDevice( 0 );
		}
		else
		{
			if(isPermitJoining)
			{
				CommonApp_PermitJoiningRequest(PERMIT_JOIN_FORBID);
			}
			else
			{
				CommonApp_PermitJoiningRequest(PERMIT_JOIN_TIMEOUT);
			}
		}
    }
    else if(osal_memcmp(keysID, "aaa", keyCounts) && keyCounts == 3)
    {
		//Reset factory mode
#if defined(HOLD_INIT_AUTHENTICATION)
		HalLedBlink ( HAL_LED_4, 0, 50, 100 );
#if !defined(ZDO_COORDINATOR)
		if(devState != DEV_HOLD)
		{
    		devStates_t tStates;
  			if (ZSUCCESS == osal_nv_item_init( 
  				ZCD_NV_NWK_HOLD_STARTUP, sizeof(tStates),  &tStates))
  			{
  				tStates = DEV_HOLD;
    			osal_nv_write(
					ZCD_NV_NWK_HOLD_STARTUP, 0, sizeof(tStates),  &tStates);
  			}
		}
#else
  		devState = DEV_INIT;
#endif

		zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
		WatchDogEnable( WDTIMX );
#endif
    }
  }
}
#endif

#ifndef ZDO_COORDINATOR
void ConnectorApp_HeartBeatEvent(void)
{
	CommonApp_HeartBeatCB(NULL, NULL, NULL);
	
	set_user_event(CommonApp_TaskID, HEARTBERAT_EVT, CommonApp_HeartBeatCB, 
  		HEARTBEAT_TIMEOUT, TIMER_LOOP_EXECUTION|TIMER_EVENT_RESIDENTS, NULL);
}
#endif


void ConnectorApp_TxHandler(uint8 txBuf[], uint8 txLen)
{
#ifndef  RS485_DEV
  
	uint16 Send_shortAddr = 0;

	if(txLen>=16 && !memcmp(txBuf, FR_HEAD_DE, 2)
		&& !memcmp(txBuf+2, FR_CMD_JOIN_CTRL, 4)
		&& !memcmp(txBuf+txLen-4, f_tail, 4))
	{
		if(!memcmp(txBuf+6, SHORT_ADDR_G, 4))
		{
			uint8 cmdData, ret;
			uint8 retData[2] = {0};
		
			cmdData = atox(txBuf+10, 2);

			if(!cmdData)
				ret = !CommonApp_PermitJoiningRequest(PERMIT_JOIN_FORBID);
			else if(cmdData == 1)
				ret = !CommonApp_PermitJoiningRequest(PERMIT_JOIN_TIMEOUT);

			incode_2_to_16(retData, &ret, 1);
			PermitJoin_Refresh(retData, 2);
		}
		else
		{
			incode_16_to_2(&Send_shortAddr, txBuf+6, 4);
			CommonApp_SendTheMessage(Send_shortAddr, txBuf, txLen);
		}
		
	}
	else if(txLen>=14 && !memcmp(txBuf, FR_HEAD_DE, 2)
		&& !memcmp(txBuf+2, FR_CMD_BROCAST_REFRESH, 4)
		&& !memcmp(txBuf+txLen-4, f_tail, 4))
	{
		incode_16_to_2(&Send_shortAddr, txBuf+6, 4);
		if(Send_shortAddr == COORDINATOR_ADDR)
		{
			CommonApp_ProcessZDOStates(DEV_ZB_COORD);
		}
		else
		{
			CommonApp_SendTheMessage(BROADCAST_ADDR, txBuf, txLen);
		}
	}
	else if(txLen>=14 && !memcmp(txBuf, FR_HEAD_DE, 2)
		&& (!memcmp(txBuf+2, FR_CMD_SINGLE_EXCUTE, 4)
		|| !memcmp(txBuf+2, FR_CMD_PEROID_EXCUTE, 4)
		|| !memcmp(txBuf+2, FR_CMD_PEROID_STOP, 4))
		&& !memcmp(txBuf+txLen-4, f_tail, 4))
	{
		incode_16_to_2(&Send_shortAddr, txBuf+6, 4);
		CommonApp_SendTheMessage(Send_shortAddr, txBuf, txLen);
	}
	else if(txLen>=14 && !memcmp(txBuf, FR_HEAD_DE, 2)
		&& !memcmp(txBuf+2, FR_CMD_SINGLE_REFRESH, 4)
		&& !memcmp(txBuf+txLen-4, f_tail, 4))
	{
		incode_16_to_2(&Send_shortAddr, txBuf+6, 4);
		if(Send_shortAddr == COORDINATOR_ADDR) //coord self
		{
			CommonApp_ProcessZDOStates(DEV_ZB_COORD);
		}
		else
		{
			CommonApp_SendTheMessage(Send_shortAddr, txBuf, txLen);
		}
	}
#else
      CommonApp_RS485SendMessage(txBuf, txLen);
#endif
}


/*********************************************************************
 */
void CommonApp_RS485SendMessage(uint8 *data, uint8 length)
{
   uint16 dest_addr;
   uint8 dev_addr=data[0];
  if(data[0]==0xFA)
  {
    CommonApp_SendTheMessage(BROADCAST_ADDR, data,length);
  }
  else
  {
    //get the short_addr
   dest_addr=get_NodeList(dev_addr);
   if(dest_addr)
   CommonApp_SendTheMessage(dest_addr, data,length);
  }
}
