/**
 * Copyright 2011-2018 sarami
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "crc6itu.h"

unsigned long crc6_table[256];

// 0x03  /* x^6 + x^1 + 1 */
void make_crc6itu_table(void)
{
	for (unsigned char n = 0; n < 64; n++) {
		unsigned char c = n;
		for (unsigned char k = 0; k < 6; k++) {
			c = (unsigned char)((c << 1) ^ ((c & 0x20) ? 0x03 : 0));
		}
		crc6_table[n] = c;
	}
#if 0
	for (int i = 0; i < 64; i += 8) {
		for (k = 0; k < 8; k++) {
			OutputString("%02x, ", crc6_table[k + i]);
		}
		OutputString("\n");
	}
#endif
}

unsigned long update_crc6(unsigned char crc, unsigned char *buf, int len) {
	unsigned long c = crc;
	for (int i = 0; i < len; i++) {
		c = (c << 8) ^ crc6_table[(c ^ buf[i]) & 0xff];
	}
	return c;
}
