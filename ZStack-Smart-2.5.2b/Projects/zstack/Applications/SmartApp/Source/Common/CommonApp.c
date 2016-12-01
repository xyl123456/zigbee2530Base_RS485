/**************************************************************************************************
  Filename:       CommonApp.c
  Revised:        $Date: 2014-04-16 16:00:27 -0800 (Wed, 16 Apr 2014) $
  Revision:       $Revision: 29217 $

  Description:    Common Application
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-07-31
**************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "CommonApp.h"

#include "OSAL.h"
#include "OSAL_Nv.h"
#include "OnBoard.h"

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
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// This list should be filled with Application specific Cluster IDs.
//通用cluster list
const cId_t CommonApp_InClusterList[COMMONAPP_MAX_INCLUSTERS] =
{
  COMMONAPP_CLUSTERID,
};

const cId_t CommonApp_OutClusterList[COMMONAPP_MAX_OUTCLUSTERS] =
{
  COMMONAPP_CLUSTERID,
};

const SimpleDescriptionFormat_t CommonApp_SimpleDesc =
{
  COMMONAPP_ENDPOINT,              //  int Endpoint;
  COMMONAPP_PROFID,                //  uint16 AppProfId[2];
  COMMONAPP_DEVICEID,              //  uint16 AppDeviceId[2];
  COMMONAPP_DEVICE_VERSION,        //  int   AppDevVer:4;
  COMMONAPP_FLAGS,                 //  int   AppFlags:4;
  COMMONAPP_MAX_INCLUSTERS,          //  byte  AppNumInClusters;
  (cId_t *)CommonApp_InClusterList,  //  byte *pAppInClusterList;
  COMMONAPP_MAX_OUTCLUSTERS,          //  byte  AppNumOutClusters;
  (cId_t *)CommonApp_OutClusterList   //  byte *pAppOutClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in CommonApp_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.
endPointDesc_t CommonApp_epDesc;


byte CommonApp_TaskID;   // Task ID for internal task/event processing
                          // This variable will be received when
                          // CommonApp_Init() is called.
devStates_t CommonApp_NwkState;
byte CommonApp_TransID;  // This is the unique message ID (counter)

afAddrType_t CommonApp_DstAddr;

//network address
uint16 nwkAddr;
uint8 nwkAddr_buf[3];
//mac address
ZLongAddr_t macAddr;

uint8 SHORT_ADDR_G[4] = "";

uint8 EXT_ADDR_G[16] = "";

uint8 isFirstState = 1;

#if(DEVICE_TYPE_ID!=0)
/* operations data */
uint8 *optData = NULL;
uint8 optDataLen = 0;
#endif


/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
extern const uint8 f_tail[4]; 

/*********************************************************************
 * LOCAL FUNCTIONS
 */
#if defined(POWERON_FACTORY_SETTING) && !defined(RTR_NWK)
void CommonApp_PowerOnFactorySetting(devStates_t status);
void CommonApp_PowerSettingCountCB( void *params, 
										uint16 *duration, uint8 *count);
#endif
static void CommonApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg );
static void CommonApp_afDatacfm(afDataConfirm_t *data);
static void CommonApp_HandleKeys( byte shift, uint16 keys );
#ifdef RTR_NWK
static void CommonApp_PermitJoiningLedIndicate(
				void *params, uint16 *duration, uint8 *count);
#endif

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      CommonApp_Init
 *
 * @brief   Initialization function for the Commonr App Task.
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

