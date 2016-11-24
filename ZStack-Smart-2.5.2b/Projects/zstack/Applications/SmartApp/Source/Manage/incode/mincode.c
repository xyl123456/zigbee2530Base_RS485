/*
 * incode.c
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
#include "mincode.h"

uint8 incode_ctox(uint8 src)
{
	char temp = 0;

	if(src>='0' && src<='9')
		temp = src-'0';
	else if(src>='a' && src<='f')
		temp = src-'a'+10;
	else if(src>='A' && src<='F')
		temp = src-'A'+10;

	return temp;
}

uint8 incode_atox(uint8 *src, int len)
{
	char temp=0, i=0, length;
	length = len;

	while(length--)
	{
		temp = incode_ctox(*(src+i)) + (temp<<4);
		i++;
	}

	return temp;
}
 
void incode_ctoxs(uint8 *dest ,uint8 *src, int len)
{
	int i, temp;
	if(len<2 || dest==NULL || src==NULL)
	{
		return;
	}

	for(i=0; i<(len>>1); i++)
	{
		temp = *(src+(i<<1));
		dest[i] = (incode_ctox(temp)<<4);
		
		temp = *(src+(i<<1)+1);
		dest[i] += incode_ctox(temp);
	}
	 
}

void incode_xtocs(uint8 *dest , uint8 *src, int len)
{
    int i, temp;
	if(len<1 || dest==NULL || src==NULL)
	{
		return;
	}

	for(i=0; i<len; i++)
	{
		temp = (*(src+i)>>4);
		if(temp < 0xA)
		{
			dest[(i<<1)] = temp + '0';	
		}
		else
		{
			dest[(i<<1)] = temp - 0xA + 'A';	
		}
		
		temp = (*(src+i)&0x0F);
		if(temp < 0xA)
		{
			dest[(i<<1)+1] = temp + '0';	
		}
		else
		{
			dest[(i<<1)+1] = temp - 0xA + 'A';	
		}
	}
}


void incode_ctox16(uint16 *dest, uint8 *src)
{
	unsigned char dsts[2];
	incode_ctoxs(dsts, src, 4);
	*dest = dsts[0]<<8;
	*dest += dsts[1];
}

void incode_xtoc16(uint8 *dest, uint16 src)
{
	unsigned char val[2];
	val[0] = (src>>8);
	val[1] = (src&0xFF);
	incode_xtocs(dest, val, 2);
}

void incode_ctox32(uint32 *dest, uint8 *src)
{
	int i;
	uint8 dsts[4];
	incode_ctoxs(dsts, src, 8);

	*dest = 0;
	for(i=0; i<4; i++)
	{
		*dest += ((*dest)<<8)+dsts[i];
	}
}

void incode_xtoc32(uint8 *dest, uint32 src)
{
	unsigned char val[4];
	val[0] = ((src>>24)&0xFF);
	val[1] = ((src>>16)&0xFF);
	val[2] = ((src>>8)&0xFF);
	val[3] = (src&0xFF);
	
	incode_xtocs(dest, val, 4);
}

uint32 gen_rand(uint8 *seed)
{
	int i;
	unsigned int ra = 0;
	for(i=0; i<8; i+=2)
	{
		ra += seed[i]<<(i<<2);
	}

	return ra;
}