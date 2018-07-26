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
#pragma once
#include "forwardDeclaration.h"

// memo
//  CONST   => const
//  LPBOOL  => BOOL*
//  CONST LPBOOL => BOOL* const
//  CONST BOOL*  => const BOOL*
#ifdef _WIN32
#pragma pack(push, sensedata, 1)
typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	SCSI_PASS_THROUGH_DIRECT ScsiPassThroughDirect;
	SENSE_DATA SenseData;
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;
#pragma pack(pop, sensedata)
#endif

typedef struct _CDVD_CAPABILITIES_PAGE_WITH_HEADER10 {
	MODE_PARAMETER_HEADER10 header;
	CDVD_CAPABILITIES_PAGE cdvd;
} CDVD_CAPABILITIES_PAGE_WITH_HEADER10, *PCDVD_CAPABILITIES_PAGE_WITH_HEADER10;

typedef struct _CDVD_CAPABILITIES_PAGE_WITH_HEADER {
	MODE_PARAMETER_HEADER header;
	CDVD_CAPABILITIES_PAGE cdvd;
} CDVD_CAPABILITIES_PAGE_WITH_HEADER, *PCDVD_CAPABILITIES_PAGE_WITH_HEADER;
#if 0
#pragma pack(push, cdbsp, 1)
typedef union _CDBSP {
	// ftp://ftp.t10.org/t10/document.95/95-104r0.pdf
	// p23-p25
	//  This document specifies SCSI command set for CD-R drive.
	struct _SONY_READ_CDDA {
		UCHAR OperationCode;    // 0xD8
		UCHAR Reserved0 : 3;
		UCHAR Fua : 1;
		UCHAR Reserved1 : 1;
		UCHAR LogicalUnitNumber : 3;
		UCHAR LogicalBlockByte0;
		UCHAR LogicalBlockByte1;
		UCHAR LogicalBlockByte2;
		UCHAR LogicalBlockByte3;
		UCHAR Reserved2;
		UCHAR TransferBlockByte0;
		UCHAR TransferBlockByte1;
		UCHAR TransferBlockByte2;
		UCHAR SubCode;
		UCHAR Reserved3;
	} SONY_READ_CDDA;

	// http://www.pioneerelectronics.com/pio/pe/images/portal/cit_3424/31636562SCSI-2RefManV31.pdf
	// p156-p158
	//	This command applies only to models DRM-602X, DRM-624X, and the DR-U Series drives. 
	struct _PINR_READ_CDDA {
		UCHAR OperationCode;    // 0xD8
		UCHAR Reserved0 : 5;
		UCHAR LogicalUnitNumber : 3;
		UCHAR LogicalBlockByte0;
		UCHAR LogicalBlockByte1;
		UCHAR LogicalBlockByte2;
		UCHAR LogicalBlockByte3;
		UCHAR Reserved1;
		UCHAR TransferBlockByte0;
		UCHAR TransferBlockByte1;
		UCHAR TransferBlockByte2;
		UCHAR SubCode;
		UCHAR Reserved2;
	} PINR_READ_CDDA;
} CDBSP, *PCDBSP;
#pragma pack(pop, cdbsp)
#endif
typedef union _CDFLAG {
	struct _READ_CD {
		// used ExpectedSectorType : 3;
		enum _EXPECTED_SECTOR_TYPE {
			All = 0,
			CDDA = 1,
			Mode1 = 2,
			Mode2 = 3,
			Mode2form1 = 4,
			Mode2form2 = 5
			// 6, 7 is reserved
		} EXPECTED_SECTOR_TYPE, *PEXPECTED_SECTOR_TYPE;

		// used ErrorFlags : 2;
		enum _ERROR_FLAGS {
			NoC2 = 0,
			byte294 = 1,
			byte296 = 2
			// 3 is reserved
		} ERROR_FLAGS, *PERROR_FLAGS;

		// used HeaderCode : 2;
		enum _HEADER_CODE {
			NoHeader = 0,
			SectorHeader = 1,
			SubHeader = 2,
			BothHeader = 3
		} HEADER_CODE, *PHEADER_CODE;

		// used SubChannelSelection : 3;
		enum _SUB_CHANNEL_SELECTION {
			NoSub = 0,
			Raw = 1,	// Raw P-W sub-channel data
			Q = 2,		// Formatted Q sub-channel data
			Pack = 4	// Corrected and de-interleaved R-W sub-channel data
						// 3, 5 to 7 is reserved
		} SUB_CHANNEL_SELECTION, *PSUB_CHANNEL_SELECTION;
	} READ_CD, *PREAD_CD;

	struct _PLXTR_READ_CDDA {
		enum _SUB_CHANNEL_SELECTION {
			NoSub = 0,
			MainQ = 1,		// Main data + Formatted Q sub-channel data
			MainPack = 2,	// Main data + Raw P-Q + Corrected and de-interleaved R-W sub-channel data
			Raw = 3,		// Raw P-W sub-channel data
			MainC2Raw = 8	// Main data + C2 error data + Raw P-W sub-channel data
							// 4 to 7, 9 to 255 is reserved
		} SUB_CHANNEL_SELECTION, *PSUB_CHANNEL_SELECTION;
	} PLXTR_READ_CDDA, *PPLXTR_READ_CDDA;
#if 0
	// ftp://ftp.t10.org/t10/document.95/95-104r0.pdf
	// p23-p25
	struct _SONY_READ_CDDA {
		enum _SUB_CHANNEL_SELECTION {
			NoSub = 0,
			MainQ = 1,		// CD-DA data with Sub-Q Code
			MainPack = 2,	// CD-DA data with all Sub Code (Pack? or Raw?)
			Raw = 3,		// All Sub Code only (Pack? or Raw?)
							// 4 to 255 is reserved
		} SUB_CHANNEL_SELECTION, *PSUB_CHANNEL_SELECTION;
	} SONY_READ_CDDA, *PSONY_READ_CDDA;

	// http://www.pioneerelectronics.com/pio/pe/images/portal/cit_3424/31636562SCSI-2RefManV31.pdf
	// p156-p158
	struct _PINR_READ_CDDA {
		enum _SUB_CHANNEL_SELECTION {
			NoSub = 0,
			MainQ = 1,		// CD-DA data with Sub-Q Code
			MainPack = 2,	// CD-DA data with all Sub Code (Pack? or Raw?)
			Raw = 3,		// All Sub Code only (Pack? or Raw?)
							// 4 to 255 is reserved
		} SUB_CHANNEL_SELECTION, *PSUB_CHANNEL_SELECTION;
	} PINR_READ_CDDA, *PPINR_READ_CDDA;
#endif
} CDFLAG, *PCDFLAG;

