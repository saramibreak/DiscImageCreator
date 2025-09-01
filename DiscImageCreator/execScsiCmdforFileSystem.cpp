/**
 * Copyright 2011-2025 sarami
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
#include "execScsiCmdforDVD.h"
#include "execScsiCmdforFileSystem.h"
#include "execIoctl.h"
#include "get.h"
#include "output.h"
#include "outputFileSystem.h"
#include "outputScsiCmdLogforCD.h"
#include "outputScsiCmdLogforDVD.h"
#include "set.h"
#include "_external/abgx360.h"
#include "_external/mbedtls/aes.h"
#include "_external/NonStandardFunction.h"

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
		DISC_MAIN_DATA_SIZE, &lpBuf, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
	try {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)pCdb, nLBA, lpBuf,
			DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
			throw FALSE;
		}
		OutputMainChannel(fileMainInfo, lpBuf, _T("3DO Directory"), nLBA, DISC_MAIN_DATA_SIZE);

		UINT ofs = THREEDO_DIR_HEADER_SIZE;
		UINT dirSize =
			MAKEUINT(MAKEWORD(lpBuf[15], lpBuf[14]), MAKEWORD(lpBuf[13], lpBuf[12]));
		OutputFs3doDirectoryRecord(lpBuf, nLBA, pPath, dirSize);

		// next dir
		CHAR szNewPath[_MAX_PATH] = {};
		CHAR fname[32 + 1] = {};
		while (ofs < dirSize) {
			LPBYTE lpDirEnt = lpBuf + ofs;
			UINT lFlags = MAKEUINT(
				MAKEWORD(lpDirEnt[3], lpDirEnt[2]), MAKEWORD(lpDirEnt[1], lpDirEnt[0]));
			strncpy(fname, (LPCCH)&lpDirEnt[32], sizeof(fname) - 1);
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
	UINT uiPaddingLen,
	LPUINT nOfs
) {
	if (*nSectorNum < byTransferLen) {
		*nOfs += uiPaddingLen;
		(*nSectorNum)++;
		return;
	}
}

BOOL IsValidPositionAndDataLength(
	LPBYTE lpDirRec
) {
	// check if stored LSB data is the same as MSB data
	return lpDirRec[2] == lpDirRec[9] && lpDirRec[3] == lpDirRec[8] &&
		lpDirRec[4] == lpDirRec[7] && lpDirRec[5] == lpDirRec[6] && // offset
		lpDirRec[10] == lpDirRec[17] && lpDirRec[11] == lpDirRec[16] &&
		lpDirRec[12] == lpDirRec[15] && lpDirRec[13] == lpDirRec[14];   // data length
}

BOOL IsValidMonthDay(
	LPBYTE lpDirRec
) {
	return 1 <= lpDirRec[19] && lpDirRec[19] <= 12 && // month
		1 <= lpDirRec[20] && lpDirRec[20] <= 31; // day
}

VOID AdjustOfs(
	LPBYTE lpBuf,
	PUINT puiOfs
) {
	if (!IsValidPositionAndDataLength(lpBuf + *puiOfs) && !IsValidMonthDay(lpBuf + *puiOfs)) {
		OutputVolDescLog("Detected corrupt directory record. Skipped it.\n");
		for (UINT i = 1; i < 256; i++) {
			if (85 <= *(lpBuf + 18 + i) && IsValidMonthDay(lpBuf + i)) {
				// [PSX] Tokimeki Memorial - forever with you (Japan) (Rev 4) or (Rev 2) or (PlayStation the Best)
				// [PSX] Aitakute... - Your Smiles in My Heart (Japan)
				// [PSX] Aitakute... - Your Smiles in My Heart - Oroshitate no Diary - Introduction Disc (Japan)
				// 8 bytes are missing in the 1st directory record
				// ========== LBA[014515, 0x038b3]: Main Channel ==========
				//        +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F
				// 0000 : 38 B3 00 08 00 00 00 00  08 00 5F 08 12 0A 32 1C   8........._...2.
				// 0010 : 24 02 00 00 01 00 00 01  01 00 3D 02 3D 02 88 00   $.........=.=...
				// 0020 : 58 41 01 00 00 00 00 00  30 00 16 00 00 00 00 00   XA......0.......
				// 0030 : 00 16 00 08 00 00 00 00  08 00 60 03 1F 00 00 00   ..........`.....
				// 0040 : 24 02 00 00 01 00 00 01  01 01 00 00 00 00 88 00   $...............
				// 0050 : 58 41 00 00 00 00 00 00
				*puiOfs = i;
				break;
			}
		}
	}
}

BOOL IsValidExtent(
	LPBYTE lpDirRec,
	INT nDirOfs,
	UINT uiPaddingSize
) {
	if (lpDirRec[nDirOfs + uiPaddingSize + 2] == lpDirRec[nDirOfs + uiPaddingSize + 9] &&
		lpDirRec[nDirOfs + uiPaddingSize + 3] == lpDirRec[nDirOfs + uiPaddingSize + 8] &&
		lpDirRec[nDirOfs + uiPaddingSize + 4] == lpDirRec[nDirOfs + uiPaddingSize + 7] &&
		lpDirRec[nDirOfs + uiPaddingSize + 5] == lpDirRec[nDirOfs + uiPaddingSize + 6]) {
		if (!(lpDirRec[nDirOfs + uiPaddingSize + 2] == 0 && lpDirRec[nDirOfs + uiPaddingSize + 3] == 0 &&
			lpDirRec[nDirOfs + uiPaddingSize + 4] == 0 && lpDirRec[nDirOfs + uiPaddingSize + 5] == 0)) {
			return TRUE;
		}
	}
	return FALSE;
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
	UINT uiDirPosNum,
	UINT uiLogicalBlkCoef,
	INT nOffset,
	PPATH_TABLE_RECORD pPathTblRec,
	UINT uiPathTblIdx
) {
#ifdef LATIN1_TEST
	memcpy(lpBuf, dirTblWithLatin1, sizeof(dirTblWithLatin1));
#else
	if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc
		, pCdb, nLBA + nOffset, lpBuf, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
		return FALSE;
	}
#endif
	BYTE byRoop = byTransferLen;
	if (*pExecType == gd) {
		byRoop = (BYTE)(byRoop - 1);
	}
	for (BYTE i = 0; i < byRoop; i++) {
		OutputMainChannel(fileMainInfo, lpBuf + DISC_MAIN_DATA_SIZE * i, NULL, nLBA + i, DISC_MAIN_DATA_SIZE);
	}

	UINT uiOfs = 0;
	AdjustOfs(lpBuf, &uiOfs);
	for (INT nSectorNum = 0; nSectorNum < byRoop;) {
		if (*(lpBuf + uiOfs) == 0) {
			break;
		}
		OutputVolDescWithLBALog1("Directory Record", nLBA + nSectorNum);
		for (;;) {
			CHAR szCurDirName[MAX_FNAME_FOR_VOLUME] = {};
			LPBYTE lpDirRec = lpBuf + uiOfs;
			if (lpDirRec[0] >= MIN_LEN_DR) {
#if 0
				// Due to this code, other disc can't dump.
				// http://forum.redump.org/post/81822/#p81822
				if (lpDirRec[0] == MIN_LEN_DR && uiOfs > 0 && uiOfs % DISC_MAIN_DATA_SIZE == 0) {
					// http://forum.redump.org/post/56490/#p56490
					// [PC] SimCity 3000 (USA)
					// Data Length should be 2048 because LBA 200205 is joliet
					// ========== LBA[200204, 0x30e0c]: Directory Record ==========
					// 		      Length of Directory Record: 34
					// 		Extended Attribute Record Length: 0
					// 		              Location of Extent: 200204
					// 		                     Data Length: 4096
					OutputVolDescLog(
						"LBA %d, ofs %u: Data length is incorrect. Skip this sector\n", nLBA, uiOfs);
					OutputMainChannel(fileVolDesc, lpBuf, NULL, nLBA, DISC_MAIN_DATA_SIZE);
					nSectorNum++;
					break;
				}
#endif
				// a DVD "DTM Race Driver 3"
				// Path table is irregular. (L type and M type is perhaps the reverse.)
				// ========== LBA[000019, 0x00013]: Main Channel ==========
				//	     :
				//	     :                          2C 00 AB 3D 0C 00 00 0C   A1.HDR; 1, .. = ....
				//	0100 : 3D AB 00 60 8B 52 52 8B  60 00 6A 01 19 10 00 0A = ..`.RR.`.j.....
				//	0110 : 00 00 00 00 01 00 00 01  0B 44 41 54 41 32 2E 43   .........DATA2.C
				//	0120 : 41 42 3B
				UINT uiMaxByte = UINT(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE);
				if (pDisc->SCSI.nAllLength >= 0x200000) {
					uiMaxByte = 0xffffffff;
				}
				UINT uiExtentPos = GetSizeOrUintForVolDesc(lpDirRec + 2, uiMaxByte) / uiLogicalBlkCoef;
				UINT uiDataLen = GetSizeOrUintForVolDesc(lpDirRec + 10, uiMaxByte);
				if (uiDataLen >= uiMaxByte) {
					// Apple Mac DL DVD
					// ========== LBA[000070, 0x00046]: Main Channel ==========
					//      :
					// 00C0 :                                      8A 00 3E 02   .;8.n....3....>.
					// 00D0 : 00 00 00 00 02 3E FF FF  FF FF FF FF FF FF 6E 01   .....>........n.
					// 00E0 : 06 04 3B 38 00 00 00 00  01 00 00 01 0D 41 52 43   ..;8.........ARC
					// 00F0 : 48 49 56 45 50 41 58 2E  47 5A 41 41 0E 02 00 00   HIVEPAX.GZAA....
					// 0100 : 00 00 00 00 00 00 00 00  50 58 2C 01 24 81 00 00   ........PX,.$...
					// 0110 : 00 00 81 24 01 00 00 00  00 00 00 01 F6 01 00 00   ...$............
					// 0120 : 00 00 01 F6 14 00 00 00  00 00 00 14 1B 00 00 00   ................
					// 0130 : 00 00 00 1B 54 46 21 01  0F 6E 01 06 04 3B 38 00   ....TF!..n...;8.
					// 0140 : 6E 01 06 04 3B 38 00 6E  01 06 05 04 0D 00 6E 01   n...;8.n......n.
					// 0150 : 06 13 1B 31 00 00

					// [DC] Pier Solar And The Great Architects (Unlicensed)
					// ========== LBA[012317, 0x0301d]: Main Channel ==========
					//      :
					// 0040 :             2A 00 1E FF  FF FF 00 FF FF FF FF FF   ....*...........
					// 0050 : FF FF FF FF 30 00 73 03  1F 16 23 29 08 00 00 00   ....0.s...#)....
					// 0060 : 01 00 00 01 08 31 38 2E  44 41 54 3B 31 00
					OutputVolDescLog("LBA %d, ofs %u: Data length is incorrect\n", nLBA, uiOfs);
				}
				OutputFsDirectoryRecord(
					pExtArg, pDisc, lpDirRec, uiExtentPos, uiDataLen, szCurDirName, pPathTblRec, uiPathTblIdx);
				if (!strncmp(szCurDirName, "DVDAUDIO.MKB", 12)) {
					pDisc->DVD.protect = cppm;
				}
				UINT uiPaddingSize = 0;
#ifdef TEST_INCORRECT_DIR_RECORD
				// The Shooting Love: XII Stag & Trizeal [DVD] (Japan)
				// 0170 :                          3C 00 91 A4 3D 00 00 3D   1.......<...=..=
				// 0180 : A4 91 24 72 01 00 00 01  72 24 6A 03 10 01 31 2C   ..$r....r$j...1,
				// 0190 : 24 00 00 00 01 00 00 01  03 00 3B 31 00 00 00 00   $.........;1....
				// 01A0 : 00 00 3C 00 C0 A4 3D 00  00 3D A4 C0 A4 16 08 00   ..<...=..=......
				// 01B0 : 00 08 16 A4
				lpDirRec[0] = 0x3c; lpDirRec[1] = 0x00; lpDirRec[2] = 0x91; lpDirRec[3] = 0xa4;
				lpDirRec[4] = 0x3d; lpDirRec[5] = 0x00; lpDirRec[6] = 0x00; lpDirRec[7] = 0x3d;
				lpDirRec[8] = 0xa4; lpDirRec[9] = 0x91; lpDirRec[10] = 0x24; lpDirRec[11] = 0x72;
				lpDirRec[12] = 0x01; lpDirRec[13] = 0x00; lpDirRec[14] = 0x00; lpDirRec[15] = 0x01;
				lpDirRec[16] = 0x72; lpDirRec[17] = 0x24; lpDirRec[18] = 0x6a; lpDirRec[19] = 0x03;
				lpDirRec[20] = 0x10; lpDirRec[21] = 0x01; lpDirRec[22] = 0x31; lpDirRec[23] = 0x2c;
				lpDirRec[24] = 0x24; lpDirRec[25] = 0x00; lpDirRec[26] = 0x00; lpDirRec[27] = 0x00;
				lpDirRec[28] = 0x01; lpDirRec[29] = 0x00; lpDirRec[30] = 0x00; lpDirRec[31] = 0x01;
				lpDirRec[32] = 0x03; lpDirRec[33] = 0x00; lpDirRec[34] = 0x3b; lpDirRec[35] = 0x31;
				lpDirRec[36] = 0x00; lpDirRec[37] = 0x00; lpDirRec[38] = 0x00; lpDirRec[39] = 0x00;
				lpDirRec[40] = 0x00; lpDirRec[41] = 0x00; lpDirRec[42] = 0x3c; lpDirRec[43] = 0x00;
				lpDirRec[44] = 0xc0; lpDirRec[45] = 0xa4; lpDirRec[46] = 0x3d; lpDirRec[47] = 0x00;
				lpDirRec[48] = 0x00; lpDirRec[49] = 0x3d; lpDirRec[50] = 0xa4; lpDirRec[51] = 0xc0;
				lpDirRec[52] = 0xa4; lpDirRec[53] = 0x16; lpDirRec[54] = 0x08; lpDirRec[55] = 0x00;
				lpDirRec[56] = 0x00; lpDirRec[57] = 0x08; lpDirRec[58] = 0x16; lpDirRec[59] = 0xa4;
#endif
				INT nDirOfs = 33 + lpDirRec[32];
				if (lpDirRec[32] % 2 == 0) {
					nDirOfs++;
				}
				if (nDirOfs < lpDirRec[0]) {
					OutputVolDescLog("LBA %d, Check if the directory record length (%u) is really correct -> ", nLBA, lpDirRec[0]);
					// Check incorrect directory record length
					LPBYTE lpNextDirRec = lpBuf + uiOfs + lpDirRec[0];
					BOOL bAllZero = TRUE;
					UINT k = uiOfs / DISC_MAIN_DATA_SIZE + 1;

					if (DISC_MAIN_DATA_SIZE * k - (uiOfs + lpDirRec[0]) >= MIN_LEN_DR &&
						!(lpDirRec[32] == 1 && szCurDirName[0] == 0) &&
						!(lpDirRec[32] == 1 && szCurDirName[0] == 1)) {
						for (INT a = 0; a < 32; a++) {
							if (lpNextDirRec[a] != 0) {
								bAllZero = FALSE;
								break;
							}
						}
					}
					BYTE semicolon = lpDirRec[32 + lpDirRec[32] - 1];
					BYTE one = lpDirRec[32 + lpDirRec[32]];

					if (IsValidExtent(lpNextDirRec, 0, 0) || bAllZero || (semicolon == ';' && one == '1')) {
						OutputVolDescLog("correct\n");
						uiOfs += lpDirRec[0];
					}
					else {
						for (; nDirOfs + uiPaddingSize < lpDirRec[0];) {
							if (IsValidExtent(lpDirRec, nDirOfs, uiPaddingSize)) {
								OutputVolDescLog("incorrect. Fixed it to %u\n", nDirOfs + uiPaddingSize);
								break;
							}
							uiPaddingSize++;
						}
						uiOfs += nDirOfs + uiPaddingSize;
						if (nDirOfs + uiPaddingSize == lpDirRec[0]) {
							OutputVolDescLog("correct\n");
						}
					}
				}
				else {
					uiOfs += lpDirRec[0];
				}
				// not upper and current directory
				if (!(lpDirRec[32] == 1 && szCurDirName[0] == 0) &&
					!(lpDirRec[32] == 1 && szCurDirName[0] == 1)) {
					if ((lpDirRec[25] & 0x02 || (pDisc->SCSI.byFormat == DISK_TYPE_CDI && lpDirRec[25] == 0))) {
						for (UINT i = 1; i < uiDirPosNum; i++) {
							if (uiExtentPos == pPathTblRec[i].uiPosOfDir) {
								BOOL bOk = TRUE;
								if (_strnicmp(szCurDirName, pPathTblRec[i].szDirName, MAX_FNAME_FOR_VOLUME)) {
									// Check ISO/IEC 8859-1 (aka Latin-1)
									for (INT j = 0; j < MAX_FNAME_FOR_VOLUME; j++) {
										UCHAR src = (UCHAR)szCurDirName[j];
										UCHAR dst = (UCHAR)pPathTblRec[i].szDirName[j];
										if (dst == '\0') {
											break;
										}
										else if (src != dst) {
											if (isalpha(dst)) {
												UCHAR dst2 = islower(dst) ? (UCHAR)(dst - 0x20) : (UCHAR)(dst + 0x20);
												if (src != dst2) {
													bOk = FALSE;
													break;
												}
											}
											else if (0xc0 <= dst) {
												UCHAR dst2 = src < dst ? (UCHAR)(dst - 0x20) : (UCHAR)(dst + 0x20);
												if (src != dst2) {
													bOk = FALSE;
													break;
												}
											}
										}
									}
								}
								if (bOk) {
									pPathTblRec[i].uiDirSize = PadSizeForVolDesc(uiDataLen);
									break;
								}
							}
						}
					}
				}
				else {
					OutputVolDescLog("\n");
				}

				if (uiOfs == (UINT)(DISC_MAIN_DATA_SIZE * (nSectorNum + 1))) {
					nSectorNum++;
					break;
				}
			}
			else {
				UINT uiPaddingLen = DISC_MAIN_DATA_SIZE * (nSectorNum + 1) - uiOfs;
				if (uiPaddingLen > MIN_LEN_DR) {
					BYTE byNextLenDR = lpDirRec[MIN_LEN_DR];
					BOOL bValidPos = IsValidPositionAndDataLength(lpDirRec + MIN_LEN_DR);
					BOOL bValidDay = IsValidMonthDay(lpDirRec + MIN_LEN_DR);
					if (byNextLenDR >= MIN_LEN_DR && bValidPos && bValidDay) {
						// Amiga Tools 4
						// The second of Direcory Record (0x22 - 0x43) is corrupt
						// ========== LBA[040915, 0x09fd3]: Main Channel ==========
						//        +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F
						// 0000 : 22 00 D3 9F 00 00 00 00  9F D3 00 08 00 00 00 00   "...............
						// 0010 : 08 00 60 02 1D 17 18 2C  00 02 00 00 01 00 00 01   ..`....,........
						// 0020 : 01 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
						// 0030 : 00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
						// 0040 : 00 00 01 01 2E 00 09 A0  00 00 00 00 A0 09 D8 01   ................
						// 0050 : 00 00 00 00 01 D8 60 02  1D 01 1D 29 00 02 00 00   ......`....)....
						// 0060 : 01 00 00 01 0D 41 52 65  78 78 2D 53 63 72 69 70   .....ARexx-Scrip
						// 0070 : 74 73                                              ts
						OutputVolDescLog(
							"LBA %d: Direcory Record is corrupt. Skip reading from %u to %u byte\n"
							, nLBA, uiOfs, uiOfs + MIN_LEN_DR - 1);
						uiOfs += MIN_LEN_DR;
						break;
					}
					else {
						ManageEndOfDirectoryRecord(&nSectorNum, byRoop, uiPaddingLen, &uiOfs);
						break;
					}
				}
				else {
					ManageEndOfDirectoryRecord(&nSectorNum, byRoop, uiPaddingLen, &uiOfs);
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
	PPATH_TABLE_RECORD pPathTblRec,
	UINT uiDirPosNum
) {
	LPBYTE bufDec = NULL;
	DWORD dwMaxTransferLen = pDevice->dwMaxTransferLength;
	if (*pExecType == gd) {
		bufDec = (LPBYTE)calloc(pDevice->dwMaxTransferLength, sizeof(BYTE));
		if (!bufDec) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		dwMaxTransferLen = pDevice->dwMaxTransferLength * DISC_MAIN_DATA_SIZE / CD_RAW_SECTOR_SIZE;
	}
	BYTE byTransferLen = 1;
	BYTE byRoop = byTransferLen;
	BOOL bRet = TRUE;
	try {
		// for CD-I
		if (uiRootDataLen == 0) {
			if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
				, (INT)pPathTblRec[0].uiPosOfDir + nSectorOfs, lpBuf, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
				throw FALSE;
			}
			uiRootDataLen =
				PadSizeForVolDesc(GetSizeOrUintForVolDesc(lpBuf + 10, (UINT)(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE)));
		}
		pPathTblRec[0].uiDirSize = uiRootDataLen;

		for (UINT uiPathTblIdx = 0; uiPathTblIdx < uiDirPosNum; uiPathTblIdx++) {
			INT nLBA = (INT)pPathTblRec[uiPathTblIdx].uiPosOfDir;
			if (pPathTblRec[uiPathTblIdx].uiDirSize > dwMaxTransferLen) {
				// [FMT] Psychic Detective Series Vol. 4 - Orgel (Japan) (v1.0)
				// [FMT] Psychic Detective Series Vol. 5 - Nightmare (Japan)
				// [IBM - PC compatible] Maria 2 - Jutai Kokuchi no Nazo (Japan) (Disc 1)
				// [IBM - PC compatible] PC Game Best Series Vol. 42 - J.B. Harold Series - Kiss of Murder - Satsui no Kuchizuke (Japan)
				// [SS] Madou Monogatari (Japan)
				// and more
				SetCommandForTransferLength(pExecType, pDevice, pCdb, dwMaxTransferLen, &byTransferLen, &byRoop);

				DWORD additionalTransferLen = pPathTblRec[uiPathTblIdx].uiDirSize / dwMaxTransferLen;
				OutputMainInfoLog("nLBA %d, uiDirSize: %lu*%lu, byTransferLen: %d*%lu [L:%d]\n"
					, nLBA, dwMaxTransferLen, additionalTransferLen, byRoop, additionalTransferLen, __LINE__);

				for (DWORD n = 0; n < additionalTransferLen; n++) {
					if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
						, lpBuf, bufDec, byTransferLen, uiDirPosNum, uiLogicalBlkCoef, nSectorOfs, pPathTblRec, uiPathTblIdx)) {
						continue;
					}
					nLBA += byRoop;
				}
				DWORD dwLastTblSize = pPathTblRec[uiPathTblIdx].uiDirSize % dwMaxTransferLen;
				if (*pExecType == gd) {
					if (byRoop != 0 && additionalTransferLen != 0) {
						dwLastTblSize = pPathTblRec[uiPathTblIdx].uiDirSize % (DISC_MAIN_DATA_SIZE * byRoop * additionalTransferLen);
					}
				}
				if (dwLastTblSize != 0) {
					SetCommandForTransferLength(pExecType, pDevice, pCdb, dwLastTblSize, &byTransferLen, &byRoop);
					OutputMainInfoLog("nLBA %d, uiDirSize: %lu, byTransferLen: %d [L:%d]\n", nLBA, dwLastTblSize, byRoop, __LINE__);

					if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
						, lpBuf, bufDec, byTransferLen, uiDirPosNum, uiLogicalBlkCoef, nSectorOfs, pPathTblRec, uiPathTblIdx)) {
						continue;
					}
				}
			}
			else {
				if (pPathTblRec[uiPathTblIdx].uiDirSize == 0 || byTransferLen == 0) {
					OutputMainErrorLog("nLBA %d, Directory Record is invalid\n", nLBA);
					throw FALSE;
				}
				SetCommandForTransferLength(pExecType, pDevice, pCdb, pPathTblRec[uiPathTblIdx].uiDirSize, &byTransferLen, &byRoop);
				OutputMainInfoLog("nLBA %d, uiDirSize: %u, byTransferLen: %d [L:%d]\n"
					, nLBA, pPathTblRec[uiPathTblIdx].uiDirSize, byRoop, __LINE__);

				if (!ReadDirectoryRecordDetail(pExecType, pExtArg, pDevice, pDisc, pCdb, nLBA
					, lpBuf, bufDec, byTransferLen, uiDirPosNum, uiLogicalBlkCoef, nSectorOfs, pPathTblRec, uiPathTblIdx)) {
					continue;
				}
			}
			OutputString("\rReading DirectoryRecord %4u/%4u", uiPathTblIdx + 1, uiDirPosNum);
		}
		OutputString("\n");
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(bufDec);
	return bRet;
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
	PPATH_TABLE_RECORD pPathTblRec,
	LPUINT uiDirPosNum
) {
#ifdef LATIN1_TEST
	uiPathTblPos = 20;
	uiPathTblSize = 46612;
#endif
	BYTE byTransferLen = 1;
	BYTE byRoop = byTransferLen;
	DWORD dwBufSize = 0;
	if (*pExecType == gd) {
		byTransferLen = 2;
		dwBufSize = (CD_RAW_SECTOR_SIZE - (uiPathTblSize % CD_RAW_SECTOR_SIZE) + uiPathTblSize) * byTransferLen * 2;
	}
	else {
		dwBufSize = DISC_MAIN_DATA_SIZE - (uiPathTblSize % DISC_MAIN_DATA_SIZE) + uiPathTblSize;
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
				throw FALSE;
			}
		}
		try {
			if (uiPathTblSize > pDevice->dwMaxTransferLength) {
				DWORD uiAdditionalTransferLen = uiPathTblSize / pDevice->dwMaxTransferLength;
				SetCommandForTransferLength(pExecType, pDevice, pCdb, pDevice->dwMaxTransferLength, &byTransferLen, &byRoop);

				for (DWORD n = 0; n < uiAdditionalTransferLen; n++) {
					OutputMainInfoLog("uiPathTblPos: %u, uiPathTblSize: %lu, byTransferLen: %d [L:%d]\n"
						, uiPathTblPos + nSectorOfs, pDevice->dwMaxTransferLength, byRoop, __LINE__);
					if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
						, (INT)uiPathTblPos + nSectorOfs, lpBuf + pDevice->dwMaxTransferLength * n, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					for (BYTE i = 0; i < byRoop; i++) {
						OutputMainChannel(fileMainInfo, lpBuf + DISC_MAIN_DATA_SIZE * i, NULL, (INT)uiPathTblPos + i, DISC_MAIN_DATA_SIZE);
					}
					uiPathTblPos += byTransferLen;
				}
				DWORD dwLastPathTblSize = uiPathTblSize % pDevice->dwMaxTransferLength;
				SetCommandForTransferLength(pExecType, pDevice, pCdb, dwLastPathTblSize, &byTransferLen, &byRoop);
				DWORD dwBufOfs = pDevice->dwMaxTransferLength * uiAdditionalTransferLen;

				OutputMainInfoLog("uiPathTblPos: %u, uiPathTblSize: %lu, byTransferLen: %d [L:%d]\n"
					, uiPathTblPos + nSectorOfs, dwLastPathTblSize, byRoop, __LINE__);
				if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
					, (INT)uiPathTblPos + nSectorOfs, lpBuf + dwBufOfs, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
				for (BYTE i = 0; i < byRoop; i++) {
					OutputMainChannel(fileMainInfo, lpBuf + dwBufOfs + DISC_MAIN_DATA_SIZE * i, NULL, (INT)uiPathTblPos + i, DISC_MAIN_DATA_SIZE);
				}
				if (!OutputFsPathTableRecord(lpBuf, uiLogicalBlkCoef, uiPathTblPos, uiPathTblSize, bPathType, pPathTblRec, uiDirPosNum)) {
					throw FALSE;
				}
			}
			else {
				OutputMainInfoLog("uiPathTblPos: %u, uiPathTblSize: %u, byTransferLen: %d [L:%d]\n"
					, uiPathTblPos + nSectorOfs, uiPathTblSize, byRoop, __LINE__);
#ifdef LATIN1_TEST
				memcpy(lpBuf, pathTblWithLatin1, sizeof(pathTblWithLatin1));
#else
				if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc, pCdb
					, (INT)uiPathTblPos + nSectorOfs, lpBuf, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
					throw FALSE;
				}
#endif
				for (BYTE i = 0; i < byRoop; i++) {
					OutputMainChannel(fileMainInfo, lpBuf + DISC_MAIN_DATA_SIZE * i, NULL, (INT)uiPathTblPos + i, DISC_MAIN_DATA_SIZE);
				}
				if (!OutputFsPathTableRecord(lpBuf, uiLogicalBlkCoef, uiPathTblPos, uiPathTblSize, bPathType, pPathTblRec, uiDirPosNum)) {
					throw FALSE;
				}
			}
			OutputVolDescLog("Directory Num: %u\n", *uiDirPosNum);
		}
		catch (BOOL ret) {
			bRet = ret;
		}
		FreeAndNull(bufDec);
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
	if (pDisc->SCSI.lp1stLBAListOnToc) {
		// Eraser Turnabout (Chinese)
		//	========== TOC ==========
		//	Pregap Track   , LBA        0 -        0, Length        1
		//	  Data Track  1, LBA        1 -   317021, Length   317021
		//	                                          Total    317022
		if (pDisc->SCSI.lp1stLBAListOnToc[byIdx] != 1 && IsDataDisc(pDisc)) {
			nPVD += pDisc->SCSI.lp1stLBAListOnToc[byIdx];
		}
	}
	INT nTmpLBA = nPVD;
	BYTE bufDec[CD_RAW_SECTOR_SIZE * 2] = {};
	for (;;) {
		if (!ExecReadDisc(pExecType, pExtArg, pDevice, pDisc
			, pCdb, nTmpLBA + nSectorOfs, lpBuf, bufDec, byTransferLen, _T(__FUNCTION__), __LINE__)) {
			break;
		}
		if (!strncmp((LPCCH)&lpBuf[1], "CD001", 5) ||
			(pDisc->SCSI.byFormat == DISK_TYPE_CDI && !strncmp((LPCCH)&lpBuf[1], "CD-I ", 5))) {
			if (nTmpLBA == nPVD) {
				WORD wLogicalBlkSize = GetSizeOrWordForVolDesc(lpBuf + 128);
				pVolDesc->ISO_9660.uiLogicalBlkCoef = (BYTE)(DISC_MAIN_DATA_SIZE / wLogicalBlkSize);
				pVolDesc->ISO_9660.uiPathTblSize =
					GetSizeOrUintForVolDesc(lpBuf + 132, (UINT)(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE));
				pVolDesc->ISO_9660.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143])) / pVolDesc->ISO_9660.uiLogicalBlkCoef;
				pVolDesc->bPathType = lType;
				if (pVolDesc->ISO_9660.uiPathTblPos == 0) {
					pVolDesc->ISO_9660.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
					pVolDesc->bPathType = mType;
				}
				pVolDesc->ISO_9660.uiRootDataLen =
					GetSizeOrUintForVolDesc(lpBuf + 166, (UINT)(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE));
				if (pVolDesc->ISO_9660.uiRootDataLen > 0) {
					pVolDesc->ISO_9660.uiRootDataLen = PadSizeForVolDesc(pVolDesc->ISO_9660.uiRootDataLen);
				}
				*lpReadVD = TRUE;
			}
			else if (lpBuf[0] == 2) {
				WORD wLogicalBlkSize = GetSizeOrWordForVolDesc(lpBuf + 128);
				pVolDesc->JOLIET.uiLogicalBlkCoef = (BYTE)(DISC_MAIN_DATA_SIZE / wLogicalBlkSize);
				pVolDesc->JOLIET.uiPathTblSize =
					GetSizeOrUintForVolDesc(lpBuf + 132, (UINT)(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE));
				pVolDesc->JOLIET.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[140], lpBuf[141]),
					MAKEWORD(lpBuf[142], lpBuf[143])) / pVolDesc->JOLIET.uiLogicalBlkCoef;
				pVolDesc->bPathType = lType;
				if (pVolDesc->JOLIET.uiPathTblPos == 0) {
					pVolDesc->JOLIET.uiPathTblPos = MAKEUINT(MAKEWORD(lpBuf[151], lpBuf[150]),
						MAKEWORD(lpBuf[149], lpBuf[148]));
					pVolDesc->bPathType = mType;
				}
				pVolDesc->JOLIET.uiRootDataLen =
					GetSizeOrUintForVolDesc(lpBuf + 166, (UINT)(pDisc->SCSI.nAllLength * DISC_MAIN_DATA_SIZE));
				if (pVolDesc->JOLIET.uiRootDataLen > 0) {
					pVolDesc->JOLIET.uiRootDataLen = PadSizeForVolDesc(pVolDesc->JOLIET.uiRootDataLen);
				}
				*lpReadVD = TRUE;
			}
			OutputMainChannel(fileMainInfo, lpBuf, _T("Check Volume Descriptor"), nTmpLBA, DISC_MAIN_DATA_SIZE);
			OutputFsVolumeDescriptor(pExtArg, pDisc, lpBuf, pVolDesc, nTmpLBA++);
		}
		else {
			break;
		}
	}
	return TRUE;
}

BOOL ReadCDForHfsCatalogFiles(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	CDB::_READ12 cdb,
	LPBYTE lpBuf,
	UINT uiCatalogFileSize
) {
	INT n1stCatalogSector = (INT)(6 + uiCatalogFileSize / DISC_MAIN_DATA_SIZE);
	INT nPad = (INT)(uiCatalogFileSize % DISC_MAIN_DATA_SIZE);

	for (INT k = 0; k < n1stCatalogSector - 7; k++) {
		if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, n1stCatalogSector + k, lpBuf,
			DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
			return FALSE;
		}
		OutputMainChannel(fileMainInfo, lpBuf, NULL, n1stCatalogSector + k, DISC_MAIN_DATA_SIZE);

		INT nRoop = lpBuf[11];
		INT nMaxBlk = 4;
		if (k == 0) {
			nRoop = lpBuf[11 + nPad];
			if (nPad == 512) {
				nMaxBlk = 3;
			}
			else if (nPad == 1024) {
				nMaxBlk = 2;
			}
			else if (nPad == 1536) {
				nMaxBlk = 1;
			}
		}
		INT nOfs = 14;
		INT nOfs2 = 0;
		for (INT blk = 1; blk <= nMaxBlk; blk++) {
			for (INT m = 0; m < nRoop; m++) {
				if (k == 0 && nOfs2 == 0 && nPad != 0) {
					nOfs2 = nPad;
				}
				OutputFsCatalogFiles(lpBuf + nOfs + nOfs2, n1stCatalogSector + k, &nOfs, nOfs2);
			}
			nOfs = 512 * blk;
			if (nOfs + nOfs2 + 11 < DISC_MAIN_DATA_SIZE) {
				nRoop = lpBuf[nOfs + nOfs2 + 11];
			}
			nOfs += 14;
		}
		OutputString("\rReading CatalogFiles %u/%u", k + 1, n1stCatalogSector - 7);
	}
	OutputString("\n");
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
		if (pDisc->SCSI.toc.TrackData[i].Control & AUDIO_DATA_TRACK) {
			// for Label Gate CD, XCP
			if (i > 1 && pDisc->SCSI.lpLastLBAListOnToc[i] - pDisc->SCSI.lp1stLBAListOnToc[i] + 1 <= 750) {
				return TRUE;
			}
			// for Anno 1602 - Im Namen des Konigs
			else if (i == 1 && pDisc->SCSI.lpLastLBAListOnToc[i] - pDisc->SCSI.lp1stLBAListOnToc[i] + 1 <= 200) {
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
			PPATH_TABLE_RECORD pPathTblRec = NULL;
			try {
				// general data track disc
				VOLUME_DESCRIPTOR volDesc = {};
				if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice
					, pDisc, i, (LPBYTE)&cdb, lpBuf, 16, 0, &bVD, &volDesc, 1)) {
					throw FALSE;
				}
				if (bVD) {
					pPathTblRec = (PPATH_TABLE_RECORD)calloc(PATH_TABLE_RECORD_SIZE, sizeof(PATH_TABLE_RECORD));
					if (!pPathTblRec) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						throw FALSE;
					}
					UINT uiDirPosNum = 0;
					if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
						, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiPathTblSize
						, volDesc.ISO_9660.uiPathTblPos, volDesc.bPathType, 0, pPathTblRec, &uiDirPosNum)) {
						throw FALSE;
					}
					if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
						, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiRootDataLen, 0, pPathTblRec, uiDirPosNum)) {
						OutputVolDescLog("Failed to read ISO9660\n");
						uiDirPosNum = 0;
						if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
							, volDesc.JOLIET.uiLogicalBlkCoef, volDesc.JOLIET.uiPathTblSize
							, volDesc.JOLIET.uiPathTblPos, volDesc.bPathType, 0, pPathTblRec, &uiDirPosNum)) {
							throw FALSE;
						}
						if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
							, volDesc.JOLIET.uiLogicalBlkCoef, volDesc.JOLIET.uiRootDataLen, 0, pPathTblRec, uiDirPosNum)) {
							throw FALSE;
						}
					}
					if (pExtArg->byScanProtectViaFile || pExtArg->byIntentionalSub) {
						if (!ReadCDForCheckingExe(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf)) {
							throw FALSE;
						}
					}
					if (pDisc->PROTECT.byExist) {
						if (pDisc->PROTECT.byExist == securomTmp && strncmp(pDisc->PROTECT.name[0], ".cms_t", 6) && strncmp(pDisc->PROTECT.name[0], ".cms_d", 6)) {
							OutputLog(standardOut | fileDisc, "Detected unknown string [%" CHARWIDTH "s], check if this disc has SecuROM", pDisc->PROTECT.name[0]);
						}
						else {
							OutputLog(standardOut | fileDisc, "Detected a protected [%" CHARWIDTH "s]. LBA %d to %d"
								, pDisc->PROTECT.name[0], pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0]
								, pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0]);
						}
						if (pDisc->PROTECT.byExist == datelAlt) {
							OutputLog(standardOut | fileDisc, ",  [%" CHARWIDTH "s]. LBA %d to %d\n"
								, pDisc->PROTECT.name2, pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd
								, pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd + pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd);
						}
						else {
							if (pDisc->PROTECT.byExist == physicalErr) {
								for (INT j = 1; j < pExtArg->FILE.readErrCnt; j++) {
									OutputLog(standardOut | fileDisc, ", [%" CHARWIDTH "s]. LBA %d to %d"
										, pDisc->PROTECT.name[j], pDisc->PROTECT.ERROR_SECTOR.nExtentPos[j]
										, pDisc->PROTECT.ERROR_SECTOR.nExtentPos[j] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[j]);
								}
							}
							else if (pDisc->PROTECT.byExist == c2Err) {
								for (INT j = 1; j < pExtArg->FILE.c2ErrCnt; j++) {
									OutputLog(standardOut | fileDisc, ", [%" CHARWIDTH "s]. LBA %d to %d"
										, pDisc->PROTECT.name[j], pDisc->PROTECT.ERROR_SECTOR.nExtentPos[j]
										, pDisc->PROTECT.ERROR_SECTOR.nExtentPos[j] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[j]);
								}
							}

							OutputLog(standardOut | fileDisc, "\n");
						}
					}
				}

				BOOL bMac = FALSE;
				INT nLBA = pDisc->SCSI.lp1stLBAListOnToc[i];
				cdb.TransferLength[3] = 1;
				if (pDisc->SCSI.trkType == TRACK_TYPE::pregapDataIn1stTrack) {
					nLBA = 0;
				}
				if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
					DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
					// some disc occur VENDOR UNIQUE ERROR
					// http://forum.redump.org/post/101367/#p101367
					// [FMT] Rainbow Islands: The Story of Bubble Bobble 2: Extra Version
					// http://redump.org/disc/74148/
					throw TRUE;
				}
				// for MAC pattern 1
				if (IsDriverDescriptorRecord(lpBuf)) {
					OutputFsDriveDescriptorRecord(lpBuf);
					if (IsApplePartionMap(lpBuf + 512)) {
						BOOL bHfs = FALSE;
						LONG firstPartition = 0;
						UINT numOfPartion = MAKEUINT(MAKEWORD(lpBuf[519], lpBuf[518]), MAKEWORD(lpBuf[517], lpBuf[516]));
						for (UINT j = 1; j <= numOfPartion; j++) {
							OutputFsPartitionMap(lpBuf + 512 * j, &bHfs);
							if (bHfs && firstPartition == 0) {
								firstPartition = MAKELONG(MAKEWORD(lpBuf[523], lpBuf[522]), MAKEWORD(lpBuf[521], lpBuf[520]));
							}
							OutputString("\rReading Partition Map %u/%u", j, numOfPartion);
						}
						OutputString("\n");
						nLBA += 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						UINT uiCatalogFileSize = 0;
						if (IsValidMacDataHeader(lpBuf + 1024)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 1024, nLBA, &uiCatalogFileSize);
						}
						else if (IsValidMacDataHeader(lpBuf + 512)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 512, nLBA, &uiCatalogFileSize);
						}
						if (uiCatalogFileSize) {
							ReadCDForHfsCatalogFiles(pExtArg, pDevice, cdb, lpBuf, uiCatalogFileSize);
						}
						// for MAC pattern 2
						nLBA += 15;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValidMacDataHeader(lpBuf + 1024)) {
							OutputFsMasterDirectoryBlocks(lpBuf + 1024, nLBA, &uiCatalogFileSize);
							ReadCDForHfsCatalogFiles(pExtArg, pDevice, cdb, lpBuf, uiCatalogFileSize);
						}
					}
					bMac = TRUE;
				}

				if (!bVD && !bMac) {
					BOOL bOtherHeader = FALSE;
					// for pce, pc-fx
					nLBA = pDisc->SCSI.n1stLBAofDataTrk;
					if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
						DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
						throw FALSE;
					}
					if (IsValidPceSector(lpBuf)) {
						OutputFsPceStuff(lpBuf, nLBA);
						nLBA = pDisc->SCSI.n1stLBAofDataTrk + 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						OutputFsPceBootSector(lpBuf, nLBA);
						bOtherHeader = TRUE;
					}
					else if (IsValidPcfxSector(lpBuf)) {
						OutputFsPcfxHeader(lpBuf, nLBA);
						nLBA = pDisc->SCSI.n1stLBAofDataTrk + 1;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						OutputFsPcfxSector(lpBuf, nLBA);
						bOtherHeader = TRUE;
					}

					if (!bOtherHeader) {
						// for 3DO
						nLBA = 0;
						if (!ExecReadCD(pExtArg, pDevice, (LPBYTE)&cdb, nLBA, lpBuf,
							DISC_MAIN_DATA_SIZE, _T(__FUNCTION__), __LINE__)) {
							throw FALSE;
						}
						if (IsValid3doDataHeader(lpBuf)) {
							OutputFs3doHeader(lpBuf, nLBA);
							if (!ReadCDFor3DODirectory(pExtArg, pDevice, pDisc, &cdb, (LPCCH)"/",
								(INT)MAKEUINT(MAKEWORD(lpBuf[103], lpBuf[102]), MAKEWORD(lpBuf[101], lpBuf[100])))) {
								throw FALSE;
							}
						}
					}
				}
			}
			catch (BOOL bErr) {
				bRet = bErr;
			}
			FreeAndNull(pPathTblRec);
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
	PPATH_TABLE_RECORD pPathTblRec = NULL;
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
			pPathTblRec = (PPATH_TABLE_RECORD)calloc(PATH_TABLE_RECORD_SIZE, sizeof(PATH_TABLE_RECORD));
			if (!pPathTblRec) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			UINT uiDirPosNum = 0;
			if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb
				, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiPathTblSize
				, volDesc.ISO_9660.uiPathTblPos, volDesc.bPathType, nSectorOfs, pPathTblRec, &uiDirPosNum)) {
				throw FALSE;
			}
			if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)&cdb, lpBuf
				, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiRootDataLen, nSectorOfs, pPathTblRec, uiDirPosNum)) {
				throw FALSE;
			}
		}
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pPathTblRec);
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
	FOUR_BYTE transferLen;
	transferLen.AsULong = 1;
	REVERSE_BYTES(&cdb->TransferLength, &transferLen);

	if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice
		, pDisc, 0, (LPBYTE)cdb, lpBuf, 16, 0, &bPVD, &volDesc, (BYTE)transferLen.AsULong)) {
		return FALSE;
	}
	if (bPVD) {
		PPATH_TABLE_RECORD pPathTblRec = (PPATH_TABLE_RECORD)calloc(PATH_TABLE_RECORD_SIZE, sizeof(PATH_TABLE_RECORD));
		if (!pPathTblRec) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		UINT uiDirPosNum = 0;
		if (!ReadPathTableRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)cdb
			, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiPathTblSize
			, volDesc.ISO_9660.uiPathTblPos, volDesc.bPathType, 0, pPathTblRec, &uiDirPosNum)) {
			FreeAndNull(pPathTblRec);
			return FALSE;
		}
		if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)cdb, lpBuf
			, volDesc.ISO_9660.uiLogicalBlkCoef, volDesc.ISO_9660.uiRootDataLen, 0, pPathTblRec, uiDirPosNum)) {
			FreeAndNull(pPathTblRec);
			return FALSE;
		}
		FreeAndNull(pPathTblRec);

		if (pDisc->PROTECT.byExist && !pExtArg->byNoSkipSS) {
			if (pDisc->PROTECT.byExist == arccos) {
				OutputLog(standardOut | fileDisc, "This disc has possibly [%" CHARWIDTH "s]", pDisc->PROTECT.name[0]);
			}
			else if (pDisc->PROTECT.byExist == securomTmp && strncmp(pDisc->PROTECT.name[0], ".cms_t", 6) && strncmp(pDisc->PROTECT.name[0], ".cms_d", 6)) {
				OutputLog(standardOut | fileDisc, "Detected unknown string [%" CHARWIDTH "s], check if this disc has SecuROM", pDisc->PROTECT.name[0]);
			}
			else {
				OutputLog(standardOut | fileDisc, "Detected protection [%" CHARWIDTH "s]", pDisc->PROTECT.name[0]);
			}
			if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] != 0 || pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0] != 0) {
				OutputLog(standardOut | fileDisc, " LBA %d to %d"
					, pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0]
					, pDisc->PROTECT.ERROR_SECTOR.nExtentPos[0] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[0]);
			}
			OutputLog(standardOut | fileDisc, "\n");
			for (INT j = 1; j < pExtArg->FILE.readErrCnt; j++) {
				OutputLog(standardOut | fileDisc, ", [%" CHARWIDTH "s]. LBA %d to %d"
					, pDisc->PROTECT.name[j], pDisc->PROTECT.ERROR_SECTOR.nExtentPos[j]
					, pDisc->PROTECT.ERROR_SECTOR.nExtentPos[j] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[j]);
			}
		}
	}

	transferLen.AsULong = 6;
	FOUR_BYTE LBA;
	LBA.AsULong = 16;
	REVERSE_BYTES(&cdb->TransferLength, &transferLen);
	REVERSE_BYTES(&cdb->LogicalBlock[0], &LBA);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	BOOL bUDF = FALSE;
	for (UINT i = 0; i < DISC_MAIN_DATA_SIZE * transferLen.AsULong; i += DISC_MAIN_DATA_SIZE, LBA.AsULong++) {
		OutputFsVolumeRecognitionSequence(lpBuf + i, (INT)LBA.AsULong, &bUDF);
	}

	if (bUDF) {
		// for Anchor Volume Descriptor Pointer
		transferLen.AsULong = 1;
		LBA.AsULong = 256;
		REVERSE_BYTES(&cdb->TransferLength, &transferLen);
		REVERSE_BYTES(&cdb->LogicalBlock[0], &LBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		UDF udf = {};
		OutputFsVolumeDescriptorSequence(lpBuf, (INT)LBA.AsULong, &udf);

		// for PVD, IUVD, PD, LVD etc.
		transferLen.AsULong = udf.uiPVDLen / DISC_MAIN_DATA_SIZE;
		LBA.AsULong = (ULONG)udf.uiPVDPos;
		REVERSE_BYTES(&cdb->TransferLength, &transferLen);
		REVERSE_BYTES(&cdb->LogicalBlock[0], &LBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}

		DWORD dwTransferLenBak = transferLen.AsULong;
		INT nLBABak = 0;
		for (UINT i = 0; i < DISC_MAIN_DATA_SIZE * transferLen.AsULong; i += DISC_MAIN_DATA_SIZE, LBA.AsULong++) {
			OutputFsVolumeDescriptorSequence(lpBuf + i, (INT)LBA.AsULong, &udf);
			WORD wTagId = MAKEWORD(lpBuf[i], lpBuf[i + 1]);
			if (wTagId == 5) {
				nLBABak = (INT)LBA.AsULong;
				transferLen.AsULong = 1;
				INT nCnt = 0;
				INT nLastLBAOfAVDP = 0;
				REVERSE_BYTES(&cdb->TransferLength, &transferLen);

				for (INT j = 0; j < 512; j++) {
					LBA.AsULong = (ULONG)(udf.uiPartitionPos + udf.uiPartitionLen + j);
					if (LBA.AsULong >= (ULONG)pDisc->SCSI.nAllLength) {
						break;
					}
					REVERSE_BYTES(&cdb->LogicalBlock[0], &LBA);
					if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
						direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						break;
					}
					wTagId = MAKEWORD(lpBuf[0], lpBuf[1]);
					if (wTagId == 2) {
						// NOTE: An AnchorVolumeDescriptorPointer structure shall be recorded in at 
						// least 2 of the following 3 locations on the media: 
						//   Logical Sector 256. 
						//   Logical Sector (N - 256). 
						//   N
						OutputDiscLog("Detected Anchor Volume Descriptor Pointer: LBA %lu\n", LBA.AsULong);
						nLastLBAOfAVDP = (INT)LBA.AsULong;
						if (++nCnt == 2) {
							break;
						}
					}
				}
				if (nCnt) {
					if (pExtArg->byAnchorVolumeDescriptorPointer) {
						// Because TOC length of -R, -RW, a part of BD-ROM disc is used as the full disc size
						OutputDiscLog(" => Updated TOC length to %d\n", nLastLBAOfAVDP + 1);
						pDisc->SCSI.nAllLength = nLastLBAOfAVDP + 1;
					}
				}
				transferLen.AsULong = dwTransferLenBak;
				LBA.AsULong = (ULONG)nLBABak;
			}
		}

		// for Integrity Sequence Extent
		transferLen.AsULong = udf.uiLogicalVolumeIntegrityLen / DISC_MAIN_DATA_SIZE;
		LBA.AsULong = (ULONG)udf.uiLogicalVolumeIntegrityPos;
		REVERSE_BYTES(&cdb->TransferLength, &transferLen);
		REVERSE_BYTES(&cdb->LogicalBlock[0], &LBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		for (UINT i = 0; i < DISC_MAIN_DATA_SIZE * transferLen.AsULong; i += DISC_MAIN_DATA_SIZE, LBA.AsULong++) {
			OutputFsVolumeDescriptorSequence(lpBuf + i, (INT)LBA.AsULong, &udf);
		}
#if 1
		// for File Set Descriptor
		transferLen.AsULong = udf.uiFSDLen / DISC_MAIN_DATA_SIZE;
		LBA.AsULong = (ULONG)(udf.uiPartitionPos + udf.uiFSDPos);
		REVERSE_BYTES(&cdb->TransferLength, &transferLen);
		REVERSE_BYTES(&cdb->LogicalBlock[0], &LBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		for (UINT i = 0; i < DISC_MAIN_DATA_SIZE * transferLen.AsULong; i += DISC_MAIN_DATA_SIZE, LBA.AsULong++) {
			OutputFsVolumeDescriptorSequence(lpBuf + i, (INT)LBA.AsULong, &udf);
		}

		// for File Entry
		transferLen.AsULong = udf.uiFileEntryLen / DISC_MAIN_DATA_SIZE;
		LBA.AsULong = (ULONG)(udf.uiPartitionPos + udf.uiFileEntryPos);
		REVERSE_BYTES(&cdb->TransferLength, &transferLen);
		REVERSE_BYTES(&cdb->LogicalBlock[0], &LBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		for (UINT i = 0; i < DISC_MAIN_DATA_SIZE * transferLen.AsULong; i += DISC_MAIN_DATA_SIZE, LBA.AsULong++) {
			OutputFsVolumeDescriptorSequence(lpBuf + i, (INT)LBA.AsULong, &udf);
		}
#if 0
		// for File Identifier Descriptor
		UINT FileLen = uiExtLen / DISC_MAIN_DATA_SIZE;
		UINT FileLenMod = uiExtLen % DISC_MAIN_DATA_SIZE;
		if (FileLenMod != 0) {
			FileLen += 1;
		}
		transferLen.AsULong = FileLen;
		LBA.AsULong = (ULONG)(udf.uiPartitionPos + udf.uiFSDPos);
		REVERSE_BYTES(&cdb->TransferLength, &transferLen);
		REVERSE_BYTES(&cdb->LogicalBlock[0], &LBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE * transferLen.AsULong, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			return FALSE;
		}
		for (UINT i = 0; i < DISC_MAIN_DATA_SIZE * transferLen.AsULong; i += DISC_MAIN_DATA_SIZE, LBA.AsULong++) {
			OutputFsVolumeDescriptorSequence(lpBuf + i, (INT)LBA.AsULong, &udf);
		}
#endif
#endif
	}
	else {
		if (pDisc->SCSI.wCurrentMedia == ProfileDvdRewritable ||
			pDisc->SCSI.wCurrentMedia == ProfileBDRom||
			pDisc->SCSI.wCurrentMedia == ProfileBDRSequentialWritable ||
			pDisc->SCSI.wCurrentMedia == ProfileBDRewritable ||
			pDisc->SCSI.wCurrentMedia == ProfilePlaystation3BDRom ||
			pDisc->SCSI.wCurrentMedia == ProfilePlaystation4BDRom
			) {
			pDisc->SCSI.nAllLength = (INT)volDesc.uiVolumeSpaceSize;
		}
	}
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
	_TCHAR* pTab,
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
	OutputVolDescLog(
		"%s Offset to left sub-tree entry: %u (0x%04x)\n"
		"%sOffset to right sub-tree entry: %u (0x%04x)\n"
		"%s       Starting sector of file: %u (0x%08x)\n"
		"%s               Total file size: %u (0x%08x)\n"
		"%s               File attributes: "
		, &pTab[0], ofsLeft, ofsLeft
		, &pTab[0], ofsRight, ofsRight
		, &pTab[0], startSector, startSector
		, &pTab[0], fileSize, fileSize
		, &pTab[0]
	);

	BYTE attribute = lpBuf[12];
	if (attribute == 0x01) {
		OutputVolDescLog("read only\n");
	}
	else if (attribute == 0x02) {
		OutputVolDescLog("hidden\n");
	}
	else if (attribute == 0x04) {
		OutputVolDescLog("system file\n");
	}
	else if (attribute == 0x10) {
		OutputVolDescLog("directory\n");
	}
	else if (attribute == 0x20) {
		OutputVolDescLog("archive\n");
	}
	else if (attribute == 0x80) {
		OutputVolDescLog("normal\n");
	}
	else {
		OutputVolDescLog("other\n");
	}
	BYTE lenOfFile = lpBuf[13];
	OutputVolDescLog(
		"%s            Length of filename: %u\n"
		"%s                      Filename: "
		, &pTab[0], lenOfFile
		, &pTab[0]
	);
	for (BYTE i = 0; i < lenOfFile; i++) {
		OutputVolDescLog("%c", lpBuf[14 + i]);
	}
	OutputVolDescLog("\n\n");
	UINT mod = (14 + lenOfFile) % sizeof(UINT);
	if (mod != 0) {
		*lpOfs = (UINT)(14 + lenOfFile + sizeof(UINT) - mod);
	}
	else {
		*lpOfs = (UINT)(14 + lenOfFile);
	}

	if (attribute == 0x10) {
		size_t idx = _tcslen(&pTab[0]);
		pTab[idx] = _T('\t');
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
	_TCHAR* pTab
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
	OutputMainInfoLog("uiDirTblSize: %u ", uiDirTblSize);
	UINT uiReadSize = uiDirTblSize;
	UINT nRoopCnt = uiReadSize / (UINT)pDevice->dwMaxTransferLength + 1;
	if (uiDirTblSize > (UINT)pDevice->dwMaxTransferLength) {
		uiReadSize = (UINT)pDevice->dwMaxTransferLength;
		OutputMainInfoLog("--> %u ", uiReadSize);
	}
	FOUR_BYTE dirPos;
	dirPos.AsULong = (ULONG)uiDirPos;
	for (UINT i = 0; i < nRoopCnt; i++) {
		pCdb->TransferLength[3] = (UCHAR)(uiReadSize / DISC_MAIN_DATA_SIZE);
		REVERSE_BYTES(pCdb->LogicalBlock, &dirPos);

		OutputMainInfoLog("uiDirPos: %lu, TransferLength: %u\n", dirPos.AsULong, pCdb->TransferLength[3]);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, pCdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, uiReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		for (UCHAR c = 0; c < pCdb->TransferLength[3]; c++) {
			OutputMainChannel(fileMainInfo, lpBuf + DISC_MAIN_DATA_SIZE * c, NULL, (INT)dirPos.AsULong + c, DISC_MAIN_DATA_SIZE);
		}
		OutputVolDescLog("%s"
			OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("DIRECTORY ENTRY")
			, &pTab[0], (INT)dirPos.AsULong, (UINT)dirPos.AsULong
		);
		BOOL bEnd = FALSE;
		UINT uiSize = 0;
		UINT uiCoeff = 1;
		for (UINT uiOfs = 0; uiSize < uiReadSize;) {
			if (!OutputXDVDFsDirectoryRecord(pExtArg, pDevice, pCdb
				, lpBuf + uiSize, &uiOfs, uiStartLBA, pTab, &bEnd)) {
				return FALSE;
			}
			if (bEnd) {
				break;
			}
			else {
				uiSize += uiOfs;
				if (uiSize > DISC_MAIN_DATA_SIZE * uiCoeff - 15) {
					uiSize += DISC_MAIN_DATA_SIZE * uiCoeff - uiSize;
					uiCoeff++;
				}
			}
		}
		if (nRoopCnt > 1) {
			uiDirTblSize -= (UINT)pDevice->dwMaxTransferLength;
			dirPos.AsULong += uiReadSize / DISC_MAIN_DATA_SIZE;
			uiReadSize = uiDirTblSize;
			if (uiDirTblSize > pDevice->dwMaxTransferLength) {
				uiReadSize = (UINT)pDevice->dwMaxTransferLength;
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
	cdb.TransferLength[3] = (UCHAR)(pDevice->dwMaxTransferLength / DISC_MAIN_DATA_SIZE);
	FOUR_BYTE LBA;
	LBA.AsULong = dwStartLBA + 32;
	REVERSE_BYTES(&cdb.LogicalBlock, &LBA);

	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, pDevice->dwMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		FreeAndNull(pBuf);
		return FALSE;
	}
	OutputMainChannel(fileMainInfo, lpBuf, NULL, (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE);

	UINT uiDirPos = MAKEUINT(MAKEWORD(lpBuf[20], lpBuf[21]), MAKEWORD(lpBuf[22], lpBuf[23]));
	UINT uiDirTblSize = MAKEUINT(MAKEWORD(lpBuf[24], lpBuf[25]), MAKEWORD(lpBuf[26], lpBuf[27]));
	CHAR date[20] = {};
	printwin32filetime(MAKEUINT64(MAKELONG(MAKEWORD(lpBuf[28], lpBuf[29]), MAKEWORD(lpBuf[30], lpBuf[31]))
		, MAKELONG(MAKEWORD(lpBuf[32], lpBuf[33]), MAKEWORD(lpBuf[34], lpBuf[35]))), date);
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("XDVDFS")
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("VOLUME DESCRIPTOR")
		"\t                        Header: %.20" CHARWIDTH "s\n"
		"\tSector of root directory table: %u (%#08x)\n"
		"\t  Size of root directory table: %u (%#08x)\n"
		"\t           Image creation time: %.20" CHARWIDTH "s\n"
		"\t                        Footer: %.20" CHARWIDTH "s\n"
		, (INT)LBA.AsULong, (UINT)LBA.AsULong
		, &lpBuf[0]
		, uiDirPos, uiDirPos
		, uiDirTblSize, uiDirTblSize
		, date, &lpBuf[2028]
	);
	if (uiDirTblSize % DISC_MAIN_DATA_SIZE != 0) {
		uiDirTblSize += DISC_MAIN_DATA_SIZE * (uiDirTblSize / DISC_MAIN_DATA_SIZE + 1) - uiDirTblSize;
	}
	_TCHAR szTab[256] = {};
	szTab[0] = _T('\t');
	OutputString("Reading Xbox DirectoryRecord\n");
	if (!ReadXBOXDirectoryRecord(
		pExtArg, pDevice, &cdb, uiDirPos + (UINT)dwStartLBA, uiDirTblSize, (UINT)dwStartLBA, szTab)) {
		FreeAndNull(pBuf);
		return FALSE;
	};
	FreeAndNull(pBuf);
	return TRUE;
}

BOOL ReadNintendoSystemHeader(
	LPCTSTR pszFullPath,
	FILE** fp,
	LPBYTE buf
) {
	*fp = CreateOrOpenFile(pszFullPath, NULL, NULL, NULL, NULL, _T(".iso"), _T("rb"), 0, 0);
	if (!*fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	if (fread(buf, sizeof(BYTE), 1024, *fp) != 1024) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(*fp);
		return FALSE;
	};
	// http://hitmen.c02.at/files/yagcd/yagcd/chap13.html#sec13.1
	// https://wiibrew.org/wiki/Wii_disc#Header
	OutputMainChannel(fileMainInfo, buf, _T("Disc Header"), 0, 1024);
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("Disc Header")
		"\t                                 Disc ID: %.1" CHARWIDTH "s\n"
		"\t                               Game Code: %.2" CHARWIDTH "s\n"
		"\t                             Region Code: %.1" CHARWIDTH "s\n"
		"\t                              Maker Code: %.2" CHARWIDTH "s\n"
		"\t                             Disc Number: %d\n"
		"\t                                 Version: %d\n"
		"\t                         Audio Streaming: %d\n"
		"\t                      Stream Buffer Size: %d\n"
		"\t                               Game Name: %" CHARWIDTH "s\n"
		, &buf[0], &buf[1], &buf[3], &buf[4]
		, buf[6], buf[7], buf[8], buf[9], &buf[32]
	);
	return TRUE;
}

BOOL ReadNintendoFileSystem(
	PDEVICE pDevice,
	LPCTSTR pszFullPath,
	DISC_TYPE_DVD type
) {
	FILE* fp = NULL;
	BYTE buf[1024] = {};
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("NintendoOpticalDiscFS"));
	if (!ReadNintendoSystemHeader(pszFullPath, &fp, buf)) {
		return FALSE;
	};
	if (fread(buf, sizeof(BYTE), 64, fp) != 64) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fp);
		return FALSE;
	};
	// http://hitmen.c02.at/files/yagcd/yagcd/chap13.html#sec13.1
	OutputMainChannel(fileMainInfo, buf, _T("NintendoOpticalDiscFS"), 0, 64);
	UINT ofsOfFst = MAKEUINT(MAKEWORD(buf[39], buf[38]), MAKEWORD(buf[37], buf[36]));
	UINT sizeOfFst = MAKEUINT(MAKEWORD(buf[43], buf[42]), MAKEWORD(buf[41], buf[40]));
	OutputVolDescLog(
		"\t      offset of debug monitor (dh.bin) ?: %u (%#08x)\n"
		"\t        addr (?) to load debug monitor ?: %#08x\n"
		"\toffset of main executable DOL (bootfile): %u (%#08x)\n"
		"\t             offset of the FST (fst.bin): %u (%#08x)\n"
		"\t                             size of FST: %u (%#08x)\n"
		"\t                     maximum size of FST: %u (%#08x)\n"
		"\t                       user position (?): %#08x\n"
		"\t                         user length (?): %u (%#08x)\n"
		"\t                                 unknown: %u (%#08x)\n"
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
	// http://hitmen.c02.at/files/yagcd/yagcd/chap13.html#sec13.3
	OutputMainChannel(fileMainInfo, buf, _T("Apploader"), 0x2440 / 0x800, 0x20);
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("Apploader")
		"\tdate (version) of the apploader: %" CHARWIDTH "s\n"
		"\t           apploader entrypoint: %#x\n"
		"\t          size of the apploader: %u (%#08x)\n"
		"\t                   trailer size: %u (%#08x)\n"
		, &buf[0]
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
	// http://hitmen.c02.at/files/yagcd/yagcd/chap13.html#sec13.4
	OutputMainChannel(fileMainInfo, lpBuf, _T("Root Directory Entry"), (INT)ofsOfFst / 0x800, sizeOfFst);
	UINT numOfEntries = MAKEUINT(MAKEWORD(lpBuf[11], lpBuf[10]), MAKEWORD(lpBuf[9], lpBuf[8]));
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("Root Directory Entry")
		"\t                   flags: %d\n"
		"\toffset into string table: %u (%#08x)\n"
		"\t           parent_offset: %u (%#08x)\n"
		"\t             num_entries: %u \n\n"
		, lpBuf[0]
		, MAKEUINT(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0))
		, MAKEUINT(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0))
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, MAKEUINT(MAKEWORD(lpBuf[7], lpBuf[6]), MAKEWORD(lpBuf[5], lpBuf[4]))
		, numOfEntries
	);
	
	UINT posOfString = numOfEntries * 12;
	// http://hitmen.c02.at/files/yagcd/yagcd/chap13.html#sec13.4.1
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("Directory Entry"));

	for (UINT i = 12; i < posOfString; i += 12) {
		if (lpBuf[0 + i] == 0 || lpBuf[0 + i] == 1) {
			UINT ofsString = MAKEUINT(MAKEWORD(lpBuf[3 + i], lpBuf[2 + i]), MAKEWORD(lpBuf[1 + i], 0));
			OutputVolDescLog(
				"\t                   flags: %d\n"
				"\toffset into string table: %u (%#08x)\n"
				, lpBuf[0 + i], ofsString, ofsString
			);
			if (lpBuf[0 + i] == 0) {
				OutputVolDescLog(
					"\t               file_name: %" CHARWIDTH "s\n"
					"\t             file_offset: %u (%#08x)\n"
					"\t             file_length: %u (%#08x)\n\n"
					, &lpBuf[posOfString + ofsString]
					, MAKEUINT(MAKEWORD(lpBuf[7 + i], lpBuf[6 + i]), MAKEWORD(lpBuf[5 + i], lpBuf[4 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[7 + i], lpBuf[6 + i]), MAKEWORD(lpBuf[5 + i], lpBuf[4 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[11 + i], lpBuf[10 + i]), MAKEWORD(lpBuf[9 + i], lpBuf[8 + i]))
					, MAKEUINT(MAKEWORD(lpBuf[11 + i], lpBuf[10 + i]), MAKEWORD(lpBuf[9 + i], lpBuf[8 + i]))
				);
			}
			else if (lpBuf[0 + i] == 1) {
				OutputVolDescLog(
					"\t                dir_name: %" CHARWIDTH "s\n"
					"\t           parent_offset: %u (%#08x)\n"
					"\t             next_offset: %u (%#08x)\n\n"
					, &lpBuf[posOfString + ofsString]
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
		// https://wiibrew.org/wiki/Wii_disc#Partition_table_entry
		OutputVolDescLog(
			"\t" OUTPUT_DHYPHEN_PLUS_STR("Partition table entry")
			"\t           Partition offset: %#08x\n"
			"\t                       Type: %u\n"
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
	OutputVolDescLog(OUTPUT_DHYPHEN_PLUS_STR("WiiFS"));
	if (!ReadNintendoSystemHeader(pszFullPath, &fp, buf)) {
		return FALSE;
	};

	fseek(fp, 0x40000, SEEK_SET);
	if (fread(buf, sizeof(BYTE), 0x20, fp) != 0x20) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		FcloseAndNull(fp);
		return FALSE;
	};
	OutputMainChannel(fileMainInfo, buf, _T("WiiFS"), 0x40000 / 0x800, 0x20);
	UINT numOfPartition[4] = {};
	numOfPartition[0] = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
	UINT ofsOfPart1 = MAKEUINT(MAKEWORD(buf[7], buf[6]), MAKEWORD(buf[5], buf[4]));
	numOfPartition[1] = MAKEUINT(MAKEWORD(buf[11], buf[10]), MAKEWORD(buf[9], buf[8]));
	UINT ofsOfPart2 = MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], buf[12]));
	numOfPartition[2] = MAKEUINT(MAKEWORD(buf[19], buf[18]), MAKEWORD(buf[17], buf[16]));
	UINT ofsOfPart3 = MAKEUINT(MAKEWORD(buf[23], buf[22]), MAKEWORD(buf[21], buf[20]));
	numOfPartition[3] = MAKEUINT(MAKEWORD(buf[27], buf[26]), MAKEWORD(buf[25], buf[24]));
	UINT ofsOfPart4 = MAKEUINT(MAKEWORD(buf[31], buf[30]), MAKEWORD(buf[29], buf[28]));

	// https://wiibrew.org/wiki/Wii_disc#Partitions_information
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("Partitions information")
		"\t       Total 1st partitions: %u\n"
		"\tPartition info table offset: %#08x\n"
		"\t       Total 2nd partitions: %u\n"
		"\tPartition info table offset: %#08x\n"
		"\t       Total 3rd partitions: %u\n"
		"\tPartition info table offset: %#08x\n"
		"\t       Total 4th partitions: %u\n"
		"\tPartition info table offset: %#08x\n"
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
	// https://wiibrew.org/wiki/Wii_disc#Region_setting
	OutputMainChannel(fileMainInfo, buf, _T("Region setting"), 0x4e000 / 0x800, 0x20);
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("Region setting")
		"\t                     Region byte: %u\n"
		"\tAge Rating byte for Japan/Taiwan: %d (%#02x)\n"
		"\tAge Rating byte for USA         : %d (%#02x)\n"
		"\tAge Rating byte for ------------: %d (%#02x)\n"
		"\tAge Rating byte for Germany     : %d (%#02x)\n"
		"\tAge Rating byte for PEGI        : %d (%#02x)\n"
		"\tAge Rating byte for Finland     : %d (%#02x)\n"
		"\tAge Rating byte for Portugal    : %d (%#02x)\n"
		"\tAge Rating byte for Britain     : %d (%#02x)\n"
		"\tAge Rating byte for Australia   : %d (%#02x)\n"
		"\tAge Rating byte for Korea       : %d (%#02x)\n"
		"\tAge Rating byte for ------------: %d (%#02x)\n"
		"\tAge Rating byte for ------------: %d (%#02x)\n"
		"\tAge Rating byte for ------------: %d (%#02x)\n"
		"\tAge Rating byte for ------------: %d (%#02x)\n"
		"\tAge Rating byte for ------------: %d (%#02x)\n"
		"\tAge Rating byte for ------------: %d (%#02x)\n"
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
		OutputErrorString("You can't decrypt iso because there isn't a key.bin\n");
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
			OutputMainChannel(fileMainInfo, buf, _T("Partition"), (INT)realOfsOfPartion / 0x800, 0x2C0);
			UINT sigType = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
			// https://wiibrew.org/wiki/Wii_disc#Partition
			// https://wiibrew.org/wiki/Ticket
			OutputVolDescLog(
				OUTPUT_DHYPHEN_PLUS_STR("Partition")
				"\t                          Ticket\n"
				"\t\t                  Signature type: %#x\n"
				"\t\tSignature by a certificate's key: "
				, sigType
			);
			INT size = 256;
			if (sigType == 0x10000) {
				size = 512;
			}
			for (INT i = 0; i < size; i++) {
				OutputVolDescLog("%02x", buf[4 + i]);
			}
			OutputVolDescLog(
				"\n"
				"\t\t                Signature issuer: %" CHARWIDTH "s\n"
				"\t\t                       ECDH data: "
				, &buf[0x140]
			);
			for (INT i = 0; i < 0x3c; i++) {
				OutputVolDescLog("%02x", buf[0x0180 + i]);
			}
			OutputVolDescLog(
				"\n"
				"\t\t             Encrypted title key: "
			);
			for (INT i = 0; i < 0x10; i++) {
				OutputVolDescLog("%02x", buf[0x01BF + i]);
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
			OutputVolDescLog(
				"\n"
				"\t\t                         Unknown: %#02x\n"
				"\t\t                       ticket_id: %#16llx\n"
				"\t\t                      Console ID: %#08x\n"
				"\t\t                        Title ID: %#16llx\n"
				"\t\t                         Unknown: %#04x\n"
				"\t\t            Ticket title version: %#04x\n"
				"\t\t           Permitted Titles Mask: %#08x\n"
				"\t\t                     Permit mask: %#08x\n"
				"\t\t                    Title Export: %#02x\n"
				"\t\t                Common Key index: %#02x\n"
				"\t\t                         Unknown: "
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
				OutputVolDescLog("%02x", buf[0x01F2 + i]);
			}
			OutputVolDescLog(
				"\n"
				"\t\t      Content access permissions: "
			);
			for (INT i = 0; i < 0x40; i++) {
				OutputVolDescLog("%02x", buf[0x0222 + i]);
			}
			OutputVolDescLog("\n");
			for (INT i = 0; i < 7 * 8; i += 8) {
				OutputVolDescLog(
					"\t\t                      Limit type: %u\n"
					"\t\t                   Maximum usage: %u\n"
					, MAKEUINT(MAKEWORD(buf[0x0267 + i], buf[0x0266 + i]), MAKEWORD(buf[0x0265 + i], buf[0x0264 + i]))
					, MAKEUINT(MAKEWORD(buf[0x026B + i], buf[0x026A + i]), MAKEWORD(buf[0x0269 + i], buf[0x0268 + i]))
				);
			}
			UINT dataSize = MAKEUINT(MAKEWORD(buf[0x02BF], buf[0x02BE]), MAKEWORD(buf[0x02BD], buf[0x02BC]));
			OutputVolDescLog(
				"\t                        TMD size: %u (%#08x)\n"
				"\t                      TMD offset: %#08x\n"
				"\t                 Cert chain size: %u (%#08x)\n"
				"\t               Cert chain offset: %#08x\n"
				"\t          Offset to the H3 table: %#08x\n"
				"\t                     Data offset: %#08x\n"
				"\t                       Data size: %u (%#08x)\n"
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
			OutputMainChannel(fileMainInfo, buf, _T("Partition"), (INT)realOfsOfPartion / 0x800, 0x1E0);

			sigType = MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]));
			// https://wiibrew.org/wiki/Title_metadata
			OutputVolDescLog(
				"\t                  Title metadata\n"
				"\t                          Header\n"
				"\t\t                  Signature type: %#x\n"
				"\t\t                       Signature: "
				, sigType
			);
			size = 256;
			if (sigType == 0x10000) {
				size = 512;
			}
			for (INT i = 0; i < size; i++) {
				OutputVolDescLog("%02x", buf[4 + i]);
			}
			OutputVolDescLog(
				"\n"
				"\t\t               Padding modulo 64: "
			);
			for (INT i = 0; i < 60; i++) {
				OutputVolDescLog("%02x", buf[0x104 + i]);
			}
			OutputVolDescLog(
				"\n"
				"\t\t                          Issuer: %" CHARWIDTH "s\n"
				"\t\t                         Version: %02x\n"
				"\t\t                  ca_crl_version: %02x\n"
				"\t\t              signer_crl_version: %02x\n"
				"\t\t                         Is vWii: %02x\n"
				"\t\t                  System Version: %016llx\n"
				"\t\t                        Title ID: %016llx\n"
				"\t\t                      Title type: %08x\n"
				"\t\t                        Group ID: %04x\n"
				"\t\t                          Region: %04x\n"
				"\t\t                         Ratings: "
				, &buf[0x140], buf[0x180], buf[0x181], buf[0x182], buf[0x183]
				, MAKEUINT64(MAKEUINT(MAKEWORD(buf[0x18B], buf[0x18A]), MAKEWORD(buf[0x189], buf[0x188]))
					, MAKEUINT(MAKEWORD(buf[0x187], buf[0x186]), MAKEWORD(buf[0x185], buf[0x184])))
				, MAKEUINT64(MAKEUINT(MAKEWORD(buf[0x193], buf[0x192]), MAKEWORD(buf[0x191], buf[0x190]))
					, MAKEUINT(MAKEWORD(buf[0x18F], buf[0x18E]), MAKEWORD(buf[0x18D], buf[0x18C])))
				, MAKEUINT(MAKEWORD(buf[0x197], buf[0x196]), MAKEWORD(buf[0x195], buf[0x194]))
				, MAKEWORD(buf[0x199], buf[0x198])
				, MAKEWORD(buf[0x19D], buf[0x19C])
			);
			for (INT i = 0; i < 16; i++) {
				OutputVolDescLog("%02x", buf[0x19E + i]);
			}
			OutputVolDescLog(
				"\n"
				"\t\t                        IPC Mask: "
			);
			for (INT i = 0; i < 12; i++) {
				OutputVolDescLog("%02x", buf[0x1BA + i]);
			}
			WORD numOfContents = MAKEWORD(buf[0x1DF], buf[0x1DE]);
			OutputVolDescLog(
				"\n"
				"\t\t                   Access rights: %08x\n"
				"\t\t                   Title version: %04x\n"
				"\t\t              Number of contents: %d\n"
				, MAKEUINT(MAKEWORD(buf[0x1DB], buf[0x1DA]), MAKEWORD(buf[0x1D9], buf[0x1D8]))
				, MAKEWORD(buf[0x1DD], buf[0x1DC]), numOfContents
			);
			if (fread(buf, sizeof(BYTE), 4, fp) != 4) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				FcloseAndNull(fp);
				return FALSE;
			};
			OutputMainChannel(fileMainInfo, buf, NULL, (INT)realOfsOfPartion / 0x800, 4);
			OutputVolDescLog(
				"\t\t                      boot index: %d\n"
				, MAKEWORD(buf[1], buf[0])
			);
			size_t contentsSize = (size_t)36 * numOfContents;
			if (fread(buf, sizeof(BYTE), contentsSize, fp) != contentsSize) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				FcloseAndNull(fp);
				return FALSE;
			};
			OutputMainChannel(fileMainInfo, buf, NULL, (INT)realOfsOfPartion / 0x800, (UINT)(36 * numOfContents));
			for (size_t i = 0; i < contentsSize; i += 36) {
				OutputVolDescLog(
					"\t                         Content\n"
					"\t\t                      Content ID: %u\n"
					"\t\t                           Index: %u\n"
					"\t\t                            Type: %u\n"
					"\t\t                            Size: %llu (%#llx)\n"
					"\t\t                       SHA1 hash: "
					, MAKEUINT(MAKEWORD(buf[3], buf[2]), MAKEWORD(buf[1], buf[0]))
					, MAKEWORD(buf[5], buf[4])
					, MAKEWORD(buf[7], buf[6])
					, MAKEUINT64(MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], buf[12]))
						, MAKEUINT(MAKEWORD(buf[11], buf[10]), MAKEWORD(buf[9], buf[8])))
					, MAKEUINT64(MAKEUINT(MAKEWORD(buf[15], buf[14]), MAKEWORD(buf[13], buf[12]))
						, MAKEUINT(MAKEWORD(buf[11], buf[10]), MAKEWORD(buf[9], buf[8])))
				);
				for (INT j = 0; j < 20; j++) {
					OutputVolDescLog("%02x", buf[16 + j + i]);
				}
			}
			OutputVolDescLog("\n");

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
				// https://wiibrew.org/wiki/Title_metadata#Certificates
				OutputVolDescLog(
					"\t                    Certificates\n"
					"\t\t                  Signature type: %#x\n"
					"\t\t                       Signature: "
					, sigType
				);
				for (INT i = 0; i < size; i++) {
					OutputVolDescLog("%02x", buf[4 + i]);
				}
				INT ofs = 4 + size + 60;
				OutputVolDescLog(
					"\n"
					"\t\t                          Issuer: %" CHARWIDTH "s\n"
					"\t\t                 Public Key Type: %08x\n"
					"\t\t                            Name: %" CHARWIDTH "s\n"
					"\t\t                      Public Key: "
					, &buf[ofs]
					, MAKEUINT(MAKEWORD(buf[ofs + 67], buf[ofs + 66]), MAKEWORD(buf[ofs + 65], buf[ofs + 64]))
					, &buf[ofs + 68]
				);
				for (INT i = 0; i < 316; i++) {
					OutputVolDescLog("%02x", buf[ofs + 132 + i]);
				}
				OutputVolDescLog("\n");
			}

			if (bDecOK) {
				_TCHAR decPath[_MAX_PATH] = {};
				FILE* fpDec = CreateOrOpenFile(pszFullPath, _T("_dec")
					, decPath, NULL, NULL, _T(".iso"), _T("wb"), (BYTE)idx2, (BYTE)numOfPartition[idx]);
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
					OutputString("\rDecrypting iso %7u/%7u", i, dataSize);
				}
				OutputString("\nDone decrypt\n");
				FcloseAndNull(fpDec);
				ReadNintendoFileSystem(pDevice, decPath, wii);
			}
		}
	}
	FcloseAndNull(fp);
	return TRUE;
}

BOOL ReadBDForPs3DiscSfb(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf
) {
	pCdb->TransferLength[3] = (UCHAR)(pDisc->BD.nSectorSizeForPs3DiscSfb);
	FOUR_BYTE LBA;
	LBA.AsULong = (ULONG)pDisc->BD.nLBAForPs3DiscSfb;
	REVERSE_BYTES(pCdb->LogicalBlock, &LBA);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, pCdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, (DWORD)(DISC_MAIN_DATA_SIZE * pDisc->BD.nSectorSizeForPs3DiscSfb), &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	// https://psdevwiki.com/ps3/PS3_DISC.SFB
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("PS3_DISC.SFB")
		"\tmagic: %c%c%c%c\n"
		"\tversion: %08x\n"
		, pDisc->BD.nLBAForPs3DiscSfb, (UINT)pDisc->BD.nLBAForPs3DiscSfb
		, lpBuf[0], lpBuf[1], lpBuf[2], lpBuf[3]
		, MAKEUINT(MAKEWORD(lpBuf[4], lpBuf[5]), MAKEWORD(lpBuf[6], lpBuf[7]))
	);
	if (lpBuf[0x20] != 0) {
		OutputDiscLog(
			"\t%" CHARWIDTH "s: %" CHARWIDTH "s\n"
			, lpBuf + 0x20, lpBuf + 0x200
		);
	}
	if (lpBuf[0x40] != 0) {
		OutputDiscLog(
			"\t%" CHARWIDTH "s: %" CHARWIDTH "s\n"
			, lpBuf + 0x40, lpBuf + 0x220
		);
	}
	if (lpBuf[0x60] != 0) {
		OutputDiscLog(
			"\t%" CHARWIDTH "s: %" CHARWIDTH "s\n"
			, lpBuf + 0x60, lpBuf + 0x230
		);
	}
	return TRUE;
}

BOOL ReadBDForPup(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf
) {
	pCdb->TransferLength[3] = (UCHAR)(pDisc->BD.nSectorSizeForPup);
	FOUR_BYTE LBA;
	LBA.AsULong = (ULONG)pDisc->BD.nLBAForPup;
	REVERSE_BYTES(pCdb->LogicalBlock, &LBA);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, pCdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, (DWORD)(DISC_MAIN_DATA_SIZE * pDisc->BD.nSectorSizeForPup), &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	// https://www.psdevwiki.com/ps3/Playstation_Update_Package_(PUP)#Structure
	EIGHT_BYTE package_version;
	REVERSE_BYTES_QUAD(&package_version, lpBuf + 8);
	EIGHT_BYTE image_version;
	REVERSE_BYTES_QUAD(&image_version, lpBuf + 0x10);
	EIGHT_BYTE segment_num;
	REVERSE_BYTES_QUAD(&segment_num, lpBuf + 0x18);
	EIGHT_BYTE file_offset;
	REVERSE_BYTES_QUAD(&file_offset, lpBuf + 0x20);
	EIGHT_BYTE file_size;
	REVERSE_BYTES_QUAD(&file_size, lpBuf + 0x28);

	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("PS3UPDAT.PUP")
		"\t          magic: %c%c%c%c%c\n"
		"\t    Format Flag: %x\n"
		"\tPackage Version: %#llx\n"
		"\t  Image Version: %#llx\n"
		"\t Segment Number: %#llx\n"
		"\t  Header Length: %#llx\n"
		"\t    Data Length: %#llx\n"
		, pDisc->BD.nLBAForPup, (UINT)pDisc->BD.nLBAForPup
		, lpBuf[0], lpBuf[1], lpBuf[2], lpBuf[3], lpBuf[4]
		, lpBuf[7], package_version.AsULongLong, image_version.AsULongLong
		, segment_num.AsULongLong, file_offset.AsULongLong, file_size.AsULongLong
	);
	
	EIGHT_BYTE id;
	EIGHT_BYTE offset;
	EIGHT_BYTE size;
	FOUR_BYTE sign_algorithm;
	BYTE buf[DISC_MAIN_DATA_SIZE + 1] = {};

	for (ULONGLONG i = 0; i < segment_num.AsULongLong; i++) {
		REVERSE_BYTES_QUAD(&id, lpBuf + 0x30 + 0x20 * i);
		REVERSE_BYTES_QUAD(&offset, lpBuf + 0x30 + 0x20 * i + 8);
		REVERSE_BYTES_QUAD(&size, lpBuf + 0x30 + 0x20 * i + 0x10);
		REVERSE_BYTES(&sign_algorithm, lpBuf + 0x30 + 0x20 * i + 0x18);
		OutputDiscLog(
			"\tSegmentEntry[%lld]\n"
			"\t\t    ID: %#llx\n"
			"\t\tOffset: %#llx\n"
			"\t\t  Size: %#llx\n"
			"\t\tSignature Algorithm: %s\n"
			, i, id.AsULongLong, offset.AsULongLong, size.AsULongLong
			, sign_algorithm.AsULong == 0 ? _T("HMAC-SHA1") : _T("HMAC-SHA256")
		);
		if (id.AsULongLong == 0x100) {
			OutputDiscLog("\t\tversion: %.5s", lpBuf + offset.AsULongLong);
		}
		else if (id.AsULongLong == 0x101) {
			OutputDiscLog("\t\tlicense.xml: ");
			if (!strncmp((LPCCH)(lpBuf + offset.AsULongLong), "<xml", 4)) {
				OutputDiscLog("%s", lpBuf + offset.AsULongLong);
				ULONGLONG remain = size.AsULongLong - (DISC_MAIN_DATA_SIZE - offset.AsULongLong);
				UINT mod = remain % DISC_MAIN_DATA_SIZE;
				UINT modSector = 0;
				if (mod != 0) {
					modSector = 1;
				}
				ULONGLONG sectorSize = remain / DISC_MAIN_DATA_SIZE + modSector;
				for (UINT j = 1; j <= sectorSize; j++) {
					LBA.AsULong = (ULONG)pDisc->BD.nLBAForPup + j;
					REVERSE_BYTES(pCdb->LogicalBlock, &LBA);

					if (!ScsiPassThroughDirect(pExtArg, pDevice, pCdb, CDB12GENERIC_LENGTH, buf,
						direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
						|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
						return FALSE;
					}
					if (j == sectorSize) {
						for (UINT k = 0; k < mod; k++) {
							OutputDiscLog("%c", buf[k]);
						}
					}
					else {
						OutputDiscLog("%s", buf);
					}
				}
			}
			else {
				for (ULONGLONG k = 0; k < size.AsULongLong; k++) {
					OutputDiscLog("%c", lpBuf[offset.AsULongLong + k]);
				}
				OutputDiscLog("\n");
			}
		}
		else if (id.AsULongLong == 0x102) {
			OutputDiscLog("\t\tpromo_flags.txt: abbr.\n");
		}
		else if (id.AsULongLong == 0x103) {
			ULONGLONG sectorSize = offset.AsULongLong / DISC_MAIN_DATA_SIZE;
			LBA.AsULong = (ULONG)(pDisc->BD.nLBAForPup + sectorSize);
			REVERSE_BYTES(pCdb->LogicalBlock, &LBA);

			if (!ScsiPassThroughDirect(pExtArg, pDevice, pCdb, CDB12GENERIC_LENGTH, buf,
				direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				return FALSE;
			}
			OutputDiscLog("\t\tupdate_flags.txt: %.5s", buf + offset.AsULongLong - DISC_MAIN_DATA_SIZE * sectorSize);
		}
		else if (id.AsULongLong == 0x104) {
			OutputDiscLog("\t\tpatch_build.txt: abbr.\n");
		}
		else if (id.AsULongLong == 0x200) {
			OutputDiscLog("\t\tps3swu.self: abbr.\n");
		}
		else if (id.AsULongLong == 0x201) {
			OutputDiscLog("\t\tvsh.tar: abbr.\n");
		}
		else if (id.AsULongLong == 0x202) {
			OutputDiscLog("\t\tdots.txt: abbr.\n");
		}
		else if (id.AsULongLong == 0x203) {
			OutputDiscLog("\t\tpatch_data.pkg: abbr.\n");
		}
		else if (id.AsULongLong == 0x300) {
			OutputDiscLog("\t\tupdate_files.tar: abbr.\n");
		}
		else if (id.AsULongLong == 0x501) {
			OutputDiscLog("\t\tspkg_hdr.tar: abbr.\n");
		}
		else if (id.AsULongLong == 0x601) {
			OutputDiscLog("\t\tps3swu2.self: abbr.\n");
		}
	}

	EIGHT_BYTE segment_index;
	BYTE digest[20] = {};

	LPBYTE digestEntry = lpBuf + 0x30 + 0x20 * segment_num.AsULongLong;

	for (ULONGLONG i = 0; i < segment_num.AsULongLong; i++) {
		REVERSE_BYTES_QUAD(&segment_index, digestEntry + 0x20 * i);
		memcpy(digest, digestEntry + 0x20 * i + 8, sizeof(digest));
		OutputDiscLog(
			"\tDigestEntry[%lld]\n"
			"\t\t     Segment Index: %#llx\n"
			"\t\tDigest (HMAC-SHA1): %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n"
			, i, segment_index.AsULongLong
			, digest[0], digest[1], digest[2], digest[3]
			, digest[4], digest[5], digest[6], digest[7]
			, digest[8], digest[9], digest[10], digest[11]
			, digest[12], digest[13], digest[14], digest[15]
			, digest[16], digest[17], digest[18], digest[19]
		);
	}

	LPBYTE headerDigest = lpBuf + 0x30 + 0x40 * segment_num.AsULongLong;
	OutputDiscLog(
		"\tHeaderDigest\n"
		"\t\tDigest (HMAC-SHA1): %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n"
		, headerDigest[0], headerDigest[1], headerDigest[2], headerDigest[3]
		, headerDigest[4], headerDigest[5], headerDigest[6], headerDigest[7]
		, headerDigest[8], headerDigest[9], headerDigest[10], headerDigest[11]
		, headerDigest[12], headerDigest[13], headerDigest[14], headerDigest[15]
		, headerDigest[16], headerDigest[17], headerDigest[18], headerDigest[19]
	);

	return TRUE;
}

BOOL ReadBDForParamSfo(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* pCdb,
	LPBYTE lpBuf,
	INT idx
) {
	pCdb->TransferLength[3] = (UCHAR)(pDisc->BD.nSectorSizeForParamSfo[idx]);
	FOUR_BYTE LBA;
	LBA.AsULong = (ULONG)pDisc->BD.nLBAForParamSfo[idx];
	REVERSE_BYTES(pCdb->LogicalBlock, &LBA);
#ifdef _WIN32
	INT direction = SCSI_IOCTL_DATA_IN;
#else
	INT direction = SG_DXFER_FROM_DEV;
#endif
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, pCdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, (DWORD)(DISC_MAIN_DATA_SIZE * pDisc->BD.nSectorSizeForParamSfo[idx]), &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	// https://www.psdevwiki.com/ps3/PARAM.SFO
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
	if (header->magic != 0x46535000) {
		OutputDiscLog("LBA[%lu]: This PARAM.SFO is encrypted\n", LBA.AsULong);
		return TRUE;
	}
	OutputDiscLog(
		OUTPUT_DHYPHEN_PLUS_STR_WITH_LBA_F("PARAM.SFO")
		"\tmagic: %c%c%c\n"
		"\tversion: %u.%02u\n"
		, pDisc->BD.nLBAForParamSfo[idx], (UINT)pDisc->BD.nLBAForParamSfo[idx]
		, (CHAR)((header->magic >> 8) & 0x000000ff)
		, (CHAR)((header->magic >> 16) & 0x000000ff), (CHAR)((header->magic >> 24) & 0x000000ff)
		, (header->version & 0x000000ff), ((header->version >> 8) & 0x000000ff)
	);

	LPBYTE keytable = lpBuf + header->key_table_start;
	LPBYTE datatable = lpBuf + header->data_table_start;
	for (UINT i = 0; i < header->tables_entries; i++) {
		psfo_index_table_entry entry =
			(psfo_index_table_entry)(lpBuf + sizeof(sfo_header) + sizeof(sfo_index_table_entry) * i);
#if 0
		OutputDiscLog(
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
			OutputDiscLog(
				"\t%" CHARWIDTH "s: %" CHARWIDTH "s\n", keytable + entry->key_offset, datatable + entry->data_offset);
		}
		else if (entry->data_fmt == 1028) {
			LPBYTE ofs = datatable + entry->data_offset;
			UINT data = MAKEUINT(MAKEWORD(ofs[0], ofs[1]), MAKEWORD(ofs[2], ofs[3]));
			OutputDiscLog(
				"\t%" CHARWIDTH "s: %x\n", keytable + entry->key_offset, data);
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
	FOUR_BYTE LBA;
	LBA.AsULong = 510;
	REVERSE_BYTES(&cdb.LogicalBlock, &LBA);

	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		FreeAndNull(pBuf);
		return FALSE;
	}
	OutputMainChannel(fileMainInfo, lpBuf, _T("SACD Header"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE);
	typedef struct _Locale {
		CHAR Language_Code[2];
		BYTE Character_Set_Code;
		BYTE Reserved;
	} Locale;
	typedef struct _TextChannel {
		BYTE Number_of_Text_Channels;
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
	mToc.Album_Set_Size = MAKEWORD(lpBuf[0x11], lpBuf[0x10]);
	mToc.Album_Sequence_Number = MAKEWORD(lpBuf[0x13], lpBuf[0x12]);
	mToc.TWOCH_TOC_1_Address = MAKEUINT(MAKEWORD(lpBuf[0x43], lpBuf[0x42]), MAKEWORD(lpBuf[0x41], lpBuf[0x40]));
	mToc.TWOCH_TOC_2_Address = MAKEUINT(MAKEWORD(lpBuf[0x47], lpBuf[0x46]), MAKEWORD(lpBuf[0x45], lpBuf[0x44]));
	mToc.MC_TOC_1_Address = MAKEUINT(MAKEWORD(lpBuf[0x4b], lpBuf[0x4a]), MAKEWORD(lpBuf[0x49], lpBuf[0x48]));
	mToc.MC_TOC_2_Address = MAKEUINT(MAKEWORD(lpBuf[0x4f], lpBuf[0x4e]), MAKEWORD(lpBuf[0x4d], lpBuf[0x4c]));
	mToc.TWOCH_TOC_Len = MAKEWORD(lpBuf[0x55], lpBuf[0x54]);
	mToc.MC_TOC_Len = MAKEWORD(lpBuf[0x57], lpBuf[0x56]);
	mToc.Disc_Date_Y = MAKEWORD(lpBuf[0x79], lpBuf[0x78]);

	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("Master_TOC")
		"\t   Master_TOC_Signature: %.8" CHARWIDTH "s\n"
		"\t           Spec_Version: %d.%02d\n"
		"\t         Album_Set_Size: %u\n"
		"\t  Album_Sequence_Number: %u\n"
		"\t   Album_Catalog_Number: %.16" CHARWIDTH "s\n"
		"\t           Album_Genre1: %02x %02x %02x %02x\n"
		"\t           Album_Genre2: %02x %02x %02x %02x\n"
		"\t           Album_Genre3: %02x %02x %02x %02x\n"
		"\t           Album_Genre4: %02x %02x %02x %02x\n"
		"\t    TWOCH_TOC_1_Address: %u (%#08x)\n"
		"\t    TWOCH_TOC_2_Address: %u (%#08x)\n"
		"\t       MC_TOC_1_Address: %u (%#08x)\n"
		"\t       MC_TOC_2_Address: %u (%#08x)\n"
		"\t      Disc_Flags_Hybrid: %hhu\n"
		"\t          TWOCH_TOC_Len: %u (%#08x)\n"
		"\t             MC_TOC_Len: %u (%#08x)\n"
		"\t    Disc_Catalog_Number: %.16" CHARWIDTH "s\n"
		"\t            Disc_Genre1: %02x %02x %02x %02x\n"
		"\t            Disc_Genre2: %02x %02x %02x %02x\n"
		"\t            Disc_Genre3: %02x %02x %02x %02x\n"
		"\t            Disc_Genre4: %02x %02x %02x %02x\n"
		"\t              Disc_Date: %u-%02u-%02u\n"
		"\tNumber_of_Text_Channels: %u\n"
		, &mToc.Master_TOC_Signature[0]
		, mToc.Major_Version, mToc.Minor_Version
		, mToc.Album_Set_Size
		, mToc.Album_Sequence_Number
		, &mToc.Album_Catalog_Number[0]
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
		, &mToc.Disc_Catalog_Number[0]
		, mToc.Disc_Genre[0][0], mToc.Disc_Genre[0][1], mToc.Disc_Genre[0][2], mToc.Disc_Genre[0][3]
		, mToc.Disc_Genre[1][0], mToc.Disc_Genre[1][1], mToc.Disc_Genre[1][2], mToc.Disc_Genre[1][3]
		, mToc.Disc_Genre[2][0], mToc.Disc_Genre[2][1], mToc.Disc_Genre[2][2], mToc.Disc_Genre[2][3]
		, mToc.Disc_Genre[3][0], mToc.Disc_Genre[3][1], mToc.Disc_Genre[3][2], mToc.Disc_Genre[3][3]
		, mToc.Disc_Date_Y, mToc.Disc_Date_M, mToc.Disc_Date_D
		, mToc.Txt_Ch.Number_of_Text_Channels
	);
	_TCHAR Lang[][14] {
		_T("UNKNOWN"), _T("ISO646"), _T("ISO8859_1"), _T("RIS506")
		, _T("KSC5601"), _T("GB2312"), _T("BIG5"), _T("ISO8859_1_ESC")
	};
	for (INT i = 0; i < mToc.Txt_Ch.Number_of_Text_Channels; i++) {
		OutputVolDescLog(
			"\t     Language_Code[%d]: %.2" CHARWIDTH "s\n"
			"\tCharacter_Set_Code[%d]: %s\n"
			, i + 1, &mToc.Txt_Ch.locale[i].Language_Code[0]
			, i + 1, &Lang[mToc.Txt_Ch.locale[i].Character_Set_Code][0]
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

	for (LBA.AsULong = 511; LBA.AsULong <= 518; LBA.AsULong++) {
		REVERSE_BYTES(&cdb.LogicalBlock, &LBA);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		OutputMainChannel(fileMainInfo, lpBuf, _T("Master_Text"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE);
		Master_Text mText = {};
		mText.Album_Title_Ptr = MAKEWORD(lpBuf[0x11], lpBuf[0x10]);
		mText.Album_Artist_Ptr = MAKEWORD(lpBuf[0x13], lpBuf[0x12]);
		mText.Album_Publisher_Ptr = MAKEWORD(lpBuf[0x15], lpBuf[0x14]);
		mText.Album_Copyright_Ptr = MAKEWORD(lpBuf[0x17], lpBuf[0x16]);
		mText.Album_Title_Phonetic_Ptr = MAKEWORD(lpBuf[0x19], lpBuf[0x18]);
		mText.Album_Artist_Phonetic_Ptr = MAKEWORD(lpBuf[0x1b], lpBuf[0x1a]);
		mText.Album_Publisher_Phonetic_Ptr = MAKEWORD(lpBuf[0x1d], lpBuf[0x1c]);
		mText.Album_Copyright_Phonetic_Ptr = MAKEWORD(lpBuf[0x1f], lpBuf[0x1e]);
		mText.Disc_Title_Ptr = MAKEWORD(lpBuf[0x21], lpBuf[0x20]);
		mText.Disc_Artist_Ptr = MAKEWORD(lpBuf[0x23], lpBuf[0x22]);
		mText.Disc_Publisher_Ptr = MAKEWORD(lpBuf[0x25], lpBuf[0x24]);
		mText.Disc_Copyright_Ptr = MAKEWORD(lpBuf[0x27], lpBuf[0x26]);
		mText.Disc_Title_Phonetic_Ptr = MAKEWORD(lpBuf[0x29], lpBuf[0x28]);
		mText.Disc_Artist_Phonetic_Ptr = MAKEWORD(lpBuf[0x2b], lpBuf[0x2a]);
		mText.Disc_Publisher_Phonetic_Ptr = MAKEWORD(lpBuf[0x2d], lpBuf[0x2c]);
		mText.Disc_Copyright_Phonetic_Ptr = MAKEWORD(lpBuf[0x2f], lpBuf[0x2e]);

		OutputVolDescLog(
			OUTPUT_DHYPHEN_PLUS_STR("Master_Text")
			"\t   Master_Text_Signature: %8" CHARWIDTH "s\n", &lpBuf[0]
		);
		OutputVolDescLog("\t             Album_Title: ");
		if (mText.Album_Title_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Title_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Album_Title_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t            Album_Artist: ");
		if (mText.Album_Artist_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Artist_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Album_Artist_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t         Album_Publisher: ");
		if (mText.Album_Publisher_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Publisher_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Album_Publisher_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t         Album_Copyright: ");
		if (mText.Album_Copyright_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Copyright_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Album_Copyright_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t    Album_Title_Phonetic: ");
		if (mText.Album_Title_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Title_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Album_Title_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t   Album_Artist_Phonetic: ");
		if (mText.Album_Artist_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Artist_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Album_Artist_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\tAlbum_Publisher_Phonetic: ");
		if (mText.Album_Publisher_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Publisher_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Album_Publisher_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\tAlbum_Copyright_Phonetic: ");
		if (mText.Album_Copyright_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Album_Copyright_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Album_Copyright_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t              Disc_Title: ");
		if (mText.Disc_Title_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Title_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Disc_Title_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t             Disc_Artist: ");
		if (mText.Disc_Artist_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Artist_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Disc_Artist_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t          Disc_Publisher: ");
		if (mText.Disc_Publisher_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Publisher_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Disc_Publisher_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t          Disc_Copyright: ");
		if (mText.Disc_Copyright_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Copyright_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Disc_Copyright_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t     Disc_Title_Phonetic: ");
		if (mText.Disc_Title_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Title_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Disc_Title_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t    Disc_Artist_Phonetic: ");
		if (mText.Disc_Artist_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Artist_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Disc_Artist_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t Disc_Publisher_Phonetic: ");
		if (mText.Disc_Publisher_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Publisher_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Disc_Publisher_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLog("\n\t Disc_Copyright_Phonetic: ");
		if (mText.Disc_Copyright_Phonetic_Ptr) {
			for (INT i = 0; lpBuf[mText.Disc_Copyright_Phonetic_Ptr + i] != 0; i++) {
				OutputVolDescLog("%c", lpBuf[mText.Disc_Copyright_Phonetic_Ptr + i]);
			}
		}
		OutputVolDescLog("\n");
	}
	LBA.AsULong = 519;
	REVERSE_BYTES(&cdb.LogicalBlock, &LBA);

	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
		direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		FreeAndNull(pBuf);
		return FALSE;
	}
	OutputMainChannel(fileMainInfo, lpBuf, _T("Manufacture"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE);
	OutputVolDescLog(
		OUTPUT_DHYPHEN_PLUS_STR("Manufacture")
		"\tManuf_Info_Signature: %.8" CHARWIDTH "s\n", &lpBuf[0]);

	ULONG ulChToc[] = { (ULONG)mToc.TWOCH_TOC_1_Address , (ULONG)mToc.MC_TOC_1_Address, 0 };
	for (ULONG c = 0; ulChToc[c] != 0; c++) {
		LBA.AsULong = ulChToc[c];
		REVERSE_BYTES(&cdb.LogicalBlock, &LBA);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		OutputMainChannel(fileMainInfo, lpBuf, _T("Area_TOC"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE);

		typedef struct _Area_TOC {
			CHAR Area_TOC_Signature[8];	// 0..15 A_TOC_0_Header
			BYTE Major_Version;
			BYTE Minor_Version;
			USHORT Area_TOC_Length;
			UINT Reserved1;
			UINT Max_Byte_Rate;			// 16..127 Area_Data 
			BYTE FS_Code;
			BYTE Frame_Format : 4;
			BYTE Reserved2 : 4;
			BYTE Reserved3[10];
			BYTE Number_of_Channels;
			BYTE Loudspeaker_Config : 5;
			BYTE Extra_Setting : 3;
			BYTE Max_Available_Channels;
			BYTE Reserved4 : 3;
			BYTE AMF1 : 1;
			BYTE AMF2 : 1;
			BYTE AMF3 : 1;
			BYTE AMF4 : 1;
			BYTE Reserved5 : 1;
			BYTE Reserved6[12];
			BYTE Track_Attribute : 4;
			BYTE Reserved7 : 4;
			BYTE Reserved8[15];
			BYTE m;
			BYTE s;
			BYTE f;
			BYTE Reserved9;
			BYTE Track_Offset;
			BYTE Number_of_Track;
			BYTE First_Bonus_Track_Number;
			BYTE Reserved10;
			UINT Track_Area_Start_Address;
			UINT Track_Area_End_Address;
			Text_Channel Txt_Ch;
			BYTE Reserved11[8];
			USHORT Track_Text_Ptr;		// 128..143 List_Pointers
			USHORT Index_List_Ptr;
			USHORT Access_List_Ptr;
			USHORT Track_WebLink_List_Ptr;
			USHORT Track_List_3_Ptr;
			USHORT Set_Of_Playlists_Ptr;
			BYTE Reserved12[4];
			BYTE Area_Text[1904];
		} Area_TOC;
		Area_TOC aToc = {};
		memcpy(&aToc, lpBuf, sizeof(Area_TOC));
		aToc.Max_Byte_Rate = MAKEUINT(MAKEWORD(lpBuf[0x13], lpBuf[0x12]), MAKEWORD(lpBuf[0x11], lpBuf[0x10]));
		aToc.Area_TOC_Length = MAKEWORD(lpBuf[0xb], lpBuf[0xa]);
		aToc.Track_Area_Start_Address = MAKEUINT(MAKEWORD(lpBuf[0x4b], lpBuf[0x4a]), MAKEWORD(lpBuf[0x49], lpBuf[0x48]));
		aToc.Track_Area_End_Address = MAKEUINT(MAKEWORD(lpBuf[0x4f], lpBuf[0x4e]), MAKEWORD(lpBuf[0x4d], lpBuf[0x4c]));

		OutputVolDescLog(
			OUTPUT_DHYPHEN_PLUS_STR("Area_TOC")
			"\t      Area_TOC_Signature: %.8" CHARWIDTH "s\n"
			"\t            Spec_Version: %d.%02d\n"
			"\t         Area_TOC_Length: %u (%#02x)\n"
			"\t           Max_Byte_Rate: %u\n"
			"\t                 FS_Code: %u\n"
			"\t            Frame_Format: %d\n"
			"\t      Number_of_Channels: %d\n"
			"\t      Loudspeaker_Config: %d\n"
			"\t           Extra_Setting: %d\n"
			"\t  Max_Available_Channels: %d\n"
			"\t                    AMF1: %d\n"
			"\t                    AMF2: %d\n"
			"\t                    AMF3: %d\n"
			"\t                    AMF4: %d\n"
			"\t         Track_Attribute: %d\n"
			"\t               Total MSF: %02d:%02d:%02d\n"
			"\t            Track_Offset: %d\n"
			"\t         Number_of_Track: %d\n"
			"\tFirst_Bonus_Track_Number: %d\n"
			"\tTrack_Area_Start_Address: %u (%#02x)\n"
			"\t  Track_Area_End_Address: %u (%#02x)\n"
			"\t       Max_Text_Channels: %d\n"
			, &lpBuf[0]
			, aToc.Major_Version, aToc.Minor_Version
			, aToc.Area_TOC_Length, aToc.Area_TOC_Length
			, aToc.Max_Byte_Rate
			, aToc.FS_Code
			, aToc.Frame_Format
			, aToc.Number_of_Channels
			, aToc.Loudspeaker_Config
			, aToc.Extra_Setting
			, aToc.Max_Available_Channels
			, aToc.AMF1, aToc.AMF2, aToc.AMF3, aToc.AMF4
			, aToc.Track_Attribute
			, aToc.m, aToc.s, aToc.f
			, aToc.Track_Offset
			, aToc.Number_of_Track
			, aToc.First_Bonus_Track_Number
			, aToc.Track_Area_Start_Address, aToc.Track_Area_Start_Address
			, aToc.Track_Area_End_Address, aToc.Track_Area_End_Address
			, aToc.Txt_Ch.Number_of_Text_Channels
		);
		for (INT i = 0; i < aToc.Txt_Ch.Number_of_Text_Channels; i++) {
			OutputVolDescLog(
				"\t      Language_Code[%d]: %.2" CHARWIDTH "s\n"
				"\t Character_Set_Code[%d]: %s\n"
				, i + 1, &aToc.Txt_Ch.locale[i].Language_Code[0]
				, i + 1, &Lang[aToc.Txt_Ch.locale[i].Character_Set_Code][0]
			);
		}
		aToc.Track_Text_Ptr = MAKEWORD(lpBuf[0x81], lpBuf[0x80]);
		aToc.Index_List_Ptr = MAKEWORD(lpBuf[0x83], lpBuf[0x82]);
		aToc.Access_List_Ptr = MAKEWORD(lpBuf[0x85], lpBuf[0x84]);
		aToc.Track_WebLink_List_Ptr = MAKEWORD(lpBuf[0x87], lpBuf[0x86]);
		OutputVolDescLog(
			"\t        Track_Text_Ptr: %u\n"
			"\t        Index_List_Ptr: %u\n"
			"\t       Access_List_Ptr: %u\n"
			"\tTrack_WebLink_List_Ptr: %u\n"
			"\t      Track_List_3_Ptr: %u\n"
			"\t  Set_Of_Playlists_Ptr: %u\n"
			, aToc.Track_Text_Ptr
			, aToc.Index_List_Ptr
			, aToc.Access_List_Ptr
			, aToc.Track_WebLink_List_Ptr
			, aToc.Track_List_3_Ptr
			, aToc.Set_Of_Playlists_Ptr
		);

		for (INT i = 0; i < aToc.Txt_Ch.Number_of_Text_Channels; i++) {
			WORD AD_Ptr = MAKEWORD(aToc.Area_Text[1 + i * 8], aToc.Area_Text[0 + i * 8]);
			WORD AC_Ptr = MAKEWORD(aToc.Area_Text[3 + i * 8], aToc.Area_Text[2 + i * 8]);
			WORD ADP_Ptr = MAKEWORD(aToc.Area_Text[5 + i * 8], aToc.Area_Text[4 + i * 8]);
			WORD ACP_Ptr = MAKEWORD(aToc.Area_Text[7 + i * 8], aToc.Area_Text[6 + i * 8]);
			OutputVolDescLog(
				"\t         Area_Description_Ptr[%d]: %u\n"
				"\t           Area_Copyright_Ptr[%d]: %u\n"
				"\tArea_Description_Phonetic_Ptr[%d]: %u\n"
				"\t  Area_Copyright_Phonetic_Ptr[%d]: %u\n"
				, i + 1, AD_Ptr
				, i + 1, AC_Ptr
				, i + 1, ADP_Ptr
				, i + 1, ACP_Ptr
			);
			if (AD_Ptr) {
				OutputVolDescLog("\t         Area_Description: %s\n", &aToc.Area_Text[AD_Ptr]);
			}
			if (AC_Ptr) {
				OutputVolDescLog("\t          Area_Copyrightn: %s\n", &aToc.Area_Text[AC_Ptr]);
			}
			if (ADP_Ptr) {
				OutputVolDescLog("\tArea_Description_Phonetic: %s\n", &aToc.Area_Text[ADP_Ptr]);
			}
			if (ACP_Ptr) {
				OutputVolDescLog("\t  Area_Copyright_Phonetic: %s\n", &aToc.Area_Text[ACP_Ptr]);
			}
		}

		LBA.AsULong = ulChToc[c] + 1;
		REVERSE_BYTES(&cdb.LogicalBlock, &LBA);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		OutputMainChannel(fileMainInfo, lpBuf, _T("Track_List_1"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE);

		typedef struct _Track_List_1 {
			CHAR Track_List_1_Signature[8];
			UINT Track_Start_Address[255];
			UINT Track_Length[255];
		} Track_List_1;
		Track_List_1 tList1 = {};

		OutputVolDescLog(
			OUTPUT_DHYPHEN_PLUS_STR("Track_List_1")
			"\t Track_List_1_Signature: %.8" CHARWIDTH "s\n"
			, &lpBuf[0]
		);
		for (INT i = 0; i < aToc.Number_of_Track; i++) {
			tList1.Track_Start_Address[i] = MAKEUINT(
				MAKEWORD(lpBuf[0xb + sizeof(UINT) * i], lpBuf[0xa + sizeof(UINT) * i])
				, MAKEWORD(lpBuf[0x9 + sizeof(UINT) * i], lpBuf[0x8 + sizeof(UINT) * i]));
			OutputVolDescLog(
				"\tTrack_Start_Address[%02d]: %9u (%#x)\n"
				, i + 1, tList1.Track_Start_Address[i], tList1.Track_Start_Address[i]
			);
		}
		for (INT i = 0; i < aToc.Number_of_Track; i++) {
			tList1.Track_Length[i] = MAKEUINT(
				MAKEWORD(lpBuf[0x407 + sizeof(UINT) * i], lpBuf[0x406 + sizeof(UINT) * i])
				, MAKEWORD(lpBuf[0x405 + sizeof(UINT) * i], lpBuf[0x404 + sizeof(UINT) * i]));
			OutputVolDescLog(
				"\t       Track_Length[%02d]: %9u (%#x)\n"
				, i + 1, tList1.Track_Length[i], tList1.Track_Length[i]
			);
		}
		LBA.AsULong = ulChToc[c] + 2;
		REVERSE_BYTES(&cdb.LogicalBlock, &LBA);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		OutputMainChannel(fileMainInfo, lpBuf, _T("Track_List_2"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE);

		typedef struct _Track_Start_Time {
			BYTE m;
			BYTE s;
			BYTE f;
			BYTE Track_Mode;
		} Track_Start_Time;
		typedef struct _Track_Time_Length {
			BYTE m;
			BYTE s;
			BYTE f;
			BYTE Track_Flags;
		} Track_Time_Length;
		typedef struct _Track_List_2 {
			CHAR Track_List_2_Signature[8];
			Track_Start_Time Start_Time[255];
			Track_Time_Length Time_Length[255];
		} Track_List_2;
		Track_List_2 tList2 = {};
		memcpy(&tList2, lpBuf, DISC_MAIN_DATA_SIZE);

		OutputVolDescLog(
			OUTPUT_DHYPHEN_PLUS_STR("Track_List_2")
			"\t   Track_List_2_Signature: %.8" CHARWIDTH "s\n"
			, &lpBuf[0]
		);
		for (INT i = 0; i < aToc.Number_of_Track; i++) {
			OutputVolDescLog(
				"\tTrack_Start_Time_Code[%02d]: %02d:%02d:%02d, Track_Mode[%02d]: %02d\n"
				, i + 1, tList2.Start_Time[i].m, tList2.Start_Time[i].s, tList2.Start_Time[i].f
				, i + 1, tList2.Start_Time[i].Track_Mode
			);
		}
		for (INT i = 0; i < aToc.Number_of_Track; i++) {
			OutputVolDescLog(
				"\t    Track_Time_Length[%02d]: %02d:%02d:%02d, Track_Flags[%02d]: %02d\n"
				, i + 1, tList2.Time_Length[i].m, tList2.Time_Length[i].s, tList2.Time_Length[i].f
				, i + 1, tList2.Time_Length[i].Track_Flags
			);
		}

		LBA.AsULong = ulChToc[c] + 3;
		REVERSE_BYTES(&cdb.LogicalBlock, &LBA);
		cdb.TransferLength[3] = 2;

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
			direction, DISC_MAIN_DATA_SIZE * 2, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			FreeAndNull(pBuf);
			return FALSE;
		}
		OutputMainChannel(fileMainInfo, lpBuf, _T("ISRC_and_Genre_List"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE * 2);

		OutputVolDescLog(
			OUTPUT_DHYPHEN_PLUS_STR("ISRC_and_Genre_List")
			"\tISRC_and_Genre_List_Signature: %.8" CHARWIDTH "s\n"
			, &lpBuf[0]
		);
		for (INT i = 0; i < aToc.Number_of_Track; i++) {
			OutputVolDescLog(
				"\t           ISRC_and_Genre[%02d]: %.12" CHARWIDTH "s\n"
				, i + 1, &lpBuf[0x8 + 12 * i]
			);
		}

		if (aToc.Access_List_Ptr) {
			LBA.AsULong = ulChToc[c] + aToc.Access_List_Ptr;
			REVERSE_BYTES(&cdb.LogicalBlock, &LBA);
			cdb.TransferLength[3] = 32;

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_MAIN_DATA_SIZE * 32, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				FreeAndNull(pBuf);
				return FALSE;
			}
			OutputMainChannel(fileMainInfo, lpBuf, _T("Access_List"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE * 32);
#pragma pack(push, acc, 1)
			typedef struct _Main_Acc_List {
				USHORT Access_Flags;
				BYTE Entrys[3];
			} Main_Acc_List;
			typedef struct _Detailed_Access {
				BYTE Sub_Entrys[9][3];
			} Detailed_Access;
			typedef struct _Access_List {
				CHAR Access_List_Signature[8];
				USHORT N_Entries;
				BYTE Main_Step_Size;
				BYTE Reserved[5];
				Main_Acc_List mAList[6550];
				BYTE Reserved2[2];
				Detailed_Access dAccess[1213];
				BYTE Reserved3[16];
			} Access_List;
#pragma pack(pop, acc)
			Access_List alist = {};
			memcpy(&alist, lpBuf, sizeof(Access_List));
			alist.N_Entries = MAKEWORD(lpBuf[0x9], lpBuf[0x8]);

			OutputVolDescLog(
				OUTPUT_DHYPHEN_PLUS_STR("Access_List")
				"\tAccess_List_Signature: %.8" CHARWIDTH "s\n"
				"\t            N_Entries: %d\n"
				"\t       Main_Step_Size: %d\n"
				, &lpBuf[0]
				, alist.N_Entries
				, alist.Main_Step_Size
			);
			for (INT i = 0; i < alist.N_Entries; i++) {
				INT detailed = alist.mAList[i].Access_Flags & 0x0001;
				OutputVolDescLog("\tDetailed: %d, ", detailed);
				if (detailed) {
					OutputVolDescLog(
						"Detailed_List_Ptr: %5d, "
						, alist.mAList[i].Access_Flags >> 1 & 0x7fff
					);
				}
				else {
					OutputVolDescLog(
						"    Access_Margin: %5d, "
						, alist.mAList[i].Access_Flags >> 1 & 0x7fff
					);
				}
				OutputVolDescLog(
					"Entrys[%04d]: %02x %02x %02x\n"
					, i + 1, alist.mAList[i].Entrys[0], alist.mAList[i].Entrys[1], alist.mAList[i].Entrys[2]
				);
			}
			for (INT i = 0, m = 0; i < alist.N_Entries; i++) {
				INT detailed = alist.mAList[i].Access_Flags & 0x0001;
				INT Access_Margin = alist.mAList[i].Access_Flags >> 1 & 0x7fff;
				if (detailed || (!detailed && Access_Margin != 0 && Access_Margin != 128 && Access_Margin != 256)) {
					for (INT j = 0; j < 9; j++) {
						OutputVolDescLog(
							"\tEntrys[%04d] Sub_Entrys[%04d][%02d]: %02x %02x %02x\n"
							, i + 1, m + 1, j + 1, alist.dAccess[m].Sub_Entrys[j][0]
							, alist.dAccess[m].Sub_Entrys[j][1], alist.dAccess[m].Sub_Entrys[j][2]
						);
					}
					m++;
				}
			}
		}

		if (aToc.Track_Text_Ptr) {
			LBA.AsULong = ulChToc[c] + aToc.Track_Text_Ptr;
			REVERSE_BYTES(&cdb.LogicalBlock, &LBA);
			cdb.TransferLength[3] = 1;

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_MAIN_DATA_SIZE, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				FreeAndNull(pBuf);
				return FALSE;
			}
			OutputMainChannel(fileMainInfo, lpBuf, _T("Track_Text_Item_Ptr"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE);

			typedef struct _Track_Text {
				CHAR Track_Text_Signature[8];
				USHORT Track_Text_Item_Ptr[8][255];
			} Track_Text;

			Track_Text TTxt = {};
			OutputVolDescLog(
				OUTPUT_DHYPHEN_PLUS_STR("Track_Text")
				"\t      Track_Text_Signature: %.8" CHARWIDTH "s\n"
				, &lpBuf[0]
			);
			for (INT h = 0; h < aToc.Txt_Ch.Number_of_Text_Channels; h++) {
				for (INT i = 0; i < aToc.Number_of_Track; i++) {
					TTxt.Track_Text_Item_Ptr[h][i] = MAKEWORD(lpBuf[0x9 + sizeof(USHORT) * i], lpBuf[0x8 + sizeof(USHORT) * i]);
					OutputVolDescLog(
						"\tTrack_Text_Item_Ptr[%d][%02d]: %d (%#x)\n"
						, h + 1, i + 1, TTxt.Track_Text_Item_Ptr[h][i], TTxt.Track_Text_Item_Ptr[h][i]
					);
				}
			}
			INT track_text_size = TTxt.Track_Text_Item_Ptr[aToc.Txt_Ch.Number_of_Text_Channels - 1][aToc.Number_of_Track - 1] - TTxt.Track_Text_Item_Ptr[0][0];
			INT track_text_sector = track_text_size / DISC_MAIN_DATA_SIZE;
			if (track_text_size == 0 || track_text_size % DISC_MAIN_DATA_SIZE) {
				track_text_sector++;
			}
			LBA.AsULong = ulChToc[c] + aToc.Track_Text_Ptr + (TTxt.Track_Text_Item_Ptr[0][0] / DISC_MAIN_DATA_SIZE);
			REVERSE_BYTES(&cdb.LogicalBlock, &LBA);
			cdb.TransferLength[3] = (UCHAR)track_text_sector;

			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				direction, DISC_MAIN_DATA_SIZE * (DWORD)track_text_sector, &byScsiStatus, _T(__FUNCTION__), __LINE__, TRUE)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				FreeAndNull(pBuf);
				return FALSE;
			}
			OutputMainChannel(fileMainInfo, lpBuf, _T("Track_Text"), (INT)LBA.AsULong, DISC_MAIN_DATA_SIZE * (DWORD)track_text_sector);

			for (int h = 0; h < aToc.Txt_Ch.Number_of_Text_Channels; h++) {
				int nOfs = 0;
				for (int i = 0; i < aToc.Number_of_Track; i++) {
					if (TTxt.Track_Text_Item_Ptr[h][i]) {
						OutputVolDescLog("\t     Number_of_Item[%d][%02d]: %02d\n"
							, h + 1, i + 1, lpBuf[nOfs]);
						int n_item = lpBuf[nOfs];
						nOfs += 4;
						for (int j = 0; j < n_item; j++) {
							switch (lpBuf[nOfs]) {
							case 1:
								OutputVolDescLog("\t              Title[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 2:
								OutputVolDescLog("\t          Performer[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 3:
								OutputVolDescLog("\t         Songwriter[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 4:
								OutputVolDescLog("\t           Composer[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 5:
								OutputVolDescLog("\t           Arranger[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 6:
								OutputVolDescLog("\t            Message[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 7:
								OutputVolDescLog("\t      Extra Message[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 8:
								OutputVolDescLog("\t          Copyright[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 129:
								OutputVolDescLog("\t        Title, phonetic text[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 130:
								OutputVolDescLog("\t    Performer, phonetic text[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 131:
								OutputVolDescLog("\t   Songwriter, phonetic text[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 132:
								OutputVolDescLog("\t     Composer, phonetic text[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 133:
								OutputVolDescLog("\t     Arranger, phonetic text[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 134:
								OutputVolDescLog("\t      Message, phonetic text[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 135:
								OutputVolDescLog("\tExtra Message, phonetic text[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							case 136:
								OutputVolDescLog("\t    Copyright, phonetic text[%d][%02d]: %s\n"
									, h + 1, i + 1, &lpBuf[nOfs + 2]);
								break;
							default:
								OutputVolDescLog("Reserved\n");
								break;
							}
							nOfs += 2;
							nOfs += strlen((const char*)&lpBuf[nOfs]) + 1;
							if (nOfs % 4) {
								nOfs += 4 - nOfs % 4;
							}
						}
						nOfs = TTxt.Track_Text_Item_Ptr[h][i + 1] - TTxt.Track_Text_Item_Ptr[h][0];
					}
				}
			}
		}
	}
	FreeAndNull(lpBuf);
	return TRUE;
}
