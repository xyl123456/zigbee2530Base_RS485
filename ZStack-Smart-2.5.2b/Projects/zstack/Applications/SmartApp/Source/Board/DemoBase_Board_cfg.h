/**************************************************************************************************
  Filename:       DemoBase_Board_cfg.h
  Revised:        $Date: 2015-02-02 19:24:32 -0800 (Tue, 02 Feb 2015) $
  Revision:       $Revision: 29217 $

  Description:    Demo Base Board Configuration.
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-07-31
**************************************************************************************************/


#ifndef DEMOBASE_BOARD_CFG_H
#define DEMOBASE_BOARD_CFG_H

#ifdef __cplusplus
extern "C"
{
#endif

#if (DEVICE_TYPE_ID==0xFE)

#define HAL_GPIO_FEATURE

#define KEY_PUSH_PORT_0_BUTTON		/* 自定义映射按键IO  */
#define HAL_KEY_INT_METHOD		/*按键触发方式为中断(默认为查询)*/

#define HAL_KEY_COMBINE_INT_METHOD	/*中断下 按键组合功能 */
#define HAL_KEY_MATCH_ID	/*按键匹配ID */
//#define HAL_KEY_LONG_SHORT_DISTINGUISH	/*按键长短按识别功能*/
#define POWERON_FACTORY_SETTING		/*电源开关计次恢复出厂设置*/

#define RS485_DEV
#ifdef RS485_DEV
#define SERIAL_COM_BAUD  HAL_UART_BR_9600
#endif


/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */
/* ------------------------------------------------------------------------------------------------
 *                                       IOCTROL Configuration
 * ------------------------------------------------------------------------------------------------
 */
#define GPIO1_BV           BV(4)
#define GPIO1_SBIT         P1_4
#define GPIO1_DDR          P1DIR
#define GPIO1_POLARITY     ACTIVE_HIGH

#define GPIO2_BV           BV(5)
#define GPIO2_SBIT         P1_5
#define GPIO2_DDR          P1DIR
#define GPIO2_POLARITY     ACTIVE_HIGH

#define GPIO3_BV           BV(6)
#define GPIO3_SBIT         P1_6
#define GPIO3_DDR          P1DIR
#define GPIO3_POLARITY     ACTIVE_HIGH

#define GPIO4_BV           BV(7)
#define GPIO4_SBIT         P1_7
#define GPIO4_DDR          P1DIR
#define GPIO4_POLARITY     ACTIVE_HIGH
  
  
/*********************************************************************
 * MACROS
 */
/* 1 - Green */
#define LED1_BV           BV(0)
#define LED1_SBIT         P1_0
#define LED1_DDR          P1DIR
#define LED1_POLARITY     ACTIVE_HIGH

/* 2 - Red */
#define LED2_BV           BV(1)
#define LED2_SBIT         P1_1
#define LED2_DDR          P1DIR
#define LED2_POLARITY     ACTIVE_HIGH

/* 3 - Yellow */
#define LED3_BV           BV(4)
#define LED3_SBIT         P1_4
#define LED3_DDR          P1DIR
#define LED3_POLARITY     ACTIVE_HIGH

/* S1 */
#define PUSH1_BV          BV(5)
#define PUSH1_SBIT        P0_5
#define PUSH1_POLARITY    ACTIVE_LOW


#define HAL_PUSH_BUTTON1()        (PUSH1_POLARITY (PUSH1_SBIT))
#define HAL_PUSH_BUTTON2()        (0)
#define HAL_PUSH_BUTTON3()        (0)
#define HAL_PUSH_BUTTON4()        (0)
#define HAL_PUSH_BUTTON5()        (0)
#define HAL_PUSH_BUTTON6()        (0)

/*定义GPIO*/
#define HAL_TURN_OFF_GPIO1()       st( GPIO1_SBIT = GPIO1_POLARITY (0); )
#define HAL_TURN_ON_GPIO1()        st( GPIO1_SBIT = GPIO1_POLARITY (1); )
#define HAL_TOGGLE_GPIO1()         st( if (GPIO1_SBIT) { GPIO1_SBIT = 0; } else { GPIO1_SBIT = 1;} )
#define HAL_STATE_GPIO1()          (GPIO1_POLARITY (GPIO1_SBIT))
  
#define HAL_TURN_OFF_GPIO2()       st( GPIO2_SBIT = GPIO2_POLARITY (0); )
#define HAL_TURN_ON_GPIO2()        st( GPIO2_SBIT = GPIO2_POLARITY (1); )
#define HAL_TOGGLE_GPIO2()         st( if (GPIO2_SBIT) { GPIO2_SBIT = 0; } else { GPIO2_SBIT = 1;} )
#define HAL_STATE_GPIO2()          (GPIO2_POLARITY (GPIO2_SBIT)) 
  
#define HAL_TURN_OFF_GPIO3()       st( GPIO3_SBIT = GPIO3_POLARITY (0); )
#define HAL_TURN_ON_GPIO3()        st( GPIO3_SBIT = GPIO3_POLARITY (1); )
#define HAL_TOGGLE_GPIO3()         st( if (GPIO3_SBIT) { GPIO3_SBIT = 0; } else { GPIO3_SBIT = 1;} )
#define HAL_STATE_GPIO3()          (GPIO3_POLARITY (GPIO3_SBIT))
  
#define HAL_TURN_OFF_GPIO4()       st( GPIO4_SBIT = GPIO4_POLARITY (0); )
#define HAL_TURN_ON_GPIO4()        st( GPIO4_SBIT = GPIO4_POLARITY (1); )
#define HAL_TOGGLE_GPIO4()         st( if (GPIO4_SBIT) { GPIO4_SBIT = 0; } else { GPIO4_SBIT = 1;} )
#define HAL_STATE_GPIO4()          (GPIO4_POLARITY (GPIO4_SBIT))
  
  

#define HAL_TURN_OFF_LED1()       st( LED1_SBIT = LED1_POLARITY (0); )
#define HAL_TURN_ON_LED1()        st( LED1_SBIT = LED1_POLARITY (1); )
#define HAL_TOGGLE_LED1()         st( if (LED1_SBIT) { LED1_SBIT = 0; } else { LED1_SBIT = 1;} )
#define HAL_STATE_LED1()          (LED1_POLARITY (LED1_SBIT))


#define HAL_TURN_OFF_LED2()       st( LED2_SBIT = LED2_POLARITY (0); )
#define HAL_TURN_ON_LED2()        st( LED2_SBIT = LED2_POLARITY (1); )
#define HAL_TOGGLE_LED2()         st( if (LED2_SBIT) { LED2_SBIT = 0; } else { LED2_SBIT = 1;} )
#define HAL_STATE_LED2()          (LED2_POLARITY (LED2_SBIT))

#define HAL_TURN_OFF_LED3()       st( LED3_SBIT = LED3_POLARITY (0); )
#define HAL_TURN_ON_LED3()        st( LED3_SBIT = LED3_POLARITY (1); )
#define HAL_TOGGLE_LED3()         st( if (LED3_SBIT) { LED3_SBIT = 0; } else { LED3_SBIT = 1;} )
#define HAL_STATE_LED3()          (LED3_POLARITY (LED3_SBIT))

#define HAL_TURN_OFF_LED4()       HAL_TURN_OFF_LED1()
#define HAL_TURN_ON_LED4()        HAL_TURN_ON_LED1()
#define HAL_TOGGLE_LED4()         HAL_TOGGLE_LED1()
#define HAL_STATE_LED4()          HAL_STATE_LED1()

/* BASE LED */
#define BASELED_PORT         P1
#define BASELED_DDR          P1DIR
#define BASELED_POLARITY     ~~

#define HAL_INIT_BASELED()		  st( BASELED_DDR |= 0xFF; BASELED_PORT = BASELED_POLARITY(0); )
#define HAL_SET_BASELED(x)        st( BASELED_PORT = BASELED_POLARITY (x); )
#define HAL_STATE_BASELED()      (BASELED_POLARITY (BASELED_PORT))

/* BASE PLC */
#define PLCIN1_BV           BV(0)
#define PLCIN1_SBIT         P1_0
#define PLCIN1_DDR          P1DIR
#define PLCIN1_POLARITY     ACTIVE_HIGH

#define PLCIN2_BV           BV(1)
#define PLCIN2_SBIT         P1_1
#define PLCIN2_DDR          P1DIR
#define PLCIN2_POLARITY     ACTIVE_HIGH

#define PLCOUT1_BV           BV(4)
#define PLCOUT1_SBIT         P1_4
#define PLCOUT1_DDR          P1DIR
#define PLCOUT1_POLARITY     ACTIVE_LOW

#define PLCOUT2_BV           BV(5)
#define PLCOUT2_SBIT         P1_5
#define PLCOUT2_DDR          P1DIR
#define PLCOUT2_POLARITY     ACTIVE_LOW

#define HAL_INIT_PLCIN1()		  	 st( PLCIN1_DDR &= ~PLCIN1_BV; )
#define HAL_STATE_PLCIN1()        	 (PLCIN1_POLARITY (PLCIN1_SBIT))

#define HAL_INIT_PLCIN2()		  	 st( PLCIN2_DDR &= ~PLCIN2_BV; )
#define HAL_STATE_PLCIN2()        	 (PLCIN2_POLARITY (PLCIN1_SBIT))

#define HAL_INIT_PLCOUT1()		  	 st( PLCOUT1_DDR |= PLCOUT1_BV; PLCOUT1_SBIT = PLCOUT1_POLARITY (0); )
#define HAL_TURN_OFF_PLCOUT1()       st( PLCOUT1_SBIT = PLCOUT1_POLARITY (0); )
#define HAL_TURN_ON_PLCOUT1()        st( PLCOUT1_SBIT = PLCOUT1_POLARITY (1); )
#define HAL_TOGGLE_PLCOUT1()         st( PLCOUT1_SBIT ^= 1; )
#define HAL_STATE_PLCOUT1()          (PLCOUT1_POLARITY (PLCOUT1_SBIT))

#define HAL_INIT_PLCOUT2()		  	 st( PLCOUT2_DDR |= PLCOUT2_BV; PLCOUT1_SBIT = PLCOUT1_POLARITY (0); )
#define HAL_TURN_OFF_PLCOUT2()       st( PLCOUT2_SBIT = PLCOUT2_POLARITY (0); )
#define HAL_TURN_ON_PLCOUT2()        st( PLCOUT2_SBIT = PLCOUT2_POLARITY (1); )
#define HAL_TOGGLE_PLCOUT2()         st( PLCOUT2_SBIT ^= 1; )
#define HAL_STATE_PLCOUT2()          (PLCOUT2_POLARITY (PLCOUT2_SBIT))

/* Base RFID */
#define RFIDBEE_BV           BV(5)
#define RFIDBEE_SBIT         P1_5
#define RFIDBEE_DDR          P1DIR
#define RFIDBEE_POLARITY     ACTIVE_HIGH

#define HAL_INIT_RFIDBEE()		  	 st( RFIDBEE_DDR |= RFIDBEE_BV; RFIDBEE_SBIT = RFIDBEE_POLARITY (0); )
#define HAL_TURN_OFF_RFIDBEE()       st( RFIDBEE_SBIT = RFIDBEE_POLARITY (0); )
#define HAL_TURN_ON_RFIDBEE()        st( RFIDBEE_SBIT = RFIDBEE_POLARITY (1); )
#define HAL_TOGGLE_RFIDBEE()         st( RFIDBEE_SBIT ^= 1; )
#define HAL_STATE_RFIDBEE()          (RFIDBEE_POLARITY (RFIDBEE_SBIT))

/* Base AirDetect */
#define AIRDETECT_BV           	BV(2)
#define AIRDETECT_SBIT         	P1_2
#define AIRDETECT_DDR          	P1DIR
#define AIRDETECT_POLARITY     	ACTIVE_LOW

#define HAL_INIT_AIRDETECT()		  	 st( AIRDETECT_DDR &= ~AIRDETECT_BV; )
#define HAL_STATE_AIRDETECT()        	 (AIRDETECT_POLARITY (AIRDETECT_SBIT))

#define HAL_LED TRUE
#define BLINK_LEDS

#define HAL_KEY TRUE

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
*********************************************************************/
#endif

#ifdef __cplusplus
}
#endif

#endif /* DEMOBASE_BOARD_CFG_H */