typedef struct _LOG_FILE {
	FILE* fpDisc;
	FILE* fpVolDesc;
	FILE* fpDrive;
	FILE* fpMainInfo;
	FILE* fpMainError;
	FILE* fpSubInfo;
	FILE* fpSubIntention;
	FILE* fpSubError;
	FILE* fpC2Error;
} LOG_FILE, *PLOG_FILE;

typedef struct _EXT_ARG {
	BYTE byQuiet;
	BYTE byAdd;
	BYTE byBe;
	BYTE byRaw;
	BYTE byPack;
	BYTE byD8;
	BYTE byC2;
	BYTE byCmi;
	BYTE byFua;
	BYTE byMCN;
	BYTE byPre;
	BYTE byMultiSession;
	BYTE byRawDump;
	BYTE byFix;
	BYTE byResume;
	BYTE byReverse;
	BYTE byScanProtectViaFile;
	BYTE byScanProtectViaSector;
	BYTE byScanAntiModStr;
	BYTE bySkipSubP;
	BYTE bySkipSubQ;
	BYTE bySkipSubRtoW;
	BYTE byLibCrypt;
	BYTE byIntentionalSub;
	BYTE by74Min;
	BYTE padding[3];
	LONG nAudioCDOffsetNum;
	DWORD dwMaxRereadNum;
	LONG nC2RereadingType;
	LONG nStartLBAForC2;
	LONG nEndLBAForC2;
	DWORD dwCacheDelNum;
	DWORD dwTimeoutNum;
	DWORD dwSubAddionalNum;
} EXT_ARG, *PEXT_ARG;

