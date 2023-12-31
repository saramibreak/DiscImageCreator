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
#include "struct.h"
#include "calcHash.h"
#include "check.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforDVD.h"
#include "get.h"
#include "init.h"
#include "output.h"
#include "xml.h"
#include "_external/prngcd.h"
#include "_external/xxhash.h"

#define ZERO_BYTE_CHUNK 2352

BOOL GetLastNonZeroPosition(
	DWORD dwBytesPerSector,
	UINT64 uiFileSize,
	FILE* fp,
	LPBYTE data,
	UINT64 ui64SectorSizeAll,
	LPUINT uiLastNonZeroSectorRear,
	LPUINT uiLastNonZeroSectorPosRear,
	LPUINT uiLastNonZeroSectorFront,
	LPUINT uiLastNonZeroSectorPosFront
) {
	BOOL bLastNonZeroRear = FALSE;
	BOOL bLastNonZeroFront = FALSE;
	INT nConsecutiveZeroCnt = 0;

	for (UINT64 j = 1; j <= ui64SectorSizeAll; j++) {
		fseek(fp, (LONG)(uiFileSize - dwBytesPerSector * j), SEEK_SET);
		if (fread(data, sizeof(BYTE), dwBytesPerSector, fp) < dwBytesPerSector) {
			OutputErrorString("Failed to read: read size %lu [F:%s][L:%d]\n", dwBytesPerSector, _T(__FUNCTION__), __LINE__);
			FcloseAndNull(fp);
			return FALSE;
		};
		for (INT k = (INT)(dwBytesPerSector - 1); 0 <= k; k--) {
			if (!bLastNonZeroRear) {
				if (data[k] != 0) {
					UINT ofs = UINT(k + 1);
					UINT mod = ofs % BYTE_SIZE_PER_SAMPLE;
					if (mod == 0) {
						*uiLastNonZeroSectorPosRear = ofs;
					}
					else {
						*uiLastNonZeroSectorPosRear = (ofs + (BYTE_SIZE_PER_SAMPLE - mod));
					}
					*uiLastNonZeroSectorRear = (UINT)(ui64SectorSizeAll - j);
					bLastNonZeroRear = TRUE;
				}
			}
			else if (bLastNonZeroRear) {
				if (data[k] == 0) {
					++nConsecutiveZeroCnt;
				}
				else if (nConsecutiveZeroCnt < ZERO_BYTE_CHUNK) {
					nConsecutiveZeroCnt = 0;
				}

				if (ZERO_BYTE_CHUNK == nConsecutiveZeroCnt) {
					INT ofs = k + nConsecutiveZeroCnt;
					UINT64 nSector = j;
					if (ofs >= CD_RAW_SECTOR_SIZE) {
						ofs -= CD_RAW_SECTOR_SIZE;
						nSector--;
					}
					*uiLastNonZeroSectorPosFront = (UINT)(ofs - ofs % BYTE_SIZE_PER_SAMPLE);
					*uiLastNonZeroSectorFront = (UINT)(ui64SectorSizeAll - nSector);
					bLastNonZeroFront = TRUE;
					break;
				}
			}
		}
		if (bLastNonZeroFront) {
			break;
		}
	}
	rewind(fp);
	return TRUE;
}

BOOL Get1stNonZeroPositionFront(
	PEXT_ARG pExtArg,
	DWORD dwBytesPerSector,
	PHASH_CHUNK hash,
	LPBOOL b1stNonZero,
	UINT cnt,
	LPBYTE data,
	LPINT nConsecutiveZeroCnt,
	LPUINT ui1stNonZeroSector,
	LPUINT ui1stNonZeroSectorPos,
	BOOL bGetHash
) {
	INT nRet = FALSE;
	for (UINT k = 0; k < dwBytesPerSector; k++) {
		if (*b1stNonZero) {
			if (data[k] == 0) {
				++(*nConsecutiveZeroCnt);
			}
			else if (*nConsecutiveZeroCnt < ZERO_BYTE_CHUNK) {
				*nConsecutiveZeroCnt = 0;
			}
		}
		else {
			if (data[k] != 0) {
				UINT ofs = (k - (k % BYTE_SIZE_PER_SAMPLE));
				*b1stNonZero = TRUE;
				if (bGetHash) {
					nRet = CalcHash(pExtArg, hash, data + ofs, (UINT)(dwBytesPerSector - ofs));
					if (!nRet) {
						return FALSE;
					}
				}
				*ui1stNonZeroSector = cnt;
				*ui1stNonZeroSectorPos = ofs;
			}
		}
	}
	return TRUE;
}

BOOL Get1stNonZeroPositionRear(
	DWORD dwBytesPerSector,
	LPBOOL b1stNonZeroRear,
	INT cnt,
	LPBYTE data,
	LPINT nConsecutiveZeroCnt,
	LPUINT ui1stNonZeroSectorRear,
	LPUINT ui1stNonZeroSectorPosRear
) {
	for (INT j = 0; j < (INT)dwBytesPerSector; j++) {
		if (data[j] == 0) {
			++(*nConsecutiveZeroCnt);
		}
		else if (*nConsecutiveZeroCnt < ZERO_BYTE_CHUNK) {
			*nConsecutiveZeroCnt = 0;
		}

		if (ZERO_BYTE_CHUNK <= *nConsecutiveZeroCnt) {
			INT nTmpPos = j - *nConsecutiveZeroCnt;
			INT nTmpSector = cnt;
			UINT ofs = (UINT)nTmpPos;
			if (nTmpPos < 0) {
				ofs = (UINT)dwBytesPerSector + nTmpPos;
				if (0 < cnt) {
					nTmpSector--;
				}
			}
			UINT mod = ofs % BYTE_SIZE_PER_SAMPLE;
			if (mod == 0) {
				*ui1stNonZeroSectorPosRear = ofs;
			}
			else {
				*ui1stNonZeroSectorPosRear = (ofs + (BYTE_SIZE_PER_SAMPLE - mod));
			}
			*ui1stNonZeroSectorRear = (UINT)nTmpSector;
			*b1stNonZeroRear = TRUE;
			break;
		}
	}
	return TRUE;
}

BOOL OutputHash(
	PEXT_ARG pExtArg,
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
#else
	XMLElement* pWriter,
#endif
	PHASH_CHUNK pHash,
	PMESSAGE_DIGEST_CHUNK pDigest
) {
#ifdef _WIN32
	HRESULT hr = S_OK;
	if (FAILED(hr = pWriter->WriteStartElement(NULL, L"rom", NULL))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	WCHAR wszFnameAndExt[_MAX_FNAME + _MAX_EXT] = {};
#ifndef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0
		, pHash->szFnameAndExt, SIZE_OF_ARRAY(pHash->szFnameAndExt)
		, wszFnameAndExt, SIZE_OF_ARRAY(wszFnameAndExt))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#else
	size_t size = SIZE_OF_ARRAY(wszFnameAndExt));
	wcsncpy(wszFnameAndExt, szFnameAndExt, size);
	wszFnameAndExt[size - 1] = 0;
