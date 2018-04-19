# DiscImageCreator
## Summary
  This command-line program dumps a CD, DVD, BD, GD, Floppy. CD and GD, it can
 dump considering a drive + CD (=combined) offset.
  It works on Windows PC (WinXP or higher).

## Recommend drive
- CD: (Drives must be able to dump by scrambled mode and read lead-out and read lead-in).
   - PLEXTOR (No OEM Drive)
     - PX-760, PX-755, PX-716, PX-714, PX-712, PX-708, PX-704, 
       Premium2, Premium, PX-W5224, PX-4824, PX-4012 (PX-714, PX-704 can't test it enough.)
       - [Firmware of a plextor drive](http://www.skcj.co.jp/discon/download/index.html)
         Without a reason, you should update the latest firmware.
       - [PleXTools Professional XL](https://web.archive.org/web/*/http://www.plextor-digital.com/index.php/component/option,com_jdownloads/Itemid,55/catid,86/cid,470/task,view.download//)
          Useful tool for PLEXTOR.
       - [PX-Info Utility](https://web.archive.org/web/*/http://www.plextor-digital.com/index.php/component/option,com_jdownloads/Itemid,0/catid,86/cid,468/task,view.download/)
          Useful tool for PLEXTOR.
   - HL-DT-ST
     - UH12NS30 (Combined offset minus disc only)
   - ASUS
     - BC-12D2HT (Combined offset minus disc only)
- GD: TSSTcorp(TS-H353A, TS-H352C, TS-H192C)
     http://forum.redump.org/post/14552/#p14552 <- This drive might be supported too.
- DVD: All supported drive
- BD: All supported drive
- Floppy: All supported drive

## Not recommend
- CD: (Because it can't read lead-in and/or lead-out and can't exec 0xd8 opcode)
   - PLEXTOR (OEM drive)
      PX-8xx/PX-Bxxx(many maker), PX-751A(BenQ DW1670), PX-750A/UF(TEAC DV-W516E),
      PX-740A/UF(BenQ DW1640), PX-6xx(Pioneer, Panasonic and so on),
      PX-504A/UF(NEC ND-1100A), PX-2xx(Lite-on, BenQ, NEC), PX-1xx(Pioneer and so on),
      PX-S2410TU(TEAC CD-W224E), PX-54TA(Mitsumi FX5400), PX-R24CS(RICOH RO-1420C),
   - Other vendor
- Protected CD:
   - SecuRom 3
     - PLEXTOR
       PX-4824A (ecc/edc of the 2 sector doesn't match)
   - CDS100, CDS200, Label Gate, XCP
     - PLEXTOR
       PX-4824A (doesn't get the TOC correctly)

## Supported Disc
  CD
  - Apple Macintosh
  - Audio CD
  - Bandai Playdia
  - Bandai / Apple Pippin
  - Commodore Amiga CD
  - Commodore Amiga CD32
  - Commodore Amiga CDTV
  - Fujitsu FM Towns series
  - IBM PC compatible
  - Mattel HyperScan
  - NEC PC-88 series CD
  - NEC PC-98 series CD
  - NEC PC-FX
  - NEC PC Engine CD - TurboGrafx-CD
  - Palm OS
  - Panasonic 3DO Interactive Multiplayer
  - Philips CD-i
  - Photo CD
  - Sega Mega-CD
  - Sega Saturn
  - Sharp X68000 CD
  - SNK Neo Geo CD
  - Sony PlayStation
  - Sony PlayStation 2
  - Tandy / Memorex Visual Information System
  - Tao iKTV CD
  - Tomy Kiss-Site CD
  - Video CD
  - VTech V.Flash

  GD
  - Namco / Sega / Nintendo Triforce
  - Sega Dreamcast
  - Sega Chihiro
  - Sega Naomi
 
  DVD
  - DVD-Video
  - IBM PC compatible
  - Sega Lindbergh
  - Sony PlayStation 2
  - VM Labs NUON DVD

  Protected Disc
  - Cactus Data Shield 100 [fake TOC]
  - Cactus Data Shield 200 [intentional C2 error]
  - Cactus Data Shield 300 
  - CD Lock [characteristic track]
  - LibCrypt [unique data on subchannel]
  - Key2Audio [pregap]
  - PhenoProtect [read errors?]
  - ProtectCD-VOB [invalid sync]
  - SafeDisc [bad(error) sector, intentional C2 error]
  - SmartE [duplicated msf]
  - SecuROM(v1 - v3) [unique data on subchannel]

## Probably Unsupported Disc
  DVD
  - Microsoft Xbox
  - Microsoft Xbox 360
     => see this. http://forum.redump.org/topic/6073/xbox-1-360-dumping-instructions/

  Protected Disc
  - RingPROTECH, ProRing [no signal sector]
  - LaserLock [no signal sector, intentional C2 error]
     => The result doesn't match at each drives.
  - CodeLock [intentional C2 error, invalid sync]
     => Compared to CloneCD or CD Manipulator, a plextor detects double errors.

## Unsupported Disc
  CD
  - Atari Jaguar CD
     => Can't get the CD offsets

  Protected Disc
  - SecuROM(v4.x or higher), StarForce, CD-Cops [recording density]
     => These needs DPM(Data position measurement). cue, ccd doesn't support DPM.
        You need to use the [Alcohol 120/52%](http://www.alcohol-soft.com/) to store it, 
  - Alpha-ROM, ROOT, TAGES [duplicated(double, triple) sector]
     => It can read in reverse, but specifications are not decided in redump.org

  Nintendo Wii U
   => I don't know the details.

## How to use
### Preparation
 Download and install Visual C++ Redistributable Packages.
  https://go.microsoft.com/fwlink/?LinkId=746571

### Dumping Guide for CD
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)]

 For secure dumping, I recommend using /c2 option  
  DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /c2
 
 About others option, run exe without args to get detail info.

### Dumping Guide for DVD
 DiscImageCreator.exe dvd [DriveLetter] foo.bin [DriveSpeed(0-16)]

### Dumping Guide for Floppy Disk
 DiscImageCreator.exe fd [DriveLetter] foo.bin

### Dumping Guide for SafeDisc
#### Method 1 (Other tools)
 http://forum.redump.org/topic/2201/dumping-safedisc-cds/

#### Method 2
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /sf

### Dumping Guide for Protected disc
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /sf

### Dumping Guide for SecuROM(v1 - v5.xx)
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /ns

### Dumping Guide for LibCrypt
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /nl

### Dumping Guide for GD-ROM (The high density area)
#### Preparation
 create the audio trap disc in advance.
 (a disc with a hacked TOC of 99 mins audio, burn it with CloneCD or Alcohol 52/120%).
   http://www.mediafire.com/?2nygv2oyzzz

#### Method 1 (Other tools)
 http://forum.redump.org/topic/2620/dreamcastnaomi-gdrom-dumping-instructions/

#### Method 2 (Other tools)
 http://forum.redump.org/topic/9436/new-dreamcast-dumping-program-test-please/

#### Method 3
1. insert the audio trap disc to a supported drive.
2. run below. (stop spinning disc)
   DiscImageCreator.exe stop [DriveLetter]
3. use a pin to press the escape eject button, so the tray will eject (or remove
   the drive cover).
4. insert the gdrom and gently push the tray back (or put the drive cover back on).
5. run below. (start rippping gdrom)
   DiscImageCreator.exe gd [DriveLetter] foo.bin [DriveSpeed(0-72)]

## Created files information
- .bin  
  2352byte/sector binary image of CD. This file is used to a cue file.
- .c2  
  c2 error binary image of CD. 1bit expresses 1byte.
- .ccd  
  CD information. Original is [CloneCD](https://www.redfox.bz/ja/clonecd.html)
- .cue  
  CD information. Original is [CDRWIN](https://web.archive.org/web/20111008191852/http://www.goldenhawk.com/cdrwin.htm)
- .dat  
  crc32/md5/sha1 of bin file. Original is [Clrmamepro](http://mamedev.emulab.it/clrmamepro/)
- .img  
  2352byte/sector binary image of CD. This file is used to a ccd file.
- .iso  
  2048byte/sector binary image of DVD/BD.
- .scm  
  scrambled image file of img file.
- .sub  
  subchannel data of CD. This file is used to a ccd file.
- _c2Error.txt  
  c2 error information which can be gotten by reading CD.
- _cmd.txt  
  command-line argument.
- _disc.txt  
  disc information returned by SCSI command.
- _drive.txt  
  drive information returned by SCSI command.
- _mainError.txt  
  text data of error message which can be gotten by reading CD.
- _mainInfo.txt  
  text data of main sector. Original is [IsoBuster](https://www.isobuster.com/)
- _subError.txt  
  text data of subchannel error.
- _subInfo.txt  
  text data of subchannel when the track number changes.
- _subIntention.txt  
  text data of subchannel for securom.
- _subReadable.txt  
  text data of the parsed sub channel file.
- _volDesc.txt  
  text data of the volume descriptor, path table and directory table.

## Development Tool
- Visual Studio 2017 (Visual C++ 2017)
- Windows Driver Kit(WDK) 7.1 (I use 7.1 to support Windows XP.)  
  Sample code path: WinDDK\7600.16385.1\src\storage\tools\spti
  url: http://msdn.microsoft.com/en-us/library/windows/hardware/ff561595(v=vs.85).aspx

## License & Copyright
 See LICENSE  
 About driveOffset.txt.  
  http://www.accuraterip.com/driveoffsets.htm  
  Copyright (c) 2018 Illustrate. All Rights Reserved.  
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

## Disclaimer
 Use this tool at own your risk.
 Trouble in regard to the use of this tool, I can not guarantee any.

## Bug report
 To: http://forum.redump.org/topic/10483/discimagecreator/
  or
 To: https://github.com/saramibreak/DiscImageCreator/issues

 if you report, please upload all .txt file app created.

## Gratitude
 Thank's redump.org users.
