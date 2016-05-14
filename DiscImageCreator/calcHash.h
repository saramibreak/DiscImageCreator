/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once
#include "_external/crc16ccitt.h"
#include "_external/crc32.h"
#include "_external/md5.h"
#include "_external/sha1.h"

VOID CalcInit(
	MD5_CTX* context,
	SHA1Context* sha
	);

BOOL CalcHash(
	LPDWORD crc,
	MD5_CTX* context,
	SHA1Context* sha,
	LPBYTE lpBuf,
	DWORD dwSize
	);

BOOL CalcEnd(
	MD5_CTX* context,
	SHA1Context* sha,
	LPBYTE digest,
	LPBYTE Message_Digest
	);
