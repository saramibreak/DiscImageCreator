/**
 * Copyright 2011-2023 sarami
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

BOOL OutputHash(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
#else
	XMLElement* pWriter,
#endif
	PEXT_ARG pExtArg,
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

	UINT ui1stNonZeroSector = 0;
	UINT ui1stNonZeroSectorPos = 0;
	UINT uiLastNonZeroSector = 0;
	UINT uiLastNonZeroSectorPos = 0;

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

			CalcInit(pExtArg, &hash);
			OutputString("Hashing: %s\n", hash.szFnameAndExt);

			for (UINT64 i = 1; i <= ui64SectorSizeAll; i++) {
				if (fread(data, sizeof(BYTE), dwBytesPerSector, fp) < dwBytesPerSector) {
					OutputErrorString("Failed to read: read size %lu [F:%s][L:%d]\n", dwBytesPerSector, _T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				nRet = CalcHash(pExtArg, &hash, data, (UINT)dwBytesPerSector);
				if (!nRet) {
					break;
				}

			}
			FcloseAndNull(fp);
			if (!nRet) {
				return nRet;
			}

			FILE* fpUni = NULL;
			UINT64 ui64SectorSizeAllUni = 0;

			if (!_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
				if (NULL == (fpUni = CreateOrOpenFile(pszFullPath, _T(" (Track all)"), NULL
					, hashUni.szFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				hashUni.ui64FileSize = GetFileSize64(0, fpUni);
				ui64SectorSizeAllUni = hashUni.ui64FileSize / (UINT64)dwBytesPerSector;

				// Check last non-zero byte position for Audio CD
				for (UINT64 j = 1; j <= ui64SectorSizeAllUni; j++) {
					fseek(fpUni, (LONG)(hashUni.ui64FileSize - dwBytesPerSector * j), SEEK_SET);
					if (fread(data, sizeof(BYTE), dwBytesPerSector, fpUni) < dwBytesPerSector) {
						OutputErrorString("Failed to read: read size %lu [F:%s][L:%d]\n", dwBytesPerSector, _T(__FUNCTION__), __LINE__);
						FcloseAndNull(fpUni);
						return FALSE;
					};
					BOOL bLastNonZero = TRUE;
					for (INT k = (INT)(dwBytesPerSector - 1); 0 <= k; k--) {
						if (data[k] != 0) {
							bLastNonZero = FALSE;
							UINT ofs = UINT(k + 1);
							UINT mod = ofs % 4;
							if (mod == 0) {
								uiLastNonZeroSectorPos = ofs;
							}
							else {
								uiLastNonZeroSectorPos = (ofs + (4 - mod));
							}
							break;
						}
					}
					if (!bLastNonZero) {
						uiLastNonZeroSector = (UINT)(ui64SectorSizeAllUni - j);
						break;
					}
				}
				CalcInit(pExtArg, &hashUni);
				rewind(fpUni);

				BOOL b1stNonZero = FALSE;
				for (UINT64 j = 1; j <= ui64SectorSizeAllUni; j++) {
					if (fread(data, sizeof(BYTE), dwBytesPerSector, fpUni) < dwBytesPerSector) {
						OutputErrorString("Failed to read: read size %lu [F:%s][L:%d]\n", dwBytesPerSector, _T(__FUNCTION__), __LINE__);
						FcloseAndNull(fpUni);
						return FALSE;
					};
					// Calc hash from the 1st non-zero byte position to the last non-zero byte position
					if (b1stNonZero) {
						if (j == static_cast<unsigned long long>(uiLastNonZeroSector) + 1) {
							CalcHash(pExtArg, &hashUni, data, uiLastNonZeroSectorPos);
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
						for (UINT k = 0; k < dwBytesPerSector; k++) {
							if (data[k] != 0) {
								b1stNonZero = TRUE;
								UINT ofs = (k - (k % 4));
								nRet = CalcHash(pExtArg, &hashUni, data + ofs, (UINT)(dwBytesPerSector - ofs));
								if (!nRet) {
									FcloseAndNull(fpUni);
									break;
								}
								ui1stNonZeroSector = (UINT)j;
								ui1stNonZeroSectorPos = ofs;
								break;
							}
						}
					}
				}
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
	MESSAGE_DIGEST_CHUNK digestUni = {};

	if (CalcEnd(pExtArg, &hash, &digest)) {
		if (!_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
			CalcEnd(pExtArg, &hashUni, &digestUni);
		}
		if (!_tcsncmp(szExt, _T(".scm"), 4) ||
			!_tcsncmp(szExt, _T(".img"), 4) ||
			!_tcsncmp(szExt, _T(".raw"), 4) ||
			find_last_string(hash.szFnameAndExt, _T("_SS.bin")) ||
			find_last_string(hash.szFnameAndExt, _T("_PFI.bin")) ||
			find_last_string(hash.szFnameAndExt, _T("_DMI.bin")) ||
			find_last_string(hash.szFnameAndExt, _T("_PIC.bin"))
			) {
#ifndef _DEBUG
			OutputHashData(pExtArg, g_LogFile.fpDisc, &hash, &digest);
			if (!_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
				OutputDiscLog(
					OUTPUT_DHYPHEN_PLUS_STR("Hash(Universal Whole image)")
					"\t 1st non-zero byte position (sample based): %6u sector + %4u byte\n"
					"\tLast non-zero byte position (sample based): %6u sector + %4u byte\n"
					, ui1stNonZeroSector, ui1stNonZeroSectorPos, uiLastNonZeroSector - 1, uiLastNonZeroSectorPos
				);
//				hashUni.ui64FileSize = (UINT64)((uiLastNonZeroSector - ui1stNonZeroSector + 1) * dwBytesPerSector + uiLastNonZeroSectorPos - ui1stNonZeroSectorPos + 1);
				OutputHashData(pExtArg, g_LogFile.fpDisc, &hashUni, &digestUni);
			}
#endif
		}
		else {
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
				, hash.szFnameAndExt, sizeof(hash.szFnameAndExt) / sizeof(hash.szFnameAndExt[0])
				, wszFnameAndExt, sizeof(wszFnameAndExt) / sizeof(wszFnameAndExt[0]))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
#else
			size_t size = sizeof(wszFnameAndExt) / sizeof(wszFnameAndExt[0]);
			wcsncpy(wszFnameAndExt, szFnameAndExt, size);
			wszFnameAndExt[size - 1] = 0;
#endif
			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"name", NULL, wszFnameAndExt))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			WCHAR buf[128] = {};
			_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%llu", hash.ui64FileSize);
			buf[127] = 0;
			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"size", NULL, buf))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%08lx", hash.crc32);
			buf[127] = 0;
			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"crc", NULL, buf))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			_snwprintf(buf, sizeof(buf) / sizeof(buf[0])
				, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
				, digest.md5[0], digest.md5[1], digest.md5[2], digest.md5[3], digest.md5[4], digest.md5[5], digest.md5[6], digest.md5[7]
				, digest.md5[8], digest.md5[9], digest.md5[10], digest.md5[11], digest.md5[12], digest.md5[13], digest.md5[14], digest.md5[15]);
			buf[127] = 0;
			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"md5", NULL, buf))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			_snwprintf(buf, sizeof(buf) / sizeof(buf[0])
				, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
				, digest.sha[0], digest.sha[1], digest.sha[2], digest.sha[3], digest.sha[4]
				, digest.sha[5], digest.sha[6], digest.sha[7], digest.sha[8], digest.sha[9]
				, digest.sha[10], digest.sha[11], digest.sha[12], digest.sha[13], digest.sha[14]
				, digest.sha[15], digest.sha[16], digest.sha[17], digest.sha[18], digest.sha[19]);
			buf[127] = 0;
			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha1", NULL, buf))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			if (pExtArg->byDatExpand) {
				WCHAR buf2[256] = {};
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest.sha224[0], digest.sha224[1], digest.sha224[2], digest.sha224[3], digest.sha224[4], digest.sha224[5], digest.sha224[6]
					, digest.sha224[7], digest.sha224[8], digest.sha224[9], digest.sha224[10], digest.sha224[11], digest.sha224[12], digest.sha224[13]
					, digest.sha224[14], digest.sha224[15], digest.sha224[16], digest.sha224[17], digest.sha224[18], digest.sha224[19], digest.sha224[20]
					, digest.sha224[21], digest.sha224[22], digest.sha224[23], digest.sha224[24], digest.sha224[25], digest.sha224[26], digest.sha224[27]);
				buf2[255] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha224", NULL, buf2))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
					return FALSE;
				}
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest.sha256[0], digest.sha256[1], digest.sha256[2], digest.sha256[3], digest.sha256[4], digest.sha256[5], digest.sha256[6], digest.sha256[7]
					, digest.sha256[8], digest.sha256[9], digest.sha256[10], digest.sha256[11], digest.sha256[12], digest.sha256[13], digest.sha256[14], digest.sha256[15]
					, digest.sha256[16], digest.sha256[17], digest.sha256[18], digest.sha256[19], digest.sha256[20], digest.sha256[21], digest.sha256[22], digest.sha256[23]
					, digest.sha256[24], digest.sha256[25], digest.sha256[26], digest.sha256[27], digest.sha256[28], digest.sha256[29], digest.sha256[30], digest.sha256[31]);
				buf2[255] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha256", NULL, buf2))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
					return FALSE;
				}
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest.sha384[0], digest.sha384[1], digest.sha384[2], digest.sha384[3], digest.sha384[4], digest.sha384[5], digest.sha384[6], digest.sha384[7]
					, digest.sha384[8], digest.sha384[9], digest.sha384[10], digest.sha384[11], digest.sha384[12], digest.sha384[13], digest.sha384[14], digest.sha384[15]
					, digest.sha384[16], digest.sha384[17], digest.sha384[18], digest.sha384[19], digest.sha384[20], digest.sha384[21], digest.sha384[22], digest.sha384[23]
					, digest.sha384[24], digest.sha384[25], digest.sha384[26], digest.sha384[27], digest.sha384[28], digest.sha384[29], digest.sha384[30], digest.sha384[31]
					, digest.sha384[32], digest.sha384[33], digest.sha384[34], digest.sha384[35], digest.sha384[36], digest.sha384[37], digest.sha384[38], digest.sha384[39]
					, digest.sha384[40], digest.sha384[41], digest.sha384[42], digest.sha384[43], digest.sha384[44], digest.sha384[45], digest.sha384[46], digest.sha384[47]);
				buf2[255] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha384", NULL, buf2))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
					return FALSE;
				}
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest.sha512[0], digest.sha512[1], digest.sha512[2], digest.sha512[3], digest.sha512[4], digest.sha512[5], digest.sha512[6], digest.sha512[7]
					, digest.sha512[8], digest.sha512[9], digest.sha512[10], digest.sha512[11], digest.sha512[12], digest.sha512[13], digest.sha512[14], digest.sha512[15]
					, digest.sha512[16], digest.sha512[17], digest.sha512[18], digest.sha512[19], digest.sha512[20], digest.sha512[21], digest.sha512[22], digest.sha512[23]
					, digest.sha512[24], digest.sha512[25], digest.sha512[26], digest.sha512[27], digest.sha512[28], digest.sha512[29], digest.sha512[30], digest.sha512[31]
					, digest.sha512[32], digest.sha512[33], digest.sha512[34], digest.sha512[35], digest.sha512[36], digest.sha512[37], digest.sha512[38], digest.sha512[39]
					, digest.sha512[40], digest.sha512[41], digest.sha512[42], digest.sha512[43], digest.sha512[44], digest.sha512[45], digest.sha512[46], digest.sha512[47]
					, digest.sha512[48], digest.sha512[49], digest.sha512[50], digest.sha512[51], digest.sha512[52], digest.sha512[53], digest.sha512[54], digest.sha512[55]
					, digest.sha512[56], digest.sha512[57], digest.sha512[58], digest.sha512[59], digest.sha512[60], digest.sha512[61], digest.sha512[62], digest.sha512[63]);
				buf2[255] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha512", NULL, buf2))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
					return FALSE;
				}
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0]), L"%08llx", digest.xxh3_64);
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"xxh3_64", NULL, buf2))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
					return FALSE;
				}
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0]), L"%08llx%08llx", digest.xxh3_128.high64, digest.xxh3_128.low64);
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
			newElem4->SetAttribute("name", hash.szFnameAndExt);

			CHAR buf[128] = {};
			_snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%llu", hash.ui64FileSize);
			buf[127] = 0;
			newElem4->SetAttribute("size", buf);

			_snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%08lx", hash.crc32);
			buf[127] = 0;
			newElem4->SetAttribute("crc", buf);

			_snprintf(buf, sizeof(buf) / sizeof(buf[0])
				, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
				, digest.md5[0], digest.md5[1], digest.md5[2], digest.md5[3], digest.md5[4], digest.md5[5], digest.md5[6], digest.md5[7]
				, digest.md5[8], digest.md5[9], digest.md5[10], digest.md5[11], digest.md5[12], digest.md5[13], digest.md5[14], digest.md5[15]);
			buf[127] = 0;
			newElem4->SetAttribute("md5", buf);

			_snprintf(buf, sizeof(buf) / sizeof(buf[0])
				, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
				, digest.sha[0], digest.sha[1], digest.sha[2], digest.sha[3], digest.sha[4]
				, digest.sha[5], digest.sha[6], digest.sha[7], digest.sha[8], digest.sha[9]
				, digest.sha[10], digest.sha[11], digest.sha[12], digest.sha[13], digest.sha[14]
				, digest.sha[15], digest.sha[16], digest.sha[17], digest.sha[18], digest.sha[19]);
			buf[127] = 0;
			newElem4->SetAttribute("sha1", buf);

			if (pExtArg->byDatExpand) {
				CHAR buf2[256] = {};
				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest.sha224[0], digest.sha224[1], digest.sha224[2], digest.sha224[3], digest.sha224[4], digest.sha224[5], digest.sha224[6]
					, digest.sha224[7], digest.sha224[8], digest.sha224[9], digest.sha224[10], digest.sha224[11], digest.sha224[12], digest.sha224[13]
					, digest.sha224[14], digest.sha224[15], digest.sha224[16], digest.sha224[17], digest.sha224[18], digest.sha224[19], digest.sha224[20]
					, digest.sha224[21], digest.sha224[22], digest.sha224[23], digest.sha224[24], digest.sha224[25], digest.sha224[26], digest.sha224[27]);
				buf2[255] = 0;
				newElem4->SetAttribute("sha224", buf2);

				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest.sha256[0], digest.sha256[1], digest.sha256[2], digest.sha256[3], digest.sha256[4], digest.sha256[5], digest.sha256[6], digest.sha256[7]
					, digest.sha256[8], digest.sha256[9], digest.sha256[10], digest.sha256[11], digest.sha256[12], digest.sha256[13], digest.sha256[14], digest.sha256[15]
					, digest.sha256[16], digest.sha256[17], digest.sha256[18], digest.sha256[19], digest.sha256[20], digest.sha256[21], digest.sha256[22], digest.sha256[23]
					, digest.sha256[24], digest.sha256[25], digest.sha256[26], digest.sha256[27], digest.sha256[28], digest.sha256[29], digest.sha256[30], digest.sha256[31]);
				buf2[255] = 0;
				newElem4->SetAttribute("sha256", buf2);

				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest.sha384[0], digest.sha384[1], digest.sha384[2], digest.sha384[3], digest.sha384[4], digest.sha384[5], digest.sha384[6], digest.sha384[7]
					, digest.sha384[8], digest.sha384[9], digest.sha384[10], digest.sha384[11], digest.sha384[12], digest.sha384[13], digest.sha384[14], digest.sha384[15]
					, digest.sha384[16], digest.sha384[17], digest.sha384[18], digest.sha384[19], digest.sha384[20], digest.sha384[21], digest.sha384[22], digest.sha384[23]
					, digest.sha384[24], digest.sha384[25], digest.sha384[26], digest.sha384[27], digest.sha384[28], digest.sha384[29], digest.sha384[30], digest.sha384[31]
					, digest.sha384[32], digest.sha384[33], digest.sha384[34], digest.sha384[35], digest.sha384[36], digest.sha384[37], digest.sha384[38], digest.sha384[39]
					, digest.sha384[40], digest.sha384[41], digest.sha384[42], digest.sha384[43], digest.sha384[44], digest.sha384[45], digest.sha384[46], digest.sha384[47]);
				buf2[255] = 0;
				newElem4->SetAttribute("sha384", buf2);

				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest.sha512[0], digest.sha512[1], digest.sha512[2], digest.sha512[3], digest.sha512[4], digest.sha512[5], digest.sha512[6], digest.sha512[7]
					, digest.sha512[8], digest.sha512[9], digest.sha512[10], digest.sha512[11], digest.sha512[12], digest.sha512[13], digest.sha512[14], digest.sha512[15]
					, digest.sha512[16], digest.sha512[17], digest.sha512[18], digest.sha512[19], digest.sha512[20], digest.sha512[21], digest.sha512[22], digest.sha512[23]
					, digest.sha512[24], digest.sha512[25], digest.sha512[26], digest.sha512[27], digest.sha512[28], digest.sha512[29], digest.sha512[30], digest.sha512[31]
					, digest.sha512[32], digest.sha512[33], digest.sha512[34], digest.sha512[35], digest.sha512[36], digest.sha512[37], digest.sha512[38], digest.sha512[39]
					, digest.sha512[40], digest.sha512[41], digest.sha512[42], digest.sha512[43], digest.sha512[44], digest.sha512[45], digest.sha512[46], digest.sha512[47]
					, digest.sha512[48], digest.sha512[49], digest.sha512[50], digest.sha512[51], digest.sha512[52], digest.sha512[53], digest.sha512[54], digest.sha512[55]
					, digest.sha512[56], digest.sha512[57], digest.sha512[58], digest.sha512[59], digest.sha512[60], digest.sha512[61], digest.sha512[62], digest.sha512[63]);
				buf2[255] = 0;
				newElem4->SetAttribute("sha512", buf2);

				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0]), "%08lx", digest.xxh3_64);
				buf2[255] = 0;
				newElem4->SetAttribute("xxh3_64", buf2);

				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0]), "%08lx%08lx", digest.xxh3_128.high64, digest.xxh3_128.low64);
				buf2[255] = 0;
				newElem4->SetAttribute("xxh3_128", buf2);
			}
			pWriter->InsertEndChild(newElem4);
#endif
		}
	}
	return TRUE;
}

BOOL OutputRomElement(
#ifdef _WIN32
	CComPtr<IXmlWriter> pWriter,
#else
	XMLElement* pWriter,
#endif
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	_TCHAR* pszFullPath,
	_TCHAR* szPath,
	SUB_DESYNC_TYPE bDesync,
	PHASH pHash
) {
	if (*pExecType == fd || *pExecType == disk) {
		if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, pDisc->dwBytesPerSector, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
			return FALSE;
		}
	}
	else if (*pExecType == dvd || IsXbox(pExecType) || *pExecType == bd || *pExecType == sacd) {
		if (*pExecType == dvd || *pExecType == xbox) {
			if (*pExecType == xbox) {
				_tcsncpy(szPath, pszFullPath, _MAX_PATH);
				PathRemoveExtension(szPath);
				if (!PathAppend(szPath, _T("_SS.bin"))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				if (!OutputHash(pWriter, pExtArg, pDisc, szPath, NOT_USE_SIZE, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
					return FALSE;
				}
			}

			_tcsncpy(szPath, pszFullPath, _MAX_PATH);
			PathRemoveExtension(szPath);
			if (!PathAppend(szPath, _T("_PFI.bin"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			if (!OutputHash(pWriter, pExtArg, pDisc, szPath, NOT_USE_SIZE, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
				return FALSE;
			}

			_tcsncpy(szPath, pszFullPath, _MAX_PATH);
			PathRemoveExtension(szPath);
			if (!PathAppend(szPath, _T("_DMI.bin"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			if (!OutputHash(pWriter, pExtArg, pDisc, szPath, NOT_USE_SIZE, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
				return FALSE;
			}
		}
		else if (*pExecType == bd) {
			_tcsncpy(szPath, pszFullPath, _MAX_PATH);
			PathRemoveExtension(szPath);
			if (!PathAppend(szPath, _T("_PIC.bin"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			if (!OutputHash(pWriter, pExtArg, pDisc, szPath, NOT_USE_SIZE, _T(".bin"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
				return FALSE;
			}
		}
		if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, DISC_MAIN_DATA_SIZE, _T(".iso"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
			return FALSE;
		}
		if (pExtArg->byRawDump) {
			if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, NOT_USE_SIZE, _T(".raw"), 1, 1, SUB_DESYNC_TYPE::NoDesync, pHash)) {
				return FALSE;
			}
		}
	}
	else {
		if (bDesync == SUB_DESYNC_TYPE::NoDesync) {
			OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("Hash(Whole image)"));
			if (pDisc->SCSI.trkType == TRACK_TYPE::dataExist ||
				pDisc->SCSI.trkType == TRACK_TYPE::pregapDataIn1stTrack) {
				if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".scm"), 1, 1, bDesync, pHash)) {
					return FALSE;
				}
			}
			if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".img"), 1, 1, bDesync, pHash)) {
				return FALSE;
			}
		}
		else if (bDesync == SUB_DESYNC_TYPE::CtlDesync) {
			if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".img"), 1, 1, bDesync, pHash)) {
				return FALSE;
			}
		}
		for (UCHAR i = pDisc->SCSI.toc.FirstTrack; i <= pDisc->SCSI.toc.LastTrack; i++) {
			if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".bin"), i, pDisc->SCSI.toc.LastTrack, bDesync, pHash)) {
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL ReadWriteDat(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	_TCHAR* pszFullPath,
	_TCHAR* szDir,
	SUB_DESYNC_TYPE bDesync,
	PHASH pHash
) {
#ifdef _WIN32
	WCHAR wszDefaultDat[_MAX_PATH] = {};
	if (!GetModuleFileNameW(NULL, wszDefaultDat, sizeof(wszDefaultDat) / sizeof(wszDefaultDat[0]))) {
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

	WCHAR wszPathForDat[_MAX_PATH] = {};
	_TCHAR szTmpPath[_MAX_PATH] = {};

	_tcsncpy(szTmpPath, pszFullPath, sizeof(szTmpPath) / sizeof(_TCHAR) - 1);

	if (bDesync == SUB_DESYNC_TYPE::IdxDesync) {
		PathRemoveExtension(szTmpPath);
		_TCHAR str1[] = _T(" (Subs indexes).dat");
		_tcsncat(szTmpPath, str1, sizeof(szTmpPath) / sizeof(_TCHAR) - _tcslen(szTmpPath) - 1);
	}
	else if (bDesync == SUB_DESYNC_TYPE::CtlDesync) {
		PathRemoveExtension(szTmpPath);
		_TCHAR str1[] = _T(" (Subs control).dat");
		_tcsncat(szTmpPath, str1, sizeof(szTmpPath) / sizeof(_TCHAR) - _tcslen(szTmpPath) - 1);
	}
	else {
		PathRenameExtension(szTmpPath, _T(".dat"));
	}

	szTmpPath[_MAX_FNAME - 1] = 0;
#ifndef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0
		, szTmpPath, sizeof(szTmpPath) / sizeof(szTmpPath[0])
		, wszPathForDat, sizeof(wszPathForDat) / sizeof(wszPathForDat[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#else
	size_t size = sizeof(wszPathForDat) / sizeof(wszPathForDat[0]);
	wcsncpy(wszPathForDat, szTmpPath, size);
	wszPathForDat[size - 1] = 0;
#endif

	CComPtr<IXmlWriter> pWriter;
	CComPtr<IStream> pWriteStream;
	if (FAILED(hr = SHCreateStreamOnFileW(wszPathForDat, STGM_CREATE | STGM_WRITE, &pWriteStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
	if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString("Dat error: %08.8lx\n", hr);
		return FALSE;
	}
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

	WCHAR wszDir[_MAX_DIR] = {};
#ifndef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, szDir, _MAX_DIR, wszDir, sizeof(wszDir) / sizeof(wszDir[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#else
	size = sizeof(wszDir) / sizeof(wszDir[0]);
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
					}
				} while (S_OK == pReader->MoveToNextAttribute());
			}
			else {
				if (FAILED(hr = pWriter->WriteStartElement(NULL, pwszLocalName, NULL))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
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
				OutputRomElement(pWriter, pExecType, pExtArg, pDisc, pszFullPath,	szTmpPath, bDesync, pHash);
			}
			if (FAILED(hr = pWriter->WriteEndElement())) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
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
		if (!GetModuleFileName(NULL, szDefaultDat, sizeof(szDefaultDat) / sizeof(szDefaultDat[0]))) {
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

	CHAR szPathForDat[_MAX_PATH] = {};
	_TCHAR szTmpPath[_MAX_PATH] = {};

	_tcsncpy(szPathForDat, pszFullPath, sizeof(szPathForDat) - 1);

	if (bDesync) {
		PathRemoveExtension(szPathForDat);
		_TCHAR str1[] = _T(" (Subs indexes).dat");
		_tcsncat(szPathForDat, str1, sizeof(szPathForDat) - _tcslen(szPathForDat) - 1);
	}
	else {
		PathRenameExtension(szPathForDat, _T(".dat"));
	}
	szPathForDat[_MAX_PATH - 1] = 0;

	XMLDocument xmlWriter;
	XMLDeclaration* decl = xmlWriter.NewDeclaration();
	xmlWriter.InsertEndChild(decl);
	XMLUnknown* unk = xmlWriter.NewUnknown("DOCTYPE datafile PUBLIC \"-//Logiqx//DTD ROM Management Datafile//EN\" \"http://www.logiqx.com/Dats/datafile.dtd\"");
	xmlWriter.InsertEndChild(unk);

	XMLElement* readElem = xmlReader.FirstChildElement();
	BOOL bDescription = FALSE;
	// datafile
	while (readElem) {
		XMLElement* newElem1 = xmlWriter.NewElement(readElem->Name());
		if (readElem->GetText() == NULL) {
			newElem1->SetText("\n");
		}
		else {
			newElem1->SetText(readElem->GetText());
		}

		XMLElement* readElem2 = readElem->FirstChildElement();
		// header, game
		while (readElem2) {
			XMLElement* newElem2 = newElem1->GetDocument()->NewElement(readElem2->Name());
			if (readElem2->GetText() == NULL) {
				if (!strcmp(readElem2->Name(), "game")) {
					newElem2->SetAttribute("name", "-insert name-");
				}
				else {
					newElem2->SetText("\n");
				}
			}
			else {
				newElem2->SetText(readElem2->GetText());
			}

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
						bDescription = TRUE;
					}
					else {
						newElem3->SetText(readElem3->GetText());
					}
				}
				newElem2->InsertEndChild(newElem3);
				readElem3 = readElem3->NextSiblingElement();
			}

			if (bDescription) {
				OutputRomElement(newElem2, pExecType, pExtArg, pDisc, pszFullPath, szTmpPath, bDesync, pHash);
			}

			newElem1->InsertEndChild(newElem2);
			readElem2 = readElem2->NextSiblingElement();
		}
		xmlWriter.InsertEndChild(newElem1);
		readElem = readElem->NextSiblingElement();
	}

	xmlWriter.SaveFile(szPathForDat);
#endif
	return TRUE;
}
