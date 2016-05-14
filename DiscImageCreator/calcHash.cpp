/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "calcHash.h"

VOID CalcInit(
	MD5_CTX* context,
	SHA1Context* sha
	)
{
	// init md5
	MD5Init(context);
	// init sha1
	SHA1Reset(sha);
}

BOOL CalcHash(
	LPDWORD crc,
	MD5_CTX* context,
	SHA1Context* sha,
	LPBYTE lpBuf,
	DWORD dwSize
	)
{
	BOOL bRet = TRUE;
	/* Return the CRC of the bytes buf[0..len-1]. */
	*crc = update_crc(*crc, lpBuf, (INT)dwSize);
	// calc md5
	MD5Update(context, lpBuf, dwSize);
	// calc sha1
	int err = SHA1Input(sha, lpBuf, dwSize);
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
	)
{
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
