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
#pragma once
#include "_external/crc16ccitt.h"
#include "_external/crc32.h"
#include "_external/md5.h"
#include "_external/sha1.h"

VOID CalcInit(
	MD5_CTX* context,
	SHA1Context* sha
);

WORD GetCrc16CCITT(
	INT len,
	LPBYTE lpBuf
);

VOID GetCrc32(
	LPDWORD crc,
	LPBYTE lpBuf,
	DWORD dwSize
);

BOOL CalcHash(
	LPDWORD crc,
	MD5_CTX* context,
	SHA1Context* sha,
	LPBYTE lpBuf,
	UINT uiSize
);

BOOL CalcEnd(
	MD5_CTX* context,
	SHA1Context* sha,
	LPBYTE digest,
	LPBYTE Message_Digest
);
