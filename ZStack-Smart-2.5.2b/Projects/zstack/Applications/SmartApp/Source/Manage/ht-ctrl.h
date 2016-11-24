#ifndef __HT_CTRL_H__
#define __HT_CTRL_H__
#include "hal_board.h"

typedef union 
{ uint16 i;
  float f;
}HTValue;

//----------------------------------------------------------------------------------
// modul-var
//----------------------------------------------------------------------------------
enum {TEMP,HUMI};

void s_connectionreset(void);
char s_measure(uint8 *p_value, uint8 *p_checksum, uint8 mode);
void calc_sht11(float *p_humidity ,float *p_temperature);

#endif