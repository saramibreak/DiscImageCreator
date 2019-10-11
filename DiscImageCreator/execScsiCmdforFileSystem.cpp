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
#include "check.h"
#include "convert.h"
#include "execScsiCmdforCD.h"
#include "execScsiCmdforCDCheck.h"
#include "execScsiCmdforFileSystem.h"
#include "execIoctl.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"
#include "outputScsiCmdLogforDVD.h"
#include "set.h"
#include "_external/abgx360.h"
#include "_external/mbedtls/aes.h"

BOOL ReadCDFor3DODirectory(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPCCH pPath,
	INT nLBA
) {
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		DISC_RAW_READ_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	try {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, nLBA, lpBuf,
			DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);

		UINT ofs = THREEDO_DIR_HEADER_SIZE;
		UINT dirSize =
			MAKEUINT(MAKEWORD(lpBuf[15], lpBuf[14]), MAKEWORD(lpBuf[13], lpBuf[12]));
		OutputFs3doDirectoryRecord(lpBuf, nLBA, pPath, dirSize);

		// next dir
		CHAR szNewPath[_MAX_PATH] = {};
		CHAR fname[32] = {};
		while (ofs < dirSize) {
			LPBYTE lpDirEnt = lpBuf + ofs;
			UINT lFlags = MAKEUINT(
				MAKEWORD(lpDirEnt[3], lpDirEnt[2]), MAKEWORD(lpDirEnt[1], lpDirEnt[0]));
			strncpy(fname, (LPCH)&lpDirEnt[32], sizeof(fname));
			UINT lastCopy = MAKEUINT(
				MAKEWORD(lpDirEnt[67], lpDirEnt[66]), MAKEWORD(lpDirEnt[65], lpDirEnt[64]));
			ofs += THREEDO_DIR_ENTRY_SIZE;

			if ((lFlags & 0xff) == 7) {
				sprintf(szNewPath, "%s%s/", pPath, fname);
				if (!ReadCDFor3DODirectory(pExtArg, pDevice, pDisc, pCdb, szNewPath,
					(INT)MAKEUINT(MAKEWORD(lpDirEnt[71], lpDirEnt[70]), MAKEWORD(lpDirEnt[69], lpDirEnt[68])))) {
					throw FALSE;
				}
			}
			for (UINT i = 0; i < lastCopy; i++) {
				ofs += 4;
			}
		}
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	return bRet;
}

VOID ManageEndOfDirectoryRecord(
	LPINT nSectorNum,
	BYTE byTransferLen,
	UINT uiZeroPaddingNum,
	LPBYTE lpDirRec,
	LPUINT nOfs
) {
	if (*nSectorNum < byTransferLen) {
		UINT j = 0;
		for (; j < uiZeroPaddingNum; j++) {
			if (lpDirRec[j] != 0) {
				break;
			}
		}
		if (j == uiZeroPaddingNum) {
			*nOfs += uiZeroPaddingNum;
			(*nSectorNum)++;
			return;
		}
	}
	else {
		return;
	}
}

BOOL ReadDirectoryRecordDetail(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	INT nLBA,
	LPBYTE lpBuf,
	LPBYTE bufDec,
	BYTE byTransferLen,
	INT nDirPosNum,
	UINT uiLogicalBlkCoef,
	INT nOffset,
	PDIRECTORY_RECORD pDirRec
) {
	if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc
		, pCdb, nLBA + nOffset, lpBuf, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE byRoop = byTransferLen;
	if (*pExecType == gd) {
		byRoop = (BYTE)(byRoop - 1);
	}
	for (BYTE i = 0; i < byRoop; i++) {
		OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, nLBA + i, DISC_RAW_READ_SIZE);
	}

	UINT uiOfs = 0;
	for (INT nSectorNum = 0; nSectorNum < byRoop;) {
		if (*(lpBuf + uiOfs) == 0) {
			break;
		}
		OutputVolDescLogA(
			OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(Directory Record), nLBA + nSectorNum, nLBA + nSectorNum);
		for (;;) {
			CHAR szCurDirName[MAX_FNAME_FOR_VOLUME] = {};
			LPBYTE lpDirRec = lpBuf + uiOfs;
			if (lpDirRec[0] >= MIN_LEN_DR) {
				if (lpDirRec[0] == MIN_LEN_DR && uiOfs > 0 && uiOfs % DISC_RAW_READ_SIZE == 0) {
					// SimCity 3000 (USA)
					OutputVolDescLogA(
						"Direcory record size of the %d sector maybe incorrect. Skip the reading of this sector\n", nLBA);
					nSectorNum++;
					break;
				}
				// a DVD "DTM Race Driver 3"
				// Path table is irregular. (L type and M type is perhaps the reverse.)
				// ========== LBA[000019, 0x00013]: Main Channel ==========
				//	:
				//	:
				//	                                2C 00 AB 3D 0C 00 00 0C   A1.HDR; 1, .. = ....
				//	0100 : 3D AB 00 60 8B 52 52 8B  60 00 6A 01 19 10 00 0A = ..`.RR.`.j.....
				//	0110 : 00 00 00 00 01 00 00 01  0B 44 41 54 41 32 2E 43   .........DATA2.C
				//	0120 : 41 42 3B
				UINT uiMaxByte = UINT(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE);
				if (pDisc->SCSI.nAllLength >= 0x200000) {
					uiMaxByte = 0xffffffff;
				}
				UINT uiExtentPos = GetSizeOrUintForVolDesc(lpDirRec + 2, uiMaxByte) / uiLogicalBlkCoef;
				UINT uiDataLen = GetSizeOrUintForVolDesc(lpDirRec + 10, uiMaxByte);
				if (uiDataLen >= uiMaxByte) {
					OutputVolDescLogA(
						"Data length is incorrect.\n");
					// Apple Mac DL DVD
					//										:
					//		                     Data Length: 4294967295
					//										:
					//		                 File Identifier: ARCHIVEPAX.GZ
					//										:

					// Pier Solar And The Great Architects
					// 0040 :             2A 00 1E FF  FF FF 00 FF FF FF FF FF   ....*...........
					// 0050 : FF FF FF FF 30 00 73 03  1F 16 23 29 08 00 00 00   ....0.s...#)....
					// 0060 : 01 00 00 01 08 31 38 2E  44 41 54 3B 31 00
#if 0
					if (*pExecType != dvd) {
						OutputVolDescLogA("Skip the reading of this sector\n");
						nSectorNum++;
						break;
					}
#endif
				}
				OutputFsDirectoryRecord(
					pExtArg, pDisc, lpDirRec, uiExtentPos, uiDataLen, szCurDirName);
				OutputVolDescLogA("\n");
				uiOfs += lpDirRec[0];

				if ((lpDirRec[25] & 0x02 || (pDisc->SCSI.byFormat == DISK_TYPE_CDI && lpDirRec[25] == 0))
					&& !(lpDirRec[32] == 1 && szCurDirName[0] == 0)
					&& !(lpDirRec[32] == 1 && szCurDirName[0] == 1)) {
					// not upper and current directory 
					for (INT i = 1; i < nDirPosNum; i++) {
						if (uiExtentPos == pDirRec[i].uiPosOfDir &&
							!_strnicmp(szCurDirName, pDirRec[i].szDirName, MAX_FNAME_FOR_VOLUME)) {
							pDirRec[i].uiDirSize = PadSizeForVolDesc(uiDataLen);
							break;
						}
					}
				}
				if (uiOfs == (UINT)(DISC_RAW_READ_SIZE * (nSectorNum + 1))) {
					nSectorNum++;
					break;
				}
			}
			else {
				UINT uiZeroPaddingNum = DISC_RAW_READ_SIZE * (nSectorNum + 1) - uiOfs;
				if (uiZeroPaddingNum > MIN_LEN_DR) {
					BYTE byNextLenDR = lpDirRec[MIN_LEN_DR];
					if (byNextLenDR >= MIN_LEN_DR) {
						// Amiga Tools 4 : The second of Direcory Record (0x22 - 0x43) is corrupt...
						// ========== LBA[040915, 0x09fd3]: Main Channel ==========
						//        +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F
						// 0000 : 22 00 D3 9F 00 00 00 00  9F D3 00 08 00 00 00 00   "...............
						// 0010 : 08 00 60 02 1D 17 18 2C  00 02 00 00 01 00 00 01   ..`....,........
						// 0020 : 01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
						// 0030 : 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
						// 0040 : 00 00 01 01 2E 00 09 A0  00 00 00 00 A0 09 D8 01   ................
						OutputMainErrorWithLBALogA(
							"Direcory Record is corrupt. Skip reading from %d to %d byte. See _mainInfo.txt\n"
							, nLBA, 0, uiOfs, uiOfs + MIN_LEN_DR - 1);
						uiOfs += MIN_LEN_DR;
						break;
					}
					else {
						ManageEndOfDirectoryRecord(&nSectorNum, byRoop, uiZeroPaddingNum, lpDirRec, &uiOfs);
						break;
					}
				}
				else {
					ManageEndOfDirectoryRecord(&nSectorNum, byRoop, uiZeroPaddingNum, lpDirRec, &uiOfs);
					break;
				}
			}
		}
	}
	return TRUE;
}

BOOL ReadDirectoryRecord(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	LPBYTE lpBuf,
	UINT uiLogicalBlkCoef,
	UINT uiRootDataLen,
	INT nSectorOfs,
	PDIRECTORY_RECORD pDirRec,
	INT nDirPosNum
) {
	LPBYTE bufDec = NULL;
	if (*pExecType == gd) {
		bufDec = (LPBYTE)calloc(pDevice->dwMaxTransferLength, sizeof(BYTE));
		if (!bufDec) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
	}
	BYTE byTransferLen = 1;
	BYTE byRoop = byTransferLen;
	// for CD-I
	if (uiRootDataLen == 0) {
		if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
			, (INT)pDirRec[0].uiPosOfDir + nSectorOfs, lpBuf, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		uiRootDataLen =
			PadSizeForVolDesc(GetSizeOrUintForVolDesc(lpBuf + 10, (UINT)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE)));
	}
	pDirRec[0].uiDirSize = uiRootDataLen;

	for (INT nDirRecIdx = 0; nDirRecIdx < nDirPosNum; nDirRecIdx++) {
		INT nLBA = (INT)pDirRec[nDirRecIdx].uiPosOfDir;
		if (pDirRec[nDirRecIdx].uiDirSize > pDevice->dwMaxTransferLength) {
			// [FMT] Psychic Detective Series Vol. 4 - Orgel (Japan) (v1.0)
			// [FMT] Psychic Detective Series Vol. 5 - Nightmare (Japan)
			// [IBM - PC compatible] Maria 2 - Jutai Kokuchi no Nazo (Japan) (Disc 1)
			// [IBM - PC compatible] PC Game Best Series Vol. 42 - J.B. Harold Series - Kiss of Murder - Satsui no Kuchizuke (Japan)
			// [SS] Madou Monogatari (Japan)
			// and more
			DWORD additionalTransferLen = pDirRec[nDirRecIdx].uiDirSize / pDevice->dwMaxTransferLength;
			SetCommandForTransferLength(pExecType, pDevice, pCdb, pDevice->dwMaxTransferLength, &byTransferLen, &byRoop);
			OutputMainInfoLogA("nLBA %d, uiDirSize: %lu*%lu, byTransferLen: %d*%lu [L:%d]\n"
				, nLBA, pDevice->dwMaxTransferLength, additionalTransferLen, byRoop, additionalTransferLen, (INT)__LINE__);

			for (DWORD n = 0; n < additionalTransferLen; n++) {
				if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
					, lpBuf, bufDec, byTransferLen, nDirPosNum, uiLogicalBlkCoef, nSectorOfs, pDirRec)) {
					continue;
				}
				nLBA += byRoop;
			}
			DWORD dwLastTblSize = pDirRec[nDirRecIdx].uiDirSize % pDevice->dwMaxTransferLength;
			if (dwLastTblSize != 0) {
				SetCommandForTransferLength(pExecType, pDevice, pCdb, dwLastTblSize, &byTransferLen, &byRoop);
				OutputMainInfoLogA("nLBA %d, uiDirSize: %lu, byTransferLen: %d [L:%d]\n"
					, nLBA, dwLastTblSize, byRoop, (INT)__LINE__);

				if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
					, lpBuf, bufDec, byTransferLen, nDirPosNum, uiLogicalBlkCoef, nSectorOfs, pDirRec)) {
					continue;
				}
			}
		}
		else {
			if (pDirRec[nDirRecIdx].uiDirSize == 0 || byTransferLen == 0) {
				OutputMainErrorLogA("Directory Record is invalid\n");
				return FALSE;
			}
			SetCommandForTransferLength(pExecType, pDevice, pCdb, pDirRec[nDirRecIdx].uiDirSize, &byTransferLen, &byRoop);
			OutputMainInfoLogA("nLBA %d, uiDirSize: %u, byTransferLen: %d [L:%d]\n"
				, nLBA, pDirRec[nDirRecIdx].uiDirSize, byRoop, (INT)__LINE__);

			if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
				, lpBuf, bufDec, byTransferLen, nDirPosNum, uiLogicalBlkCoef, nSectorOfs, pDirRec)) {
				continue;
			}
		}
		OutputString(_T("\rReading DirectoryRecord %4d/%4d"), nDirRecIdx + 1, nDirPosNum);
	}
	OutputString(_T("\n"));
	return TRUE;
}