#endif
	if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"name", NULL, wszFnameAndExt))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	WCHAR buf[128] = {};
	_snwprintf(buf, SIZE_OF_ARRAY(buf), L"%llu", pHash->ui64FileSize);
	buf[127] = 0;
	if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"size", NULL, buf))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	_snwprintf(buf, SIZE_OF_ARRAY(buf), L"%08lx", pHash->crc32);
	buf[127] = 0;
	if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"crc", NULL, buf))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	_snwprintf(buf, SIZE_OF_ARRAY(buf)
		, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
		, pDigest->md5[0], pDigest->md5[1], pDigest->md5[2], pDigest->md5[3], pDigest->md5[4], pDigest->md5[5], pDigest->md5[6], pDigest->md5[7]
		, pDigest->md5[8], pDigest->md5[9], pDigest->md5[10], pDigest->md5[11], pDigest->md5[12], pDigest->md5[13], pDigest->md5[14], pDigest->md5[15]);
	buf[127] = 0;
	if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"md5", NULL, buf))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	_snwprintf(buf, SIZE_OF_ARRAY(buf)
		, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
		, pDigest->sha[0], pDigest->sha[1], pDigest->sha[2], pDigest->sha[3], pDigest->sha[4]
		, pDigest->sha[5], pDigest->sha[6], pDigest->sha[7], pDigest->sha[8], pDigest->sha[9]
		, pDigest->sha[10], pDigest->sha[11], pDigest->sha[12], pDigest->sha[13], pDigest->sha[14]
		, pDigest->sha[15], pDigest->sha[16], pDigest->sha[17], pDigest->sha[18], pDigest->sha[19]);
	buf[127] = 0;
	if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha1", NULL, buf))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (pExtArg->byDatExpand) {
		WCHAR buf2[256] = {};
		_snwprintf(buf2, SIZE_OF_ARRAY(buf2)
			, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
			, pDigest->sha224[0], pDigest->sha224[1], pDigest->sha224[2], pDigest->sha224[3], pDigest->sha224[4], pDigest->sha224[5], pDigest->sha224[6]
			, pDigest->sha224[7], pDigest->sha224[8], pDigest->sha224[9], pDigest->sha224[10], pDigest->sha224[11], pDigest->sha224[12], pDigest->sha224[13]
			, pDigest->sha224[14], pDigest->sha224[15], pDigest->sha224[16], pDigest->sha224[17], pDigest->sha224[18], pDigest->sha224[19], pDigest->sha224[20]
			, pDigest->sha224[21], pDigest->sha224[22], pDigest->sha224[23], pDigest->sha224[24], pDigest->sha224[25], pDigest->sha224[26], pDigest->sha224[27]);
		buf2[255] = 0;
		if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha224", NULL, buf2))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Dat error: %08.8lx\n", hr);
			return FALSE;
		}
		_snwprintf(buf2, SIZE_OF_ARRAY(buf2)
			, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
			, pDigest->sha256[0], pDigest->sha256[1], pDigest->sha256[2], pDigest->sha256[3], pDigest->sha256[4], pDigest->sha256[5], pDigest->sha256[6], pDigest->sha256[7]
			, pDigest->sha256[8], pDigest->sha256[9], pDigest->sha256[10], pDigest->sha256[11], pDigest->sha256[12], pDigest->sha256[13], pDigest->sha256[14], pDigest->sha256[15]
			, pDigest->sha256[16], pDigest->sha256[17], pDigest->sha256[18], pDigest->sha256[19], pDigest->sha256[20], pDigest->sha256[21], pDigest->sha256[22], pDigest->sha256[23]
			, pDigest->sha256[24], pDigest->sha256[25], pDigest->sha256[26], pDigest->sha256[27], pDigest->sha256[28], pDigest->sha256[29], pDigest->sha256[30], pDigest->sha256[31]);
		buf2[255] = 0;
		if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha256", NULL, buf2))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Dat error: %08.8lx\n", hr);
			return FALSE;
		}
		_snwprintf(buf2, SIZE_OF_ARRAY(buf2)
			, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
			, pDigest->sha384[0], pDigest->sha384[1], pDigest->sha384[2], pDigest->sha384[3], pDigest->sha384[4], pDigest->sha384[5], pDigest->sha384[6], pDigest->sha384[7]
			, pDigest->sha384[8], pDigest->sha384[9], pDigest->sha384[10], pDigest->sha384[11], pDigest->sha384[12], pDigest->sha384[13], pDigest->sha384[14], pDigest->sha384[15]
			, pDigest->sha384[16], pDigest->sha384[17], pDigest->sha384[18], pDigest->sha384[19], pDigest->sha384[20], pDigest->sha384[21], pDigest->sha384[22], pDigest->sha384[23]
			, pDigest->sha384[24], pDigest->sha384[25], pDigest->sha384[26], pDigest->sha384[27], pDigest->sha384[28], pDigest->sha384[29], pDigest->sha384[30], pDigest->sha384[31]
			, pDigest->sha384[32], pDigest->sha384[33], pDigest->sha384[34], pDigest->sha384[35], pDigest->sha384[36], pDigest->sha384[37], pDigest->sha384[38], pDigest->sha384[39]
			, pDigest->sha384[40], pDigest->sha384[41], pDigest->sha384[42], pDigest->sha384[43], pDigest->sha384[44], pDigest->sha384[45], pDigest->sha384[46], pDigest->sha384[47]);
		buf2[255] = 0;
		if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha384", NULL, buf2))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Dat error: %08.8lx\n", hr);
			return FALSE;
		}
		_snwprintf(buf2, SIZE_OF_ARRAY(buf2)
			, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
			, pDigest->sha512[0], pDigest->sha512[1], pDigest->sha512[2], pDigest->sha512[3], pDigest->sha512[4], pDigest->sha512[5], pDigest->sha512[6], pDigest->sha512[7]
			, pDigest->sha512[8], pDigest->sha512[9], pDigest->sha512[10], pDigest->sha512[11], pDigest->sha512[12], pDigest->sha512[13], pDigest->sha512[14], pDigest->sha512[15]
			, pDigest->sha512[16], pDigest->sha512[17], pDigest->sha512[18], pDigest->sha512[19], pDigest->sha512[20], pDigest->sha512[21], pDigest->sha512[22], pDigest->sha512[23]
			, pDigest->sha512[24], pDigest->sha512[25], pDigest->sha512[26], pDigest->sha512[27], pDigest->sha512[28], pDigest->sha512[29], pDigest->sha512[30], pDigest->sha512[31]
			, pDigest->sha512[32], pDigest->sha512[33], pDigest->sha512[34], pDigest->sha512[35], pDigest->sha512[36], pDigest->sha512[37], pDigest->sha512[38], pDigest->sha512[39]
			, pDigest->sha512[40], pDigest->sha512[41], pDigest->sha512[42], pDigest->sha512[43], pDigest->sha512[44], pDigest->sha512[45], pDigest->sha512[46], pDigest->sha512[47]
			, pDigest->sha512[48], pDigest->sha512[49], pDigest->sha512[50], pDigest->sha512[51], pDigest->sha512[52], pDigest->sha512[53], pDigest->sha512[54], pDigest->sha512[55]
			, pDigest->sha512[56], pDigest->sha512[57], pDigest->sha512[58], pDigest->sha512[59], pDigest->sha512[60], pDigest->sha512[61], pDigest->sha512[62], pDigest->sha512[63]);
		buf2[255] = 0;
		if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha512", NULL, buf2))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Dat error: %08.8lx\n", hr);
			return FALSE;
		}
		_snwprintf(buf2, SIZE_OF_ARRAY(buf2), L"%08llx", pDigest->xxh3_64);
		if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"xxh3_64", NULL, buf2))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Dat error: %08.8lx\n", hr);
			return FALSE;
		}
		_snwprintf(buf2, SIZE_OF_ARRAY(buf2), L"%08llx%08llx", pDigest->xxh3_128.high64, pDigest->xxh3_128.low64);
		if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"xxh3_128", NULL, buf2))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Dat error: %08.8lx\n", hr);
			return FALSE;
		}
	}
	if (FAILED(hr = pWriter->WriteEndElement())) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
