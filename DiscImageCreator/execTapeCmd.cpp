/**
 * Copyright 2011-2020 sarami
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
#include "output.h"

BOOL ReadTape(
	_TCHAR* pszFullPath
) {
#if _WIN32
	DEVICE device = {};
#ifdef _WIN32
	device.hDevice = CreateFile(_T("\\\\.\\TAPE0"), GENERIC_READ | GENERIC_WRITE,
		0, 0, OPEN_EXISTING, 0, NULL);
#else
	pDevice->hDevice = open(pDevice->drivepath, O_RDONLY | O_NONBLOCK, 0777);
#endif
	if (device.hDevice == INVALID_HANDLE_VALUE) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	DWORD dwRet = 0;
	LPBYTE buff = NULL;
#if 0
	dwRet = CreateTapePartition(device.hDevice, TAPE_FIXED_PARTITIONS, 0, 0);
	if (dwRet != NO_ERROR) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
#endif
	BOOL bRet = TRUE;
	try {
		dwRet = PrepareTape(device.hDevice, TAPE_LOCK, FALSE);
		if (dwRet != NO_ERROR) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwSize = sizeof(TAPE_GET_MEDIA_PARAMETERS);
		TAPE_GET_MEDIA_PARAMETERS tapemedia = {};
		dwRet = GetTapeParameters(device.hDevice, GET_TAPE_MEDIA_INFORMATION, &dwSize, &tapemedia);
		if (dwRet != NO_ERROR) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		OutputDiscLog(_T(
			"TAPE_GET_MEDIA_PARAMETERS\n"
			"          Size: %lu\n"
			"      Capacity: %lld bytes\n"
			"     Remaining: %lld bytes\n"
			"     BlockSize: %lu\n"
			"PartitionCount: %lu\n"
			"WriteProtected: %s\n")
			, dwSize
			, tapemedia.Capacity.QuadPart
			, tapemedia.Remaining.QuadPart
			, tapemedia.BlockSize
			, tapemedia.PartitionCount
			, BOOLEAN_TO_STRING_YES_NO(tapemedia.WriteProtected)
		);

		dwSize = sizeof(TAPE_GET_DRIVE_PARAMETERS);
		TAPE_GET_DRIVE_PARAMETERS tapedrive;
		dwRet = GetTapeParameters(device.hDevice, GET_TAPE_DRIVE_INFORMATION, &dwSize, &tapedrive);
		if (dwRet != NO_ERROR) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		OutputDriveLog(_T(
			"TAPE_GET_DRIVE_PARAMETERS\n"
			"                 Size: %lu\n"
			"                  ECC: %s\n"
			"          Compression: %s\n"
			"          DataPadding: %s\n"
			"       ReportSetmarks: %s\n"
			"     DefaultBlockSize: %lu\n"
			"     MaximumBlockSize: %lu\n"
			"     MinimumBlockSize: %lu\n"
			"MaximumPartitionCount: %lu\n"
			"          FeaturesLow: 0x%08lx\n"
			"         FeaturesHigh: 0x%08lx\n"
			"   EOTWarningZoneSize: %lu\n")
			, dwSize
			, BOOLEAN_TO_STRING_YES_NO(tapedrive.ECC)
			, BOOLEAN_TO_STRING_YES_NO(tapedrive.Compression)
			, BOOLEAN_TO_STRING_YES_NO(tapedrive.DataPadding)
			, BOOLEAN_TO_STRING_YES_NO(tapedrive.ReportSetmarks)
			, tapedrive.DefaultBlockSize
			, tapedrive.MaximumBlockSize
			, tapedrive.MinimumBlockSize
			, tapedrive.MaximumPartitionCount
			, tapedrive.FeaturesLow
			, tapedrive.FeaturesHigh
			, tapedrive.EOTWarningZoneSize
		);
		dwRet = GetTapeStatus(device.hDevice);
		if (dwRet != NO_ERROR) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
#if 0
		TAPE_SET_MEDIA_PARAMETERS mediaParam = {};
		mediaParam.BlockSize = 512;
		dwRet = SetTapeParameters(device.hDevice, SET_TAPE_MEDIA_INFORMATION, &mediaParam);
		if (dwRet != NO_ERROR) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
#endif
		buff = (LPBYTE)calloc(tapedrive.DefaultBlockSize, sizeof(BYTE));
		if (!buff) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwReadSize = 0;
		dwRet = SetTapePosition(device.hDevice, TAPE_REWIND, 1, 0, 0, TRUE);
		if (dwRet != NO_ERROR) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		FILE* fpTape = CreateOrOpenFile(pszFullPath, NULL, NULL, NULL, NULL, _T(".bin"), _T("wb"), 0, 0);
		if (!fpTape) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		DWORD dwBinSize = 0;
//		LPVOID lpContext = NULL;
		for (int i = 0; ; i++) {
			if (!ReadFile(device.hDevice, buff, 1024, &dwReadSize, NULL)) {
//			if (!BackupRead(device.hDevice, buff, tapedrive.DefaultBlockSize, &dwReadSize, FALSE, TRUE, &lpContext)) {
				if (GetLastError() != ERROR_NO_DATA_DETECTED) {
					if (GetLastError() == ERROR_MORE_DATA) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						OutputString("dwReadSize: %lu\n", dwReadSize);
						fwrite(buff, sizeof(BYTE), dwReadSize, fpTape);
						continue;
					}
					else if (GetLastError() == ERROR_FILEMARK_DETECTED) {
						OutputString("dwReadSize: %lu\n", dwReadSize);
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						continue;
#if 0
						DWORD dwPartition = 0;
						DWORD dwOffsetLow = 0;
						DWORD dwOffsetHigh = 0;
						dwRet = GetTapePosition(device.hDevice, TAPE_LOGICAL_POSITION, &dwPartition, &dwOffsetLow, &dwOffsetHigh);
						if (dwRet != NO_ERROR) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							return FALSE;
						}
						OutputString(
							" dwPartition: %lu\n"
							" dwOffsetLow: %lu\n"
							"dwOffsetHigh: %lu\n"
							, dwPartition
							, dwOffsetLow
							, dwOffsetHigh
						);
						dwRet = SetTapePosition(device.hDevice, TAPE_LOGICAL_BLOCK, dwPartition, dwOffsetLow, dwOffsetHigh, TRUE);
						if (dwRet != NO_ERROR) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							throw FALSE;
						}
						continue;
#endif
					}
					else {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					}
				}
				else {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				}
				break;
			}
			fwrite(buff, sizeof(BYTE), dwReadSize, fpTape);
			dwBinSize += dwReadSize;
			OutputString("\rCreating bin %lu", dwBinSize);
		}
		OutputString("\n");
		FcloseAndNull(fpTape);

		dwRet = PrepareTape(device.hDevice, TAPE_UNLOCK, FALSE);
		if (dwRet != NO_ERROR) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(buff);
	return bRet;
#endif
}
