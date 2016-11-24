/**************************************************************************************************
  Filename:       hal_key.c
  Revised:        $Date: 2010-09-15 19:02:45 -0700 (Wed, 15 Sep 2010) $
  Revision:       $Revision: 23815 $

  Description:    This file contains the interface to the HAL KEY Service.


  Copyright 2006-2010 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
/*********************************************************************
 NOTE: If polling is used, the hal_driver task schedules the KeyRead()
       to occur every 100ms.  This should be long enough to naturally
       debounce the keys.  The KeyRead() function remembers the key
       state of the previous poll and will only return a non-zero
       value if the key state changes.

 NOTE: If interrupts are used, the KeyRead() function is scheduled
       25ms after the interrupt occurs by the ISR.  This delay is used
       for key debouncing.  The ISR disables any further Key interrupt
       until KeyRead() is executed.  KeyRead() will re-enable Key
       interrupts after executing.  Unlike polling, when interrupts
       are enabled, the previous key state is not remembered.  This
       means that KeyRead() will return the current state of the keys
       (not a change in state of the keys).

 NOTE: If interrupts are used, the KeyRead() fucntion is scheduled by
       the ISR.  Therefore, the joystick movements will only be detected
       during a pushbutton interrupt caused by S1 or the center joystick
       pushbutton.

 NOTE: When a switch like S1 is pushed, the S1 signal goes from a normally
       high state to a low state.  This transition is typically clean.  The
       duration of the low state is around 200ms.  When the signal returns
       to the high state, there is a high likelihood of signal bounce, which
       causes a unwanted interrupts.  Normally, we would set the interrupt
       edge to falling edge to generate an interrupt when S1 is pushed, but
       because of the signal bounce, it is better to set the edge to rising
       edge to generate an interrupt when S1 is released.  The debounce logic
       can then filter out the signal bounce.  The result is that we typically
       get only 1 interrupt per button push.  This mechanism is not totally
       foolproof because occasionally, signal bound occurs during the falling
       edge as well.  A similar mechanism is used to handle the joystick
       pushbutton on the DB.  For the EB, we do not have independent control
       of the interrupt edge for the S1 and center joystick pushbutton.  As
       a result, only one or the other pushbuttons work reasonably well with
       interrupts.  The default is the make the S1 switch on the EB work more
       reliably.

*********************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-03-03
**************************************************************************************************/

/**************************************************************************************************
 *                                            INCLUDES
 **************************************************************************************************/
#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_board.h"
#include "hal_drivers.h"
#include "hal_adc.h"
#include "hal_key.h"
#include "hal_led.h"
#include "hal_uart.h"
#include "osal.h"

#if (defined HAL_KEY) && (HAL_KEY == TRUE)

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/
#define HAL_KEY_RISING_EDGE   0
#define HAL_KEY_FALLING_EDGE  1

#define HAL_KEY_DEBOUNCE_VALUE  25

/* CPU port interrupt */
#define HAL_KEY_CPU_PORT_0_IF P0IF
#define HAL_KEY_CPU_PORT_1_IF P1IF
#define HAL_KEY_CPU_PORT_2_IF P2IF

#ifdef KEY_PUSH_PORT_1_BUTTON
#if !defined(PUSH_PORT_1_POLARITY) || !defined(HAL_KEY_PORT_1_BITS)
#error lack of some macro be defined when use "KEY_PUSH_PORT_1_BUTTON"!
#endif

/* KEY SW at port 1 */
#define HAL_KEY_PORT_1_SEL	P1SEL
#define HAL_KEY_PORT_1_DIR	P1DIR
#define HAL_KEY_PORT_1_ICTL	P1IEN
#define HAL_KEY_PORT_1_IEN 	IEN2
#define HAL_KEY_PORT_1_IENBITS  BV(4)
#define HAL_KEY_PORT_1_PXIFG	P1IFG

#define HAL_KEY_PORT_1_ICTLBITS		HAL_KEY_PORT_1_BITS

#define HAL_KEY_PUSH_PORT_1_BUTTON()	((PUSH_PORT_1_POLARITY(P1)) & HAL_KEY_PORT_1_BITS)
#endif

