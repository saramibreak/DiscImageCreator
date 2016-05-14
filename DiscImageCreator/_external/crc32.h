/*
 * This code is using "RFC 1952 GZIP File Format Specification."
 */
#pragma once

void make_crc_table(void);

unsigned long update_crc(
	unsigned long crc,
	unsigned char* buf,
	int len
	);
