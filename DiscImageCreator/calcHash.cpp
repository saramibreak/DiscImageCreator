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
#include "calcHash.h"

VOID CalcInit(
	MD5_CTX* context,
	SHA1Context* sha
) {
	// init md5
	MD5Init(context);
	// init sha1
	SHA1Reset(sha);
}

WORD GetCrc16CCITT(
	INT len,
	LPBYTE lpBuf
) {
	return (WORD)update_crc16(len, lpBuf);
}

VOID GetCrc32(
	LPDWORD crc,
	LPBYTE lpBuf,
	DWORD dwSize
) {
	*crc = update_crc(*crc, lpBuf, (INT)dwSize);
}

BOOL CalcHash(
	LPDWORD crc,
	MD5_CTX* context,
	SHA1Context* sha,
	LPBYTE lpBuf,
	UINT uiSize
) {
	BOOL bRet = TRUE;
	/* Return the CRC of the bytes buf[0..len-1]. */
	*crc = update_crc(*crc, lpBuf, (INT)uiSize);
	// calc md5
	MD5Update(context, lpBuf, uiSize);
	// calc sha1
	int err = SHA1Input(sha, lpBuf, uiSize);
	if (err)	{
		fprintf(stderr, "SHA1Input Error %d.\n", err);
		bRet = FALSE;
	}
	return bRet;
}

BOOL CalcEnd(
	MD5_CTX* context,
	SHA1Context* sha,
	LPBYTE digest,
	LPBYTE Message_Digest
) {
	BOOL bRet = TRUE;
	// end md5
	MD5Final(digest, context);
	// end sha1
	int err = SHA1Result(sha, Message_Digest);
	if (err) {
		fprintf(stderr,	"SHA1Result Error %d, could not compute message digest.\n",	err);
		bRet = FALSE;
	}
	return bRet;
}
