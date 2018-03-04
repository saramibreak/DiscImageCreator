/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#include "struct.h"
#include "check.h"
#include "convert.h"
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforDVD.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforDVD.h"

BOOL ReadDVDForFileSystem(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	CDB::_READ12* cdb,
	LPBYTE lpBuf
) {
	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
		pDevice->dwMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	INT nLBA = 16;
	BOOL bPVD = FALSE;
	VOLUME_DESCRIPTOR volDesc;
	if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice, pDisc, 0, (LPBYTE)cdb, lpBuf, 16, &bPVD, &volDesc)) {
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
			, volDesc.ISO_9660.dwLogicalBlkCoef, volDesc.ISO_9660.dwPathTblSize
			, volDesc.ISO_9660.dwPathTblPos, pDirRec, &nDirPosNum)) {
			FreeAndNull(pDirRec);
			return FALSE;
		}
		if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)cdb, lpBuf
			, volDesc.ISO_9660.dwLogicalBlkCoef, volDesc.ISO_9660.dwRootDataLen, pDirRec, nDirPosNum)) {
			FreeAndNull(pDirRec);
			return FALSE;
		}
		FreeAndNull(pDirRec);
	}

	INT nStart = DISC_RAW_READ_SIZE * nLBA;
	INT nEnd = DISC_RAW_READ_SIZE * 32;
	for (INT i = nStart; i <= nEnd; i += DISC_RAW_READ_SIZE, nLBA++) {
		OutputFsVolumeRecognitionSequence(lpBuf + i, nLBA);
	}

	nLBA = 32;
	DWORD dwTransferLen = pDevice->dwMaxTransferLength / DISC_RAW_READ_SIZE;
	cdb->LogicalUnitNumber = pDevice->address.Lun;
	REVERSE_BYTES(&cdb->TransferLength, &dwTransferLen);
	cdb->LogicalBlock[0] = 0;
	cdb->LogicalBlock[1] = 0;
	cdb->LogicalBlock[2] = 0;
	cdb->LogicalBlock[3] = (UCHAR)nLBA;

	if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
		pDevice->dwMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	if (lpBuf[20] == 0 && lpBuf[21] == 0 && lpBuf[22] == 0 && lpBuf[23] == 0) {
		for (INT i = 0; i <= nEnd; i += DISC_RAW_READ_SIZE, nLBA++) {
			OutputFsVolumeDescriptorSequence(lpBuf + i, nLBA);
		}
	}

	cdb->LogicalBlock[2] = 1;
	cdb->LogicalBlock[3] = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, cdb, CDB12GENERIC_LENGTH, lpBuf,
		pDevice->dwMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}
	nLBA = 256;
	OutputFsVolumeDescriptorSequence(lpBuf, nLBA);
	return TRUE;
}

