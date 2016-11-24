#define ACTIVE_LOW        !
#define ACTIVE_HIGH       !!    /* double negation forces result to be '1' */

#define HAL_UART_DMA_DISABLE_RTCT	/*串口RT , CT 引脚无效*/
#define HOLD_INIT_AUTHENTICATION	/* 自定义初始化认证方式 */

#include "DemoBase_Board_cfg.h"
#include "hal_board_cfg.h"