/* SW_6 is at P0.1 */
#define HAL_KEY_SW_6_PORT   P0
#ifndef KEY_PUSH_PORT_0_BUTTON
#define HAL_KEY_SW_6_BIT    BV(1)
#else
/* SW_6 change to P0.5 */
#define HAL_KEY_SW_6_BIT    PUSH1_BV
#endif
#define HAL_KEY_SW_6_SEL    P0SEL
#define HAL_KEY_SW_6_DIR    P0DIR

/* edge interrupt */
#define HAL_KEY_SW_6_EDGEBIT  BV(0)
#define HAL_KEY_SW_6_EDGE     HAL_KEY_FALLING_EDGE


/* SW_6 interrupts */
#define HAL_KEY_SW_6_IEN      IEN1  /* CPU interrupt mask register */
#define HAL_KEY_SW_6_IENBIT   BV(5) /* Mask bit for all of Port_0 */
#define HAL_KEY_SW_6_ICTL     P0IEN /* Port Interrupt Control register */
#ifndef KEY_PUSH_PORT_0_BUTTON
#define HAL_KEY_SW_6_ICTLBIT  BV(1) /* P0IEN - P0.1 enable/disable bit */
#else
#define HAL_KEY_SW_6_ICTLBIT  PUSH1_BV /* P0IEN - P0.5 enable/disable bit */
#endif
#define HAL_KEY_SW_6_PXIFG    P0IFG /* Interrupt flag at source */

/* Joy stick move at P2.0 */
#define HAL_KEY_JOY_MOVE_PORT   P2
#define HAL_KEY_JOY_MOVE_BIT    BV(0)
#define HAL_KEY_JOY_MOVE_SEL    P2SEL
#define HAL_KEY_JOY_MOVE_DIR    P2DIR

/* edge interrupt */
#define HAL_KEY_JOY_MOVE_EDGEBIT  BV(3)
#define HAL_KEY_JOY_MOVE_EDGE     HAL_KEY_FALLING_EDGE

/* Joy move interrupts */
#define HAL_KEY_JOY_MOVE_IEN      IEN2  /* CPU interrupt mask register */
#define HAL_KEY_JOY_MOVE_IENBIT   BV(1) /* Mask bit for all of Port_2 */
#define HAL_KEY_JOY_MOVE_ICTL     P2IEN /* Port Interrupt Control register */
#define HAL_KEY_JOY_MOVE_ICTLBIT  BV(0) /* P2IENL - P2.0<->P2.3 enable/disable bit */
#define HAL_KEY_JOY_MOVE_PXIFG    P2IFG /* Interrupt flag at source */

#define HAL_KEY_JOY_CHN   HAL_ADC_CHANNEL_6

#define HAL_INTERVAL_KEY_CLOCK_THRESHOLD	900u
#define HAL_LONG_KEY_TIMEOUT	4100u

#define HAL_KEY_PUSH_CLOCK_SHORT_THRESHOLD	0x200
#define HAL_KEY_PUSH_CLOCK_LONG_THRESHOLD	0x400

#if !(defined HAL_KEY_INT_METHOD) && (defined HAL_KEY_COMBINE_INT_METHOD)
#error "you must define HAL_KEY_INT_METHOD before define HAL_KEY_COMBINE_INT_METHOD"
#endif


/**************************************************************************************************
 *                                            TYPEDEFS
 **************************************************************************************************/


/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
static uint16 halKeySavedKeys;     /* used to store previous key state in polling mode */
static halKeyCBack_t pHalKeyProcessFunction;
static uint8 HalKeyConfigured;
bool Hal_KeyIntEnable;            /* interrupt enable/disable flag */

#ifdef HAL_KEY_COMBINE_INT_METHOD
static uint32 preKeyClock;		/* last time clock */
static uint32 currentKeyClock;	/* current time clock */
static uint8 constantKeyCount;	
static bool countEnd;
static uint8 KeyCount;
static uint16 mKeys;
#endif

#ifdef HAL_KEY_MATCH_ID
#ifndef HAL_KEY_COMBINE_INT_METHOD
#error must define "HAL_KEY_COMBINE_INT_METHOD" before "HAL_KEY_MATCH_ID".
#endif

static const char keylist[] = {'a', '1', '2','3','4','5','6','7','8'};
static uint8 keyID[HAL_KEY_MATCH_ID_SIZE];
#endif