void CommonApp_Init( uint8 task_id )
{
    CommonApp_TaskID = task_id;
    CommonApp_NwkState = DEV_INIT;
    CommonApp_TransID = 0;

#if(HAL_UART==TRUE)
#ifndef HAL_UART01_BOTH
	Serial_Init(Data_TxHandler);
#else
	Serial_Init(Data0_TxHandler, Data1_TxHandler);
#endif
#endif
  
    // Device hardware initialization can be added here or in main() (Zmain.c).
    // If the hardware is application specific - add it here.
    // If the hardware is other parts of the device add it in main().
  
    CommonApp_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
    CommonApp_DstAddr.endPoint = COMMONAPP_ENDPOINT;
    CommonApp_DstAddr.addr.shortAddr = 0xFFFF;

  // Fill out the endpoint description.
    CommonApp_epDesc.endPoint = COMMONAPP_ENDPOINT;
    CommonApp_epDesc.task_id = &CommonApp_TaskID;
    CommonApp_epDesc.simpleDesc
              = (SimpleDescriptionFormat_t *)&CommonApp_SimpleDesc;
    CommonApp_epDesc.latencyReq = noLatencyReqs;
  
    // Register the endpoint description with the AF
    afRegister( &CommonApp_epDesc );
  
    // Register for all key events - This app will handle all key events
    RegisterForKeys( CommonApp_TaskID );
  
    ZDO_RegisterForZDOMsg( CommonApp_TaskID, End_Device_Bind_rsp );
    ZDO_RegisterForZDOMsg( CommonApp_TaskID, Match_Desc_rsp );

    CommonApp_InitConfirm(task_id);
}


/*********************************************************************
 * @fn      CommonrApp_ProcessEvent
 *
 * @brief   Common Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 CommonApp_ProcessEvent(uint8 task_id, uint16 events)
{
    afIncomingMSGPacket_t *MSGpkt;
    afDataConfirm_t *afDataConfirm;
  
    // Data Confirmation message fields
    byte sentEP;
    ZStatus_t sentStatus;
    byte sentTransID;       // This should match the value sent
    (void)task_id;  // Intentionally unreferenced parameter
    
    if ( events & SYS_EVENT_MSG )
    {
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( CommonApp_TaskID );
      while ( MSGpkt )
      {
        switch ( MSGpkt->hdr.event )
        {
        case ZDO_CB_MSG:
		  CommonApp_ProcessZDOMsgs( (zdoIncomingMsg_t *)MSGpkt );
          break;

        case KEY_CHANGE:
		  CommonApp_HandleKeys( ((keyChange_t *)MSGpkt)->state, ((keyChange_t *)MSGpkt)->keys );
          break;

        case AF_DATA_CONFIRM_CMD:
          // This message is received as a confirmation of a data packet sent.
          // The status is of ZStatus_t type [defined in ZComDef.h]
          // The message fields are defined in AF.h
          afDataConfirm = (afDataConfirm_t *)MSGpkt;
          sentEP = afDataConfirm->endpoint;
          sentStatus = afDataConfirm->hdr.status;
          sentTransID = afDataConfirm->transID;
          (void)sentEP;
          (void)sentTransID;

          // Action taken when confirmation is received.
          if ( sentStatus != ZSuccess )
          {
            CommonApp_afDatacfm( afDataConfirm );
          }
          break;

        case AF_INCOMING_MSG_CMD:
#ifndef POWER_SAVING
#if defined(HAL_MT7620_GPIO_MAP) || (DEVICE_TYPE_ID==13) || (DEVICE_TYPE_ID==14)
		  HalLedSet( HAL_LED_1,  HAL_LED_MODE_BLINK);
#else
		  HalLedSet(HAL_LED_2, HAL_LED_MODE_BLINK);
#endif
          CommonApp_MessageMSGCB( MSGpkt );
#endif
          break;

        case ZDO_STATE_CHANGE:
		  HalLedSet(HAL_LED_3, HAL_LED_MODE_BLINK);
		  CommonApp_NwkState = (devStates_t)(MSGpkt->hdr.status);
#if defined(POWERON_FACTORY_SETTING) && !defined(RTR_NWK)
		  CommonApp_PowerOnFactorySetting(CommonApp_NwkState);
#endif
		  CommonApp_ProcessZDOStates( CommonApp_NwkState );
                  isFirstState = 0;
  
          break;

        default:
          break;
      }

      // Release the memory
      osal_msg_deallocate( (uint8 *)MSGpkt );

      // Next
      MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( CommonApp_TaskID );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }
  
  return process_event(task_id, events);
}


/*********************************************************************
 * Event Generation Functions
 */

#if defined(POWERON_FACTORY_SETTING) && !defined(RTR_NWK)
#define POWERSETTING_COUNT	3
static uint8 hasConn = 0;
static uint8 isNotTimeOut = 1;
static uint8 isPowerSetCount = 0;
static uint8 powerSetCount = 0;
/*********************************************************************
 * @fn      CommonApp_PowerOnFactorySetting
 *
 * @brief   reset to factory setting by power on off
 *
 * @param   network status
 *
 * @return  none
 */