BOOL ReadDVD(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath
) {
	FILE* fp = CreateOrOpenFile(
		pszPath, NULL, NULL, NULL, NULL, _T(".iso"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	BOOL bRet = TRUE;
	LPBYTE pBuf = NULL;
	try {
		if (NULL == (pBuf = (LPBYTE)calloc(
			pDevice->dwMaxTransferLength + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);

		DWORD dwTransferLen = 1;
		CDB::_READ12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_READ12;
		cdb.LogicalUnitNumber = pDevice->address.Lun;
		REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
		if (pExtArg->byFua) {
			cdb.ForceUnitAccess = TRUE;
		}
		if (!ReadDVDForFileSystem(pExecType, pExtArg, pDevice, pDisc, &cdb, lpBuf)) {
			throw FALSE;
		}
		FlushLog();

		BYTE byScsiStatus = 0;
		dwTransferLen = pDevice->dwMaxTransferLength / DISC_RAW_READ_SIZE;
		REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);

		for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA += dwTransferLen) {
			if (pDisc->SCSI.nAllLength - nLBA < (INT)dwTransferLen) {
				dwTransferLen = (UINT)(pDisc->SCSI.nAllLength - nLBA);
				REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
			}
			REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				pDevice->dwMaxTransferLength, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			fwrite(lpBuf, sizeof(BYTE), (size_t)DISC_RAW_READ_SIZE * dwTransferLen, fp);
			OutputString(_T("\rCreating iso(LBA) %8lu/%8u"),
				nLBA + dwTransferLen - 1, pDisc->SCSI.nAllLength - 1);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL ret) {
		bRet = ret;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);
	return bRet;
}

BOOL ReadDVDRaw(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	LPCTSTR pszPath
) {
	FILE* fp = CreateOrOpenFile(
		pszPath, NULL, NULL, NULL, NULL, _T(".raw"), _T("wb"), 0, 0);
	if (!fp) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	DWORD dwTransferLen = 31;
	LPBYTE pBuf = NULL;
	BOOL bRet = TRUE;
	try {
		if (NULL == (pBuf = (LPBYTE)calloc(DVD_RAW_READ *
			dwTransferLen + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
		BYTE cdblen = CDB12GENERIC_LENGTH;
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		if (pDevice->szVendorId && !strncmp(pDevice->szVendorId, "PLEXTOR", 7)) {
			lpCmd[0] = SCSIOP_READ_DATA_BUFF;
			lpCmd[1] = 0x02;
			lpCmd[2] = 0x00;
			lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			cdblen = CDB10GENERIC_LENGTH;
		}
		else if (pDevice->szVendorId && !strncmp(pDevice->szVendorId, "HL-DT-ST", 8)) {
			lpCmd[0] = 0xE7; // vendor specific command
			lpCmd[1] = 0x48; // H
			lpCmd[2] = 0x49; // I
			lpCmd[3] = 0x54; // T
			lpCmd[4] = 0x01; // read MCU memory sub-command
			lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
		}
		// 2048 * 2294912 = 4699979776
		// 2064 * 2294912 = 4736698368
		UINT64 nMaxSize = 4736698368;
		for (UINT64 nLBA = 0; nLBA < nMaxSize; nLBA += DVD_RAW_READ * dwTransferLen) {
			if (nMaxSize - nLBA < (INT)dwTransferLen) {
				dwTransferLen = (UINT)(nMaxSize - nLBA);
				if (pDevice->szVendorId && !strncmp(pDevice->szVendorId, "PLEXTOR", 7)) {
					lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
					lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
					lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				}
				else if (pDevice->szVendorId && !strncmp(pDevice->szVendorId, "HL-DT-ST", 8)) {
					lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
					lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				}
			}
			if (pDevice->szVendorId && !strncmp(pDevice->szVendorId, "PLEXTOR", 7)) {
				lpCmd[3] = LOBYTE(HIWORD(nLBA));
				lpCmd[4] = HIBYTE(LOWORD(nLBA));
				lpCmd[5] = LOBYTE(LOWORD(nLBA));
			}
			else if (pDevice->szVendorId && !strncmp(pDevice->szVendorId, "HL-DT-ST", 8)) {
				lpCmd[6] = HIBYTE(HIWORD(nLBA));
				lpCmd[7] = LOBYTE(HIWORD(nLBA));
				lpCmd[8] = HIBYTE(LOWORD(nLBA));
				lpCmd[9] = LOBYTE(LOWORD(nLBA));
			}
			BYTE byScsiStatus = 0;
			if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, cdblen, lpBuf, 
				(DWORD)DVD_RAW_READ * dwTransferLen, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}

			fwrite(lpBuf, (size_t)DVD_RAW_READ * dwTransferLen, sizeof(BYTE), fp);
			OutputString(_T("\rCreating raw(LBA) %10llu/%10llu"), 
				nLBA + DVD_RAW_READ * dwTransferLen - 1, nMaxSize - 1);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);
#if 0
	// TODO: descrambled wii-rom
	unscrambler *u = unscrambler_new ();
	CHAR pszPathA[_MAX_PATH];
	ZeroMemory(pszPathA, sizeof(pszPathA));
	WideCharToMultiByte(CP_THREAD_ACP, 0, pszPath, -1, pszPathA, _MAX_PATH, NULL, NULL);

	CHAR pszInPath[_MAX_PATH];
	ZeroMemory(pszInPath, sizeof(pszInPath));
	FILE* fpIn = CreateOrOpenFileA(pszPathA, pszInPath, NULL, ".raw", "rb", 0, 0);
	if (!fpIn) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	CHAR pszOutPath[_MAX_PATH];
	ZeroMemory(pszOutPath, sizeof(pszOutPath));
	FILE* fpOut = CreateOrOpenFileA(pszPathA, pszOutPath, NULL, ".iso", "wb", 0, 0);
	if (!fpOut) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	UINT current_sector = 0;
	unscrambler_set_disctype(3);
	unscrambler_unscramble_file(u, pszInPath, pszOutPath, &current_sector);
	u = (unscrambler *)unscrambler_destroy (u);
#endif
	return bRet;
}

BOOL ReadDVDForCMI(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	CONST WORD wSize =
		sizeof(DVD_DESCRIPTOR_HEADER) + sizeof(DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR);
	_declspec(align(4)) BYTE pBuf[wSize] = { 0 };

	CDB::_READ_DVD_STRUCTURE cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	cdb.Format = DvdMaxDescriptor;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wSize);
	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(CopyrightManagementInformation));

	BOOL bRet = TRUE;
	BYTE byScsiStatus = 0;
	for (INT nLBA = 0; nLBA < pDisc->SCSI.nAllLength; nLBA++) {
		REVERSE_BYTES(&cdb.RMDBlockNumber, &nLBA);
		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, pBuf, 
			wSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
			|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			bRet = FALSE;
			break;
		}
		OutputDVDCopyrightManagementInformation(
			(PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR)(pBuf + sizeof(DVD_DESCRIPTOR_HEADER)), nLBA);
		OutputString(
			_T("\rWriting CMI log(LBA) %8u/%8u"), nLBA, pDisc->SCSI.nAllLength - 1);
	}
	OutputString(_T("\n"));
	return bRet;
}

BOOL ExecReadingKey(
	PDEVICE pDevice
) {
	BOOL bRet = TRUE;
	PDVD_COPY_PROTECT_KEY dvdKey = (PDVD_COPY_PROTECT_KEY)calloc(DVD_DISK_KEY_LENGTH, sizeof(BYTE));
	try {
		if (!DvdStartSession(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_CHALLENGE_KEY_LENGTH;
		dvdKey->KeyType = DvdChallengeKey;
#if 0
		for (int j = 13; 3 < j; j--) {
			dvdKey->KeyData[j] = (UCHAR)(j - 4);
		}
#endif
		if (!SendKey(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_BUS_KEY_LENGTH;
		dvdKey->KeyType = DvdBusKey1;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_CHALLENGE_KEY_LENGTH;
		dvdKey->KeyType = DvdChallengeKey;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
#if 0
		// need to exec DeCSS
		// ref: https://github.com/mpc-hc/mpc-hc/tree/develop/src/DeCSS
		dvdKey->KeyLength = DVD_BUS_KEY_LENGTH;
		dvdKey->KeyType = DvdBusKey2;
		if (!SendKey(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_TITLE_KEY_LENGTH;
		dvdKey->KeyType = DvdTitleKey;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_DISK_KEY_LENGTH;
		dvdKey->KeyType = DvdDiskKey;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
		dvdKey->KeyLength = DVD_ASF_LENGTH;
		dvdKey->KeyType = DvdAsf;
		if (!ReadKey(pDevice, dvdKey)) {
			throw FALSE;
		}
#endif
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(dvdKey);
	return bRet;
}

BOOL ReadDiscStructure(
	PEXEC_TYPE pExecType,
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc
) {
	CONST WORD wMaxDVDStructureSize =
		sizeof(DVD_DESCRIPTOR_HEADER) + sizeof(DVD_STRUCTURE_LIST_ENTRY) * 0xff;
	_declspec(align(4)) BYTE pBuf[wMaxDVDStructureSize] = { 0 };

	CDB::_READ_DVD_STRUCTURE cdb = { 0 };
	cdb.OperationCode = SCSIOP_READ_DVD_STRUCTURE;
	if (*pExecType == bd) {
		cdb.Reserved1 = 1;
	}
	cdb.Format = 0xff;
	REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wMaxDVDStructureSize);

	BYTE byScsiStatus = 0;
	if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
		pBuf, wMaxDVDStructureSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
		|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
		return FALSE;
	}

	PDVD_DESCRIPTOR_HEADER pDescHeader = ((PDVD_DESCRIPTOR_HEADER)pBuf);
	REVERSE_SHORT(&pDescHeader->Length);
	WORD wDataSize = pDescHeader->Length - sizeof(pDescHeader->Length);
	WORD wEntrySize = wDataSize / sizeof(DVD_STRUCTURE_LIST_ENTRY);
	DISC_CONTENTS discContents = { 0 };

	OutputDiscLogA(OUTPUT_DHYPHEN_PLUS_STR(DiscStructure));
	for (WORD i = 0; i < wEntrySize; i++) {
		PDVD_STRUCTURE_LIST_ENTRY pEntry = 
			((PDVD_STRUCTURE_LIST_ENTRY)pDescHeader->Data + i);
		WORD wFormatLen = MAKEWORD(pEntry->FormatLength[1], pEntry->FormatLength[0]);
		OutputDiscLogA(
			"FormatCode: %02x, Sendable: %3s, Readable: %3s, FormatLength: %u\n", 
			pEntry->FormatCode,	BOOLEAN_TO_STRING_YES_NO_A(pEntry->Sendable),
			BOOLEAN_TO_STRING_YES_NO_A(pEntry->Readable), wFormatLen);
		if (wFormatLen == 0) {
			OutputDiscLogA("Skiped because length is 0\n\n");
			continue;
		}
		if (*pExecType == dvd) {
			if (pEntry->FormatCode == 0xff) {
				OutputDiscLogA("Skiped\n\n");
				break;
			}
			else if (pEntry->FormatCode == 0x02) {
				if (discContents.protect == css) {
					ExecReadingKey(pDevice);
				}
				else {
					OutputDiscLogA("Skiped because of DVD with CSS only\n\n");
					continue;
				}
			}
			else if (pEntry->FormatCode == 0x03 && !discContents.ucBca) {
				OutputDiscLogA("Skiped because of no BCA data\n\n");
				continue;
			}
			else if (pEntry->FormatCode == 0x05) {
				OutputDiscLogA("Skiped. If you see the detailed, use /c option\n\n");
				continue;
			}
			else if (pEntry->FormatCode == 0x06 || pEntry->FormatCode == 0x07) {
				if (discContents.protect == cprm) {
					ExecReadingKey(pDevice);
				}
				else {
					OutputDiscLogA("Skiped because of DVD with CPRM only\n\n");
					continue;
				}
			}
			else if ((0x08 <= pEntry->FormatCode && pEntry->FormatCode <= 0x0b) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdRam) {
				OutputDiscLogA("Skiped because of DVD-RAM only\n\n");
				continue;
			}
			else if ((0x0c <= pEntry->FormatCode && pEntry->FormatCode <= 0x10) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdRecordable) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdRWSequential)) {
				OutputDiscLogA("Skiped because of DVD-R, RW only\n\n");
				continue;
			}
			else if ((pEntry->FormatCode == 0x11 || pEntry->FormatCode == 0x30) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdPlusRW) &&
				(pDisc->SCSI.wCurrentMedia != ProfileDvdPlusR)) {
				OutputDiscLogA("Skiped because of DVD+R, RW only\n\n");
				continue;
			}
			else if ((0x20 <= pEntry->FormatCode && pEntry->FormatCode <= 0x24) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdDashRDualLayer) {
				OutputDiscLogA("Skiped because of DVD-R DL only\n\n");
				continue;
			}
			else if ((pEntry->FormatCode == 0xc0) &&
				pDisc->SCSI.wCurrentMedia != ProfileDvdRewritable) {
				OutputDiscLogA("Skiped because of DVD-R Rewritable only\n\n");
				continue;
			}
		}
		else if (*pExecType == bd) {
			if ((pEntry->FormatCode == 0x08 || pEntry->FormatCode == 0x0a ||
				pEntry->FormatCode == 0x12 || pEntry->FormatCode == 0x30) &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRSequentialWritable &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRRandomWritable &&
				pDisc->SCSI.wCurrentMedia != ProfileBDRewritable) {
				OutputDiscLogA("Skiped because of BD-R, RW only\n\n");
				continue;
			}
		}
		LPBYTE lpFormat = (LPBYTE)calloc(wFormatLen, sizeof(BYTE));
		if (!lpFormat) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			return FALSE;
		}
		cdb.LayerNumber = 0;
		cdb.Format = pEntry->FormatCode;
		REVERSE_BYTES_SHORT(&cdb.AllocationLength, &wFormatLen);

		if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, 
			lpFormat, wFormatLen, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
			byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
			OutputLogA(standardError | fileDisc, "FormatCode: %02x failed\n", pEntry->FormatCode);
		}
		else {
			if (*pExecType == dvd) {
				DWORD dwSectorLen = 0;
				OutputDVDStructureFormat(pEntry->FormatCode, wFormatLen - sizeof(DVD_DESCRIPTOR_HEADER)
					, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), &dwSectorLen, &discContents);
				if (pEntry->FormatCode == DvdPhysicalDescriptor) {
					PDVD_FULL_LAYER_DESCRIPTOR dvdpd = (PDVD_FULL_LAYER_DESCRIPTOR)(lpFormat + sizeof(DVD_DESCRIPTOR_HEADER));
					if (dvdpd->commonHeader.TrackPath == 0 && dvdpd->commonHeader.NumberOfLayers == 1) {
						cdb.LayerNumber = 1;
						if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH,
							lpFormat, wFormatLen, &byScsiStatus, _T(__FUNCTION__), __LINE__) ||
							byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
							OutputLogA(standardError | fileDisc, "FormatCode: %02x failed\n", pEntry->FormatCode);
						}
						else {
							DWORD dwSectorLen2 = 0;
							OutputDVDStructureFormat(pEntry->FormatCode, wFormatLen - sizeof(DVD_DESCRIPTOR_HEADER)
								, lpFormat + sizeof(DVD_DESCRIPTOR_HEADER), &dwSectorLen2, &discContents);
							OutputDiscLogA("\tLayerAllSector : % 7lu (%#lx)\n", dwSectorLen + dwSectorLen2, dwSectorLen + dwSectorLen2);
							dwSectorLen += dwSectorLen2;
						}
					}
					pDisc->SCSI.nAllLength = (INT)dwSectorLen;
				}
			}
			else if (*pExecType == bd) {
				OutputBDStructureFormat(pEntry->FormatCode,
					wFormatLen - sizeof(DVD_DESCRIPTOR_HEADER),	lpFormat + sizeof(DVD_DESCRIPTOR_HEADER));
			}
		}
		OutputDiscLogA("\n");
		FreeAndNull(lpFormat);
	}
	return TRUE;
}