#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
static uint8 keysPush[HAL_KEY_MATCH_ID_SIZE];
static uint8 isCombine;
#endif

#if defined(HAL_KEY_LONG_SHORT_DISTINGUISH) && !defined(HAL_KEY_INT_METHOD)
#error must define "HAL_KEY_INT_METHOD" before "HAL_KEY_LONG_SHORT_DISTINGUISH".
#endif

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
void halProcessKeyInterrupt(void);
uint8 halGetJoyKeyInput(void);
#ifdef HAL_KEY_MATCH_ID
extern void set_keys_id(uint16 keys);
#endif
#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
extern void set_keys_push(uint8 keyPush);
#endif


/**************************************************************************************************
 *                                        FUNCTIONS - API
 **************************************************************************************************/


/**************************************************************************************************
 * @fn      HalKeyInit
 *
 * @brief   Initilize Key Service
 *
 * @param   none
 *
 * @return  None
 **************************************************************************************************/
void HalKeyInit( void )
{
  /* Initialize previous key to 0 */
  halKeySavedKeys = 0;

  HAL_KEY_SW_6_SEL &= ~(HAL_KEY_SW_6_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_6_DIR &= ~(HAL_KEY_SW_6_BIT);    /* Set pin direction to Input */

  /* No KEY JOY */
#ifndef KEY_PUSH_PORT_0_BUTTON
  HAL_KEY_JOY_MOVE_SEL &= ~(HAL_KEY_JOY_MOVE_BIT); /* Set pin function to GPIO */
  HAL_KEY_JOY_MOVE_DIR &= ~(HAL_KEY_JOY_MOVE_BIT); /* Set pin direction to Input */
#endif

#ifdef KEY_PUSH_PORT_1_BUTTON
  HAL_KEY_PORT_1_SEL &= ~(HAL_KEY_PORT_1_BITS);
  HAL_KEY_PORT_1_DIR &= ~(HAL_KEY_PORT_1_BITS);
#endif

#ifdef HAL_KEY_COMBINE_INT_METHOD
	preKeyClock = 0;
	currentKeyClock = 0;
	constantKeyCount = 0;
	countEnd = FALSE;
	KeyCount = 0;
#endif

  /* Initialize callback function */
  pHalKeyProcessFunction  = NULL;

  /* Start with key is not configured */
  HalKeyConfigured = FALSE;
}


/**************************************************************************************************
 * @fn      HalKeyConfig
 *
 * @brief   Configure the Key serivce
 *
 * @param   interruptEnable - TRUE/FALSE, enable/disable interrupt
 *          cback - pointer to the CallBack function
 *
 * @return  None
 **************************************************************************************************/
void HalKeyConfig (bool interruptEnable, halKeyCBack_t cback)
{
  /* Enable/Disable Interrupt or */
  Hal_KeyIntEnable = interruptEnable;

  /* Register the callback fucntion */
  pHalKeyProcessFunction = cback;

  /* Determine if interrupt is enable or not */
  if (Hal_KeyIntEnable)
  {
    /* Rising/Falling edge configuratinn */

#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
    PICTL &= ~(HAL_KEY_SW_6_EDGEBIT);    /* Clear the edge bit */
    /* For falling edge, the bit must be set. */
  #if (HAL_KEY_SW_6_EDGE == HAL_KEY_FALLING_EDGE)
    PICTL |= HAL_KEY_SW_6_EDGEBIT;
  #endif
#else
    HalKeyEdgeChanged(0);
#endif


    /* Interrupt configuration:
     * - Enable interrupt generation at the port
     * - Enable CPU interrupt
     * - Clear any pending interrupt
     */
    HAL_KEY_SW_6_ICTL |= HAL_KEY_SW_6_ICTLBIT;
    HAL_KEY_SW_6_IEN |= HAL_KEY_SW_6_IENBIT;
    HAL_KEY_SW_6_PXIFG = ~(HAL_KEY_SW_6_BIT);

#ifdef KEY_PUSH_PORT_1_BUTTON
#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
	PICTL &= ~0x06;
	PICTL |= 0x06;
#else
    HalKeyEdgeChanged(1);
#endif

	HAL_KEY_PORT_1_ICTL |= HAL_KEY_PORT_1_ICTLBITS;
    HAL_KEY_PORT_1_IEN |= HAL_KEY_PORT_1_IENBITS;
    HAL_KEY_PORT_1_PXIFG = ~(HAL_KEY_PORT_1_BITS);
#endif

#ifndef KEY_PUSH_PORT_0_BUTTON
    /* Rising/Falling edge configuratinn */

    HAL_KEY_JOY_MOVE_ICTL &= ~(HAL_KEY_JOY_MOVE_EDGEBIT);    /* Clear the edge bit */
    /* For falling edge, the bit must be set. */
  #if (HAL_KEY_JOY_MOVE_EDGE == HAL_KEY_FALLING_EDGE)
    HAL_KEY_JOY_MOVE_ICTL |= HAL_KEY_JOY_MOVE_EDGEBIT;
  #endif


    /* Interrupt configuration:
     * - Enable interrupt generation at the port
     * - Enable CPU interrupt
     * - Clear any pending interrupt
     */
    HAL_KEY_JOY_MOVE_ICTL |= HAL_KEY_JOY_MOVE_ICTLBIT;
    HAL_KEY_JOY_MOVE_IEN |= HAL_KEY_JOY_MOVE_IENBIT;
    HAL_KEY_JOY_MOVE_PXIFG = ~(HAL_KEY_JOY_MOVE_BIT);
#endif

    /* Do this only after the hal_key is configured - to work with sleep stuff */
    if (HalKeyConfigured == TRUE)
    {
      osal_stop_timerEx(Hal_TaskID, HAL_KEY_EVENT);  /* Cancel polling if active */
    }
  }
  else    /* Interrupts NOT enabled */
  {
    HAL_KEY_SW_6_ICTL &= ~(HAL_KEY_SW_6_ICTLBIT); /* don't generate interrupt */
    HAL_KEY_SW_6_IEN &= ~(HAL_KEY_SW_6_IENBIT);   /* Clear interrupt enable bit */

#ifdef KEY_PUSH_PORT_1_BUTTON
	HAL_KEY_PORT_1_ICTL &= ~(HAL_KEY_PORT_1_ICTLBITS);
    HAL_KEY_PORT_1_IEN &= ~(HAL_KEY_PORT_1_IENBITS);
#endif

    osal_set_event(Hal_TaskID, HAL_KEY_EVENT);
  }

  /* Key now is configured */
  HalKeyConfigured = TRUE;
}

/**************************************************************************************************
 * @fn      HalKeyRead
 *
 * @brief   Read the current value of a key
 *
 * @param   None
 *
 * @return  keys - current keys status
 **************************************************************************************************/
uint8 HalKeyRead ( void )
{
  uint16 keys = 0;

  if (HAL_PUSH_BUTTON1())
  {
    keys |= HAL_KEY_SW_6;
  }

#ifdef KEY_PUSH_PORT_1_BUTTON
  keys |= (HAL_KEY_PUSH_PORT_1_BUTTON() << 8);
#endif

#ifndef KEY_PUSH_PORT_0_BUTTON
  if ((HAL_KEY_JOY_MOVE_PORT & HAL_KEY_JOY_MOVE_BIT))  /* Key is active low */
  {
    keys |= halGetJoyKeyInput();
  }
#endif

  return keys;
}


/**************************************************************************************************
 * @fn      HalKeyPoll
 *
 * @brief   Called by hal_driver to poll the keys
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalKeyPoll (void)
{
  uint16 keys = 0;
#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
  uint8 mKeyPush = 0;
#ifdef KEY_PUSH_PORT_1_BUTTON
  static uint16 tKeys = 0;
#endif
#endif

#ifndef KEY_PUSH_PORT_0_BUTTON
  if ((HAL_KEY_JOY_MOVE_PORT & HAL_KEY_JOY_MOVE_BIT))  /* Key is active HIGH */
  {
    keys = halGetJoyKeyInput();
  }
#endif

#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
  if (HAL_PUSH_BUTTON1())
  {
    keys |= HAL_KEY_SW_6;
  }
#else
  if (HalKeyGetEdge(0) ^ PUSH1_SBIT)
  {
    uint16 pushclock = (uint16)HalKeyEdgeChanged(0);
    if(pushclock != 0)
    {
	  if (pushclock < HAL_KEY_PUSH_CLOCK_SHORT_THRESHOLD)
	  {
		mKeyPush = HAL_KEY_SHORT_PUSH;
	  }
	  else if (pushclock > HAL_KEY_PUSH_CLOCK_LONG_THRESHOLD)
	  {
		mKeyPush = HAL_KEY_LONG_PUSH;
	  }

	  keys |= HAL_KEY_SW_6;
    }
  }
#endif

#ifdef KEY_PUSH_PORT_1_BUTTON
#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
  keys |= (HAL_KEY_PUSH_PORT_1_BUTTON() << 8);
#else

  if ((!!HalKeyGetEdge(1)) ^ (!HAL_KEY_PUSH_PORT_1_BUTTON()))
  {

    uint16 pushclock = (uint16)HalKeyEdgeChanged(1);
	
    if(pushclock != 0)
    {
	  if (pushclock < HAL_KEY_PUSH_CLOCK_SHORT_THRESHOLD)
	  {
		mKeyPush = HAL_KEY_SHORT_PUSH;
	  }
	  else if (pushclock > HAL_KEY_PUSH_CLOCK_LONG_THRESHOLD)
	  {
		mKeyPush = HAL_KEY_LONG_PUSH;
	  }

	  keys |= tKeys;
    }
	else
	{
	  tKeys = (HAL_KEY_PUSH_PORT_1_BUTTON() << 8);
	}
  }
#endif
#endif


  /* If interrupts are not enabled, previous key status and current key status
   * are compared to find out if a key has changed status.
   */
  if (!Hal_KeyIntEnable)
  {
    if (keys == halKeySavedKeys)
    {
      /* Exit - since no keys have changed */
      return;
    }
    /* Store the current keys for comparation next time */
    halKeySavedKeys = keys;
  }
  else
  {
#ifdef HAL_KEY_COMBINE_INT_METHOD
	if (!keys) return;

	if (!constantKeyCount)
	{
		preKeyClock = osal_GetSystemClock();
#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
		isCombine = 1;
#ifdef KEY_PUSH_PORT_1_BUTTON
		tKeys = 0;
#endif
#endif
	}
	
	currentKeyClock = osal_GetSystemClock();
	countEnd = FALSE;
	osal_stop_timerEx(Hal_TaskID, HAL_KEY_COUNT_EVENT);
	osal_stop_timerEx( Hal_TaskID, HAL_LONG_KEY_EVENT);

	if (currentKeyClock - preKeyClock < HAL_INTERVAL_KEY_CLOCK_THRESHOLD)
	{
		constantKeyCount++;
		preKeyClock = currentKeyClock;
		
		mKeys = keys;
#ifdef HAL_KEY_MATCH_ID
		set_keys_id(keys);
#endif
#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
		if(constantKeyCount == 1 && mKeyPush == HAL_KEY_SHORT_PUSH)
		{
			isCombine = 0;
			memset(keyID, 0, sizeof(keyID));
			memset(keysPush, 0, sizeof(keysPush));
		}

		set_keys_push(mKeyPush);
		
		if(!isCombine)
		{
			KeyCount = constantKeyCount;
			constantKeyCount = 0;
			goto functionSolve;
		}
#endif

		osal_start_timerEx( Hal_TaskID, 
			HAL_KEY_COUNT_EVENT, HAL_INTERVAL_KEY_CLOCK_THRESHOLD);

		return;
	}
	else
	{
		KeyCount = constantKeyCount;
		constantKeyCount = 0;
		return;
	}
#endif
  }

#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
functionSolve:
#endif
  /* Invoke Callback if new keys were depressed */
  if (keys && (pHalKeyProcessFunction))
  {
    (pHalKeyProcessFunction) (keys, HAL_KEY_STATE_NORMAL);
  }
}

