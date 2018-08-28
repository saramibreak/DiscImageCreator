/*
 * Homepage: http://oku.edu.mie-u.ac.jp/~okumura/algo/
 * Original code is "src\crc16t.c" in algo.lzh
 * Copyright (c) 1991 Haruhiko Okumura
 */

#pragma warning(push)
#pragma warning(disable:4018 4068)
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#include "crc16ccitt.h"

/***********************************************************
	crc16t.c -- CRC
***********************************************************/
#include <limits.h>
#define CRCPOLY1  0x1021U  /* x^{16}+x^{12}+x^5+1 */
unsigned int crctable[UCHAR_MAX + 1];

void make_crc16_table(void)
{
	unsigned int i, j, r;

	for (i = 0; i <= UCHAR_MAX; i++) {
		r = i << (16 - CHAR_BIT);
		for (j = 0; j < CHAR_BIT; j++)
			if (r & 0x8000U) r = (r << 1) ^ CRCPOLY1;
			else             r <<= 1;
			crctable[i] = r & 0xFFFFU;
	}
}

unsigned int update_crc16(int n, byte c[])
{
	unsigned int r;

//	r = 0xFFFFU;
	r = 0;
	while (--n >= 0)
		r = (r << CHAR_BIT) ^ crctable[(byte)(r >> (16 - CHAR_BIT)) ^ *c++];
	return ~r & 0xFFFFU;
}
#pragma warning(pop)