void CommonApp_PowerOnFactorySetting(devStates_t status)
{
  if(isFirstState)
  {
	if ( osal_nv_item_init( ZCD_NV_POWER_SETTING_COUNT,
                              sizeof(powerSetCount),
                              &powerSetCount ) == ZSUCCESS )
	{
		osal_nv_read( ZCD_NV_POWER_SETTING_COUNT,
		            	0,
		                sizeof( powerSetCount ),
		                &powerSetCount);
	}

	set_user_event(CommonApp_TaskID, POWERSETTING_COUNT_EVT, CommonApp_PowerSettingCountCB, 
  		POWERSETTING_TIMEOUT, TIMER_ONE_EXECUTION, NULL);
  }
	
  if(status == DEV_ZB_COORD || status == DEV_ROUTER 
  		|| status == DEV_END_DEVICE)
  {
	hasConn = 1;
	
	if(powerSetCount != 0)
	{
		powerSetCount = 0;
		if ( osal_nv_item_init( ZCD_NV_POWER_SETTING_COUNT,
                              sizeof(powerSetCount),
                              &powerSetCount ) == ZSUCCESS )
		{
		    osal_nv_write( ZCD_NV_POWER_SETTING_COUNT,
		                  0,
		                  sizeof( powerSetCount ),
		                  &powerSetCount);
		}
	}

	return;
  }
  
  if ((zgReadStartupOptions()&ZCD_STARTOPT_DEFAULT_NETWORK_STATE)
  		|| hasConn || isNotTimeOut)
  {
    return;
  }

  if(!isPowerSetCount)
  {
  	isPowerSetCount = 1;
  	powerSetCount++;
  	if ( osal_nv_item_init( ZCD_NV_POWER_SETTING_COUNT,
                            sizeof(powerSetCount),
                            &powerSetCount ) == ZSUCCESS )
  	{
	  osal_nv_write( ZCD_NV_POWER_SETTING_COUNT,
		             0,
		             sizeof( powerSetCount ),
		             &powerSetCount);
  	}

	if(powerSetCount <= POWERSETTING_COUNT)
  	{
	  return;
  	}

#if defined(HOLD_INIT_AUTHENTICATION)
    if(devState != DEV_HOLD)
    {
      HalLedBlink ( HAL_LED_4, 0, 50, 100 );
      devStates_t tStates;
      if (ZSUCCESS == osal_nv_item_init( 
                          ZCD_NV_NWK_HOLD_STARTUP, sizeof(tStates),  &tStates))
      {
        tStates = DEV_HOLD;
        osal_nv_write(ZCD_NV_NWK_HOLD_STARTUP, 0, sizeof(tStates),  &tStates);
      }

      zgWriteStartupOptions(ZG_STARTUP_SET, ZCD_STARTOPT_DEFAULT_NETWORK_STATE);
      WatchDogEnable( WDTIMX );
    }
#endif
  }
}

void CommonApp_PowerSettingCountCB( void *params, 
										uint16 *duration, uint8 *count)
{
	if(CommonApp_NwkState != DEV_ZB_COORD && CommonApp_NwkState != DEV_ROUTER 
  		|| CommonApp_NwkState != DEV_END_DEVICE)
	{
		HalLedBlink ( HAL_LED_2, 2, 50, 100 );
	}
	
	isNotTimeOut = 0;
}
#endif


/*********************************************************************
 * @fn      CommonApp_ProcessZDOMsgs()
 *
 * @brief   Process response messages
 *
 * @param   incoming message
 *
 * @return  none
 */