#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
uint8 HalKeyGetEdge(uint8 port)
{
  if (port == 0)
  {
	  return (PICTL & HAL_KEY_SW_6_EDGEBIT);
  }
  else if (port == 1)
  {
	  return (PICTL & 0x06);
  }

  return 0;
}


uint32 HalKeyEdgeChanged(uint8 port)
{
  static uint32 highClock, lowClock;
  
  if (port == 0)
  {
    if (PUSH1_SBIT)
    {
	  PICTL |= HAL_KEY_SW_6_EDGEBIT;
	  //HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF);
	  highClock = osal_GetSystemClock();
	}
	else
	{
	  PICTL &= ~HAL_KEY_SW_6_EDGEBIT;
	  //HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
	  lowClock = osal_GetSystemClock();
	}

	if(!HAL_PUSH_BUTTON1())
	{
	  if(PUSH1_POLARITY(0))
	  {
		return highClock-lowClock;
	  }
	  else
	  {
	  	return lowClock-highClock;
	  }
	}
	else
	{
		return 0;
	}
  }
#ifdef KEY_PUSH_PORT_1_BUTTON
  else if (port == 1)
  {
	if((!!HAL_KEY_PUSH_PORT_1_BUTTON()) ^ (PUSH_PORT_1_POLARITY(0) & 0x01))
    {
	  PICTL |= 0x06;
	  //HalLedSet(HAL_LED_1, HAL_LED_MODE_OFF);
	  highClock = osal_GetSystemClock();
	}
	else
	{
	  PICTL &= ~0x06;
	  //HalLedSet(HAL_LED_1, HAL_LED_MODE_ON);
	  lowClock = osal_GetSystemClock();
	}

	if(!HAL_KEY_PUSH_PORT_1_BUTTON())
	{
	  if(PUSH_PORT_1_POLARITY(0) & 0x01)
	  {
		return highClock-lowClock;
	  }
	  else
	  {
	  	return lowClock-highClock;
	  }
	}
	else
	{
		return 0;
	}
  }
#endif

  return 0;
}
#endif