BOOL ReadPathTableRecord(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE pCdb,
	UINT uiLogicalBlkCoef,
	UINT uiPathTblSize,
	UINT uiPathTblPos,
	BOOL bPathType,
	INT nSectorOfs,
	PDIRECTORY_RECORD pDirRec,
	LPINT nDirPosNum
) {
	BYTE byTransferLen = 1;
	BYTE byRoop = byTransferLen;
	DWORD dwBufSize = 0;
	if (*pExecType == gd) {
		byTransferLen = 2;
		dwBufSize = (CD_RAW_SECTOR_SIZE - (uiPathTblSize % CD_RAW_SECTOR_SIZE) + uiPathTblSize) * byTransferLen * 2;
	}
	else {
		dwBufSize = DISC_RAW_READ_SIZE - (uiPathTblSize % DISC_RAW_READ_SIZE) + uiPathTblSize;
	}
	SetCommandForTransferLength(pExecType, pDevice, pCdb, uiPathTblSize, &byTransferLen, &byRoop);
	
	LPBYTE lpBuf = (LPBYTE)calloc(dwBufSize, sizeof(BYTE));
	if (!lpBuf) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	BOOL bRet = TRUE;
	try {
		LPBYTE bufDec = NULL;
		if (*pExecType == gd) {
			if (NULL == (bufDec = (LPBYTE)calloc(size_t(CD_RAW_SECTOR_SIZE * byTransferLen), sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				return FALSE;
			}
		}
		if (uiPathTblSize > pDevice->dwMaxTransferLength) {
			DWORD uiAdditionalTransferLen = uiPathTblSize / pDevice->dwMaxTransferLength;
			SetCommandForTransferLength(pExecType, pDevice, pCdb, pDevice->dwMaxTransferLength, &byTransferLen, &byRoop);
			OutputMainInfoLogA("uiPathTblSize: %lu, byTransferLen: %d [L:%d]\n"
				, pDevice->dwMaxTransferLength, byRoop, (INT)__LINE__);

			for (DWORD n = 0; n < uiAdditionalTransferLen; n++) {
				if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
					, (INT)uiPathTblPos + nSectorOfs, lpBuf + pDevice->dwMaxTransferLength * n, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				for (BYTE i = 0; i < byRoop; i++) {
					OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, (INT)uiPathTblPos + i, DISC_RAW_READ_SIZE);
				}
				uiPathTblPos += byTransferLen;
			}
			DWORD dwLastPathTblSize = uiPathTblSize % pDevice->dwMaxTransferLength;
			SetCommandForTransferLength(pExecType, pDevice, pCdb, dwLastPathTblSize, &byTransferLen, &byRoop);
			DWORD dwBufOfs = pDevice->dwMaxTransferLength * uiAdditionalTransferLen;

			OutputMainInfoLogA(
				"uiPathTblSize: %lu, byTransferLen: %d [L:%d]\n", dwLastPathTblSize, byRoop, (INT)__LINE__);
			if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
				, (INT)uiPathTblPos + nSectorOfs, lpBuf + dwBufOfs, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			for (BYTE i = 0; i < byRoop; i++) {
				OutputCDMain(fileMainInfo, lpBuf + dwBufOfs + DISC_RAW_READ_SIZE * i, (INT)uiPathTblPos + i, DISC_RAW_READ_SIZE);
			}
			if (!OutputFsPathTableRecord(lpBuf, uiLogicalBlkCoef, uiPathTblPos, uiPathTblSize, bPathType, pDirRec, nDirPosNum)) {
				throw FALSE;
			}
		}
		else {
			OutputMainInfoLogA(
				"uiPathTblSize: %u, byTransferLen: %d [L:%d]\n", uiPathTblSize, byRoop, (INT)__LINE__);
			if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
				, (INT)uiPathTblPos + nSectorOfs, lpBuf, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			for (BYTE i = 0; i < byRoop; i++) {
				OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, (INT)uiPathTblPos + i, DISC_RAW_READ_SIZE);
			}
			if (!OutputFsPathTableRecord(lpBuf, uiLogicalBlkCoef, uiPathTblPos, uiPathTblSize, bPathType, pDirRec, nDirPosNum)) {
				throw FALSE;
			}
		}
		OutputVolDescLogA("Directory Num: %u\n", *nDirPosNum);
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(lpBuf);
	return bRet;
}

BOOL ReadVolumeDescriptor(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	BYTE byIdx,
	LPBYTE pCdb,
	LPBYTE lpBuf,
	INT nPVD,
	INT nSectorOfs,
	LPBOOL lpReadVD,
	PVOLUME_DESCRIPTOR pVolDesc,
	BYTE byTransferLen
) {
	if (pDisc->SCSI.lpFirstLBAListOnToc) {
		nPVD += pDisc->SCSI.lpFirstLBAListOnToc[byIdx];
	}
	INT nTmpLBA = nPVD;
	BYTE bufDec[CD_RAW_SECTOR_SIZE * 2] = {};
	for (;;) {
		if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc
			, pCdb, nTmpLBA + nSectorOfs, lpBuf, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
			break;
		}
		if (!strncmp((LPCH)&lpBuf[1], "CD001", 5) ||
			(pDisc->SCSI.byFormat == DISK_TYPE_CDI && !strncmp((LPCH)&lpBuf[1], "CD-I ", 5))) {
			if (nTmpLBA == nPVD) {
				WORD wLogicalBlkSize = GetSizeOrWordForVolDesc(lpBuf + 128);
				pVolDesc->ISO_9660.uiLogicalBlkCoef = (BYTE)(DISC_RAW_READ_SIZE / wLogicalBlkSize);
				pVolDesc->ISO_9660.uiPathTblSize =
					GetSizeOrUintForVolDesc(lpBuf + 132, (UINT)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
				pVolDesc->ISO_9660.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143])) / pVolDesc->ISO_9660.uiLogicalBlkCoef;
				pVolDesc->bPathType = lType;
				if (pVolDesc->ISO_9660.uiPathTblPos == 0) {
					pVolDesc->ISO_9660.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
					pVolDesc->bPathType = mType;
				}
				pVolDesc->ISO_9660.uiRootDataLen =
					GetSizeOrUintForVolDesc(lpBuf + 166, (UINT)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
				if (pVolDesc->ISO_9660.uiRootDataLen > 0) {
					pVolDesc->ISO_9660.uiRootDataLen = PadSizeForVolDesc(pVolDesc->ISO_9660.uiRootDataLen);
				}
				*lpReadVD = TRUE;
			}
			else if (lpBuf[0] == 2) {
				WORD wLogicalBlkSize = GetSizeOrWordForVolDesc(lpBuf + 128);
				pVolDesc->JOLIET.uiLogicalBlkCoef = (BYTE)(DISC_RAW_READ_SIZE / wLogicalBlkSize);
				pVolDesc->JOLIET.uiPathTblSize =
					GetSizeOrUintForVolDesc(lpBuf + 132, (UINT)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
				pVolDesc->JOLIET.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143])) / pVolDesc->JOLIET.uiLogicalBlkCoef;
				pVolDesc->bPathType = lType;
				if (pVolDesc->JOLIET.uiPathTblPos == 0) {
					pVolDesc->JOLIET.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
					pVolDesc->bPathType = mType;
				}
				pVolDesc->JOLIET.uiRootDataLen =
					GetSizeOrUintForVolDesc(lpBuf + 166, (UINT)(pDisc->SCSI.nAllLength * DISC_RAW_READ_SIZE));
				if (pVolDesc->JOLIET.uiRootDataLen > 0) {
					pVolDesc->JOLIET.uiRootDataLen = PadSizeForVolDesc(pVolDesc->JOLIET.uiRootDataLen);
				}
				*lpReadVD = TRUE;
			}
			OutputCDMain(fileMainInfo, lpBuf, nTmpLBA, DISC_RAW_READ_SIZE);
			OutputFsVolumeDescriptor(pExtArg, pDisc, lpBuf, nTmpLBA++);
		}
		else {
			break;
		}
	}
	return TRUE;
}

BOOL ReadCDForFileSystem(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	BOOL bRet = TRUE;
	for (BYTE i = 0; i < pDisc->SCSI.toc.LastTrack; i++) {
		if ((pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
			// for Label Gate CD, XCP
			if (i > 1 && pDisc->SCSI.lpLastLBAListOnToc[i] - pDisc->SCSI.lpFirstLBAListOnToc[i] + 1 <= 750) {
				return TRUE;
			}
			// for Anno 1602 - Im Namen des Konigs
			else if (i == 1 && pDisc->SCSI.lpLastLBAListOnToc[i] - pDisc->SCSI.lpFirstLBAListOnToc[i] + 1 <= 200) {
				return TRUE;
			}
			LPBYTE pBuf = NULL;
			LPBYTE lpBuf = NULL;
			if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
				(UINT)pDevice->dwMaxTransferLength, &lpBuf, _T(__FUNCTION__), __LINE__)) {
				return FALSE;
			}
			CDB::_READ12 cdb = {};
			cdb.OperationCode = SCSIOP_READ12;
			cdb.TransferLength[3] = 1;
			BOOL bVD = FALSE;
			PDIRECTORY_RECORD pDirRec = NULL;
			try {
				// general data track disc
				VOLUME_DESCRIPTOR volDesc = {};
				if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice
					, pDisc, i, (LPBYTE)&cdb, lpBuf, 16, 0, &bVD, &volDesc, 1)) {
					throw FALSE;
				}
				if (bVD) {
					pDirRec = (PDIRECTORY_RECORD)calloc(DIRECTORY_RECORD_SIZE, sizeof(DIRECTORY_RECORD));
					if (!pDirRec) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					INT nDirPosNum = 0;
					if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
						, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiPathTblSize
						, volDesc.ISO_9660.uiPathTblPos, volDesc.bPathType, 0, pDirRec, &nDirPosNum)) {
						throw FALSE;
					}
					if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
						, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiRootDataLen, 0, pDirRec, nDirPosNum)) {
						OutputVolDescLogA("Failed to read ISO9660\n");
						nDirPosNum = 0;
						if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
							, volDesc.JOLIET.uiLogicalBlkCoef, volDesc.JOLIET.uiPathTblSize
							, volDesc.JOLIET.uiPathTblPos, volDesc.bPathType, 0, pDirRec, &nDirPosNum)) {
							throw FALSE;
						}
						if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
							, volDesc.JOLIET.uiLogicalBlkCoef, volDesc.JOLIET.uiRootDataLen, 0, pDirRec, nDirPosNum)) {
							throw FALSE;
						}
					}
					if (pExtArg->byScanProtectViaFile || pExtArg->byIntentionalSub) {
						if (!ReadCDForCheckingExe(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf)) {
							throw FALSE;
						}
					}
					if (pDisc->PROTECT.byExist) {
						OutputLogA(standardOut | fileDisc, "Detected a protected file [%s]. LBA %d to %d"
							, pDisc->PROTECT.name, pDisc->PROTECT.ERROR_SECTOR.nExtentPos
							, pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize);
						if (pDisc->PROTECT.byExist == microids || pDisc->PROTECT.byExist == datelAlt) {
							OutputLogA(standardOut | fileDisc, ",  [%s]. LBA %d to %d\n"
								, pDisc->PROTECT.name2, pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd
								, pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd + pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd);
						}
						else {
							OutputLogA(standardOut | fileDisc, "\n");
						}
					}
				}
				else {
					BOOL bOtherHeader = FALSE;
					// for pce, pc-fx
					INT nLBA = pDisc->SCSI.nFirstLBAofDataTrack;
					if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
						DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					if (IsValidPceSector(lpBuf)) {
						OutputFsPceStuff(lpBuf, nLBA);
						nLBA = pDisc->SCSI.nFirstLBAofDataTrack + 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						OutputFsPceBootSector(lpBuf, nLBA);
						bOtherHeader = TRUE;
					}
					else if (IsValidPcfxSector(lpBuf)) {
						OutputFsPcfxHeader(lpBuf, nLBA);
						nLBA = pDisc->SCSI.nFirstLBAofDataTrack + 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						OutputFsPcfxSector(lpBuf, nLBA);
						bOtherHeader = TRUE;
					}

					if (!bOtherHeader) {
						// for 3DO
						nLBA = 0;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValid3doDataHeader(lpBuf)) {
							OutputFs3doHeader(lpBuf, nLBA);
							if (!ReadCDFor3DODirectory(pExtArg, pDevice, pDisc, &cdb, (LPCCH)"/",
								(INT)MAKEUINT(MAKEWORD(lpBuf[103], lpBuf[102]), MAKEWORD(lpBuf[101], lpBuf[100])))) {
								throw FALSE;
							}
							bOtherHeader = TRUE;
						}
					}
					if (!bOtherHeader) {
						// for MAC pattern 1
						nLBA = 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValidMacDataHeader(lpBuf + 1024)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 1024, nLBA);
							bOtherHeader = TRUE;
						}
						else if (IsValidMacDataHeader(lpBuf + 512)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 512, nLBA);
							bOtherHeader = TRUE;
						}
						// for MAC pattern 2
						nLBA = 16;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_RAW_READ_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValidMacDataHeader(lpBuf + 1024)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 1024, nLBA);
							bOtherHeader = TRUE;
						}
					}
					if (bOtherHeader) {
						FreeAndNull(pBuf);
						break;
					}
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			FreeAndNull(pDirRec);
			FreeAndNull(pBuf);
		}
	}
	return bRet;
}