void CommonApp_ProcessZDOMsgs( zdoIncomingMsg_t *inMsg )
{
  switch ( inMsg->clusterID )
  {
    case End_Device_Bind_rsp:
      if ( ZDO_ParseBindRsp( inMsg ) == ZSuccess )
      {
        // Light LED
        //HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
      }
#if defined( BLINK_LEDS )
      else
      {
        // Flash LED to show failure
        //HalLedSet ( HAL_LED_4, HAL_LED_MODE_FLASH );
      }
#endif
      break;

    case Match_Desc_rsp:
      {
        ZDO_ActiveEndpointRsp_t *pRsp = ZDO_ParseEPListRsp( inMsg );
        if ( pRsp )
        {
          if ( pRsp->status == ZSuccess && pRsp->cnt )
          {
            CommonApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
            CommonApp_DstAddr.addr.shortAddr = pRsp->nwkAddr;
            // Take the first endpoint, Can be changed to search through endpoints
            CommonApp_DstAddr.endPoint = pRsp->epList[0];

            // Light LED
            //HalLedSet( HAL_LED_4, HAL_LED_MODE_ON );
          }
          osal_mem_free( pRsp );
        }
      }
      break;
  }
}


/*********************************************************************
 * @fn      CommonApp_afDatacfm()
 *
 * @brief   Process send data comfirm
 *
 * @param   comfirm message
 *
 * @return  none
 */
void CommonApp_afDatacfm(afDataConfirm_t *data)
{}


/*********************************************************************
 * @fn      CommonApp_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_4
 *                 HAL_KEY_SW_3
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
void CommonApp_HandleKeys( uint8 shift, uint16 keys )
{
  zAddrType_t dstAddr;

  // Shift is used to make each button/switch dual purpose.
  if ( shift == HAL_KEY_STATE_SHIFT )
  {
    if ( keys & HAL_KEY_SW_1 )
    {
    }
    if ( keys & HAL_KEY_SW_2 )
    {
    }
    if ( keys & HAL_KEY_SW_3 )
    {
    }
    if ( keys & HAL_KEY_SW_4 )
    {
    }
  }
  else
  {
    if ( keys & HAL_KEY_SW_1 )
    {
      // Since SW1 isn't used for anything else in this application...
#if defined( SWITCH1_BIND )
      // we can use SW1 to simulate SW2 for devices that only have one switch,
      keys |= HAL_KEY_SW_2;
#elif defined( SWITCH1_MATCH )
      // or use SW1 to simulate SW4 for devices that only have one switch
      keys |= HAL_KEY_SW_4;
#endif
    }

    if ( keys & HAL_KEY_SW_2 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );

      // Initiate an End Device Bind Request for the mandatory endpoint
      dstAddr.addrMode = Addr16Bit;
      dstAddr.addr.shortAddr = COORDINATOR_ADDR; // Coordinator
      ZDP_EndDeviceBindReq( &dstAddr, NLME_GetShortAddr(),
                            CommonApp_epDesc.endPoint,
                            COMMONAPP_PROFID,
                            COMMONAPP_MAX_INCLUSTERS, (cId_t *)CommonApp_InClusterList,
                            COMMONAPP_MAX_OUTCLUSTERS, (cId_t *)CommonApp_OutClusterList,
                            FALSE );
    }

    if ( keys & HAL_KEY_SW_3 )
    {
    }

    if ( keys & HAL_KEY_SW_4 )
    {
      HalLedSet ( HAL_LED_4, HAL_LED_MODE_OFF );
      // Initiate a Match Description Request (Service Discovery)
      dstAddr.addrMode = AddrBroadcast;
      dstAddr.addr.shortAddr = NWK_BROADCAST_SHORTADDR;
      ZDP_MatchDescReq( &dstAddr, NWK_BROADCAST_SHORTADDR,
                        COMMONAPP_PROFID,
                        COMMONAPP_MAX_INCLUSTERS, (cId_t *)CommonApp_InClusterList,
                        COMMONAPP_MAX_OUTCLUSTERS, (cId_t *)CommonApp_OutClusterList,
                        FALSE );
    }
  }

  if ( (keys & HAL_KEY_SW_6) 
#ifdef KEY_PUSH_PORT_1_BUTTON
		|| (keys & HAL_KEY_PORT_1_SWITCHS)
#endif
  		)
  {
#ifdef HAL_KEY_COMBINE_INT_METHOD	
    CommonApp_HandleCombineKeys(keys, halGetKeyCount());
#endif
  }
}


/******************************************************************************
 * @fn         CommonApp_GetDeviceInfo
 *
 * @brief       The CommonApp_GetDeviceInfo function retrieves a Device Information
 *              Property.
 *
 * @param       param - The identifier for the device information
 *              pValue - A buffer to hold the device information
 *
 * @return      none
 */