#ifdef HAL_KEY_COMBINE_INT_METHOD
static bool countDown = FALSE;
void HalKeyCountPoll (void)
{
  // 长按事件检测
  if(HAL_PUSH_BUTTON1() || countDown
#ifdef KEY_PUSH_PORT_1_BUTTON
  		|| HAL_KEY_PUSH_PORT_1_BUTTON()
#endif
  )
  {	
#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
  	if(constantKeyCount == 1)
  	{
	  countDown = FALSE;
	  KeyCount = constantKeyCount;
  	  constantKeyCount = 0;
	  osal_start_timerEx( Hal_TaskID, 
			HAL_LONG_KEY_EVENT, HAL_LONG_KEY_TIMEOUT);
	  return;
	}
#endif

  	preKeyClock = osal_GetSystemClock();
	
  	if(HAL_PUSH_BUTTON1()
#ifdef KEY_PUSH_PORT_1_BUTTON
  		|| HAL_KEY_PUSH_PORT_1_BUTTON()
#endif
	)
  	{
	  countDown = TRUE;
  	}
	else
	{
	  countDown = FALSE;
	  countEnd = TRUE;
	  osal_start_timerEx( Hal_TaskID, 
			HAL_KEY_COUNT_EVENT, HAL_INTERVAL_KEY_CLOCK_THRESHOLD);
	  return;
	}

	osal_start_timerEx( Hal_TaskID, HAL_KEY_COUNT_EVENT, 25);
	return;
  }
  else if(countEnd)
  {
  	KeyCount = 0;
    constantKeyCount = 0;
  	countEnd = FALSE;
#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
	return;
#endif
  }

  //组合按键事件处理
  KeyCount = constantKeyCount;
  constantKeyCount = 0;
  if (mKeys&& (pHalKeyProcessFunction))
  {
    (pHalKeyProcessFunction) (mKeys, HAL_KEY_STATE_NORMAL);
  }
}

