# DiscImageCreator
## Overview
  This command-line program dumps a disc (CD, GD, DVD, HD-DVD, BD, GC/Wii, XBOX, XBOX 360) and disk (Floppy, MO, USB etc).  
  CD and GD, it can dump considering a drive + CD (=combined) offset.  
  What drive offset? Please look [this document](http://dbpoweramp.com/spoons-audio-guide-cd-ripping.htm)
  
  This program works on Windows PC (WinXP or higher) and Linux.
  
  [DICUI](https://github.com/SabreTools/DICUI) can work this program with GUI.

## Bug report
 To: http://forum.redump.org/topic/10483/discimagecreator/  
  or  
 To: https://github.com/saramibreak/DiscImageCreator/issues

 if you report a bug, try to use the latest test version.  
   Windows: http://www.mediafire.com/file/eq80y20l9cwf48f/DiscImageCreator_test.7z  
   Linux: http://www.mediafire.com/file/uw3e03kdkmsmvvq/DiscImageCreator_linux_test.tar.gz 

 Nevertheless a bug exists, please upload all .txt file the app created.

## Requirement
### Package
 Download and install Visual C++ Redistributable Packages. (for Windows PC)  
  https://aka.ms/vs/16/release/VC_redist.x86.exe

### Recommend drive
- CD: (Drives must be able to dump by scrambled mode and read lead-out and read lead-in).
   - PLEXTOR (No OEM Drive)
     - DVD model: PX-760, PX-755, PX-716, PX-714, PX-712, PX-708, PX-704
     - CD model: Premium2, Premium, PX-W5224, PX-4824, PX-4012
       - This app needs the latest firmware except PX-708. (PX-708 can use latest and 1.04)  
         See Doc/Firmware&Tool.md if you want to download above firmware or useful tool
   - ASUS
     - BC-12D2HT (Combined offset minus disc only), BW-16D1HT (ditto)
       - BW-16D1HT Firmware 3.02 supports the combined offset plus disc
   - Hitachi-LG
     - UH12NS30 (Combined offset minus disc only)
- CD: (Swappable drive) (This is the comfirmed drive list. Actually, many drive perhaps supports to swap)
   - Sony Optiarc
     - AD-7200 (Combined offset plus disc only) 
   - TSSTcorp
     - TS-H353A (Combined offset plus disc only), TS-H352C (ditto)
- GD:
   - TSSTcorp
     - TS-H353A, TS-H352C, TS-H192C
     - http://forum.redump.org/post/14552/#p14552 <- This drive might be supported too.
- DVD: All supported drive
- GC/Wii
   - Hitachi-LG
     - GDR-8082N, GDR-8083N, GDR-8084N  
       GDR-8161B, GDR-8162B, GDR-8163B, GDR-8164B  
       GCC-4160N, GCC-4240N, GCC-4243N, GCC-4244N, GCC-4247N  
       (GDR-8085N, GDR-8087N and GCC-4246N haven't tested yet, but probably supports to dump.)  
       (GCC-4241N and GCC-4242N supports to dump but many errors occurred.) 
- XBOX, XBOX 360
   - TSSTcorp
     - TS-H353A, TS-H352C, SH-D162C, SH-D162D, SH-D163A, SH-D163B (needs the firmware hacked by kreon)
- XBOX, XBOX 360: (Swappable drive) (This is the comfirmed drive list. Actually, many drive perhaps supports to swap)
   - Hitachi-LG
     - GSA-4163B
- HD-DVD: All supported drive
- BD: All supported drive (PS3 is only supported by [some mediatek drive](https://rpcs3.net/quickstart) or PS3 drive)
    - You need to get the [3k3y ripper](https://web.archive.org/web/20150212063714/http://www.3k3y.com/) if you want to dump the data1/data2

### Not recommend
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

## How to use
See [wiki](https://github.com/saramibreak/DiscImageCreator/wiki)

## Supported Media
### Optical disc
  CD
  - Apple Macintosh
  - Atari Jaguar CD
  - Audio CD
  - Bandai Playdia
  - Bandai / Apple Pippin
  - Commodore Amiga CD
  - Commodore Amiga CD32
  - Commodore Amiga CDTV
  - Fujitsu FM Towns series
  - Hasbro VideoNow
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
 
  PD

  DVD
  - DVD-Video
  - IBM PC compatible
  - Sega Lindbergh
  - Sony PlayStation 2
  - VM Labs NUON DVD

  Nintendo Optical Disc
  - GameCube
  - Wii
  
  XBOX, XBOX 360

  HD-DVD

  BD
  - Microsoft Xbox One
  - Sony PlayStation 3
  - Sony PlayStation 4

  Protected Disc
  - Cactus Data Shield 100 [fake TOC]
  - Cactus Data Shield 200 [intentional C2 error]
  - Cactus Data Shield 300 
  - CD Lock [characteristic track]
  - LaserLock [no signal sector]
  - LibCrypt [unique data on subchannel]
  - Key2Audio [pregap]
  - PhenoProtect [read errors?]
  - Proring [no signal sector]
  - ProtectCD-VOB [invalid sync]
  - SafeDisc [bad(error) sector, intentional C2 error]
  - SecuROM(v1 - v3) [unique data on subchannel]
  - SmartE [duplicated msf]

### Magnetic storage media
  - FD(3.5inch), SparQ, SuperDisk(120MB), ZIP(100MB, 250MB)

### Magneto-Optical disk
  - MD(Hi-MD format), MO(128MB, 230MB, 640MB)

### Memory card
  - CompactFlash, Memory Stick, Secure Digital(SD) card, USB flash drive

## Probably Unsupported Disc
  Protected Disc
  - CodeLock [intentional C2 error, invalid sync]  
     => Compared with CloneCD or CD Manipulator, plextor detects double errors.

## Unsupported Disc
  Protected Disc
  - SecuROM(v4.x or higher), StarForce, CD-Cops [recording density]  
     => These needs DPM(Data position measurement). cue, ccd doesn't support DPM.
        You need to use the [Alcohol 120/52%](http://www.alcohol-soft.com/) to store it, 
  - Alpha-ROM, ROOT, TAGES [duplicated(double, triple) sector]  
     => It can read in reverse, but specifications are not decided in redump.org

  Nintendo Wii U
   => This is a BD based disc, but I don't know the details.

## Created files information
- .bin  
  2352 byte/sector binary image of CD. This file is used to a cue file.
- .c2  
  c2 error binary image of CD. 1bit expresses 1byte.
- .ccd  
  CD information. Original is [CloneCD](https://www.redfox.bz/ja/clonecd.html)
- .cue  
  CD information. Original is [CDRWIN](https://web.archive.org/web/20111008191852/http://www.goldenhawk.com/cdrwin.htm)
- .dat  
  crc32/md5/sha1 of bin file. Original is [Clrmamepro](http://mamedev.emulab.it/clrmamepro/)
- .img  
  2352 byte/sector binary image of CD. This file is used to a ccd file.
- .iso  
  2048 byte/sector binary image of DVD/BD/GC/Wii/XBOX.
- .scm  
  scrambled image file of img file.
- .raw  
  scrambled image file of iso file.
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
- _mdsReadable.txt  
  text data of the parsed mds file.
- _volDesc.txt  
  text data of the volume descriptor, path table and directory table.

## Development Tool
- Visual Studio 2019 (Visual C++ 2019)
  - Windows build
    - [Windows Driver Kit (WDK) 7.1.0](https://www.microsoft.com/en-us/download/details.aspx?id=11800)  
    - [Visual Leak Detector](https://kinddragon.github.io/vld/) (DEBUG build only)

  - Linux build on Windows  
    - Windows Subsystem for Linux (WSL)  
      https://devblogs.microsoft.com/cppblog/targeting-windows-subsystem-for-linux-from-visual-studio/  
      https://docs.microsoft.com/en-us/windows/wsl/install-win10

- Linux
  - gcc, make, [css_auth](http://www.cs.cmu.edu/~dst/DeCSS/) (if you dump a DVD with DRM)

## License & Copyright
See LICENSE  
- About driveOffset.txt.  
    http://www.accuraterip.com/driveoffsets.htm  
    Copyright (c) 2018 Illustrate. All Rights Reserved.  

- About _external folder  
  prngcd.cpp  
    Copyright (c) 2015 Jonathan Gevaryahu. All rights reserved.  

  aes.cpp, aesni.cpp, platform_util.cpp, mbedtls folder  
    https://github.com/ARMmbed/mbedtls  
    Apache License Version 2.0  
    Copyright (C) 2006-2015, ARM Limited, All Rights Reserved

  abgx360.cpp  
    http://abgx360.cc/  
    Copyright 2008-2012 by Seacrest <Seacrest[at]abgx360[dot]net>

  rijndael-alg-fst.cpp/h  
    Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>  
    Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>  
    Paulo Barreto <paulo.barreto@terra.com.br>  
    This code is hereby placed in the public domain.

  crc16  
    http://oku.edu.mie-u.ac.jp/~okumura/algo/  src\crc16t.c in algo.lzh  
    Copyright (c) 1991 Haruhiko Okumura

  crc32  
    https://www.rfc-editor.org/info/rfc1952  
    Copyright (c) 1996 L. Peter Deutsch

  md5  
    https://www.rfc-editor.org/info/rfc1321  
    Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights reserved.

  sha1  
    https://www.rfc-editor.org/info/rfc3174  
    Copyright (C) The Internet Society (2001).  All Rights Reserved.

  tinyxml2  
    https://github.com/leethomason/tinyxml2  
    zlib license  
    Original code by Lee Thomason (www.grinninglizard.com)

- About Release_ANSI folder  
  E_WISE_W.EXE  
    http://kannegieser.net/veit/  
    Copyright (c) 2002. 7. 1 Veit Kannegieser
    
  i6comp.exe  
    Copyright (c) Jun-16-2000 fOSSiL,Dec-27-2000 Morlac
  
    
## Disclaimer
 Use this tool at own your risk.
 Trouble in regard to the use of this tool, I can not guarantee any.

## Gratitude
 Thank's redump.org users.
