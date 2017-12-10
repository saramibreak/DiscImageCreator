/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

VOID OutputFsVolumeDescriptorSequence(
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputFsVolumeRecognitionSequence(
	LPBYTE lpBuf,
	INT nLBA
);

VOID OutputDVDStructureFormat(
	BYTE byFormatCode,
	LPBYTE lpBcaFlag,
	LPBOOL lpbCPRM,
	WORD wFormatLength,
	LPBYTE lpFormat,
	LPDWORD lpdwSectorLength
);

VOID OutputDVDCopyrightManagementInformation(
	PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR dvdCopyright,
	INT nLBA
);