#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
void HalLongKeyListener(void)
{
	if((HAL_PUSH_BUTTON1() 
#ifdef KEY_PUSH_PORT_1_BUTTON
  		|| HAL_KEY_PUSH_PORT_1_BUTTON()
#endif
		)&& KeyCount == 1)
  	{
		if (mKeys && (pHalKeyProcessFunction))
	  	{
			KeyCount = 0;
    		(pHalKeyProcessFunction) (mKeys, HAL_KEY_STATE_NORMAL);
  		}
	}
}
#endif

uint8 halGetKeyCount(void)
{
	uint8 mCount = KeyCount;
	KeyCount = 0;
	
	return mCount;
}


#ifdef HAL_KEY_MATCH_ID
void set_keys_id(uint16 keys)
{
	switch(keys)
	{
	case HAL_KEY_SW_6: 
		keyID[constantKeyCount-1] = keylist[0];
		break;
		
	case HAL_KEY_PORT_1_SWITCH_1: 
		keyID[constantKeyCount-1] = keylist[1];
		break;
		
	case HAL_KEY_PORT_1_SWITCH_2: 
		keyID[constantKeyCount-1] = keylist[2];
		break;
		
	case HAL_KEY_PORT_1_SWITCH_3: 
		keyID[constantKeyCount-1] = keylist[3];
		break;
		
	case HAL_KEY_PORT_1_SWITCH_4: 
		keyID[constantKeyCount-1] = keylist[4];
		break;
		
	case HAL_KEY_PORT_1_SWITCH_5: 
		keyID[constantKeyCount-1] = keylist[5];
		break;
		
	case HAL_KEY_PORT_1_SWITCH_6: 
		keyID[constantKeyCount-1] = keylist[6];
		break;
		
	case HAL_KEY_PORT_1_SWITCH_7: 
		keyID[constantKeyCount-1] = keylist[7];
		break;
		
	case HAL_KEY_PORT_1_SWITCH_8: 
		keyID[constantKeyCount-1] = keylist[8];
		break;
		
	default: break;
	}

}

