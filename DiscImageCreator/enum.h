/*
 * This code is released under the Microsoft Public License (MS-PL). See License.txt, below.
 */
#pragma once

typedef enum _EXEC_TYPE {
	cd,
	data,
	audio,
	dvd,
	gd,
	fd,
	stop,
	start,
	eject,
	closetray,
	reset,
	sub
} EXEC_TYPE, *PEXEC_TYPE;

typedef enum _LOG_TYPE {
	standardOut = 1,
	standardErr = 1 << 1,
	fileDisc = 1 << 2,
	fileVolDesc = 1 << 3,
	fileDrive = 1 << 4,
	fileMainInfo = 1 << 5,
	fileMainError = 1 << 6,
	fileSubInfo = 1 << 7,
	fileSubError = 1 << 8,
	fileC2Error = 1 << 9
} LOG_TYPE, *PLOG_TYPE;

typedef enum _MAIN_DATA_TYPE {
	unscrambled,
	scrambled
} MAIN_DATA_TYPE, *PMAIN_DATA_TYPE;

typedef enum _PROTECT_TYPE {
	no,
	cdidx,
	codelock,
	laserlock,
	proring,
	protectCDVOB,
	safeDiscLite,
	safeDisc,
	smartE
} PROTECT_TYPE, *PPROTECT_TYPE;

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
