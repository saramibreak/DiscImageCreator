============================= What is this tool? ==============================
* Summary
  This command-line program rips CD, DVD, GD, Floppy. In case of CD and GD, it
 rips considering a drive + CD (=combined) offset.
  It works on Windows PC (WinXP or higher).

* Recommend drive
 CD: (Drives must be able to rip by scrambled mode and read lead-out and read lead-in).
   - PLEXTOR (No OEM Drive)
     - PX-760, PX-755, PX-716, PX-714, PX-712, PX-708, PX-704, 
       Premium2, Premium, PX-W5224, PX-4824, PX-4012 (PX-714, PX-704 can't test it enough.)
       - Firmware of a plextor drive: http://www.skcj.co.jp/discon/download/index.html
         (Without a reason, you should update the latest firmware.)
       - PleXTools Professional XL: https://web.archive.org/web/*/http://www.plextor-digital.com/index.php/component/option,com_jdownloads/Itemid,55/catid,86/cid,470/task,view.download//
       - PX-Info Utility: https://web.archive.org/web/*/http://www.plextor-digital.com/index.php/component/option,com_jdownloads/Itemid,0/catid,86/cid,468/task,view.download/
          Useful tool for PLEXTOR.
   - HL-DT-ST
     - UH12NS30 (Combined Offset minus disc only)
 GD: http://forum.redump.org/post/14552/#p14552
   and TSSTcorp(TS-H353A, TS-H352C, TS-H192C)
 DVD: All supported drive
 Floppy: All supported drive

* Not recommend (because can't read lead-in and/or lead-out, can't exec 0xd8 command)
 CD:
   - PLEXTOR (OEM drive)
      PX-8xx/PX-Bxxx(many maker), PX-751A(BenQ DW1670), PX-750A/UF(TEAC DV-W516E),
      PX-740A/UF(BenQ DW1640), PX-6xx(Pioneer, Panasonic and so on),
      PX-504A/UF(NEC ND-1100A), PX-2xx(Lite-on, BenQ, NEC), PX-1xx(Pioneer and so on),
      PX-S2410TU(TEAC CD-W224E), PX-54TA(Mitsumi FX5400), PX-R24CS(RICOH RO-1420C),
   - Other maker
 Protected CD:
   - SecuRom 3
    - PLEXTOR
       PX-4824A (ecc/edc of the 2 sector doesn't match)
   - CDS200, Label Gate, XCP
    - PLEXTOR
       PX-4824A (doesn't get the TOC correctly)

* Development Tool
** first release - 20131124
 Visual Studio 2010
 Windows Driver Kit(WDK)
  Sample code path: WinDDK\7600.16385.1\src\storage\tools\spti
  url: http://msdn.microsoft.com/en-us/library/windows/hardware/ff561595(v=vs.85).aspx
** 20131217 - 20151128
 Visual Studio 2013 (including WDK)
** 20160120 - 
 Visual Studio 2015 (including WDK)

* License
 See License.txt.
 About driveOffset.txt.
  http://www.accuraterip.com/driveoffsets.htm
  Copyright (c) 2015 Illustrate. All Rights Reserved.
 About _external folder
  scramble data to CD: Copyright (c) 2015 Jonathan Gevaryahu
  crc16: http://oku.edu.mie-u.ac.jp/~okumura/algo/
         src\crc16.c in algo.lzh
  crc32: http://www.ietf.org/rfc.html
         using rfc1952 sample code.
  md5: http://www.ietf.org/rfc.html
       using rfc1321 sample code.
  sha1: http://www.ietf.org/rfc.html
        using rfc3174 sample code.

* Disclaimer
 Use this tool at own your risk.
 Trouble in regard to the use of this tool, I can not guarantee any.

* Bug report
 To: http://forum.redump.org/topic/10483/discimagecreator/

* Gratitude
 Thank's redump.org users.

========================== Supported/Unsupported Disc =========================
* Supported Disc (I tested these)
 - Apple Macintosh
 - Audio CD
 - Bandai Playdia
 - DVD-Video
 - Fujitsu FM Towns series
 - IBM PC compatible
 - NEC PC-98 series CD
 - NEC PC-Engine CD
 - NEC PC-FX
 - Panasonic 3DO Interactive Multiplayer
 - Philips CD-i
 - Sega Dreamcast
 - Sega Mega-CD
 - Sega Saturn
 - Sony PlayStation
 - Sony PlayStation 2
 - SNK Neo Geo CD
 - VCD

* Support WIP (Because I haven't enough to test)
 - Protected Disc
    bad(error) sector                   => SafeDisc, SmartE, Cactus Data Shield 300
    weak sector                         => SafeDisc v2 or higher
    unique data on subchannel           => LibCrypt, SecuROM(v1 - v4.8x)
    no signal sector                    => LaserLock, RingPROTECH, ProRing
    Intensional(deliberate) C1/C2 error => Cactus Data Shield 200, CodeLock
    characteristic track                => CD Lock

* Supported Disc? (I haven't tested these yet)
 - Acorn Archimedes
 - Bandai / Apple Pippin
 - Commodore Amiga CD
 - Commodore Amiga CD32
 - Commodore Amiga CDTV
 - Mattel HyperScan
 - Microsoft Xbox
 - Microsoft Xbox 360
 - NEC PC-88 series CD
 - Sharp X68000 CD
 - Tandy / Memorex Visual Information System
 - Tao iKTV CD
 - Tomy Kiss-Site CD
 - VM Labs NUON DVD
 - VTech V.Flash

* Unsupported Disc
 - Protected Disc
    different frequency               => SecuROM(v5 or higher), StarForce, CD-Cops
     These needs DPM(Data position measurement). cue, ccd doesn't support DPM.
     To store DPM, you need to use the Alcohol 120/52% (http://www.alcohol-soft.com/)
    duplicated(double, triple) sector => Alpha-ROM, ROOT, TAGES
     It can read in reverse, but specifications are not decided in redump.org
    Fake TOC                          => Cactus Data Shield 100
     Can't analyze the illegal TOC at the present.
 - HD DVD & Blu-ray Disc
    I don't have a drive, so I can't test.
 - Nintendo GameCube & Wii
    I don't implement a code to decrypt. (if you have a supported drive,
   you can rip a "encrypted" image.)

============================= Ripping information =============================
* Preparation
 Download and install Visual C++ Redistributable Packages.
  http://www.microsoft.com/en-us/download/details.aspx?id=40784
 EccEdc checking tool (for CD)
  http://www.mediafire.com/?ls3xlze3op452a5/
  Put it to directory of DiscImageCreator.exe.
  (If EccEdc doesn't exist, DiscImageCreator works.)

* Ripping Guide for CD, DVD, Floppy Disk
 Run exe without args to get detail info.

* Ripping Guide for SafeDisc
** Method 1 (Other tools)
 http://forum.redump.org/topic/2201/dumping-safedisc-cds/

** Method 2
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /rc

* Ripping Guide for SmartE, LaserLock, RingPROTECH, ProRing
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /rc

* Ripping Guide for LibCrypt, SecuROM(v1 - v4.8x)
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /se

* Ripping Guide for GD-ROM (The high density area)
** Preparation
 create the audio trap disc in advance.
 (a disc with a hacked TOC of 99 mins audio, burn it with CloneCD or Alcohol 52/120%).
   http://www.mediafire.com/?2nygv2oyzzz

** Method 1 (Other tools)
 http://forum.redump.org/topic/2620/dreamcastnaomi-gdrom-dumping-instructions/

** Method 2
1. insert the audio trap disc to a supported drive.
2. run below. (stop spinning disc)
   DiscImageCreator.exe stop [DriveLetter]
3. use a pin to press the escape eject button, so the tray will eject (or remove
   the drive cover).
4. insert the gdrom and gently push the tray back (or put the drive cover back on).
5. run below. (start rippping gdrom)
   DiscImageCreator.exe gd [DriveLetter] foo.bin [DriveSpeed(0-72)]

========================== Created files information ==========================
 .bin
  2352byte/sector binary image. This file is used to a cue file.
 .c2
  store the c2 error. 1bit expresses 1byte.
 .ccd
  store the CD information. Original format is CloneCD (http://www.slysoft.com/en/clonecd.html)
 .cue
  store the CD information. Original format is CDRWIN (https://web.archive.org/web/20111008191852/http://www.goldenhawk.com/cdrwin.htm)
 .dat
  store the crc32/md5/sha1 of bin file. Original format is clrmamepro (http://mamedev.emulab.it/clrmamepro/)
 .img
  2352byte/sector binary image. This file is used to a ccd file.
 .scm
  scrambled image file of img file.
 .sub
  store the sub channel data. This file is used to a ccd file.
 _c2error.txt
  store disc c2 error information getting read disc.
 _cmd.txt
  store the command-line argument.
 _disc.txt
  store the disc information returned by SCSI command.
 _drive.txt
  store the drive information returned by SCSI command.
 _mainError.txt
  store the disc main error information getting read disc.
 _mainInfo.txt
  store the disc main info information getting read disc.
 _sub.txt
  store the parsed sub channel file as text data.
 _subError.txt
  store the disc sub error information getting read disc.
 _subInfo.txt
  store the disc sub info information getting read disc.
 _volDesc.txt
  store the volume descriptor, path table, directory table of CD as text data.

========================= Drive information (I tested) ========================
Vendor					Model					Firmware(*1)	Lead-in	Lead-out	Scrambled	GD-ROM(*2)				Wii-ROM
LG(HL-DT-ST)			GCC-4320B				1.00			No		No			Yes			No						No
HITACHI(HL-DT-ST)		GDR-8161B				0045			No		No			No			No						Yes
HP(HL-DT-ST)			GDR-8163B				0B26			No		No			No			No						Yes
HITACHI-LG(HL-DT-ST)	GDR-8163B				0S24			No		No			No			No						Yes
HITACHI-LG(HL-DT-ST)	GWA-8164B(GDR8164B)		0M08			No		No			No			No						Yes
HITACHI-LG(HL-DT-ST)	GDR-H20N				0D04			No		No			No			No						Yes
HITACHI-LG(HL-DT-ST)	GDR-H20N				0D08			No		No			No			No						Yes
LITEON					DH-20A3S				9P58			No		No			Yes			No						No
LITEON					LH-18A1P				GL0J			Yes		Yes			No			No						No
LITEON					LH-20A1S				9L09			Yes		Yes			No			No						No
LITEON					LTD-163					GDHG			No		No			No			Partial(about 99:59:74)	No
LITEON(JLMS)			LTD-166S(XJ-HD166S)		DS1E			Yes		No			No			No						No
LITEON					SOHW-812S(SOHW-832S)	CG5M			Yes		Yes			No			No						No
MATSHITA				SW-9574S				ADX4			No		No			No			No						No
NEC						CDR-1700A(286)			3.06			No		No			No			No						No
NEC						CDR-3001A(28F)			3.32			No		No			No			No						No
Optiarc					AD-7203S				1-B0			No		No			No			No						No
PLEXTOR					PX-12TS					1.01			Yes		Yes			Yes			No						No
PLEXTOR					PX-20TS					1.00			Yes		Yes			Yes			No						No
PLEXTOR					PX-40TS					1.14			Yes		Yes			Yes			No						No
PLEXTOR					PX-R412C				1.07			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-R820Ti				1.08			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-W8220Ti				1.05			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-W8432Ti				1.09			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-W124TS				1.07			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-W1210TA				1.10			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-W1210TS				1.06			Yes		Yes			Yes			No						No
PLEXTOR					PX-W1610TA				1.05			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-S88T					1.06			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-W2410TA				1.04			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-W4012TA				1.07			Yes		Yes			Yes			No						No
PLEXTOR					PX-W4012TS				1.06			Yes		Yes			Yes			No						No
PLEXTOR					PX-W4824TA				1.07			Yes		Yes			Yes			No						No
PLEXTOR					PX-W5224A				1.04			Yes		Yes			Yes			No						No
PLEXTOR					PREMIUM					1.07			Yes		Yes			Yes			No						No
PLEXTOR					PX-320A					1.06			Yes		Yes			Yes(*3)		No						No
PLEXTOR					PX-504A					1.02			No		No			No			No						No
PLEXTOR					PX-708A(PX-708UF)		1.12			Yes		Yes			Yes			No						No
PLEXTOR					PX-712SA(PX-712A)		1.09			Yes		Yes			Yes			Partial(about 79:59:74)	No
PLEXTOR					PX-716A					1.11			Yes		Yes			Yes			No						No
PLEXTOR					PX-750A					1.03			No		No			No			No						No
PLEXTOR					PX-755SA(PX-755A)		1.08			Yes		Yes			Yes			Partial(about 79:59:74)	No
PLEXTOR					PX-760A					1.07			Yes		Yes			Yes			No						No
Slimtype				DS8A3S					HAT9			No		No			Yes			No						No
SONY					CRX200E					1.0f			No		No			Yes			No						No
TSSTcorp				SH-W162L				LC03			No		No			Yes			No						No
TSSTcorp				TS-L162C				DE00			No		No			No			No						No
TSSTcorp				TS-H192C				DE01			No		No			No			Yes						No
TSSTcorp				TS-H192C				HI05			No		No			No			Yes						No
TSSTcorp				TS-H192C				IB01(IB00)		No		No			No			No						No
TSSTcorp				TS-H192CN				NE06(NE07)		No		No			No			Yes						No
TSSTcorp				TS-H292B				DE03			No		No			No			No						No
TSSTcorp				TS-H352C				DE06			No		No			No			Yes						No
TSSTcorp				TS-H352C				NE02			No		No			No			Yes						No
HP(TSSTcorp)			TS-H353A				BA08			No		No			No			Yes						No
HP(TSSTcorp)			TS-H353B				bc03(BC05)		No		No			Yes			No						No
TSSTcorp				TS-H492C				IB01			No		No			No			No						No
TSSTcorp				TS-H652C(TS-H652D)		TI06			No		No			No			No						No

*1
 HITACHI-LG: 0xyy (x is OEM code, yy is number)
  D: DELL, M: ??, S: ??
 PLEXTOR: 1.xx (xx is number)
 TSSTcorp: xxyy (xx is OEM code, yy is number)
  BA: ??, BC: ??, DE: DELL, HI: HITACHI, IB: IBM, LC: LaCie, NE: NEC, TI: ??

*2
 If you rip a GD-ROM, you should rip to internal(and/or NTFS) HDD. 
 Otherwise, if you have a supported drive, you can't only rip about 99:59:74.
 The reason is unknown.
 TSSTcorp TS-H192C Reading often fails. (03-02-00, MEDIUM_ERROR - NO SEEK COMPLETE)

*3
 Single data track disc fails. (05-64-00, ILLEGAL_REQUEST. ILLEGAL MODE FOR THIS TRACK)
 Not supported the C2 error report on READ D8 command