typedef struct _DEVICE {
#ifdef _WIN32
	HANDLE hDevice;
#else
	int hDevice;
#endif
	SCSI_ADDRESS address;
	UINT_PTR AlignmentMask;
	DWORD dwMaxTransferLength;
	CHAR szVendorId[8];
	CHAR szProductId[16];
	CHAR szProductRevisionLevel[4];
	BYTE byPlxtrDrive;
	BYTE bySuccessReadToc;
	BYTE bySuccessReadTocFull;
#ifdef _WIN32
	BYTE byDriveLetter;
#else
	CHAR drivepath[15];
#endif
	WORD wMaxReadSpeed;
	BYTE padding[2];
	_CDFLAG::_READ_CD::_ERROR_FLAGS supportedC2Type;
	DWORD dwTimeOutValue;
	DRIVE_DATA_ORDER driveOrder;
	struct _TRANSFER {
		DWORD dwBufLen;
		DWORD dwBufC2Offset;
		DWORD dwBufSubOffset;
	} TRANSFER, *PTRANSFER;
	struct _FEATURE {
		BYTE byCanCDText;
		BYTE byC2ErrorData;
		BYTE byModePage2a;
		BYTE bySetCDSpeed;
		BYTE byReadBufCapa;
		BYTE reserved[3];
	} FEATURE, *PFEATURE;
} DEVICE, *PDEVICE;

// Don't define value of BYTE(1byte) or SHOUT(2byte) before CDROM_TOC structure
// Because Paragraph Boundary (under 4bit of start address of buffer must 0)
// reference
// http://msdn.microsoft.com/ja-jp/library/aa290049(v=vs.71).aspx
typedef struct _DISC {
	struct _SCSI {
#ifdef _WIN32
		_declspec(align(4)) CDROM_TOC toc; // get at CDROM_READ_TOC_EX_FORMAT_TOC
#else
		__attribute__ ((aligned(4))) CDROM_TOC toc; // get at CDROM_READ_TOC_EX_FORMAT_TOC
#endif
		INT nAllLength;				// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPINT lpFirstLBAListOnToc;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPINT lpLastLBAListOnToc;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT nFirstLBAofDataTrack;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT nLastLBAofDataTrack;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		TRACK_TYPE trackType;		// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byFirstDataTrackNum;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byLastDataTrackNum;	// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byFormat;				// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		BYTE padding1;
		BOOL bMultiSession;			// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		LPBYTE lpSessionNumList;	// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		INT nFirstLBAofLeadout;		// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		INT nFirstLBAof2ndSession;	// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		LPSTR* pszTitle;			// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
		LPSTR* pszPerformer;		// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
		LPSTR* pszSongWriter;		// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
		WORD wCurrentMedia;			// get at SCSIOP_GET_CONFIGURATION
		BYTE padding2[2];
	} SCSI;
	struct _MAIN {
		INT nAdjustSectorNum;
		INT nCombinedOffset;
		UINT uiMainDataSlideSize;
		INT nOffsetStart;
		INT nOffsetEnd;
		INT nFixStartLBA;
		INT nFixEndLBA;
		INT nFixFirstLBAofLeadout;		// for sliding offset
		INT nFixFirstLBAof2ndSession;	// for sliding offset
		// 0 origin, max is last track num.
		LPBYTE lpModeList;
		BOOL bPathType; // use path table record
		LPDWORD lpAllSectorCrc32;
		LPINT lpAllLBAOfC2Error;
		INT nC2ErrorCnt;
	} MAIN;
	struct _SUB {
		INT nSubChannelOffset;
		INT nFirstLBAForMCN[3][2];
		INT nRangeLBAForMCN[3][2];
		INT nPrevMCNSector;
		BYTE byDesync;
		BYTE byCatalog;
		CHAR szCatalog[META_CATALOG_SIZE];
		INT nFirstLBAForISRC[3][2];
		INT nRangeLBAForISRC[3][2];
		INT nPrevISRCSector;
		// 0 origin, max is last track num.
		LPSTR* pszISRC;
		// 0 origin, max is last track num.
		// toc indexes in priority. single ptr: LBA per track. double ptr: LBA per index
		LPINT* lpFirstLBAListOnSub;
		// 0 origin, max is last track num.
		// sub indexes in priority. single ptr: LBA per track. double ptr: LBA per index
		LPINT* lpFirstLBAListOnSubSync;
		// 0 origin, max is last track num.
		LPINT lpFirstLBAListOfDataTrackOnSub;
		// 0 origin, max is last track num.
		LPINT lpLastLBAListOfDataTrackOnSub;
		// 0 origin, max is last track num.
		LPBYTE lpCtlList;
		// 0 origin, max is last track num.
		LPBYTE lpEndCtlList;
		// 0 origin, max is last track num.
		LPBOOL lpISRCList;
		// 0 origin, max is last track num.
		LPBYTE lpRtoWList;
		INT nCorruptCrcH;
		INT nCorruptCrcL;
	} SUB;
	struct _PROTECT {
		BYTE byExist;
		BYTE byTmpForSafeDisc;
		BYTE byRestoreCounter; // for SecuROM
		BYTE reserved;
		CHAR name[MAX_FNAME_FOR_VOLUME];
		// for skipping unreadable file
		struct _ERROR_SECTOR {
			INT nExtentPos;
			INT nNextExtentPos; // for safedisc
			INT nSectorSize;
			INT nExtentPos2nd; // for Der KorsaR
			INT nSectorSize2nd; // for Der KorsaR
		} ERROR_SECTOR;
		INT nPrevLBAOfPathTablePos; // for CodeLock
		INT nNextLBAOfLastVolDesc; // for CodeLock
		LPINT pExtentPosForExe; // for CodeLock
		LPCH* pNameForExe; // for CodeLock
		INT nCntForExe; // for CodeLock
	} PROTECT;
	struct _DVD {
		UCHAR ucBca;
		UCHAR pad[3];
		DISC_TYPE disc;
		PROTECT_TYPE_DVD protect;
		DWORD dwFixNum;
		DWORD dwDVDStartPsn;
		DWORD dwXBOXStartPsn;
		DWORD dwLayer0SectorLength;
		DWORD dwLayer1SectorLength;
		DWORD securitySectorRange[23][2];
	} DVD;
} DISC, *PDISC;

