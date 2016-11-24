/**************************************************************************************************
  Filename:       hal_key.h
  Revised:        $Date: 2007-07-06 10:42:24 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

  Description:    This file contains the interface to the KEY Service.


  Copyright 2005-2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-02-05
**************************************************************************************************/

#ifndef HAL_KEY_H
#define HAL_KEY_H

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
 *                                             INCLUDES
 **************************************************************************************************/
#include "hal_board.h"

/**************************************************************************************************
 * MACROS
 **************************************************************************************************/

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/

/* Interrupt option - Enable or disable */
#define HAL_KEY_INTERRUPT_DISABLE    0x00
#define HAL_KEY_INTERRUPT_ENABLE     0x01

/* Key state - shift or nornal */
#define HAL_KEY_STATE_NORMAL          0x00
#define HAL_KEY_STATE_SHIFT           0x01

/* Switches (keys) */
#define HAL_KEY_SW_1 0x0001  // Joystick up
#define HAL_KEY_SW_2 0x0002  // Joystick right
#define HAL_KEY_SW_5 0x0004  // Joystick center
#define HAL_KEY_SW_4 0x0008  // Joystick left
#define HAL_KEY_SW_3 0x0010  // Joystick down
#define HAL_KEY_SW_6 0x0020  // Button S1 if available
#define HAL_KEY_SW_7 0x0040  // Button S2 if available

/* Joystick */
#define HAL_KEY_UP     0x01  // Joystick up
#define HAL_KEY_RIGHT  0x02  // Joystick right
#define HAL_KEY_CENTER 0x04  // Joystick center
#define HAL_KEY_LEFT   0x08  // Joystick left
#define HAL_KEY_DOWN   0x10  // Joystick down

#define HAL_KEY_PORT_1_SWITCH_1 0x0100 
#define HAL_KEY_PORT_1_SWITCH_2 0x0200
#define HAL_KEY_PORT_1_SWITCH_3 0x0400
#define HAL_KEY_PORT_1_SWITCH_4 0x0800
#define HAL_KEY_PORT_1_SWITCH_5 0x1000
#define HAL_KEY_PORT_1_SWITCH_6 0x2000
#define HAL_KEY_PORT_1_SWITCH_7 0x4000
#define HAL_KEY_PORT_1_SWITCH_8 0x8000
#define HAL_KEY_PORT_1_SWITCHS	0xFF00	//switch 1.  ~  switch 8.

#define HAL_KEY_MATCH_ID_SIZE	32

#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
#define HAL_KEY_SHORT_PUSH	1
#define HAL_KEY_LONG_PUSH	2
#define HAL_KEY_LONG_LONG_PUSH	3
#endif

/**************************************************************************************************
 * TYPEDEFS
 **************************************************************************************************/
typedef void (*halKeyCBack_t) (uint16 keys, uint8 state);

/**************************************************************************************************
 *                                             GLOBAL VARIABLES
 **************************************************************************************************/
extern bool Hal_KeyIntEnable;

/**************************************************************************************************
 *                                             FUNCTIONS - API
 **************************************************************************************************/

/*
 * Initialize the Key Service
 */
extern void HalKeyInit( void );

/*
 * Configure the Key Service
 */
extern void HalKeyConfig( bool interruptEnable, const halKeyCBack_t cback);

/*
 * Read the Key status
 */
extern uint8 HalKeyRead( void);

/*
 * Enter sleep mode, store important values
 */
extern void HalKeyEnterSleep ( void );

/*
 * Exit sleep mode, retore values
 */
extern uint8 HalKeyExitSleep ( void );

/*
 * This is for internal used by hal_driver
 */
extern void HalKeyPoll ( void );

#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
extern uint8 HalKeyGetEdge(uint8 port);
extern uint32 HalKeyEdgeChanged(uint8 port);
#endif
#ifdef HAL_KEY_COMBINE_INT_METHOD
extern void HalKeyCountPoll ( void );
#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
extern void HalLongKeyListener( void );
#endif
#ifdef HAL_KEY_MATCH_ID
extern uint8 *get_keys_id(void);
#endif
#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
uint8 *get_keys_push(void);
#endif
#endif
extern uint8 halGetKeyCount( void );

/*
 * This is for internal used by hal_sleep
 */
extern bool HalKeyPressed( void );

/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
