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
	BOOL bDesync,
	PHASH pHash
) {
	_TCHAR szFnameAndExt[_MAX_FNAME + _MAX_EXT] = {};
	DWORD crc32 = 0;
	MD5_CTX md5 = {};
	SHA1Context sha = {};
	SHA224Context sha224 = {};
	SHA256Context sha256 = {};
	SHA384Context sha384 = {};
	SHA512Context sha512 = {};
	UINT64 ui64FileSize = 0;

	DWORD crc32Uni = 0;
	MD5_CTX md5Uni = {};
	SHA1Context shaUni = {};
	SHA224Context sha224Uni = {};
	SHA256Context sha256Uni = {};
	SHA384Context sha384Uni = {};
	SHA512Context sha512Uni = {};

	UINT ui1stNonZeroSector = 0;
	UINT ui1stNonZeroSectorPos = 0;
	UINT uiLastNonZeroSector = 0;
	UINT uiLastNonZeroSectorPos = 0;

	if (pHash->uiMax == 0 || (pExtArg->byRawDump && !_tcsncmp(szExt, _T(".iso"), 4))) {
		// for CD
		_TCHAR szOutPath[_MAX_PATH] = {};
		FILE* fp = NULL;
		if (bDesync) {
			fp = CreateOrOpenFile(pszFullPath, _T(" (Subs indexes)"), szOutPath
				, szFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack);
		}
		else {
			fp = CreateOrOpenFile(pszFullPath, NULL, szOutPath
				, szFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack);
		}
		if (!fp) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			OutputErrorString(" => %s\n", szOutPath);
			return FALSE;
		}

		ui64FileSize = GetFileSize64(0, fp);
		UINT64 ui64SectorSizeAll = ui64FileSize / (UINT64)dwBytesPerSector;

		if (ui64FileSize >= dwBytesPerSector) {
			BYTE data[CD_RAW_SECTOR_SIZE] = {};
			int nRet = TRUE;

			if (!_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
				// Check last non-zero byte position for Audio CD
				for (UINT64 j = 1; j <= ui64SectorSizeAll; j++) {
					fseek(fp, (LONG)(ui64FileSize - dwBytesPerSector * j), SEEK_SET);
					if (fread(data, sizeof(BYTE), dwBytesPerSector, fp) < dwBytesPerSector) {
						OutputErrorString("Failed to read: read size %lu [F:%s][L:%d]\n", dwBytesPerSector, _T(__FUNCTION__), __LINE__);
						return FALSE;
					};
					BOOL bLastNonZero = TRUE;
					for (INT k = (INT)(dwBytesPerSector - 1); 0 <= k; k--) {
						if (data[k] != 0) {
							bLastNonZero = FALSE;
							uiLastNonZeroSectorPos = (UINT)k;
							break;
						}
					}
					if (!bLastNonZero) {
						uiLastNonZeroSector = (UINT)(ui64SectorSizeAll - j);
						break;
					}
				}
				CalcInit(&md5Uni, &shaUni);
				if (pExtArg->byDatExpand) {
					CalcInitExpand(&sha224Uni, &sha256Uni, &sha384Uni, &sha512Uni);
				}
				rewind(fp);
			}

			BOOL b1stNonZero = FALSE;
			BOOL bCompleteUniHash = FALSE;

			CalcInit(&md5, &sha);
			if (pExtArg->byDatExpand) {
				CalcInitExpand(&sha224, &sha256, &sha384, &sha512);
			}

			OutputString("Hashing: %s\n", szFnameAndExt);

			for (UINT64 i = 1; i <= ui64SectorSizeAll; i++) {
				if (fread(data, sizeof(BYTE), dwBytesPerSector, fp) < dwBytesPerSector) {
					OutputErrorString("Failed to read: read size %lu [F:%s][L:%d]\n", dwBytesPerSector, _T(__FUNCTION__), __LINE__);
					return FALSE;
				};
				nRet = CalcHash(&crc32, &md5, &sha, data, (UINT)dwBytesPerSector);
				if (!nRet) {
					break;
				}
				if (pExtArg->byDatExpand) {
					nRet = CalcHashExpand(&sha224, &sha256, &sha384, &sha512, data, (UINT)dwBytesPerSector);
					if (!nRet) {
						break;
					}
				}
				if (!_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly && !bCompleteUniHash) {
					// Calc hash from the 1st non-zero byte position to the last non-zero byte position
					if (b1stNonZero) {
						if (i == static_cast<unsigned long long>(uiLastNonZeroSector) + 1) {
							nRet = CalcHash(&crc32Uni, &md5Uni, &shaUni, data, uiLastNonZeroSectorPos + 1);
							if (!nRet) {
								break;
							}
							if (pExtArg->byDatExpand) {
								nRet = CalcHashExpand(&sha224Uni, &sha256Uni, &sha384Uni, &sha512Uni, data, uiLastNonZeroSectorPos + 1);
								if (!nRet) {
									break;
								}
							}
							bCompleteUniHash = TRUE;
						}
						else {
							nRet = CalcHash(&crc32Uni, &md5Uni, &shaUni, data, (UINT)dwBytesPerSector);
							if (!nRet) {
								break;
							}
							if (pExtArg->byDatExpand) {
								nRet = CalcHashExpand(&sha224Uni, &sha256Uni, &sha384Uni, &sha512Uni, data, (UINT)dwBytesPerSector);
								if (!nRet) {
									break;
								}
							}
						}
					}
					else {
						for (UINT k = 0; k < dwBytesPerSector; k++) {
							if (data[k] != 0) {
								b1stNonZero = TRUE;
								nRet = CalcHash(&crc32Uni, &md5Uni, &shaUni, data + k, (UINT)(dwBytesPerSector - k));
								if (!nRet) {
									break;
								}
								if (pExtArg->byDatExpand) {
									nRet = CalcHashExpand(&sha224Uni, &sha256Uni, &sha384Uni, &sha512Uni, data + k, (UINT)(dwBytesPerSector - k));
									if (!nRet) {
										break;
									}
								}
								ui1stNonZeroSector = (UINT)i;
								ui1stNonZeroSectorPos = k;
								break;
							}
						}
					}
				}
			}
			FcloseAndNull(fp);
			if (!nRet) {
				return nRet;
			}
		}
	}
	else {
		// for DVD, BD
		crc32 = pHash->pHashChunk[pHash->uiCount].crc32;
		memcpy(&md5, &pHash->pHashChunk[pHash->uiCount].md5, sizeof(md5));
		memcpy(&sha, &pHash->pHashChunk[pHash->uiCount].sha, sizeof(sha));
		if (pExtArg->byDatExpand) {
			memcpy(&sha224, &pHash->pHashChunk[pHash->uiCount].sha224, sizeof(sha224));
			memcpy(&sha256, &pHash->pHashChunk[pHash->uiCount].sha256, sizeof(sha256));
			memcpy(&sha384, &pHash->pHashChunk[pHash->uiCount].sha384, sizeof(sha384));
			memcpy(&sha512, &pHash->pHashChunk[pHash->uiCount].sha512, sizeof(sha512));
		}
		_tcsncpy(szFnameAndExt, pHash->pHashChunk[pHash->uiCount].szFnameAndExt, sizeof(szFnameAndExt));
		ui64FileSize = pHash->pHashChunk[pHash->uiCount].ui64FileSize;
		pHash->uiCount++;
	}

	BYTE digest[16] = {};
	BYTE Message_Digest[20] = {};
	BYTE Message_Digest224[28] = {};
	BYTE Message_Digest256[32] = {};
	BYTE Message_Digest384[48] = {};
	BYTE Message_Digest512[64] = {};

	BYTE digestUni[16] = {};
	BYTE Message_DigestUni[20] = {};
	BYTE Message_Digest224Uni[28] = {};
	BYTE Message_Digest256Uni[32] = {};
	BYTE Message_Digest384Uni[48] = {};
	BYTE Message_Digest512Uni[64] = {};

	if (CalcEnd(&md5, &sha, digest, Message_Digest)) {
		if (pExtArg->byDatExpand) {
			CalcEndExpand(&sha224, &sha256, &sha384, &sha512
				, Message_Digest224, Message_Digest256, Message_Digest384, Message_Digest512);
		}
		if (!_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
			if (CalcEnd(&md5Uni, &shaUni, digestUni, Message_DigestUni)) {
				if (pExtArg->byDatExpand) {
					CalcEndExpand(&sha224Uni, &sha256Uni, &sha384Uni, &sha512Uni
						, Message_Digest224Uni, Message_Digest256Uni, Message_Digest384Uni, Message_Digest512Uni);
				}
			}
		}
		if (!_tcsncmp(szExt, _T(".scm"), 4) ||
			!_tcsncmp(szExt, _T(".img"), 4) ||
			!_tcsncmp(szExt, _T(".raw"), 4) ||
			find_last_string(szFnameAndExt, _T("_SS.bin")) ||
			find_last_string(szFnameAndExt, _T("_PFI.bin")) ||
			find_last_string(szFnameAndExt, _T("_DMI.bin")) ||
			find_last_string(szFnameAndExt, _T("_PIC.bin"))
			) {
#ifndef _DEBUG
			OutputHashData(pExtArg, g_LogFile.fpDisc, szFnameAndExt, ui64FileSize, crc32, digest
				, Message_Digest, Message_Digest224, Message_Digest256, Message_Digest384, Message_Digest512);
			if (!_tcsncmp(szExt, _T(".img"), 4) && pDisc->SCSI.trkType == TRACK_TYPE::audioOnly) {
				OutputDiscLog(
					OUTPUT_DHYPHEN_PLUS_STR("Hash(Universal Whole image)")
					"\t 1st non-zero byte position: %6u sector + %4u byte\n"
					"\tLast non-zero byte position: %6u sector + %4u byte\n"
					, ui1stNonZeroSector, ui1stNonZeroSectorPos, uiLastNonZeroSector, uiLastNonZeroSectorPos
				);
				UINT64 ui64UniSize = (UINT64)((uiLastNonZeroSector - ui1stNonZeroSector + 1) * dwBytesPerSector + uiLastNonZeroSectorPos - ui1stNonZeroSectorPos + 1);
				OutputHashData(pExtArg, g_LogFile.fpDisc, szFnameAndExt, ui64UniSize, crc32Uni, digestUni
					, Message_DigestUni, Message_Digest224Uni, Message_Digest256Uni, Message_Digest384Uni, Message_Digest512Uni);
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
				, szFnameAndExt, sizeof(szFnameAndExt) / sizeof(szFnameAndExt[0])
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
			_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%llu", ui64FileSize);
			buf[127] = 0;
			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"size", NULL, buf))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%08lx", crc32);
			buf[127] = 0;
			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"crc", NULL, buf))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			_snwprintf(buf, sizeof(buf) / sizeof(buf[0])
				, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
				, digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7]
				, digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);
			buf[127] = 0;
			if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"md5", NULL, buf))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString("Dat error: %08.8lx\n", hr);
				return FALSE;
			}
			_snwprintf(buf, sizeof(buf) / sizeof(buf[0])
				, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
				, Message_Digest[0], Message_Digest[1], Message_Digest[2], Message_Digest[3], Message_Digest[4]
				, Message_Digest[5], Message_Digest[6], Message_Digest[7], Message_Digest[8], Message_Digest[9]
				, Message_Digest[10], Message_Digest[11], Message_Digest[12], Message_Digest[13], Message_Digest[14]
				, Message_Digest[15], Message_Digest[16], Message_Digest[17], Message_Digest[18], Message_Digest[19]);
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
					, Message_Digest224[0], Message_Digest224[1], Message_Digest224[2], Message_Digest224[3], Message_Digest224[4], Message_Digest224[5], Message_Digest224[6]
					, Message_Digest224[7], Message_Digest224[8], Message_Digest224[9], Message_Digest224[10], Message_Digest224[11], Message_Digest224[12], Message_Digest224[13]
					, Message_Digest224[14], Message_Digest224[15], Message_Digest224[16], Message_Digest224[17], Message_Digest224[18], Message_Digest224[19], Message_Digest224[20]
					, Message_Digest224[21], Message_Digest224[22], Message_Digest224[23], Message_Digest224[24], Message_Digest224[25], Message_Digest224[26], Message_Digest224[27]);
				buf2[255] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha224", NULL, buf2))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
					return FALSE;
				}
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, Message_Digest256[0], Message_Digest256[1], Message_Digest256[2], Message_Digest256[3], Message_Digest256[4], Message_Digest256[5], Message_Digest256[6], Message_Digest256[7]
					, Message_Digest256[8], Message_Digest256[9], Message_Digest256[10], Message_Digest256[11], Message_Digest256[12], Message_Digest256[13], Message_Digest256[14], Message_Digest256[15]
					, Message_Digest256[16], Message_Digest256[17], Message_Digest256[18], Message_Digest256[19], Message_Digest256[20], Message_Digest256[21], Message_Digest256[22], Message_Digest256[23]
					, Message_Digest256[24], Message_Digest256[25], Message_Digest256[26], Message_Digest256[27], Message_Digest256[28], Message_Digest256[29], Message_Digest256[30], Message_Digest256[31]);
				buf2[255] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha256", NULL, buf2))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
					return FALSE;
				}
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, Message_Digest384[0], Message_Digest384[1], Message_Digest384[2], Message_Digest384[3], Message_Digest384[4], Message_Digest384[5], Message_Digest384[6], Message_Digest384[7]
					, Message_Digest384[8], Message_Digest384[9], Message_Digest384[10], Message_Digest384[11], Message_Digest384[12], Message_Digest384[13], Message_Digest384[14], Message_Digest384[15]
					, Message_Digest384[16], Message_Digest384[17], Message_Digest384[18], Message_Digest384[19], Message_Digest384[20], Message_Digest384[21], Message_Digest384[22], Message_Digest384[23]
					, Message_Digest384[24], Message_Digest384[25], Message_Digest384[26], Message_Digest384[27], Message_Digest384[28], Message_Digest384[29], Message_Digest384[30], Message_Digest384[31]
					, Message_Digest384[32], Message_Digest384[33], Message_Digest384[34], Message_Digest384[35], Message_Digest384[36], Message_Digest384[37], Message_Digest384[38], Message_Digest384[39]
					, Message_Digest384[40], Message_Digest384[41], Message_Digest384[42], Message_Digest384[43], Message_Digest384[44], Message_Digest384[45], Message_Digest384[46], Message_Digest384[47]);
				buf2[255] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha384", NULL, buf2))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString("Dat error: %08.8lx\n", hr);
					return FALSE;
				}
				_snwprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, Message_Digest512[0], Message_Digest512[1], Message_Digest512[2], Message_Digest512[3], Message_Digest512[4], Message_Digest512[5], Message_Digest512[6], Message_Digest512[7]
					, Message_Digest512[8], Message_Digest512[9], Message_Digest512[10], Message_Digest512[11], Message_Digest512[12], Message_Digest512[13], Message_Digest512[14], Message_Digest512[15]
					, Message_Digest512[16], Message_Digest512[17], Message_Digest512[18], Message_Digest512[19], Message_Digest512[20], Message_Digest512[21], Message_Digest512[22], Message_Digest512[23]
					, Message_Digest512[24], Message_Digest512[25], Message_Digest512[26], Message_Digest512[27], Message_Digest512[28], Message_Digest512[29], Message_Digest512[30], Message_Digest512[31]
					, Message_Digest512[32], Message_Digest512[33], Message_Digest512[34], Message_Digest512[35], Message_Digest512[36], Message_Digest512[37], Message_Digest512[38], Message_Digest512[39]
					, Message_Digest512[40], Message_Digest512[41], Message_Digest512[42], Message_Digest512[43], Message_Digest512[44], Message_Digest512[45], Message_Digest512[46], Message_Digest512[47]
					, Message_Digest512[48], Message_Digest512[49], Message_Digest512[50], Message_Digest512[51], Message_Digest512[52], Message_Digest512[53], Message_Digest512[54], Message_Digest512[55]
					, Message_Digest512[56], Message_Digest512[57], Message_Digest512[58], Message_Digest512[59], Message_Digest512[60], Message_Digest512[61], Message_Digest512[62], Message_Digest512[63]);
				buf2[255] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"sha512", NULL, buf2))) {
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
			newElem4->SetAttribute("name", szFnameAndExt);

			CHAR buf[128] = {};
			_snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%llu", ui64FileSize);
			buf[127] = 0;
			newElem4->SetAttribute("size", buf);

			_snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%08lx", crc32);
			buf[127] = 0;
			newElem4->SetAttribute("crc", buf);

			_snprintf(buf, sizeof(buf) / sizeof(buf[0])
				, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
				, digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7]
				, digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);
			buf[127] = 0;
			newElem4->SetAttribute("md5", buf);

			_snprintf(buf, sizeof(buf) / sizeof(buf[0])
				, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
				, Message_Digest[0], Message_Digest[1], Message_Digest[2], Message_Digest[3], Message_Digest[4]
				, Message_Digest[5], Message_Digest[6], Message_Digest[7], Message_Digest[8], Message_Digest[9]
				, Message_Digest[10], Message_Digest[11], Message_Digest[12], Message_Digest[13], Message_Digest[14]
				, Message_Digest[15], Message_Digest[16], Message_Digest[17], Message_Digest[18], Message_Digest[19]);
			buf[127] = 0;
			newElem4->SetAttribute("sha1", buf);

			if (pExtArg->byDatExpand) {
				CHAR buf2[256] = {};
				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, Message_Digest224[0], Message_Digest224[1], Message_Digest224[2], Message_Digest224[3], Message_Digest224[4], Message_Digest224[5], Message_Digest224[6]
					, Message_Digest224[7], Message_Digest224[8], Message_Digest224[9], Message_Digest224[10], Message_Digest224[11], Message_Digest224[12], Message_Digest224[13]
					, Message_Digest224[14], Message_Digest224[15], Message_Digest224[16], Message_Digest224[17], Message_Digest224[18], Message_Digest224[19], Message_Digest224[20]
					, Message_Digest224[21], Message_Digest224[22], Message_Digest224[23], Message_Digest224[24], Message_Digest224[25], Message_Digest224[26], Message_Digest224[27]);
				buf2[255] = 0;
				newElem4->SetAttribute("sha224", buf2);

				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, Message_Digest256[0], Message_Digest256[1], Message_Digest256[2], Message_Digest256[3], Message_Digest256[4], Message_Digest256[5], Message_Digest256[6], Message_Digest256[7]
					, Message_Digest256[8], Message_Digest256[9], Message_Digest256[10], Message_Digest256[11], Message_Digest256[12], Message_Digest256[13], Message_Digest256[14], Message_Digest256[15]
					, Message_Digest256[16], Message_Digest256[17], Message_Digest256[18], Message_Digest256[19], Message_Digest256[20], Message_Digest256[21], Message_Digest256[22], Message_Digest256[23]
					, Message_Digest256[24], Message_Digest256[25], Message_Digest256[26], Message_Digest256[27], Message_Digest256[28], Message_Digest256[29], Message_Digest256[30], Message_Digest256[31]);
				buf2[255] = 0;
				newElem4->SetAttribute("sha256", buf2);

				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, Message_Digest384[0], Message_Digest384[1], Message_Digest384[2], Message_Digest384[3], Message_Digest384[4], Message_Digest384[5], Message_Digest384[6], Message_Digest384[7]
					, Message_Digest384[8], Message_Digest384[9], Message_Digest384[10], Message_Digest384[11], Message_Digest384[12], Message_Digest384[13], Message_Digest384[14], Message_Digest384[15]
					, Message_Digest384[16], Message_Digest384[17], Message_Digest384[18], Message_Digest384[19], Message_Digest384[20], Message_Digest384[21], Message_Digest384[22], Message_Digest384[23]
					, Message_Digest384[24], Message_Digest384[25], Message_Digest384[26], Message_Digest384[27], Message_Digest384[28], Message_Digest384[29], Message_Digest384[30], Message_Digest384[31]
					, Message_Digest384[32], Message_Digest384[33], Message_Digest384[34], Message_Digest384[35], Message_Digest384[36], Message_Digest384[37], Message_Digest384[38], Message_Digest384[39]
					, Message_Digest384[40], Message_Digest384[41], Message_Digest384[42], Message_Digest384[43], Message_Digest384[44], Message_Digest384[45], Message_Digest384[46], Message_Digest384[47]);
				buf2[255] = 0;
				newElem4->SetAttribute("sha384", buf2);

				_snprintf(buf2, sizeof(buf2) / sizeof(buf2[0])
					, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, Message_Digest512[0], Message_Digest512[1], Message_Digest512[2], Message_Digest512[3], Message_Digest512[4], Message_Digest512[5], Message_Digest512[6], Message_Digest512[7]
					, Message_Digest512[8], Message_Digest512[9], Message_Digest512[10], Message_Digest512[11], Message_Digest512[12], Message_Digest512[13], Message_Digest512[14], Message_Digest512[15]
					, Message_Digest512[16], Message_Digest512[17], Message_Digest512[18], Message_Digest512[19], Message_Digest512[20], Message_Digest512[21], Message_Digest512[22], Message_Digest512[23]
					, Message_Digest512[24], Message_Digest512[25], Message_Digest512[26], Message_Digest512[27], Message_Digest512[28], Message_Digest512[29], Message_Digest512[30], Message_Digest512[31]
					, Message_Digest512[32], Message_Digest512[33], Message_Digest512[34], Message_Digest512[35], Message_Digest512[36], Message_Digest512[37], Message_Digest512[38], Message_Digest512[39]
					, Message_Digest512[40], Message_Digest512[41], Message_Digest512[42], Message_Digest512[43], Message_Digest512[44], Message_Digest512[45], Message_Digest512[46], Message_Digest512[47]
					, Message_Digest512[48], Message_Digest512[49], Message_Digest512[50], Message_Digest512[51], Message_Digest512[52], Message_Digest512[53], Message_Digest512[54], Message_Digest512[55]
					, Message_Digest512[56], Message_Digest512[57], Message_Digest512[58], Message_Digest512[59], Message_Digest512[60], Message_Digest512[61], Message_Digest512[62], Message_Digest512[63]);
				buf2[255] = 0;
				newElem4->SetAttribute("sha512", buf2);
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
	BOOL bDesync,
	PHASH pHash
) {
	if (*pExecType == fd || *pExecType == disk) {
		if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, pDisc->dwBytesPerSector, _T(".bin"), 1, 1, FALSE, pHash)) {
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
				if (!OutputHash(pWriter, pExtArg, pDisc, szPath, NOT_USE_SIZE, _T(".bin"), 1, 1, FALSE, pHash)) {
					return FALSE;
				}
			}

			_tcsncpy(szPath, pszFullPath, _MAX_PATH);
			PathRemoveExtension(szPath);
			if (!PathAppend(szPath, _T("_PFI.bin"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			if (!OutputHash(pWriter, pExtArg, pDisc, szPath, NOT_USE_SIZE, _T(".bin"), 1, 1, FALSE, pHash)) {
				return FALSE;
			}

			_tcsncpy(szPath, pszFullPath, _MAX_PATH);
			PathRemoveExtension(szPath);
			if (!PathAppend(szPath, _T("_DMI.bin"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
			if (!OutputHash(pWriter, pExtArg, pDisc, szPath, NOT_USE_SIZE, _T(".bin"), 1, 1, FALSE, pHash)) {
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
			if (!OutputHash(pWriter, pExtArg, pDisc, szPath, NOT_USE_SIZE, _T(".bin"), 1, 1, FALSE, pHash)) {
				return FALSE;
			}
		}
		if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, DISC_MAIN_DATA_SIZE, _T(".iso"), 1, 1, FALSE, pHash)) {
			return FALSE;
		}
		if (pExtArg->byRawDump) {
			if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, NOT_USE_SIZE, _T(".raw"), 1, 1, FALSE, pHash)) {
				return FALSE;
			}
		}
	}
	else {
		if (!pDisc->SUB.byDesync || !bDesync) {
			OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("Hash(Whole image)"));
			if (pDisc->SCSI.trkType == TRACK_TYPE::dataExist ||
				pDisc->SCSI.trkType == TRACK_TYPE::pregapDataIn1stTrack) {
				if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".scm"), 1, 1, FALSE, pHash)) {
					return FALSE;
				}
			}
			if (!OutputHash(pWriter, pExtArg, pDisc, pszFullPath, CD_RAW_SECTOR_SIZE, _T(".img"), 1, 1, FALSE, pHash)) {
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
	BOOL bDesync,
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

	if (bDesync) {
		PathRemoveExtension(szTmpPath);
		_TCHAR str1[] = _T(" (Subs indexes).dat");
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
