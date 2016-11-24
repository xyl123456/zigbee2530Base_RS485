/**************************************************************************************************
  Filename:       serial-comm.h
  Revised:        $Date: 2014-04-25 17:21:26 -0800 (Wed, 25 Apr 2014) $
  Revision:       $Revision: 29217 $

  Description:    Setting serial port.
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-08-27
**************************************************************************************************/

#ifndef SERIAL_COM_H
#define SERIAL_COM_H


#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "string.h"
#include "hal_uart.h"

/*********************************************************************
 * CONSTANTS
 */
#ifndef HAL_UART01_BOTH
#ifndef SERIAL_COM_PORT
#if HAL_UART_DMA==2 || HAL_UART_ISR==2
#define SERIAL_COM_PORT  1
#else
#define SERIAL_COM_PORT  0
#endif
#endif

#if !defined( SERIAL_COM_BAUD )
//#define SERIAL_COM_BAUD  HAL_UART_BR_38400
#define SERIAL_COM_BAUD  HAL_UART_BR_115200
#endif

#else
#define SERIAL_COM_PORT0  0
#define SERIAL_COM_PORT1  1

#define SERIAL_COM_BAUD0  HAL_UART_BR_9600
#define SERIAL_COM_BAUD1  HAL_UART_BR_115200
#endif

// When the Rx buf space is less than this threshold, invoke the Rx callback.
#if !defined( SERIAL_COM_THRESH )
#define SERIAL_COM_THRESH  64
#endif

#if !defined( SERIAL_COM_RX_SZ )
#define SERIAL_COM_RX_SZ  128
#endif

#if !defined( SERIAL_COM_TX_SZ )
#define SERIAL_COM_TX_SZ  128
#endif

// Millisecs of idle time after a byte is received before invoking Rx callback.
#if !defined( SERIAL_COM_IDLE )
#define SERIAL_COM_IDLE  6
#endif

// This is the max byte count per OTA message.
#if !defined( SERIAL_COM_TX_MAX )
#define SERIAL_COM_TX_MAX  80
#endif

#define SERIAL_COM_RSP_CNT  4

#define FRAME_DATA_LENGTH   64		//frame data length by serial port

//type structure of UART receive handler
typedef void(*UART_TxHandler)(uint8[], uint8);

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * FUNCTIONS
 */
#if(HAL_UART==TRUE)
#ifndef HAL_UART01_BOTH
extern void Serial_Init(UART_TxHandler txHandler);
extern void Data_TxHandler(uint8 txBuf[], uint8 txLen);
#else
extern void Serial_Init(UART_TxHandler tx0Handler, UART_TxHandler tx1Handler);
extern void Data0_TxHandler(uint8 txBuf[], uint8 txLen);
extern void Data1_TxHandler(uint8 txBuf[], uint8 txLen);
#endif

extern void SerialTx_Handler(int port, UART_TxHandler txHandler);
#endif

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_COM_H */

