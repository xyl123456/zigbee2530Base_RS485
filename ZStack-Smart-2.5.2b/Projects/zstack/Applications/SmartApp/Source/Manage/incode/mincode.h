/*
 * incode.h
 *
 * Copyright (C) 2013 loongsky development.
 *
 * Sam Chen <xuejian1354@163.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
 
#ifndef __MINCODE_H__
#define __MINCODE_H__

#include <stdio.h>
#include "hal_types.h"

uint8 incode_ctox(uint8 src);
uint8 incode_atox(uint8 *src, int len);
void incode_ctoxs(uint8 *dest , uint8 *src, int len);
void incode_xtocs(uint8 *dest , uint8 *src, int len);
void incode_ctox16(uint16 *dest, uint8 *src);
void incode_xtoc16(uint8 *dest, uint16 src);
void incode_ctox32(uint32 *dest, uint8 *src);
void incode_xtoc32(uint8 *dest, uint32 src);
uint32 gen_rand(uint8 *seed);

#endif //  __MINCODE_H __
