#pragma once
typedef unsigned char byte;

void MakeCrc16CCITTTable(void);
unsigned int GetCrc16CCITT(int n, byte c[]);
#if 0
void MakeCrc6ITUTable(void);
char ecc6_checkpq(
	const byte* data,
	size_t major_count,
	size_t minor_count,
	size_t major_mult,
	size_t minor_inc,
	const byte* ecc
);
#endif