BOOL ReadGDForFileSystem(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		(UINT)pDevice->dwMaxTransferLength, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BOOL bRet = TRUE;
	PDIRECTORY_RECORD pDirRec = NULL;
	CDB::_READ_CD cdb = {};
	SetReadCDCommand(pDevice, &cdb,
		CDFLAG::_READ_CD::CDDA, 2, CDFLAG::_READ_CD::NoC2, CDFLAG::_READ_CD::NoSub);
	try {
		INT nSectorOfs = pDisc->MAIN.nAdjustSectorNum - 1;
		if (pDisc->MAIN.nCombinedOffset < 0) {
			nSectorOfs = pDisc->MAIN.nAdjustSectorNum;
		}
		BOOL bVD = FALSE;
		VOLUME_DESCRIPTOR volDesc;
		if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice, pDisc, 0
			, (LPBYTE)&cdb, lpBuf, FIRST_LBA_FOR_GD + 16, nSectorOfs, &bVD, &volDesc, 2)) {
			throw FALSE;
		}
		if (bVD) {
			pDirRec = (PDIRECTORY_RECORD)calloc(DIRECTORY_RECORD_SIZE, sizeof(DIRECTORY_RECORD));
			if (!pDirRec) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			INT nDirPosNum = 0;
			if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
				, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiPathTblSize
				, volDesc.ISO_9660.uiPathTblPos, volDesc.bPathType, nSectorOfs, pDirRec, &nDirPosNum)) {
				throw FALSE;
			}
			if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
				, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiRootDataLen, nSectorOfs, pDirRec, nDirPosNum)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pDirRec);
	FreeAndNull(pBuf);
	return bRet;
}

BOOL ReadDVDForFileSystem(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* cdb,
	LPBYTE lpBuf
) {
	BOOL bPVD = FALSE;
	VOLUME_DESCRIPTOR volDesc;
	DWORD dwTransferLen = 1;
	REVERSE_BYTES(&cdb->TransferLength, &dwTransferLen);

	if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice
		, pDisc, 0, (LPBYTE)cdb, lpBuf, 16, 0, &bPVD, &volDesc, (BYTE)dwTransferLen)) {
		return FALSE;
	}
	if (bPVD) {
		PDIRECTORY_RECORD pDirRec = (PDIRECTORY_RECORD)calloc(DIRECTORY_RECORD_SIZE, sizeof(DIRECTORY_RECORD));
		if (!pDirRec) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		INT nDirPosNum = 0;
		if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)cdb
			, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiPathTblSize
			, volDesc.ISO_9660.uiPathTblPos, volDesc.bPathType, 0, pDirRec, &nDirPosNum)) {
			FreeAndNull(pDirRec);
			return FALSE;
		}
		if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)cdb, lpBuf
			, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiRootDataLen, 0, pDirRec, nDirPosNum)) {
			FreeAndNull(pDirRec);
			return FALSE;
		}
		FreeAndNull(pDirRec);
		if (pDisc->PROTECT.byExist && !pExtArg->byNoSkipSS) {
			OutputLogA(standardOut | fileDisc, "Detected protection [%s]. LBA %d to %d\n"
				, pDisc->PROTECT.name, pDisc->PROTECT.ERROR_SECTOR.nExtentPos
				, pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize);
		}
	}

	INT nLBA = 18;
	dwTransferLen = 14;
	REVERSE_BYTES(&cdb->TransferLength, &dwTransferLen);
	cdb->LogicalBlock[0] = 0;
	cdb->LogicalBlock[1] = 0;
	cdb->LogicalBlock[2] = 0;
	cdb->LogicalBlock[3] = (UCHAR)nLBA;

#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_RAW_READ_SIZE * dwTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	for (INT i = 0; i < DISC_RAW_READ_SIZE * 14; i += DISC_RAW_READ_SIZE, nLBA++) {
		OutputFsVolumeRecognitionSequence(lpBuf + i, nLBA);
	}

	nLBA = 32;
	dwTransferLen = pDevice->dwMaxTransferLength / DISC_RAW_READ_SIZE;
	REVERSE_BYTES(&cdb->TransferLength, &dwTransferLen);
	cdb->LogicalBlock[0] = 0;
	cdb->LogicalBlock[1] = 0;
	cdb->LogicalBlock[2] = 0;
	cdb->LogicalBlock[3] = (UCHAR)nLBA;

	if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_RAW_READ_SIZE * dwTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	if (lpBuf[20] == 0 && lpBuf[21] == 0 && lpBuf[22] == 0 && lpBuf[23] == 0) {
		for (INT i = 0; i < DISC_RAW_READ_SIZE * 32; i += DISC_RAW_READ_SIZE, nLBA++) {
			OutputFsVolumeDescriptorSequence(lpBuf + i, nLBA);
		}
	}

	cdb->LogicalBlock[2] = 1;
	cdb->LogicalBlock[3] = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_RAW_READ_SIZE * dwTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	nLBA = 256;
	OutputFsVolumeDescriptorSequence(lpBuf, nLBA);
	return TRUE;
}

// http://web.archive.org/web/20151026074806/http://home.comcast.net/~admiral_powerslave/dvddrives.html
BOOL OutputXDVDFsDirectoryRecord(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	LPUINT lpOfs,
	UINT uiStartLBA,
	LPBYTE pTab,
	LPBOOL pEnd
) {
	WORD ofsLeft = MAKEWORD(lpBuf[0], lpBuf[1]);
	if (ofsLeft == 0xffff) {
		*pEnd = TRUE;
		return TRUE;
	}
	WORD ofsRight = MAKEWORD(lpBuf[2], lpBuf[3]);
	UINT startSector = MAKEUINT(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7]));
	UINT fileSize = MAKEUINT(MAKEWORD(lpBuf[8], lpBuf[9]), MAKEWORD(lpBuf[10], lpBuf[11]));
	OutputVolDescLogA(
		"%s Offset to left sub-tree entry: %d(0x%x)\n"
		"%sOffset to right sub-tree entry: %d(0x%x)\n"
		"%s       Starting sector of file: %d(0x%x)\n"
		"%s               Total file size: %d(0x%x)\n"
		"%s               File attributes: "
		, (LPCH)&pTab[0], ofsLeft, ofsLeft
		, (LPCH)&pTab[0], ofsRight, ofsRight
		, (LPCH)&pTab[0], startSector, startSector
		, (LPCH)&pTab[0], fileSize, fileSize
		, (LPCH)&pTab[0]
	);

	BYTE attribute = lpBuf[12];
	if (attribute == 0x01) {
		OutputVolDescLogA("read only\n");
	}
	else if (attribute == 0x02) {
		OutputVolDescLogA("hidden\n");
	}
	else if (attribute == 0x04) {
		OutputVolDescLogA("system file\n");
	}
	else if (attribute == 0x10) {
		OutputVolDescLogA("directory\n");
	}
	else if (attribute == 0x20) {
		OutputVolDescLogA("archive\n");
	}
	else if (attribute == 0x80) {
		OutputVolDescLogA("normal\n");
	}
	else {
		OutputVolDescLogA("other\n");
	}
	BYTE lenOfFile = lpBuf[13];
	OutputVolDescLogA(
		"%s            Length of filename: %d\n"
		"%s                      Filename: "
		, (LPCH)&pTab[0], lenOfFile
		, (LPCH)&pTab[0]
	);
	for (BYTE i = 0; i < lenOfFile; i++) {
		OutputVolDescLogA("%c", lpBuf[14 + i]);
	}
	OutputVolDescLogA("\n\n");
	UINT mod = (14 + lenOfFile) % sizeof(UINT);
	if (mod != 0) {
		*lpOfs = (UINT)(14 + lenOfFile + sizeof(UINT) - mod);
	}
	else {
		*lpOfs = (UINT)(14 + lenOfFile);
	}

	if (attribute == 0x10) {
		size_t idx = strlen((LPCH)&pTab[0]);
		pTab[idx] = '\t';
		if (!ReadXBOXDirectoryRecord(pExtArg, pDevice, pCdb
			, startSector + uiStartLBA, fileSize, uiStartLBA, pTab)) {
			return FALSE;
		}
		pTab[idx] = 0;
	}
	return TRUE;
}

BOOL ReadXBOXDirectoryRecord(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_READ12* pCdb,
	UINT uiDirPos,
	UINT uiDirTblSize,
	UINT uiStartLBA,
	LPBYTE pTab
) {
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		uiDirTblSize, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	BYTE byScsiStatus = 0;
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	pCdb->TransferLength[3] = (UCHAR)(uiDirTblSize / DISC_RAW_READ_SIZE);
	REVERSE_BYTES(pCdb->LogicalBlock, &uiDirPos);

	if (!ScsiPassThroughDirect(pExtArg, pDevice, pCdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, uiDirTblSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		FreeAndNull(pBuf);
		return FALSE;
	}
	for (UCHAR c = 0; c < pCdb->TransferLength[3]; c++) {
		OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * c, (INT)uiDirPos + c, DISC_RAW_READ_SIZE);
	}
	OutputVolDescLogA("%s"
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(DIRECTORY ENTRY)
		, (LPCH)&pTab[0], (INT)uiDirPos, (INT)uiDirPos
	);
	BOOL bEnd = FALSE;
	UINT uiSize = 0;
	UINT uiCoeff = 1;
	for (UINT uiOfs = 0; uiSize < uiDirTblSize;) {
		if (!OutputXDVDFsDirectoryRecord(pExtArg, pDevice, pCdb
			, lpBuf + uiSize, &uiOfs, uiStartLBA, pTab, &bEnd)) {
			return FALSE;
		}
		if (bEnd) {
			break;
		}
		else {
			uiSize += uiOfs;
			if (uiSize > DISC_RAW_READ_SIZE * uiCoeff - 15) {
				uiSize += DISC_RAW_READ_SIZE * uiCoeff - uiSize;
				uiCoeff++;
			}
		}
	}
	FreeAndNull(pBuf);
	return TRUE;
}

