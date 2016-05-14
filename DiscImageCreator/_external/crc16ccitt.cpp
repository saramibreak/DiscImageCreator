/*
 * Original code is "src\crc16t.c" in algo.lzh
 * homepage: http://oku.edu.mie-u.ac.jp/~okumura/algo/
 */

#pragma warning(push)
#pragma warning(disable:4018)
#include "crc16ccitt.h"

/***********************************************************
	crc16t.c -- CRC
***********************************************************/
#include <limits.h>
#define CRCPOLY1  0x1021U  /* x^{16}+x^{12}+x^5+1 */
unsigned int crctable[UCHAR_MAX + 1];

void MakeCrc16CCITTTable(void)
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

unsigned int GetCrc16CCITT(int n, byte c[])
{
	unsigned int r;

//	r = 0xFFFFU;
	r = 0;
	while (--n >= 0)
		r = (r << CHAR_BIT) ^ crctable[(byte)(r >> (16 - CHAR_BIT)) ^ *c++];
	return ~r & 0xFFFFU;
}
#if 0
byte crc6_lut[256];

void MakeCrc6ITUTable(void)
{
	size_t i;
	for (i = 0; i < 256; i++) {
		size_t j;
		for (byte bit = 8; bit > 0; --bit) {
			j = (i << 1) ^ (i & 0x80 ? 0x03 : 0);
		}
		crc6_lut[i] = j;
	}
}

byte GetCrc6ITU(int n, byte c[], int i)
{
	unsigned char tmp = c[i];
	tmp = c[i] ^ crc6_lut[tmp];
	return crc6_lut[tmp];
}
#endif
#pragma warning(pop)
