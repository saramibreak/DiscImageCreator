#pragma once
typedef unsigned char byte;

void MakeCrc16CCITTTable(void);
unsigned int GetCrc16CCITT(int n, byte c[]);
#if 0
void MakeCrc6ITUTable(void);
byte GetCrc6ITU(int n, byte c[], int i);
#endif