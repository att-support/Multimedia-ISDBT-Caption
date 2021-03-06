//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CCRC_16.h"

#include <limits.h>
#define CRCPOLY1  0x1021U  /* x^{16}+x^{12}+x^5+1 */
#define CRCPOLY2  0x8408U  /* 左右逆転 */
unsigned short CCRC_16::Calc(unsigned char *buf,unsigned long length)
{
	unsigned int n = length;
	unsigned char *c = buf;
	unsigned int i, j, r, t;
	unsigned short dCRC16=0x0000;

	r = 0x0000;
	for (i = 0; i < n; i++) {
		r ^= (unsigned int)c[i] << (16 - CHAR_BIT);
		for (j = 0; j < CHAR_BIT; j++)
			if (r & 0x8000) r = (r << 1) ^ CRCPOLY1;
			else             r <<= 1;
	}
	t = ~r;
	dCRC16 = r;
	return dCRC16;
}

