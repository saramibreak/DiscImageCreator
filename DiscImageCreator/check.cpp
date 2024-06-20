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
#include "check.h"
#include "convert.h"
#include "output.h"

// These global variable is declared at DiscImageCreator.cpp
extern BYTE g_aSyncHeader[SYNC_SIZE];
// This global variable is set if function is error
extern LONG s_lineNum;

BOOL IsXbox(
	PEXEC_TYPE pExecType
) {
	if (*pExecType == xbox || *pExecType == xboxswap ||
		*pExecType == xgd2swap || *pExecType == xgd3swap) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsCDorRelatedDisc(
	PEXEC_TYPE pExecType,
	PDISC pDisc
) {
	if (pDisc->SCSI.wCurrentMedia == ProfileCdrom ||
		pDisc->SCSI.wCurrentMedia == ProfileCdRecordable ||
		pDisc->SCSI.wCurrentMedia == ProfileCdRewritable ||
		(pDisc->SCSI.wCurrentMedia == ProfileInvalid && (*pExecType == gd)) ||
		pDisc->SCSI.wCurrentMedia == ProfilePlaystationCdrom ||
		pDisc->SCSI.wCurrentMedia == ProfilePlaystation2Cdrom) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsDVDorRelatedDisc(
	PDISC pDisc
) {
	if (pDisc->SCSI.wCurrentMedia == ProfileDvdRom ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdRecordable ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdRam ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdRewritable ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdRWSequential ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdDashRDualLayer ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdDashRLayerJump ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdPlusRW ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdPlusR ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdPlusRWDualLayer ||
		pDisc->SCSI.wCurrentMedia == ProfileDvdPlusRDualLayer ||
		pDisc->SCSI.wCurrentMedia == ProfileHDDVDRom ||
		pDisc->SCSI.wCurrentMedia == ProfileHDDVDRecordable ||
		pDisc->SCSI.wCurrentMedia == ProfileHDDVDRam ||
		pDisc->SCSI.wCurrentMedia == ProfileHDDVDRewritable ||
		pDisc->SCSI.wCurrentMedia == ProfileHDDVDRDualLayer ||
		pDisc->SCSI.wCurrentMedia == ProfileHDDVDRWDualLayer ||
		pDisc->SCSI.wCurrentMedia == ProfilePlaystation2DvdRom ||
		pDisc->SCSI.wCurrentMedia == ProfilePlaystation3DvdRom
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsBDorRelatedDisc(
	PDISC pDisc
) {
	if (pDisc->SCSI.wCurrentMedia == ProfileBDRom ||
		pDisc->SCSI.wCurrentMedia == ProfileBDRSequentialWritable ||
		pDisc->SCSI.wCurrentMedia == ProfileBDRRandomWritable ||
		pDisc->SCSI.wCurrentMedia == ProfileBDRewritable ||
		pDisc->SCSI.wCurrentMedia == ProfilePlaystation3BDRom ||
		pDisc->SCSI.wCurrentMedia == ProfilePlaystation4BDRom
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsCDR(
	PDISC pDisc
) {
	if (pDisc->SCSI.wCurrentMedia == ProfileCdRecordable ||
		pDisc->SCSI.wCurrentMedia == ProfileCdRewritable) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsCDiFormatWithMultiTrack(
	PDISC pDisc
) {
	if (pDisc->SCSI.byFormat == DISK_TYPE_CDI && pDisc->SCSI.toc.LastTrack > 1) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsAudioOnlyDisc(
	PDISC pDisc
) {
	if (pDisc->SCSI.trkType == TRACK_TYPE::audioTrack ||
		pDisc->SCSI.trkType == (TRACK_TYPE::audioTrack | TRACK_TYPE::pregapIn1stTrack | TRACK_TYPE::pregapAudioIn1stTrack)) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsDataDisc(
	PDISC pDisc
) {
	if (pDisc->SCSI.trkType & TRACK_TYPE::dataTrack ||
		pDisc->SCSI.trkType & TRACK_TYPE::pregapDataIn1stTrack) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsValidPS3Drive(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szVendorId, "SONY    ", DRIVE_VENDOR_ID_SIZE)) {
		if (!strncmp(pDevice->szProductId, "PS-SYSTEM   302R", DRIVE_PRODUCT_ID_SIZE)) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL IsValid0xF1SupportedDrive(
	PDEVICE pDevice
) {
	if (!pDevice->by0xF1Drive) {
		if (!strncmp(pDevice->szVendorId, "ASUS    ", DRIVE_VENDOR_ID_SIZE)) {
			if (!strncmp(pDevice->szProductId, "BW-16D1HT       ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "3.00", DRIVE_VERSION_ID_SIZE) ||
					!strncmp(pDevice->szProductRevisionLevel, "3.02", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			else if (!strncmp(pDevice->szProductId, "BW-12B1ST   a   ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			else if (!strncmp(pDevice->szProductId, "BC-12D2HT       ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "3.00", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
		}
		else if (!strncmp(pDevice->szVendorId, "HL-DT-ST", DRIVE_VENDOR_ID_SIZE)) {
			if (!strncmp(pDevice->szProductId, "DVDRAM GH24NSD1 ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "LG00", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			else if (!strncmp(pDevice->szProductId, "BDDVDRW UH12NS40", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  BH14NS40 ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE)) {
					if (!strncmp(pDevice->szVendorSpecific, "N1A11A1", 7)) {
						pDevice->by0xF1Drive = TRUE;
					}
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  BH14NS58 ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  BH16NS40 ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE)) {
					if (!strncmp(pDevice->szVendorSpecific, "N1A29A2", 7)) {
						pDevice->by0xF1Drive = TRUE;
					}
				}
				else if (!strncmp(pDevice->szProductRevisionLevel, "1.02", DRIVE_VERSION_ID_SIZE)) {
					if (!strncmp(pDevice->szVendorSpecific, "N000200", 7)) {
						pDevice->by0xF1Drive = TRUE;
					}
				}
				else if (!strncmp(pDevice->szProductRevisionLevel, "1.03", DRIVE_VERSION_ID_SIZE)) {
					if (!strncmp(pDevice->szVendorSpecific, "N0A03A0", 7)) {
						pDevice->by0xF1Drive = TRUE;
					}
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  BH16NS55 ", DRIVE_PRODUCT_ID_SIZE)) {
				// 1.02 can read the lead-out without 0xF1 https://github.com/saramibreak/DiscImageCreator/issues/227#issuecomment-1698389331
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE) ||
					!strncmp(pDevice->szProductRevisionLevel, "1.01", DRIVE_VERSION_ID_SIZE) ||
					!strncmp(pDevice->szProductRevisionLevel, "1.02", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  BH16NS58 ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  WH14NS40 ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE)) {
					if (!strncmp(pDevice->szVendorSpecific, "N001401", 7)) {
						pDevice->by0xF1Drive = TRUE;
					}
				}
				else if (!strncmp(pDevice->szProductRevisionLevel, "1.01", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
				else if (!strncmp(pDevice->szProductRevisionLevel, "1.02", DRIVE_VERSION_ID_SIZE)) {
					if (!strncmp(pDevice->szVendorSpecific, "N000100", 7)) {
						pDevice->by0xF1Drive = TRUE;
					}
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  WH16NS40 ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.02", DRIVE_VERSION_ID_SIZE)) {
					if (!strncmp(pDevice->szVendorSpecific, "N001401", 7)) {
						pDevice->by0xF1Drive = TRUE;
					}
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  WH16NS48 ", DRIVE_PRODUCT_ID_SIZE)) {
				if (!strncmp(pDevice->szProductRevisionLevel, "1.D3", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			else if (!strncmp(pDevice->szProductId, "BD-RE  WH16NS60 ", DRIVE_PRODUCT_ID_SIZE)) {
				// 1.02 doesn't support 0xF1 https://github.com/saramibreak/DiscImageCreator/issues/226#issuecomment-1698268874
				if (!strncmp(pDevice->szProductRevisionLevel, "1.00", DRIVE_VERSION_ID_SIZE)) {
					pDevice->by0xF1Drive = TRUE;
				}
			}
			// BD-RE BP60NB10 1.02 does not support https://github.com/saramibreak/DiscImageCreator/issues/280
		}
	}
	if (pDevice->by0xF1Drive && !pDevice->bCanReadLeadout) {
		// Using ASMedia SATA controller and linux, 0xF1 drive can read the lead-out sector using 0xbe
		// https://github.com/saramibreak/DiscImageCreator/issues/227#issuecomment-1763596504
		return TRUE;
	}
	return FALSE;
}

BOOL IsValidAsusDriveWith310(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szVendorId, "ASUS    ", DRIVE_VENDOR_ID_SIZE)) {
		if (!strncmp(pDevice->szProductId, "BW-16D1HT       ", DRIVE_PRODUCT_ID_SIZE)) {
			if (!strncmp(pDevice->szProductRevisionLevel, "3.10", DRIVE_VERSION_ID_SIZE)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL IsValidPlextorDrive(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szVendorId, "PLEXTOR ", DRIVE_VENDOR_ID_SIZE)) {
		if (!strncmp(pDevice->szProductId, "DVDR   PX-760A  ", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.07", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX760A;
				pExtArg->uiC2Offset = 295;
			}
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-755A  ", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.08", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX755A;
				pExtArg->uiC2Offset = 295;
			}
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-716AL ", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.02", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX716AL;
				pExtArg->uiC2Offset = 295;
			}
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-716A  ", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.11", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX716A;
				pExtArg->uiC2Offset = 295;
			}
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-714A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX714A;
			pExtArg->uiC2Offset = 295;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-712A  ", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.09", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX712A;
				pExtArg->uiC2Offset = 295;
			}
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-708A2 ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX708A2;
			pExtArg->uiC2Offset = 294;
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-708A  ", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.12", DRIVE_VERSION_ID_SIZE) &&
				strncmp(pDevice->szProductRevisionLevel, "1.04", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX708A;
				pExtArg->uiC2Offset = 294;
			}
		}
		else if (!strncmp(pDevice->szProductId, "DVDR   PX-704A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX704A;
			pExtArg->uiC2Offset = 294;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-320A  ", DRIVE_PRODUCT_ID_SIZE)) {
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PX320A;
			pExtArg->uiC2Offset = 294;
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PREMIUM2 ", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.03", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PREMIUM2;
				pExtArg->uiC2Offset = 294;
			}
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PREMIUM  ", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.07", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PREMIUM;
				pExtArg->uiC2Offset = 294;
			}
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W5224A", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.04", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW5224A;
				pExtArg->uiC2Offset = 294;
			}
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4824A", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.07", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW4824A;
				pExtArg->uiC2Offset = 294;
			}
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4012A", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.07", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW4012A;
				pExtArg->uiC2Offset = 294;
			}
		}
		else if (!strncmp(pDevice->szProductId, "CD-R   PX-W4012S", DRIVE_PRODUCT_ID_SIZE)) {
			if (strncmp(pDevice->szProductRevisionLevel, "1.06", DRIVE_VERSION_ID_SIZE)) {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::NotLatest;
			}
			else {
				pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::PXW4012S;
				pExtArg->uiC2Offset = 294;
			}
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
			pDevice->byPlxtrDrive = PLXTR_DRIVE_TYPE::Other;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL IsPregapOfTrack1ReadableDrive(
	PDEVICE pDevice
) {
	BOOL bRet = TRUE;
	if (pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX760A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX755A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX716AL &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX716A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX714A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX712A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX708A2 &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX708A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX704A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX320A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PREMIUM2 &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PREMIUM &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXW5224A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXW4012A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXW4012S &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXW2410A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXS88T &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXW1610A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXW8220T &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PXR820T
		) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsPlextorDVDDrive(
	PDEVICE pDevice
) {
	BOOL bRet = TRUE;
	if (pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX760A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX755A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX716AL &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX716A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX714A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX712A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX708A2 &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX708A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX704A
		) {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsPlextor712OrNewer(
	PDEVICE pDevice
) {
	BOOL bRet = TRUE;
	if (pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX760A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX755A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX716AL &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX716A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX714A &&
		pDevice->byPlxtrDrive != PLXTR_DRIVE_TYPE::PX712A
		) {
		bRet = FALSE;
	}
	return bRet;
}

VOID SupportIndex0InTrack1(
	PEXT_ARG pExtArg,
	PDEVICE pDevice
) {
	if (!IsPregapOfTrack1ReadableDrive(pDevice)) {
		OutputString(
			"This drive doesn't support to rip from 00:00:00 to 00:01:74 AMSF. /p option was ignored\n");
		pExtArg->byPre = FALSE;
	}
}

BOOL IsEnoughDiskSpaceForDump(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	_TCHAR* pszPath
) {
	ULARGE_INTEGER  ui64Used;
	ULARGE_INTEGER  ui64Free;
	ULARGE_INTEGER  ui64Avail;
	ULARGE_INTEGER  ui64Total;
	BOOL bRet = FALSE;

	GetDiskFreeSpaceEx(pszPath, &ui64Free, &ui64Total, &ui64Avail);
	ui64Used.QuadPart = (ui64Total.QuadPart - ui64Avail.QuadPart);

	OutputString(
		"DiskSize of [%s]\n"
		"\tTotal: %14llu bytes\n"
		"\t Used: %14llu bytes\n"
		"\t---------------------------\n"
		"\tSpace: %14llu bytes\n"
		, pszPath, ui64Total.QuadPart, ui64Used.QuadPart, ui64Avail.QuadPart
	);
	CONST INT nAllLogsFileSize = 1000000000;
	if ((IsCDorRelatedDisc(pExecType, pDisc) && ui64Avail.QuadPart > (ULONGLONG)(pDisc->SCSI.nAllLength * 3 + nAllLogsFileSize)) ||
		(IsDVDorRelatedDisc(pDisc) && ui64Avail.QuadPart > (ULONGLONG)(pDisc->SCSI.nAllLength + nAllLogsFileSize)) ||
		(IsBDorRelatedDisc(pDisc) && ui64Avail.QuadPart > (ULONGLONG)(pDisc->SCSI.nAllLength + nAllLogsFileSize))
		) {
		OutputString("\t => There is enough disk space for dumping\n");
		bRet = TRUE;
	}
	else if (*pExecType == fd || *pExecType == disk) {
		bRet = TRUE;
	}
	else {
		OutputString("\t => There is not enough disk space for dumping\n");
	}
	return bRet;
}

BOOL IsValidMainDataHeader(
	LPBYTE lpBuf
) {
	BOOL bRet = TRUE;
	for (size_t c = 0; c < sizeof(g_aSyncHeader); c++) {
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
	for (size_t c = 0; c < sizeof(a3doHeader); i++, c++) {
		if (lpBuf[i] != a3doHeader[c]) {
			bRet = FALSE;
			break;
		}
	}
	if (bRet) {
		for (i = 132; i < 2048; i += 8) {
			if (strncmp((LPCCH)&lpBuf[i], "duck", 4) &&
				strncmp((LPCCH)&lpBuf[i + 4], "iama", 4)) {
				bRet = FALSE;
				break;
			}
		}
	}
	return bRet;
}

BOOL IsFat(
	LPBYTE lpBuf
) {
	if ((lpBuf[0] == 0xeb && lpBuf[2] == 0x90) || lpBuf[0] == 0xe9) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsExFat(
	LPBYTE lpBuf
) {
	if (lpBuf[0] == 0xeb && lpBuf[1] == 0x76 && lpBuf[2] == 0x90) {
		return TRUE;
	}
	return FALSE;
}
BOOL IsDriverDescriptorRecord(
	LPBYTE lpBuf
) {
	if (lpBuf[0] == 0x45 && lpBuf[1] == 0x52) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsApplePartionMap(
	LPBYTE lpBuf
) {
	if (lpBuf[0] == 0x50 && lpBuf[1] == 0x4d) {
		return TRUE;
	}
	return FALSE;
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
	for (size_t i = 0; i < sizeof(warningStr); i++) {
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
	BOOL bRet = FALSE;
	if (!strncmp((LPCCH)&lpBuf[0], "PC-FX:Hu_CD-ROM ", 16)) {
		bRet = TRUE;
	}
	// Super PCEngine Fan Deluxe - Special CD-ROM Vol. 1 (Japan)
	else if (!strncmp((LPCCH)&lpBuf[1], "UDSON CD-EMUL2 ", 15)) {
		bRet = TRUE;
	}
	return bRet;
}

BOOL IsValidPregapSector(
	PDISC pDisc,
	PSUBCH pSubch,
	INT nLBA
) {
	BOOL bRet = FALSE;
	if (/*(pSubch->current.byCtl & AUDIO_DATA_TRACK) == 0 &&*/
		(pSubch->next.byCtl & AUDIO_DATA_TRACK) == 0 &&
		pSubch->next.byIndex == 0) {
		if (nLBA == pDisc->SCSI.lp1stLBAListOnToc[pSubch->prev.byTrackNum] - 225 ||
			nLBA == pDisc->SCSI.lp1stLBAListOnToc[pSubch->prev.byTrackNum] - 150 ||
			nLBA == pDisc->SCSI.lp1stLBAListOnToc[pSubch->prev.byTrackNum] - 149) {
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
		if ((10000 <= nLBA && nLBA < 20000) || (40000 <= nLBA && nLBA < 50000)) {
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL IsValidSecuRomSector(
	BOOL bSecuRom,
	PDISC pDisc,
	INT nLBA
) {
	BOOL bRet = FALSE;
	if (bSecuRom) {
		if (pDisc->PROTECT.byExist == securomV1) {
			if (30000 <= nLBA && nLBA < 50000) {
				bRet = TRUE;
			}
		}
		else if (pDisc->PROTECT.byExist == securomV3_1 || pDisc->PROTECT.byExist == securomV3_2 || pDisc->PROTECT.byExist == securomV3_3) {
			if ((0 <= nLBA && nLBA < 8) || (5000 <= nLBA && nLBA < 25000)) {
				bRet = TRUE;
			}
		}
		else if (pDisc->PROTECT.byExist == securomV2 ||
			pDisc->PROTECT.byExist == securomV4) {
			if (5000 <= nLBA && nLBA < 25000) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidProtectedSector(
	PDISC pDisc,
	INT nLBA,
	INT idx
) {
	BOOL bRet = FALSE;
	if (pDisc->PROTECT.byExist != c2Err) {
		if ((pDisc->PROTECT.byExist && pDisc->PROTECT.ERROR_SECTOR.nExtentPos[idx] <= nLBA &&
			nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos[idx] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[idx]) ||
			(pDisc->PROTECT.byExist == datelAlt
				&& pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd <= nLBA &&
				nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos2nd + pDisc->PROTECT.ERROR_SECTOR.nSectorSize2nd)) {
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL IsSafeDiscErrorNum(
	UINT uiErrorNum
) {
	BOOL bRet = FALSE;
	// 288 and 264 and 240 are reentrant's info
	if (uiErrorNum == 312 || uiErrorNum == 288 || uiErrorNum == 264 || uiErrorNum == 240) {
		bRet = TRUE;
	}
	return bRet;
}

BOOL IsValidSafeDiscSector(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector
) {
	BOOL bRet = FALSE;
	if ((pDisc->PROTECT.byExist == safeDisc || pDisc->PROTECT.byExist == safeDiscLite) &&
		 IsSafeDiscErrorNum(pDiscPerSector->uiC2errorNum)) {
		bRet = TRUE;
	}
	return bRet;
}

BOOL IsValidIntentionalC2error(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	INT idx
) {
	BOOL bRet = FALSE;
	if (pDisc->PROTECT.ERROR_SECTOR.nExtentPos[idx] <= nLBA &&
		nLBA <= pDisc->PROTECT.ERROR_SECTOR.nExtentPos[idx] + pDisc->PROTECT.ERROR_SECTOR.nSectorSize[idx]) {
		if (pDisc->PROTECT.byExist == codelock ||
			pDisc->PROTECT.byExist == datel ||
			pDisc->PROTECT.byExist == datelAlt ||
			pDisc->PROTECT.byExist == c2Err ||
			IsValidSafeDiscSector(pDisc, pDiscPerSector)) {
			bRet = TRUE;
		}
	}
	return bRet;
}

BOOL IsCheckingSubChannel(
	PDISC pDisc,
	INT nLBA
) {
	BOOL bCheckSub = TRUE;
	if ((pDisc->SCSI.n1stLBAofLeadout <= nLBA && nLBA < pDisc->SCSI.n1stLBAof2ndSession)) {
		bCheckSub = FALSE;
	}
	return bCheckSub;
}

BOOL IsValidSubQAdrMCN(
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
	if ((lpSubcode[19] & 0x0f) != 0 || lpSubcode[20] != 0) {
		bRet = FALSE;
	}
	return bRet;
}

// reference
// https://isrc.jmd.ne.jp/about/pattern.html
// https://isrc.jmd.ne.jp/about/error.html
BOOL IsValidSubQAdrISRC(
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
	UINT uiSubAdditionalNum,
	PSUBCH pSubch,
	INT nRangeLBA,
	INT n1stLBA,
	INT nPrevAdrSector,
	INT nLBA
) {
	BOOL bRet = FALSE;
	INT idx = (nLBA - n1stLBA) / nRangeLBA;
	INT nTmpLBA = n1stLBA + nRangeLBA * idx;
	if (nLBA < 0 ||
		(nLBA == nTmpLBA) ||
		(nLBA - nPrevAdrSector == nRangeLBA)) {
		if (1 <= uiSubAdditionalNum) {
			if (pSubch->next.byAdr != ADR_ENCODES_MEDIA_CATALOG &&
				pSubch->next.byAdr != ADR_ENCODES_ISRC) {
				bRet = TRUE;
			}
		}
		else {
			bRet = TRUE;
		}
	}
	else if (nLBA + 1 == nTmpLBA && pSubch->prev.byAdr == ADR_ENCODES_CURRENT_POSITION) {
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

BOOL IsValidSubQCtl(
	PSUBCH pSubch,
	BYTE byEndCtl
) {
	BOOL bRet = TRUE;
	switch (pSubch->current.byCtl) {
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

	if (pSubch->prev.byCtl != pSubch->current.byCtl) {
		if ((pSubch->current.byCtl != byEndCtl) && pSubch->current.byCtl != 0) {
			s_lineNum = __LINE__;
			bRet = FALSE;
		}
		else if (pSubch->prev.byCtl == pSubch->next.byCtl) {
			s_lineNum = __LINE__;
			bRet = FALSE;
		}
		else {
			if (pSubch->current.byAdr == ADR_ENCODES_CURRENT_POSITION) {
				if (pSubch->prev.byTrackNum + 1 == pSubch->current.byTrackNum) {
					if (pSubch->prev.nRelativeTime + 1 == pSubch->current.nRelativeTime) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
				}
				else if (pSubch->prev.byTrackNum == pSubch->current.byTrackNum &&
					pSubch->prev.byIndex == pSubch->current.byIndex) {
					if (pSubch->prev.nRelativeTime + 1 == pSubch->current.nRelativeTime) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
				}
			}
			// EVE - burst error (Disc 3) (Terror Disc)
			// LBA[188021, 0x2DE75],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:71], AMSF[41:48:71], RtoW[0, 0, 0, 0]
			// LBA[188022, 0x2DE76], Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :72], RtoW[0, 0, 0, 0]
			// LBA[188023, 0x2DE77],  Data,      Copy NG,                  Track[01], Idx[01], RMSF[41:46:73], AMSF[41:48:73], RtoW[0, 0, 0, 0]
			else if (pSubch->current.byAdr == ADR_ENCODES_MEDIA_CATALOG) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			if (pSubch->prevPrev.byCtl == pSubch->current.byCtl) {
				bRet = TRUE;
			}
		}
	}
	return bRet;
}

BOOL IsValidSubQIdx(
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	LPBOOL bPrevIndex,
	LPBOOL bPrevPrevIndex
) {
	if (nLBA < 1) {
		return TRUE;
	}
	else if (MAXIMUM_NUMBER_INDEXES < pDiscPerSector->subch.current.byIndex) {
		s_lineNum = __LINE__;
		return FALSE;
	}
	BOOL bRet = TRUE;
	if (pDiscPerSector->subch.prev.byIndex != pDiscPerSector->subch.current.byIndex) {
		if (nLBA != pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->byTrackNum]) {
			if (pDiscPerSector->subch.prev.byIndex + 1 < pDiscPerSector->subch.current.byIndex) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pDiscPerSector->subch.next.byTrackNum > 0 &&
				pDiscPerSector->subch.current.byIndex - 1 == pDiscPerSector->subch.next.byIndex &&
				pDiscPerSector->subch.prev.byIndex == pDiscPerSector->subch.next.byIndex &&
				pDiscPerSector->subch.next.byAdr == ADR_ENCODES_CURRENT_POSITION) {
				// 1552 Tenka Tairan
				// LBA[126959, 0x1efef], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[01], RMSF[01:50:13], AMSF[28:14:59], RtoW[0, 0, 0, 0]
				// LBA[126960, 0x1eff0], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[02], RMSF[01:50:14], AMSF[28:14:60], RtoW[0, 0, 0, 0]
				// LBA[126961, 0x1eff1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[14], Idx[01], RMSF[01:50:15], AMSF[28:14:61], RtoW[0, 0, 0, 0]
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pDiscPerSector->subch.nextNext.byTrackNum > 0 &&
				pDiscPerSector->subch.prev.byIndex + 1 == pDiscPerSector->subch.current.byIndex &&
				pDiscPerSector->subch.prev.byIndex == pDiscPerSector->subch.nextNext.byIndex) {
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
				if (pDiscPerSector->subch.prev.byIndex == 0 && pDiscPerSector->subch.current.byIndex == 1 &&
					pDiscPerSector->subch.current.nRelativeTime == 0) {
					bRet = TRUE;
				}
				else if ((pDiscPerSector->subch.current.byIndex == 0 || pDiscPerSector->subch.current.byIndex == 1) &&
					pDiscPerSector->subch.prev.byTrackNum + 1 == pDiscPerSector->subch.current.byTrackNum &&
					pDiscPerSector->subch.current.nRelativeTime < pDiscPerSector->subch.prev.nRelativeTime) {
					bRet = TRUE;
				}
				// multi index sector
				else if (pDiscPerSector->subch.prev.byIndex + 1 == pDiscPerSector->subch.current.byIndex &&
					pDiscPerSector->subch.prev.nRelativeTime + 1 == pDiscPerSector->subch.current.nRelativeTime) {
					bRet = TRUE;
				}
				// first pregap sector
				else if (pDiscPerSector->subch.prev.byIndex == 1 && pDiscPerSector->subch.current.byIndex == 0) {
					if (pDiscPerSector->subch.current.nRelativeTime - 1 == pDiscPerSector->subch.next.nRelativeTime) {
						bRet = TRUE;
					}
					// Shanghai - Matekibuyu (Japan)
					// LBA[016447, 0x0403f], Audio, 2ch, Copy NG, Pre-emphasis No, Track[04], Idx[01], RMSF[00:16:11], AMSF[03:41:22], RtoW[0, 0, 0, 0]
					// LBA[016448, 0x04040], Audio, 2ch, Copy NG, Pre-emphasis No, Track[05], Idx[00], RMSF[00:21:74], AMSF[03:41:23], RtoW[0, 0, 0, 0]
					// LBA[016449, 0x04041], Audio, 2ch, Copy NG, Pre-emphasis No, Track[05], Idx[00], RMSF[00:01:73], AMSF[03:41:24], RtoW[0, 0, 0, 0]
					else if (IsValidPregapSector(pDisc, &pDiscPerSector->subch, nLBA)) {
						bRet = TRUE;
					}
				}
				if (pDiscPerSector->subch.prevPrev.byIndex == pDiscPerSector->subch.current.byIndex) {
					bRet = TRUE;
					if (pDiscPerSector->subch.prev.byIndex - 1 == pDiscPerSector->subch.current.byIndex) {
						*bPrevIndex = FALSE;
					}
				}
			}
		}
		else {
			// Tomb Raider II (Rayovac)
			// LBA[288601, 0x46759]: P[ff], Q[015401032941006410010d09]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[54], Idx[01], RMSF[03:29:41], AMSF[64:10:01]}, RtoW[0, 0, 0, 0]
			// LBA[288602, 0x4675a]: P[ff], Q[015581000000006410023599]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[55], Idx[81], RMSF[00:00:00], AMSF[64:10:02]}, RtoW[0, 0, 0, 0]
			// LBA[288603, 0x4675b]: P[00], Q[015501000001006410037268]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[55], Idx[01], RMSF[00:00:01], AMSF[64:10:03]}, RtoW[0, 0, 0, 0]
			if (1 < pDiscPerSector->subch.current.byIndex) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
		}
	}
	else if (pDiscPerSector->subch.prev.byIndex == pDiscPerSector->subch.current.byIndex &&
		pDiscPerSector->subch.prevPrev.byIndex - 1 == pDiscPerSector->subch.current.byIndex &&
		pDiscPerSector->subch.prevPrev.byTrackNum + 1 != pDiscPerSector->subch.current.byTrackNum) {
		*bPrevPrevIndex = FALSE;
	}
	return bRet;
}

BOOL IsValidSubQTrack(
	PEXEC_TYPE pExecType,
	PDISC pDisc,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA,
	LPBOOL bPrevTrackNum
) {
	if (*pExecType != swap && pDisc->SCSI.toc.LastTrack < pDiscPerSector->subch.current.byTrackNum) {
		s_lineNum = __LINE__;
		return FALSE;
	}
	else if (pDiscPerSector->subch.next.byAdr == ADR_NO_MODE_INFORMATION &&
		pDiscPerSector->subch.next.byTrackNum > 0 &&
		pDiscPerSector->subch.next.byTrackNum < pDiscPerSector->subch.current.byTrackNum) {
		s_lineNum = __LINE__;
		return FALSE;
	}
	BOOL bRet = TRUE;
	if (pDiscPerSector->subch.prev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
		pDiscPerSector->subch.prev.byTrackNum != pDiscPerSector->subch.current.byTrackNum) {
		if (pDiscPerSector->subch.prev.byTrackNum + 2 <= pDiscPerSector->subch.current.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
		else if (pDiscPerSector->subch.current.byTrackNum < pDiscPerSector->subch.prev.byTrackNum) {
			if (pDiscPerSector->subch.prevPrev.byTrackNum == pDiscPerSector->subch.current.byTrackNum) {
				*bPrevTrackNum = FALSE;
			}
			else {
				s_lineNum = __LINE__;
				return FALSE;
			}
		}
		else {
			if (pDiscPerSector->subch.prev.byTrackNum + 1 == pDiscPerSector->subch.current.byTrackNum) {
				if (*pExecType != swap && nLBA != pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->byTrackNum]) {
					// Super CD-ROM^2 Taiken Soft Shuu (Japan)
					// LBA[139289, 0x22019], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:00], AMSF[30:59:14], RtoW[0, 0, 0, 0]
					// LBA[139290, 0x2201a], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:01], AMSF[30:59:15], RtoW[0, 0, 0, 0]
					// LBA[139291, 0x2201b], Audio, 2ch, Copy NG, Pre-emphasis No, Track[17], Idx[01], RMSF[01:19:02], AMSF[30:59:16], RtoW[0, 0, 0, 0]
					// LBA[139292, 0x2201c], Audio, 2ch, Copy NG, Pre-emphasis No, Track[16], Idx[01], RMSF[01:19:03], AMSF[30:59:17], RtoW[0, 0, 0, 0]
					if ((pDiscPerSector->subch.prev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
						pDiscPerSector->subch.prev.nRelativeTime + 1 == pDiscPerSector->subch.current.nRelativeTime) ||
						(pDiscPerSector->subch.prevPrev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
							pDiscPerSector->subch.prevPrev.nRelativeTime + 2 == pDiscPerSector->subch.current.nRelativeTime)) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
					// Bangai-O (Europe) [GD]
					// LBA[361151, 0x582bf]: P[ff], Q[01220000015900801726ecd5]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[00], RMSF[00:01:59], AMSF[80:17:26]}, RtoW[0, 0, 0, 0]
					// LBA[361152, 0x582c0]: P[ff], Q[01230000015800801727bd86]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[23], Idx[00], RMSF[00:01:58], AMSF[80:17:27]}, RtoW[0, 0, 0, 0]
					// LBA[361153, 0x582c1]: P[ff], Q[01220000015700801728c2b3]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[00], RMSF[00:01:57], AMSF[80:17:28]}, RtoW[0, 0, 0, 0]
					//  :
					// LBA[361285, 0x58345]: P[ff], Q[0122000000000080191061a1]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[00], RMSF[00:00:00], AMSF[80:19:10]}, RtoW[0, 0, 0, 0]
					// LBA[361286, 0x58346]: P[ff], Q[012201000000008019113653]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[00:00:00], AMSF[80:19:11]}, RtoW[0, 0, 0, 0]
					// LBA[361287, 0x58347]: P[00], Q[01220100000100801912ac61]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[00:00:01], AMSF[80:19:12]}, RtoW[0, 0, 0, 0]
					else if (nLBA < pDisc->SCSI.lp1stLBAListOnToc[pDiscPerSector->subch.prev.byTrackNum] &&
						(pDiscPerSector->subch.prev.nRelativeTime - 1 == pDiscPerSector->subch.current.nRelativeTime)) {
						s_lineNum = __LINE__;
						bRet = FALSE;
					}
					else if (pDiscPerSector->subch.next.byAdr == ADR_ENCODES_CURRENT_POSITION && pDiscPerSector->subch.next.byTrackNum > 0) {
						// Ys III (Japan)
						// LBA[226292, 0x373f4], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[01], RMSF[00:37:62], AMSF[50:19:17], RtoW[0, 0, 0, 0]
						// LBA[226293, 0x373f5], Audio, 2ch, Copy NG, Pre-emphasis No, Track[21], Idx[01], RMSF[00:37:63], AMSF[50:19:18], RtoW[0, 0, 0, 0]
						// LBA[226294, 0x373f6], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[01], RMSF[00:37:64], AMSF[50:19:19], RtoW[0, 0, 0, 0]
						if (pDiscPerSector->subch.current.byTrackNum != pDiscPerSector->subch.next.byTrackNum) {
							// Sega Flash Vol. 3 (Eu)
							// LBA[221184, 0x36000], Audio, 2ch, Copy NG, Pre-emphasis No, Track[30], Idx[01], RMSF[00:05:00], AMSF[49:11:09], RtoW[0, 0, 0, 0]
							// LBA[221185, 0x36001], Audio, 2ch, Copy NG, Pre-emphasis No, Track[31], Idx[01], RMSF[00:01:74], AMSF[49:11:10], RtoW[0, 0, 0, 0]
							// LBA[221186, 0x36002], Audio, 2ch, Copy NG, Pre-emphasis No, Track[11], Idx[01], RMSF[00:01:73], AMSF[49:11:11], RtoW[0, 0, 0, 0]
							// LBA[221187, 0x36003], Audio, 2ch, Copy NG, Pre-emphasis No, Track[31], Idx[00], RMSF[00:01:72], AMSF[49:11:12], RtoW[0, 0, 0, 0]
							if (pDiscPerSector->subch.prev.byTrackNum != pDiscPerSector->subch.next.byTrackNum) {
								if (pDiscPerSector->subch.nextNext.byAdr == ADR_ENCODES_CURRENT_POSITION && pDiscPerSector->subch.nextNext.byTrackNum > 0) {
									if (pDiscPerSector->subch.current.byTrackNum != pDiscPerSector->subch.nextNext.byTrackNum) {
										s_lineNum = __LINE__;
										bRet = FALSE;
									}
									else {
										OutputSubInfoWithLBALog(
											"This track num is maybe incorrect. Could you try /s 2 option [L:%d]\n", nLBA, pDiscPerSector->byTrackNum, (INT)__LINE__);
									}
								}
								else {
									BOOL bPrevIndex, bPrevPrevIndex = FALSE;
									BOOL bIdx = IsValidSubQIdx(pDisc, pDiscPerSector, nLBA, &bPrevIndex, &bPrevPrevIndex);
									BOOL bCtl = IsValidSubQCtl(&pDiscPerSector->subch, pDisc->SUB.lpEndCtlList[pDiscPerSector->subch.current.byTrackNum - 1]);
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
										OutputSubInfoWithLBALog(
											"This track num is maybe incorrect. Could you try /s 2 option [L:%d]\n", nLBA, pDiscPerSector->byTrackNum, (INT)__LINE__);
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
					else if ((pDiscPerSector->subch.next.byAdr == ADR_ENCODES_MEDIA_CATALOG ||
						pDiscPerSector->subch.next.byAdr == ADR_ENCODES_ISRC) && pDiscPerSector->subch.nextNext.byTrackNum > 0) {
						if (pDiscPerSector->subch.current.byTrackNum != pDiscPerSector->subch.nextNext.byTrackNum) {
							s_lineNum = __LINE__;
							bRet = FALSE;
						}
					}
					else {
						BOOL bPrevIndex, bPrevPrevIndex = FALSE;
						BOOL bIdx = IsValidSubQIdx(pDisc, pDiscPerSector, nLBA, &bPrevIndex, &bPrevPrevIndex);
						BOOL bCtl = IsValidSubQCtl(&pDiscPerSector->subch, pDisc->SUB.lpEndCtlList[pDiscPerSector->subch.current.byTrackNum - 1]);
						// Garou Densetsu Special [PCE]
						// LBA[294048, 0x47ca0], Audio, 2ch, Copy NG, Pre-emphasis No, Track[22], Idx[01], RMSF[02:57:31], AMSF[65:22:48], RtoW[0, 0, 0, 0]
						// LBA[294049, 0x47ca1], Audio, 2ch, Copy NG, Pre-emphasis No, Track[23], Idx[20], RMSF[00:27:62], AMSF[06:70:07], RtoW[0, 0, 0, 0]
						// LBA[294050, 0x47ca2], Audio, 2ch, Copy NG, Pre-emphasis No, Track[20], Idx[81], RMSF[02:134:49], AMSF[65:19:30], RtoW[0, 0, 0, 0]
						// LBA[294051, 0x47ca3], Audio, 2ch, Copy NG, Pre-emphasis No, Track[119], Idx[121], RMSF[57:117:165], AMSF[165:161:121], RtoW[0, 0, 0, 0]
						if ((!bIdx || !bCtl) && pDiscPerSector->subch.current.byTrackNum != pDiscPerSector->subch.next.byTrackNum) {
							s_lineNum = __LINE__;
							bRet = FALSE;
						}
						// Kuusou Kagaku Sekai Gulliver Boy (Japan)
						// LBA[159095, 0x26d77],  Data,      Copy NG,                  Track[02], Idx[73], RMSF[00:35:23], AMSF[83:113:41], RtoW[0, 0, 0, 0]
						// LBA[159096, 0x26d78],  Data,      Copy NG,                  Track[02], Idx[74], RMSF[00:35:23], AMSF[123:23:41], RtoW[0, 0, 0, 0]
						// LBA[159097, 0x26d79],  Data,      Copy NG,                  Track[03], Idx[00], RMSF[00:35:23], AMSF[82:22:41], RtoW[0, 0, 0, 0]
						// LBA[159098, 0x26d7a],  Data,      Copy NG,                  Track[03], Idx[01], RMSF[00:35:23], AMSF[146:34:41], RtoW[0, 0, 0, 0]
						else if (!bIdx || !bCtl || pDiscPerSector->subch.current.byIndex == 0) {
							s_lineNum = __LINE__;
							bRet = FALSE;
						}
					}
				}
				else {
					// todo
				}
			}
			if (pDiscPerSector->subch.prevPrev.byTrackNum == pDiscPerSector->subch.current.byTrackNum) {
				bRet = TRUE;
			}
		}
	}
	else if (pDiscPerSector->subch.prevPrev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
		pDiscPerSector->subch.prevPrev.byTrackNum != pDiscPerSector->subch.current.byTrackNum) {
		// Sonic CD (USA)
		// LBA[089657, 0x15e39]: P[00], Q[01070100165000195732b85e]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[07], Idx[01], RMSF[00:16:50], AMSF[19:57:32]}, RtoW[0, 0, 0, 0]
		// LBA[089658, 0x15e3a]: P[00], Q[01070100165100195733022e]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[07], Idx[01], RMSF[00:16:51], AMSF[19:57:33]}, RtoW[0, 0, 0, 0]
		// LBA[089659, 0x15e3b]: P[00], Q[0200000000000000003457a2]{Audio, 2ch, Copy NG, Pre-emphasis No, MediaCatalogNumber [0000000000000], AMSF[     :34]}, RtoW[0, 0, 0, 0]
		// LBA[089660, 0x15e3c]: P[00], Q[01060100165300195735cd48]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[06], Idx[01], RMSF[00:16:53], AMSF[19:57:35]}, RtoW[0, 0, 0, 0]
		// LBA[089661, 0x15e3d]: P[00], Q[0107010016540019573671dc]{Audio, 2ch, Copy NG, Pre-emphasis No, Track[07], Idx[01], RMSF[00:16:54], AMSF[19:57:36]}, RtoW[0, 0, 0, 0]
		if (pDiscPerSector->subch.prevPrev.byTrackNum > pDiscPerSector->subch.current.byTrackNum) {
			s_lineNum = __LINE__;
			return FALSE;
		}
		else if (pDiscPerSector->subch.prevPrev.byTrackNum + 2 <= pDiscPerSector->subch.current.byTrackNum ||
			pDiscPerSector->subch.prevPrev.byTrackNum == 110) {
			s_lineNum = __LINE__;
			return FALSE;
		}
	}
	return bRet;
}

BOOL IsValidBCD(
	BYTE bySrc
) {
	if (((bySrc >> 4) & 0x0f) > 9) {
		return FALSE;
	}
	if ((bySrc & 0x0f) > 9) {
		return FALSE;
	}
	return TRUE;
}

BOOL IsValidSubQMSF(
	PEXEC_TYPE pExecType,
	LPBYTE lpSubcode,
	BYTE m,
	BYTE s,
	BYTE f
) {
	if (*pExecType == gd) {
		if (lpSubcode[m] > 0xc2) {
			return FALSE;
		}
	}
	else {
		if (!IsValidBCD(lpSubcode[m])) {
			return FALSE;
		}
		else if (lpSubcode[m] > 0x99) {
			return FALSE;
		}
	}

	if (!IsValidBCD(lpSubcode[s])) {
		return FALSE;
	}
	else if (lpSubcode[s] > 0x59) {
		return FALSE;
	}

	if (!IsValidBCD(lpSubcode[f])) {
		return FALSE;
	}
	else if (lpSubcode[f] > 0x74) {
		return FALSE;
	}
	return TRUE;
}

BOOL IsValidSubQRMSF(
	PEXEC_TYPE pExecType,
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
) {
	BOOL bRet = IsValidSubQMSF(pExecType, pDiscPerSector->subcode.current, 15, 16, 17);
	if (!bRet) {
		s_lineNum = __LINE__;
		return bRet;
	}
	INT tmpLBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.current[15]), 
		BcdToDec(pDiscPerSector->subcode.current[16]), BcdToDec(pDiscPerSector->subcode.current[17]));
	if (tmpLBA != 0) {
		if (pDiscPerSector->subch.current.byIndex > 0) {
			if (pDiscPerSector->subch.prev.byAdr == ADR_ENCODES_CURRENT_POSITION &&
				pDiscPerSector->subch.prev.nRelativeTime != 0 && pDiscPerSector->subch.current.nRelativeTime != 0 &&
				pDiscPerSector->subch.prev.nRelativeTime + 1 != pDiscPerSector->subch.current.nRelativeTime) {
				s_lineNum = __LINE__;
				bRet = FALSE;
			}
			else if (pDiscPerSector->subch.prev.byIndex > 0 &&
				pDiscPerSector->subch.prev.nRelativeTime + 1 != pDiscPerSector->subch.current.nRelativeTime) {
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
			if (pDiscPerSector->subch.prevPrev.nRelativeTime + 2 == pDiscPerSector->subch.current.nRelativeTime) {
				bRet = TRUE;
			}
		}
		else if (pDiscPerSector->subch.current.byIndex == 0) {
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
			if (pDiscPerSector->subch.prev.byTrackNum == pDiscPerSector->subch.current.byTrackNum &&
				pDiscPerSector->subch.prev.nRelativeTime != pDiscPerSector->subch.current.nRelativeTime + 1) {
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
			pDiscPerSector->subch.prev.byTrackNum == pDiscPerSector->subch.current.byTrackNum &&
			pDiscPerSector->subch.prev.byIndex == pDiscPerSector->subch.current.byIndex) {
			if (pDiscPerSector->subch.current.byIndex != 0) {
				if (pDiscPerSector->subch.prev.nRelativeTime + 1 != pDiscPerSector->subch.current.nRelativeTime) {
					s_lineNum = __LINE__;
					bRet = FALSE;
				}
			}
			else if (pDiscPerSector->subch.current.byIndex == 0) {
				if (pDiscPerSector->subch.prev.nRelativeTime != pDiscPerSector->subch.current.nRelativeTime + 1) {
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
	PDISC_PER_SECTOR pDiscPerSector,
	INT nLBA
) {
	BOOL bRet = IsValidSubQMSF(pExecType, pDiscPerSector->subcode.current, 19, 20, 21);
	if (bRet) {
		INT tmpLBA = MSFtoLBA(BcdToDec(pDiscPerSector->subcode.current[19]),
			BcdToDec(pDiscPerSector->subcode.current[20]), BcdToDec(pDiscPerSector->subcode.current[21])) - 150;
		if (nLBA != tmpLBA || bRipPregap) {
			if (pDiscPerSector->subch.prev.nAbsoluteTime + 1 != pDiscPerSector->subch.current.nAbsoluteTime) {
				bRet = FALSE;
			}
		}
	}
	return bRet;
}

BOOL ContainsC2Error(
	PDEVICE pDevice,
	PDISC pDisc,
	UINT uiStart,
	UINT uiEnd,
	LPBYTE lpBuf,
	LPUINT lpuiC2errorNum,
	INT nLBA,
	BOOL bOutputLog
) {
	BOOL bRet = RETURNED_NO_C2_ERROR_1ST;
	*lpuiC2errorNum = 0;
	BOOL bErr = FALSE;
	INT aC2ErrPosBuf[CD_RAW_READ_C2_294_SIZE * CHAR_BIT] = {};
	INT nC2ErrPosCnt = 0;

	for (UINT uiC2ErrorPos = uiStart; uiC2ErrorPos < uiEnd; uiC2ErrorPos++) {
		UINT uiPos = pDevice->TRANSFER.uiBufC2Offset + uiC2ErrorPos;
		if (uiC2ErrorPos < CD_RAW_READ_C2_294_SIZE - 10 && pDevice->byPlxtrDrive &&
			lpBuf[uiPos] == 0 && lpBuf[uiPos + 1] == 0xf0 && lpBuf[uiPos + 2] == 0xf0 && lpBuf[uiPos + 3] == 0xf0 &&
			lpBuf[uiPos + 4] == 0 && lpBuf[uiPos + 5] == 0 && lpBuf[uiPos + 6] == 0 &&
			lpBuf[uiPos + 7] == 0x0f && lpBuf[uiPos + 8] == 0x0f && lpBuf[uiPos + 9] == 0x0f && lpBuf[uiPos + 10] == 0) {
			if (bOutputLog) {
				OutputLog(standardError | fileC2Error,
					"\n LBA[%06d, %#07x] Detected C2 error \"00 F0 F0 F0 00 00 00 0F 0F 0F 00\"\n"
					"This error can't be fixed by plextor drive. Needs to dump it by non-plextor drive and replace it\n"
					, nLBA, (UINT)nLBA);
			}
			uiC2ErrorPos += 10;
		}
		else if (lpBuf[uiPos] != 0) {
			// Ricoh based drives (+97 read offset, like the Aopen CD-RW CRW5232)
			// use lsb points to 1st byte of main. 
			// But almost drive is msb points to 1st byte of main.
//			INT nBit = 0x01;
			INT nBit = 0x80;
			if (bOutputLog && !bErr) {
				OutputC2ErrorLog("                        ofs: ");
			}
			for (INT n = 0; n < CHAR_BIT; n++) {
				// exist C2 error
				if (lpBuf[uiPos] & nBit) {
					// uiC2ErrorPos * CHAR_BIT => position of byte
					// (position of byte) + n => position of bit
					bRet = RETURNED_EXIST_C2_ERROR;
					(*lpuiC2errorNum)++;
					if (bOutputLog) {
						aC2ErrPosBuf[nC2ErrPosCnt++] = (INT)uiC2ErrorPos * 8 + n;
						OutputC2ErrorLog("%x, ", uiC2ErrorPos * 8 + n);
						bErr = TRUE;
					}
				}
//				nBit <<= 1;
				nBit >>= 1;
			}
		}
	}
	if (bOutputLog && bErr) {
		OutputC2ErrorLog("\n              ofs corrected: ");
		for (INT i = 0; i < nC2ErrPosCnt; i++) {
			if (0 <= aC2ErrPosBuf[i] + pDisc->MAIN.nCombinedOffset) {
				OutputC2ErrorLog("%x, ", (UINT)(aC2ErrPosBuf[i] + pDisc->MAIN.nCombinedOffset));
			}
			else {
				OutputC2ErrorLog("-%x, ", (UINT)(abs(pDisc->MAIN.nCombinedOffset + aC2ErrPosBuf[i])));
			}
		}
		OutputC2ErrorLog("\n");
	}
	return bRet;
}

BOOL AnalyzeIfoFile(
	PDEVICE pDevice,
	PDISC pDisc
) {
	BOOL bRet = TRUE;
	CONST size_t bufSize = 40;
	_TCHAR szBuf[bufSize] = {};
#ifdef _WIN32
	_sntprintf(szBuf, bufSize, _T("%c:\\VIDEO_TS\\VIDEO_TS.IFO"), pDevice->byDriveLetter);
#else
	_sntprintf(szBuf, bufSize, _T("%s/VIDEO_TS/VIDEO_TS.IFO"), pDevice->drivepath);
#endif

	if (PathFileExists(szBuf)) {
		pDisc->DVD.discType = DISC_TYPE_DVD::video;
		_TCHAR szFnameAndExt[_MAX_FNAME + _MAX_EXT] = {};
		FILE* fp = CreateOrOpenFile(szBuf, NULL, NULL, szFnameAndExt, NULL, _T(".IFO"), _T("rb"), 0, 0);
		if (!fp) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		LPBYTE pSector = NULL;
		LPUINT lpPgciStartByte = NULL;
		try {
			size_t stBufSize = 0x13 + 12 * USHRT_MAX;
			if (NULL == (pSector = (LPBYTE)calloc(stBufSize, sizeof(BYTE)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}
			if (fread(pSector, sizeof(BYTE), DISC_MAIN_DATA_SIZE, fp) != DISC_MAIN_DATA_SIZE) {
				throw FALSE;
			}
			WORD wNumOfTitleSets = MAKEWORD(pSector[0x3f], pSector[0x3e]);
			LONG lStartOfsOfPttSrpt = MAKELONG(MAKEWORD(pSector[0xc7], pSector[0xc6]), MAKEWORD(pSector[0xc5], pSector[0xc4]));
			fseek(fp, DISC_MAIN_DATA_SIZE * lStartOfsOfPttSrpt, SEEK_SET);
			if (fread(pSector, sizeof(BYTE), DISC_MAIN_DATA_SIZE, fp) != DISC_MAIN_DATA_SIZE) {
				throw FALSE;
			}
			WORD wNumOfTitlePlayMaps = MAKEWORD(pSector[1], pSector[0]);
			OutputDiscLog(OUTPUT_DHYPHEN_PLUS_STR("IFO Information")
				"%s, NumberOfTitlePlayMaps: %d\n", szFnameAndExt, wNumOfTitlePlayMaps);
			for (WORD v = 0; v < wNumOfTitlePlayMaps; v++) {
				WORD wNumOfChapters = MAKEWORD(pSector[0xb + 12 * v], pSector[0xa + 12 * v]);
				BYTE byNumOfTitleSet = pSector[0xe + 12 * v];
				BYTE byNumOfTitleSetTitleNumber = pSector[0xf + 12 * v];
				UINT uiStartSector = MAKEUINT(MAKEWORD(pSector[0x13 + 12 * v], pSector[0x12 + 12 * v])
					, MAKEWORD(pSector[0x11 + 12 * v], pSector[0x10 + 12 * v]));
				OutputDiscLog("\tTitle %2d, VTS_%02d, TitleNumber %2d, NumberOfChapters: %2d, StartSector: %u\n"
					, v + 1, byNumOfTitleSet, byNumOfTitleSetTitleNumber, wNumOfChapters, uiStartSector);
			}
			INT nPgcCnt = 0;
			if (NULL == (lpPgciStartByte = (LPUINT)calloc(USHRT_MAX, sizeof(UINT)))) {
				OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
				throw FALSE;
			}

			for (WORD w = 1; w <= wNumOfTitleSets; w++) {
#ifdef _WIN32
				_sntprintf(szBuf, bufSize, _T("%c:\\VIDEO_TS\\VTS_%.02d_0.IFO"), pDevice->byDriveLetter, w);
#else
				_sntprintf(szBuf, bufSize, _T("%s/VIDEO_TS/VTS_%.02d_0.IFO"), pDevice->drivepath, w);
#endif
				if (PathFileExists(szBuf)) {
					FILE* fpVts = CreateOrOpenFile(szBuf, NULL, NULL, szFnameAndExt, NULL, _T(".IFO"), _T("rb"), 0, 0);
					if (!fpVts) {
						OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
						break;
					}
					if (fread(pSector, sizeof(BYTE), DISC_MAIN_DATA_SIZE, fpVts) != DISC_MAIN_DATA_SIZE) {
						FcloseAndNull(fpVts);
						break;
					}
					LONG lStartOfsOfPgci = MAKELONG(MAKEDWORD(pSector[0xcf], pSector[0xce]), MAKEDWORD(pSector[0xcd], pSector[0xcc]));
					fseek(fpVts, DISC_MAIN_DATA_SIZE * lStartOfsOfPgci, SEEK_SET);

					if (fread(pSector, sizeof(BYTE), 8, fpVts) != 8) {
						FcloseAndNull(fpVts);
						break;
					}
					fseek(fpVts, -8, SEEK_CUR);
					WORD wNumOfPgciSrp = MAKEWORD(pSector[1], pSector[0]);
					UINT wByteOfPgciSrpTbl = MAKEUINT(MAKEWORD(pSector[7], pSector[6]), MAKEWORD(pSector[5], pSector[4]));
					LPBYTE pTmpBuf = NULL;
					if (stBufSize < wByteOfPgciSrpTbl + 8) {
						if (NULL == (pTmpBuf = (LPBYTE)realloc(pSector, wByteOfPgciSrpTbl + 8))) {
							OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
							throw FALSE;
						}
						else {
							pSector = pTmpBuf;
						}
					}
					if (fread(pSector, sizeof(BYTE), 8 + wByteOfPgciSrpTbl, fpVts) != 8 + wByteOfPgciSrpTbl) {
						FcloseAndNull(fpVts);
						break;
					}

					for (WORD x = 0; x < wNumOfPgciSrp; x++) {
						lpPgciStartByte[x] = MAKEUINT(MAKEWORD(pSector[0xf + 8 * x], pSector[0xe + 8 * x])
							, MAKEWORD(pSector[0xd + 8 * x], pSector[0xc + 8 * x]));
						INT nNumOfPrograms = pSector[lpPgciStartByte[x] + 2];
						INT nNumOfCells = pSector[lpPgciStartByte[x] + 3];
						UINT uiPlayBackTimeH = pSector[lpPgciStartByte[x] + 4];
						UINT uiPlayBackTimeM = pSector[lpPgciStartByte[x] + 5];
						UINT uiPlayBackTimeS = pSector[lpPgciStartByte[x] + 6];
						OutputDiscLog("%s, ProgramChain %2d, NumberOfPrograms %2d, NumberOfCells %2d, PlayBackTime -- %02x:%02x:%02x\n"
							, szFnameAndExt, x + 1, nNumOfPrograms, nNumOfCells, uiPlayBackTimeH, uiPlayBackTimeM, uiPlayBackTimeS);
						nPgcCnt++;
					}
				}
				else {
					bRet = FALSE;
					break;
				}
			}
			OutputDiscLog("NumberOfProgramChain: %d\n", nPgcCnt);
			if (wNumOfTitlePlayMaps != nPgcCnt) {
				OutputLog(standardOut | fileDisc, "Detected irregular title number\n");
			}
			OutputDiscLog("\n");
		}
		catch (BOOL bErr) {
			bRet = bErr;
		}
		FcloseAndNull(fp);
		FreeAndNull(lpPgciStartByte);
		FreeAndNull(pSector);
	}
	else {
		bRet = FALSE;
	}
	return bRet;
}

BOOL IsSjis(
	LPCH pTmpText,
	size_t stTxtIdx,
	size_t stTmpTextLen
) {
	BOOL bSjis = TRUE;
	for (size_t m = 0; m < stTmpTextLen; m += 2) {
		WORD c = MAKEWORD(*(pTmpText + stTxtIdx + m + 1), *(pTmpText + stTxtIdx + m));
		if (c < 0x8140 ||
			(0x8492 < c && c < 0x889f) ||
			0x9873 < c) {
			bSjis = FALSE;
			break;
		}
	}
	return bSjis;
}

// https://programming-place.net/ppp/contents/c/rev_res/string013.html
_TCHAR* find_last_string(const _TCHAR* s, const _TCHAR* target)
{
	const size_t s_len = _tcslen(s);
	const size_t target_len = _tcslen(target);

	if (s_len < target_len) {
		return NULL;
	}

	size_t index = s_len - target_len;

	for (;;) {
		if (_tcsncmp(&s[index], target, target_len) == 0) {
			return (_TCHAR*)&s[index];
		}

		if (index <= 0) {
			break;
		}
		--index;
	}
	return NULL;
}
