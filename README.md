# DiscImageCreator
## Summary
  This command-line program dumps a CD, GD, DVD, HD-DVD, BD, GC/Wii, XBOX, XBOX 360 and Floppy.  
  CD and GD, it can dump considering a drive + CD (=combined) offset.  
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
   - Hitachi-LG
     - UH12NS30 (Combined offset minus disc only)
   - ASUS
     - BC-12D2HT (Combined offset minus disc only), BW-16D1HT (ditto)
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
     - GDR-8082N, GDR-8161B, GDR-8162B, GDR-8163B, GDR-8164B  
       GCC-4160N, GCC-4240N, GCC-4243N, GCC-4247N  
       (4241N, 4244N, 4246N hasn't tested yet, but probably supports to dump.)  
       (4242N supports to dump but many errors occurred in my drive.) 
- XBOX, XBOX 360
   - TSSTcorp
     - TS-H353A, TS-H352C, SH-D162C, SH-D162D, SH-D163A, SH-D163B (needs the firmware hacked by kreon)
- HD-DVD: All supported drive
- BD: All supported drive (PS3 is only supported by [some mediatek drive](https://rpcs3.net/quickstart))
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

  Nintendo Optical Disc
  - GameCube
  - Wii
  
  XBOX, XBOX 360 (except XGD3)

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

  BD
  - Microsoft Xbox One
  - Sony PlayStation 3
  - Sony PlayStation 4

## Probably Unsupported Disc
  Protected Disc
  - RingPROTECH, ProRing [no signal sector]  
     => The result doesn't match at each drives.
  - LaserLock [no signal sector, intentional C2 error]  
     => The result doesn't match at each drives. (0xd8 ripping is more errors than 0xbe ripping.)
  - CodeLock [intentional C2 error, invalid sync]  
     => Compared with CloneCD or CD Manipulator, plextor detects double errors.

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
   => This is a BD based disc, but I don't know the details.

## How to use
### Preparation
 Download and install Visual C++ Redistributable Packages.
  https://go.microsoft.com/fwlink/?LinkId=746571

### Dumping Guide for CD
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)]

 For secure dumping, I recommend using /c2 option  
  DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /c2
 
 About other options, run exe without args to get detail info.

### Dumping Guide for CD with swap
#### Preparation
 create the audio trap disc in advance.  
 (a disc with a hacked TOC of 99 mins audio, burn it with CloneCD or Alcohol 52/120%).  
 http://www.mediafire.com/?2nygv2oyzzz

1. Insert the audio trap disc to a supported drive.
2. Run below. (stop spinning disc)  
   DiscImageCreator.exe stop [DriveLetter]
3. Use a pin to press the escape eject button, so the tray will eject (or remove the drive cover).
4. Insert the disc and run below.  
   DiscImageCreator.exe close [DriveLetter]  
   (or gently push the tray back or put the drive cover back on).
5. Run below. (start dumping scrambled img)  
   DiscImageCreator.exe swap [DriveLetter] foo.bin [DriveSpeed(0-72)]
6. When dumping finished, the drive tray open automatically.
7. The drive tray close automatically after 3000 msec.
8. Read TOC and img is descrambled automatically.

### Dumping Guide for SafeDisc
#### Method 1 (Other tools)
 http://forum.redump.org/topic/2201/dumping-safedisc-cds/

#### Method 2
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /sf

### Dumping Guide for SecuROM(v1 - v5.xx)
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /ns

### Dumping Guide for LibCrypt
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /nl

### Dumping Guide for other protected disc
 DiscImageCreator.exe cd [DriveLetter] foo.bin [DriveSpeed(0-72)] /sf

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
1. Insert the audio trap disc to a supported drive.
2. Run below. (stop spinning disc)
   DiscImageCreator.exe stop [DriveLetter]
3. Use a pin to press the escape eject button, so the tray will eject (or remove
   the drive cover).
4. Insert the gdrom and run below. (close the drive tray)  
   DiscImageCreator.exe close [DriveLetter]  
   (or gently push the tray back or put the drive cover back on).
5. Run below. (start dumping the GD-ROM)  
   DiscImageCreator.exe gd [DriveLetter] foo.bin [DriveSpeed(0-72)]

### Dumping Guide for DVD
 DiscImageCreator.exe dvd [DriveLetter] foo.iso [DriveSpeed(0-16)]

### Dumping Guide for GC/Wii
#### Attention
Compared with Friidump and Rawdump, dumping speed is very slow.

#### Preparation
 To unscramble GC/Wii raw image, put unscramler.exe in DiscImageCreator directory.  
 https://github.com/saramibreak/unscrambler/releases

 DiscImageCreator.exe dvd [DriveLetter] foo.raw [DriveSpeed(0-16)] /raw

### Dumping Guide for XBOX/XBOX 360 on kreon drive
 DiscImageCreator.exe xbox [DriveLetter] foo.iso

### Dumping Guide for XBOX/XBOX360 (XGD2)/XBOX360 (XGD3) on genaral drive
 Prepare DVD-DL or create the DVD+R DL trap disc in advance.  

#### How to prepare
 You need a pressed DVD-DL disc with around 8.5GB data on it
 
 XBOX: Layerbreak larger than 1913776 and the exact size needed (in sectors) is (your_disc_layerbreak - 1913776) * 2 + 3820880

 XBOX360 (XGD2): Layerbreak larger than 1913760 and the exact size needed (in sectors) is (your_disc_layerbreak - 1913760) * 2 + 3825924

 XBOX360 (XGD3): Layerbreak larger than 2133520 and the exact size needed (in sectors) is (your_disc_layerbreak - 2133520) * 2 + 4267015

#### How to create
 XBOX: DVD (Length is 3820880 or larger, Layerbreak is 1913776 or larger)

 XBOX360 (XGD2): DVD (Length is 3825924 or larger, Layerbreak is 1913760 or larger)

 XBOX360 (XGD3): DVD (Length is 4267015 or larger, Layerbreak is 2133520 or larger)

1. Create image file of the above DVD using dumping tool (e.g. isobuster)
2. Run [ImgBurn](http://www.imgburn.com/)
3. Setting LayerBreak manually (L0 sector num is about a half size of DVD length)  
   e.g. If DVD length is 3900304, L0 sector num is about 1950160
4. Before burn the image file, you need to confirm the size is correct  
   e.g. (1950160 - 1913776) * 2 + 3820880 = 3893648 => 3893648 is smaller than 3900304, so the DVD+R DL this image is burnt can dump a XBOX disc
5. Burn image file to DVD+R DL

#### Dump the disc
1. Insert the DVD-DL or DVD+R DL trap disc to a general DVD drive.
2. Run below. (stop spinning disc)  
   DiscImageCreator.exe stop [DriveLetter]
3. Use a pin to press the escape eject button, so the tray will eject (or remove
   the drive cover).
4. Insert the XBOX disc and run below. (close the drive tray)  
   DiscImageCreator.exe close [DriveLetter]  
   (or gently push the tray back or put the drive cover back on).
5. Run below.  
   e.g. Dead or Alive 3 http://redump.org/disc/27157/  
   DiscImageCreator.exe xboxswap [DriveLetter] foo.iso 292756 467910 686060 830074 999794 1212958 1579164 1719138 2010470 2372562 2527492 2682830 2915560 3065604 3219138 3455656  

   e.g. Blue Dragon http://redump.org/disc/27088/  
   DiscImageCreator.exe xgd2swap [DriveLetter] foo.iso 3825631 108976 3719856  

   e.g. Battlefield 3 (Disc 1) (Multiplayer/Co-Op) http://redump.org/disc/35131/  
   DiscImageCreator.exe xgd3swap [DriveLetter] foo.iso 4267015 12544 4246304  

### Dumping Guide for BD
 DiscImageCreator.exe bd [DriveLetter] foo.iso

### Dumping Guide for Floppy Disk
 DiscImageCreator.exe fd [DriveLetter] foo.bin


## All commands and options
        cd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/a (val)]
           [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4)] [/f (val)] [/m]
           [/p] [/ms] [/sf (val)] [/ss] [/np] [/nq] [/nr] [/ns] [/s (val)]
                Dump a CD from A to Z
                For PLEXTOR or drive that can scramble Dumping
        swap <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/a (val)]
           [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4)] [/f (val)] [/m]
           [/p] [/ms] [/sf (val)] [/ss] [/np] [/nq] [/nr] [/ns] [/s (val)]
                Dump a CD from A to Z using swap trick
                For no PLEXTOR or drive that can't scramble Dumping
        data <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>
             [/q] [/be (str) or /d8] [/c2 (val1) (val2) (val3) (val4)]
             [/sf (val)] [/ss] [/r] [/np] [/nq] [/nr] [/ns] [/s (val)]
                Dump a CD from start to end (using 'all' flag)
                For no PLEXTOR or drive that can't scramble dumping
        audio <DriveLetter> <Filename> <DriveSpeed(0-72)> <StartLBA> <EndLBA+1>
              [/q] [/a (val)] [/c2 (val1) (val2) (val3) (val4)]
              [/be (str) or /d8] [/sf (val)] [/np] [/nq] [/nr] [/ns] [/s (val)]
                Dump a CD from start to end (using 'cdda' flag)
                For dumping a lead-in, lead-out mainly
        gd <DriveLetter> <Filename> <DriveSpeed(0-72)> [/q] [/be (str) or /d8]
           [/c2 (val1) (val2) (val3) (val4)] [/np] [/nq] [/nr] [/ns] [/s (val)]
                Dump a HD area of GD from A to Z
        dvd <DriveLetter> <Filename> <DriveSpeed(0-16)> [/c] [/f (val)] [/raw] [/q]
                Dump a DVD from A to Z
        xbox <DriveLetter> <Filename> [/f (val)] [/q]
                Dump a disc from A to Z
        xboxswap <DriveLetter> <Filename> <StartLBAOfSecuritySector_1>
                                          <StartLBAOfSecuritySector_2>
                                                         :
                                          <StartLBAOfSecuritySector_16> [/f (val)] [/q]
                Dump a Xbox disc from A to Z using swap trick
        xgd2swap <DriveLetter> <Filename> <AllSectorLength>
                  <StartLBAOfSecuritySector_1> <StartLBAOfSecuritySector_2> [/f (val)] [/q]
                Dump a XGD2 disc from A to Z using swap trick
        xgd3swap <DriveLetter> <Filename> <AllSectorLength>
                  <StartLBAOfSecuritySector_1> <StartLBAOfSecuritySector_2> [/f (val)] [/q]
                Dump a XGD3 disc from A to Z using swap trick
        bd <DriveLetter> <Filename> [/f (val)] [/q]
                Dump a BD from A to Z
        fd <DriveLetter> <Filename>
                Dump a floppy disk
        stop <DriveLetter>
                Spin off the disc
        start <DriveLetter>
                Spin up the disc
        eject <DriveLetter>
                Eject the tray
        close <DriveLetter>
                Close the tray
        reset <DriveLetter>
                Reset the drive (Only PLEXTOR)
        ls <DriveLetter>
                Show maximum speed of the drive
        sub <Subfile>
                Parse CloneCD sub file and output to readable format
        mds <Mdsfile>
                Parse Alchohol 120/52 mds file and output to readable format
    Option (generic)
        /f      Use 'Force Unit Access' flag to delete the drive cache
                        val     delete per specified value (default: 1)
        /q      Disable beep
    Option (for CD read mode)
        /a      Add CD offset manually (Only Audio CD)
                        val     samples value
        /be     Use 0xbe as the opcode for Reading CD forcibly
                        str      raw: sub channel mode is raw (default)
                                pack: sub channel mode is pack
        /d8     Use 0xd8 as the opcode for Reading CD forcibly
        /c2     Continue reading CD to recover C2 error existing sector
                        val1    value to reread (default: 4000)
                        val2    0: reread sector c2 error is reported (default)
                                1: reread all (or from first to last) sector
                        val3    first LBA to reread (default: 0)
                        val4    last LBA to reread (default: end-of-sector)
                                val3, 4 is used when val2 is 1
        /m      Use if MCN exists in the first pregap sector of the track
                        For some PC-Engine
        /p      Dumping the AMSF from 00:00:00 to 00:01:74
                        For SagaFrontier Original Sound Track (Disc 3) etc.
                        Support drive: PLEXTOR PX-W5224, PREMIUM, PREMIUM2
                                       PX-704, 708, 712, 714, 716, 755, 760
        /r      Read CD from the reverse
                        For Alpha-Disc, Tages (very slow)
        /ms     Read the lead-out of 1st session and the lead-in of 2nd session
                        For Multi-session
        /74     Read the lead-out about 74:00:00
                        For ring data (a.k.a Saturn Ring) of Sega Saturn
        /sf     Scan file to detect protect. If reading error exists,
                continue reading and ignore c2 error on specific sector
                        For CodeLock, LaserLock, RingProtect, RingPROTECH
                            SafeDisc, SmartE, CD.IDX, ProtectCD-VOB, CDS300
                        val     timeout value (default: 60)
        /ss     Scan sector to detect protect. If reading error exists,
                continue reading and ignore c2 error on specific sector
                        For ProtectCD-VOB
        /am     Scan anti-mod string
                        For PlayStation
    Option (for CD SubChannel)
        /np     Not fix SubP
        /nq     Not fix SubQ
        /nr     Not fix SubRtoW
        /nl     Not fix SubQ (RMSF, AMSF, CRC) (LBA 10000 - 19999)
                                               (LBA 40000 - 49999)
                        For PlayStation LibCrypt
        /ns     Not fix SubQ (RMSF, AMSF, CRC) (LBA 0 - 7, 5000 - 24999)
                                            or (LBA 30000 - 49999)
                        For SecuROM
        /s      Use if it reads subchannel precisely
                        val     0: no read next sub (fast, but lack precision)
                                1: read next sub (normal, this val is default)
                                2: read next & next next sub (slow, precision)
    Option (for DVD)
        /c      Log Copyright Management Information
        /raw    Dumping DVD by raw (2064 byte/sector)
                        Comfirmed drive: Mediatec MT chip (Lite-on etc.), PLEXTOR
                                       Hitachi-LG GDR, GCC
                         -> GDR (8082N, 8161B to 8164B) and GCC (4160N, 4240N to 4247N)
                            supports GC/Wii dumping
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
  2048 byte/sector binary image of DVD/BD/GC/Wii.
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
- Visual Studio 2017 (Visual C++ 2017)
  - Windows build
    - Windows Driver Kit (WDK) 7.1
      I use 7.1 to support Windows XP.  
      Sample code path: WinDDK\7600.16385.1\src\storage\tools\spti
      url: http://msdn.microsoft.com/en-us/library/windows/hardware/ff561595(v=vs.85).aspx

  - Linux build on Windows
    - Windows Subsystem for Linux (WSL)
      https://blogs.msdn.microsoft.com/vcblog/2017/02/08/targeting-windows-subsystem-for-linux-from-visual-studio/

  - Linux build on Linux
    - gcc, make 

## License & Copyright
See LICENSE  
- About driveOffset.txt.  
  http://www.accuraterip.com/driveoffsets.htm  
  Copyright (c) 2018 Illustrate. All Rights Reserved.  

- About _external folder  
  prngcd.cpp: Copyright (c) 2015 Jonathan Gevaryahu. All rights reserved.  

  crc16: http://oku.edu.mie-u.ac.jp/~okumura/algo/  src\crc16t.c in algo.lzh  
         Copyright (c) 1991 Haruhiko Okumura  

  crc32: https://www.rfc-editor.org/info/rfc1952  
         Copyright (c) 1996 L. Peter Deutsch  

  md5: https://www.rfc-editor.org/info/rfc1321  
       Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All rights reserved.  

  sha1: https://www.rfc-editor.org/info/rfc3174  
        Copyright (C) The Internet Society (2001).  All Rights Reserved.

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
