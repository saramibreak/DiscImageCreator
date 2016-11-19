// DiscImageCreator.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
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
#include "_external\prngcd.h"

#define DEFAULT_REREAD_VAL			(1024)
#define DEFAULT_MAX_C2_ERROR_VAL	(4096)
#define DEFAULT_REREAD_SPEED_VAL	(4)
#define DEFAULT_SPTD_TIMEOUT_VAL	(60)

BYTE g_aSyncHeader[SYNC_SIZE] = {
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};

// These static variable is set at printAndSetPath().
static _TCHAR s_szCurrentdir[_MAX_PATH];
static _TCHAR s_szDrive[_MAX_DRIVE];
static _TCHAR s_szDir[_MAX_DIR];
static _TCHAR s_szFname[_MAX_FNAME];
static _TCHAR s_szExt[_MAX_EXT];

// These static variable is set at checkArg().
static DWORD s_dwSpeed = 0;
static INT s_nStartLBA = 0;
static INT s_nEndLBA = 0;

#define playtime (200)
#define c4 (262)
#define d4 (294)
#define e4 (330)
#define f4 (349)
#define g4 (392)
#define a4 (440)
#define b4 (494)
#define c5 (523)
#define d5 (587)
#define e5 (659)
#define f5 (698)
#define g5 (784)
#define a5 (880)
#define b5 (988)
#define c6 (1047)

int soundBeep(int nRet)
{
	if (nRet) {
		if (!Beep(c4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(d4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(e4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(f4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(g4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(a4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(b4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(c5, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
	}
	else {
		if (!Beep(c5, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(b4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(a4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(g4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(f4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(e4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(d4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
		if (!Beep(c4, playtime)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		};
	}
	return TRUE;
}

BOOL GetCreatedFileList(
	PHANDLE h,
	PWIN32_FIND_DATA lp,
	LPTSTR szPathWithoutFileName,
	size_t stPathSize
	)
{
	// "*" is wild card
	_sntprintf(szPathWithoutFileName, stPathSize, _T("%s\\%s\\*"), s_szDrive, s_szDir);
	szPathWithoutFileName[stPathSize - 1] = 0;

	*h = FindFirstFile(szPathWithoutFileName, lp);
	if (*h == INVALID_HANDLE_VALUE) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("%s\n"), szPathWithoutFileName);
		return FALSE;
	}
	else {
		// delete '*'
		szPathWithoutFileName[_tcslen(szPathWithoutFileName) - 1] = '\0';
	}
	return TRUE;
}

int calculatingHash(CComPtr<IXmlWriter> pWriter)
{
	HANDLE h = NULL;
	WIN32_FIND_DATA lp = { 0 };
	_TCHAR szPathWithoutFileName[_MAX_PATH] = { 0 };
	BOOL bRet = GetCreatedFileList(
		&h, &lp, szPathWithoutFileName, sizeof(szPathWithoutFileName));
	if (!bRet) {
		return FALSE;
	}
	_TCHAR szPathForCalc[_MAX_PATH] = { 0 };
	_TCHAR szExt[_MAX_EXT] = { 0 };
	WCHAR wszFnameForDat[_MAX_PATH] = { 0 };
	// http://msdn.microsoft.com/en-us/library/aa364428%28v=vs.85%29.aspx
	//  The order in which the search returns the files, such as alphabetical order, is not guaranteed, and is dependent on the file system. 
	//  If the data must be sorted, the application must do the ordering after obtaining all the results.
	//  The order in which this function returns the file names is dependent on the file system type. 
	//  With the NTFS file system and CDFS file systems, the names are usually returned in alphabetical order. 
	//  With FAT file systems, the names are usually returned in the order the files were written to the disk, which may or may not be in alphabetical order.
	//  However, as stated previously, these behaviors are not guaranteed.
	do {
		if ((lp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) {
			_tsplitpath(lp.cFileName, NULL, NULL, NULL, szExt);
			if (!_tcsncmp(s_szFname, lp.cFileName, _tcslen(s_szFname)) &&
				(!_tcsncmp(szExt, _T(".bin"), 4) || !_tcsncmp(szExt, _T(".iso"), 4) ||
				!_tcsncmp(szExt, _T(".img"), 4))) {
				_sntprintf(szPathForCalc, sizeof(szPathForCalc) / sizeof(szPathForCalc[0]),
					_T("%s%s"), szPathWithoutFileName, lp.cFileName);
				FILE* fp = CreateOrOpenFile(
					szPathForCalc, NULL, NULL, NULL, NULL, szExt, _T("rb"), 0, 0);
				if (!fp) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					break;
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
					OutputString(_T("Calculating hash: %s\n"), lp.cFileName);
					// TODO: This code can more speed up! if reduce calling fread()
					for (UINT64 i = 0; i < ui64SectorSizeAll; i++) {
						fread(data, sizeof(BYTE), dwSectorSizeOne, fp);
						bRet = CalcHash(&crc32, &context,
							&sha, data, dwSectorSizeOne);
						if (!bRet) {
							break;
						}
					}
					FcloseAndNull(fp);
					if (!bRet) {
						break;
					}
					BYTE digest[16] = { 0 };
					BYTE Message_Digest[20] = { 0 };
					bRet = CalcEnd(&context, &sha, digest, Message_Digest);
					if (!bRet) {
						break;
					}
					if (!_tcsncmp(szExt, _T(".img"), 4)) {
#ifndef _DEBUG
						OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(Hash(entire image)));
						OutputHashData(g_LogFile.fpDisc, lp.cFileName,
							ui64FileSize, crc32, digest, Message_Digest);
#endif
					}
					else {
						HRESULT hr = S_OK;
						if (FAILED(hr = pWriter->WriteStartElement(NULL, L"rom", NULL))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							bRet = FALSE;
							break;
						}
#ifndef UNICODE
						if(!MultiByteToWideChar(CP_ACP, 0
							, lp.cFileName, sizeof(lp.cFileName) / sizeof(lp.cFileName[0])
							, wszFnameForDat, sizeof(wszFnameForDat) / sizeof(wszFnameForDat[0]))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							bRet = FALSE;
							break;
						}
#else
						size_t size = sizeof(wszFnameForDat) / sizeof(wszFnameForDat[0]);
						wcsncpy(wszFnameForDat, lp.cFileName, size);
						wszFnameForDat[size - 1] = 0;
#endif
						if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"name", NULL, wszFnameForDat))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							bRet = FALSE;
							break;
						}
						WCHAR buf[128] = { 0 };
						_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%llu", ui64FileSize);
						buf[127] = 0;
						if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"size", NULL, buf))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							bRet = FALSE;
							break;
						}
						_snwprintf(buf, sizeof(buf) / sizeof(buf[0]), L"%08lx", crc32);
						buf[127] = 0;
						if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"crc", NULL, buf))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							bRet = FALSE;
							break;
						}
						_snwprintf(buf, sizeof(buf) / sizeof(buf[0])
							, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
							, digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], digest[6], digest[7]
							, digest[8], digest[9], digest[10], digest[11], digest[12], digest[13], digest[14], digest[15]);
						buf[127] = 0;
						if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"md5", NULL, buf))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							bRet = FALSE;
							break;
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
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							bRet = FALSE;
							break;
						}
						if (FAILED(hr = pWriter->WriteEndElement())) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							bRet = FALSE;
							break;
						}
					}
				}
			}
		}
	} while (FindNextFile(h, &lp));
	OutputString(_T("\n"));
	FindClose(h);
	return bRet;
}