void CommonApp_GetDeviceInfo ( uint8 param, void *pValue )
{
  switch(param)
  {
    case ZB_INFO_DEV_STATE:
      osal_memcpy(pValue, &devState, sizeof(uint8));
      break;
    case ZB_INFO_IEEE_ADDR:
      osal_memcpy(pValue, &aExtendedAddress, Z_EXTADDR_LEN);
      break;
    case ZB_INFO_SHORT_ADDR:
      osal_memcpy(pValue, &_NIB.nwkDevAddress, sizeof(uint16));
      break;
    case ZB_INFO_PARENT_SHORT_ADDR:
      osal_memcpy(pValue, &_NIB.nwkCoordAddress, sizeof(uint16));
      break;
    case ZB_INFO_PARENT_IEEE_ADDR:
      osal_memcpy(pValue, &_NIB.nwkCoordExtAddress, Z_EXTADDR_LEN);
      break;
    case ZB_INFO_CHANNEL:
      osal_memcpy(pValue, &_NIB.nwkLogicalChannel, sizeof(uint8));
      break;
    case ZB_INFO_PAN_ID:
      osal_memcpy(pValue, &_NIB.nwkPanId, sizeof(uint16));
      break;
    case ZB_INFO_EXT_PAN_ID:
      osal_memcpy(pValue, &_NIB.extendedPANID, Z_EXTADDR_LEN);
      break;
  }
}


#if(DEVICE_TYPE_ID!=0)
int8 CommonDevice_SetData(uint8 const *data, uint8 dataLen)
{
	if(optData != NULL && optDataLen < dataLen && dataLen <= MAX_OPTDATA_SIZE)
	{
		osal_mem_free(optData);
		optData = NULL;
	}

	if(dataLen <= MAX_OPTDATA_SIZE)
	{
		if(optData == NULL && dataLen != 0)
		{
			optData = osal_mem_alloc(dataLen);
		}

		osal_memcpy(optData, data, dataLen);
		optDataLen = dataLen;

		return set_device_data(optData, optDataLen);
	}	
	
	return -1;
}


int8 CommonDevice_GetData(uint8 *data, uint8 *dataLen)
{
	return get_device_data(data, dataLen);
}


/*********************************************************************
 * @fn      CommonApp_CmdPeroidCB
 *
 * @brief   command peroid event callback function
 *
 * @param   none
 *
 * @return  none
 */
void CommonApp_CmdPeroidCB( void *params, uint16 *duration, uint8 *count)
{
	uint8 buf[FRAME_DATA_SIZE] = {0};
	uint8 len = 0;
	
	if(!CommonDevice_GetData(buf, &len))
		Update_Refresh(buf, len);
}


/***************************************************************
                     控制命令与返回
*****************************************************************/
int8 DataCmd_Ctrl(uint8 *data, uint8 length)
{
	return CommonDevice_SetData(data, length);
}

void Update_Refresh(uint8 *data, uint8 length)
{
  	UR_t mFrame;
	uint8 *fBuf;		//pointer data buffer
	uint16 fLen;		//buffer data length
  
	memcpy(mFrame.head, FR_HEAD_UR, 3);
#ifdef RTR_NWK
	mFrame.type = FR_DEV_ROUTER;
#else
	mFrame.type = FR_DEV_ENDDEV;
#endif
	memcpy(mFrame.ed_type, FR_APP_DEV, 2);
	memcpy(mFrame.short_addr, SHORT_ADDR_G, 4);

	mFrame.data = data;
	mFrame.data_len = length;
	
	memcpy(mFrame.tail, f_tail, 4);

	if(!SSAFrame_Package(HEAD_UR, &mFrame, &fBuf, &fLen))
	{
		if(nwkAddr == COORDINATOR_ADDR)
		{
			CommonApp_GetDevDataSend(fBuf, fLen);
		}
		else
		{
			CommonApp_SendTheMessage(COORDINATOR_ADDR, fBuf, fLen);
		}
	}
}
#endif


