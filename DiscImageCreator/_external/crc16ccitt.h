#pragma once
typedef unsigned char byte;

void make_crc16_table(void);
unsigned int update_crc16(int n, byte c[]);
