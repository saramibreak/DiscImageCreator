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
#include "execIoctl.h"
#include "execScsiCmd.h"
#include "execScsiCmdforDVD.h"
#include "get.h"
#include "output.h"
#include "outputScsiCmdLog.h"
#include "outputScsiCmdLogforCD.h"
#include "outputScsiCmdLogforDVD.h"

#define GAMECUBE_SIZE	(712880)
#define WII_SL_SIZE		(2294912)
#define WII_DL_SIZE		(4155840)

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
	if (!ReadVolumeDescriptor(pExecType, pExtArg, pDevice, pDisc, 0, (LPBYTE)cdb, lpBuf, 16, 0, &bPVD, &volDesc)) {
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
			, volDesc.ISO_9660.dwPathTblPos, 0, pDirRec, &nDirPosNum)) {
			FreeAndNull(pDirRec);
			return FALSE;
		}
		if (!ReadDirectoryRecord(pExecType, pExtArg, pDevice, pDisc, (LPBYTE)cdb, lpBuf
			, volDesc.ISO_9660.dwLogicalBlkCoef, volDesc.ISO_9660.dwRootDataLen, 0, pDirRec, nDirPosNum)) {
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
			if (dwTransferLen > (UINT)(pDisc->SCSI.nAllLength - nLBA)) {
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

BOOL IsSupported0xE7Old(
	PDEVICE pDevice
) {
	if (!strncmp(pDevice->szProductId, "RW/DVD GCC-4160N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4240N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8160B", DRIVE_PRODUCT_ID_SIZE)
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL IsSupported0xE7New(
	PDEVICE pDevice
) {
	if (// comfirmed
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8082N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8161B", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8162B", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8163B", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8164B", DRIVE_PRODUCT_ID_SIZE) ||
		// not comfirmed
		!strncmp(pDevice->szProductId, "CDRW-DVD GCC4244", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "CDRW-DVD GCC4247", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "CDRW-DVD GCCT10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "CDRW-DVD GCCT20N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-D10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-D20N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-M10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-R10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-T10N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR-T20N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR3120L", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8083N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8084N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8085N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDR8087N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDRH10N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "DVD-ROM GDRH20N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4241N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4242N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4243N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4244N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4246N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-4247N", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-5241P", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-C10N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-C20N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-M10N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-T10N ", DRIVE_PRODUCT_ID_SIZE) ||
		!strncmp(pDevice->szProductId, "RW/DVD GCC-T20N ", DRIVE_PRODUCT_ID_SIZE)
		) {
		return TRUE;
	}
	return FALSE;
}

BOOL ReadDVDRaw(
	PEXT_ARG pExtArg,
	PDEVICE pDevice,
	PDISC pDisc,
	LPCTSTR pszPath
) {
	FILE* fp = NULL;
	_TCHAR szMode[4] = _T("wb");
	if (pExtArg->byResume) {
		memcpy(szMode, _T("ab+"), 3);
	}
	if (NULL == (fp = CreateOrOpenFile(pszPath, NULL, NULL, NULL, NULL, _T(".raw"), szMode, 0, 0))) {
		OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
		return FALSE;
	}
	LPBYTE pBuf = NULL;
	BOOL bRet = TRUE;
	try {
		DWORD dwTransferLen = 16;
		INT nMemBlkSize = 5;
		if (NULL == (pBuf = (LPBYTE)calloc(
			(size_t)DVD_RAW_READ * dwTransferLen * nMemBlkSize + pDevice->AlignmentMask, sizeof(BYTE)))) {
			OutputLastErrorNumAndString(_T(__FUNCTION__), __LINE__);
			throw FALSE;
		}
		LPBYTE lpBuf = (LPBYTE)ConvParagraphBoundary(pDevice, pBuf);
		BOOL bType = 0;
		INT nBaseAddr = 0x80000000;
		BYTE cdblen = CDB10GENERIC_LENGTH;
		BYTE lpCmd[CDB12GENERIC_LENGTH] = { 0 };
		INT nDriveSampleOffset = 0;
		if (!GetDriveOffset(pDevice->szProductId, &nDriveSampleOffset)) {
			throw  FALSE;
		}
		// Panasonic MN103S chip
		if (nDriveSampleOffset == 102) {
			if (IsSupported0xE7Old(pDevice) || IsSupported0xE7New(pDevice)) {
				if (IsSupported0xE7Old(pDevice)) {
					nMemBlkSize = 1;
					nBaseAddr = 0xa13000;
				}
				lpCmd[0] = 0xE7; // vendor specific command
				lpCmd[1] = 0x48; // H
				lpCmd[2] = 0x49; // I
				lpCmd[3] = 0x54; // T
				lpCmd[4] = 0x01; // read MCU memory sub-command
				lpCmd[10] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				lpCmd[11] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				cdblen = CDB12GENERIC_LENGTH;
			}
			else {
				nMemBlkSize = 1;
				lpCmd[0] = SCSIOP_READ_DATA_BUFF;
				lpCmd[1] = 0x01;
				lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
				lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
				bType = 1;
			}
		}
		// Mediatek MT chip
		else if (nDriveSampleOffset == 6) {
			dwTransferLen = 1;
			nMemBlkSize = 1;
			lpCmd[0] = SCSIOP_READ_DATA_BUFF;
			lpCmd[1] = 0x01;
			lpCmd[2] = 0x01;
			lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			bType = 2;
		}
		// Plextor etc.
		else {
			nMemBlkSize = 1;
			lpCmd[0] = SCSIOP_READ_DATA_BUFF;
			lpCmd[1] = 0x02;
			lpCmd[6] = LOBYTE(HIWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[7] = HIBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			lpCmd[8] = LOBYTE(LOWORD(DVD_RAW_READ * dwTransferLen));
			bType = 3;
		}

		CDB::_READ12 cdb = { 0 };
		cdb.OperationCode = SCSIOP_READ12;
		cdb.LogicalUnitNumber = pDevice->address.Lun;
		REVERSE_BYTES(&cdb.TransferLength, &dwTransferLen);
		if ((pDisc->SCSI.nAllLength == GAMECUBE_SIZE ||
			pDisc->SCSI.nAllLength == WII_SL_SIZE ||
			pDisc->SCSI.nAllLength == WII_DL_SIZE)
			&& IsSupported0xE7New(pDevice)) {
				cdb.Streaming = TRUE;
		}
		else {
			cdb.ForceUnitAccess = TRUE; // not read the drive cache memory
		}

		BYTE byScsiStatus = 0;
		DWORD dwReadAddrForHG[5] = { 
			nBaseAddr + 0 * dwTransferLen * DVD_RAW_READ,
			nBaseAddr + 1 * dwTransferLen * DVD_RAW_READ,
			nBaseAddr + 2 * dwTransferLen * DVD_RAW_READ,
			nBaseAddr + 3 * dwTransferLen * DVD_RAW_READ,
			nBaseAddr + 4 * dwTransferLen * DVD_RAW_READ
		};
		DWORD dwTransferAndMemSize = dwTransferLen * nMemBlkSize;
		DWORD dwReadSize = (DWORD)DISC_RAW_READ_SIZE * dwTransferLen;
		DWORD dwRawReadSize = (DWORD)DVD_RAW_READ * dwTransferLen;
		DWORD dwRawWriteSize = (DWORD)DVD_RAW_READ * dwTransferLen * nMemBlkSize;
		DWORD dwSectorNum = 0x30000;
		DWORD dwOfs[16] = {
			0, DVD_RAW_READ, DVD_RAW_READ * 2, DVD_RAW_READ * 3, DVD_RAW_READ * 4,
			DVD_RAW_READ * 5, DVD_RAW_READ * 6, DVD_RAW_READ * 7, DVD_RAW_READ * 8,
			DVD_RAW_READ * 9, DVD_RAW_READ * 10, DVD_RAW_READ * 11, DVD_RAW_READ * 12,
			DVD_RAW_READ * 13, DVD_RAW_READ * 14, DVD_RAW_READ * 15
		};
		INT nRereadNum = 0;
		INT nRereadRoop = 0;
		INT nLBA = 0;
		if (pExtArg->byResume) {
			INT64 size = (INT64)GetFileSize64(0, fp);
			_fseeki64(fp, size - DVD_RAW_READ, SEEK_SET);
			fread(lpBuf, sizeof(BYTE), DVD_RAW_READ, fp);
			_fseeki64(fp, size, SEEK_SET);
			dwSectorNum = MAKEDWORD(MAKEWORD(lpBuf[3], lpBuf[2]), MAKEWORD(lpBuf[1], 0)) + 1;
			nLBA = (INT)(size / DVD_RAW_READ);
		}
		for (; nLBA < pDisc->SCSI.nAllLength; nLBA += dwTransferAndMemSize) {
			if ((INT)dwTransferAndMemSize > pDisc->SCSI.nAllLength - nLBA) {
				nMemBlkSize = (pDisc->SCSI.nAllLength - nLBA) / (INT)dwTransferLen;
				dwRawWriteSize = (DWORD)DVD_RAW_READ * dwTransferLen * nMemBlkSize;
				dwTransferAndMemSize = dwTransferLen * nMemBlkSize;
			}
			REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
			// store the disc data to the drive cache memory
			if (nRereadNum) {
				INT tmp0 = 0;
				REVERSE_BYTES(&cdb.LogicalBlock, &tmp0);
				if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
					dwReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
				REVERSE_BYTES(&cdb.LogicalBlock, &nLBA);
			}
			if (!ScsiPassThroughDirect(pExtArg, pDevice, &cdb, CDB12GENERIC_LENGTH, lpBuf,
				dwReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
				|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
				throw FALSE;
			}
			BOOL bCheckSectorNum = TRUE;
			for (INT i = 0; i < nMemBlkSize; i++) {
				if (bType == 0) {
					lpCmd[6] = HIBYTE(HIWORD(dwReadAddrForHG[i]));
					lpCmd[7] = LOBYTE(HIWORD(dwReadAddrForHG[i]));
					lpCmd[8] = HIBYTE(LOWORD(dwReadAddrForHG[i]));
					lpCmd[9] = LOBYTE(LOWORD(dwReadAddrForHG[i]));
				}
				else if (bType == 1) {
					lpCmd[2] = HIBYTE(HIWORD(nLBA));
					lpCmd[3] = LOBYTE(HIWORD(nLBA));
					lpCmd[4] = HIBYTE(LOWORD(nLBA));
					lpCmd[5] = LOBYTE(LOWORD(nLBA));
				}
				DWORD dwOfs2 = dwRawReadSize * i;
				// read the drive cache memory
				if (!ScsiPassThroughDirect(pExtArg, pDevice, lpCmd, cdblen, lpBuf + dwOfs2,
					dwRawReadSize, &byScsiStatus, _T(__FUNCTION__), __LINE__)
					|| byScsiStatus >= SCSISTAT_CHECK_CONDITION) {
					throw FALSE;
				}
#if 1
				for (DWORD j = 0; j < dwTransferLen; j++) {
					DWORD dwOfs3 = dwOfs2 + dwOfs[j];
					DWORD dwGotSectorNum = MAKEDWORD(MAKEWORD(lpBuf[3 + dwOfs3]
						, lpBuf[2 + dwOfs3]), MAKEWORD(lpBuf[1 + dwOfs3], 0));
					if (dwSectorNum != dwGotSectorNum - j - i * dwTransferLen) {
						OutputString(" Expected sector num: %7ld, Got sector num: %7ld "
							, dwSectorNum, dwGotSectorNum - j - i * dwTransferLen);
						bCheckSectorNum = FALSE;
						break;
					}
				}
#endif
				if (!bCheckSectorNum) {
					break;
				}
			}
			if (bCheckSectorNum) {
				nRereadRoop = 0;
				nRereadNum = 0;
				fwrite(lpBuf, sizeof(BYTE), dwRawWriteSize, fp);
				dwSectorNum += dwTransferAndMemSize;
			}
			else {
				nRereadNum += 1;
				if (nRereadNum == 10) {
					OutputString("Max Reread %d. LBA: %7d\n", nRereadNum, nLBA);
					break;
				}
				OutputString("Reread %d. LBA: %7d\n", nRereadNum, nLBA);
				nLBA -= dwTransferAndMemSize;
			}
			OutputString(_T("\rCreating raw(LBA) %7u/%7u")
				, nLBA + (INT)dwTransferAndMemSize, pDisc->SCSI.nAllLength);
		}
		OutputString(_T("\n"));
	}
	catch (BOOL bErr) {
		bRet = bErr;
	}
	FreeAndNull(pBuf);
	FcloseAndNull(fp);

	if ((pDisc->SCSI.nAllLength == GAMECUBE_SIZE ||
		pDisc->SCSI.nAllLength == WII_SL_SIZE ||
		pDisc->SCSI.nAllLength == WII_DL_SIZE)
		&& IsSupported0xE7New(pDevice)) {
		TCHAR str[_MAX_PATH * 3] = { 0 };
		if (GetUnscCmd(str, pszPath)) {
			_tsystem(str);
		}
	}
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
			OutputDiscWithLBALogA("Read error\n", nLBA);
		}
		else {
			OutputDVDCopyrightManagementInformation(
				(PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR)(pBuf + sizeof(DVD_DESCRIPTOR_HEADER)), nLBA);
		}
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
			else if (pEntry->FormatCode == 0x03 && !discContents.ucBca && !pExtArg->byRawDump) {
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
							if (pEntry->FormatCode == 0) {
								ReadTOC(pExtArg, pExecType, pDevice, pDisc);
							}
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
	// FormatCode: 00 failed (for gamecube. wii disc does success)
	if (pDisc->SCSI.nAllLength == 0) {
		if (discContents.disc == DISC_TYPE::gamecube) {
			pDisc->SCSI.nAllLength = GAMECUBE_SIZE;
		}
		else {
			pDisc->SCSI.nAllLength = WII_SL_SIZE;
		}
	}
	return TRUE;
}
