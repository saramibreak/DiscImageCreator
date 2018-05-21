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
#include "crc32ecma267.h"

unsigned long crc32_table[256];

// 0x80000011  /* x^32 + x^31 + x^4 + 1 */
void make_crc32ecma267_table(void)
{
	for (unsigned long n = 0; n < 256; n++) {
		unsigned long c = n << 24;
		for (unsigned long k = 0; k < 8; k++) {
			c = (c << 1) ^ ((c & 0x80000000) ? 0x80000011 : 0);
		}
		crc32_table[n] = c;
	}
#if 0
	for (int i = 0; i < 256; i += 8) {
		for (int k = 0; k < 8; k++) {
			OutputString("%08x, ", crc32_table[k + i]);
		}
		OutputString("\n");
	}
#endif
}

unsigned long update_crc32ecma267(unsigned long crc, unsigned char *buf, int len) {
	unsigned long c = crc;
	for (int i = 0; i < len; i++) {
		c = (c << 8) ^ crc32_table[((c >> 24) ^ buf[i]) & 0xff];
	}
	return c;
}