#else
	XMLElement* newElem4 = pWriter->GetDocument()->NewElement("rom");
	newElem4->SetAttribute("name", pHash->szFnameAndExt);

	CHAR buf[128] = {};
	_snprintf(buf, SIZE_OF_ARRAY(buf), "%llu", pHash->ui64FileSize);
	buf[127] = 0;
	newElem4->SetAttribute("size", buf);

	_snprintf(buf, SIZE_OF_ARRAY(buf), "%08lx", pHash->crc32);
	buf[127] = 0;
	newElem4->SetAttribute("crc", buf);

	_snprintf(buf, SIZE_OF_ARRAY(buf)
		, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
		, pDigest->md5[0], pDigest->md5[1], pDigest->md5[2], pDigest->md5[3], pDigest->md5[4], pDigest->md5[5], pDigest->md5[6], pDigest->md5[7]
		, pDigest->md5[8], pDigest->md5[9], pDigest->md5[10], pDigest->md5[11], pDigest->md5[12], pDigest->md5[13], pDigest->md5[14], pDigest->md5[15]);
	buf[127] = 0;
	newElem4->SetAttribute("md5", buf);

	_snprintf(buf, SIZE_OF_ARRAY(buf)
		, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
		, pDigest->sha[0], pDigest->sha[1], pDigest->sha[2], pDigest->sha[3], pDigest->sha[4]
		, pDigest->sha[5], pDigest->sha[6], pDigest->sha[7], pDigest->sha[8], pDigest->sha[9]
		, pDigest->sha[10], pDigest->sha[11], pDigest->sha[12], pDigest->sha[13], pDigest->sha[14]
		, pDigest->sha[15], pDigest->sha[16], pDigest->sha[17], pDigest->sha[18], pDigest->sha[19]);
	buf[127] = 0;
	newElem4->SetAttribute("sha1", buf);

	if (pExtArg->byDatExpand) {
		CHAR buf2[256] = {};
		_snprintf(buf2, SIZE_OF_ARRAY(buf)
			, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
			, pDigest->sha224[0], pDigest->sha224[1], pDigest->sha224[2], pDigest->sha224[3], pDigest->sha224[4], pDigest->sha224[5], pDigest->sha224[6]
			, pDigest->sha224[7], pDigest->sha224[8], pDigest->sha224[9], pDigest->sha224[10], pDigest->sha224[11], pDigest->sha224[12], pDigest->sha224[13]
			, pDigest->sha224[14], pDigest->sha224[15], pDigest->sha224[16], pDigest->sha224[17], pDigest->sha224[18], pDigest->sha224[19], pDigest->sha224[20]
			, pDigest->sha224[21], pDigest->sha224[22], pDigest->sha224[23], pDigest->sha224[24], pDigest->sha224[25], pDigest->sha224[26], pDigest->sha224[27]);
		buf2[255] = 0;
		newElem4->SetAttribute("sha224", buf2);

		_snprintf(buf2, SIZE_OF_ARRAY(buf2)
			, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
			, pDigest->sha256[0], pDigest->sha256[1], pDigest->sha256[2], pDigest->sha256[3], pDigest->sha256[4], pDigest->sha256[5], pDigest->sha256[6], pDigest->sha256[7]
			, pDigest->sha256[8], pDigest->sha256[9], pDigest->sha256[10], pDigest->sha256[11], pDigest->sha256[12], pDigest->sha256[13], pDigest->sha256[14], pDigest->sha256[15]
			, pDigest->sha256[16], pDigest->sha256[17], pDigest->sha256[18], pDigest->sha256[19], pDigest->sha256[20], pDigest->sha256[21], pDigest->sha256[22], pDigest->sha256[23]
			, pDigest->sha256[24], pDigest->sha256[25], pDigest->sha256[26], pDigest->sha256[27], pDigest->sha256[28], pDigest->sha256[29], pDigest->sha256[30], pDigest->sha256[31]);
		buf2[255] = 0;
		newElem4->SetAttribute("sha256", buf2);

		_snprintf(buf2, SIZE_OF_ARRAY(buf2)
			, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
			, pDigest->sha384[0], pDigest->sha384[1], pDigest->sha384[2], pDigest->sha384[3], pDigest->sha384[4], pDigest->sha384[5], pDigest->sha384[6], pDigest->sha384[7]
			, pDigest->sha384[8], pDigest->sha384[9], pDigest->sha384[10], pDigest->sha384[11], pDigest->sha384[12], pDigest->sha384[13], pDigest->sha384[14], pDigest->sha384[15]
			, pDigest->sha384[16], pDigest->sha384[17], pDigest->sha384[18], pDigest->sha384[19], pDigest->sha384[20], pDigest->sha384[21], pDigest->sha384[22], pDigest->sha384[23]
			, pDigest->sha384[24], pDigest->sha384[25], pDigest->sha384[26], pDigest->sha384[27], pDigest->sha384[28], pDigest->sha384[29], pDigest->sha384[30], pDigest->sha384[31]
			, pDigest->sha384[32], pDigest->sha384[33], pDigest->sha384[34], pDigest->sha384[35], pDigest->sha384[36], pDigest->sha384[37], pDigest->sha384[38], pDigest->sha384[39]
			, pDigest->sha384[40], pDigest->sha384[41], pDigest->sha384[42], pDigest->sha384[43], pDigest->sha384[44], pDigest->sha384[45], pDigest->sha384[46], pDigest->sha384[47]);
		buf2[255] = 0;
		newElem4->SetAttribute("sha384", buf2);

		_snprintf(buf2, SIZE_OF_ARRAY(buf2)
			, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
			, pDigest->sha512[0], pDigest->sha512[1], pDigest->sha512[2], pDigest->sha512[3], pDigest->sha512[4], pDigest->sha512[5], pDigest->sha512[6], pDigest->sha512[7]
			, pDigest->sha512[8], pDigest->sha512[9], pDigest->sha512[10], pDigest->sha512[11], pDigest->sha512[12], pDigest->sha512[13], pDigest->sha512[14], pDigest->sha512[15]
			, pDigest->sha512[16], pDigest->sha512[17], pDigest->sha512[18], pDigest->sha512[19], pDigest->sha512[20], pDigest->sha512[21], pDigest->sha512[22], pDigest->sha512[23]
			, pDigest->sha512[24], pDigest->sha512[25], pDigest->sha512[26], pDigest->sha512[27], pDigest->sha512[28], pDigest->sha512[29], pDigest->sha512[30], pDigest->sha512[31]
			, pDigest->sha512[32], pDigest->sha512[33], pDigest->sha512[34], pDigest->sha512[35], pDigest->sha512[36], pDigest->sha512[37], pDigest->sha512[38], pDigest->sha512[39]
			, pDigest->sha512[40], pDigest->sha512[41], pDigest->sha512[42], pDigest->sha512[43], pDigest->sha512[44], pDigest->sha512[45], pDigest->sha512[46], pDigest->sha512[47]
			, pDigest->sha512[48], pDigest->sha512[49], pDigest->sha512[50], pDigest->sha512[51], pDigest->sha512[52], pDigest->sha512[53], pDigest->sha512[54], pDigest->sha512[55]
			, pDigest->sha512[56], pDigest->sha512[57], pDigest->sha512[58], pDigest->sha512[59], pDigest->sha512[60], pDigest->sha512[61], pDigest->sha512[62], pDigest->sha512[63]);
		buf2[255] = 0;
		newElem4->SetAttribute("sha512", buf2);

		_snprintf(buf2, SIZE_OF_ARRAY(buf2), "%08lx", pDigest->xxh3_64);
		buf2[255] = 0;
		newElem4->SetAttribute("xxh3_64", buf2);

		_snprintf(buf2, SIZE_OF_ARRAY(buf2), "%08lx%08lx", pDigest->xxh3_128.high64, pDigest->xxh3_128.low64);
		buf2[255] = 0;
		newElem4->SetAttribute("xxh3_128", buf2);
	}
	pWriter->InsertEndChild(newElem4);