BOOL ReadXBOXFileSystem(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	DWORD dwStartLBA
) {
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		(UINT)pDevice->dwMaxTransferLength, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	CDB::_READ12 cdb = {};
	cdb.OperationCode = SCSIOP_READ12;
	BYTE byScsiStatus = 0;
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	cdb.TransferLength[3] = (UCHAR)(pDevice->dwMaxTransferLength / DISC_RAW_READ_SIZE);
	INT nLBA = (INT)dwStartLBA + 32;
	REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);

	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, pDevice->dwMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		FreeAndNull(pBuf);
		return FALSE;
	}
	OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);

	UINT uiDirPos = MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]));
	UINT uiDirTblSize = MAKEUINT(MAKEWORD(lpBuf[24], lpBuf[25]), MAKEWORD(lpBuf[26], lpBuf[27]));
	CHAR date[20] = {};
	printwin32filetime(MAKEUINT64(MAKELONG(MAKEWORD(lpBuf[28], lpBuf[29]), MAKEWORD(lpBuf[30], lpBuf[31]))
		, MAKELONG(MAKEWORD(lpBuf[32], lpBuf[33]), MAKEWORD(lpBuf[34], lpBuf[35]))), date);
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR(XDVDFS)
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(VOLUME DESCRIPTOR)
		"\t                        Header: %.20s\n"
		"\tSector of root directory table: %d(%#x)\n"
		"\t  Size of root directory table: %d(%#x)\n"
		"\t           Image creation time: %.20s\n"
		"\t                        Footer: %.20s\n"
		, nLBA, nLBA
		, (LPCH)&lpBuf[0]
		, uiDirPos, uiDirPos
		, uiDirTblSize, uiDirTblSize
		, date, (LPCH)&lpBuf[2028]
	);
	if (uiDirTblSize % DISC_RAW_READ_SIZE != 0) {
		uiDirTblSize += DISC_RAW_READ_SIZE * (uiDirTblSize / DISC_RAW_READ_SIZE + 1) - uiDirTblSize;
	}
	BYTE szTab[256] = {};
	szTab[0] = '\t';
	if (!ReadXBOXDirectoryRecord(
		pExtArg, pDevice, &cdb, uiDirPos + (UINT)dwStartLBA, uiDirTblSize, (UINT)dwStartLBA, szTab)) {
		FreeAndNull(pBuf);
		return FALSE;
	};
	FreeAndNull(pBuf);
	return TRUE;
}

// http://hitmen.c02.at/files/yagcd/yagcd/chap13.html#sec13
// https://wiibrew.org/wiki/Wii_Disc
BOOL ReadNintendoSystemHeader(
	LPCSTR pszFullPath,
	FILE** fp,
	LPBYTE buf
) {
	*fp = CreateOrOpenFileA(pszFullPath, NULL, NULL, NULL, NULL, ".iso", "rb", 0, 0);
	if (!*fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (fread(buf, sizeof(BYTE), 1024, *fp) != 1024) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(*fp);
		return FALSE;
	};
	OutputCDMain(fileMainInfo, buf, 0, 1024);
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR(Disc Header)
		"\t                                 Disc ID: %.1s\n"
		"\t                               Game Code: %.2s\n"
		"\t                             Region Code: %.1s\n"
		"\t                              Maker Code: %.2s\n"
		"\t                             Disc Number: %d\n"
		"\t                                 Version: %d\n"
		"\t                         Audio Streaming: %d\n"
		"\t                      Stream Buffer Size: %d\n"
		"\t                               Game Name: %s\n"
		, (LPCH)&buf[0], (LPCH)&buf[1], (LPCH)&buf[3], (LPCH)&buf[4]
		, buf[6], buf[7], buf[8], buf[9], (LPCH)&buf[32]
	);
	return TRUE;
}

