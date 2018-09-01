/******************************************************************************

abgx360.c

The ultimate tool for Xbox 360 ISOs and Stealth files!

Copyright 2008-2012 by Seacrest <Seacrest[at]abgx360[dot]net>

******************************************************************************/
#pragma warning(disable:4464)
#include "abgx360.h"
#include "../output.h"

void decryptChallengeResponse(unsigned char* dcrt, unsigned char* ss)
{
	int i, j, Nr;
	u32 rk[4 * (MAXNR + 1)];
	const u8 cipherKey[16] = { 0xD1,0xE3,0xB3,0x3A,0x6C,0x1E,0xF7,0x70,0x5F,0x6D,0xE9,0x3B,0xB6,0xC0,0xDC,0x71 };
	u8 ct[16], pt[16], ivec[16];
	memset(ivec, 0, 16);
	Nr = rijndaelKeySetupDec(rk, cipherKey, 128);
	for (i = 0; i < 15; i++) {
		memcpy(ct, ss + (0x304 + (i * 16)), 16);
		rijndaelDecrypt(rk, Nr, ct, pt);
		for (j = 0; j < 16; j++) pt[j] ^= ivec[j];
		memcpy(ivec, ct, 16);
		memcpy(dcrt + (i * 16), pt, 16);
	}
}

void printwin32filetime(unsigned long long win32filetime, char* date) {
	int month, leap, year = 1601;
	unsigned long long seconds = win32filetime / 10000000;
	unsigned long long minutes = seconds / 60;
	unsigned long long hours = minutes / 60;
	long days = (long)(hours / 24);
	while (days > 364) {
		if (year % 400 == 0) days -= 366;
		else if (year % 100 == 0) days -= 365;
		else if (year % 4 == 0) days -= 366;
		else days -= 365;
		year++;
	}
	// days == -1 means the while loop executed during a leap year with days=365
	if (days == -1) {
		_snprintf(date, 20, "%u/12/31 %02llu:%02llu:%02llu", year - 1, hours % 24, minutes % 60, seconds % 60);
	}
	else {
		// days++ makes it more natural (days=1 is now Jan 1 and Feb 1 is days=32)
		days++;
		// check if year is a leap year
		if (year % 400 == 0) leap = 1;
		else if (year % 100 == 0) leap = 0;
		else if (year % 4 == 0) leap = 1;
		else leap = 0;
		// find out what month it is and subtract days from the previous months
		if (days > 334 + leap) { days -= 334 + leap; month = 12; }
		else if (days > 304 + leap) { days -= 304 + leap; month = 11; }
		else if (days > 273 + leap) { days -= 273 + leap; month = 10; }
		else if (days > 243 + leap) { days -= 243 + leap; month = 9; }
		else if (days > 212 + leap) { days -= 212 + leap; month = 8; }
		else if (days > 181 + leap) { days -= 181 + leap; month = 7; }
		else if (days > 151 + leap) { days -= 151 + leap; month = 6; }
		else if (days > 120 + leap) { days -= 120 + leap; month = 5; }
		else if (days > 90 + leap) { days -= 90 + leap;  month = 4; }
		else if (days > 59 + leap) { days -= 59 + leap;  month = 3; }
		else if (days > 31) { days -= 31;       month = 2; }
		else month = 1;
		_snprintf(date, 20, "%u/%02i/%02lu %02llu:%02llu:%02llu", year, month, days, hours % 24, minutes % 60, seconds % 60);
	}
	return;
}