int readWriteDat(_TCHAR* pszFullPath)
{
	WCHAR wszDir[_MAX_DIR] = { 0 };
#ifndef UNICODE
	if (!MultiByteToWideChar(CP_ACP, 0
		, s_szDir, sizeof(s_szDir) / sizeof(s_szDir[0])
		, wszDir, sizeof(wszDir) / sizeof(wszDir[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#else
	size_t size = sizeof(wszDir) / sizeof(wszDir[0]);
	wcsncpy(wszDir, s_szDir, size);
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
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	if (FAILED(hr = pReader->SetInput(pReadStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Parse))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}

	WCHAR wszPathForDat[_MAX_PATH] = { 0 };
#ifndef UNICODE
	if(!MultiByteToWideChar(CP_ACP, 0
		, pszFullPath, sizeof(s_szCurrentdir) / sizeof(s_szCurrentdir[0])
		, wszPathForDat, sizeof(wszPathForDat) / sizeof(wszPathForDat[0]))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#else
	size = sizeof(wszPathForDat) / sizeof(wszPathForDat[0]);
	wcsncpy(wszPathForDat, pszFullPath, size);
	wszPathForDat[size - 1] = 0;
#endif
	if(!PathRenameExtensionW(wszPathForDat, L".dat")) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	CComPtr<IXmlWriter> pWriter;
	CComPtr<IStream> pWriteStream;
	if (FAILED(hr = SHCreateStreamOnFileW(wszPathForDat, STGM_CREATE | STGM_WRITE, &pWriteStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(&pWriter), 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->SetOutput(pWriteStream))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}

	if (FAILED(hr = pWriter->WriteStartDocument(XmlStandalone_Omit))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->WriteDocType(L"datafile"
		, L"-//Logiqx//DTD ROM Management Datafile//EN", L"http://www.logiqx.com/Dats/datafile.dtd", NULL))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
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
				OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
				return FALSE;
			}
			else if (!pwszLocalName) {
				OutputErrorString(_T("[L:%d] LocalName is NULL\n"), __LINE__);
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
						OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
						return FALSE;
					}
					else if (!pwszAttributeName) {
						OutputErrorString(_T("[L:%d] AttributeName is NULL\n"), __LINE__);
						return FALSE;
					}
					if (!wcsncmp(pwszAttributeName, L"name", 4)) {
						if (FAILED(hr = pWriter->WriteStartElement(NULL, pwszLocalName, NULL))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							return FALSE;
						}
						if (FAILED(hr = pWriter->WriteAttributeString(NULL, pwszAttributeName, NULL, pCurrentDir))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
							return FALSE;
						}
					}
				} while (S_OK == pReader->MoveToNextAttribute());
			}
			else {
				if (FAILED(hr = pWriter->WriteStartElement(NULL, pwszLocalName, NULL))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
					return FALSE;
				}
			}
			break;
		case XmlNodeType_Text:
			if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
				return FALSE;
			}
			else if (!pwszValue) {
				OutputErrorString(_T("Error GetValue, NULL, L:%d\n"), __LINE__);
				return FALSE;
			}
			if (!wcsncmp(pwszLocalName, L"description", 11) && !wcsncmp(pwszValue, L"foo", 3)) {
				if (FAILED(hr = pWriter->WriteString(pCurrentDir))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
					return FALSE;
				}
			}
			else {
				if (FAILED(hr = pWriter->WriteString(pwszValue))) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
					return FALSE;
				}
			}
			break;
		case XmlNodeType_EndElement:
			if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
				return FALSE;
			}
			else if (!pwszLocalName) {
				OutputErrorString(_T("[L:%d] LocalName is NULL\n"), __LINE__);
				return FALSE;
			}
			if (!wcsncmp(pwszLocalName, L"game", 4)) {
				if (!calculatingHash(pWriter)) {
					return FALSE;
				}
			}
			if (FAILED(hr = pWriter->WriteEndElement())) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
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
				OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
				return FALSE;
			}
			if (FAILED(hr = pWriter->WriteWhitespace(L"\t"))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
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
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	if (FAILED(hr = pWriter->Flush())) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("[L:%d] Dat error: %08.8lx\n"), __LINE__, hr);
		return FALSE;
	}
	return TRUE;
}