BOOL ReadNintendoFileSystem(
	PDEVICE pDevice,
	LPCTSTR pszFullPath,
	DISC_TYPE type
) {
	FILE* fp = NULL;
	BYTE buf[1024] = {};
	OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(NintendoOpticalDiscFS));
	if (!ReadNintendoSystemHeader(pszFullPath, &fp, buf)) {
		return FALSE;
	};
	if (fread(buf, sizeof(BYTE), 64, fp) != 64) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fp);
		return FALSE;
	};
	OutputCDMain(fileMainInfo, buf, 0, 64);
	UINT ofsOfFst = MAKEUINT(MAKEWORD(buf[39], buf[38]), MAKEWORD(buf[37], buf[36]));
	UINT sizeOfFst = MAKEUINT(MAKEWORD(buf[43], buf[42]), MAKEWORD(buf[41], buf[40]));
	OutputVolDescLogA(
		"\t      offset of debug monitor (dh.bin) ?: %d(%#x)\n"
		"\t        addr (?) to load debug monitor ?: %#x\n"
		"\toffset of main executable DOL (bootfile): %d(%#x)\n"
		"\t             offset of the FST (fst.bin): %d(%#x)\n"
		"\t                             size of FST: %d(%#x)\n"
		"\t                     maximum size of FST: %d(%#x)\n"
		"\t                       user position (?): %#x\n"
		"\t                         user length (?): %d(%#x)\n"
		"\t                                 unknown: %d(%#x)\n"
		, MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]))
		, MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]))
		, MAKEUINT(MAKEWORD(buf[7], buf[6]), MAKEWORD(buf[5], buf[4]))
		, MAKEUINT(MAKEWORD(buf[35], buf[34]), MAKEWORD(buf[33], buf[32]))
		, MAKEUINT(MAKEWORD(buf[35], buf[34]), MAKEWORD(buf[33], buf[32]))
		, ofsOfFst, ofsOfFst, sizeOfFst, sizeOfFst
		, MAKEUINT(MAKEWORD(buf[47], buf[46]), MAKEWORD(buf[45], buf[44]))
		, MAKEUINT(MAKEWORD(buf[47], buf[46]), MAKEWORD(buf[45], buf[44]))
		, MAKEUINT(MAKEWORD(buf[51], buf[50]), MAKEWORD(buf[49], buf[48]))
		, MAKEUINT(MAKEWORD(buf[55], buf[54]), MAKEWORD(buf[53], buf[52]))
		, MAKEUINT(MAKEWORD(buf[55], buf[54]), MAKEWORD(buf[53], buf[52]))
		, MAKEUINT(MAKEWORD(buf[59], buf[58]), MAKEWORD(buf[57], buf[56]))
		, MAKEUINT(MAKEWORD(buf[59], buf[58]), MAKEWORD(buf[57], buf[56]))
	);
	if (type == wii) {
		ofsOfFst <<= 2;
		sizeOfFst <<= 2;
	}
	fseek(fp, 0x2440, SEEK_SET);
	if (fread(buf, sizeof(BYTE), 0x20, fp) != 0x20) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fp);
		return FALSE;
	};
	OutputCDMain(fileMainInfo, buf, 0x2440 / 0x800, 0x20);
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR(Apploader)
		"\tdate (version) of the apploader: %s\n"
		"\t           apploader entrypoint: %#x\n"
		"\t          size of the apploader: %d(%#x)\n"
		"\t                   trailer size: %d(%#x)\n"
		, (LPCH)&buf[0]
		, MAKEUINT(MAKEWORD(buf[19], buf[18]), MAKEWORD(buf[17], buf[16]))
		, MAKEUINT(MAKEWORD(buf[23], buf[22]), MAKEWORD(buf[21], buf[20]))
		, MAKEUINT(MAKEWORD(buf[23], buf[22]), MAKEWORD(buf[21], buf[20]))
		, MAKEUINT(MAKEWORD(buf[27], buf[26]), MAKEWORD(buf[25], buf[24]))
		, MAKEUINT(MAKEWORD(buf[27], buf[26]), MAKEWORD(buf[25], buf[24]))
	);
	fseek(fp, (LONG)ofsOfFst, SEEK_SET);
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf, sizeOfFst, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		FcloseAndNull(fp);
		return FALSE;
	}
	if (fread(lpBuf, sizeof(BYTE), sizeOfFst, fp) != sizeOfFst) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FreeAndNull(pBuf);
		FcloseAndNull(fp);
		return FALSE;
	};
	OutputCDMain(fileMainInfo, lpBuf, (INT)ofsOfFst / 0x800, (INT)sizeOfFst);
	UINT numOfEntries = MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], lpBuf[8]));
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR(Root Directory Entry)
		"\t                   flags: %d\n"
		"\toffset into string table: %d(%#x)\n"
		"\t           parent_offset: %d(%#x)\n"
		"\t             num_entries: %d\n\n"
		, lpBuf[0]
		, MAKEUINT(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0))
		, MAKEUINT(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0))
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, numOfEntries
	);
	
	UINT posOfString = numOfEntries * 12;
	OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(Directory Entry));

	for (UINT i = 12; i < posOfString; i += 12) {
		if (lpBuf[0 + i] == 0 || lpBuf[0 + i] == 1) {
			UINT ofsString = MAKEUINT(MAKEWORD(lpBuf[3 + i], lpBuf[2 + i]), MAKEWORD(lpBuf[1 + i], 0));
			OutputVolDescLogA(
				"\t                   flags: %d\n"
				"\toffset into string table: %d(%#x)\n"
				, lpBuf[0 + i], ofsString, ofsString
			);
			if (lpBuf[0 + i] == 0) {
				OutputVolDescLogA(
					"\t               file_name: %s\n"
					"\t             file_offset: %d(%#x)\n"
					"\t             file_length: %d(%#x)\n\n"
					, (LPCH)&lpBuf[posOfString + ofsString]
					, MAKEUINT(MAKEWORD(lpBuf[7 + i], lpBuf[6 + i]), MAKEWORD(lpBuf[5 + i], lpBuf[4 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[7 + i], lpBuf[6 + i]), MAKEWORD(lpBuf[5 + i], lpBuf[4 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[11 + i], lpBuf[10 + i]), MAKEWORD(lpBuf[9 + i], lpBuf[8 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[11 + i], lpBuf[10 + i]), MAKEWORD(lpBuf[9 + i], lpBuf[8 + i]))
				);
			}
			else if (lpBuf[0 + i] == 1) {
				OutputVolDescLogA(
					"\t                dir_name: %s\n"
					"\t           parent_offset: %d(%#x)\n"
					"\t             next_offset: %d(%#x)\n\n"
					, (LPCH)&lpBuf[posOfString + ofsString]
					, MAKEUINT(MAKEWORD(lpBuf[7 + i], lpBuf[6 + i]), MAKEWORD(lpBuf[5 + i], lpBuf[4 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[7 + i], lpBuf[6 + i]), MAKEWORD(lpBuf[5 + i], lpBuf[4 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[11 + i], lpBuf[10 + i]), MAKEWORD(lpBuf[9 + i], lpBuf[8 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[11 + i], lpBuf[10 + i]), MAKEWORD(lpBuf[9 + i], lpBuf[8 + i]))
				);
			}
		}
		else {
			break;
		}
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);
	return TRUE;
}

BOOL ReadPartitionTblEntry(
	FILE* fp,
	LPBYTE buf,
	INT idx,
	UINT numOfPartion,
	UINT ofsOfPartionInfoTbl,
	UINT ofsOfPartion[][4]
) {
	fseek(fp, (LONG)ofsOfPartionInfoTbl << 2, SEEK_SET);
	for (UINT i = 0; i < numOfPartion; i++) {
		if (fread(buf, sizeof(BYTE), 8, fp) != 8) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			FcloseAndNull(fp);
			return FALSE;
		};
		ofsOfPartion[idx][i] = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
		OutputVolDescLogA(
			OUTPUT_DHYPHEN_PLUS_STR(Partition table entry)
			"\t           Partition offset: %#x\n"
			"\t                       Type: %d\n"
			, ofsOfPartion[idx][i], MAKEUINT(MAKEWORD(buf[7], buf[6]), MAKEWORD(buf[5], buf[4]))
		);
	}
	return TRUE;
}

BOOL ReadWiiPartition(
	PDEVICE pDevice,
	LPCTSTR pszFullPath
) {
	FILE* fp = NULL;
	BYTE buf[0x8000] = {};
	OutputVolDescLogA(OUTPUT_DHYPHEN_PLUS_STR(WiiFS));
	if (!ReadNintendoSystemHeader(pszFullPath, &fp, buf)) {
		return FALSE;
	};

	fseek(fp, 0x40000, SEEK_SET);
	if (fread(buf, sizeof(BYTE), 0x20, fp) != 0x20) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fp);
		return FALSE;
	};
	OutputCDMain(fileMainInfo, buf, 0x40000 / 0x800, 0x20);
	UINT numOfPartition[4] = {};
	numOfPartition[0] = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
	UINT ofsOfPart1 = MAKEUINT(MAKEWORD(buf[7], buf[6]), MAKEWORD(buf[5], buf[4]));
	numOfPartition[1] = MAKEUINT(MAKEWORD(buf[11], buf[10]), MAKEWORD(buf[9], buf[8]));
	UINT ofsOfPart2 = MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], buf[12]));
	numOfPartition[2] = MAKEUINT(MAKEWORD(buf[19], buf[18]), MAKEWORD(buf[17], buf[16]));
	UINT ofsOfPart3 = MAKEUINT(MAKEWORD(buf[23], buf[22]), MAKEWORD(buf[21], buf[20]));
	numOfPartition[3] = MAKEUINT(MAKEWORD(buf[27], buf[26]), MAKEWORD(buf[25], buf[24]));
	UINT ofsOfPart4 = MAKEUINT(MAKEWORD(buf[31], buf[30]), MAKEWORD(buf[29], buf[28]));
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR(Partitions information)
		"\t       Total 1st partitions: %d\n"
		"\tPartition info table offset: %#x\n"
		"\t       Total 2nd partitions: %d\n"
		"\tPartition info table offset: %#x\n"
		"\t       Total 3rd partitions: %d\n"
		"\tPartition info table offset: %#x\n"
		"\t       Total 4th partitions: %d\n"
		"\tPartition info table offset: %#x\n"
		, numOfPartition[0], ofsOfPart1, numOfPartition[1], ofsOfPart2
		, numOfPartition[2], ofsOfPart3, numOfPartition[3], ofsOfPart4
	);
	UINT ofsOfPartion[4][4] = {};
	ReadPartitionTblEntry(fp, buf, 0, numOfPartition[0], ofsOfPart1, &ofsOfPartion[0]);
	if (numOfPartition[1] != 0) {
		ReadPartitionTblEntry(fp, buf, 1, numOfPartition[1], ofsOfPart2, &ofsOfPartion[1]);
		if (numOfPartition[2] != 0) {
			ReadPartitionTblEntry(fp, buf, 2, numOfPartition[2], ofsOfPart3, &ofsOfPartion[2]);
			if (numOfPartition[3] != 0) {
				ReadPartitionTblEntry(fp, buf, 3, numOfPartition[3], ofsOfPart4, &ofsOfPartion[3]);
			}
		}
	}

	fseek(fp, 0x4e000, SEEK_SET);
	if (fread(buf, sizeof(BYTE), 0x20, fp) != 0x20) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fp);
		return FALSE;
	};
	OutputCDMain(fileMainInfo, buf, 0x4e000 / 0x800, 0x20);
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR(Region setting)
		"\t                     Region byte: %d\n"
		"\tAge Rating byte for Japan/Taiwan: %d(%#x)\n"
		"\tAge Rating byte for USA         : %d(%#x)\n"
		"\tAge Rating byte for ------------: %d(%#x)\n"
		"\tAge Rating byte for Germany     : %d(%#x)\n"
		"\tAge Rating byte for PEGI        : %d(%#x)\n"
		"\tAge Rating byte for Finland     : %d(%#x)\n"
		"\tAge Rating byte for Portugal    : %d(%#x)\n"
		"\tAge Rating byte for Britain     : %d(%#x)\n"
		"\tAge Rating byte for Australia   : %d(%#x)\n"
		"\tAge Rating byte for Korea       : %d(%#x)\n"
		"\tAge Rating byte for ------------: %d(%#x)\n"
		"\tAge Rating byte for ------------: %d(%#x)\n"
		"\tAge Rating byte for ------------: %d(%#x)\n"
		"\tAge Rating byte for ------------: %d(%#x)\n"
		"\tAge Rating byte for ------------: %d(%#x)\n"
		"\tAge Rating byte for ------------: %d(%#x)\n"
		, MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]))
		, buf[16], buf[16], buf[17], buf[17], buf[18], buf[18], buf[19], buf[19]
		, buf[20], buf[20], buf[21], buf[21], buf[22], buf[22], buf[23], buf[23]
		, buf[24], buf[24], buf[25], buf[25], buf[26], buf[26], buf[27], buf[27]
		, buf[28], buf[28], buf[29], buf[29], buf[30], buf[30], buf[31], buf[31]
	);

	BYTE commonKey[16] = {};
	BOOL bDecOK = FALSE;
	FILE* fpKey = OpenProgrammabledFile(_T("key.bin"), _T("rb"));
	if (!fpKey) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		OutputErrorString(_T("You can't decrypt iso because there isn't a key.bin\n"));
	}
	else {
		if (fread(commonKey, sizeof(BYTE), sizeof(commonKey), fpKey) != sizeof(commonKey)) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		}
		FcloseAndNull(fpKey);
		bDecOK = TRUE;
	}
	for (UINT idx = 0; ofsOfPartion[idx][0] != 0; idx++) {
		for (UINT idx2 = 0; idx2 < numOfPartition[idx]; idx2++) {
			UINT realOfsOfPartion = ofsOfPartion[idx][idx2] << 2;
			fseek(fp, (LONG)realOfsOfPartion, SEEK_SET);
			if (fread(buf, sizeof(BYTE), 0x2C0, fp) != 0x2C0) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				FcloseAndNull(fp);
				return FALSE;
			};
			OutputCDMain(fileMainInfo, buf, (INT)realOfsOfPartion / 0x800, 0x2C0);
			UINT sigType = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
			OutputVolDescLogA(
				OUTPUT_DHYPHEN_PLUS_STR(Partition)
				"\t                  Signature type: %#x\n"
				"\tSignature by a certificate's key: "
				, sigType
			);
			INT size = 256;
			if (sigType == 0x10000) {
				size = 512;
			}
			for (INT i = 0; i < size; i++) {
				OutputVolDescLogA("%02x", buf[4 + i]);
			}
			OutputVolDescLogA(
				"\n"
				"\t                Signature issuer: %s\n"
				"\t                       ECDH data: "
				, (LPCH)&buf[0x140]
			);
			for (INT i = 0; i < 0x3c; i++) {
				OutputVolDescLogA("%02x", buf[0x0180 + i]);
			}
			OutputVolDescLogA(
				"\n"
				"\t             Encrypted title key: "
			);
			for (INT i = 0; i < 0x10; i++) {
				OutputVolDescLogA("%02x", buf[0x01BF + i]);
			}

			// https://wiibrew.org/wiki/Wii_Security
			BYTE decTitleKey[16] = {};
			mbedtls_aes_context context = {};
			if (bDecOK) {
				BYTE iv[16] = {};
				BYTE encTitleKey[16] = {};
				memcpy(iv, &buf[0x1DC], 8);
				memcpy(encTitleKey, &buf[0x1BF], sizeof(encTitleKey));

				mbedtls_aes_setkey_dec(&context, commonKey, 128);
				mbedtls_aes_crypt_cbc(&context, MBEDTLS_AES_DECRYPT, 16, iv, encTitleKey, decTitleKey);
				mbedtls_aes_setkey_dec(&context, decTitleKey, 128);
			}
			OutputVolDescLogA(
				"\n"
				"\t                         Unknown: %#02x\n"
				"\t                       ticket_id: %#llx\n"
				"\t                      Console ID: %#x\n"
				"\t                        Title ID: %#llx\n"
				"\t                         Unknown: %#04x\n"
				"\t            Ticket title version: %#x\n"
				"\t           Permitted Titles Mask: %#x\n"
				"\t                     Permit mask: %#x\n"
				"\t                    Title Export: %#x\n"
				"\t                Common Key index: %#x\n"
				"\t                         Unknown: "
				, buf[0x01CF]
				, MAKEUINT64(MAKEUINT(MAKEWORD(buf[0x01D7], buf[0x01D6]), MAKEWORD(buf[0x01D5], buf[0x01D4]))
					, MAKEUINT(MAKEWORD(buf[0x01D3], buf[0x01D2]), MAKEWORD(buf[0x01D1], buf[0x01D0])))
				, MAKEUINT(MAKEWORD(buf[0x01DB], buf[0x01DA]), MAKEWORD(buf[0x01D9], buf[0x01D8]))
				, MAKEUINT64(MAKEUINT(MAKEWORD(buf[0x01E3], buf[0x01E2]), MAKEWORD(buf[0x01E1], buf[0x01E0]))
					, MAKEUINT(MAKEWORD(buf[0x01DF], buf[0x01DE]), MAKEWORD(buf[0x01DD], buf[0x01DC])))
				, MAKEWORD(buf[0x01E5], buf[0x01E4])
				, MAKEWORD(buf[0x01E7], buf[0x01E6])
				, MAKEUINT(MAKEWORD(buf[0x01EB], buf[0x01EA]), MAKEWORD(buf[0x01E9], buf[0x01E8]))
				, MAKEUINT(MAKEWORD(buf[0x01EF], buf[0x01EE]), MAKEWORD(buf[0x01ED], buf[0x01EC]))
				, buf[0x01F0], buf[0x01F1]
			);
			for (INT i = 0; i < 0x30; i++) {
				OutputVolDescLogA("%02x", buf[0x01F2 + i]);
			}
			OutputVolDescLogA(
				"\n"
				"\t      Content access permissions: "
			);
			for (INT i = 0; i < 0x40; i++) {
				OutputVolDescLogA("%02x", buf[0x0222 + i]);
			}
			OutputVolDescLogA("\n");
			for (INT i = 0; i < 7 * 8; i += 8) {
				UINT enable = MAKEUINT(MAKEWORD(buf[0x0267 + i], buf[0x0266 + i]), MAKEWORD(buf[0x0265 + i], buf[0x0264 + i]));
				if (enable) {
					OutputVolDescLogA(
						"\t               Enable time limit: %d\n"
						"\t            Time limit (Seconds): %d\n"
						, enable
						, MAKEUINT(MAKEWORD(buf[0x026B + i], buf[0x026A + i]), MAKEWORD(buf[0x0269 + i], buf[0x0268 + i]))
					);
				}
			}
			UINT dataSize = MAKEUINT(MAKEWORD(buf[0x02BF], buf[0x02BE]), MAKEWORD(buf[0x02BD], buf[0x02BC]));
			OutputVolDescLogA(
				"\t                        TMD size: %d(%#x)\n"
				"\t                      TMD offset: %#x\n"
				"\t                 Cert chain size: %d(%#x)\n"
				"\t               Cert chain offset: %#x\n"
				"\t          Offset to the H3 table: %#x\n"
				"\t                     Data offset: %#x\n"
				"\t                       Data size: %d(%#x)\n"
				, MAKEUINT(MAKEWORD(buf[0x02A7], buf[0x02A6]), MAKEWORD(buf[0x02A5], buf[0x02A4]))
				, MAKEUINT(MAKEWORD(buf[0x02A7], buf[0x02A6]), MAKEWORD(buf[0x02A5], buf[0x02A4]))
				, MAKEUINT(MAKEWORD(buf[0x02AB], buf[0x02AA]), MAKEWORD(buf[0x02A9], buf[0x02A8]))
				, MAKEUINT(MAKEWORD(buf[0x02AF], buf[0x02AE]), MAKEWORD(buf[0x02AD], buf[0x02AC]))
				, MAKEUINT(MAKEWORD(buf[0x02AF], buf[0x02AE]), MAKEWORD(buf[0x02AD], buf[0x02AC]))
				, MAKEUINT(MAKEWORD(buf[0x02B3], buf[0x02B2]), MAKEWORD(buf[0x02B1], buf[0x02B0]))
				, MAKEUINT(MAKEWORD(buf[0x02B7], buf[0x02B6]), MAKEWORD(buf[0x02B5], buf[0x02B4]))
				, MAKEUINT(MAKEWORD(buf[0x02BB], buf[0x02BA]), MAKEWORD(buf[0x02B9], buf[0x02B8]))
				, dataSize, dataSize
			);

			if (fread(buf, sizeof(BYTE), 0x1E0, fp) != 0x1E0) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				FcloseAndNull(fp);
				return FALSE;
			};
			OutputCDMain(fileMainInfo, buf, (INT)realOfsOfPartion / 0x800, 0x1E0);
			sigType = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
			OutputVolDescLogA(
				OUTPUT_DHYPHEN_PLUS_STR(Partition)
				"\t                  Signature type: %#x\n"
				"\t                       Signature: "
				, sigType
			);
			size = 256;
			if (sigType == 0x10000) {
				size = 512;
			}
			for (INT i = 0; i < size; i++) {
				OutputVolDescLogA("%02x", buf[4 + i]);
			}
			OutputVolDescLogA(
				"\n"
				"\t               Padding modulo 64: "
			);
			for (INT i = 0; i < 60; i++) {
				OutputVolDescLogA("%02x", buf[0x104 + i]);
			}
			OutputVolDescLogA(
				"\n"
				"\t                          Issuer: %s\n"
				"\t                         Version: %x\n"
				"\t                  ca_crl_version: %x\n"
				"\t              signer_crl_version: %x\n"
				"\t               Padding modulo 64: %x\n"
				"\t                  System Version: %llx\n"
				"\t                        Title ID: %llx\n"
				"\t                      Title type: %x\n"
				"\t                        Group ID: %x\n"
				"\t                          Region: %x\n"
				"\t                         Ratings: "
				, (LPCH)&buf[0x140], buf[0x180], buf[0x181], buf[0x182], buf[0x183]
				, MAKEUINT64(MAKEUINT(MAKEWORD(buf[0x18B], buf[0x18A]), MAKEWORD(buf[0x189], buf[0x188]))
					, MAKEUINT(MAKEWORD(buf[0x187], buf[0x186]), MAKEWORD(buf[0x185], buf[0x184])))
				, MAKEUINT64(MAKEUINT(MAKEWORD(buf[0x193], buf[0x192]), MAKEWORD(buf[0x191], buf[0x190]))
					, MAKEUINT(MAKEWORD(buf[0x18F], buf[0x18E]), MAKEWORD(buf[0x18D], buf[0x18C])))
				, MAKEUINT(MAKEWORD(buf[0x197], buf[0x196]), MAKEWORD(buf[0x195], buf[0x194]))
				, MAKEWORD(buf[0x199], buf[0x198])
				, MAKEWORD(buf[0x19D], buf[0x19C])
			);
			for (INT i = 0; i < 16; i++) {
				OutputVolDescLogA("%02x", buf[0x19E + i]);
			}
			OutputVolDescLogA(
				"\n"
				"\t                        IPC Mask: "
			);
			for (INT i = 0; i < 12; i++) {
				OutputVolDescLogA("%02x", buf[0x1BA + i]);
			}
			WORD numOfContents = MAKEWORD(buf[0x1DF], buf[0x1DE]);
			OutputVolDescLogA(
				"\n"
				"\t                   Access rights: %x\n"
				"\t                   Title version: %x\n"
				"\t              Number of contents: %d\n"
				, MAKEUINT(MAKEWORD(buf[0x1DB], buf[0x1DA]), MAKEWORD(buf[0x1D9], buf[0x1D8]))
				, MAKEWORD(buf[0x1DD], buf[0x1DC]), numOfContents
			);
			if (fread(buf, sizeof(BYTE), 4, fp) != 4) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				FcloseAndNull(fp);
				return FALSE;
			};
			OutputCDMain(fileMainInfo, buf, (INT)realOfsOfPartion / 0x800, 4);
			OutputVolDescLogA(
				"\t                      boot index: %d\n"
				"\t               Padding modulo 64: %d\n"
				, MAKEWORD(buf[1], buf[0]), MAKEWORD(buf[3], buf[2])
			);
			size_t contentsSize = (size_t)36 * numOfContents;
			if (fread(buf, sizeof(BYTE), contentsSize, fp) != contentsSize) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				FcloseAndNull(fp);
				return FALSE;
			};
			OutputCDMain(fileMainInfo, buf, (INT)realOfsOfPartion / 0x800, 36 * numOfContents);
			for (size_t i = 0; i < contentsSize; i += 36) {
				OutputVolDescLogA(
					"\t                      Content ID: %d\n"
					"\t                           Index: %d\n"
					"\t                            Type: %d\n"
					"\t                            Size: %lld(%#llx)\n"
					"\t                       SHA1 hash: "
					, MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]))
					, MAKEWORD(buf[5], buf[4])
					, MAKEWORD(buf[7], buf[6])
					, MAKEUINT64(MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], buf[12]))
						, MAKEUINT(MAKEWORD(buf[11], buf[10]), MAKEWORD(buf[9], buf[8])))
					, MAKEUINT64(MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], buf[12]))
						, MAKEUINT(MAKEWORD(buf[11], buf[10]), MAKEWORD(buf[9], buf[8])))
				);
				for (INT j = 0; j < 20; j++) {
					OutputVolDescLogA("%02x", buf[16 + j + i]);
				}
			}
			OutputVolDescLogA("\n");

			fseek(fp, 24, SEEK_CUR);
			for (INT k = 0; k < 3; k++) {
				if (fread(buf, sizeof(BYTE), 4, fp) != 4) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					FcloseAndNull(fp);
					return FALSE;
				};
				sigType = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
				if (sigType == 0x10000) {
					size = 512;
					if (fread(&buf[4], sizeof(BYTE), 1020, fp) != 1020) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						FcloseAndNull(fp);
						return FALSE;
					};
				}
				else if (sigType == 0x10001) {
					size = 256;
					if (fread(&buf[4], sizeof(BYTE), 764, fp) != 764) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						FcloseAndNull(fp);
						return FALSE;
					};
				}
				OutputVolDescLogA(
					OUTPUT_DHYPHEN_PLUS_STR(Certificates)
					"\t                  Signature type: %#x\n"
					"\t                       Signature: "
					, sigType
				);
				for (INT i = 0; i < size; i++) {
					OutputVolDescLogA("%02x", buf[4 + i]);
				}
				INT ofs = 4 + size + 60;
				OutputVolDescLogA(
					"\n"
					"\t                          Issuer: %s\n"
					"\t                             Tag: %x\n"
					"\t                            Name: %s\n"
					"\t                             Key: "
					, (LPCH)&buf[ofs]
					, MAKEUINT(MAKEWORD(buf[ofs + 67], buf[ofs + 66]), MAKEWORD(buf[ofs + 65], buf[ofs + 64]))
					, (LPCH)&buf[ofs + 68]
				);
				for (INT i = 0; i < 316; i++) {
					OutputVolDescLogA("%02x", buf[ofs + 132 + i]);
				}
				OutputVolDescLogA("\n");
			}

			if (bDecOK) {
				CHAR decPath[_MAX_PATH] = {};
				FILE* fpDec = CreateOrOpenFileA(pszFullPath, "_dec"
					, decPath, NULL, NULL, ".iso", "wb", (BYTE)idx2, (BYTE)numOfPartition[idx]);
				if (!fpDec) {
					OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
					return FALSE;
				}
				BYTE decBuf[0x8000] = {};
				for (UINT i = 0; i < dataSize; i += 0x8000) {
					fseek(fp, (LONG)(realOfsOfPartion + 0x20000 + i), SEEK_SET);
					if (fread(buf, sizeof(BYTE), 0x8000, fp) != 0x8000) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						FcloseAndNull(fp);
						FcloseAndNull(fpDec);
						return FALSE;
					};
					mbedtls_aes_crypt_cbc(&context, MBEDTLS_AES_DECRYPT, 0x7c00, &buf[0x3D0], &buf[0x400], decBuf);
					fwrite(decBuf, sizeof(BYTE), 0x7c00, fpDec);
					OutputString(_T("\rDecrypting iso %7u/%7u"), i, dataSize);
				}
				OutputString("\n");
				FcloseAndNull(fpDec);
				ReadNintendoFileSystem(pDevice, decPath, wii);
			}
		}
	}
	FcloseAndNull(fp);
	return TRUE;
}