typedef struct _VOLUME_DESCRIPTOR {
	struct _ISO_9660 {
		DWORD dwLogicalBlkCoef;
		DWORD dwPathTblSize;
		DWORD dwPathTblPos;
		DWORD dwRootDataLen;
	} ISO_9660;
	struct _JOLIET {
		DWORD dwLogicalBlkCoef;
		DWORD dwPathTblSize;
		DWORD dwPathTblPos;
		DWORD dwRootDataLen;
	} JOLIET;
} VOLUME_DESCRIPTOR, *PVOLUME_DESCRIPTOR;

typedef struct _DIRECTORY_RECORD {
	DWORD dwDirNameLen;
	DWORD dwPosOfDir;
	DWORD dwNumOfUpperDir;
	CHAR szDirName[MAX_FNAME_FOR_VOLUME];
	DWORD dwDirSize;
} DIRECTORY_RECORD, *PDIRECTORY_RECORD;

// This buffer stores all CD data (main + c2 + sub) obtained from SCSI read command
// Depending on the situation, this may store main, main + sub.
typedef struct _DATA_IN_CD {
	LPBYTE current;
	LPBYTE next;
	LPBYTE nextNext;
} DATA_IN_CD, *PDATA_IN_CD;

// This buffer stores the header of mode 1 of main channel obtained from DATA_IN_CD structure
// If it doesn't exist in DATA_IN_CD, set manually
typedef struct _MAIN_HEADER {
	BYTE prev[MAINHEADER_MODE1_SIZE];
	BYTE current[MAINHEADER_MODE1_SIZE];
} MAIN_HEADER, *PMAIN_HEADER;

// This buffer stores the aligned subcode obtained from DATA_IN_CD structure
typedef struct _SUBCODE {
	BYTE current[CD_RAW_READ_SUBCODE_SIZE];
	BYTE next[CD_RAW_READ_SUBCODE_SIZE];
	BYTE nextNext[CD_RAW_READ_SUBCODE_SIZE];
} SUBCODE, *PSUBCODE;

// This buffer stores the Q channel of SUBCODE structure
typedef struct _SUB_Q_PER_SECTOR {
	BYTE reserved;
	BYTE byCtl : 4;		// 13th byte
	BYTE byAdr : 4;		// 13th byte
	BYTE byTrackNum;	// 14th byte
	BYTE byIndex;		// 15th byte
	INT nRelativeTime;	// 16th - 18th byte
	INT nAbsoluteTime;	// 20th - 22nd byte
} SUB_Q_PER_SECTOR, *PSUB_Q_PER_SECTOR;