uint8 *get_keys_id(void)
{
	return keyID;
}
#endif

#ifdef HAL_KEY_LONG_SHORT_DISTINGUISH
void set_keys_push(uint8 keyPush)
{
	keysPush[constantKeyCount-1] = keyPush;
}

uint8 *get_keys_push(void)
{
	return keysPush;
}
#endif

#else
uint8 halGetKeyCount(void)
{	
	return 0;
}
#endif

/**************************************************************************************************
 * @fn      halGetJoyKeyInput
 *
 * @brief   Map the ADC value to its corresponding key.
 *
 * @param   None
 *
 * @return  keys - current joy key status
 **************************************************************************************************/
uint8 halGetJoyKeyInput(void)
{
  /* The joystick control is encoded as an analog voltage.
   * Read the JOY_LEVEL analog value and map it to joy movement.
   */
  uint8 adc;
  uint8 ksave0 = 0;
  uint8 ksave1;

  /* Keep on reading the ADC until two consecutive key decisions are the same. */
  do
  {
    ksave1 = ksave0;    /* save previouse key reading */

    adc = HalAdcRead (HAL_KEY_JOY_CHN, HAL_ADC_RESOLUTION_8);

    if ((adc >= 2) && (adc <= 38))
    {
       ksave0 |= HAL_KEY_UP;
    }
    else if ((adc >= 74) && (adc <= 88))
    {
      ksave0 |= HAL_KEY_RIGHT;
    }
    else if ((adc >= 60) && (adc <= 73))
    {
      ksave0 |= HAL_KEY_LEFT;
    }
    else if ((adc >= 39) && (adc <= 59))
    {
      ksave0 |= HAL_KEY_DOWN;
    }
    else if ((adc >= 89) && (adc <= 100))
    {
      ksave0 |= HAL_KEY_CENTER;
    }
  } while (ksave0 != ksave1);

  return ksave0;
}





/**************************************************************************************************
 * @fn      halProcessKeyInterrupt
 *
 * @brief   Checks to see if it's a valid key interrupt, saves interrupt driven key states for
 *          processing by HalKeyRead(), and debounces keys by scheduling HalKeyRead() 25ms later.
 *
 * @param
 *
 * @return
 **************************************************************************************************/