BOOL ReadBDForParamSfo(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf
) {
	DWORD tmpTransfer = 1;
	REVERSE_BYTES(pCdb->TransferLength, &tmpTransfer);
	REVERSE_BYTES(pCdb->LogicalBlock, &pDisc->BD.nLBAForParamSfo);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, pCdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	// http://www.psdevwiki.com/ps3/PARAM.SFO
	typedef struct _sfo_header {
		UINT magic; /************ Always PSF */
		UINT version; /********** Usually 1.1 */
		UINT key_table_start; /** Start offset of key_table */
		UINT data_table_start; /* Start offset of data_table */
		UINT tables_entries; /*** Number of entries in all tables */
	} sfo_header, *psfo_header;

	typedef struct _sfo_index_table_entry {
		WORD key_offset; /*** param_key offset (relative to start offset of key_table) */
		WORD data_fmt; /***** param_data data type */
		UINT data_len; /***** param_data used bytes */
		UINT data_max_len; /* param_data total bytes */
		UINT data_offset; /** param_data offset (relative to start offset of data_table) */
	} sfo_index_table_entry, *psfo_index_table_entry;

	psfo_header header = (psfo_header)lpBuf;
	OutputDiscLogA(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(PARAM.SFO)
		"\tmagic: %c%c%c\n"
		"\tversion: %d.%02d\n"
		, pDisc->BD.nLBAForParamSfo, pDisc->BD.nLBAForParamSfo
		, ((header->magic >> 8) & 0x000000ff)
		, ((header->magic >> 16) & 0x000000ff), ((header->magic >> 24) & 0x000000ff)
		, (header->version & 0x000000ff), ((header->version >> 8) & 0x000000ff)
	);

	LPBYTE keytable = lpBuf + header->key_table_start;
	LPBYTE datatable = lpBuf + header->data_table_start;
	for (UINT i = 0; i < header->tables_entries; i++) {
		psfo_index_table_entry entry =
			(psfo_index_table_entry)(lpBuf + sizeof(sfo_header) + sizeof(sfo_index_table_entry) * i);
#if 0
		OutputDiscLogA(
			"\tkey_offset[%d]: %d\n"
			"\tdata_fmt[%d]: %d\n"
			"\tdata_len[%d]: %d\n"
			"\tdata_max_len[%d]: %d\n"
			"\tdata_offset[%d]: %d\n"
			, i, entry[i]->key_offset, i, entry[i]->data_fmt, i
			, entry[i]->data_len, i, entry[i]->data_max_len, i, entry[i]->data_offset
		);
#endif
		if (entry->data_fmt == 516) {
			OutputDiscLogA(
				"\t%s: %s\n", keytable + entry->key_offset, datatable + entry->data_offset);
		}
		else if (entry->data_fmt == 1028) {
			LPBYTE ofs = datatable + entry->data_offset;
			UINT data = MAKEUINT(MAKEWORD(ofs[0], ofs[1]), MAKEWORD(ofs[2], ofs[3]));
			OutputDiscLogA(
				"\t%s: %x\n", keytable + entry->key_offset, data);
		}
	}
	return TRUE;
}