int exec(_TCHAR* argv[], PEXEC_TYPE pExecType, PEXT_ARG pExtArg, _TCHAR* pszFullPath)
{
	BOOL bRet = TRUE;
	SetLastError(NO_ERROR);

	if (*pExecType == sub) {
		bRet = WriteParsingSubfile(pszFullPath);
	}
	else {
		_TCHAR szBuf[8] = { 0 };
		_sntprintf(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("\\\\.\\%c:"), argv[2][0]);
		szBuf[7] = 0;
		DEVICE devData = { 0 };
		devData.hDevice = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (devData.hDevice == INVALID_HANDLE_VALUE) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
#if 0
		DWORD dwSize = 0;
		GET_LENGTH_INFORMATION tLenInf;
		DeviceIoControl(devData.hDevice, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &tLenInf, sizeof(tLenInf), &dwSize, NULL );
		OutputString(_T("%llx\n"), tLenInf.Length.QuadPart);
#define LODWORD(l)           ((DWORD)(((UINT64)(l)) & 0xffffffff))
#define HIDWORD(l)           ((DWORD)((((UINT64)(l)) >> 32) & 0xffffffff))
		OutputString(_T("%x\n"), LODWORD(tLenInf.Length.QuadPart));
		OutputString(_T("%x\n"), HIDWORD(tLenInf.Length.QuadPart));
		if (!LockFile(devData.hDevice, 0, 0, 0, 0)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		DWORD dwReturned = 0;
		CDROM_EXCLUSIVE_ACCESS exclusive;
		exclusive.RequestType = ExclusiveAccessQueryState;
		exclusive.Flags = 0;

		CDROM_EXCLUSIVE_LOCK_STATE lockstate = { 0 };
		bRet = DeviceIoControl(devData.hDevice, IOCTL_CDROM_EXCLUSIVE_ACCESS,
			&exclusive, sizeof(CDROM_EXCLUSIVE_ACCESS), &lockstate,
			sizeof(CDROM_EXCLUSIVE_LOCK_STATE), &dwReturned, NULL);
		printf("RequestType %u, Flags %u\n", exclusive.RequestType, exclusive.Flags);
		printf("LockState %u, CallerName %s\n", lockstate.LockState, lockstate.CallerName);

		CDROM_EXCLUSIVE_LOCK lock;
		exclusive.RequestType = ExclusiveAccessLockDevice;
		lock.Access = exclusive;
		bRet = DeviceIoControl(devData.hDevice, IOCTL_CDROM_EXCLUSIVE_ACCESS,
			&lock, sizeof(CDROM_EXCLUSIVE_LOCK), &lock,
			sizeof(CDROM_EXCLUSIVE_LOCK), &dwReturned, NULL);
		printf("RequestType %u, CallerName %s\n", lock.Access.RequestType, lock.CallerName);
#endif
		// 1st: set TimeOutValue here (because use ScsiPassThroughDirect)
		if (pExtArg->byReadContinue) {
			devData.dwTimeOutValue = pExtArg->dwTimeoutNum;
		}
		else {
			devData.dwTimeOutValue = DEFAULT_SPTD_TIMEOUT_VAL;
		}
		if (*pExecType == stop) {
			bRet = StartStopUnit(pExtArg, &devData, STOP_UNIT_CODE, STOP_UNIT_CODE);
		}
		else if (*pExecType == start) {
			bRet = StartStopUnit(pExtArg, &devData, START_UNIT_CODE, STOP_UNIT_CODE);
		}
		else if (*pExecType == eject) {
			bRet = StartStopUnit(pExtArg, &devData, STOP_UNIT_CODE, START_UNIT_CODE);
		}
		else if (*pExecType == closetray) {
			bRet = StartStopUnit(pExtArg, &devData, START_UNIT_CODE, START_UNIT_CODE);
		}
		else if (*pExecType == reset) {
			bRet = Reset(pExtArg, &devData);
		}
		else {
			DISC discData = { '\0' };
			PDISC pDisc = &discData;
			MAIN_HEADER mainHeader = { 0 };
			FILE* fpCcd = NULL;
			try {
#ifndef _DEBUG
				// 2nd: create logfile here (because logging all working)
				if (!InitLogFile(pExecType, pExtArg, pszFullPath)) {
					throw FALSE;
				}
#endif
				if (!TestUnitReady(pExtArg, &devData)) {
					throw FALSE;
				}
				if (*pExecType == fd) {
					if (!DiskGetMediaTypes(&devData, pszFullPath)) {
						throw FALSE;
					}
				}
				else {
					ReadDriveInformation(pExtArg, &devData, pDisc, s_dwSpeed);
					ReadDiscInformation(pExtArg, &devData);
					if (*pExecType != dvd) {
						if (!ReadTOC(pExtArg, pExecType, &devData, pDisc)) {
							throw FALSE;
						}
					}
					if (discData.SCSI.wCurrentMedia == ProfileCdrom || 
						discData.SCSI.wCurrentMedia == ProfileCdRecordable ||
						discData.SCSI.wCurrentMedia == ProfileCdRewritable ||
						(discData.SCSI.wCurrentMedia == ProfileInvalid && (*pExecType == gd))) {
						if (!pExtArg->byC2) {
							OutputString(
								_T("Warning: /c2 option isn't set. The result of ripping may not be correct if c2 error exists.\n"));
						}
						// 6th: open ccd here (because use ReadTOCFull and from there)
						if (*pExecType == cd && !pExtArg->byReverse) {
							fpCcd = CreateOrOpenFile(pszFullPath, NULL,
								NULL, NULL, NULL, _T(".ccd"), _T(WFLAG), 0, 0);
							if (!fpCcd) {
								OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
								throw FALSE;
							}
						}
						InitMainDataHeader(pExecType, pExtArg, &mainHeader);
						if (!InitSubData(pExecType, &pDisc)) {
							throw FALSE;
						}
						if (!InitTocFullData(pExecType, &pDisc)) {
							throw FALSE;
						}
						if (!InitTocTextData(pExecType, &devData, &pDisc)) {
							throw FALSE;
						}
						if (!InitProtectData(&pDisc)) {
							throw FALSE;
						}
						make_scrambled_table();
						make_crc_table();
						MakeCrc16CCITTTable();
#if 0
						MakeCrc6ITUTable();
#endif
						if (*pExecType != data) {
							if (!ReadCDForSearchingOffset(pExecType, pExtArg, &devData, pDisc)) {
								throw FALSE;
							}
							if (!ReadCDForCheckingReadInOut(pExtArg, &devData, pDisc)) {
								throw FALSE;
							}
						}
						if (!ReadTOCFull(pExtArg, &devData, &discData, fpCcd)) {
							throw FALSE;
						}
						if (!pExtArg->byReverse) {
							// Typically, CD+G data is included in audio only disc
							// But exceptionally, WonderMega Collection (SCD)(mixed disc) exists CD+G data.
							if (!ReadCDForCheckingSubRtoW(pExtArg, &devData, pDisc)) {
								throw FALSE;
							}
							if (!pDisc->SCSI.byAudioOnly) {
								if (!ReadCDForFileSystem(pExtArg, &devData, pDisc)) {
									throw FALSE;
								}
							}
						}
						if (*pExecType == cd) {
							bRet = ReadCDAll(pExecType, pExtArg,
								&devData, pDisc, &mainHeader, pszFullPath, fpCcd);
						}
						else if (*pExecType == gd) {
							if (!ReadCDForGDTOC(pExtArg, &devData, pDisc)) {
								throw FALSE;
							}
							bRet = ReadCDPartial(pExecType, pExtArg, &devData,
								pDisc, &mainHeader, pszFullPath, FIRST_LBA_FOR_GD,
								549149 + 1, CDFLAG::_READ_CD::CDDA);
						}
						else if (*pExecType == data) {
							bRet = ReadCDPartial(pExecType, pExtArg, &devData,
								pDisc, &mainHeader, pszFullPath, s_nStartLBA,
								s_nEndLBA, CDFLAG::_READ_CD::All);
						}
						else if (*pExecType == audio) {
							bRet = ReadCDPartial(pExecType, pExtArg, &devData,
								pDisc, &mainHeader, pszFullPath, s_nStartLBA,
								s_nEndLBA, CDFLAG::_READ_CD::CDDA);
						}
					}
					else if (discData.SCSI.wCurrentMedia == ProfileDvdRom || 
						discData.SCSI.wCurrentMedia == ProfileDvdRecordable ||
						discData.SCSI.wCurrentMedia == ProfileDvdRam || 
						discData.SCSI.wCurrentMedia == ProfileDvdRewritable || 
						discData.SCSI.wCurrentMedia == ProfileDvdRWSequential || 
						discData.SCSI.wCurrentMedia == ProfileDvdDashRDualLayer || 
						discData.SCSI.wCurrentMedia == ProfileDvdDashRLayerJump || 
						discData.SCSI.wCurrentMedia == ProfileDvdPlusRW || 
//						discData.SCSI.wCurrentMedia == ProfileInvalid ||
						discData.SCSI.wCurrentMedia == ProfileDvdPlusR) {
						bRet = ReadDVDStructure(pExtArg, &devData, &discData);
						FlushLog();

						if (pExtArg->byCmi) {
							bRet = ReadDVDForCMI(pExtArg, &devData, &discData);
						}
						if (bRet) {
							if (argv[5] && !_tcsncmp(argv[5], _T("raw"), 3)) {
#if 0
								bRet = ReadDVDRaw(&devData, &discData, szVendorId, pszFullPath);
#endif
							}
							else {
								bRet = ReadDVD(pExtArg, &devData, &discData, pszFullPath);
							}
						}
					}
					if (bRet && (*pExecType == cd || *pExecType == dvd || *pExecType == gd)) {
						bRet = readWriteDat(pszFullPath);
					}
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			TerminateLBAPerTrack(&pDisc);
			TerminateSubData(pExecType, &pDisc);
			TerminateProtectData(&pDisc);
			TerminateTocFullData(&pDisc);
			if (devData.bySuccessReadToc) {
				TerminateTocTextData(pExecType, &devData, &pDisc);
			}
			FcloseAndNull(fpCcd);
#ifndef _DEBUG
			TerminateLogFile(pExecType, pExtArg);
#endif
		}
#if 0
		if (!UnlockFile(devData.hDevice, 0, 0, 0, 0)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
#endif
		if (devData.hDevice && !CloseHandle(devData.hDevice)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	return bRet;
}

int printAndSetPath(_TCHAR* szPathFromArg, _TCHAR* pszFullPath)
{
	if (!GetCurrentDirectory(sizeof(s_szCurrentdir) / sizeof(s_szCurrentdir[0]), s_szCurrentdir)) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	_tsplitpath(szPathFromArg, s_szDrive, s_szDir, s_szFname, s_szExt);

	if (!s_szDrive[0] || !s_szDir[0]) {
		_tcsncpy(pszFullPath, s_szCurrentdir, _MAX_PATH);
		pszFullPath[_MAX_PATH] = 0;
		if (s_szDir[0]) {
			if (!PathAppend(pszFullPath, s_szDir)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		if (!PathFileExists(pszFullPath)) {
			OutputErrorString(_T("%s doesn't exist, so create.\n"), pszFullPath);
			if (!CreateDirectory(pszFullPath, NULL)) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		if (!PathAppend(pszFullPath, s_szFname)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		_tsplitpath(pszFullPath, s_szDrive, s_szDir, s_szFname, NULL);
	}
	else {
		_tcsncpy(pszFullPath, szPathFromArg, _MAX_PATH);
	}
	OutputString(
		_T("CurrentDirectory\n")
		_T("\t%s\n")
		_T("WorkingPath\n")
		_T("\t Argument: %s\n")
		_T("\t FullPath: %s\n")
		_T("\t    Drive: %s\n")
		_T("\tDirectory: %s\n")
		_T("\t Filename: %s\n")
		_T("\tExtension: %s\n"),
		s_szCurrentdir, szPathFromArg, pszFullPath, s_szDrive, s_szDir, s_szFname, s_szExt);

	return TRUE;
}

int SetOptionS(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->dwSubAddionalNum = _tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->dwSubAddionalNum = 1;
		OutputString(_T("/s val is omitted. set [%d]\n"), 1);
	}
	return TRUE;
}

int SetOptionRc(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byReadContinue = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->dwTimeoutNum = _tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->dwTimeoutNum = DEFAULT_SPTD_TIMEOUT_VAL;
		OutputString(
			_T("/rc val is omitted. set [%d]\n"), DEFAULT_SPTD_TIMEOUT_VAL);
	}
	return TRUE;
}

int SetOptionC2(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byC2 = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->dwMaxRereadNum = _tcstoul(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
			pExtArg->dwMaxC2ErrorNum = _tcstoul(argv[(*i)++], &endptr, 10) * 2;
			if (*endptr) {
				OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
				return FALSE;
			}
			else if (pExtArg->dwMaxC2ErrorNum == 0) {
				OutputString(
					_T("/c2 val2 is 0. Changed default val:[%d]\n")
					, DEFAULT_MAX_C2_ERROR_VAL);
				pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL * 2;
			}
			if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
				pExtArg->dwRereadSpeedNum = _tcstoul(argv[(*i)++], &endptr, 10);
				if (*endptr) {
					OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
					return FALSE;
				}
			}
			else {
				pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
				OutputString(
					_T("/c2 val3 is omitted. set [%d]\n")
					, DEFAULT_REREAD_SPEED_VAL);
			}
		}
		else {
			pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL * 2;
			pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
			OutputString(
				_T("/c2 val2 is omitted. set [%d]\n")
				_T("/c2 val3 is omitted. set [%d]\n")
				, DEFAULT_MAX_C2_ERROR_VAL
				, DEFAULT_REREAD_SPEED_VAL);
		}
	}
	else {
		pExtArg->dwMaxRereadNum = DEFAULT_REREAD_VAL;
		pExtArg->dwMaxC2ErrorNum = DEFAULT_MAX_C2_ERROR_VAL * 2;
		pExtArg->dwRereadSpeedNum = DEFAULT_REREAD_SPEED_VAL;
		OutputString(
			_T("/c2 val1 is omitted. set [%d]\n")
			_T("/c2 val2 is omitted. set [%d]\n")
			_T("/c2 val3 is omitted. set [%d]\n")
			, DEFAULT_REREAD_VAL
			, DEFAULT_MAX_C2_ERROR_VAL
			, DEFAULT_REREAD_SPEED_VAL);
	}
	return TRUE;
}

int SetOptionBe(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	pExtArg->byBe = TRUE;
	pExtArg->byD8 = FALSE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		if (!_tcsncmp(argv[(*i)++], _T("pack"), 4)) {
			pExtArg->byPack = TRUE;
		}
		else if (!_tcsncmp(argv[(*i)++], _T("raw"), 3)) {
			pExtArg->byRaw = TRUE;
		}
		else {
			OutputErrorString(_T("Bad arg: [%s] Please enter pack or raw\n"), argv[*i]);
			return FALSE;
		}
	}
	else {
		pExtArg->byRaw = TRUE;
		OutputString(_T("submode of /be is omitted. set [raw]\n"));
	}
	return TRUE;
}

int SetOptionA(int argc, _TCHAR* argv[], PEXT_ARG pExtArg, int* i)
{
	_TCHAR* endptr = NULL;
	pExtArg->byAdd = TRUE;
	if (argc > *i && _tcsncmp(argv[*i], _T("/"), 1)) {
		pExtArg->nAudioCDOffsetNum = _tcstol(argv[(*i)++], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
	}
	else {
		pExtArg->nAudioCDOffsetNum = 0;
		OutputString(_T("/a val is omitted. set [%d]\n"), 0);
	}
	return TRUE;
}

int checkArg(int argc, _TCHAR* argv[], PEXEC_TYPE pExecType, PEXT_ARG pExtArg, _TCHAR* pszFullPath)
{
	_TCHAR* endptr = NULL;
	if (argc >= 5 && (!_tcsncmp(argv[1], _T("cd"), 2) || !_tcsncmp(argv[1], _T("gd"), 2))) {
		s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		pExtArg->dwSubAddionalNum = 1;
		for (INT i = 6; i <= argc; i++) {
			if (!_tcsncmp(argv[i - 1], _T("/a"), 2)) {
				if (!SetOptionA(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else if (!_tcsncmp(argv[i - 1], _T("/be"), 3)) {
				if (!SetOptionBe(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else if (!_tcsncmp(argv[i - 1], _T("/d8"), 3)) {
				pExtArg->byBe = FALSE;
				pExtArg->byD8 = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/c2"), 3)) {
				if (!SetOptionC2(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else if (!_tcsncmp(argv[i - 1], _T("/f"), 2)) {
				pExtArg->byFua = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/l"), 2)) {
				pExtArg->byLibCrypt = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/m"), 2)) {
				pExtArg->byMCN = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/p"), 2)) {
				pExtArg->byPre = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/rc"), 3)) {
				if (!SetOptionRc(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else if (!_tcsncmp(argv[i - 1], _T("/raw"), 4)) {
				pExtArg->byRawDump = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/r"), 2)) {
				pExtArg->byReverse = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/se"), 3)) {
				pExtArg->byIntentionalSub = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/s"), 2)) {
				if (!SetOptionS(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else {
				OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
				return FALSE;
			}
		}
		if (!_tcsncmp(argv[1], _T("cd"), 2)) {
			*pExecType = cd;
		}
		else if (!_tcsncmp(argv[1], _T("gd"), 2)) {
			*pExecType = gd;
		}
		printAndSetPath(argv[3], pszFullPath);
	}
	else if (argc >= 5 && !_tcsncmp(argv[1], _T("dvd"), 3)) {
		s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		for (INT i = 6; i <= argc; i++) {
			if (!_tcsncmp(argv[i - 1], _T("/c"), 2)) {
				pExtArg->byCmi = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/f"), 2)) {
				pExtArg->byFua = TRUE;
			}
			else {
				OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
				return FALSE;
			}
		}
		*pExecType = dvd;
		printAndSetPath(argv[3], pszFullPath);
	}
	else if (argc >= 7 && (!_tcsncmp(argv[1], _T("data"), 4) || !_tcsncmp(argv[1], _T("audio"), 5))) {
		s_dwSpeed = _tcstoul(argv[4], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		s_nStartLBA = _tcstol(argv[5], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}
		s_nEndLBA = _tcstol(argv[6], &endptr, 10);
		if (*endptr) {
			OutputErrorString(_T("Bad arg: [%s] Please integer\n"), endptr);
			return FALSE;
		}

		for (INT i = 8; i <= argc; i++) {
			if (!_tcsncmp(argv[i - 1], _T("/a"), 2)) {
				if (!SetOptionA(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else if (!_tcsncmp(argv[i - 1], _T("/be"), 3)) {
				if (!SetOptionBe(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else if (!_tcsncmp(argv[i - 1], _T("/d8"), 3)) {
				pExtArg->byBe = FALSE;
				pExtArg->byD8 = TRUE;
			}
			else if (!_tcsncmp(argv[i - 1], _T("/c2"), 3)) {
				if (!SetOptionC2(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else if (!_tcsncmp(argv[i - 1], _T("/s"), 2)) {
				if (!SetOptionS(argc, argv, pExtArg, &i)) {
					return FALSE;
				}
			}
			else {
				OutputErrorString(_T("Unknown option: [%s]\n"), argv[i - 1]);
				return FALSE;
			}
		}
		if (!_tcsncmp(argv[1], _T("data"), 4)) {
			*pExecType = data;
		}
		else if (!_tcsncmp(argv[1], _T("audio"), 5)) {
			*pExecType = audio;
		}
		printAndSetPath(argv[3], pszFullPath);
	}
	else if (argc == 4 && !_tcsncmp(argv[1], _T("fd"), 2)) {
		*pExecType = fd;
		printAndSetPath(argv[3], pszFullPath);
	}
	else if (argc == 3 && !_tcsncmp(argv[1], _T("stop"), 4)) {
		*pExecType = stop;
	}
	else if (argc == 3 && !_tcsncmp(argv[1], _T("start"), 5)) {
		*pExecType = start;
	}
	else if (argc == 3 && !_tcsncmp(argv[1], _T("eject"), 5)) {
		*pExecType = eject;
	}
	else if (argc == 3 && !_tcsncmp(argv[1], _T("close"), 5)) {
		*pExecType = closetray;
	}
	else if (argc == 3 && !_tcsncmp(argv[1], _T("reset"), 5)) {
		*pExecType = reset;
	}
	else if (argc == 3 && !_tcsncmp(argv[1], _T("sub"), 3)) {
		*pExecType = sub;
		printAndSetPath(argv[2], pszFullPath);
	}
	else {
		if (argc > 1) {
			OutputErrorString(_T("Invalid argument\n"));
		}
		return FALSE;
	}
	return TRUE;
}

int createCmdFile(int argc, _TCHAR* argv[], _TCHAR* pszFullPath)
{
	if (argc >= 4) {
		FILE* fpCmd = CreateOrOpenFile(
			pszFullPath, _T("_cmd"), NULL, NULL, NULL, _T(".txt"), _T(WFLAG), 0, 0);
		if (!fpCmd) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		fwrite(_T(__DATE__), _tcslen(_T(__DATE__)), sizeof(_TCHAR), fpCmd);
		_fputts(_T(" "), fpCmd);
		fwrite(_T(__TIME__), _tcslen(_T(__TIME__)), sizeof(_TCHAR), fpCmd);
		_fputts(_T("\n"), fpCmd);
		for (int i = 0; i < argc; i++) {
			fwrite(argv[i], _tcslen(argv[i]), sizeof(_TCHAR), fpCmd);
			_fputts(_T(" "), fpCmd);
		}
		FcloseAndNull(fpCmd);
	}
	return TRUE;
}

void printUsage(void)
{
	OutputString(
		_T("Usage\n")
		_T("\tcd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/a (val)]\n")
		_T("\t   [/be (str) or /d8] [/c2 (val1) (val2) (val3)] [/f] [/l] [/m] [/p]\n")
		_T("\t   [/r] [/raw] [/rc (val)] [/s (val)] [/se]\n")
		_T("\t\tRipping a CD from a to z\n")
		_T("\t\tFor PLEXTOR or drive that can scramble ripping\n")
		_T("\tdata <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t     [/be (str) or /d8] [/c2 (val1) (val2) (val3)] [/s (val)]\n")
		_T("\t\tRipping a CD from start to end (using 'all' flag)\n")
		_T("\t\tFor no PLEXTOR or drive that can't scramble ripping\n")
		_T("\taudio <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>\n")
		_T("\t      [/a (val)]\n")
		_T("\t\tRipping a CD from start to end (using 'cdda' flag)\n")
		_T("\t\tFor dumping a lead-in, lead-out mainly\n")
		_T("\tgd <DriveLetter> <Filename> <DriveSpeed(0-72)>\n")
		_T("\t   [/c2 (val1) (val2) (val3)]\n")
		_T("\t\tRipping a HD area of GD from a to z\n")
		_T("\tdvd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/c] [/f]\n")
		);
	_tsystem(_T("pause"));
	OutputString(
		_T("\t\tRipping a DVD from a to z\n")
		_T("\tfd <DriveLetter> <Filename>\n")
		_T("\t\tRipping a floppy disk\n")
		_T("\tstop <DriveLetter>\n")
		_T("\t\tSpin off the disc\n")
		_T("\tstart <DriveLetter>\n")
		_T("\t\tSpin up the disc\n")
		_T("\teject <DriveLetter>\n")
		_T("\t\tEject the tray\n")
		_T("\tclose <DriveLetter>\n")
		_T("\t\tClose the tray\n")
		_T("\treset <DriveLetter>\n")
		_T("\t\tReset the drive (Only PLEXTOR)\n")
		_T("\tsub <Subfile>\n")
		_T("\t\tParse CloneCD sub file\n")
		_T("Option Info\n")
		_T("\t/a\tAdd CD offset manually (Only Audio CD)\n")
		_T("\t\t\tval\tsamples value\n")
		_T("\t/be\tUse 0xbe as ReadCD command forcibly (for data disc)\n")
		_T("\t\t\tstr\t raw: sub channel mode is raw (default)\n")
		_T("\t\t\t   \tpack: sub channel mode is pack\n")
		_T("\t/d8\tUse 0xd8 as ReadCD command forcibly (for data disc)\n")
		_T("\t/c2\tContinue to read cd to recover C2 error existing sector\n")
		_T("\t\t\tval1\tvalue to reread (default: 1024)\n")
		);
	_tsystem(_T("pause"));
	OutputString(
		_T("\t\t\tval2\tvalue to fix a C2 error (default: 4096)\n")
		_T("\t\t\tval3\tvalue to reread speed (default: 4)\n")
		_T("\t/f\tUse 'Force Unit Access' flag to defeat the cache (very slow)\n")
		_T("\t/l\tNot fix SubQ (RMSF, AMSF, CRC) (RMSFs 03:08:00 - 03:35:74)\n")
		_T("\t  \t                               (RMSFs 09:20:00 - 09:50:74)\n")
		_T("\t\t\tFor PlayStation LibCrypt discs\n")
		_T("\t/m\tIf MCN exists in the first pregap sector of the track, use this\n")
		_T("\t\t\tFor some PC-Engine discs\n")
		_T("\t/p\tRipping AMSF from 00:00:00 to 00:01:74\n")
		_T("\t\t\tFor SagaFrontier Original Sound Track (Disc 3) etc.\n")
		_T("\t\t\tSupport drive: PLEXTOR PX-W5224, PREMIUM, PREMIUM2\n")
		_T("\t\t\t               PX-704, 708, 712, 714, 716, 755, 760\n")
		_T("\t/r\tReverse reading CD (including data track)\n")
		_T("\t\t\tFor Alpha-Disc, very slow\n")
		_T("\t/raw\tReading CD all (=including lead-in/out)\n")
		_T("\t\t\tFor raw dumping\n")
		_T("\t/rc\tIf read error exists, continue reading and ignore c2 error on\n")
		_T("\t   \tspecific sector (Only CD)\n")
		_T("\t\t\tFor CodeLock, LaserLock, RingPROTECH, safedisc, smartE\n")
		_T("\t\t\tval\ttimeout value (default: 60)\n")
		_T("\t/s\tIf it reads subchannel precisely, use this\n")
		_T("\t\t\tval\t0: no read next sub (fast, but lack precision)\n")
		_T("\t\t\t   \t1: read next sub (normal, this val is default)\n")
		_T("\t\t\t   \t2: read next & next next sub (slow, precision)\n")
		);
	_tsystem(_T("pause"));
	OutputString(
		_T("\t/se\tNot fix SubQ (RMSF, AMSF, CRC) (RMSFs 01:06:50 - 04:02:74)\n")
		_T("\t   \t                            or (RMSFs 08:55:50 - 09:38:74)\n")
		_T("\t\t\tFor intentional subchannel error of a SecuRom\n")
		_T("\t/c\tLog Copyright Management Information (Only DVD)\n")
		);
	_tsystem(_T("pause"));
}

int printSeveralInfo()
{
	if (!OutputWindowsVer()) {
		return FALSE;
	}
	OutputString(_T("AppVersion\n"));
#ifdef _WIN64
	OutputString(_T("\tx64, "));
#else
	OutputString(_T("\tx86, "));
#endif
#ifdef UNICODE
	OutputString(_T("UnicodeBuild, "));
#else
	OutputString(_T("AnsiBuild, "));
#endif
	OutputString(_T("%s %s\n"), _T(__DATE__), _T(__TIME__));
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
#ifdef UNICODE
	if (_setmode(_fileno(stdin), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
	if (_setmode(_fileno(stdout), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
	if (_setmode(_fileno(stderr), _O_U8TEXT) == -1) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
#endif
	int nRet = TRUE;
	HANDLE hMutex = CreateMutex(NULL, FALSE, _T("DiscImageCreator"));
	if (!hMutex || 
		GetLastError() == ERROR_INVALID_HANDLE || 
		GetLastError() == ERROR_ALREADY_EXISTS) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return EXIT_FAILURE;
	}
#if 0
	HANDLE hSnapshot;
	if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
    
		if (Process32First(hSnapshot,&pe32)) {
			do {
				if (!_tcscmp(pe32.szExeFile, _T("IsoBuster.exe"))) {
					OutputErrorString(_T("Please close %s\n"), pe32.szExeFile);
					CloseHandle(hSnapshot);
					soundBeep(FALSE);
					return FALSE;
				}
			} while(Process32Next(hSnapshot,&pe32));
		}
		CloseHandle(hSnapshot);
	}
#endif
	nRet = printSeveralInfo();
	if (nRet) {
		EXEC_TYPE execType;
		EXT_ARG extArg = { 0 };
		_TCHAR szFullPath[_MAX_PATH + 1] = { 0 };
		if (!checkArg(argc, argv, &execType, &extArg, szFullPath)) {
			printUsage();
			nRet = FALSE;
		}
		else {
			time_t now;
			struct tm* ts;
			_TCHAR szBuf[128] = { 0 };

			now = time(NULL);
			ts = localtime(&now);
			_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
			OutputString(_T("Start time: %s\n"), szBuf);

			nRet = createCmdFile(argc, argv, szFullPath);
			if (nRet) {
				nRet = exec(argv, &execType, &extArg, szFullPath);
			}

			now = time(NULL);
			ts = localtime(&now);
			_tcsftime(szBuf, sizeof(szBuf) / sizeof(szBuf[0]), _T("%Y-%m-%d(%a) %H:%M:%S"), ts);
			OutputString(_T("End time: %s\n"), szBuf);
		}
		if (!CloseHandle(hMutex)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			nRet = FALSE;
		}
	}
	nRet = soundBeep(nRet);
#ifdef _DEBUG
	_tsystem(_T("pause"));
#endif
	return nRet = nRet == TRUE ? EXIT_SUCCESS : EXIT_FAILURE;
}

