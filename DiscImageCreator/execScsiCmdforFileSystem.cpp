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
		, pCdb, nLBA + nOffset, lpBuf, bufDec, byTransferLen)) {
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
					if (*pExecType != dvd) {
						OutputVolDescLogA("Skip the reading of this sector\n");
						nSectorNum++;
						break;
					}
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
			, (INT)pDirRec[0].uiPosOfDir + nSectorOfs, lpBuf, bufDec, byTransferLen)) {
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
					, (INT)uiPathTblPos + nSectorOfs, lpBuf + pDevice->dwMaxTransferLength * n, bufDec, byTransferLen)) {
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
				, (INT)uiPathTblPos + nSectorOfs, lpBuf + dwBufOfs, bufDec, byTransferLen)) {
				throw FALSE;
			}
			for (BYTE i = 0; i < byRoop; i++) {
				OutputCDMain(fileMainInfo, lpBuf + dwBufOfs + DISC_RAW_READ_SIZE * i, (INT)uiPathTblPos + i, DISC_RAW_READ_SIZE);
			}
			if (!OutputFsPathTableRecord(pDisc, lpBuf, uiLogicalBlkCoef, uiPathTblPos, uiPathTblSize, pDirRec, nDirPosNum)) {
				throw FALSE;
			}
		}
		else {
			OutputMainInfoLogA(
				"uiPathTblSize: %u, byTransferLen: %d [L:%d]\n", uiPathTblSize, byRoop, (INT)__LINE__);
			if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
				, (INT)uiPathTblPos + nSectorOfs, lpBuf, bufDec, byTransferLen)) {
				throw FALSE;
			}
			for (BYTE i = 0; i < byRoop; i++) {
				OutputCDMain(fileMainInfo, lpBuf + DISC_RAW_READ_SIZE * i, (INT)uiPathTblPos + i, DISC_RAW_READ_SIZE);
			}
			if (!OutputFsPathTableRecord(pDisc, lpBuf, uiLogicalBlkCoef, uiPathTblPos, uiPathTblSize, pDirRec, nDirPosNum)) {
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
			, pCdb, nTmpLBA + nSectorOfs, lpBuf, bufDec, byTransferLen)) {
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
				if (pVolDesc->ISO_9660.uiPathTblPos == 0) {
					pVolDesc->ISO_9660.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
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
				if (pVolDesc->JOLIET.uiPathTblPos == 0) {
					pVolDesc->JOLIET.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
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
						, volDesc.ISO_9660.uiPathTblPos, 0, pDirRec, &nDirPosNum)) {
						throw FALSE;
					}
					if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
						, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiRootDataLen, 0, pDirRec, nDirPosNum)) {
						OutputVolDescLogA("Failed to read ISO9660\n");
						nDirPosNum = 0;
						if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
							, volDesc.JOLIET.uiLogicalBlkCoef, volDesc.JOLIET.uiPathTblSize
							, volDesc.JOLIET.uiPathTblPos, 0, pDirRec, &nDirPosNum)) {
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
						if (pDisc->PROTECT.byExist == microids) {
							OutputLogA(standardOut | fileDisc, ", %d to %d\n"
								, pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd
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
				, volDesc.ISO_9660.uiPathTblPos, nSectorOfs, pDirRec, &nDirPosNum)) {
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
			, volDesc.ISO_9660.uiPathTblPos, 0, pDirRec, &nDirPosNum)) {
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
			OutputLogA(standardOut | fileDisc, "Detected a protected file [%s]. LBA %d to %d\n"
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
	INT nNest,
	LPCSTR* szTab,
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
		, szTab[nNest], ofsLeft, ofsLeft
		, szTab[nNest], ofsRight, ofsRight
		, szTab[nNest], startSector, startSector
		, szTab[nNest], fileSize, fileSize
		, szTab[nNest]
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
		, szTab[nNest], lenOfFile
		, szTab[nNest]
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
		if (nNest < 6) {
			nNest++;
		}
		if (!ReadXBOXDirectoryRecord(pExtArg, pDevice, pCdb
			, startSector + uiStartLBA, fileSize, uiStartLBA, nNest)) {
			return FALSE;
		}
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
	INT nNest
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
	LPCSTR szTab[] = { "\t", "\t\t", "\t\t\t", "\t\t\t\t", "\t\t\t\t\t", "\t\t\t\t\t\t", "\t\t\t\t\t\t\t" };
	OutputVolDescLogA("%s", szTab[nNest]);
	OutputVolDescLogA(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F(DIRECTORY ENTRY)
		, (INT)uiDirPos, (INT)uiDirPos
	);
	BOOL bEnd = FALSE;
	UINT uiSize = 0;
	UINT uiCoeff = 1;
	for (UINT uiOfs = 0; uiSize < uiDirTblSize;) {
		if (!OutputXDVDFsDirectoryRecord(pExtArg, pDevice, pCdb
			, lpBuf + uiSize, &uiOfs, uiStartLBA, nNest, szTab, &bEnd)) {
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
		"\tSector of root directory table: %d(0x%x)\n"
		"\t  Size of root directory table: %d(0x%x)\n"
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
	if (!ReadXBOXDirectoryRecord(
		pExtArg, pDevice, &cdb, uiDirPos + (UINT)dwStartLBA, uiDirTblSize, (UINT)dwStartLBA, 0)) {
		FreeAndNull(pBuf);
		return FALSE;
	};
	FreeAndNull(pBuf);
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