BOOL ReadSACDFileSystem(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	LPBYTE pBuf = NULL;
	LPBYTE lpBuf = NULL;
	if (!GetAlignedCallocatedBuffer(pDevice, &pBuf,
		(UINT)pDevice->dwMaxTransferLength, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	CDB::_READ12 cdb = {};
	cdb.OperationCode = SCSIOP_READ12;
	BYTE byScsiStatus = 0;
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	cdb.TransferLength[3] = 1;
	INT nLBA = 510;
	REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);

	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		FreeAndNull(pBuf);
		return FALSE;
	}
	OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);
	typedef struct _Locale {
		CHAR Language_Code[2];
		BYTE Character_Set_Code;
		BYTE Reserved;
	} Locale;
	typedef struct _TextChannel {
		BYTE Max_Text_Channels;
		BYTE Reserved6[7];
		Locale locale[8];
	} Text_Channel;
	typedef struct _Master_TOC {
		CHAR Master_TOC_Signature[8];	// 0..15 M_TOC_0_Header
		BYTE Major_Version;
		BYTE Minor_Version;
		BYTE Reserved1[6];
		USHORT Album_Set_Size;			// 16..63 Album_Info
		USHORT Album_Sequence_Number;
		BYTE Reserved2[4];
		CHAR Album_Catalog_Number[16];
		BYTE Album_Genre[4][4];
		BYTE Reserved3[8];
		UINT TWOCH_TOC_1_Address;		// 64..127 Disc_Info
		UINT TWOCH_TOC_2_Address;
		UINT MC_TOC_1_Address;
		UINT MC_TOC_2_Address;
		BYTE Disc_Flags_Reserved : 7;
		BYTE Disc_Flags_Hybrid : 1;
		BYTE Reserved4[3];
		USHORT TWOCH_TOC_Len;
		USHORT MC_TOC_Len;
		BYTE Disc_Catalog_Number[16];
		BYTE Disc_Genre[4][4];
		USHORT Disc_Date_Y;
		BYTE Disc_Date_M;
		BYTE Disc_Date_D;
		UINT Reserved5;
		Text_Channel Txt_Ch;
		BYTE Disc_WebLink_Info[128];
	} Master_TOC;
	Master_TOC mToc = {};
	memcpy(&mToc, lpBuf, sizeof(Master_TOC));
	REVERSE_BYTES_SHORT(&mToc.Album_Set_Size, &lpBuf[0x10]);
	REVERSE_BYTES_SHORT(&mToc.Album_Sequence_Number, &lpBuf[0x12]);
	REVERSE_BYTES(&mToc.TWOCH_TOC_1_Address, &lpBuf[0x40]);
	REVERSE_BYTES(&mToc.TWOCH_TOC_2_Address, &lpBuf[0x44]);
	REVERSE_BYTES(&mToc.MC_TOC_1_Address, &lpBuf[0x48]);
	REVERSE_BYTES(&mToc.MC_TOC_2_Address, &lpBuf[0x4c]);
	REVERSE_BYTES_SHORT(&mToc.TWOCH_TOC_Len, &lpBuf[0x54]);
	REVERSE_BYTES_SHORT(&mToc.MC_TOC_Len, &lpBuf[0x56]);
	REVERSE_BYTES_SHORT(&mToc.Disc_Date_Y, &lpBuf[0x78]);
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR(Master_TOC)
		"\t Master_TOC_Signature: %.8s\n"
		"\t         Spec_Version: %d.%02d\n"
		"\t       Album_Set_Size: %d\n"
		"\tAlbum_Sequence_Number: %d\n"
		"\t Album_Catalog_Number: %.16s\n"
		"\t         Album_Genre1: %02x %02x %02x %02x\n"
		"\t         Album_Genre2: %02x %02x %02x %02x\n"
		"\t         Album_Genre3: %02x %02x %02x %02x\n"
		"\t         Album_Genre4: %02x %02x %02x %02x\n"
		"\t  TWOCH_TOC_1_Address: %d (%#x)\n"
		"\t  TWOCH_TOC_2_Address: %d (%#x)\n"
		"\t     MC_TOC_1_Address: %d (%#x)\n"
		"\t     MC_TOC_2_Address: %d (%#x)\n"
		"\t    Disc_Flags_Hybrid: %d\n"
		"\t        TWOCH_TOC_Len: %d (%#x)\n"
		"\t           MC_TOC_Len: %d (%#x)\n"
		"\t  Disc_Catalog_Number: %.16s\n"
		"\t          Disc_Genre1: %02x %02x %02x %02x\n"
		"\t          Disc_Genre2: %02x %02x %02x %02x\n"
		"\t          Disc_Genre3: %02x %02x %02x %02x\n"
		"\t          Disc_Genre4: %02x %02x %02x %02x\n"
		"\t            Disc_Date: %d/%02d/%02d\n"
		"\t    Max_Text_Channels: %d\n"
		, (LPCH)&mToc.Master_TOC_Signature[0]
		, mToc.Major_Version, mToc.Minor_Version
		, mToc.Album_Set_Size
		, mToc.Album_Sequence_Number
		, (LPCH)&mToc.Album_Catalog_Number[0]
		, mToc.Album_Genre[0][0], mToc.Album_Genre[0][1], mToc.Album_Genre[0][2], mToc.Album_Genre[0][3]
		, mToc.Album_Genre[1][0], mToc.Album_Genre[1][1], mToc.Album_Genre[1][2], mToc.Album_Genre[1][3]
		, mToc.Album_Genre[2][0], mToc.Album_Genre[2][1], mToc.Album_Genre[2][2], mToc.Album_Genre[2][3]
		, mToc.Album_Genre[3][0], mToc.Album_Genre[3][1], mToc.Album_Genre[3][2], mToc.Album_Genre[3][3]
		, mToc.TWOCH_TOC_1_Address, mToc.TWOCH_TOC_1_Address
		, mToc.TWOCH_TOC_2_Address, mToc.TWOCH_TOC_2_Address
		, mToc.MC_TOC_1_Address, mToc.MC_TOC_1_Address
		, mToc.MC_TOC_2_Address, mToc.MC_TOC_2_Address
		, mToc.Disc_Flags_Hybrid
		, mToc.TWOCH_TOC_Len, mToc.TWOCH_TOC_Len
		, mToc.MC_TOC_Len, mToc.MC_TOC_Len
		, (LPCH)&mToc.Disc_Catalog_Number[0]
		, mToc.Disc_Genre[0][0], mToc.Disc_Genre[0][1], mToc.Disc_Genre[0][2], mToc.Disc_Genre[0][3]
		, mToc.Disc_Genre[1][0], mToc.Disc_Genre[1][1], mToc.Disc_Genre[1][2], mToc.Disc_Genre[1][3]
		, mToc.Disc_Genre[2][0], mToc.Disc_Genre[2][1], mToc.Disc_Genre[2][2], mToc.Disc_Genre[2][3]
		, mToc.Disc_Genre[3][0], mToc.Disc_Genre[3][1], mToc.Disc_Genre[3][2], mToc.Disc_Genre[3][3]
		, mToc.Disc_Date_Y, mToc.Disc_Date_M, mToc.Disc_Date_D
		, mToc.Txt_Ch.Max_Text_Channels
	);
	CHAR Lang[][14] {
		"UNKNOWN", "ISO646", "ISO8859_1", "RIS506", "KSC5601", "GB2312", "BIG5", "ISO8859_1_ESC"
	};
	for (INT i = 0; i < mToc.Txt_Ch.Max_Text_Channels; i++) {
		OutputVolDescLogA(
			"\t     Language_Code[%d]: %.2s\n"
			"\tCharacter_Set_Code[%d]: %s\n"
			, i + 1, (LPCH)&mToc.Txt_Ch.locale[i].Language_Code[0]
			, i + 1, (LPCH)&Lang[mToc.Txt_Ch.locale[i].Character_Set_Code]
		);
	}

	typedef struct _Master_Text {
		CHAR Master_Text_Signature[8];
		BYTE Reserved1[8];
		USHORT Album_Title_Ptr;
		USHORT Album_Artist_Ptr;
		USHORT Album_Publisher_Ptr;
		USHORT Album_Copyright_Ptr;
		USHORT Album_Title_Phonetic_Ptr;
		USHORT Album_Artist_Phonetic_Ptr;
		USHORT Album_Publisher_Phonetic_Ptr;
		USHORT Album_Copyright_Phonetic_Ptr;
		USHORT Disc_Title_Ptr;
		USHORT Disc_Artist_Ptr;
		USHORT Disc_Publisher_Ptr;
		USHORT Disc_Copyright_Ptr;
		USHORT Disc_Title_Phonetic_Ptr;
		USHORT Disc_Artist_Phonetic_Ptr;
		USHORT Disc_Publisher_Phonetic_Ptr;
		USHORT Disc_Copyright_Phonetic_Ptr;
		BYTE Reserved2[16];
	} Master_Text;

	for (nLBA = 511; nLBA <= 518; nLBA++) {
		REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);
		Master_Text mText = {};
		REVERSE_BYTES_SHORT(&mText.Album_Title_Ptr, &lpBuf[0x10]);
		REVERSE_BYTES_SHORT(&mText.Album_Artist_Ptr, &lpBuf[0x12]);
		REVERSE_BYTES_SHORT(&mText.Album_Publisher_Ptr, &lpBuf[0x14]);
		REVERSE_BYTES_SHORT(&mText.Album_Copyright_Ptr, &lpBuf[0x16]);
		REVERSE_BYTES_SHORT(&mText.Album_Title_Phonetic_Ptr, &lpBuf[0x18]);
		REVERSE_BYTES_SHORT(&mText.Album_Artist_Phonetic_Ptr, &lpBuf[0x1a]);
		REVERSE_BYTES_SHORT(&mText.Album_Publisher_Phonetic_Ptr, &lpBuf[0x1c]);
		REVERSE_BYTES_SHORT(&mText.Album_Copyright_Phonetic_Ptr, &lpBuf[0x1e]);
		REVERSE_BYTES_SHORT(&mText.Disc_Title_Ptr, &lpBuf[0x20]);
		REVERSE_BYTES_SHORT(&mText.Disc_Artist_Ptr, &lpBuf[0x22]);
		REVERSE_BYTES_SHORT(&mText.Disc_Publisher_Ptr, &lpBuf[0x24]);
		REVERSE_BYTES_SHORT(&mText.Disc_Copyright_Ptr, &lpBuf[0x26]);
		REVERSE_BYTES_SHORT(&mText.Disc_Title_Phonetic_Ptr, &lpBuf[0x28]);
		REVERSE_BYTES_SHORT(&mText.Disc_Artist_Phonetic_Ptr, &lpBuf[0x2a]);
		REVERSE_BYTES_SHORT(&mText.Disc_Publisher_Phonetic_Ptr, &lpBuf[0x2c]);
		REVERSE_BYTES_SHORT(&mText.Disc_Copyright_Phonetic_Ptr, &lpBuf[0x2e]);
		OutputVolDescLogA(
			OUTPUT_DHYPHEN_PLUS_STR(Master_Text)
			"\t   Master_Text_Signature: %8s\n", (LPCH)&lpBuf[0]
		);
		OutputVolDescLogA("\t             Album_Title: ");
		if (mText.Album_Title_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Title_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Album_Title_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t            Album_Artist: ");
		if (mText.Album_Artist_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Artist_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Album_Artist_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t         Album_Publisher: ");
		if (mText.Album_Publisher_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Publisher_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Album_Publisher_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t         Album_Copyright: ");
		if (mText.Album_Copyright_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Copyright_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Album_Copyright_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t    Album_Title_Phonetic: ");
		if (mText.Album_Title_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Title_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Album_Title_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t   Album_Artist_Phonetic: ");
		if (mText.Album_Artist_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Artist_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Album_Artist_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\tAlbum_Publisher_Phonetic: ");
		if (mText.Album_Publisher_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Publisher_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Album_Publisher_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\tAlbum_Copyright_Phonetic: ");
		if (mText.Album_Copyright_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Copyright_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Album_Copyright_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t              Disc_Title: ");
		if (mText.Disc_Title_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Title_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Disc_Title_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t             Disc_Artist: ");
		if (mText.Disc_Artist_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Artist_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Disc_Artist_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t          Disc_Publisher: ");
		if (mText.Disc_Publisher_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Publisher_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Disc_Publisher_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t          Disc_Copyright: ");
		if (mText.Disc_Copyright_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Copyright_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Disc_Copyright_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t     Disc_Title_Phonetic: ");
		if (mText.Disc_Title_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Title_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Disc_Title_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t    Disc_Artist_Phonetic: ");
		if (mText.Disc_Artist_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Artist_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Disc_Artist_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t Disc_Publisher_Phonetic: ");
		if (mText.Disc_Publisher_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Publisher_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Disc_Publisher_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n\t Disc_Copyright_Phonetic: ");
		if (mText.Disc_Copyright_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Copyright_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLogA("%c", lpBuf[mText.Disc_Copyright_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLogA("\n");
	}
	nLBA = 519;
	REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);

	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		FreeAndNull(pBuf);
		return FALSE;
	}
	OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR(Manufacture)
		"\tManuf_Info_Signature: %.8s\n", (LPCH)&lpBuf[0]);

	INT nChToc[] = { (INT)mToc.TWOCH_TOC_1_Address , (INT)mToc.MC_TOC_1_Address, 0 };
	for (INT c = 0; nChToc[c] != 0; c++) {
		nLBA = nChToc[c];
		REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);

		typedef struct _Area_TOC {
			CHAR Area_TOC_Signature[8];	// 0..15 A_TOC_0_Header
			BYTE Major_Version;
			BYTE Minor_Version;
			USHORT Area_TOC_Length;
			UINT Reserved1;
			UINT Unknown1;				// 16..127 Area_Data 
			BYTE Unknown2;
			BYTE Frame_Format;
			BYTE Reserved2[10];
			BYTE N_Channels;
			BYTE Unknown3[15];
			BYTE Unknown4[16];
			BYTE m;
			BYTE s;
			BYTE f;
			BYTE Reserved3;
			BYTE Unknown5;
			BYTE Last_Track_Number;
			BYTE Reserved4[2];
			UINT Audio_Start_Address;
			UINT Audio_End_Address;
			Text_Channel Txt_Ch;
			USHORT Track_Text_Ptr;		// 128..143 List_Pointers
			USHORT Index_List_Ptr;
			USHORT Access_List_Ptr;
			USHORT Track_WebLink_List_Ptr;
			BYTE Reserved7[8];
			BYTE Area_Text[1904];
		} Area_TOC;
		Area_TOC aToc = {};
		memcpy(&aToc, lpBuf, sizeof(Area_TOC));
		REVERSE_BYTES(&aToc.Unknown1, &lpBuf[0x10]);
		REVERSE_BYTES_SHORT(&aToc.Area_TOC_Length, &lpBuf[0xa]);
		REVERSE_BYTES(&aToc.Audio_Start_Address, &lpBuf[0x48]);
		REVERSE_BYTES(&aToc.Audio_End_Address, &lpBuf[0x4c]);
		OutputVolDescLogA(
			OUTPUT_DHYPHEN_PLUS_STR(Area_TOC)
			"\t    Area_TOC_Signature: %.8s\n"
			"\t          Spec_Version: %d.%02d\n"
			"\t       Area_TOC_Length: %d (%#02x)\n"
			"\t               Unknown: %d (%#02x)\n"
			"\t               Unknown: %d (%#02x)\n"
			"\t          Frame_Format: %d\n"
			"\t            N_Channels: %d\n"
			"\t             Total MSF: %02d:%02d:%02d\n"
			"\t               Unknown: %d\n"
			"\t     Last_Track_Number: %d\n"
			"\t   Audio_Start_Address: %d (%#02x)\n"
			"\t     Audio_End_Address: %d (%#02x)\n"
			"\t     Max_Text_Channels: %d\n"
			, (LPCH)&lpBuf[0]
			, aToc.Major_Version, aToc.Minor_Version
			, aToc.Area_TOC_Length, aToc.Area_TOC_Length
			, aToc.Unknown1, aToc.Unknown1
			, aToc.Unknown2, aToc.Unknown2
			, aToc.Frame_Format
			, aToc.N_Channels
			, aToc.m, aToc.s, aToc.f
			, aToc.Unknown5, aToc.Last_Track_Number
			, aToc.Audio_Start_Address, aToc.Audio_Start_Address
			, aToc.Audio_End_Address, aToc.Audio_End_Address
			, aToc.Txt_Ch.Max_Text_Channels
		);
		for (INT i = 0; i < aToc.Txt_Ch.Max_Text_Channels; i++) {
			OutputVolDescLogA(
				"\t      Language_Code[%d]: %.2s\n"
				"\t Character_Set_Code[%d]: %s\n"
				, i + 1, (LPCH)&aToc.Txt_Ch.locale[i].Language_Code[0]
				, i + 1, (LPCH)&Lang[aToc.Txt_Ch.locale[i].Character_Set_Code]
			);
		}
		REVERSE_BYTES_SHORT(&aToc.Track_Text_Ptr, &lpBuf[0x80]);
		REVERSE_BYTES_SHORT(&aToc.Index_List_Ptr, &lpBuf[0x82]);
		REVERSE_BYTES_SHORT(&aToc.Access_List_Ptr, &lpBuf[0x84]);
		REVERSE_BYTES_SHORT(&aToc.Track_WebLink_List_Ptr, &lpBuf[0x88]);
		OutputVolDescLogA(
			"\t        Track_Text_Ptr: %d (%#02x)\n"
			"\t        Index_List_Ptr: %d (%#02x)\n"
			"\t       Access_List_Ptr: %d (%#02x)\n"
			"\tTrack_WebLink_List_Ptr: %d (%#02x)\n"
			, aToc.Track_Text_Ptr, aToc.Track_Text_Ptr
			, aToc.Index_List_Ptr, aToc.Index_List_Ptr
			, aToc.Access_List_Ptr, aToc.Access_List_Ptr
			, aToc.Track_WebLink_List_Ptr, aToc.Track_WebLink_List_Ptr
		);

		for (nLBA = nChToc[c] + 1; nLBA <= nChToc[c] + 2; nLBA++) {
			REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				FreeAndNull(pBuf);
				return FALSE;
			}
			OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);

			typedef struct _Track_List {
				CHAR Track_List_1_Signature[8];
				UINT Track_Start_Address[255];
				UINT Track_Length[255];
			} Track_List;
			Track_List tList = {};
			OutputVolDescLogA(
				OUTPUT_DHYPHEN_PLUS_STR(Track_List)
				"\t Track_List_1_Signature: %.8s\n"
				, (LPCH)&lpBuf[0]
			);
			for (INT i = 0; i < aToc.Last_Track_Number; i++) {
				REVERSE_BYTES(&tList.Track_Start_Address[i], &lpBuf[0x8 + sizeof(UINT) * i]);
				OutputVolDescLogA(
					"\tTrack_Start_Address[%02d]: %9d (%#02x)\n"
					, i + 1, tList.Track_Start_Address[i], tList.Track_Start_Address[i]
				);
			}
			for (INT i = 0; i < aToc.Last_Track_Number; i++) {
				REVERSE_BYTES(&tList.Track_Length[i], &lpBuf[0x404 + sizeof(UINT) * i]);
				OutputVolDescLogA(
					"\t       Track_Length[%02d]: %9d (%#02x)\n"
					, i + 1, tList.Track_Length[i], tList.Track_Length[i]
				);
			}
		}

		nLBA = nChToc[c] + 3;
		REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
		cdb.TransferLength[3] = 2;

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_RAW_READ_SIZE * 2, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE * 2);

		OutputVolDescLogA(
			OUTPUT_DHYPHEN_PLUS_STR(ISRC_and_Genre_List)
			"\tISRC_and_Genre_List_Signature: %.8s\n"
			, (LPCH)&lpBuf[0]
		);
		for (INT i = 0; i < aToc.Last_Track_Number; i++) {
			OutputVolDescLogA(
				"\t           ISRC_and_Genre[%02d]: %.12s\n"
				, i + 1, (LPCH)&lpBuf[0x8 + 12 * i]
			);
		}

		if (aToc.Access_List_Ptr) {
			nLBA = nChToc[c] + aToc.Access_List_Ptr;
			REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
			cdb.TransferLength[3] = 32;

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_RAW_READ_SIZE * 32, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				FreeAndNull(pBuf);
				return FALSE;
			}
			OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE * 32);
#pragma pack(push, acc, 1)
			typedef struct _Main_Acc_List {
				USHORT Access_Flags;
				BYTE Entrys[3];
			} Main_Acc_List;
			typedef struct _Detailed_Access {
				BYTE Sub_Entrys[3];
			} Detailed_Access;
#pragma pack(pop, acc)
			typedef struct _Access_List {
				CHAR Access_List_Signature[8];
				USHORT N_Entries;
				BYTE Main_Step_Size;
				BYTE Reserved[5];
				Main_Acc_List mAList[6550];
				Detailed_Access dAccess[6550];
			} Access_List;
			Access_List alist = {};
			memcpy(&alist, lpBuf, sizeof(Access_List));
			REVERSE_BYTES_SHORT(&alist.N_Entries, &lpBuf[0x8]);

			OutputVolDescLogA(
				OUTPUT_DHYPHEN_PLUS_STR(Access_List)
				"\tAccess_List_Signature: %.8s\n"
				"\t            N_Entries: %d\n"
				"\t       Main_Step_Size: %d\n"
				, (LPCH)&lpBuf[0]
				, alist.N_Entries
				, alist.Main_Step_Size
			);
			for (INT i = 0; i < alist.N_Entries; i++) {
				REVERSE_BYTES_SHORT(&alist.mAList[i].Access_Flags, &lpBuf[0x10 + 5 * i]);
				if (alist.mAList[i].Access_Flags != 0 || alist.mAList[i].Entrys[0] != 0 ||
					alist.mAList[i].Entrys[1] != 0 || alist.mAList[i].Entrys[2] != 0) {
					OutputVolDescLogA(
						"\t   Access_Flags[%04d]: %d\n"
						"\t         Entrys[%04d]: %02x %02x %02x\n"
						, i + 1, alist.mAList[i].Access_Flags
						, i + 1, alist.mAList[i].Entrys[0], alist.mAList[i].Entrys[1], alist.mAList[i].Entrys[2]
					);
				}
				if (alist.dAccess[i].Sub_Entrys[0] != 0 ||
					alist.dAccess[i].Sub_Entrys[1] != 0 || alist.dAccess[i].Sub_Entrys[2] != 0) {
					OutputVolDescLogA(
						"\t     Sub_Entrys[%04d]: %02x %02x %02x\n"
						, i + 1, alist.dAccess[i].Sub_Entrys[0], alist.dAccess[i].Sub_Entrys[1], alist.dAccess[i].Sub_Entrys[2]
					);
				}
			}
		}

		if (aToc.Track_Text_Ptr) {
			nLBA = nChToc[c] + aToc.Track_Text_Ptr;
			REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
			cdb.TransferLength[3] = 1;

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				FreeAndNull(pBuf);
				return FALSE;
			}
			OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);

			typedef struct _Track_Text {
				CHAR Track_Text_Signature[8];
				USHORT Track_Text_Pos[255];
			} Track_Text;

			Track_Text TTxt = {};
			OutputVolDescLogA(
				OUTPUT_DHYPHEN_PLUS_STR(Track_Text)
				"\tTrack_Text_Signature: %.8s\n"
				, (LPCH)&lpBuf[0]
			);
			for (INT i = 0; i < aToc.Last_Track_Number; i++) {
				REVERSE_BYTES_SHORT(&TTxt.Track_Text_Pos[i], &lpBuf[0x8 + sizeof(USHORT) * i]);
				OutputVolDescLogA(
					"\t  Track_Text_Pos[%02d]: %d (%#x)\n"
					, i + 1, TTxt.Track_Text_Pos[i], TTxt.Track_Text_Pos[i]
				);
			}

			nLBA = nChToc[c] + aToc.Track_Text_Ptr + (TTxt.Track_Text_Pos[0] / DISC_RAW_READ_SIZE);
			REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
			cdb.TransferLength[3] = 1;

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_RAW_READ_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				FreeAndNull(pBuf);
				return FALSE;
			}
			OutputCDMain(fileMainInfo, lpBuf, nLBA, DISC_RAW_READ_SIZE);
			INT nOfs = 0;
			for (INT i = 0; i < aToc.Last_Track_Number; i++) {
				nOfs = TTxt.Track_Text_Pos[i] - TTxt.Track_Text_Pos[0];
				if (lpBuf[nOfs + 0x04] == 0x01) {
					OutputVolDescLogA("\t           Title[%02d]:", i + 1);
				}
				for (INT j = 0; lpBuf[nOfs + 0x05 + j] != 0; j++) {
					OutputVolDescLogA("%c", lpBuf[nOfs + 0x05 + j]);
				}
				OutputVolDescLogA("\n");
			}
		}
	}
	FreeAndNull(lpBuf);
	return TRUE;
}
