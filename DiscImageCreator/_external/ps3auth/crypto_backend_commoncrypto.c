#include "crypto_backend.h"
#include <CommonCrypto/CommonCryptor.h>
#include <string.h>

/*
 * CBC / NoPadding
 * CCCrypt uses CBC unless kCCOptionECBMode is set and pads only with kCCOptionPKCS7Padding.
 */
static int CBCNoPad_CommonCrypto(
	CCOperation op,
	CCAlgorithm alg,
	size_t block_size,
	const unsigned char* key,
	size_t key_len,
	const unsigned char* iv,
	const unsigned char* src,
	size_t len,
	unsigned char* dst
) {
	if (!iv || !src || !dst) {
		return -1;
	}
	if (len == 0 || (len % block_size) != 0) {
		return -1;
	}

	size_t moved = 0;
	if (CCCrypt(op, alg, 0, key, key_len, iv, src, len, dst, len, &moved) != kCCSuccess) {
		return -1;
	}
	return 0;
}

/*
 * 2-key 3DES (112bit) / CBC / NoPadding
 * - key16: 16 bytes (K1 || K2)
 * - iv8  : 8 bytes
 * CommonCrypto only takes 3-key 3DES, and K1 || K2 || K1 is the same cipher.
 */
static int TripleDES_CommonCrypto(
	CCOperation op,
	const unsigned char* key16,
	const unsigned char* iv8,
	const unsigned char* src,
	size_t len,
	unsigned char* dst
) {
	if (!key16) {
		return -1;
	}
	unsigned char key24[kCCKeySize3DES];
	memcpy(key24, key16, 16);
	memcpy(key24 + 16, key16, 8);
	return CBCNoPad_CommonCrypto(op, kCCAlgorithm3DES, kCCBlockSize3DES, key24, sizeof(key24), iv8, src, len, dst);
}

static int exec_aes128_cbc_nopad_enc(
	void* u,
	const uint8_t key[16],
	const uint8_t iv[16],
	const uint8_t* in,
	size_t in_len,
	uint8_t* out,
	size_t out_cap,
	size_t* out_len
) {
	(void)u;
	if (!out_len) {
		return -1;
	}
	if (out_cap < in_len) {
		return -1;
	}
	if (CBCNoPad_CommonCrypto(kCCEncrypt, kCCAlgorithmAES, kCCBlockSizeAES128, key, kCCKeySizeAES128, iv, in, in_len, out) != 0) {
		return -1;
	}
	*out_len = in_len;
	return 0;
}

static int exec_aes128_cbc_nopad_dec(
	void* u,
	const uint8_t key[16],
	const uint8_t iv[16],
	const uint8_t* in,
	size_t in_len,
	uint8_t* out,
	size_t out_cap,
	size_t* out_len
) {
	(void)u;
	if (!out_len) {
		return -1;
	}
	if (out_cap < in_len) {
		return -1;
	}
	if (CBCNoPad_CommonCrypto(kCCDecrypt, kCCAlgorithmAES, kCCBlockSizeAES128, key, kCCKeySizeAES128, iv, in, in_len, out) != 0) {
		return -1;
	}
	*out_len = in_len;
	return 0;
}

static int exec_tdes_ede_cbc_nopad_enc(
	void* u,
	const uint8_t key[16],
	const uint8_t iv[8],
	const uint8_t* in,
	size_t in_len,
	uint8_t* out,
	size_t out_cap,
	size_t* out_len
) {
	(void)u;
	if (!out_len) {
		return -1;
	}
	if (out_cap < in_len) {
		return -1;
	}
	if (TripleDES_CommonCrypto(kCCEncrypt, key, iv, in, in_len, out) != 0) {
		return -1;
	}
	*out_len = in_len;
	return 0;
}

static int exec_tdes_ede_cbc_nopad_dec(
	void* u,
	const uint8_t key[16],
	const uint8_t iv[8],
	const uint8_t* in,
	size_t in_len,
	uint8_t* out,
	size_t out_cap,
	size_t* out_len
) {
	(void)u;
	if (!out_len) {
		return -1;
	}
	if (out_cap < in_len) {
		return -1;
	}
	if (TripleDES_CommonCrypto(kCCDecrypt, key, iv, in, in_len, out) != 0) {
		return -1;
	}
	*out_len = in_len;
	return 0;
}

crypto_backend crypto_backend_make(void)
{
	static const crypto_backend_vtbl CRYPTO_VTBL = {
		.aes128_cbc_nopad_enc = exec_aes128_cbc_nopad_enc,
		.aes128_cbc_nopad_dec = exec_aes128_cbc_nopad_dec,
		.tdes_ede_cbc_nopad_enc = exec_tdes_ede_cbc_nopad_enc,
		.tdes_ede_cbc_nopad_dec = exec_tdes_ede_cbc_nopad_dec
	};
	crypto_backend c = {
		.user = NULL,
		.v = &CRYPTO_VTBL,
	};
	return c;
}