void PermitJoin_Refresh(uint8 *data, uint8 length)
{
    uint8 buf[48] = {0};
	memcpy(buf, FR_HEAD_UJ, 3);
	memcpy(buf+3, SHORT_ADDR_G, 4);
	memcpy(buf+7, data, length);
	memcpy(buf+7+length, f_tail, 4);
	
#ifdef SSA_CONNECTOR
	CommonApp_GetDevDataSend(buf, 11+length);
#else
	CommonApp_SendTheMessage(COORDINATOR_ADDR, buf, 11+length);
#endif
}

bool isPermitJoining = 0;
#ifdef RTR_NWK
void CommonApp_PermitJoiningLedIndicate(
				void *params, uint16 *duration, uint8 *count)
{
	uint8 mode = (int)params;
	switch(mode)
	{
	case HAL_LED_MODE_ON:
		isPermitJoining = TRUE;
#if (defined(HAL_MT7620_GPIO_MAP) && !(DEVICE_TYPE_ID==0xF0))  || (DEVICE_TYPE_ID==13)
		HalLedSet( HAL_LED_2,  mode);
#else
		HalLedSet( HAL_LED_1,  mode);
#endif
		break;

	case HAL_LED_MODE_OFF:
		isPermitJoining = FALSE;
#if (defined(HAL_MT7620_GPIO_MAP) && !(DEVICE_TYPE_ID==0xF0))  || (DEVICE_TYPE_ID==13)
		HalLedSet( HAL_LED_2,  mode);
#else
		HalLedSet( HAL_LED_1,  mode);
#endif
		break;
	}
}
#endif

ZStatus_t CommonApp_PermitJoiningRequest( byte PermitDuration )
{
#ifdef RTR_NWK
	if(PermitDuration)
	{
		CommonApp_PermitJoiningLedIndicate( (void *)HAL_LED_MODE_ON, NULL, NULL );
		
		update_user_event(CommonApp_TaskID, PERMIT_JOIN_EVT, 
							CommonApp_PermitJoiningLedIndicate, 
	  						PermitDuration*1000, TIMER_ONE_EXECUTION, 
	  						(void *)HAL_LED_MODE_OFF);
	}
	else
	{
		CommonApp_PermitJoiningLedIndicate( (void *)HAL_LED_MODE_OFF, NULL, NULL );

		update_user_event(CommonApp_TaskID, PERMIT_JOIN_EVT, 
				NULL, TIMER_NO_LIMIT, TIMER_CLEAR_EXECUTION, NULL);
	}

	return NLME_PermitJoiningRequest(PermitDuration);
#else 
	return 0;
#endif
}

/*********************************************************************
 * @fn      CommonApp_SendTheMessage
 *
 * @brief   Send "the" message.
 *
 * @param   none
 *
 * @return  none
 */
void CommonApp_SendTheMessage(uint16 dstNwkAddr, uint8 *data, uint8 length)
{
  CommonApp_DstAddr.addrMode = (afAddrMode_t)Addr16Bit;
  CommonApp_DstAddr.endPoint = COMMONAPP_ENDPOINT;
  CommonApp_DstAddr.addr.shortAddr = dstNwkAddr;
  
  AF_DataRequest( &CommonApp_DstAddr, &CommonApp_epDesc,
                       COMMONAPP_CLUSTERID,
                       length,
                       data,
                       &CommonApp_TransID,
                       AF_DISCV_ROUTE, AF_DEFAULT_RADIUS );
}

void CommonApp_GetDevDataSend(uint8 *buf, uint16 len)
{
#if (HAL_UART==TRUE)
  #ifndef HAL_UART01_BOTH
        
	HalUARTWrite(SERIAL_COM_PORT, buf, len);
  #else
  	HalUARTWrite(SERIAL_COM_PORT1, buf, len);
  #endif
#endif
}

void RS485_GetDevDataSend(uint8 *buf, uint16 len)
{
  uint8 lenght = len;

  if(lenght==3)
  {
   //判断是否是在链表中
    setNodeList(buf,len);
  }
  else
  HalUARTWrite(SERIAL_COM_PORT, buf, len);
}