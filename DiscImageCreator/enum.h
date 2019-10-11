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

typedef enum _EXEC_TYPE {
	cd,
	swap,
	data,
	audio,
	gd,
	dvd,
	xbox,
	xboxswap,
	xgd2swap,
	xgd3swap,
	bd,
	sacd,
	fd,
	disk,
	stop,
	start,
	ejecttray,
	closetray,
	reset,
	drivespeed,
	sub,
	mds,
	merge
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

typedef enum _BYTE_PAD_TYPE {
	padByAll0,
	padByUsr0,
	padByUsr55,
	padByPrevSector
} BYTE_PAD_TYPE, *PBYTE_PAD_TYPE;

typedef enum _TRACK_TYPE {
	audioOnly,
	dataExist,
	pregapAudioIn1stTrack,
	pregapDataIn1stTrack
} TRACK_TYPE, *PTRACK_TYPE;

// Protection list
// https://www.cdmediaworld.com/hardware/cdrom/cd_protections.shtml
typedef enum _PROTECT_TYPE_CD {
	no,
	cds300,
	codelock,
	datel,
	datelAlt,
	laserlock,
	proring,
	protectCDVOB,
	safeDiscLite,
	safeDisc,
	securomTmp,
	securomV1, // a.k.a SecuROM OLD ((8 shifted RMSF/AMSF + 1 error) * 24 times = 216 sector error)
	securomV2, // a.k.a SecuROM NEW ((8 shifted RMSF/AMSF + 1 error) * 10 times = 90 sector error)
	securomV3_1, // a.k.a SecuROM NEW ((8 shifted RMSF/AMSF + 1 error) * 11 times = 99 sector error) 
	securomV3_2, // a.k.a SecuROM NEW ((8 shifted RMSF/AMSF + 1 error) * 11 times = 99 sector error) 
	securomV4, // a.k.a SecuROM NEW (LBA -1 + 10 random error = 11 sector error)
	smartE,
	ripGuard,
	physicalErr,
	edcEccErr,
	microids
} PROTECT_TYPE_CD, *PPROTECT_TYPE_CD;

typedef enum _PROTECT_TYPE_DVD {
	noProtect,
	css,
	cprm,
	aacs
} PROTECT_TYPE_DVD, *PPROTECT_TYPE_DVD;

typedef enum _DISC_TYPE {
	formal,	// DVD, BD
	gamecube,
	wii
} DISC_TYPE, *PDISC_TYPE;

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
	NotLatest,
	Other,
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

// SONY PS-SYSTEM   302R and 408R
typedef enum _FEATURE_PROFILE_TYPE_EX {
	ProfilePlaystationCdrom = 0xff50,
	ProfilePlaystation2Cdrom = 0xff60,
	ProfilePlaystation2DvdRom = 0xff61,
	ProfilePlaystation3DvdRom = 0xff70,
	ProfilePlaystation3BDRom = 0xff71,
	ProfilePlaystation4BDRom = 0xff80,
} FEATURE_PROFILE_TYPE_EX, *PFEATURE_PROFILE_TYPE_EX;
