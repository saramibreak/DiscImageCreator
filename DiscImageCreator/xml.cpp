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

BOOL ReadWriteDat(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	_TCHAR* pszFullPath,
	_TCHAR* szDrive,
	_TCHAR* szDir,
	_TCHAR* szFname,
	BOOL bDesync
) {
	WCHAR wszDir[_MAX_DIR] = { 0 };
#ifndef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0, szDir, _MAX_DIR, wszDir, sizeof(wszDir) / sizeof(wszDir[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#else
	size_t size = sizeof(wszDir) / sizeof(wszDir[0]);
	wcsncpy(wszDir, szDir, size);
	wszDir[size - 1] = 0;
#endif
	LPWCH p = wcsrchr(wszDir, L'\\');
	*p = NULL;
	p = wcsrchr(wszDir, L'\\');
	LPWCH pCurrentDir = p + 1;

	WCHAR wszDefaultDat[_MAX_PATH] = { 0 };
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
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	if (FAILED(hr = pReader->SetInput(pReadStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Parse))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}

	WCHAR wszPathForDat[_MAX_PATH] = { 0 };
	_TCHAR szPathWithoutExtension[_MAX_FNAME] = { 0 };
	if (bDesync) {
		_sntprintf(szPathWithoutExtension, _MAX_FNAME, _T("%s\\%s\\%s (Subs indexes)"), szDrive, szDir, szFname);
	}
	else {
		_sntprintf(szPathWithoutExtension, _MAX_FNAME, _T("%s\\%s\\%s"), szDrive, szDir, szFname);
	}
	szPathWithoutExtension[_MAX_FNAME - 1] = 0;
#ifndef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0
		, szPathWithoutExtension, sizeof(szPathWithoutExtension) / sizeof(szPathWithoutExtension[0])
		, wszPathForDat, sizeof(wszPathForDat) / sizeof(wszPathForDat[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#else
	size = sizeof(wszPathForDat) / sizeof(wszPathForDat[0]);
	wcsncpy(wszPathForDat, szPathWithoutExtension, size);
	wszPathForDat[size - 1] = 0;
#endif
	if (!PathAddExtensionW(wszPathForDat, L".dat")) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	CComPtr<IXmlWriter> pWriter;
	CComPtr<IStream> pWriteStream;
	if (FAILED(hr = SHCreateStreamOnFileW(wszPathForDat, STGM_CREATE | STGM_WRITE, &pWriteStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->SetOutput(pWriteStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}

	if (FAILED(hr = pWriter->WriteStartDocument(XmlStandalone_Omit))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->WriteDocType(L"datafile"
		, L"-//Logiqx//DTD ROM Management Datafile//EN", L"http://www.logiqx.com/Dats/datafile.dtd", NULL))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}

	XmlNodeType nodeType;
	LPCWSTR pwszLocalName = NULL;
	LPCWSTR pwszValue = NULL;
	while (S_OK == pReader->Read(&nodeType)) {
		switch (nodeType) {
		case XmlNodeType_Element:
			if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
				return FALSE;
			}
			else if (!pwszLocalName) {
				OutputErrorString(_T("[L:%d] LocalName is NULL\n"), (INT)__LINE__);
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
						OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
						return FALSE;
					}
					else if (!pwszAttributeName) {
						OutputErrorString(_T("[L:%d] AttributeName is NULL\n"), (INT)__LINE__);
						return FALSE;
					}
					if (!wcsncmp(pwszAttributeName, L"name", 4)) {
						if (FAILED(hr = pWriter->WriteStartElement(NULL, pwszLocalName, NULL))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
							return FALSE;
						}
						if (FAILED(hr = pWriter->WriteAttributeString(NULL, pwszAttributeName, NULL, pCurrentDir))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
							return FALSE;
						}
					}
				} while (S_OK == pReader->MoveToNextAttribute());
			}
			else {
				if (FAILED(hr = pWriter->WriteStartElement(NULL, pwszLocalName, NULL))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
			}
			break;
		case XmlNodeType_Text:
			if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
				return FALSE;
			}
			else if (!pwszValue) {
				OutputErrorString(_T("Error GetValue, NULL, L:%d\n"), (INT)__LINE__);
				return FALSE;
			}
			if (!wcsncmp(pwszLocalName, L"description", 11) && !wcsncmp(pwszValue, L"foo", 3)) {
				if (FAILED(hr = pWriter->WriteString(pCurrentDir))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
			}
			else {
				if (FAILED(hr = pWriter->WriteString(pwszValue))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
			}
			break;
		case XmlNodeType_EndElement:
			if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
				return FALSE;
			}
			else if (!pwszLocalName) {
				OutputErrorString(_T("[L:%d] LocalName is NULL\n"), (INT)__LINE__);
				return FALSE;
			}
			if (!wcsncmp(pwszLocalName, L"game", 4)) {
				if (*pExecType == dvd) {
					if (!OutputHash(pWriter, pszFullPath, _T(".iso"), 1, 1, FALSE)) {
						return FALSE;
					}
				}
				else {
					if (!pDisc->SUB.byDesync || !bDesync) {
						OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(Hash(Whole image)));
						if (pDisc->SCSI.trackType != TRACK_TYPE::audioOnly) {
							if (!OutputHash(pWriter, pszFullPath, _T(".scm"), 1, 1, FALSE)) {
								return FALSE;
							}
						}
						if (!OutputHash(pWriter, pszFullPath, _T(".img"), 1, 1, FALSE)) {
							return FALSE;
						}
					}
					UCHAR uiTrack = pDisc->SCSI.toc.FirstTrack;
					UCHAR uiLastTrack = pDisc->SCSI.toc.LastTrack;
					if (*pExecType == gd) {
						uiTrack = pDisc->GDROM_TOC.FirstTrack;
						uiLastTrack = pDisc->GDROM_TOC.LastTrack;
					}
					for (; uiTrack <= uiLastTrack; uiTrack++) {
						if (!OutputHash(pWriter, pszFullPath, _T(".bin"), uiTrack, uiLastTrack, bDesync)) {
							return FALSE;
						}
					}
				}

			}
			if (FAILED(hr = pWriter->WriteEndElement())) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
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
#if 0
			if (FAILED(hr = pWriter->WriteWhitespace(L"\n"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
				return FALSE;
			}
			if (FAILED(hr = pWriter->WriteWhitespace(L"\t"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
				return FALSE;
			}
#endif
			break;
		case XmlNodeType_XmlDeclaration:
			break;
		default:
			break;
		}
	}
	if (FAILED(hr = pWriter->WriteEndDocument())) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->Flush())) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
		return FALSE;
	}
	return TRUE;
}

BOOL OutputHash(
	CComPtr<IXmlWriter> pWriter,
	_TCHAR* pszFullPath,
	LPCTSTR szExt,
	UCHAR uiTrack,
	UCHAR uiLastTrack,
	BOOL bDesync
) {
	_TCHAR pszFnameAndExt[_MAX_PATH] = { 0 };
	FILE* fp = NULL;
	if (bDesync) {
		fp = CreateOrOpenFile(pszFullPath, _T(" (Subs indexes)"), NULL
			, pszFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack);
	}
	else {
		fp = CreateOrOpenFile(pszFullPath, NULL, NULL
			, pszFnameAndExt, NULL, szExt, _T("rb"), uiTrack, uiLastTrack);
	}
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	UINT64 ui64FileSize = GetFileSize64(0, fp);
	DWORD dwSectorSizeOne = CD_RAW_SECTOR_SIZE;
	if (!_tcsncmp(szExt, _T(".iso"), 4)) {
		dwSectorSizeOne = DISC_RAW_READ_SIZE;
	}
	UINT64 ui64SectorSizeAll = ui64FileSize / (UINT64)dwSectorSizeOne;

	if (ui64FileSize > dwSectorSizeOne * 10) {
		MD5_CTX context = { 0 };
		SHA1Context sha = { 0 };
		CalcInit(&context, &sha);

		BYTE data[CD_RAW_SECTOR_SIZE] = { 0 };
		DWORD crc32 = 0;
		OutputString(_T("Calculating hash: %s\n"), pszFnameAndExt);
		int nRet = TRUE;
		// TODO: This code can more speed up! if reduce calling fread()
		for (UINT64 i = 0; i < ui64SectorSizeAll; i++) {
			fread(data, sizeof(BYTE), dwSectorSizeOne, fp);
			nRet = CalcHash(&crc32, &context, &sha, data, dwSectorSizeOne);
			if (!nRet) {
				break;
			}
		}
		FcloseAndNull(fp);
		if (!nRet) {
			return nRet;
		}
		BYTE digest[16] = { 0 };
		BYTE Message_Digest[20] = { 0 };
		if (CalcEnd(&context, &sha, digest, Message_Digest)) {
			if (!_tcsncmp(szExt, _T(".scm"), 4) || !_tcsncmp(szExt, _T(".img"), 4)) {
#ifndef _DEBUG
				OutputHashData(g_LogFile.fpDisc, pszFnameAndExt,
					ui64FileSize, crc32, digest, Message_Digest);
#endif
			}
			else {
				HRESULT hr = S_OK;
				if (FAILED(hr = pWriter->WriteStartElement(NULL, L"rom", NULL))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
				WCHAR wszFnameAndExt[_MAX_PATH] = { 0 };
#ifndef UNICODE
				if (!MultiByteToWideChar(CP_ACP, 0
					, pszFnameAndExt, sizeof(pszFnameAndExt) / sizeof(pszFnameAndExt[0])
					, wszFnameAndExt, sizeof(wszFnameAndExt) / sizeof(wszFnameAndExt[0]))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					return FALSE;
				}
#else
				size_t size = sizeof(wszFnameAndExt) / sizeof(wszFnameAndExt[0]);
				wcsncpy(wszFnameAndExt, pszFnameAndExt, size);
				wszFnameAndExt[size - 1] = 0;
#endif
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"name", NULL, wszFnameAndExt))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
				WCHAR buf[128] = { 0 };
				_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%llu", ui64FileSize);
				buf[127] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"size", NULL, buf))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
				_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%08lx", crc32);
				buf[127] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"crc", NULL, buf))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
				_snwprintf(buf, sizeof(buf) / sizeof(buf[0])
					, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
					, digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7]
					, digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);
				buf[127] = 0;
				if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"md5", NULL, buf))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
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
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
				if (FAILED(hr = pWriter->WriteEndElement())) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("Dat error: %08.8lx\n"), hr);
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}
