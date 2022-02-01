/**
 * Copyright 2011-2021 sarami
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
#include "calcHash.h"

// memo
//  CONST   => const
//  LPBOOL  => BOOL*
//  CONST LPBOOL => BOOL* const
//  CONST BOOL*  => const BOOL*
#ifdef _WIN32
#pragma pack(push, sensedata, 1)
typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	SCSI_PASS_THROUGH_DIRECT Sptd;
	SENSE_DATA SenseData;
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;
#pragma pack(pop, sensedata)
#endif

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
	FILE* fpSubReadable;
	FILE* fpSubError;
	FILE* fpC2Error;
	FILE* fpRawReadable;
	FILE* fpMdsReadable;
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
	BYTE byVideoNow;
	BYTE byVideoNowColor;
	BYTE byVideoNowXp;
	BYTE byNoSkipSS;
	BYTE byAtari;
	BYTE byAnchorVolumeDescriptorPointer;
	BYTE byMicroSoftCabFile;
	BYTE byPadSector;
	BYTE byMultiSectorReading; // for 0xF1 supported drive
	BYTE byVerifyAudioCDOfs;
	BYTE byPadding[2];
	INT nAudioCDOffsetNum;
	UINT uiMaxRereadNum; // for c2 error
	INT nAllSectors;	// use for xbox360
	UINT uiSecuritySector[16];	// use for xbox/xbox360
	INT nC2RereadingType; // 0 or 1
	INT nStartLBAForC2;
	INT nEndLBAForC2;
	UINT uiCacheDelNum; // delete cache of the drive per value. Default is 1 (DEFAULT_CACHE_DELETE_VAL)
	DWORD dwTimeoutNum; // default is 60 (DEFAULT_SPTD_TIMEOUT_VAL)
	UINT uiSubAddionalNum;
	UINT uiSkipSectors; // for proring, LaserLock
	UINT uiSkipSectors2; // for some LaserLock
	UINT uiPadNum; // 0 : main channel is padded by 0x00, 1 : main channel is padded by 0xAA
	UINT uiRetryCnt; // for 0xf1 drive
	UINT uiVerifyAudio; // for /vrfy
	struct _FILE {
		CHAR readError[MAX_READ_ERROR_FILE_COUNT][MAX_FNAME_FOR_VOLUME];
		INT readErrCnt;
		CHAR c2Error[MAX_READ_ERROR_FILE_COUNT][MAX_FNAME_FOR_VOLUME];
		INT c2ErrCnt;
	} FILE;
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
	CHAR szVendorSpecific[20];
	BYTE byPlxtrDrive;
	BYTE by0xF1Drive;
	BYTE byLoadingMechanism;
#ifdef _WIN32
	BYTE byDriveLetter;
#else
	CHAR drivepath[10];
#endif
	WORD wMaxReadSpeed;
	BYTE bySuccessReadToc;
	BYTE bySuccessReadTocFull;
	CDFLAG::_READ_CD::_ERROR_FLAGS supportedC2Type;
	CDFLAG::_READ_CD::_SUB_CHANNEL_SELECTION sub;
	DWORD dwTimeOutValue;
	DRIVE_DATA_ORDER driveOrder;
	INT nDriveSampleOffset;
	struct _TRANSFER {
		UINT uiBufLen;
		UINT uiBufC2Offset;
		UINT uiBufSubOffset;
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
		INT nAllLength;					// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPINT lp1stLBAListOnToc;		// get at CDROM_READ_TOC_EX_FORMAT_TOC
		LPINT lpLastLBAListOnToc;		// get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT n1stLBAofDataTrk;			// get at CDROM_READ_TOC_EX_FORMAT_TOC
		INT nLastLBAofDataTrkOnToc;		// get at CDROM_READ_TOC_EX_FORMAT_TOC
		TRACK_TYPE trkType;				// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE by1stDataTrkNum;			// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byLastDataTrkNum;			// get at CDROM_READ_TOC_EX_FORMAT_TOC
		BYTE byFormat;					// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		BYTE by1stMultiSessionTrkNum;	// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		BOOL bMultiSession;				// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		LPBYTE lpSessionNumList;		// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		INT n1stLBAofLeadout;			// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		INT n1stLBAofLeadin;
		INT nLeadoutLenOf1stSession;
		INT nLeadinLenOf2ndSession;
		INT nEndLBAOfLeadin;
		INT nPregapLenOf1stTrkOf2ndSession;
		INT n1stLBAof2ndSession;		// get at CDROM_READ_TOC_EX_FORMAT_FULL_TOC
		struct _CDTEXT {
			LPSTR* pszTitle;			// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
			LPSTR* pszPerformer;		// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
			LPSTR* pszSongWriter;		// get at CDROM_READ_TOC_EX_FORMAT_CDTEXT
			BOOL bExist;
		} CDTEXT[MAX_CDTEXT_LANG];
		WORD wCurrentMedia;				// get at SCSIOP_GET_CONFIGURATION
		BYTE padding2[2];
	} SCSI;
	struct _MAIN {
		INT nAdjustSectorNum;
		INT nCombinedOffset;
		INT nCombinedOffsetOrg;
		UINT uiMainDataSlideSize;
		INT nOffsetStart;
		INT nOffsetEnd;
		INT nFixStartLBA;
		INT nFixEndLBA;
		INT nFix1stLBAofLeadout;	// for sliding offset
		INT nFix1stLBAof2ndSession;	// for sliding offset
		LPBYTE lpModeList; // 0 origin, max is last track num.
		LPDWORD lpAllSectorCrc32;
		LPINT lpAllLBAOfC2Error;
		INT nC2ErrorCnt;
		BOOL bResetOffset;
		BOOL bManySamples;
	} MAIN;
	struct _SUB {
		INT nSubChannelOffset;
		INT n1stLBAForMCN[3][2];
		INT nRangeLBAForMCN[3][2];
		INT nPrevMCNSector;
		BYTE byDesync;
		BYTE byCatalog;
		CHAR szCatalog[META_CATALOG_SIZE];
		BYTE byAdr6;
		CHAR szAdr6[META_ADR6_SIZE];
		BYTE byCtlOfLBA0; // for pregap sector of track 1
		BYTE byIdxOfLBA0; // for pregap sector of track 1
		INT n1stLBAForISRC[3][2];
		INT nRangeLBAForISRC[3][2];
		INT nPrevISRCSector;
		LPSTR* pszISRC; // 0 origin, max is last track num.
		LPINT* lp1stLBAListOnSub; // 0 origin, max is last track num. // toc indexes in priority. single ptr: LBA per track. double ptr: LBA per index
		LPINT* lp1stLBAListOnSubSync; // 0 origin, max is last track num. // sub indexes in priority. single ptr: LBA per track. double ptr: LBA per index
		LPINT lp1stLBAListOfDataTrackOnSub; // 0 origin, max is last track num.
		LPINT lpLastLBAListOfDataTrackOnSub; // 0 origin, max is last track num.
		LPBYTE lpCtlList; // 0 origin, max is last track num.
		LPBYTE lpEndCtlList; // 0 origin, max is last track num.
		LPBOOL lpISRCList; // 0 origin, max is last track num.
		LPBYTE lpRtoWList; // 0 origin, max is last track num.
		INT nCorruptCrcH;
		INT nCorruptCrcL;
		INT n1stRmsfOfTrk;
		INT n1stPchannelOfTrk;
	} SUB;
	struct _PROTECT {
		BYTE byExist;
		BYTE byTmpForSafeDisc;
		BYTE byRestoreCounter; // for SecuROM
		BYTE reserved;
		CHAR name[MAX_READ_ERROR_FILE_COUNT][MAX_FNAME_FOR_VOLUME];
		CHAR name2[MAX_FNAME_FOR_VOLUME]; // for Der KorsaR, DVD Region X
		CHAR padding[3];
		struct _ERROR_SECTOR { // for skipping unreadable file
			INT nExtentPos[MAX_READ_ERROR_FILE_COUNT];
			INT nNextExtentPos; // for safedisc
			INT nSectorSize[MAX_READ_ERROR_FILE_COUNT];
			INT nExtentPos2nd; // for Der KorsaR, DVD Region X
			INT nSectorSize2nd; // for Der KorsaR, DVD Region X
		} ERROR_SECTOR;
		INT nPrevLBAOfPathTablePos; // for CodeLock
		INT nNextLBAOfLastVolDesc; // for CodeLock
		LPINT pExtentPosForExe;
		LPINT pSectorSizeForExe;
		LPINT pDataLenForExe;
		LPCH* pNameForExe;
		LPCH* pFullNameForExe;
		INT nCntForExe;
	} PROTECT;
	struct _DVD {
		UCHAR ucBca;
		UCHAR version; // for DVD-RAM
		UCHAR pad[2];
		DISC_TYPE_DVD discType;
		PROTECT_TYPE_DVD protect;
		UINT fixNum;
		DWORD dwDVDStartPsn;
		DWORD dwXboxStartPsn;
		DWORD dwLayer0SectorLength;
		DWORD dwLayer1SectorLength;
		DWORD dwXboxLayer0SectorLength;
		DWORD dwXboxLayer1SectorLength;
		DWORD securitySectorRange[23][2]; // for Xbox
		DWORD dwXboxSwapOfs;
	} DVD;
	struct _BD {
		INT nLBAForParamSfo[MAX_PARAMSFO_NUM];
		INT nParamSfoCnt;
	} BD;
	LPBYTE lpCachedBuf; // for Asus 0xF1 opcode
	UINT uiCachedSectorNum; // for Asus 0xF1 opcode
	DWORD dwBytesPerSector; // only use by disk command
} DISC, *PDISC;

typedef struct _VOLUME_DESCRIPTOR {
	struct _ISO_9660 {
		UINT uiLogicalBlkCoef;
		UINT uiPathTblSize;
		UINT uiPathTblPos;
		UINT uiRootDataLen;
	} ISO_9660;
	struct _JOLIET {
		UINT uiLogicalBlkCoef;
		UINT uiPathTblSize;
		UINT uiPathTblPos;
		UINT uiRootDataLen;
	} JOLIET;
	BOOL bPathType; // use path table record
	UINT uiVolumeSpaceSize;
} VOLUME_DESCRIPTOR, *PVOLUME_DESCRIPTOR;

typedef struct _PATH_TABLE_RECORD {
	UINT uiDirNameLen;
	UINT uiPosOfDir;
	UINT uiNumOfUpperDir;
	CHAR szDirName[MAX_FNAME_FOR_VOLUME];
	CHAR padding[3];
	UINT uiDirSize; // This is actually DIRECTORY RECORD info
} PATH_TABLE_RECORD, *PPATH_TABLE_RECORD;

typedef struct _UDF {
	UINT uiPVDPos;	// from Anchor Volume Descriptor Pointer
	UINT uiPVDLen;	// from Anchor Volume Descriptor Pointer
	UINT uiFSDPos;	// from Logical Volume Descriptor
	UINT uiFSDLen;	// from Logical Volume Descriptor
	UINT uiLogicalVolumeIntegrityPos;	// from Logical Volume Descriptor
	UINT uiLogicalVolumeIntegrityLen;	// from Logical Volume Descriptor
	UINT uiPartitionPos;	// from Partition Descriptor
	UINT uiPartitionLen;	// from Partition Descriptor
	UINT uiFileEntryPos;	
	UINT uiFileEntryLen;
} UDF, *PUDF;

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
	BYTE prev[CD_RAW_READ_SUBCODE_SIZE];
	BYTE current[CD_RAW_READ_SUBCODE_SIZE];
	BYTE next[CD_RAW_READ_SUBCODE_SIZE];
	BYTE nextNext[CD_RAW_READ_SUBCODE_SIZE];
} SUBCODE, *PSUBCODE;

// This buffer stores the PQ channel of SUBCODE structure
typedef struct _SUBCH_PER_SECTOR {
	BYTE byP;
	BYTE byCtl : 4;		// 13th byte
	BYTE byAdr : 4;		// 13th byte
	BYTE byTrackNum;	// 14th byte
	BYTE byIndex;		// 15th byte
	INT nRelativeTime;	// 16th - 18th byte
	INT nAbsoluteTime;	// 20th - 22nd byte
} SUBCH_PER_SECTOR, *PSUBCH_PER_SECTOR;

typedef struct _SUBCH {
	SUBCH_PER_SECTOR prevPrev;
	SUBCH_PER_SECTOR prev;
	SUBCH_PER_SECTOR current;
	SUBCH_PER_SECTOR next;
	SUBCH_PER_SECTOR nextNext;
} SUBCH, *PSUBCH;

typedef struct _DISC_PER_SECTOR {
	DATA_IN_CD data;
	MAIN_HEADER mainHeader;
	SUBCODE subcode;
	SUBCH subch;
	UINT uiC2errorNum;
	BYTE byTrackNum;
	BYTE padding[3];
	BOOL bNextTrk;
	BOOL bLibCrypt;
	BOOL bSecuRom;
	BOOL bReturnCode;
} DISC_PER_SECTOR, *PDISC_PER_SECTOR;

// This buffer stores the R to W channel (only use to check)
typedef struct _SUB_R_TO_W {
	CHAR command;
	CHAR instruction;
	CHAR parityQ[2];
	CHAR data[16];
	CHAR parityP[4];
} SUB_R_TO_W, *PSUB_R_TO_W;

#pragma pack(push, hash, 1)
typedef struct _HASH_CHUNK {
	DWORD crc32;
	MD5_CTX md5;
	SHA1Context sha;
	_TCHAR szFnameAndExt[_MAX_FNAME + _MAX_EXT];
	UINT64 ui64FileSize;
} HASH_CHUNK, * PHASH_CHUNK;
#pragma pack(pop, hash)

typedef struct _HASH {
	PHASH_CHUNK pHashChunk;
	UINT uiIndex;
	UINT uiCount;
	UINT uiMax;
} HASH, *PHASH;

#pragma pack(push, mds, 1)
typedef struct _MDS_HEADER {
	UCHAR fileId[16];
	WORD unknown1;
	WORD mediaType;
	WORD sessionNum;
	UINT unknown2;
	WORD lenOfBca;
	UCHAR zero1[8];
	UINT ofsToBca;
	UCHAR zero2[24];
	UINT ofsToDiscStructures;
	UCHAR zero3[12];
	UINT ofsTo1stSessionBlk;
	UINT ofsToDpm;
	UCHAR zero4[8];
} MDS_HEADER, *PMDS_HEADER;

typedef struct _MDS_FOR_DVD_BLK {
	PDVD_BCA_DESCRIPTOR bca;
	DVD_COPYRIGHT_DESCRIPTOR copyright;
	DVD_MANUFACTURER_DESCRIPTOR dmi;
	DVD_FULL_LAYER_DESCRIPTOR layer;
} MDS_FOR_DVD_BLK, *PMDS_FOR_DVD_BLK;

typedef struct _MDS_SESSION_BLK {
	INT startSector;
	UINT endSector;
	WORD sessionNum;
	UCHAR totalDataBlkNum;
	UCHAR DataBlkNum;
	WORD firstTrackNum;
	WORD lastTrackNum;
	UCHAR zero[4];
	UINT ofsTo1stDataBlk;
} MDS_SESSION_BLK, *PMDS_SESSION_BLK;

typedef struct _MDS_DATA_BLK {
	UCHAR trackMode;
	UCHAR numOfSubch;
	UCHAR Control : 4;
	UCHAR Adr : 4;
	UCHAR Reserved1;
	UCHAR Point;
	UCHAR MsfExtra[3];
	UCHAR Zero;
	UCHAR Msf[3];
	UINT ofsToIndexBlk;
	WORD sectorSize;
	UCHAR unknown1;
	UCHAR zero2[17];
	UINT trackStartSector;
	UINT64 trackStartOffset;
	UINT NumOfFname;
	UINT OfsToFname;
	UCHAR zero3[24];
} MDS_DATA_BLK, *PMDS_DATA_BLK;

typedef struct _MDS_IDX_BLK {
	UINT NumOfIdx0;
	UINT NumOfIdx1;
} MDS_IDX_BLK, *PMDS_IDX_BLK;

typedef struct _MDS_FNAME_BLK {
	UINT ofsToFname;
	UCHAR fnameFmt;
	UCHAR zero4[11];
	__wchar_t fnameString[6];
} MDS_FNAME_BLK, *PMDS_FNAME_BLK;

typedef struct _MDS_DPM_HEADER {
	UINT dpmBlkTotalNum;
#if !defined(__midl)
	UINT ofsToDpmBlk[0];
#endif
} MDS_DPM_HEADER, *PMDS_DPM_HEADER;

typedef struct _MDS_DPM_BLK {
	UINT dpmBlkNum;
	UINT unknown1;
	UINT resolution;
	UINT entry;
#if !defined(__midl)
	UINT readTime[0];
#endif
} MDS_DPM_BLK, *PMDS_DPM_BLK;
#pragma pack(pop, mds)

typedef struct _FAT {
	BYTE SecPerClus;
	BYTE padding;
	WORD RootEntCnt;
	UINT RootDirStartSector;
	UINT DataStartSector;
} FAT, *PFAT;

// https://docs.microsoft.com/en-us/windows/win32/menurc/vs-versioninfo
#pragma pack(push, version, 1)
typedef struct {
	WORD             wLength;
	WORD             wValueLength;
	WORD             wType;
	WCHAR            szKey[16];
	WORD             Padding1;
	VS_FIXEDFILEINFO Value;
} VS_VERSIONINFO, *PVS_VERSIONINFO;

// https://docs.microsoft.com/en-us/windows/win32/menurc/stringfileinfo
typedef struct {
	WORD        wLength;
	WORD        wValueLength;
	WORD        wType;
	WCHAR       szKey[15];
} StringFileInfo;

// https://docs.microsoft.com/en-us/windows/win32/menurc/stringtable
typedef struct {
	WORD   wLength;
	WORD   wValueLength;
	WORD   wType;
	WCHAR  szKey[9];
} StringTable;

// https://docs.microsoft.com/en-us/windows/win32/menurc/string-str
typedef struct {
	WORD  wLength;
	WORD  wValueLength;
	WORD  wType;
#if !defined(__midl)
	WCHAR szKey[0];
#endif
} String, *PString;

typedef struct {
	VS_VERSIONINFO ver;
	StringFileInfo sfi;
	StringTable     st;
} FILE_VERSIONINFO, *PFILE_VERSIONINFO;
#pragma pack(pop, version)
