/**
 * Copyright 2011-2024 sarami
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

VOID CalcInit(
	PEXT_ARG pExtArg,
	PHASH_CHUNK pHash
) {
	// init md5
	MD5Init(&pHash->md5);
	// init sha1
	SHA1Reset(&pHash->sha);

	if (pExtArg->byDatExpand) {
		SHA224Reset(&pHash->sha224);
		SHA256Reset(&pHash->sha256);
		SHA384Reset(&pHash->sha384);
		SHA512Reset(&pHash->sha512);
		pHash->xxh3_64 = XXH3_createState();
		pHash->xxh3_128 = XXH3_createState();
		XXH3_64bits_reset(pHash->xxh3_64);
		XXH3_128bits_reset(pHash->xxh3_128);
	}
}

BOOL CalcHash(
	PEXT_ARG pExtArg,
	PHASH_CHUNK pHash,
	LPBYTE lpBuf,
	UINT uiSize
) {
	/* Return the CRC of the bytes buf[0..len-1]. */
	pHash->crc32 = update_crc(pHash->crc32, lpBuf, (INT)uiSize);
	// calc md5
	MD5Update(&pHash->md5, lpBuf, uiSize);
	// calc sha1
	int err = SHA1Input(&pHash->sha, lpBuf, uiSize);
	if (err) {
		fprintf(stderr, "SHA1Input Error %d.\n", err);
		return FALSE;
	}
	if (pExtArg->byDatExpand) {
		err = SHA224Input(&pHash->sha224, lpBuf, uiSize);
		if (err) {
			fprintf(stderr, "SHA224Input Error %d.\n", err);
			return FALSE;
		}
		err = SHA256Input(&pHash->sha256, lpBuf, uiSize);
		if (err) {
			fprintf(stderr, "SHA256Input Error %d.\n", err);
			return FALSE;
		}
		err = SHA384Input(&pHash->sha384, lpBuf, uiSize);
		if (err) {
			fprintf(stderr, "SHA384Input Error %d.\n", err);
			return FALSE;
		}
		err = SHA512Input(&pHash->sha512, lpBuf, uiSize);
		if (err) {
			fprintf(stderr, "SHA512Input Error %d.\n", err);
			return FALSE;
		}
		XXH3_64bits_update(pHash->xxh3_64, lpBuf, uiSize);
		XXH3_128bits_update(pHash->xxh3_128, lpBuf, uiSize);
	}
	return TRUE;
}

BOOL CalcEnd(
	PEXT_ARG pExtArg,
	PHASH_CHUNK pHash,
	PMESSAGE_DIGEST_CHUNK pDigest
) {
	// end md5
	MD5Final(pDigest->md5, &pHash->md5);
	// end sha1
	int err = SHA1Result(&pHash->sha, pDigest->sha);
	if (err) {
		fprintf(stderr, "SHA1Result Error %d, could not compute message digest.\n", err);
		return FALSE;
	}
	if (pExtArg->byDatExpand) {
		err = SHA224Result(&pHash->sha224, pDigest->sha224);
		if (err) {
			fprintf(stderr, "SHA224Result Error %d, could not compute message digest.\n", err);
			return FALSE;
		}
		err = SHA256Result(&pHash->sha256, pDigest->sha256);
		if (err) {
			fprintf(stderr, "SHA256Result Error %d, could not compute message digest.\n", err);
			return FALSE;
		}
		err = SHA384Result(&pHash->sha384, pDigest->sha384);
		if (err) {
			fprintf(stderr, "SHA224Result Error %d, could not compute message digest.\n", err);
			return FALSE;
		}
		err = SHA512Result(&pHash->sha512, pDigest->sha512);
		if (err) {
			fprintf(stderr, "SHA512Result Error %d, could not compute message digest.\n", err);
			return FALSE;
		}
		pDigest->xxh3_64 = XXH3_64bits_digest(pHash->xxh3_64);
		pDigest->xxh3_128 = XXH3_128bits_digest(pHash->xxh3_128);
		XXH3_freeState(pHash->xxh3_64);
		XXH3_freeState(pHash->xxh3_128);
	}
	return TRUE;
}
