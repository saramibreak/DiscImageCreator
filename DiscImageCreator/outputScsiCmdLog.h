/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

VOID OutputInquiry(
	PDEVICE pDevice,
	PINQUIRYDATA pInquiry
	);

VOID OutputGetConfigurationHeader(
	PGET_CONFIGURATION_HEADER pConfigHeader
	);

VOID OutputGetConfigurationFeatureProfileType(
	WORD wFeatureProfileType
	);

VOID OutputGetConfigurationFeatureNumber(
	PDEVICE pDevice,
	LPBYTE lpConf,
	DWORD dwAllLen
	);

VOID OutputCDTextOther(
	PCDROM_TOC_CD_TEXT_DATA_BLOCK pDesc,
	WORD wTocTextEntries,
	BYTE bySizeInfoIdx,
	BYTE bySizeInfoCnt
	);

VOID OutputDiscInformation(
	PDISC_INFORMATION pDiscInformation
	);

VOID OutputModeParmeterHeader(
	PMODE_PARAMETER_HEADER pHeader
	);

VOID OutputModeParmeterHeader10(
	PMODE_PARAMETER_HEADER10 pHeader
	);

VOID OutputCDVDCapabilitiesPage(
	PDEVICE pDevice,
	PCDVD_CAPABILITIES_PAGE cdvd
	);

VOID OutputReadBufferCapacity(
	PREAD_BUFFER_CAPACITY_DATA pReadBufCapaData
	);

VOID OutputSetSpeed(
	PCDROM_SET_SPEED pSetspeed
	);

VOID OutputEeprom(
	LPBYTE pBuf,
	INT nRoop,
	BOOL byPlxtrDrive
	);