void halProcessKeyInterrupt (void)
{
  bool valid=FALSE;

  if (HAL_KEY_SW_6_PXIFG & HAL_KEY_SW_6_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_SW_6_PXIFG = ~(HAL_KEY_SW_6_BIT); /* Clear Interrupt Flag */
#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
    valid = TRUE;
#else
    if (HalKeyGetEdge(0) ^ PUSH1_SBIT)
    {
	  valid = TRUE;
	}
#endif
  }

#ifndef KEY_PUSH_PORT_0_BUTTON
  if (HAL_KEY_JOY_MOVE_PXIFG & HAL_KEY_JOY_MOVE_BIT)  /* Interrupt Flag has been set */
  {
    HAL_KEY_JOY_MOVE_PXIFG = ~(HAL_KEY_JOY_MOVE_BIT); /* Clear Interrupt Flag */
    valid = TRUE;
  }
#endif

#ifdef KEY_PUSH_PORT_1_BUTTON
  if(HAL_KEY_PORT_1_PXIFG & HAL_KEY_PORT_1_BITS)
  {
    HAL_KEY_PORT_1_PXIFG &= ~HAL_KEY_PORT_1_BITS;
#ifndef HAL_KEY_LONG_SHORT_DISTINGUISH
    valid = TRUE;
#else
    if ((!!HalKeyGetEdge(1)) ^ (!HAL_KEY_PUSH_PORT_1_BUTTON()))
    {
      valid = TRUE;
    }
#endif
  }
#endif

  if (valid)
  {
    osal_start_timerEx (Hal_TaskID, HAL_KEY_EVENT, HAL_KEY_DEBOUNCE_VALUE);
  }
}

/**************************************************************************************************
 * @fn      HalKeyEnterSleep
 *
 * @brief  - Get called to enter sleep mode
 *
 * @param
 *
 * @return
 **************************************************************************************************/
void HalKeyEnterSleep ( void )
{
}

/**************************************************************************************************
 * @fn      HalKeyExitSleep
 *
 * @brief   - Get called when sleep is over
 *
 * @param
 *
 * @return  - return saved keys
 **************************************************************************************************/
uint8 HalKeyExitSleep ( void )
{
  /* Wake up and read keys */
  return ( HalKeyRead () );
}

/***************************************************************************************************
 *                                    INTERRUPT SERVICE ROUTINE
 ***************************************************************************************************/

/**************************************************************************************************
 * @fn      halKeyPort0Isr
 *
 * @brief   Port0 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort0Isr, P0INT_VECTOR )
{
  HAL_ENTER_ISR();

  if (HAL_KEY_SW_6_PXIFG & HAL_KEY_SW_6_BIT)
  {
    halProcessKeyInterrupt();
  }

  /*
    Clear the CPU interrupt flag for Port_0
    PxIFG has to be cleared before PxIF
  */
  HAL_KEY_SW_6_PXIFG = 0;
  HAL_KEY_CPU_PORT_0_IF = 0;
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}

#ifdef KEY_PUSH_PORT_1_BUTTON
/**************************************************************************************************
 * @fn      halKeyPort1Isr
 *
 * @brief   Port1 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort1Isr, P1INT_VECTOR )
{
  HAL_ENTER_ISR();

  if (HAL_KEY_PORT_1_PXIFG & HAL_KEY_PORT_1_BITS)
  {
    halProcessKeyInterrupt();
  }

  /*
    Clear the CPU interrupt flag for Port_1
    PxIFG has to be cleared before PxIF
  */
  HAL_KEY_PORT_1_PXIFG = 0;
  HAL_KEY_CPU_PORT_1_IF = 0;
  
  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}
#endif

#ifndef KEY_PUSH_PORT_0_BUTTON
/**************************************************************************************************
 * @fn      halKeyPort2Isr
 *
 * @brief   Port2 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort2Isr, P2INT_VECTOR )
{
  HAL_ENTER_ISR();
  
  if (HAL_KEY_JOY_MOVE_PXIFG & HAL_KEY_JOY_MOVE_BIT)
  {
    halProcessKeyInterrupt();
  }

  /*
    Clear the CPU interrupt flag for Port_2
    PxIFG has to be cleared before PxIF
    Notes: P2_1 and P2_2 are debug lines.
  */
  HAL_KEY_JOY_MOVE_PXIFG = 0;
  HAL_KEY_CPU_PORT_2_IF = 0;

  CLEAR_SLEEP_MODE();
  HAL_EXIT_ISR();
}
#endif

#else


void HalKeyInit(void){}
void HalKeyConfig(bool interruptEnable, halKeyCBack_t cback){}
uint8 HalKeyRead(void){ return 0;}
void HalKeyPoll(void){}

#endif /* HAL_KEY */





/**************************************************************************************************
**************************************************************************************************/



