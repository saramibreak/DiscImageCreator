/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execScsiCmd.h"
#include "execScsiCmdforCD.h"
#include "output.h"
#include "outputScsiCmdLogforCD.h"
#include "set.h"
#include "calcHash.h"

// These global variable is declared at DiscImageCreator.cpp
extern BYTE g_aSyncHeader[SYNC_SIZE];
// This static variable is set if function is error
static LONG s_lineNum;

BOOL IsValidMainDataHeader(
	LPBYTE lpBuf
) {
	BOOL bRet = TRUE;
	for (INT c = 0; c < sizeof(g_aSyncHeader); c++) {
		if (lpBuf[c] != g_aSyncHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL IsValid3doDataHeader(
	LPBYTE lpBuf
) {
	// judge it from the 1st sector(=LBA 0).
	BOOL bRet = TRUE;
	CONST BYTE a3doHeader[] = {
		0x01, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x01, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x43, 0x44, 0x2d, 0x52, 0x4f, 0x4d, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	INT i = 0;
	for (INT c = 0; c < sizeof(a3doHeader); i++, c++) {
		if (lpBuf[i] != a3doHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	if (bRet) {
		for (i = 132; i < 2048; i += 8) {
			if (strncmp((CHAR*)&lpBuf[i], "duck", 4) &&
				strncmp((CHAR*)&lpBuf[i + 4], "iama", 4)) {
				bRet = FALSE;
				break;
			}
		}
	}
	return bRet;
}

// http://d.hatena.ne.jp/zariganitosh/20130501/hfs_plus_struct
// http://www.opensource.apple.com/source/xnu/xnu-2050.18.24/bsd/hfs/hfs_format.h	
BOOL IsValidMacDataHeader(
	LPBYTE lpBuf
) {
	// judge it from the 2nd sector(=LBA 1).
	BOOL bRet = TRUE;
	if (lpBuf[0] != 0x42 || lpBuf[1] != 0x44) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidPceSector(
	LPBYTE lpBuf
) {
	BOOL bRet = TRUE;
	CONST BYTE warningStr[] = {
		0x82, 0xb1, 0x82, 0xcc, 0x83, 0x76, 0x83, 0x8d,
		0x83, 0x4f, 0x83, 0x89, 0x83, 0x80, 0x82, 0xcc,
		0x92, 0x98, 0x8d, 0xec, 0x8c, 0xa0, 0x82, 0xcd,
		0x8a, 0x94, 0x8e, 0xae, 0x89, 0xef, 0x8e, 0xd0,
		0x00, 0x83, 0x6e, 0x83, 0x68, 0x83, 0x5c, 0x83,
		0x93, 0x82, 0xaa, 0x8f, 0x8a, 0x97, 0x4c, 0x82,
		0xb5, 0x82, 0xc4, 0x82, 0xa8, 0x82, 0xe8, 0x82
	};
	for (INT i = 0; i < sizeof(warningStr); i++) {
		if (lpBuf[i] != warningStr[i]) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

BOOL IsValidPcfxSector(
	LPBYTE lpBuf
) {
	BOOL bRet = TRUE;
	if (strncmp((LPCH)&lpBuf[0], "PC-FX:Hu_CD-ROM", 15)) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidPlextorDrive(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szVendorId, "PLEXTOR ", DRIVE_VENDER_ID_SIZE)) {
		if (!strncmp(pDevice->szProductId, "DVDR   PX-760A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX760A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-755A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX755A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-716AL ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX716AL;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-716A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX716A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-714A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX714A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-712A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX712A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-708A2 ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX708A2;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-708A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX708A;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-704A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX704A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-320A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX320A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PREMIUM2 ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PREMIUM2;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PREMIUM  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PREMIUM;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W5224A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW5224A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4824A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW4824A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4012A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW4012A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4012S", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW4012S;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W2410A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW2410A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-S88T  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXS88T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W1610A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW1610A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W1210A", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW1210A;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W1210S", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW1210S;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W124TS", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW124TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W8432T", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW8432T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W8220T", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW8220T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4220T", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW4220T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-R820T ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXR820T;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-R412C ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXR412C;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-40TS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX40TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-40TSUW", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX40TSUW;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-40TW  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX40TW;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-32TS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX32TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-32CS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX32CS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-20TS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX20TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-12TS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX12TS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-12CS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX12CS;
		}
		else if (!strncmp(pDevice->szProductId, "CD-ROM PX-8XCS  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX8XCS;
		}
		else {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::No;
		}
	}
	return TRUE;
}

BOOL IsValidPregapSector(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA
) {
	BOOL bRet = FALSE;
	if ((pSubQ->present.byCtl & AUDIO_DATA_TRACK) == 0 &&
		(pSubQ->next.byCtl & AUDIO_DATA_TRACK) == 0 &&
		pSubQ->next.byIndex == 0) {
		if (*pExecType != gd &&
			nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 225 ||
			nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 150 ||
			nLBA == pDisc->SCSI.lpFirstLBAListOnToc[pSubQ->prev.byTrackNum] - 149) {
			bRet = TRUE;
		}
		else if (*pExecType == gd &&
			nLBA == pDisc->GDROM_TOC.TrackData[pSubQ->prev.byTrackNum].Address - 225 ||
			nLBA == pDisc->GDROM_TOC.TrackData[pSubQ->prev.byTrackNum].Address - 150 ||
			nLBA == pDisc->GDROM_TOC.TrackData[pSubQ->prev.byTrackNum].Address - 149) {
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL IsValidLibCryptSector(
	BOOL bLibCrypt,
	INT nLBA
) {
	BOOL bRet = FALSE;
	if (bLibCrypt) {
		if ((14100 <= nLBA && nLBA < 16200) || (42000 <= nLBA && nLBA < 44400)) {
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL IsValidIntentionalSubSector(
	BOOL bIntentionalSub,
	PDISC pDisc,
	INT nLBA
) {
	BOOL bRet = FALSE;
	if (bIntentionalSub) {
		if (pDisc->PROTECT.byExist == securomV1) {
			if (30800 <= nLBA && nLBA < 34800) {
				bRet = TRUE;
			}
			else if (40000 <= nLBA && nLBA < 45800) {
				bRet = TRUE;
			}
		}
		else if (pDisc->PROTECT.byExist == securomV3) {
			if (0 <= nLBA && nLBA < 8 || 5000 <= nLBA && nLBA < 18800) {
				bRet = TRUE;
			}
		}
		else if (pDisc->PROTECT.byExist == securomV2 ||
			pDisc->PROTECT.byExist == securomV4) {
			if (5000 <= nLBA && nLBA < 18800) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQCtl(
	PSUB_Q pSubQ,
	BYTE byEndCtl
) {
	BOOL bRet = TRUE;
	switch (pSubQ->present.byCtl) {
	case 0:
		break;
	case AUDIO_WITH_PREEMPHASIS:
		break;
	case DIGITAL_COPY_PERMITTED:
		break;
	case DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		break;
	case AUDIO_DATA_TRACK:
		break;
	case AUDIO_DATA_TRACK | DIGITAL_COPY_PERMITTED:
		break;
	case TWO_FOUR_CHANNEL_AUDIO:
		break;
	case TWO_FOUR_CHANNEL_AUDIO | AUDIO_WITH_PREEMPHASIS:
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED:
		break;
	case TWO_FOUR_CHANNEL_AUDIO | DIGITAL_COPY_PERMITTED | AUDIO_WITH_PREEMPHASIS:
		break;
	default:
		s_lineNum = __LINE__;
		return FALSE;
	}

	if (pSubQ->prev.byCtl != pSubQ->present.byCtl) {
		if ((pSubQ->present.byCtl != byEndCtl) && pSubQ->present.byCtl != 0) {
			s_lineNum = __LINE__;
			bRet = FALSE;
		}
		else {
			if (pSubQ->present.byAdr == ADR_ENCODES_CURRENT_POSITION) {
				if (pSubQ->prev.byTrackNum + 1 == pSubQ->present.byTrackNum) {
					if (pSubQ->prev.nRelativeTime + 1 == pSubQ->present.nRelativeTime) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
				}
				else if (pSubQ->prev.byTrackNum == pSubQ->present.byTrackNum &&
					pSubQ->prev.byIndex == pSubQ->present.byIndex) {
					if (pSubQ->prev.nRelativeTime + 1 == pSubQ->present.nRelativeTime) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
				}
			}
			// EVE - burst error (Disc 3) (Terror Disc)
			// LBA[188021, 0x2DE75],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:71], AMSF[41:48:71], RtoW[0, 0, 0, 0]
			// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :72], RtoW[0, 0, 0, 0]
			// LBA[188023, 0x2DE77],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:73], AMSF[41:48:73], RtoW[0, 0, 0, 0]
			else if (pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			if (pSubQ->prevPrev.byCtl == pSubQ->present.byCtl) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQIdx(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA,
	BYTE byCurrentTrackNum,
	LPBOOL bPrevIndex,
	LPBOOL bPrevPrevIndex
) {
	if (nLBA < 1) {
		return TRUE;
	}
	else if (MAXIMUM_NUMBER_INDEXES < pSubQ->present.byIndex) {
		s_lineNum = __LINE__;
		return FALSE;
	}
	BOOL bRet = TRUE;
	if (pSubQ->prev.byIndex != pSubQ->present.byIndex) {
		if (nLBA != pDisc->SCSI.lpFirstLBAListOnToc[byCurrentTrackNum]) {
			if (pSubQ->prev.byIndex + 1 < pSubQ->present.byIndex) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pSubQ->next.byTrackNum > 0 && 
				pSubQ->present.byIndex - 1 == pSubQ->next.byIndex &&
				pSubQ->prev.byIndex == pSubQ->next.byIndex &&
				pSubQ->next.byAdr == ADR_ENCODES_CURRENT_POSITION) {
				// 1552 Tenka Tairan
				// LBA[126959, 0x1efef], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[01], RMSF[01:50:13], AMSF[28:14:59], RtoW[0, 0, 0, 0]
				// LBA[126960, 0x1eff0], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[02], RMSF[01:50:14], AMSF[28:14:60], RtoW[0, 0, 0, 0]
				// LBA[126961, 0x1eff1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[01], RMSF[01:50:15], AMSF[28:14:61], RtoW[0, 0, 0, 0]
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pSubQ->nextNext.byTrackNum > 0 &&
				pSubQ->prev.byIndex + 1 == pSubQ->present.byIndex &&
				pSubQ->prev.byIndex == pSubQ->nextNext.byIndex) {
				// Super Schwarzschild 2 (Japan)
				// LBA[234845, 0x3955D], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RMSF[01:50:09], AMSF[52:13:20], RtoW[0, 0, 0, 0]
				// LBA[234846, 0x3955E], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[02], RMSF[01:50:10], AMSF[52:13:21], RtoW[0, 0, 0, 0]
				// LBA[234847, 0x3955F], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :22], RtoW[0, 0, 0, 0]
				// LBA[234848, 0x39560], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RMSF[01:50:12], AMSF[52:13:23], RtoW[0, 0, 0, 0]
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else {
				bRet = FALSE;
				// first sector on TOC
				if (pSubQ->prev.byIndex == 0 && pSubQ->present.byIndex == 1 &&
					pSubQ->present.nRelativeTime == 0) {
					bRet = TRUE;
				}
				else if ((pSubQ->present.byIndex == 0 || pSubQ->present.byIndex == 1) &&
					pSubQ->prev.byTrackNum + 1 == pSubQ->present.byTrackNum &&
					pSubQ->present.nRelativeTime < pSubQ->prev.nRelativeTime) {
					bRet = TRUE;
				}
				// multi index sector
				else if (pSubQ->prev.byIndex + 1 == pSubQ->present.byIndex &&
					pSubQ->prev.nRelativeTime + 1 == pSubQ->present.nRelativeTime) {
					bRet = TRUE;
				}
				// first pregap sector
				else if (pSubQ->prev.byIndex == 1 && pSubQ->present.byIndex == 0) {
					if (pSubQ->present.nRelativeTime - 1 == pSubQ->next.nRelativeTime) {
						bRet = TRUE;
					}
					// Shanghai - Matekibuyu (Japan)
					// LBA[016447, 0x0403f], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[01], RMSF[00:16:11], AMSF[03:41:22], RtoW[0, 0, 0, 0]
					// LBA[016448, 0x04040], Audio, 2ch, Copy NG, Pre-emphasis No, Track[05], Idx[00], RMSF[00:21:74], AMSF[03:41:23], RtoW[0, 0, 0, 0]
					// LBA[016449, 0x04041], Audio, 2ch, Copy NG, Pre-emphasis No, Track[05], Idx[00], RMSF[00:01:73], AMSF[03:41:24], RtoW[0, 0, 0, 0]
					else if (IsValidPregapSector(pExecType, pDisc, pSubQ, nLBA)) {
						bRet = TRUE;
					}
				}
				if (pSubQ->prevPrev.byIndex == pSubQ->present.byIndex) {
					bRet = TRUE;
					if (pSubQ->prev.byIndex - 1 == pSubQ->present.byIndex) {
						*bPrevIndex = FALSE;
					}
				}
			}
		}
	}
	else if (pSubQ->prev.byIndex == pSubQ->present.byIndex &&
		pSubQ->prevPrev.byIndex - 1 == pSubQ->present.byIndex &&
		pSubQ->prevPrev.byTrackNum + 1 != pSubQ->present.byTrackNum) {
		*bPrevPrevIndex = FALSE;
	}
	return bRet;
}

BOOL IsValidSubQTrack(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	PSUB_Q pSubQ,
	INT nLBA,
	BYTE byCurrentTrackNum,
	LPBOOL bPrevTrackNum
) {
	if (*pExecType == gd) {
		if (pDisc->GDROM_TOC.LastTrack < pSubQ->present.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
	}
	else {
		if (pDisc->SCSI.toc.LastTrack < pSubQ->present.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
		else if (pSubQ->next.byAdr == ADR_NO_MODE_INFORMATION &&
			pSubQ->next.byTrackNum > 0 &&
			pSubQ->next.byTrackNum < pSubQ->present.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
	}
	BOOL bRet = TRUE;
	if (pSubQ->prev.byTrackNum != pSubQ->present.byTrackNum) {
		if (pSubQ->prev.byTrackNum + 2 <= pSubQ->present.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
		else if (pSubQ->present.byTrackNum < pSubQ->prev.byTrackNum) {
			if (pSubQ->prevPrev.byTrackNum == pSubQ->present.byTrackNum) {
				*bPrevTrackNum = FALSE;
			}
			else {
				s_lineNum = __LINE__;
				return FALSE;
			}
		}
		else {
			if (pSubQ->prev.byTrackNum + 1 == pSubQ->present.byTrackNum) {
				if (*pExecType != gd && nLBA != pDisc->SCSI.lpFirstLBAListOnToc[byCurrentTrackNum]) {
					// Super CD-ROM^2 Taiken Soft Shuu (Japan)
					// LBA[139289, 0x22019], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:00], AMSF[30:59:14], RtoW[0, 0, 0, 0]
					// LBA[139290, 0x2201a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:01], AMSF[30:59:15], RtoW[0, 0, 0, 0]
					// LBA[139291, 0x2201b], Audio, 2ch, Copy NG, Pre-emphasis No, Track[17], Idx[01], RMSF[01:19:02], AMSF[30:59:16], RtoW[0, 0, 0, 0]
					// LBA[139292, 0x2201c], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:03], AMSF[30:59:17], RtoW[0, 0, 0, 0]
					if ((pSubQ->prev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
						pSubQ->prev.nRelativeTime + 1 == pSubQ->present.nRelativeTime) ||
						(pSubQ->prevPrev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
							pSubQ->prevPrev.nRelativeTime + 2 == pSubQ->present.nRelativeTime)) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
					else if (pSubQ->next.byAdr == ADR_ENCODES_CURRENT_POSITION && pSubQ->next.byTrackNum > 0) {
						// Ys III (Japan)
						// LBA[226292, 0x373f4], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[01], RMSF[00:37:62], AMSF[50:19:17], RtoW[0, 0, 0, 0]
						// LBA[226293, 0x373f5], Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RMSF[00:37:63], AMSF[50:19:18], RtoW[0, 0, 0, 0]
						// LBA[226294, 0x373f6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[01], RMSF[00:37:64], AMSF[50:19:19], RtoW[0, 0, 0, 0]
						if (pSubQ->present.byTrackNum != pSubQ->next.byTrackNum) {
							// Sega Flash Vol. 3 (Eu)
							// LBA[221184, 0x36000], Audio, 2ch, Copy NG, Pre-emphasis No, Track[30], Idx[01], RMSF[00:05:00], AMSF[49:11:09], RtoW[0, 0, 0, 0]
							// LBA[221185, 0x36001], Audio, 2ch, Copy NG, Pre-emphasis No, Track[31], Idx[01], RMSF[00:01:74], AMSF[49:11:10], RtoW[0, 0, 0, 0]
							// LBA[221186, 0x36002], Audio, 2ch, Copy NG, Pre-emphasis No, Track[11], Idx[01], RMSF[00:01:73], AMSF[49:11:11], RtoW[0, 0, 0, 0]
							// LBA[221187, 0x36003], Audio, 2ch, Copy NG, Pre-emphasis No, Track[31], Idx[00], RMSF[00:01:72], AMSF[49:11:12], RtoW[0, 0, 0, 0]
							if (pSubQ->prev.byTrackNum != pSubQ->next.byTrackNum) {
								if (pSubQ->nextNext.byAdr == ADR_ENCODES_CURRENT_POSITION && pSubQ->nextNext.byTrackNum > 0) {
									if (pSubQ->present.byTrackNum != pSubQ->nextNext.byTrackNum) {
										s_lineNum = __LINE__;
										bRet = FALSE;
									}
									else {
										OutputSubInfoWithLBALogA(
											"This track num is maybe incorrect. Could you try /s 2 option [L:%d]\n", nLBA, byCurrentTrackNum, (INT)__LINE__);
									}
								}
								else {
									BOOL bPrevIndex, bPrevPrevIndex = FALSE;
									BOOL bIdx = IsValidSubQIdx(pExecType, pDisc, pSubQ, nLBA, byCurrentTrackNum, &bPrevIndex, &bPrevPrevIndex);
									BOOL bCtl = IsValidSubQCtl(pSubQ, pDisc->SUB.lpEndCtlList[pSubQ->present.byTrackNum - 1]);
									// Garou Densetsu Special [PCE]
									// LBA[293944, 0x47c38], Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[02:57:31], AMSF[65:22:48], RtoW[0, 0, 0, 0]
									// LBA[293945, 0x47c39], Audio, 2ch, Copy NG, Pre-emphasis No, Track[23], Idx[81], RMSF[02:17:12], AMSF[105:22:51], RtoW[0, 0, 0, 0]
									// LBA[293946, 0x47c3a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[82], Idx[01], RMSF[12:57:13], AMSF[64:24:143], RtoW[0, 0, 0, 0]
									// LBA[293947, 0x47c3b], Audio, 2ch, Copy NG, Pre-emphasis No, Track[119], Idx[111], RMSF[57:117:165], AMSF[165:161:121], RtoW[0, 0, 0, 0]
									if (!bIdx || !bCtl) {
										s_lineNum = __LINE__;
										bRet = FALSE;
									}
									else {
										OutputSubInfoWithLBALogA(
											"This track num is maybe incorrect. Could you try /s 2 option [L:%d]\n", nLBA, byCurrentTrackNum, (INT)__LINE__);
									}
								}
							}
							else {
								s_lineNum = __LINE__;
								bRet = FALSE;
							}
						}
					}
					// Godzilla - Rettou Shinkan
					// LBA[125215, 0x1e91f], Audio, 2ch, Copy NG, Pre-emphasis No, Track[13], Idx[01], RelTime[00:54:41], AbsTime[27:51:40], RtoW[0, 0, 0, 0]
					// LBA[125216, 0x1e920], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[00], RelTime[00:01:73], AbsTime[27:51:41], RtoW[0, 0, 0, 0]
					// LBA[125217, 0x1e921], Audio, 2ch, Copy NG, Pre-emphasis No, Media Catalog Number  [0000000000000], AbsTime[     :42], RtoW[0, 0, 0, 0]
					// LBA[125218, 0x1e922], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[00], RelTime[00:01:71], AbsTime[27:51:43], RtoW[0, 0, 0, 0]
					else if ((pSubQ->next.byAdr == ADR_ENCODES_MEDIA_CATALOG ||
						pSubQ->next.byAdr == ADR_ENCODES_ISRC) && pSubQ->nextNext.byTrackNum > 0) {
						if (pSubQ->present.byTrackNum != pSubQ->nextNext.byTrackNum) {
							s_lineNum = __LINE__;
							bRet = FALSE;
						}
					}
					else {
						BOOL bPrevIndex, bPrevPrevIndex = FALSE;
						BOOL bIdx = IsValidSubQIdx(pExecType, pDisc, pSubQ, nLBA, byCurrentTrackNum, &bPrevIndex, &bPrevPrevIndex);
						BOOL bCtl = IsValidSubQCtl(pSubQ, pDisc->SUB.lpEndCtlList[pSubQ->present.byTrackNum - 1]);
						// Garou Densetsu Special [PCE]
						// LBA[294048, 0x47ca0], Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[02:57:31], AMSF[65:22:48], RtoW[0, 0, 0, 0]
						// LBA[294049, 0x47ca1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[23], Idx[20], RMSF[00:27:62], AMSF[06:70:07], RtoW[0, 0, 0, 0]
						// LBA[294050, 0x47ca2], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[81], RMSF[02:134:49], AMSF[65:19:30], RtoW[0, 0, 0, 0]
						// LBA[294051, 0x47ca3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[119], Idx[121], RMSF[57:117:165], AMSF[165:161:121], RtoW[0, 0, 0, 0]
						if ((!bIdx || !bCtl) && pSubQ->present.byTrackNum != pSubQ->next.byTrackNum) {
							s_lineNum = __LINE__;
							bRet = FALSE;
						}
						// Kuusou Kagaku Sekai Gulliver Boy (Japan)
						// LBA[159095, 0x26d77],  Data,      Copy NG,                  Track[02], Idx[73], RMSF[00:35:23], AMSF[83:113:41], RtoW[0, 0, 0, 0]
						// LBA[159096, 0x26d78],  Data,      Copy NG,                  Track[02], Idx[74], RMSF[00:35:23], AMSF[123:23:41], RtoW[0, 0, 0, 0]
						// LBA[159097, 0x26d79],  Data,      Copy NG,                  Track[03], Idx[00], RMSF[00:35:23], AMSF[82:22:41], RtoW[0, 0, 0, 0]
						// LBA[159098, 0x26d7a],  Data,      Copy NG,                  Track[03], Idx[01], RMSF[00:35:23], AMSF[146:34:41], RtoW[0, 0, 0, 0]
						else if (!bIdx || !bCtl || pSubQ->present.byIndex == 0) {
							s_lineNum = __LINE__;
							bRet = FALSE;
						}
					}
				}
				else {
					// todo
				}
			}
			if (pSubQ->prevPrev.byTrackNum == pSubQ->present.byTrackNum) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQMSF(
	PEXEC_TYPE pExecType,
	LPBYTE lpSubcode,
	BYTE m,
	BYTE s,
	BYTE f
) {
	BOOL bRet = TRUE;
	if (*pExecType == gd) {
		if (lpSubcode[m] > 0xc2) {
			bRet = FALSE;
		}
	}
	else {
		if (lpSubcode[m] > 0x99) {
			bRet = FALSE;
		}
	}
	if (lpSubcode[s] > 0x59) {
		bRet = FALSE;
	}
	else if (lpSubcode[f] > 0x74) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsValidSubQRMSF(
	PEXEC_TYPE pExecType,
	PSUB_Q pSubQ,
	LPBYTE lpSubcode,
	INT nLBA
) {
	BOOL bRet = IsValidSubQMSF(pExecType, lpSubcode, 15, 16, 17);
	if (!bRet) {
		return bRet;
	}
	INT tmpLBA = MSFtoLBA(BcdToDec(lpSubcode[15]), 
		BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]));
	if (tmpLBA != 0) {
		if (pSubQ->present.byIndex > 0) {
			if (pSubQ->prev.nRelativeTime != 0 && pSubQ->present.nRelativeTime != 0 &&
				pSubQ->prev.nRelativeTime + 1 != pSubQ->present.nRelativeTime) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pSubQ->prev.byIndex > 0 &&
				pSubQ->prev.nRelativeTime + 1 != pSubQ->present.nRelativeTime) {
				// ???
				// LBA[015496, 0x03c88], Audio, 2ch, Copy NG, Pre-emphasis No, Track[09], Idx[01], RMSF[00:13:42], AMSF[03:28:46], RtoW[0, 0, 0, 0]
				// LBA[015497, 0x03c89], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RMSF[00:00:00], AMSF[03:28:47], RtoW[0, 0, 0, 0]
				// LBA[015498, 0x03c8a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RMSF[08:00:01], AMSF[03:28:48], RtoW[0, 0, 0, 0]
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			// Nights into Dreams (US)
			// LBA[201301, 0x31255], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RMSF[01:26:00], AMSF[44:46:01], RtoW[0, 0, 0, 0]
			// LBA[201302, 0x31256], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :02], RtoW[0, 0, 0, 0]
			// LBA[201303, 0x31257], Audio, 2ch, Copy NG, Pre-emphasis No, Track[19], Idx[01], RMSF[00:26:02], AMSF[44:46:03], RtoW[0, 0, 0, 0]
			//  :
			// LBA[201528, 0x31338], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[01], RMSF[00:00:01], AMSF[44:49:03], RtoW[0, 0, 0, 0]
			if (pSubQ->prevPrev.nRelativeTime + 2 == pSubQ->present.nRelativeTime) {
				bRet = TRUE;
			}
		}
		else if (pSubQ->present.byIndex == 0) {
			// SagaFrontier Original Sound Track (Disc 3)
			// LBA[009948, 0x026DC], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RMSF[00:00:01], AMSF[02:14:48], RtoW[0, 0, 0, 0]
			// LBA[009949, 0x026DD], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RMSF[00:00:00], AMSF[02:14:49], RtoW[0, 0, 0, 0]
			// LBA[009950, 0x026DE], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:00], AMSF[02:14:50], RtoW[0, 0, 0, 0]
			// LBA[009951, 0x026DF], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:01], AMSF[02:14:51], RtoW[0, 0, 0, 0]
			// Now on Never (Nick Carter) (ZJCI-10118)
			// LBA[000598, 0x00256], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RMSF[00:00:02], AMSF[00:09:73], RtoW[0, 0, 0, 0]
			// LBA[000599, 0x00257], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[00], RMSF[00:00:01], AMSF[00:09:74], RtoW[0, 0, 0, 0]
			// LBA[000600, 0x00258], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:00], AMSF[00:10:00], RtoW[0, 0, 0, 0]
			// LBA[000601, 0x00259], Audio, 2ch, Copy NG, Pre-emphasis No, Track[01], Idx[01], RMSF[00:00:01], AMSF[00:10:01], RtoW[0, 0, 0, 0]
			if (pSubQ->prev.byTrackNum == pSubQ->present.byTrackNum &&
				pSubQ->prev.nRelativeTime != pSubQ->present.nRelativeTime + 1) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
		}
	}
	else if (tmpLBA == 0) {
		// Midtown Madness (Japan)
		// LBA[198294, 0x30696], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :69], RtoW[0, 0, 0, 0]
		//  :
		// LBA[198342, 0x306c6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[08], Idx[01], RMSF[02:34:74], AMSF[44:06:42], RtoW[0, 0, 0, 0]
		// LBA[198343, 0x306c7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[09], Idx[00], RMSF[00:01:74], AMSF[44:06:43], RtoW[0, 0, 0, 0]
		// LBA[198344, 0x306c8], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :44], RtoW[0, 0, 0, 0]
		// LBA[198345, 0x306c9], Audio, 2ch, Copy NG, Pre-emphasis No, Track[09], Idx[00], RMSF[00:01:72], AMSF[44:06:45], RtoW[0, 0, 0, 0]
		//  :
		// LBA[198392, 0x306f8], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :17], RtoW[0, 0, 0, 0]
		if (nLBA != 0 &&
			pSubQ->prev.byTrackNum == pSubQ->present.byTrackNum &&
			pSubQ->prev.byIndex == pSubQ->present.byIndex) {
			if (pSubQ->present.byIndex != 0) {
				if (pSubQ->prev.nRelativeTime + 1 != pSubQ->present.nRelativeTime) {
					s_lineNum = __LINE__;
					bRet = FALSE;
				}
			}
			else if (pSubQ->present.byIndex == 0) {
				if (pSubQ->prev.nRelativeTime != pSubQ->present.nRelativeTime + 1) {
					s_lineNum = __LINE__;
					bRet = FALSE;
				}
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQAFrame(
	LPBYTE lpSubcode,
	INT nLBA
) {
	BYTE byFrame = 0;
	BYTE bySecond = 0;
	BYTE byMinute = 0;
	LBAtoMSF(nLBA + 150, &byMinute, &bySecond, &byFrame);
	if (BcdToDec(lpSubcode[21]) != byFrame) {
		return FALSE;
	}
	return TRUE;
}

BOOL IsValidSubQAMSF(
	PEXEC_TYPE pExecType,
	BOOL bRipPregap,
	PSUB_Q pSubQ,
	LPBYTE lpSubcode,
	INT nLBA
) {
	BOOL bRet = IsValidSubQMSF(pExecType, lpSubcode, 19, 20, 21);
	if (bRet) {
		INT tmpLBA = MSFtoLBA(BcdToDec(lpSubcode[19]), 
			BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21])) - 150;
		if (nLBA != tmpLBA || bRipPregap) {
			if (pSubQ->prev.nAbsoluteTime + 1 != pSubQ->present.nAbsoluteTime) {
				bRet = FALSE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQMCN(
	LPBYTE lpSubcode
) {
	BOOL bRet = TRUE;
	for (INT i = 13; i <= 19; i++) {
		if (isdigit(((lpSubcode[i] >> 4) & 0x0f) + 0x30) == 0) {
			bRet = FALSE;
			break;
		}
		if (isdigit((lpSubcode[i] & 0x0f) + 0x30) == 0) {
			bRet = FALSE;
			break;
		}
	}
	return bRet;
}

// reference
// https://isrc.jmd.ne.jp/about/pattern.html
// https://isrc.jmd.ne.jp/about/error.html
BOOL IsValidSubQISRC(
	LPBYTE lpSubcode
) {
	INT ch = ((lpSubcode[13] >> 2) & 0x3f) + 0x30;
	if (isupper(ch) == 0) {
		return FALSE;
	}

	ch = (((lpSubcode[13] << 4) & 0x30) |
		((lpSubcode[14] >> 4) & 0x0f)) + 0x30;
	if (isupper(ch) == 0) {
		return FALSE;
	}

	ch = (((lpSubcode[14] << 2) & 0x3c) |
		((lpSubcode[15] >> 6) & 0x03)) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	ch = (lpSubcode[15] & 0x3f) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	ch = ((lpSubcode[16] >> 2) & 0x3f) + 0x30;
	if (isupper(ch) == 0 && isdigit(ch) == 0) {
		return FALSE;
	}

	for (INT i = 17; i <= 19; i++) {
		if (isdigit(((lpSubcode[i] >> 4) & 0x0f) + 0x30) == 0) {
			return FALSE;
		}
		if (isdigit((lpSubcode[i] & 0x0f) + 0x30) == 0) {
			return FALSE;
		}
	}

	if (isdigit(((lpSubcode[20] >> 4) & 0x0f) + 0x30) == 0) {
		return FALSE;
	}
	return TRUE;
}

BOOL IsValidSubQAdrSector(
	DWORD dwSubAdditionalNum,
	BYTE byPrevAdr,
	BYTE byNextAdr,
	INT nRangeLBA,
	INT nFirstLBA,
	INT nPrevAdrSector,
	INT nLBA
) {
	BOOL bRet = FALSE;
	INT idx = (nLBA - nFirstLBA) / nRangeLBA;
	INT nTmpLBA = nFirstLBA + nRangeLBA * idx;
	if (nLBA < 0 ||
		(nLBA == nTmpLBA) ||
		(nLBA - nPrevAdrSector == nRangeLBA)) {
		if (1 <= dwSubAdditionalNum) {
			if (byNextAdr != ADR_ENCODES_MEDIA_CATALOG &&
				byNextAdr != ADR_ENCODES_ISRC) {
				bRet = TRUE;
			}
		}
		else {
			bRet = TRUE;
		}
	}
	else if (nLBA + 1 == nTmpLBA && byPrevAdr == ADR_ENCODES_CURRENT_POSITION) {
		// Originally, MCN sector exists per same frame number, but in case of 1st sector or next idx of the track, MCN sector slides at the next sector
		//
		// SaGa Frontier Original Sound Track (Disc 3) [First MCN Sector: 33, MCN sector exists per 91 frame]
		// LBA[039709, 0x09b1d], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :34], RtoW[0, 0, 0, 0]
		//  :
		// LBA[039799, 0x09b77], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[03:26:24], AMSF[08:52:49], RtoW[0, 0, 0, 0]
		// LBA[039800, 0x09b78], Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[00], RMSF[00:01:36], AMSF[08:52:50], RtoW[0, 0, 0, 0]
		// LBA[039801, 0x09b79], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[08:52:51], RtoW[0, 0, 0, 0]
		// LBA[039802, 0x09b7a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[03], Idx[00], RMSF[00:01:34], AMSF[08:52:51], RtoW[0, 0, 0, 0]
		//  :
		// LBA[039891, 0x09bd3], Audio, 2ch, Copy NG, Pre - emphasis No, MediaCatalogNumber[0000000000000], AMSF[    :66], RtoW[0, 0, 0, 0]

		// Super Real Marjang Special (Japan) [First MCN Sector: 71, MCN sector exists per: 98 frame]
		// LBA[090819, 0x162c3], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :69], RtoW[0, 0, 0, 0]
		//  :
		// LBA[090916, 0x16324], Audio, 2ch, Copy NG, Pre-emphasis No, Track[37], Idx[01], RMSF[00:09:12], AMSF[20:14:16], RtoW[0, 0, 0, 0]
		// LBA[090917, 0x16325], Audio, 2ch, Copy NG, Pre-emphasis No, Track[38], Idx[01], RMSF[00:00:00], AMSF[20:14:17], RtoW[0, 0, 0, 0]
		// LBA[090918, 0x16326], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :18], RtoW[0, 0, 0, 0]
		// LBA[090919, 0x16327], Audio, 2ch, Copy NG, Pre-emphasis No, Track[38], Idx[01], RMSF[00:00:02], AMSF[20:14:19], RtoW[0, 0, 0, 0]
		//  :
		// LBA[091015, 0x16387], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :40], RtoW[0, 0, 0, 0]

		// Midtown Madness (Japan) [First MCN sector: 40, MCN sector exists per 98 frame]
		// LBA[209270, 0x33176], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :20], RtoW[0, 0, 0, 0]
		//  :
		// LBA[209367, 0x331d7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[00], RMSF[00:00:00], AMSF[46:33:42], RtoW[0, 0, 0, 0]
		// LBA[209368, 0x331d8], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RMSF[00:00:00], AMSF[46:33:43], RtoW[0, 0, 0, 0]
		// LBA[209369, 0x331d9], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :44], RtoW[0, 0, 0, 0]
		// LBA[209370, 0x331da], Audio, 2ch, Copy NG, Pre-emphasis No, Track[10], Idx[01], RMSF[00:00:02], AMSF[46:33:45], RtoW[0, 0, 0, 0]
		//  :
		// LBA[209466, 0x3323a], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :66], RtoW[0, 0, 0, 0]
		bRet = TRUE;
	}
	return bRet;
}

VOID CheckAndFixSubP(
	LPBYTE lpSubcode,
	BYTE byCurrentTrackNum,
	INT nLBA
) {
	BOOL bFF = FALSE;
	BOOL b00 = FALSE;
	for (INT i = 0; i < 12; i++) {
		if (lpSubcode[i] == 0xff) {
			bFF = TRUE;
			break;
		}
		else if (lpSubcode[i] == 0x00) {
			b00 = TRUE;
			break;
		}
	}
	for (INT i = 0; i < 12; i++) {
		if (bFF && lpSubcode[i] != 0xff) {
			OutputSubErrorWithLBALogA("P[%02d]:[%#04x] -> [0xff]\n"
				, nLBA, byCurrentTrackNum, i, lpSubcode[i]);
			lpSubcode[i] = 0xff;
		}
		else if (b00 && lpSubcode[i] != 0x00) {
			OutputSubErrorWithLBALogA("P[%02d]:[%#04x] -> [0x00]\n"
				, nLBA, byCurrentTrackNum, i, lpSubcode[i]);
			lpSubcode[i] = 0x00;
		}
		else if (!bFF && !b00) {
			OutputSubErrorWithLBALogA("P[%02d]:[%#04x] -> [0x00]\n"
				, nLBA, byCurrentTrackNum, i, lpSubcode[i]);
			lpSubcode[i] = 0x00;
		}
	}
	return;
}

BOOL CheckAndFixSubQAdrMCN(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA
) {
	if (!pDisc->SUB.byCatalog) {
		return FALSE;
	}
	INT session = pDisc->SCSI.lpSessionNumList[byCurrentTrackNum - 1];
	INT nRangeLBA = pDisc->SUB.nRangeLBAForMCN[0][session - 1];
	if (nRangeLBA == -1) {
		return FALSE;
	}

	INT nFirstLBA = pDisc->SUB.nFirstLBAForMCN[0][session - 1];
	INT nPrevMCNSector = pDisc->SUB.nPrevMCNSector;
	BOOL bRet = IsValidSubQAdrSector(pExtArg->dwSubAddionalNum, pSubQ->prev.byAdr,
		pSubQ->next.byAdr, nRangeLBA, nFirstLBA, nPrevMCNSector, nLBA);
	if (!bRet) {
		nRangeLBA = pDisc->SUB.nRangeLBAForMCN[1][session - 1];
		if (nRangeLBA != -1) {
			nFirstLBA = pDisc->SUB.nFirstLBAForMCN[1][session - 1];
			bRet = IsValidSubQAdrSector(pExtArg->dwSubAddionalNum, pSubQ->prev.byAdr,
				pSubQ->next.byAdr, nRangeLBA, nFirstLBA, nPrevMCNSector, nLBA);
		}
	}
	if (bRet) {
		if (pSubQ->present.byAdr != ADR_ENCODES_MEDIA_CATALOG &&
			pSubQ->prev.byAdr != ADR_ENCODES_MEDIA_CATALOG) {
			OutputSubErrorWithLBALogA("Q[12]:Adr[%d] -> [0x02]\n"
				, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
			pSubQ->present.byAdr = ADR_ENCODES_MEDIA_CATALOG;
			lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
		}
		if (pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
			CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
			BOOL bMCN = IsValidSubQMCN(lpSubcode);
			SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);

			if (strncmp(pDisc->SUB.szCatalog, szCatalog, META_CATALOG_SIZE) || !bMCN) {
				OutputSubErrorWithLBALogA(
					"Q[13-19]:MCN[%13s], Sub[19]Lo:[%x], Sub[20]:[%#04x] -> [%13s], [19]Lo:[0], [20]:[0x00]\n"
					, nLBA, byCurrentTrackNum, szCatalog, lpSubcode[19] & 0x0f, lpSubcode[20], pDisc->SUB.szCatalog);
				SetBufferFromMCN(pDisc, lpSubcode);
			}
			bRet = TRUE;
		}
	}
	else {
		if (pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
			CHAR szCatalog[META_CATALOG_SIZE] = { 0 };
			BOOL bMCN = IsValidSubQMCN(lpSubcode);
			SetMCNToString(pDisc, lpSubcode, szCatalog, FALSE);

			if (strncmp(pDisc->SUB.szCatalog, szCatalog, META_CATALOG_SIZE) || !bMCN) {
				OutputSubErrorWithLBALogA("Q[12]:Adr[%d] -> No MCN frame\n"
					, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
				return FALSE;
			}
			INT nTmpFirstLBA = nLBA % pDisc->SUB.nRangeLBAForMCN[0][session - 1];
			OutputMainInfoWithLBALogA("Range of MCN is different [%d]\n"
				, nLBA, byCurrentTrackNum, nLBA - pDisc->SUB.nPrevMCNSector);
			pDisc->SUB.nFirstLBAForMCN[0][session - 1] = nTmpFirstLBA;
			bRet = TRUE;
		}
	}

	if (bRet) {
		pDisc->SUB.nPrevMCNSector = nLBA;
		if ((lpSubcode[19] & 0x0f) != 0) {
			OutputSubErrorWithLBALogA("Q[19]:[%x] -> [%x]\n"
				, nLBA, byCurrentTrackNum, lpSubcode[19], lpSubcode[19] & 0xf0);
			lpSubcode[19] &= 0xf0;
		}
		if (lpSubcode[20] != 0) {
			OutputSubErrorWithLBALogA("Q[20]:[%x] -> [0x00]\n"
				, nLBA, byCurrentTrackNum, lpSubcode[20]);
			lpSubcode[20] = 0;
		}
		UpdateTmpSubQDataForMCN(pExecType, pExtArg, pDisc, pSubQ, nLBA, byCurrentTrackNum);
	}
	return bRet;
}

BOOL CheckAndFixSubQAdrISRC(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA
) {
	if (!pDisc->SUB.lpISRCList[byCurrentTrackNum - 1]) {
		return FALSE;
	}
	INT session = pDisc->SCSI.lpSessionNumList[byCurrentTrackNum - 1];
	INT nRangeLBA = pDisc->SUB.nRangeLBAForISRC[0][session - 1];
	if (nRangeLBA == -1) {
		return FALSE;
	}
	INT nFirstLBA = pDisc->SUB.nFirstLBAForISRC[0][session - 1];
	INT nPrevISRCSector = pDisc->SUB.nPrevISRCSector;
	BOOL bRet = IsValidSubQAdrSector(pExtArg->dwSubAddionalNum, pSubQ->prev.byAdr,
		pSubQ->next.byAdr, nRangeLBA, nFirstLBA, nPrevISRCSector, nLBA);
	if (!bRet) {
		nRangeLBA = pDisc->SUB.nRangeLBAForISRC[1][session - 1];
		if (nRangeLBA != -1) {
			nFirstLBA = pDisc->SUB.nFirstLBAForISRC[1][session - 1];
			bRet = IsValidSubQAdrSector(pExtArg->dwSubAddionalNum, pSubQ->prev.byAdr,
				pSubQ->next.byAdr, nRangeLBA, nFirstLBA, nPrevISRCSector, nLBA);
		}
	}
	if (bRet) {
		if (pSubQ->present.byAdr != ADR_ENCODES_ISRC &&
			pSubQ->prev.byAdr != ADR_ENCODES_ISRC) {
			OutputSubErrorWithLBALogA("Q[12]:Adr[%d] -> [0x03]\n"
				, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
			pSubQ->present.byAdr = ADR_ENCODES_ISRC;
			lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
		}
		if (pSubQ->present.byAdr == ADR_ENCODES_ISRC) {
			CHAR szISRC[META_ISRC_SIZE] = { 0 };
			BOOL bISRC = IsValidSubQISRC(lpSubcode);
			SetISRCToString(pDisc, lpSubcode, szISRC, (BYTE)(byCurrentTrackNum - 1), FALSE);

			if (strncmp(pDisc->SUB.pszISRC[byCurrentTrackNum - 1], szISRC, META_ISRC_SIZE) || !bISRC) {
				OutputSubErrorWithLBALogA(
					"Q[13-20]:ISRC[%12s], SubQ[20]Lo:[%x] -> [%12s], SubQ[20]Lo:[0]\n"
					, nLBA, byCurrentTrackNum, szISRC, lpSubcode[20] & 0x0f, pDisc->SUB.pszISRC[byCurrentTrackNum - 1]);
					
				CHAR tmpISRC[META_ISRC_SIZE] = { 0 };
				strncpy(tmpISRC, pDisc->SUB.pszISRC[byCurrentTrackNum - 1], sizeof(tmpISRC) / sizeof(tmpISRC[0]));
				lpSubcode[13] = (BYTE)(((tmpISRC[0] - 0x30) << 2) | ((tmpISRC[1] - 0x30) >> 4));
				lpSubcode[14] = (BYTE)(((tmpISRC[1] - 0x30) << 4) | ((tmpISRC[2] - 0x30) >> 2));
				lpSubcode[15] = (BYTE)(((tmpISRC[2] - 0x30) << 6) | (tmpISRC[3] - 0x30));
				lpSubcode[16] = (BYTE)((tmpISRC[4] - 0x30) << 6);
				for (INT i = 17, j = 5; i < 20; i++, j += 2) {
					lpSubcode[i] = (BYTE)(tmpISRC[j] - 0x30);
					lpSubcode[i] <<= 4;
					lpSubcode[i] |= (BYTE)(tmpISRC[j + 1] - 0x30);
				}
				lpSubcode[20] = (BYTE)(tmpISRC[11] - 0x30);
				lpSubcode[20] <<= 4;
			}
			bRet = TRUE;
		}
	}
	else {
		if (pSubQ->present.byAdr == ADR_ENCODES_ISRC) {
			CHAR szISRC[META_ISRC_SIZE] = { 0 };
			BOOL bISRC = IsValidSubQISRC(lpSubcode);
			SetISRCToString(pDisc, lpSubcode, szISRC, (BYTE)(byCurrentTrackNum - 1), FALSE);

			if (strncmp(pDisc->SUB.pszISRC[byCurrentTrackNum - 1], szISRC, META_ISRC_SIZE) || !bISRC) {
				OutputSubErrorWithLBALogA("Q[12]:Adr[%d] -> No ISRC frame\n"
					, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
				return FALSE;
			}
			INT nTmpFirstLBA = nLBA % pDisc->SUB.nRangeLBAForISRC[0][session - 1];
			OutputMainInfoWithLBALogA("Range of ISRC is different [%d]\n"
				, nLBA, byCurrentTrackNum, nLBA - pDisc->SUB.nPrevISRCSector);
			pDisc->SUB.nFirstLBAForISRC[0][session - 1] = nTmpFirstLBA;
			bRet = TRUE;
		}
	}

	if (bRet) {
		pDisc->SUB.nPrevISRCSector = nLBA;
		// because tracknum, index... doesn't exist
		if (pSubQ->present.byAdr != ADR_ENCODES_ISRC) {
			pSubQ->present.byAdr = ADR_ENCODES_ISRC;
			lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
		}
		if ((lpSubcode[16] & 0x03) != 0) {
			OutputSubErrorWithLBALogA("Q[16]:[%x] -> [%x]\n"
				, nLBA, byCurrentTrackNum, lpSubcode[16], lpSubcode[16] & 0xfc);
			lpSubcode[16] &= 0xfc;
		}
		if ((lpSubcode[20] & 0x0f) != 0) {
			OutputSubErrorWithLBALogA("Q[20]:[%x] -> [%x]\n"
				, nLBA, byCurrentTrackNum, lpSubcode[20], lpSubcode[20] & 0xf0);
			lpSubcode[20] &= 0xf0;
		}
		UpdateTmpSubQDataForISRC(pSubQ);
	}
	return bRet;
}

VOID CheckAndFixSubQ(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDISC pDisc,
	LPBYTE lpSubcode,
	PSUB_Q pSubQ,
	BYTE byCurrentTrackNum,
	INT nLBA,
	BOOL bLibCrypt,
	BOOL bSecuRom
) {
	WORD crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
	BYTE tmp1 = HIBYTE(crc16);
	BYTE tmp2 = LOBYTE(crc16);
	// Red Alert (Japan) -> 208050 - 208052 is same subQ, so crc16 doesn't check this
	// LBA[208049, 0x32cb1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[56], Idx[01], RMSF[03:15:34], AMSF[46:15:74], RtoW[0, 0, 0, 0]
	// LBA[208050, 0x32cb2], Audio, 2ch, Copy NG, Pre-emphasis No, Track[56], Idx[01], RMSF[03:15:35], AMSF[46:16:00], RtoW[0, 0, 0, 0]
	// LBA[208051, 0x32cb3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[56], Idx[01], RMSF[03:15:35], AMSF[46:16:00], RtoW[0, 0, 0, 0]
	// LBA[208052, 0x32cb4], Audio, 2ch, Copy NG, Pre-emphasis No, Track[56], Idx[01], RMSF[03:15:35], AMSF[46:16:00], RtoW[0, 0, 0, 0]
	// LBA[208053, 0x32cb5], Audio, 2ch, Copy NG, Pre-emphasis No, Track[56], Idx[01], RMSF[03:15:36], AMSF[46:16:03], RtoW[0, 0, 0, 0]
	// LBA[208054, 0x32cb6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[56], Idx[01], RMSF[03:15:37], AMSF[46:16:04], RtoW[0, 0, 0, 0]
	BOOL bAMSF = IsValidSubQAMSF(pExecType,	pExtArg->byPre, pSubQ, lpSubcode, nLBA);
	BOOL bAFrame = IsValidSubQAFrame(lpSubcode, nLBA);

	if (-76 < nLBA) {
		if (lpSubcode[22] == tmp1 && lpSubcode[23] == tmp2 && (bAMSF || bAFrame)) {
			if (pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				UpdateTmpSubQDataForMCN(pExecType, pExtArg, pDisc, pSubQ, nLBA, byCurrentTrackNum);
			}
			else if (pSubQ->present.byAdr == ADR_ENCODES_ISRC) {
				UpdateTmpSubQDataForISRC(pSubQ);
			}
			else if (pSubQ->present.byAdr == ADR_ENCODES_CDTV_SPECIFIC) {
				UpdateTmpSubQDataForCDTV(pExecType, pDisc, pSubQ, nLBA, byCurrentTrackNum);
			}
			return;
		}
	}
	else {
		if (lpSubcode[22] != tmp1 || lpSubcode[23] != tmp2) {
			// TODO
			OutputSubErrorWithLBALogA("Q <TODO>\n", nLBA, byCurrentTrackNum);
		}
		return;
	}

	if (pSubQ->prevPrev.byTrackNum == 110 ||
		pSubQ->prev.byTrackNum == 110 ||
		pSubQ->present.byTrackNum == 110) {
		// skip lead-out
		if (nLBA > pDisc->SCSI.nAllLength - 10) {
			return;
		}
		else if (pDisc->SCSI.lpSessionNumList[byCurrentTrackNum] >= 2) {
			// Wild Romance [Kyosuke Himuro]
			// LBA[043934, 0x0ab9e], Audio, 2ch, Copy NG, Pre-emphasis No, Track[02], Idx[01], RMSF[04:19:39], AMSF[09:47:59], RtoW[0, 0, 0, 0]
			// LBA[055335, 0x0d827], Audio, 2ch, Copy NG, Pre-emphasis No, Track[110], Idx[01], RMSF[00:00:01], AMSF[09:47:61], RtoW[0, 0, 0, 0]
			// LBA[055336, 0x0d828],  Data,      Copy NG,                  Track[03], Idx[01], RMSF[00:00:01], AMSF[12:19:61], RtoW[0, 0, 0, 0]
			pSubQ->present.byCtl = AUDIO_DATA_TRACK;
			pSubQ->present.byTrackNum = (BYTE)(byCurrentTrackNum + 1);
			pSubQ->present.nRelativeTime = 0;
			return;
		}
	}

	BOOL bAdrCurrent = FALSE;
	if (1 <= pExtArg->dwSubAddionalNum) {
		// first check adr:2
		if (!CheckAndFixSubQAdrMCN(pExecType, pExtArg,
			pDisc, lpSubcode, pSubQ, byCurrentTrackNum, nLBA)) {
			// Next check adr:3
			if (!CheckAndFixSubQAdrISRC(pExtArg, pDisc,
				lpSubcode, pSubQ, byCurrentTrackNum, nLBA)) {
				bAdrCurrent = TRUE;
			}
		}
	}
	else {
		// If it doesn't read the next sector,
		// adr premises that it isn't ADR_ENCODES_CURRENT_POSITION
		if (pSubQ->present.byAdr == ADR_ENCODES_ISRC) {
			// first check adr:3
			if (!CheckAndFixSubQAdrISRC(pExtArg, pDisc,
				lpSubcode, pSubQ, byCurrentTrackNum, nLBA)) {
				// Next check adr:2
				if (!CheckAndFixSubQAdrMCN(pExecType, pExtArg,
					pDisc, lpSubcode, pSubQ, byCurrentTrackNum, nLBA)) {
					bAdrCurrent = TRUE;
				}
			}
		}
		else if (pSubQ->present.byAdr != ADR_ENCODES_CURRENT_POSITION) {
			// first check adr:2
			if (!CheckAndFixSubQAdrMCN(pExecType, pExtArg,
				pDisc, lpSubcode, pSubQ, byCurrentTrackNum, nLBA)) {
				// Next check adr:3
				if (!CheckAndFixSubQAdrISRC(pExtArg, pDisc,
					lpSubcode, pSubQ, byCurrentTrackNum, nLBA)) {
					bAdrCurrent = TRUE;
				}
			}
		}
	}
	if (bAdrCurrent && pSubQ->present.byAdr != ADR_ENCODES_CURRENT_POSITION) {
		if (pSubQ->present.byAdr > ADR_ENCODES_ISRC) {
			OutputSubErrorWithLBALogA("Q[12]:Adr[%d] -> [0x01]\n"
				, nLBA, byCurrentTrackNum, pSubQ->present.byAdr);
		}
		pSubQ->present.byAdr = ADR_ENCODES_CURRENT_POSITION;
		lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
	}

	BYTE SubQcodeOrg[12] = { 0 };
	if (bLibCrypt || bSecuRom) {
		memcpy(SubQcodeOrg, &lpSubcode[12], sizeof(SubQcodeOrg));
	}

	if (pSubQ->present.byAdr == ADR_ENCODES_CURRENT_POSITION) {
		BOOL bPrevTrackNum = TRUE;
		if (!IsValidSubQTrack(pExecType, pDisc, pSubQ, nLBA, byCurrentTrackNum, &bPrevTrackNum)) {
			OutputSubErrorWithLBALogA("Q[13]:TrackNum[%02u] L:[%ld] -> "
				, nLBA, byCurrentTrackNum, pSubQ->present.byTrackNum, s_lineNum);
			if (*pExecType == gd && byCurrentTrackNum == pDisc->GDROM_TOC.LastTrack) {
				pSubQ->present.byTrackNum = pDisc->GDROM_TOC.LastTrack;
				OutputSubErrorLogA("[%02u], L:[%d]\n", pDisc->GDROM_TOC.LastTrack, (INT)__LINE__);
			}
			else if (*pExecType != gd && byCurrentTrackNum == pDisc->SCSI.toc.LastTrack) {
				pSubQ->present.byTrackNum = pDisc->SCSI.toc.LastTrack;
				OutputSubErrorLogA("[%02u], L:[%d]\n", pDisc->SCSI.toc.LastTrack, (INT)__LINE__);
			}
			else if (*pExecType != gd && pDisc->SCSI.lpFirstLBAListOnToc[byCurrentTrackNum] < nLBA) {
				pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
				OutputSubErrorLogA("[%02u], L:[%d]\n", pSubQ->prev.byTrackNum + 1, (INT)__LINE__);
			}
			else if (pSubQ->prev.byIndex != 0 && pSubQ->present.byIndex == 1 && pSubQ->present.nRelativeTime == 0) {
				// Bikkuriman Daijikai (Japan)
				// LBA[106402, 0x19FA2], Audio, 2ch, Copy NG, Pre-emphasis No, Track[70], Idx[01], RMSF[00:16:39], AMSF[23:40:52], RtoW[0, 0, 0, 0]
				// LBA[106403, 0x19FA3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[79], Idx[01], RMSF[00:00:00], AMSF[21:40:53], RtoW[0, 0, 0, 0]
				// LBA[106404, 0x19FA4], Audio, 2ch, Copy NG, Pre-emphasis No, Track[71], Idx[01], RMSF[00:00:01], AMSF[23:40:54], RtoW[0, 0, 0, 0]
				pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
				OutputSubErrorLogA("[%02u], L:[%d]\n", pSubQ->prev.byTrackNum + 1, (INT)__LINE__);
			}
			else if (pSubQ->present.byIndex == 0 && IsValidPregapSector(pExecType, pDisc, pSubQ, nLBA)) {
				// Network Q RAC Rally Championship (Netherlands)
				// LBA[202407, 0x316a7], Audio, 2ch, Copy NG, Pre-emphasis No, Track[13], Idx[01], RMSF[05:21:29], AMSF[45:00:57], RtoW[0, 0, 0, 0]
				// LBA[202408, 0x316a8], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[00], RMSF[00:01:74], AMSF[45:00:58], RtoW[0, 0, 0, 0]
				// LBA[202409, 0x316a9], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[00], RMSF[00:01:73], AMSF[45:00:59], RtoW[0, 0, 0, 0]
				pSubQ->present.byTrackNum = (BYTE)(pSubQ->prev.byTrackNum + 1);
				OutputSubErrorLogA("[%02u], L:[%d]\n", pSubQ->prev.byTrackNum + 1, (INT)__LINE__);
			}
			else {
				pSubQ->present.byTrackNum = pSubQ->prev.byTrackNum;
				OutputSubErrorLogA("[%02u], L:[%d]\n", pSubQ->prev.byTrackNum, (INT)__LINE__);
			}
			lpSubcode[13] = DecToBcd(pSubQ->present.byTrackNum);
		}
		else if (!bPrevTrackNum) {
			if (pSubQ->prev.byIndex < MAXIMUM_NUMBER_INDEXES && 
				0 < pSubQ->prev.byTrackNum && pSubQ->prev.byTrackNum <= pDisc->SCSI.toc.LastTrack) {
				OutputSubErrorWithLBALogA("Q[13]:PrevTrackNum[%02u] -> [%02u]\n"
					, nLBA, byCurrentTrackNum, pSubQ->prev.byTrackNum, pSubQ->prevPrev.byTrackNum);
				pDisc->SUB.lpFirstLBAListOnSub[pSubQ->prev.byTrackNum - 1][pSubQ->prev.byIndex] = -1;
				pDisc->SUB.lpFirstLBAListOnSubSync[pSubQ->prev.byTrackNum - 1][pSubQ->prev.byIndex] = -1;
				pDisc->SUB.lpFirstLBAListOfDataTrackOnSub[pSubQ->prev.byTrackNum - 1] = -1;
			}
			pSubQ->prev.byTrackNum = pSubQ->prevPrev.byTrackNum;
			pSubQ->prev.byIndex = pSubQ->prevPrev.byIndex;
			pSubQ->prev.nRelativeTime = pSubQ->prevPrev.nRelativeTime + 1;
		}
		BOOL bPrevIndex = TRUE;
		BOOL bPrevPrevIndex = TRUE;
		if (!IsValidSubQIdx(pExecType, pDisc, pSubQ, nLBA, byCurrentTrackNum, &bPrevIndex, &bPrevPrevIndex)) {
			BYTE tmpIdx = pSubQ->prev.byIndex;
			if (IsValidPregapSector(pExecType, pDisc, pSubQ, nLBA)) {
				if (pSubQ->next.byIndex == 0 && pSubQ->next.nRelativeTime + 1 == pSubQ->present.nRelativeTime) {
					tmpIdx = 0;
				}
			}
			OutputSubErrorWithLBALogA("Q[14]:Idx[%02u] -> [%02u], L:[%ld]\n"
				, nLBA, byCurrentTrackNum, pSubQ->present.byIndex, tmpIdx, s_lineNum);
			pSubQ->present.byIndex = tmpIdx;
			lpSubcode[14] = DecToBcd(pSubQ->present.byIndex);
		}
		else if (!bPrevIndex) {
			if (pSubQ->prev.byIndex < MAXIMUM_NUMBER_INDEXES && 
				0 < pSubQ->prev.byTrackNum && pSubQ->prev.byTrackNum <= pDisc->SCSI.toc.LastTrack) {
				OutputSubErrorWithLBALogA("Q[14]:PrevIdx[%02u] -> [%02u]\n"
					, nLBA - 1, byCurrentTrackNum, pSubQ->prev.byIndex, pSubQ->prevPrev.byIndex);
				pDisc->SUB.lpFirstLBAListOnSub[pSubQ->prev.byTrackNum - 1][pSubQ->prev.byIndex] = -1;
				pDisc->SUB.lpFirstLBAListOnSubSync[pSubQ->prev.byTrackNum - 1][pSubQ->prev.byIndex] = -1;
			}
			pSubQ->prev.byTrackNum = pSubQ->prevPrev.byTrackNum;
			pSubQ->prev.byIndex = pSubQ->prevPrev.byIndex;
		}
		else if (!bPrevPrevIndex) {
			if (pSubQ->prevPrev.byIndex < MAXIMUM_NUMBER_INDEXES &&
				0 < pSubQ->prev.byTrackNum && pSubQ->prev.byTrackNum <= pDisc->SCSI.toc.LastTrack) {
				OutputSubErrorWithLBALogA("Q[14]:PrevPrevIdx[%02u] -> [%02u]\n"
					, nLBA - 1, byCurrentTrackNum, pSubQ->prevPrev.byIndex, pSubQ->prev.byIndex);
				pDisc->SUB.lpFirstLBAListOnSub[pSubQ->prevPrev.byTrackNum - 1][pSubQ->prevPrev.byIndex] = -1;
				pDisc->SUB.lpFirstLBAListOnSubSync[pSubQ->prevPrev.byTrackNum - 1][pSubQ->prevPrev.byIndex] = -1;
			}
			pSubQ->prevPrev.byTrackNum = pSubQ->prev.byTrackNum;
			pSubQ->prevPrev.byIndex = pSubQ->prev.byIndex;
		}

		if (!IsValidSubQRMSF(pExecType, pSubQ, lpSubcode, nLBA)) {
			if (!(pSubQ->prev.byIndex == 0 && pSubQ->present.byIndex == 1) &&
				!(pSubQ->prev.byIndex >= 1 && pSubQ->present.byIndex == 0) ||
				nLBA == 0 && pDisc->PROTECT.byExist == securomV3) {
				BYTE byFrame = 0;
				BYTE bySecond = 0;
				BYTE byMinute = 0;
				INT tmpRel = 0;
				if (pSubQ->present.byIndex > 0) {
					if (pDisc->SCSI.lpFirstLBAListOnToc[byCurrentTrackNum] != nLBA) {
						tmpRel = pSubQ->prev.nRelativeTime + 1;
					}
				}
				else if (pSubQ->present.byIndex == 0) {
					tmpRel = pSubQ->prev.nRelativeTime - 1;
				}
				// Colin McRae Rally 2.0 (Europe) (En,Fr,De,Es,It) etc
				if (bSecuRom && pDisc->PROTECT.byExist == securomV3 && nLBA == 7) {
					tmpRel -= 1;
				}
				LBAtoMSF(tmpRel, &byMinute, &bySecond, &byFrame);
				BYTE byPrevFrame = 0;
				BYTE byPrevSecond = 0;
				BYTE byPrevMinute = 0;
				LBAtoMSF(pSubQ->prev.nRelativeTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
				OutputSubErrorWithLBALogA(
					"Q[15-17]:PrevRel[%d, %02u:%02u:%02u], Rel[%d, %02u:%02u:%02u] -> [%d, %02u:%02u:%02u], L:[%ld]"
					, nLBA, byCurrentTrackNum, pSubQ->prev.nRelativeTime, byPrevMinute, byPrevSecond, byPrevFrame
					, pSubQ->present.nRelativeTime,	BcdToDec(lpSubcode[15]), BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17])
					, tmpRel, byMinute, bySecond, byFrame, s_lineNum);
				if (bLibCrypt || bSecuRom) {
					OutputSubErrorLogA(
						" But this sector may be the intentional error of RMSF. see _subinfo.txt");
					INT nMax = 8;
					if (nLBA < 8) {
						nMax = 7;
					}
					if (0 < pDisc->PROTECT.byRestoreCounter && pDisc->PROTECT.byRestoreCounter < nMax &&
						pSubQ->prev.nRelativeTime + 2 != pSubQ->present.nRelativeTime) {
						LBAtoMSF(pSubQ->prev.nRelativeTime + 2, &byPrevMinute, &byPrevSecond, &byPrevFrame);
						SubQcodeOrg[3] = DecToBcd(byPrevMinute);
						SubQcodeOrg[4] = DecToBcd(byPrevSecond);
						SubQcodeOrg[5] = DecToBcd(byPrevFrame);
						OutputSubErrorLogA(" And this RMSF is a random error. Fixed [%d, %02u:%02u:%02u]\n"
							, pSubQ->prev.nRelativeTime + 2, byPrevMinute, byPrevSecond, byPrevFrame);
					}
					else {
						OutputSubErrorLogA("\n");
					}
				}
				else {
					OutputSubErrorLogA("\n");
				}
				pSubQ->present.nRelativeTime = tmpRel;
				lpSubcode[15] = DecToBcd(byMinute);
				lpSubcode[16] = DecToBcd(bySecond);
				lpSubcode[17] = DecToBcd(byFrame);
			}
		}

		if (lpSubcode[18] != 0) {
			OutputSubErrorWithLBALogA("Q[18]:[%#04x] -> [0x00]\n"
				, nLBA, byCurrentTrackNum, lpSubcode[18]);
			lpSubcode[18] = 0;
		}

		if (!IsValidSubQAMSF(pExecType, pExtArg->byPre, pSubQ, lpSubcode, nLBA)) {
			BYTE byFrame = 0;
			BYTE bySecond = 0;
			BYTE byMinute = 0;
			INT tmpAbs = nLBA + 150;
			LBAtoMSF(tmpAbs, &byMinute, &bySecond, &byFrame);
			BYTE byPrevFrame = 0;
			BYTE byPrevSecond = 0;
			BYTE byPrevMinute = 0;
			LBAtoMSF(pSubQ->prev.nAbsoluteTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
			OutputSubErrorWithLBALogA(
				"Q[19-21]:PrevAbs[%d, %02u:%02u:%02u], Abs[%d, %02u:%02u:%02u] -> [%d, %02u:%02u:%02u]"
				, nLBA, byCurrentTrackNum, pSubQ->prev.nAbsoluteTime, byPrevMinute, byPrevSecond, byPrevFrame
				, pSubQ->present.nAbsoluteTime,	BcdToDec(lpSubcode[19]), BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21])
				, tmpAbs, byMinute, bySecond, byFrame);
			if (bLibCrypt || bSecuRom) {
				OutputSubErrorLogA(
					" But this sector may be the intentional error of AMSF. see _subinfo.txt");
				INT nMax = 8;
				if (nLBA < 8) {
					nMax = 7;
				}
				if (0 < pDisc->PROTECT.byRestoreCounter && pDisc->PROTECT.byRestoreCounter < nMax &&
					pSubQ->prev.nAbsoluteTime + 2 != pSubQ->present.nAbsoluteTime) {
					LBAtoMSF(pSubQ->prev.nAbsoluteTime + 2, &byPrevMinute, &byPrevSecond, &byPrevFrame);
					SubQcodeOrg[7] = DecToBcd(byPrevMinute);
					SubQcodeOrg[8] = DecToBcd(byPrevSecond);
					SubQcodeOrg[9] = DecToBcd(byPrevFrame);
					OutputSubErrorLogA(" And this AMSF is a random error. Fixed [%d, %02u:%02u:%02u]\n"
						, pSubQ->prev.nAbsoluteTime + 2, byPrevMinute, byPrevSecond, byPrevFrame);
				}
				else {
					OutputSubErrorLogA("\n");
				}
			}
			else {
				OutputSubErrorLogA("\n");
			}
			pSubQ->present.nAbsoluteTime = MSFtoLBA(byMinute, bySecond, byFrame) + 150;
			lpSubcode[19] = DecToBcd(byMinute);
			lpSubcode[20] = DecToBcd(bySecond);
			lpSubcode[21] = DecToBcd(byFrame);
		}
	}
	else if (nLBA >= 0 &&
		(pSubQ->present.byAdr == ADR_ENCODES_MEDIA_CATALOG ||
		pSubQ->present.byAdr == ADR_ENCODES_ISRC)) {
		if (!IsValidSubQAFrame(lpSubcode, nLBA)) {
			BYTE byFrame = 0;
			BYTE bySecond = 0;
			BYTE byMinute = 0;
			LBAtoMSF(nLBA + 150, &byMinute, &bySecond, &byFrame);
			BYTE byPrevFrame = 0;
			BYTE byPrevSecond = 0;
			BYTE byPrevMinute = 0;
			LBAtoMSF(pSubQ->prev.nAbsoluteTime, &byPrevMinute, &byPrevSecond, &byPrevFrame);
			OutputSubErrorWithLBALogA(
				"Q[21]:PrevAbsFrame[%02u], AbsFrame[%02u] -> [%02u]\n"
				, nLBA, byCurrentTrackNum, byPrevFrame, BcdToDec(lpSubcode[21]), byFrame);
			pSubQ->present.nAbsoluteTime = MSFtoLBA(byMinute, bySecond, byFrame) + 150;
			lpSubcode[21] = DecToBcd(byFrame);
		}
	}
	if (!IsValidSubQCtl(pSubQ, pDisc->SUB.lpEndCtlList[pSubQ->present.byTrackNum - 1])) {
		OutputSubErrorWithLBALogA("Q[12]:Ctl[%u] -> [%u], L:[%ld]\n"
			, nLBA, byCurrentTrackNum, pSubQ->present.byCtl, pSubQ->prev.byCtl, s_lineNum);
		pSubQ->present.byCtl = pSubQ->prev.byCtl;
		lpSubcode[12] = (BYTE)(pSubQ->present.byCtl << 4 | pSubQ->present.byAdr);
	}
	// lpSubcode has already fixed a ramdom error (= original crc)
	crc16 = (WORD)GetCrc16CCITT(10, &lpSubcode[12]);
	if (bLibCrypt || bSecuRom) {
		BOOL bExist = FALSE;
		WORD xorCrc16 = (WORD)(crc16 ^ 0x8001);
		if (SubQcodeOrg[10] == HIBYTE(xorCrc16) &&
			SubQcodeOrg[11] == LOBYTE(xorCrc16)) {
			OutputSubInfoWithLBALogA(
				"Detected intentional error. CRC-16 is original:[%04x] and XORed with 0x8001:[%04x]"
				, nLBA, byCurrentTrackNum, crc16, xorCrc16);
			bExist = TRUE;
		}
		else {
			// lpSubcode isn't fixed (= recalc crc)
			WORD reCalcCrc16 = (WORD)GetCrc16CCITT(10, &SubQcodeOrg[0]);
			WORD reCalcXorCrc16 = (WORD)(reCalcCrc16 ^ 0x0080);
			if (SubQcodeOrg[10] == HIBYTE(reCalcXorCrc16) &&
				SubQcodeOrg[11] == LOBYTE(reCalcXorCrc16) && 
				pDisc->PROTECT.byRestoreCounter == 8) {
				OutputSubInfoWithLBALogA(
					"Detected intentional error. CRC-16 is recalculated:[%04x] and XORed with 0x0080:[%04x]"
					, nLBA, byCurrentTrackNum, reCalcCrc16, reCalcXorCrc16);
				bExist = TRUE;
			}
		}
		if (bExist) {
			OutputSubInfoLogA(
				" Restore RMSF[%02x:%02x:%02x to %02x:%02x:%02x] AMSF[%02x:%02x:%02x to %02x:%02x:%02x]\n"
				, lpSubcode[15], lpSubcode[16], lpSubcode[17], SubQcodeOrg[3], SubQcodeOrg[4], SubQcodeOrg[5]
				, lpSubcode[19], lpSubcode[20], lpSubcode[21], SubQcodeOrg[7], SubQcodeOrg[8], SubQcodeOrg[9]);
			// rmsf
			lpSubcode[15] = SubQcodeOrg[3];
			lpSubcode[16] = SubQcodeOrg[4];
			lpSubcode[17] = SubQcodeOrg[5];
			// amsf
			lpSubcode[19] = SubQcodeOrg[7];
			lpSubcode[20] = SubQcodeOrg[8];
			lpSubcode[21] = SubQcodeOrg[9];
			// crc
			lpSubcode[22] = SubQcodeOrg[10];
			lpSubcode[23] = SubQcodeOrg[11];
			OutputIntentionalSubchannel(nLBA, &lpSubcode[12]);
			pDisc->PROTECT.byRestoreCounter = 0;
		}
		else {
			if (pDisc->PROTECT.byExist == securomV1 || pDisc->PROTECT.byExist == securomV2 || pDisc->PROTECT.byExist == securomV3) {
				INT nPrevRMSF = MSFtoLBA(BcdToDec(lpSubcode[15]), BcdToDec(lpSubcode[16]), BcdToDec(lpSubcode[17]));
				INT nRMSF = MSFtoLBA(BcdToDec(SubQcodeOrg[3]), BcdToDec(SubQcodeOrg[4]), BcdToDec(SubQcodeOrg[5]));
				INT nPrevAMSF = MSFtoLBA(BcdToDec(lpSubcode[19]), BcdToDec(lpSubcode[20]), BcdToDec(lpSubcode[21]));
				INT nAMSF = MSFtoLBA(BcdToDec(SubQcodeOrg[7]), BcdToDec(SubQcodeOrg[8]), BcdToDec(SubQcodeOrg[9]));
				if (nPrevRMSF + 1 == nRMSF && nPrevAMSF + 1 == nAMSF ||
					nPrevRMSF == nRMSF && nPrevAMSF + 1 == nAMSF && pDisc->PROTECT.byExist == securomV3 && 0 <= nLBA && nLBA < 9) {
					OutputSubInfoWithLBALogA(
						"Detected shifted sub. Restore RMSF[%02x:%02x:%02x to %02x:%02x:%02x] AMSF[%02x:%02x:%02x to %02x:%02x:%02x]\n"
						, nLBA, byCurrentTrackNum, lpSubcode[15], lpSubcode[16], lpSubcode[17], SubQcodeOrg[3], SubQcodeOrg[4], SubQcodeOrg[5]
						, lpSubcode[19], lpSubcode[20], lpSubcode[21], SubQcodeOrg[7], SubQcodeOrg[8], SubQcodeOrg[9]);
					// rmsf
					lpSubcode[15] = SubQcodeOrg[3];
					lpSubcode[16] = SubQcodeOrg[4];
					lpSubcode[17] = SubQcodeOrg[5];
					// amsf
					lpSubcode[19] = SubQcodeOrg[7];
					lpSubcode[20] = SubQcodeOrg[8];
					lpSubcode[21] = SubQcodeOrg[9];
					OutputIntentionalSubchannel(nLBA, &lpSubcode[12]);
					pDisc->PROTECT.byRestoreCounter++;
				}
				else {
					OutputSubInfoWithLBALogA("Intentional error doesn't exist.\n", nLBA, byCurrentTrackNum);
				}
			}
			else {
				OutputSubInfoWithLBALogA("Intentional error doesn't exist.\n", nLBA, byCurrentTrackNum);
			}
		}
	}
	else {
		tmp1 = HIBYTE(crc16);
		tmp2 = LOBYTE(crc16);
		if (lpSubcode[22] != tmp1) {
			OutputSubErrorWithLBALogA("Q[22]:CrcHigh[%#04x] -> [%#04x]\n", nLBA, byCurrentTrackNum, lpSubcode[22], tmp1);
			lpSubcode[22] = tmp1;
		}
		if (lpSubcode[23] != tmp2) {
			OutputSubErrorWithLBALogA("Q[23]:CrcLow[%#04x] -> [%#04x]\n", nLBA, byCurrentTrackNum, lpSubcode[23], tmp2);
			lpSubcode[23] = tmp2;
		}
	}
	return;
}

VOID CheckAndFixSubRtoW(
	PDEVICE pDevice,
	PDISC pDisc,
	LPBYTE lpBuf,
	LPBYTE lpSubcode,
	BYTE byCurrentTrackNum,
	INT nLBA
) {
#if 0
	BYTE lpSubcodeOrg[CD_RAW_READ_SUBCODE_SIZE] = { 0 };
	memcpy(lpSubcodeOrg, lpBuf + pDevice->TRANSFER.dwBufSubOffset, CD_RAW_READ_SUBCODE_SIZE);
	SUB_R_TO_W scRW[4] = { 0 };
	BYTE tmpCode[24] = { 0 };
	for (INT k = 0; k < 4; k++) {
		for (INT j = 0; j < 24; j++) {
			tmpCode[j] = (BYTE)(*(lpSubcodeOrg + (k * 24 + j)) & 0x3f);
		}
		memcpy(&scRW[k], tmpCode, sizeof(scRW[k]));
		if (scRW[k].parityQ[0] != 0 || scRW[k].parityQ[1] != 0) {
			for (INT m = 0; m < 24; m++) {
				BYTE crc6 = GetCrc6ITU(24, tmpCode, m);
				OutputSubInfoLogA("%d=%02x. ", m, crc6);
			}
			OutputSubInfoLogA("\n");
			OutputSubInfoWithLBALogA(
				"Pack[%2d]: parityQ[0][%02x], parityQ[1][%02x]\n",
				nLBA, nLBA, byCurrentTrackNum, k, scRW[k].parityQ[0], scRW[k].parityQ[1]);
		}
		if (scRW[k].parityP[0] != 0 || scRW[k].parityP[1] != 0 ||
			scRW[k].parityP[2] != 0 || scRW[k].parityP[3] != 0) {
			for (INT m = 0; m < 24; m++) {
				BYTE crc6 = GetCrc6ITU(24, tmpCode, m);
				OutputSubInfoLogA("%d=%02x. ", m, crc6);
			}
			OutputSubInfoLogA("\n");
			OutputSubInfoWithLBALogA(
				"Pack[%2d]: parityP[0][%02x], parityP[1][%02x], parityP[2][%02x], parityP[3][%02x]\n",
				nLBA, nLBA, byCurrentTrackNum, k, scRW[k].parityP[0], scRW[k].parityP[1], scRW[k].parityP[2], scRW[k].parityP[3]);
		}
	}
#else
	UNREFERENCED_PARAMETER(lpBuf);
	UNREFERENCED_PARAMETER(pDevice);
#endif
	INT idx = byCurrentTrackNum - 1;
	if (pDisc->SUB.lpRtoWList[idx] == SUB_RTOW_TYPE::Zero ||
		pDisc->SUB.lpRtoWList[idx] == SUB_RTOW_TYPE::PSXSpecific) {
		for (INT j = 24; j < CD_RAW_READ_SUBCODE_SIZE; j++) {
			if (lpSubcode[j] != 0) {
				if ((24 <= j && j <= 34) ||
					(j == 35 && pDisc->SUB.lpRtoWList[idx] == SUB_RTOW_TYPE::Zero)) {
					OutputSubErrorWithLBALogA("R[%02d]:[%#04x] -> [0x00]\n"
						, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if ((36 <= j && j <= 46) ||
					(j == 47 && pDisc->SUB.lpRtoWList[idx] == SUB_RTOW_TYPE::Zero)) {
					OutputSubErrorWithLBALogA("S[%02d]:[%#04x] -> [0x00]\n"
						, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if ((48 <= j && j <= 58) ||
					(j == 59 && pDisc->SUB.lpRtoWList[idx] == SUB_RTOW_TYPE::Zero)) {
					OutputSubErrorWithLBALogA("T[%02d]:[%#04x] -> [0x00]\n"
						, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if ((60 <= j && j <= 70) ||
					(j == 71 && pDisc->SUB.lpRtoWList[idx] == SUB_RTOW_TYPE::Zero)) {
					OutputSubErrorWithLBALogA("U[%02d]:[%#04x] -> [0x00]\n"
						, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if ((72 <= j && j <= 82) ||
					(j == 83 && pDisc->SUB.lpRtoWList[idx] == SUB_RTOW_TYPE::Zero)) {
					OutputSubErrorWithLBALogA("V[%02d]:[%#04x] -> [0x00]\n"
						, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
				else if ((84 <= j && j <= 94) ||
					(j == 95 && pDisc->SUB.lpRtoWList[idx] == SUB_RTOW_TYPE::Zero)) {
					OutputSubErrorWithLBALogA("W[%02d]:[%#04x] -> [0x00]\n"
						, nLBA, byCurrentTrackNum, j, lpSubcode[j]);
					lpSubcode[j] = 0;
				}
			}
		}
	}
	return;
}

VOID CheckAndFixSubChannel(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	BYTE byCurrentTrackNum,
	INT nLBA,
	BOOL bLibCrypt,
	BOOL bSecuRom
) {
	if (pDisc->SUB.nSubChannelOffset) {
		SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.next, pDiscPerSector->subcode.next);
		if (1 <= pExtArg->dwSubAddionalNum) {
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.nextNext, pDiscPerSector->subcode.nextNext);
		}
	}
	else {
		if (1 <= pExtArg->dwSubAddionalNum) {
			SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.next, pDiscPerSector->subcode.next);
			if (2 <= pExtArg->dwSubAddionalNum) {
				SetTmpSubQDataFromBuffer(&pDiscPerSector->subQ.nextNext, pDiscPerSector->subcode.nextNext);
			}
		}
	}
	if (!pExtArg->bySkipSubP) {
		CheckAndFixSubP(pDiscPerSector->subcode.present, byCurrentTrackNum, nLBA);
	}
	if (!pExtArg->bySkipSubQ) {
		CheckAndFixSubQ(pExecType, pExtArg, pDisc, pDiscPerSector->subcode.present
			, &pDiscPerSector->subQ, byCurrentTrackNum, nLBA, bLibCrypt, bSecuRom);
	}
	if (!pExtArg->bySkipSubRtoW) {
		CheckAndFixSubRtoW(pDevice, pDisc, pDiscPerSector->data.present
			, pDiscPerSector->subcode.present, byCurrentTrackNum, nLBA);
	}
	return;
}

BOOL ContainsC2Error(
	PDEVICE pDevice,
	LPBYTE lpBuf
) {
	BOOL bRet = RETURNED_NO_C2_ERROR_1ST;
	for (WORD wC2ErrorPos = 0; wC2ErrorPos < CD_RAW_READ_C2_294_SIZE; wC2ErrorPos++) {
		DWORD dwPos = pDevice->TRANSFER.dwBufC2Offset + wC2ErrorPos;
		if (lpBuf[dwPos] != 0) {
			// Ricoh based drives (+97 read offset, like the Aopen CD-RW CRW5232)
			// use lsb points to 1st byte of main. 
			// But almost drive is msb points to 1st byte of main.
//			INT nBit = 0x01;
			INT nBit = 0x80;
			for (INT n = 0; n < CHAR_BIT; n++) {
				// exist C2 error
				if (lpBuf[dwPos] & nBit) {
					// wC2ErrorPos * CHAR_BIT => position of byte
					// (position of byte) + n => position of bit
					bRet = RETURNED_EXIST_C2_ERROR;
					break;
				}
//				nBit <<= 1;
				nBit >>= 1;
			}
			if (bRet == RETURNED_EXIST_C2_ERROR) {
				break;
			}
		}
	}
	return bRet;
}

VOID SupportIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	if (pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX760A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX755A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX716AL &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX716A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX714A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX712A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX708A2 &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX708A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX704A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PREMIUM2 &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PREMIUM &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXW5224A) {
		OutputString(
			_T("This drive doesn't support to rip from 00:00:00 to 00:01:74 AMSF. /p option is ignored\n"));
		pExtArg->byPre = FALSE;
	}
}

BOOL IsCheckingSubChannel(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	INT nLBA
) {
	BOOL bCheckSub = TRUE;
	if (!pExtArg->byRawDump &&
		(pDisc->SCSI.nFirstLBAofLeadout <= nLBA &&
		nLBA < pDisc->SCSI.nFirstLBAof2ndSession)) {
		bCheckSub = FALSE;
	}
	return bCheckSub;
}

VOID CheckAndFixMainHeader(
	PEXT_ARG pExtArg,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	BYTE byCurrentTrackNum,
	INT nMainDataType
) {
	LPBYTE lpWorkBuf = pDiscPerSector->data.present + pDisc->MAIN.uiMainDataSlideSize;
	BOOL bHeader = IsValidMainDataHeader(lpWorkBuf);
	INT idx = byCurrentTrackNum - 1;
	if (bHeader) {
		if ((pDisc->PROTECT.byExist == smartE || pDisc->PROTECT.byExist == microids) &&
			(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize) ||
			(pDisc->PROTECT.byExist == microids && pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd + pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd)) {
			BYTE m, s, f;
			if (!pExtArg->byBe) {
				m = BcdToDec(BYTE(lpWorkBuf[12] ^ 0x01));
				s = BcdToDec(BYTE(lpWorkBuf[13] ^ 0x80));
				f = BcdToDec(lpWorkBuf[14]);
			}
			else {
				m = BcdToDec(pDiscPerSector->data.present[12]);
				s = BcdToDec(pDiscPerSector->data.present[13]);
				f = BcdToDec(pDiscPerSector->data.present[14]);
			}
			INT tmpLBA = MSFtoLBA(m, s, f) - 150;
//			if (tmpLBA < nLBA) {
			if (tmpLBA != nLBA) {
				BYTE rm, rs, rf, mb, sb;
				LBAtoMSF(nLBA + 150, &rm, &rs, &rf);
				mb = rm;
				sb = rs;
				rm = DecToBcd(rm);
				rs = DecToBcd(rs);
				if (!pExtArg->byBe) {
					rm ^= 0x01;
					rs ^= 0x80;
				}
				lpWorkBuf[12] = rm;
				lpWorkBuf[13] = rs;
				lpWorkBuf[14] = DecToBcd(rf);
				OutputMainErrorWithLBALogA(
					"Original AMSF[%02u:%02u:%02u] -> Fixed AMSF[%02u:%02u:%02u]\n"
					, nLBA, byCurrentTrackNum, m, s, f, mb, sb, rf);
			}
			if (lpWorkBuf[15] != 0x61 && lpWorkBuf[15] != 0x62 && lpWorkBuf[15] != 0x01 && lpWorkBuf[15] != 0x02) {
				OutputMainErrorWithLBALogA("Original Mode[0x%02x] -> Fixed Mode[0x%02x]\n"
					, nLBA, byCurrentTrackNum, lpWorkBuf[15], pDiscPerSector->mainHeader.present[15]);
				lpWorkBuf[15] = pDiscPerSector->mainHeader.present[15];
			}
		}
	}
	else {
		if (pExtArg->byScanProtectViaFile && pDisc->PROTECT.byExist) {
			if ((pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize) ||
				(pDisc->PROTECT.byExist == microids && pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd <= nLBA &&
					nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd + pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd)) {
				OutputMainErrorWithLBALogA("Original Mode[0x%02x] -> Fixed Mode[0x%02x]\n"
					, nLBA, byCurrentTrackNum, lpWorkBuf[15] ,pDiscPerSector->mainHeader.present[15]);
				lpWorkBuf[15] = pDiscPerSector->mainHeader.present[15];
			}
			if ((pDisc->PROTECT.byExist == protectCDVOB &&
				(pDisc->SCSI.toc.TrackData[idx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
				pDiscPerSector->subQ.present.byCtl == AUDIO_DATA_TRACK)) {
				if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos == 0) {
					// 1st error sector
					pDisc->PROTECT.ERROR_SECTOR.nExtentPos = nLBA;
					pDisc->PROTECT.ERROR_SECTOR.nSectorSize = pDisc->SCSI.nAllLength - nLBA - 1;
				}
				// forced to set scrambled data to reserved byte
				OutputMainErrorWithLBALogA(
					"Original reserved byte[0x%02x%02x%02x%02x%02x%02x%02x%02x] "
					"-> Fixed reserved byte[0x486436ab56ff7ec0]\n"
					, nLBA, byCurrentTrackNum, lpWorkBuf[0x814], lpWorkBuf[0x815], lpWorkBuf[0x816], lpWorkBuf[0x817],
					lpWorkBuf[0x818], lpWorkBuf[0x819], lpWorkBuf[0x81a], lpWorkBuf[0x81b]);
				lpWorkBuf[0x814] = 0x48;
				lpWorkBuf[0x815] = 0x64;
				lpWorkBuf[0x816] = 0x36;
				lpWorkBuf[0x817] = 0xab;
				lpWorkBuf[0x818] = 0x56;
				lpWorkBuf[0x819] = 0xff;
				lpWorkBuf[0x81a] = 0x7e;
				lpWorkBuf[0x81b] = 0xc0;
			}
		}
	}
	UpdateTmpMainHeader(&pDiscPerSector->mainHeader,
		lpWorkBuf, pDiscPerSector->subQ.present.byCtl, nMainDataType);

	if (!bHeader) {
		if (pExtArg->byScanProtectViaFile && pDisc->PROTECT.byExist) {
			if ((pDisc->PROTECT.byExist == protectCDVOB &&
				(pDisc->SCSI.toc.TrackData[idx].Control & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK &&
				pDiscPerSector->subQ.present.byCtl == AUDIO_DATA_TRACK) ||
				(pDisc->PROTECT.ERROR_SECTOR.nExtentPos <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos + pDisc->PROTECT.ERROR_SECTOR.nSectorSize) ||
				(pDisc->PROTECT.byExist == microids && pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd + pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd)) {

				OutputMainErrorWithLBALogA(
					"This sector is data, but sync is invalid, so the header is generated\n"
					, nLBA, byCurrentTrackNum);
				OutputCDMain(fileMainError, lpWorkBuf, nLBA, MAINHEADER_MODE1_SIZE);
				memcpy(lpWorkBuf, pDiscPerSector->mainHeader.present, MAINHEADER_MODE1_SIZE);
				OutputCDMain(fileMainError, lpWorkBuf, nLBA, MAINHEADER_MODE1_SIZE);
			}
		}
		else {
			INT nOfs = pDisc->MAIN.nCombinedOffset;
			BYTE ctl = 0;
			INT nAdd = 0;

			if (-4704 <= nOfs && nOfs < -2352) {
				if (2 <= pExtArg->dwSubAddionalNum) {
					ctl = pDiscPerSector->subQ.nextNext.byCtl;
					nAdd += 2;
				}
			}
			else if (-2352 <= nOfs && nOfs < 0) {
				if (1 <= pExtArg->dwSubAddionalNum) {
					ctl = pDiscPerSector->subQ.next.byCtl;
					nAdd++;
				}
			}
			else if (0 <= nOfs && nOfs < 2352) {
				ctl = pDiscPerSector->subQ.present.byCtl;
			}
			else if (2352 <= nOfs && nOfs < 4704) {
				ctl = pDiscPerSector->subQ.prev.byCtl;
				nAdd--;
			}
			else if (4704 <= nOfs && nOfs < 7056) {
				ctl = pDiscPerSector->subQ.prevPrev.byCtl;
				nAdd -= 2;
			}
			if ((ctl & AUDIO_DATA_TRACK) == AUDIO_DATA_TRACK) {
				OutputMainErrorWithLBALogA(
					"This sector is data, but sync is invalid\n"
					, nLBA + nAdd, byCurrentTrackNum);
#if 1
				OutputCDMain(fileMainError, lpWorkBuf, nLBA + nAdd, MAINHEADER_MODE1_SIZE);
#endif
			}
		}
	}
}
