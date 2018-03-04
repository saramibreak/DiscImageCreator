/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

typedef enum _EXEC_TYPE {
	cd,
	data,
	audio,
	dvd,
	bd,
	raw,
	gd,
	fd,
	stop,
	start,
	eject,
	closetray,
	reset,
	sub,
	mds
} EXEC_TYPE, *PEXEC_TYPE;

typedef enum _LOG_TYPE {
	standardOut = 1,
	standardError = 1 << 1,
	fileDisc = 1 << 2,
	fileVolDesc = 1 << 3,
	fileDrive = 1 << 4,
	fileMainInfo = 1 << 5,
	fileMainError = 1 << 6,
	fileSubInfo = 1 << 7,
	fileSubIntention = 1 << 8,
	fileSubError = 1 << 9,
	fileC2Error = 1 << 10
} LOG_TYPE, *PLOG_TYPE;

typedef enum _MAIN_DATA_TYPE {
	unscrambled,
	scrambled
} MAIN_DATA_TYPE, *PMAIN_DATA_TYPE;

typedef enum _TRACK_TYPE {
	audioOnly,
	dataExist,
	pregapIn1stTrack,
} TRACK_TYPE, *PTRACK_TYPE;

typedef enum _PROTECT_TYPE_CD {
	no,
	cdidx,
	cds300,
	codelock,
	laserlock,
	proring,
	protectCDVOB,
	safeDiscLite,
	safeDisc,
	securomTmp,
	securomV1, // a.k.a SecuROM OLD (8 shifted RMSF/AMSF + 1 error * 24 times = 216 sector error)
	securomV2, // a.k.a SecuROM NEW (8 shifted RMSF/AMSF + 1 error * 10 times = 90 sector error)
	securomV3, // a.k.a SecuROM NEW (8 shifted RMSF/AMSF + LBA -1 + 10 random error = 99 sector error) 
	securomV4, // a.k.a SecuROM NEW (LBA -1 + 10 random error = 11 sector error)
	smartE,
	bluebyte,
	microids
} PROTECT_TYPE_CD, *PPROTECT_TYPE_CD;

typedef enum _PROTECT_TYPE_DVD {
	noProtect,
	css,
	cprm,
	aacs
} PROTECT_TYPE_DVD, *PPROTECT_TYPE_DVD;

typedef enum _PATH_TYPE {
	lType,
	mType
} PATH_TYPE, *PPATH_TYPE;

// a naming rule of PLEXTOR drive
// Plefix
//    PX = PLEXTOR
// Infix
//   7xx = DVD+R drive, DVD+R write speed is xx (704 - 716)
//   3xx = DVD-ROM drive, CD-RW rewrite speed is yy
//     W = CD-RW drive
//      xxyy = CD-R write speed is xx, CD-RW rewrite speed is yy
//      xyzz = CD-R write speed is x, CD-RW rewrite speed is y, CD-ROM read speed is zz
//   Sxy = Slim drive, CD-R write speed is x, CD-RW rewrite speed is y
//  Rxyy = CD-R drive, CD-R write speed is x, CD-ROM read speed is yy
//    xx = CD-ROM drive, CD-ROM read speed is xx
// Suffix
//     A = ATA
//     L = Slot Loading
//     T = Tray
//     C = Caddy
//     S = Scsi
//     U = Ultra
//     W = Wide
typedef enum _PLXTR_DRIVE_TYPE {
	No,
	PX760A,
	PX755A,
	PX716AL,
	PX716A,
	PX714A,
	PX712A,
	PX708A2,
	PX708A,
	PX704A,
	PX320A,
	PREMIUM2,
	PREMIUM,
	PXW5224A,
	PXW4824A,
	PXW4012A,
	PXW4012S,
	PXW2410A,
	PXS88T,
	PXW1610A,
	PXW1210A,
	PXW1210S,
	PXW124TS,
	PXW8432T,
	PXW8220T,
	PXW4220T,
	PXR820T,
	PXR412C,
	PX40TS,
	PX40TSUW,
	PX40TW,
	PX32TS,
	PX32CS,
	PX20TS,
	PX12TS,
	PX12CS,
	PX8XCS
} PLXTR_DRIVE_TYPE, *PPLXTR_DRIVE_TYPE;

typedef enum _DRIVE_DATA_ORDER {
	NoC2,
	MainC2Sub, // PLEXTOR, nec-based drive
	MainSubC2 // mediatek-based drive
} DRIVE_DATA_ORDER, *PDRIVE_DATA_ORDER;

typedef enum _SUB_RTOW_TYPE {
	Zero,
	CDG,
	Full,
	AnyFull,
	PSXSpecific
} SUB_RTOW_TYPE, *PSUB_RTOW_TYPE;
