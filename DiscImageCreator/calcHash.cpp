/**
 * Copyright 2011-2021 sarami
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

VOID CalcInitExpand(
	SHA224Context* sha224,
	SHA256Context* sha256,
	SHA384Context* sha384,
	SHA512Context* sha512
) {
	SHA224Reset(sha224);
	SHA256Reset(sha256);
	SHA384Reset(sha384);
	SHA512Reset(sha512);
}

BOOL CalcHashExpand(
	SHA224Context* sha224,
	SHA256Context* sha256,
	SHA384Context* sha384,
	SHA512Context* sha512,
	LPBYTE lpBuf,
	UINT uiSize
) {
	int err = SHA224Input(sha224, lpBuf, uiSize);
	if (err) {
		fprintf(stderr, "SHA224Input Error %d.\n", err);
		return FALSE;
	}
	err = SHA256Input(sha256, lpBuf, uiSize);
	if (err) {
		fprintf(stderr, "SHA256Input Error %d.\n", err);
		return FALSE;
	}
	err = SHA384Input(sha384, lpBuf, uiSize);
	if (err) {
		fprintf(stderr, "SHA384Input Error %d.\n", err);
		return FALSE;
	}
	err = SHA512Input(sha512, lpBuf, uiSize);
	if (err) {
		fprintf(stderr, "SHA512Input Error %d.\n", err);
		return FALSE;
	}
	return TRUE;
}

BOOL CalcEndExpand(
	SHA224Context* sha224,
	SHA256Context* sha256,
	SHA384Context* sha384,
	SHA512Context* sha512,
	LPBYTE Message_Digest224,
	LPBYTE Message_Digest256,
	LPBYTE Message_Digest384,
	LPBYTE Message_Digest512
) {
	int err = SHA224Result(sha224, Message_Digest224);
	if (err) {
		fprintf(stderr, "SHA224Result Error %d, could not compute message digest.\n", err);
		return FALSE;
	}
	err = SHA256Result(sha256, Message_Digest256);
	if (err) {
		fprintf(stderr, "SHA256Result Error %d, could not compute message digest.\n", err);
		return FALSE;
	}
	err = SHA384Result(sha384, Message_Digest384);
	if (err) {
		fprintf(stderr, "SHA224Result Error %d, could not compute message digest.\n", err);
		return FALSE;
	}
	err = SHA512Result(sha512, Message_Digest512);
	if (err) {
		fprintf(stderr, "SHA512Result Error %d, could not compute message digest.\n", err);
		return FALSE;
	}
	return TRUE;

}