typedef struct _SUB_Q {
	SUB_Q_PER_SECTOR prevPrev;
	SUB_Q_PER_SECTOR prev;
	SUB_Q_PER_SECTOR current;
	SUB_Q_PER_SECTOR next;
	SUB_Q_PER_SECTOR nextNext;
} SUB_Q, *PSUB_Q;

typedef struct _DISC_PER_SECTOR {
	DATA_IN_CD data;
	MAIN_HEADER mainHeader;
	SUBCODE subcode;
	SUB_Q subQ;
	DWORD dwC2errorNum;
	BYTE byTrackNum;
	BYTE padding[3];
	BOOL bLibCrypt;
	BOOL bSecuRom;
} DISC_PER_SECTOR, *PDISC_PER_SECTOR;

// This buffer stores the R to W channel (only use to check)
typedef struct _SUB_R_TO_W {
	CHAR command;
	CHAR instruction;
	CHAR parityQ[2];
	CHAR data[16];
	CHAR parityP[4];
} SUB_R_TO_W, *PSUB_R_TO_W;

#pragma pack(push, mds, 1)
typedef struct _MDS_HEADER {
	UCHAR fileId[16];
	WORD unknown1;
	WORD mediaType;
	WORD sessionNum;
	DWORD unknown2;
	WORD lenOfBca;
	UCHAR zero1[8];
	DWORD ofsToBca;
	UCHAR zero2[24];
	DWORD ofsToDS;
	UCHAR zero3[12];
	DWORD ofsTo1stSessionBlk;
	DWORD ofsToDpm;
	UCHAR zero4[8];
} MDS_HEADER, *PMDS_HEADER;

typedef struct _MDS_FOR_DVD_BLK {
	UCHAR bca[2048];
	DWORD zero;
	DVD_FULL_LAYER_DESCRIPTOR layer;
} MDS_FOR_DVD_BLK, *PMDS_FOR_DVD_BLK;

typedef struct _MDS_SESSION_BLK {
	DWORD startSector;
	DWORD endSector;
	WORD sessionNum;
	UCHAR totalDataBlkNum;
	UCHAR DataBlkNum;
	WORD firstTrackNum;
	WORD lastTrackNum;
	DWORD zero;
	DWORD ofsTo1stDataBlk;
} MDS_SESSION_BLK, *PMDS_SESSION_BLK;

typedef struct _MDS_DATA_BLK {
	UCHAR trackMode;
	UCHAR numOfSubch;
	UCHAR adrCtl;
	UCHAR trackNum;
	UCHAR point;
	DWORD zero;
	UCHAR m;
	UCHAR s;
	UCHAR f;
	DWORD ofsToIndexBlk;
	WORD sectorSize;
	UCHAR unknown;
	UCHAR zero2[17];
	DWORD trackStartSector;
	DWORD ofsFromHeadToIdx1;
	DWORD unknown2;
	DWORD NumOfFname;
	DWORD OfsToFname;
	UCHAR zero3[24];
} MDS_DATA_BLK, *PMDS_DATA_BLK;

typedef struct _MDS_IDX_BLK {
	DWORD NumOfIdx0;
	DWORD NumOfIdx1;
} MDS_IDX_BLK, *PMDS_IDX_BLK;

typedef struct _MDS_FNAME_BLK {
	DWORD ofsToFname;
	UCHAR fnameFmt;
	UCHAR zero4[11];
	__wchar_t fnameString[6];
} MDS_FNAME_BLK, *PMDS_FNAME_BLK;

typedef struct _MDS_DPM_HEADER {
	DWORD dpmBlkTotalNum;
#if !defined(__midl)
	DWORD ofsToDpmBlk[0];
#endif
} MDS_DPM_HEADER, *PMDS_DPM_HEADER;

typedef struct _MDS_DPM_BLK {
	DWORD dpmBlkNum;
	DWORD unknown1;
	DWORD resolution;
	DWORD entry;
#if !defined(__midl)
	DWORD readTime[0];
#endif
} MDS_DPM_BLK, *PMDS_DPM_BLK;
#pragma pack(pop, mds)
