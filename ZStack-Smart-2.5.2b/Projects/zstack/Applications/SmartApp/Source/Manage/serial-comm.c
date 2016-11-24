/**************************************************************************************************
  Filename:       serial-comm.c
  Revised:        $Date: 2014-04-25 17:25:52 -0800 (Wed, 25 Apr 2014) $
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
#include "hal_led.h"

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

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
/*´®¿Ú»º³åÇø*/
#if(HAL_UART==TRUE)
//static uint8 Serial_TxSeq;
static uint8 Serial_TxBuf[SERIAL_COM_TX_MAX];
static uint8 Serial_TxLen;

#ifndef HAL_UART01_BOTH
static UART_TxHandler mData_TxHandler;
#else
static UART_TxHandler mData0_TxHandler;
static UART_TxHandler mData1_TxHandler;
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void SerialTx_CallBack(uint8 port, uint8 event);
static void SerialTx_DataCB( uint8 port, uint8 txBuf[] , uint8 txLen);

/*********************************************************************
 * NETWORK LAYER CALLBACKS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

#ifndef HAL_UART01_BOTH
void Serial_Init(UART_TxHandler txHandler)
{
	halUARTCfg_t uartConfig;

	mData_TxHandler = txHandler;
	Serial_TxLen = 0;

	uartConfig.configured           = TRUE;
	uartConfig.baudRate             = SERIAL_COM_BAUD;
	uartConfig.flowControl          = HAL_UART_FLOW_OFF;
	uartConfig.flowControlThreshold = SERIAL_COM_THRESH;
	uartConfig.rx.maxBufSize        = SERIAL_COM_RX_SZ;
	uartConfig.tx.maxBufSize        = SERIAL_COM_TX_SZ;
	uartConfig.idleTimeout          = SERIAL_COM_IDLE;
	uartConfig.intEnable            = TRUE;
	uartConfig.callBackFunc         = SerialTx_CallBack;
	
	HalUARTOpen (SERIAL_COM_PORT, &uartConfig);
}
#else
void Serial_Init(UART_TxHandler tx0Handler, UART_TxHandler tx1Handler)
{
	halUARTCfg_t uartConfig0, uartConfig1;
	
	mData0_TxHandler = tx0Handler;
	mData1_TxHandler = tx1Handler;
	Serial_TxLen = 0;

	uartConfig0.configured           = TRUE;
	uartConfig0.baudRate             = SERIAL_COM_BAUD0;
	uartConfig0.flowControl          = HAL_UART_FLOW_OFF;
	uartConfig0.flowControlThreshold = SERIAL_COM_THRESH;
	uartConfig0.rx.maxBufSize        = SERIAL_COM_RX_SZ;
	uartConfig0.tx.maxBufSize        = SERIAL_COM_TX_SZ;
	uartConfig0.idleTimeout          = SERIAL_COM_IDLE;
	uartConfig0.intEnable            = TRUE;
	uartConfig0.callBackFunc         = SerialTx_CallBack;

	uartConfig1.configured           = TRUE;
	uartConfig1.baudRate             = SERIAL_COM_BAUD1;
	uartConfig1.flowControl          = HAL_UART_FLOW_OFF;
	uartConfig1.flowControlThreshold = SERIAL_COM_THRESH;
	uartConfig1.rx.maxBufSize        = SERIAL_COM_RX_SZ;
	uartConfig1.tx.maxBufSize        = SERIAL_COM_TX_SZ;
	uartConfig1.idleTimeout          = SERIAL_COM_IDLE;
	uartConfig1.intEnable            = TRUE;
	uartConfig1.callBackFunc         = SerialTx_CallBack;
	
	HalUARTOpen (SERIAL_COM_PORT0, &uartConfig0);
	HalUARTOpen (SERIAL_COM_PORT1, &uartConfig1);
}
#endif

/*********************************************************************
 * @fn      SerialTx_CallBack()
 *
 * @brief   receive data from serial port
 *
 * @param   serial receive port, event
 *
 * @return  none
 */
void SerialTx_CallBack(uint8 port, uint8 event)
{
  uint8 data_TxLen = 0;
  uint8 data_TxBuf[FRAME_DATA_LENGTH] = {0};

  if ((event & (HAL_UART_RX_FULL | HAL_UART_RX_ABOUT_FULL 
  			| HAL_UART_RX_TIMEOUT)) && !Serial_TxLen)
  {
    if (Serial_TxLen < SERIAL_COM_TX_MAX)
    {
      Serial_TxLen = HalUARTRead( port, Serial_TxBuf, SERIAL_COM_TX_MAX);
    }

    if(Serial_TxLen)
    {  
	  if(Serial_TxLen<FRAME_DATA_LENGTH)
        data_TxLen = Serial_TxLen;
	  else
	  	data_TxLen = FRAME_DATA_LENGTH;
	  
      memcpy(data_TxBuf, Serial_TxBuf, data_TxLen);

      memset(Serial_TxBuf, 0, SERIAL_COM_TX_MAX);
      Serial_TxLen = 0;

  	  SerialTx_DataCB(port, data_TxBuf, data_TxLen);
	}
  }
}


/*********************************************************************
 * @fn      SerialTx_DataCB()
 *
 * @brief   deal with serial receice data
 *
 * @param   none
 *
 * @return  none
 */
void SerialTx_DataCB( uint8 port, uint8 txBuf[] , uint8 txLen)
{
  HalLedSet(HAL_LED_2, HAL_LED_MODE_BLINK);
#ifndef HAL_UART01_BOTH
  mData_TxHandler(txBuf, txLen);
#else
  
  if(port == SERIAL_COM_PORT0)
  {
	mData0_TxHandler(txBuf, txLen);
  }
  else if(port == SERIAL_COM_PORT1)
  {
	mData1_TxHandler(txBuf, txLen);
  }
#endif
}


/*********************************************************************
 * @fn      SerialTx_Handler
 *
 * @brief   update user uart receive handler
 *
 * @param   uart receive handler
 *
 * @return  none
 */
void SerialTx_Handler(int port, UART_TxHandler txHandler)
{
#if(HAL_UART==TRUE)
  if(txHandler != NULL)
  {
#ifndef HAL_UART01_BOTH
  	mData_TxHandler = txHandler;
#else
	if(port == SERIAL_COM_PORT0)
	{
	  mData0_TxHandler = txHandler;
	}
	else if(port == SERIAL_COM_PORT1)
	{
	  mData1_TxHandler = txHandler;
	}
#endif
  }
#endif
}


#ifndef HAL_UART01_BOTH
/*********************************************************************
 * @fn      Data_TxHandler()
 *
 * @brief   deault function to deal with serial receice data
 *
 * @param   point to data, data length
 *
 * @return  none
 */
void Data_TxHandler(uint8 txBuf[], uint8 txLen)
{
  HalUARTWrite(SERIAL_COM_PORT, txBuf, txLen);
}
#else
void Data0_TxHandler(uint8 txBuf[], uint8 txLen)
{
  HalUARTWrite(SERIAL_COM_PORT1, txBuf, txLen);
}

void Data1_TxHandler(uint8 txBuf[], uint8 txLen)
{
  HalUARTWrite(SERIAL_COM_PORT0, txBuf, txLen);
}
#endif
#endif