#endif
	return TRUE;
}

BOOL CalcAndGetHash(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
#else
	XMLElement* pWriter,
#endif
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	_TCHAR* pszFullPath,
	DWORD dwBytesPerSector,
	LPCTSTR szExt,
	UCHAR uiTrack,
	UCHAR uiLastTrack,
	SUB_DESYNC_TYPE bDesync,
	PHASH pHash
) {
	if (dwBytesPerSector == 0) {
		return FALSE;
	}
	HASH_CHUNK hash = {};
	HASH_CHUNK hashUni = {};

	UINT ui1stNonZeroSectorRear = 0;
	UINT ui1stNonZeroSectorPosRear = 0;
	UINT ui1stNonZeroSectorFront = 0;
	UINT ui1stNonZeroSectorPosFront = 0;
	UINT uiLastNonZeroSectorRear = 0;
	UINT uiLastNonZeroSectorPosRear = 0;
	UINT uiLastNonZeroSectorFront = 0;
	UINT uiLastNonZeroSectorPosFront = 0;

	if (pHash->uiMax == 0 || (pExtArg->byRawDump && !_tcsncmp(szExt, _T(".iso"), 4))) {
		// for CD
		FILE* fp = NULL;
		if (bDesync == SUB_DESYNC_TYPE::IdxDesync) {
			if (NULL == (fp = CreateOrOpenFile(pszFullPath, _T(" (Subs indexes)"), NULL
				, hash.szFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		else if (bDesync == SUB_DESYNC_TYPE::CtlDesync) {
			if (NULL == (fp = CreateOrOpenFile(pszFullPath, _T(" (Subs control)"), NULL
				, hash.szFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		else {
			if (NULL == (fp = CreateOrOpenFile(pszFullPath, NULL, NULL
				, hash.szFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}

		hash.ui64FileSize = GetFileSize64(0, fp);
		UINT64 ui64SectorSizeAll = hash.ui64FileSize / (UINT64)dwBytesPerSector;

		if (hash.ui64FileSize >= dwBytesPerSector) {
			BYTE data[CD_RAW_SECTOR_SIZE] = {};
			int nRet = TRUE;

			if (!_tcsncmp(szExt, _T(".bin"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
				GetLastNonZeroPosition(dwBytesPerSector, hash.ui64FileSize, fp, data, ui64SectorSizeAll
					, &uiLastNonZeroSectorRear, &uiLastNonZeroSectorPosRear, &uiLastNonZeroSectorFront, &uiLastNonZeroSectorPosFront);
			}
			CalcInit(pExtArg, &hash);
			OutputString("Hashing: %s\n", hash.szFnameAndExt);
			BOOL b1stNonZeroFront = FALSE;
			BOOL b1stNonZeroRear = FALSE;
			INT nConsecutiveZeroCnt = 0;

			for (UINT64 i = 0; i < ui64SectorSizeAll; i++) {
				if (fread(data, sizeof(BYTE), dwBytesPerSector, fp) < dwBytesPerSector) {
					OutputErrorString("Failed to read: read size %lu [F:%s][L:%d]\n", dwBytesPerSector, _T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				nRet = CalcHash(pExtArg, &hash, data, (UINT)dwBytesPerSector);
				if (!nRet) {
					break;
				}
				if (!_tcsncmp(szExt, _T(".bin"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
					if (!b1stNonZeroFront) {
						Get1stNonZeroPositionFront(pExtArg, dwBytesPerSector, &hash, &b1stNonZeroFront, (UINT)i
							, data, &nConsecutiveZeroCnt, &ui1stNonZeroSectorFront, &ui1stNonZeroSectorPosFront, FALSE);
						if (b1stNonZeroFront && ui1stNonZeroSectorFront == 0) {
							Get1stNonZeroPositionRear(dwBytesPerSector, &b1stNonZeroRear, (INT)i, data
								, &nConsecutiveZeroCnt, &ui1stNonZeroSectorRear, &ui1stNonZeroSectorPosRear);
							if (ui1stNonZeroSectorRear == 0xffffffff && ui1stNonZeroSectorPosRear == 2352) {
								ui1stNonZeroSectorRear = uiLastNonZeroSectorRear;
								ui1stNonZeroSectorPosRear = uiLastNonZeroSectorPosRear;
								b1stNonZeroRear = TRUE;
							}
						}
					}
					else if (b1stNonZeroFront && !b1stNonZeroRear) {
						Get1stNonZeroPositionRear(dwBytesPerSector, &b1stNonZeroRear, (INT)i, data
							, &nConsecutiveZeroCnt, &ui1stNonZeroSectorRear, &ui1stNonZeroSectorPosRear);
					}
				}
			}

			if (!_tcsncmp(szExt, _T(".bin"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
				if (nConsecutiveZeroCnt < ZERO_BYTE_CHUNK) {
					ui1stNonZeroSectorRear = uiLastNonZeroSectorRear;
					ui1stNonZeroSectorPosRear = uiLastNonZeroSectorPosRear;
				}
				OutputDiscLog(
					OUTPUT_DHYPHEN_PLUS_STR("Non-zero byte position of the track %02d")
					"\t 1st non-zero byte position (sample based)(Front): %6u sector + %4u byte\n"
					"\t 1st non-zero byte position (sample based)(Rear) : %6u sector + %4u byte\n"
					"\tLast non-zero byte position (sample based)(Front): %6u sector + %4u byte\n"
					"\tLast non-zero byte position (sample based)(Rear) : %6u sector + %4u byte (Last sector: %6llu)\n"
					, uiTrack, ui1stNonZeroSectorFront, ui1stNonZeroSectorPosFront
					, ui1stNonZeroSectorRear, ui1stNonZeroSectorPosRear
					, uiLastNonZeroSectorFront, uiLastNonZeroSectorPosFront
					, uiLastNonZeroSectorRear, uiLastNonZeroSectorPosRear, ui64SectorSizeAll
				);
			}
			FcloseAndNull(fp);
			if (!nRet) {
				return nRet;
			}

			FILE* fpUni = NULL;
			UINT64 ui64SectorSizeAllUni = 0;

			if (IsPregapOfTrack1ReadableDrive(pDevice) && !_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly && !pExtArg->byAtari) {
				if (NULL == (fpUni = CreateOrOpenFile(pszFullPath, _T(" (Track all)"), NULL
					, hashUni.szFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				hashUni.ui64FileSize = GetFileSize64(0, fpUni);
				ui64SectorSizeAllUni = hashUni.ui64FileSize / (UINT64)dwBytesPerSector;

				GetLastNonZeroPosition(dwBytesPerSector, hashUni.ui64FileSize, fpUni, data, ui64SectorSizeAllUni
					, &uiLastNonZeroSectorRear, &uiLastNonZeroSectorPosRear, &uiLastNonZeroSectorFront, &uiLastNonZeroSectorPosFront);
				CalcInit(pExtArg, &hashUni);

				b1stNonZeroRear = FALSE;
				for (UINT64 j = 0; j < ui64SectorSizeAllUni; j++) {
					if (fread(data, sizeof(BYTE), dwBytesPerSector, fpUni) < dwBytesPerSector) {
						OutputErrorString("Failed to read: read size %lu [F:%s][L:%d]\n", dwBytesPerSector, _T(__FUNCTION__), __LINE__);
						FcloseAndNull(fpUni);
						return FALSE;
					};
					// Calc hash from the 1st non-zero byte position to the last non-zero byte position
					if (b1stNonZeroRear) {
						if (j == static_cast<unsigned long long>(uiLastNonZeroSectorRear)) {
							CalcHash(pExtArg, &hashUni, data, uiLastNonZeroSectorPosRear);
							break;
						}
						else {
							nRet = CalcHash(pExtArg, &hashUni, data, (UINT)dwBytesPerSector);
							if (!nRet) {
								FcloseAndNull(fpUni);
								break;
							}
						}
					}
					else {
						Get1stNonZeroPositionFront(pExtArg, dwBytesPerSector, &hashUni, &b1stNonZeroRear
							, (UINT)j, data, &nConsecutiveZeroCnt, &ui1stNonZeroSectorRear, &ui1stNonZeroSectorPosRear, TRUE);
					}
				}
				OutputDiscLog(
					OUTPUT_DHYPHEN_PLUS_STR("Non-zero byte position of (Track all).img")
					"\t 1st non-zero byte position (sample based): %6u sector + %4u byte\n"
					"\tLast non-zero byte position (sample based): %6u sector + %4u byte\n"
					, ui1stNonZeroSectorRear, ui1stNonZeroSectorPosRear, uiLastNonZeroSectorRear, uiLastNonZeroSectorPosRear
				);
			}
			FcloseAndNull(fpUni);
		}
	}
	else {
		// for DVD, BD
		hash.crc32 = pHash->pHashChunk[pHash->uiCount].crc32;
		memcpy(&hash.md5, &pHash->pHashChunk[pHash->uiCount].md5, sizeof(hash.md5));
		memcpy(&hash.sha, &pHash->pHashChunk[pHash->uiCount].sha, sizeof(hash.sha));
		if (pExtArg->byDatExpand) {
			memcpy(&hash.sha224, &pHash->pHashChunk[pHash->uiCount].sha224, sizeof(hash.sha224));
			memcpy(&hash.sha256, &pHash->pHashChunk[pHash->uiCount].sha256, sizeof(hash.sha256));
			memcpy(&hash.sha384, &pHash->pHashChunk[pHash->uiCount].sha384, sizeof(hash.sha384));
			memcpy(&hash.sha512, &pHash->pHashChunk[pHash->uiCount].sha512, sizeof(hash.sha512));
			hash.xxh3_64 = pHash->pHashChunk[pHash->uiCount].xxh3_64;
			hash.xxh3_128 = pHash->pHashChunk[pHash->uiCount].xxh3_128;
		}
		_tcsncpy(hash.szFnameAndExt, pHash->pHashChunk[pHash->uiCount].szFnameAndExt, sizeof(hash.szFnameAndExt));
		hash.ui64FileSize = pHash->pHashChunk[pHash->uiCount].ui64FileSize;
		pHash->uiCount++;
	}

	MESSAGE_DIGEST_CHUNK digest = {};
	if (CalcEnd(pExtArg, &hash, &digest)) {
		OutputHash(pExtArg, pWriter, &hash, &digest);
	}
	if (IsPregapOfTrack1ReadableDrive(pDevice) && !_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly && !pExtArg->byAtari) {
		MESSAGE_DIGEST_CHUNK digestUni = {};
		if (CalcEnd(pExtArg, &hashUni, &digestUni)) {
			OutputHash(pExtArg, pWriter, &hashUni, &digestUni);
		}
	}
	return TRUE;
}

BOOL OutputRomElement(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
	CComPtr<IXmlWriter> pWriterSuppl,
#else
	XMLElement* pWriter,
	XMLElement* pWriterSuppl,
#endif
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	_TCHAR* pszFullPath,
	SUB_DESYNC_TYPE bDesync,
	PHASH pHash
) {
	if (*pExecType == fd || *pExecType == disk) {
		if (!CalcAndGetHash(pWriter, pExtArg, pDevice, pDisc, pszFullPath, pDisc->dwBytesPerSector, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
			return FALSE;
		}
	}
	else if (*pExecType == dvd || IsXbox(pExecType) || *pExecType == bd || *pExecType == sacd) {
		_TCHAR szPath[_MAX_PATH] = {};
		if (*pExecType == dvd || *pExecType == xbox) {
			if (*pExecType == xbox) {
				_tcsncpy(szPath, pszFullPath, _MAX_PATH - 1);
				PathRemoveExtension(szPath);
				_tcsncat(szPath, _T("_SS.bin"), _MAX_PATH - _tcslen(szPath) - 1);

				if (!CalcAndGetHash(pWriterSuppl, pExtArg, pDevice, pDisc, szPath, DISC_MAIN_DATA_SIZE, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
					return FALSE;
				}
			}

			_tcsncpy(szPath, pszFullPath, _MAX_PATH - 1);
			PathRemoveExtension(szPath);
			_tcsncat(szPath, _T("_PFI.bin"), _MAX_PATH - _tcslen(szPath) - 1);

			if (!CalcAndGetHash(pWriterSuppl, pExtArg, pDevice, pDisc, szPath, DISC_MAIN_DATA_SIZE, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
				return FALSE;
			}

			_tcsncpy(szPath, pszFullPath, _MAX_PATH - 1);
			PathRemoveExtension(szPath);
			_tcsncat(szPath, _T("_DMI.bin"), _MAX_PATH - _tcslen(szPath) - 1);

			if (!CalcAndGetHash(pWriterSuppl, pExtArg, pDevice, pDisc, szPath, DISC_MAIN_DATA_SIZE, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
				return FALSE;
			}
		}
		else if (*pExecType == bd) {
			_tcsncpy(szPath, pszFullPath, _MAX_PATH - 1);
			PathRemoveExtension(szPath);
			_tcsncat(szPath, _T("_PIC.bin"), _MAX_PATH - _tcslen(szPath) - 1);

			if (!CalcAndGetHash(pWriterSuppl, pExtArg, pDevice, pDisc, szPath, DISC_MAIN_DATA_SIZE, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
				return FALSE;
			}
		}
		if (pExtArg->byRawDump) {
			if (!CalcAndGetHash(pWriterSuppl, pExtArg, pDevice, pDisc, pszFullPath, DISC_MAIN_DATA_SIZE, _T(".raw"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
				return FALSE;
			}
		}
		if (!CalcAndGetHash(pWriter, pExtArg, pDevice, pDisc, pszFullPath, DISC_MAIN_DATA_SIZE, _T(".iso"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
			return FALSE;
		}
	}
	else {
		if (bDesync == SUB_DESYNC_TYPE::NoDesync) {
			if (pDisc->SCSI.trkType == TRACK_TYPE::dataExist ||
				pDisc->SCSI.trkType == TRACK_TYPE::pregapDataIn1stTrack) {
				if (!CalcAndGetHash(pWriterSuppl, pExtArg, pDevice, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".scm"), 1, 1, bDesync, pHash)) {
					return FALSE;
				}
			}
			if (!CalcAndGetHash(pWriterSuppl, pExtArg, pDevice, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".img"), 1, 1, bDesync, pHash)) {
				return FALSE;
			}
		}
		else if (bDesync == SUB_DESYNC_TYPE::CtlDesync) {
			if (!CalcAndGetHash(pWriterSuppl, pExtArg, pDevice, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".img"), 1, 1, bDesync, pHash)) {
				return FALSE;
			}
		}
		for (UCHAR i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
			if (!CalcAndGetHash(pWriter, pExtArg, pDevice, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".bin"), i, pDisc->SCSI.toc.LastTrack, bDesync, pHash)) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL XmlWriterInit(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
	CComPtr<IStream> pWriteStream
#else
	XMLDocument* xmlWriter
#endif
) {
#ifdef _WIN32
	HRESULT hr = S_OK;
	if (FAILED(hr = pWriter->SetOutput(pWriteStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->WriteStartDocument(XmlStandalone_Omit))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->WriteDocType(L"datafile"
		, L"-//Logiqx//DTD ROM Management Datafile//EN", L"http://www.logiqx.com/Dats/datafile.dtd", NULL))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
#else
	XMLDeclaration* decl = xmlWriter->NewDeclaration();
	xmlWriter->InsertEndChild(decl);
	XMLUnknown* unk = xmlWriter->NewUnknown("DOCTYPE datafile PUBLIC \"-//Logiqx//DTD ROM Management Datafile//EN\" \"http://www.logiqx.com/Dats/datafile.dtd\"");
	xmlWriter->InsertEndChild(unk);
#endif
	return TRUE;
}

BOOL XmlWriteStartElementForGamename(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
	LPCWSTR pwszLocalName,
	LPCWSTR pwszAttributeName,
	LPWCH pCurrentDir
#else
	XMLElement* readElem,
	XMLDocument* xmlWriter,
	XMLElement** newElem1
#endif
) {
#ifdef _WIN32
	HRESULT hr = S_OK;
	if (FAILED(hr = pWriter->WriteStartElement(NULL, pwszLocalName, NULL))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->WriteAttributeString(NULL, pwszAttributeName, NULL, pCurrentDir))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
#else
	*newElem1 = xmlWriter->NewElement(readElem->Name());
	if (readElem->GetText() == NULL) {
		(*newElem1)->SetText("\n");
	}
	else {
		(*newElem1)->SetText(readElem->GetText());
	}
#endif
	return TRUE;
}

BOOL XmlWriteStartElementForOthername(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
	LPCWSTR pwszLocalName
#else
	XMLElement* readElem2,
	XMLElement* newElem1,
	XMLElement** newElem2
#endif
) {
#ifdef _WIN32
	HRESULT hr = S_OK;
	if (FAILED(hr = pWriter->WriteStartElement(NULL, pwszLocalName, NULL))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
#else
	*newElem2 = newElem1->GetDocument()->NewElement(readElem2->Name());
	if (readElem2->GetText() == NULL) {
		if (!strcmp(readElem2->Name(), "game")) {
			(*newElem2)->SetAttribute("name", "-insert name-");
		}
		else {
			(*newElem2)->SetText("\n");
		}
	}
	else {
		(*newElem2)->SetText(readElem2->GetText());
	}
#endif
	return TRUE;
}

BOOL XmlWriteString(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
	LPCWSTR pwszLocalName,
	LPCWSTR pwszValue,
	LPWCH pCurrentDir
#else
	XMLElement* readElem2,
	XMLElement* newElem2,
	LPBOOL bDescription
#endif
) {
#ifdef _WIN32
	HRESULT hr = S_OK;
	if (!wcsncmp(pwszLocalName, L"description", 11) && !wcsncmp(pwszValue, L"foo", 3)) {
		if (FAILED(hr = pWriter->WriteString(pCurrentDir))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Dat error: %08.8lx\n", hr);
			return FALSE;
		}
	}
	else {
		if (FAILED(hr = pWriter->WriteString(pwszValue))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString("Dat error: %08.8lx\n", hr);
			return FALSE;
		}
	}
#else
	XMLElement* readElem3 = readElem2->FirstChildElement();
	// description, rom name etc.
	while (readElem3) {
		XMLElement* newElem3 = newElem2->GetDocument()->NewElement(readElem3->Name());
		if (readElem3->GetText() == NULL) {
			newElem3->SetText("\n");
		}
		else {
			if (!strcmp(readElem3->Name(), "description") && !strcmp(readElem2->Name(), "game")) {
				newElem3->SetText("-insert description-");
				*bDescription = TRUE;
			}
			else {
				newElem3->SetText(readElem3->GetText());
			}
		}
		newElem2->InsertEndChild(newElem3);
		readElem3 = readElem3->NextSiblingElement();
	}
#endif
	return TRUE;
}

#ifdef _WIN32
BOOL XmlWriteEndElement(
	CComPtr<IXmlWriter> pWriter
) {
	HRESULT hr = S_OK;
	if (FAILED(hr = pWriter->WriteEndElement())) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	return TRUE;
}

BOOL XmlWriteEndDocument(
	CComPtr<IXmlWriter> pWriter
) {
	HRESULT hr = S_OK;
	if (FAILED(hr = pWriter->WriteEndDocument())) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->Flush())) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	return TRUE;
}
#endif

BOOL ReadWriteDat(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	_TCHAR* pszFullPath,
	_TCHAR* szDir,
	SUB_DESYNC_TYPE bDesync,
	PHASH pHash
) {
#ifdef _WIN32
	WCHAR wszDefaultDat[_MAX_PATH] = {};
	if (!GetModuleFileNameW(NULL, wszDefaultDat, SIZE_OF_ARRAY(wszDefaultDat))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!PathRemoveFileSpecW(wszDefaultDat)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!PathAppendW(wszDefaultDat, L"default.dat")) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	HRESULT hr = S_OK;
	CComPtr<IXmlReader> pReader;
	CComPtr<IStream> pReadStream;
	if (FAILED(hr = SHCreateStreamOnFileW(wszDefaultDat, STGM_READ, &pReadStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = pReader->SetInput(pReadStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Parse))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}

	_TCHAR szDatPath[_MAX_PATH] = {};
	_TCHAR szDatPathSuppl[_MAX_PATH] = {};
	_tcsncpy(szDatPath, pszFullPath, SIZE_OF_ARRAY(szDatPath) - 1);
	_tcsncpy(szDatPathSuppl, pszFullPath, SIZE_OF_ARRAY(szDatPathSuppl) - 1);

	if (bDesync == SUB_DESYNC_TYPE::IdxDesync) {
		PathRemoveExtension(szDatPath);
		_TCHAR str1[] = _T(" (Subs indexes).dat");
		_tcsncat(szDatPath, str1, SIZE_OF_ARRAY(szDatPath) - _tcslen(szDatPath) - 1);

		PathRemoveExtension(szDatPathSuppl);
		_TCHAR str2[] = _T("_Suppl (Subs indexes).dat");
		_tcsncat(szDatPathSuppl, str2, SIZE_OF_ARRAY(szDatPathSuppl) - _tcslen(szDatPathSuppl) - 1);
	}
	else if (bDesync == SUB_DESYNC_TYPE::CtlDesync) {
		PathRemoveExtension(szDatPath);
		_TCHAR str1[] = _T(" (Subs control).dat");
		_tcsncat(szDatPath, str1, SIZE_OF_ARRAY(szDatPath) - _tcslen(szDatPath) - 1);

		PathRemoveExtension(szDatPathSuppl);
		_TCHAR str2[] = _T("_suppl (Subs control).dat");
		_tcsncat(szDatPathSuppl, str2, SIZE_OF_ARRAY(szDatPathSuppl) - _tcslen(szDatPathSuppl) - 1);
	}
	else {
		PathRenameExtension(szDatPath, _T(".dat"));

		PathRemoveExtension(szDatPathSuppl);
		_TCHAR str1[] = _T("_suppl.dat");
		_tcsncat(szDatPathSuppl, str1, SIZE_OF_ARRAY(szDatPathSuppl) - _tcslen(szDatPathSuppl) - 1);
	}
	szDatPath[_MAX_FNAME - 1] = 0;
	szDatPathSuppl[_MAX_FNAME - 1] = 0;

	WCHAR wszDatPathForStream[_MAX_PATH] = {};
	WCHAR wszDatPathForStreamSuppl[_MAX_PATH] = {};
#ifndef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0
		, szDatPath, SIZE_OF_ARRAY(szDatPath)
		, wszDatPathForStream, SIZE_OF_ARRAY(wszDatPathForStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (!MultiByteToWideChar(CP_ACP, 0
		, szDatPathSuppl, SIZE_OF_ARRAY(szDatPathSuppl)
		, wszDatPathForStreamSuppl, SIZE_OF_ARRAY(wszDatPathForStreamSuppl))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
}
#else
	size_t size = SIZE_OF_ARRAY(wszDatPathForStream);
	wcsncpy(wszDatPathForStream, szDatPath, size);
	wszDatPathForStream[size - 1] = 0;

	size = SIZE_OF_ARRAY(wszDatPathForStreamSuppl);
	wcsncpy(wszDatPathForStreamSuppl, szDatPath, size);
	wszDatPathForStreamSuppl[size - 1] = 0;
#endif

	CComPtr<IXmlWriter> pWriter = NULL;
	if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	CComPtr<IStream> pWriteStream = NULL;
	if (FAILED(hr = SHCreateStreamOnFileW(wszDatPathForStream, STGM_CREATE | STGM_WRITE, &pWriteStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}

	CComPtr<IXmlWriter> pWriterSuppl = NULL;
	if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriterSuppl), 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	CComPtr<IStream> pWriteStreamSuppl = NULL;
	if (FAILED(hr = SHCreateStreamOnFileW(wszDatPathForStreamSuppl, STGM_CREATE | STGM_WRITE, &pWriteStreamSuppl))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}

	if (!XmlWriterInit(pWriter, pWriteStream)) {
		return FALSE;
	}
	if (!XmlWriterInit(pWriterSuppl, pWriteStreamSuppl)) {
		return FALSE;
	}

	WCHAR wszDir[_MAX_DIR] = {};
#ifndef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, szDir, _MAX_DIR, wszDir, SIZE_OF_ARRAY(wszDir))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#else
	size = SIZE_OF_ARRAY(wszDir);
	wcsncpy(wszDir, szDir, size);
	wszDir[size - 1] = 0;
#endif
	LPWCH p = wcsrchr(wszDir, L'\\');
	if (wszDir[1] != '\0') {
		*p = NULL;
		p = wcsrchr(wszDir, L'\\');
	}
	LPWCH pCurrentDir = p + 1;

	XmlNodeType nodeType;
	LPCWSTR pwszLocalName = NULL;
	LPCWSTR pwszValue = NULL;
	while (S_OK == pReader->Read(&nodeType)) {
		switch (nodeType) {
		case XmlNodeType_Element:
			if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			else if (!pwszLocalName) {
				OutputErrorString("[L:%d] LocalName is NULL\n", __LINE__);
				return FALSE;
			}
			if (!wcsncmp(pwszLocalName, L"game", 4)) {
				if (S_FALSE == pReader->MoveToFirstAttribute()) {
					break;
				}
				do {
					LPCWSTR pwszAttributeName = NULL;
					if (FAILED(hr = pReader->GetLocalName(&pwszAttributeName, NULL))) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						OutputErrorString("Dat error: %08.8lx\n", hr);
						return FALSE;
					}
					else if (!pwszAttributeName) {
						OutputErrorString("[L:%d] AttributeName is NULL\n", __LINE__);
						return FALSE;
					}
					if (!wcsncmp(pwszAttributeName, L"name", 4)) {
						if (!XmlWriteStartElementForGamename(pWriter, pwszLocalName, pwszAttributeName, pCurrentDir)) {
							return FALSE;
						}
						if (!XmlWriteStartElementForGamename(pWriterSuppl, pwszLocalName, pwszAttributeName, pCurrentDir)) {
							return FALSE;
						}
					}
				} while (S_OK == pReader->MoveToNextAttribute());
			}
			else {
				if (!XmlWriteStartElementForOthername(pWriter, pwszLocalName)) {
					return FALSE;
				}
				if (!XmlWriteStartElementForOthername(pWriterSuppl, pwszLocalName)) {
					return FALSE;
				}
			}
			break;
		case XmlNodeType_Text:
			if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			else if (!pwszValue) {
				OutputErrorString("[L:%d] GetValue is NULL\n", __LINE__);
				return FALSE;
			}
			if (!XmlWriteString(pWriter, pwszLocalName, pwszValue, pCurrentDir)) {
				return FALSE;
			}
			if (!XmlWriteString(pWriterSuppl, pwszLocalName, pwszValue, pCurrentDir)) {
				return FALSE;
			}
			break;
		case XmlNodeType_EndElement:
			if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			else if (!pwszLocalName) {
				OutputErrorString("[L:%d] LocalName is NULL\n", __LINE__);
				return FALSE;
			}
			if (!wcsncmp(pwszLocalName, L"game", 4)) {
				OutputRomElement(pWriter, pWriterSuppl, pExecType, pExtArg, pDevice, pDisc, pszFullPath, bDesync, pHash);
			}
			if (!XmlWriteEndElement(pWriter)) {
				return FALSE;
			}
			if (!XmlWriteEndElement(pWriterSuppl)) {
				return FALSE;
			}
			break;
		case XmlNodeType_None:
			break;
		case XmlNodeType_Attribute:
			break;
		case XmlNodeType_CDATA:
			break;
		case XmlNodeType_ProcessingInstruction:
			break;
		case XmlNodeType_Comment:
			break;
		case XmlNodeType_DocumentType:
			break;
		case XmlNodeType_Whitespace:
			break;
		case XmlNodeType_XmlDeclaration:
			break;
		default:
			break;
		}
	}
	if (!XmlWriteEndDocument(pWriter)) {
		return FALSE;
	}
	if (!XmlWriteEndDocument(pWriterSuppl)) {
		return FALSE;
	}
#else
	UNREFERENCED_PARAMETER(szDir);
	CHAR szDefaultDat[_MAX_PATH] = {};
	if (PathFileExists("/usr/local/share/DiscImageCreator/default.dat")) {
		PathSet(szDefaultDat, "/usr/local/share/DiscImageCreator/default.dat");
	}
	else if (PathFileExists("/usr/share/DiscImageCreator/default.dat")) {
		PathSet(szDefaultDat, "/usr/share/DiscImageCreator/default.dat");
	}
	else {
		if (!GetModuleFileName(NULL, szDefaultDat, SIZE_OF_ARRAY(szDefaultDat))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!PathRemoveFileSpec(szDefaultDat)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		if (!PathAppend(szDefaultDat, "default.dat")) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}

	XMLDocument xmlReader;
	XMLError err = xmlReader.LoadFile(szDefaultDat);
	if (err != XML_SUCCESS) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(" => %s\n", szDefaultDat);
		return FALSE;
	}

	XMLDocument xmlWriter;
	XMLDocument xmlWriterSuppl;
	XmlWriterInit(&xmlWriter);
	XmlWriterInit(&xmlWriterSuppl);

	XMLElement* readElem = xmlReader.FirstChildElement();
	BOOL bDescription = FALSE;
	// datafile
	while (readElem) {
		XMLElement* newElem1 = NULL;
		XMLElement* newElem1Suppl = NULL;
		XmlWriteStartElementForGamename(readElem, &xmlWriter, &newElem1);
		XmlWriteStartElementForGamename(readElem, &xmlWriterSuppl, &newElem1Suppl);

		XMLElement* readElem2 = readElem->FirstChildElement();
		// header, game
		while (readElem2) {
			XMLElement* newElem2 = NULL;
			XMLElement* newElem2Suppl = NULL;
			XmlWriteStartElementForOthername(readElem2, newElem1, &newElem2);
			XmlWriteStartElementForOthername(readElem2, newElem1Suppl, &newElem2Suppl);

			XmlWriteString(readElem2, newElem2, &bDescription);
			XmlWriteString(readElem2, newElem2Suppl, &bDescription);
			if (bDescription) {
				OutputRomElement(newElem2, newElem2Suppl, pExecType, pExtArg, pDevice, pDisc, pszFullPath, bDesync, pHash);
			}

			newElem1->InsertEndChild(newElem2);
			newElem1Suppl->InsertEndChild(newElem2Suppl);
			readElem2 = readElem2->NextSiblingElement();
		}
		xmlWriter.InsertEndChild(newElem1);
		xmlWriterSuppl.InsertEndChild(newElem1Suppl);
		readElem = readElem->NextSiblingElement();
	}

	CHAR szDatPath[_MAX_PATH] = {};
	CHAR szDatPathSuppl[_MAX_PATH] = {};
	_tcsncpy(szDatPath, pszFullPath, sizeof(szDatPath) - 1);
	_tcsncpy(szDatPathSuppl, pszFullPath, sizeof(szDatPathSuppl) - 1);

	if (bDesync == SUB_DESYNC_TYPE::IdxDesync) {
		PathRemoveExtension(szDatPath);
		_TCHAR str1[] = _T(" (Subs indexes).dat");
		_tcsncat(szDatPath, str1, sizeof(szDatPath) - _tcslen(szDatPath) - 1);

		PathRemoveExtension(szDatPathSuppl);
		_TCHAR str2[] = _T("_suppl (Subs indexes).dat");
		_tcsncat(szDatPathSuppl, str2, sizeof(szDatPathSuppl) - _tcslen(szDatPathSuppl) - 1);
	}
	else if (bDesync == SUB_DESYNC_TYPE::CtlDesync) {
		PathRemoveExtension(szDatPath);
		_TCHAR str1[] = _T(" (Subs control).dat");
		_tcsncat(szDatPath, str1, sizeof(szDatPath) - _tcslen(szDatPath) - 1);

		PathRemoveExtension(szDatPathSuppl);
		_TCHAR str2[] = _T("_suppl (Subs control).dat");
		_tcsncat(szDatPathSuppl, str2, sizeof(szDatPathSuppl) - _tcslen(szDatPathSuppl) - 1);
	}
	else {
		PathRenameExtension(szDatPath, _T(".dat"));

		PathRemoveExtension(szDatPathSuppl);
		_TCHAR str[] = _T("_suppl.dat");
		_tcsncat(szDatPathSuppl, str, sizeof(szDatPathSuppl) - _tcslen(szDatPathSuppl) - 1);
	}
	szDatPath[_MAX_PATH - 1] = 0;
	szDatPathSuppl[_MAX_PATH - 1] = 0;

	xmlWriter.SaveFile(szDatPath);
	xmlWriterSuppl.SaveFile(szDatPathSuppl);
#endif
	return TRUE;
}
