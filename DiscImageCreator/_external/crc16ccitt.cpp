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
#if 0
// ref
// https://en.wikipedia.org/wiki/Cyclic_redundancy_check
// https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction
#define CRCPOLY2  0x03U  /* x^{6}+x^{1}++1 */
byte ecc_f_lut[64];
byte ecc_b_lut[64];

void MakeCrc6ITUTable(void)
{
	unsigned int i, j, r;

	for (i = 0; i <= 0x3f; i++) {
		r = i;
		for (j = 0; j < 6; j++)
			if (r & 0x20U) r = (r << 1) ^ CRCPOLY2;
			else             r <<= 1;
			ecc_f_lut[i] = r & 0x3FU;
			ecc_b_lut[i ^ r] = i;
	}
}

char ecc6_checkpq(
	const byte* data,
	size_t major_count,
	size_t minor_count,
	size_t major_mult,
	size_t minor_inc,
	const byte* ecc
) {
	size_t size = major_count * minor_count;
	size_t major;
	for (major = 0; major < major_count; major++) {
		size_t index = (major >> 1) * major_mult + (major & 1);
		byte ecc_a = 0;
		byte ecc_b = 0;
		size_t minor;
		for (minor = 0; minor < minor_count; minor++) {
			byte temp = data[index];
			index += minor_inc;
			if (index >= size) {
				index -= size;
			}
			ecc_a ^= temp;
			ecc_b ^= temp;
			ecc_a = ecc_f_lut[ecc_a];
		}
		ecc_a = ecc_b_lut[ecc_f_lut[ecc_a] ^ ecc_b];
		if (
			ecc[major] != (ecc_a) ||
			ecc[major + major_count] != (ecc_a ^ ecc_b)
			) {
			return 0;
		}
	}
	return 1;
}
#endif
#pragma warning(pop)
