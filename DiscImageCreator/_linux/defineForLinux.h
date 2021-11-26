#pragma once

#ifdef __linux__
#define _snprintf    snprintf

// from BaseTsd.h
typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed long long    INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned long long  UINT64, *PUINT64, ULONGLONG;

#if defined(__x86_64__)
typedef long long INT_PTR, *PINT_PTR;
typedef unsigned long long UINT_PTR, *PUINT_PTR;

typedef long long LONG_PTR, * PLONG_PTR;
typedef unsigned long long ULONG_PTR, *PULONG_PTR;

#define __int3264   __int64

#else
typedef int INT_PTR, * PINT_PTR;
typedef unsigned int UINT_PTR, * PUINT_PTR;

typedef long LONG_PTR, * PLONG_PTR;
typedef unsigned long ULONG_PTR, * PULONG_PTR;

#define __int3264   __int32

#endif

//
// Add Windows flavor DWORD_PTR types
//

typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

//
// The following types are guaranteed to be signed and 64 bits wide.
//

typedef long long LONG64, *PLONG64;


//
// The following types are guaranteed to be unsigned and 64 bits wide.
//

typedef unsigned long long ULONG64, *PULONG64;
typedef unsigned long long DWORD64, *PDWORD64;

// from WinDef.h
typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
#define FALSE               0
#define TRUE                1
#define far
#define near
#define CONST               const
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;
typedef int near            *PINT;
typedef int far             *LPINT;
typedef WORD near           *PWORD;
typedef WORD far            *LPWORD;
typedef long far            *LPLONG;
typedef DWORD near          *PDWORD;
typedef DWORD far           *LPDWORD;
typedef void far            *LPVOID;
typedef CONST void far      *LPCVOID;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;
typedef UINT			   *LPUINT;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#pragma GCC diagnostic pop
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

// WTypes.h
 typedef double DOUBLE;

// from WinNT.h
//
// For compilers that don't support nameless unions/structs
//
#ifndef DUMMYUNIONNAME
#if defined(NONAMELESSUNION)// || !defined(_MSC_EXTENSIONS)
#define DUMMYUNIONNAME   u
#define DUMMYUNIONNAME2  u2
#define DUMMYUNIONNAME3  u3
#define DUMMYUNIONNAME4  u4
#define DUMMYUNIONNAME5  u5
#define DUMMYUNIONNAME6  u6
#define DUMMYUNIONNAME7  u7
#define DUMMYUNIONNAME8  u8
#define DUMMYUNIONNAME9  u9
#else
#define DUMMYUNIONNAME
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#define DUMMYUNIONNAME4
#define DUMMYUNIONNAME5
#define DUMMYUNIONNAME6
#define DUMMYUNIONNAME7
#define DUMMYUNIONNAME8
#define DUMMYUNIONNAME9
#endif
#endif // DUMMYUNIONNAME

#ifndef DUMMYSTRUCTNAME
#if defined(NONAMELESSUNION)// || !defined(_MSC_EXTENSIONS)
#define DUMMYSTRUCTNAME  s
#define DUMMYSTRUCTNAME2 s2
#define DUMMYSTRUCTNAME3 s3
#define DUMMYSTRUCTNAME4 s4
#define DUMMYSTRUCTNAME5 s5
#else
#define DUMMYSTRUCTNAME
#define DUMMYSTRUCTNAME2
#define DUMMYSTRUCTNAME3
#define DUMMYSTRUCTNAME4
#define DUMMYSTRUCTNAME5
#endif
#endif // DUMMYSTRUCTNAME

#define UNALIGNED
#define UNALIGNED64

#define VOID void
typedef void *PVOID;
typedef char CHAR;
typedef short SHORT;
typedef long LONG;

typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
typedef WCHAR *PWCHAR, *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR, *LPWSTR, *PWSTR;
typedef PWSTR *PZPWSTR;
typedef CONST PWSTR *PCZPWSTR;
typedef CONST WCHAR *LPCWSTR, *PCWSTR;
typedef PCWSTR *PZPCWSTR;

typedef WCHAR *PZZWSTR;
typedef CONST WCHAR *PCZZWSTR;

typedef WCHAR *PNZWCH;
typedef CONST WCHAR *PCNZWCH;
typedef CHAR *PCHAR, *LPCH, *PCH;
typedef CONST CHAR *LPCCH, *PCCH;

typedef CHAR *NPSTR, *LPSTR, *PSTR;
typedef PSTR *PZPSTR;
typedef CONST PSTR *PCZPSTR;
typedef CONST CHAR *LPCSTR, *PCSTR;
typedef PCSTR *PZPCSTR;

typedef CHAR *PZZSTR;
typedef CONST CHAR *PCZZSTR;

typedef CHAR *PNZCH;
typedef CONST CHAR *PCNZCH;

typedef LPCH LPTCH, PTCH;
typedef LPCCH LPCTCH, PCTCH;
typedef LPSTR PTSTR, LPTSTR, PUTSTR, LPUTSTR;
typedef LPCSTR PCTSTR, LPCTSTR, PCUTSTR, LPCUTSTR;
typedef PZZSTR PZZTSTR, PUZZTSTR;
typedef PCZZSTR PCZZTSTR, PCUZZTSTR;
typedef PNZCH PNZTCH, PUNZTCH;
typedef PCNZCH PCNZTCH, PCUNZTCH;
#define __TEXT(quote) quote         // r_winnt

#define TEXT(quote) __TEXT(quote)   // r_winnt

typedef SHORT *PSHORT;
typedef LONG *PLONG;

typedef void *HANDLE;

typedef long long LONGLONG;

#if defined(MIDL_PASS)
typedef struct _LARGE_INTEGER {
#else // MIDL_PASS
typedef union _LARGE_INTEGER {
	struct {
		DWORD LowPart;
		LONG HighPart;
	} DUMMYSTRUCTNAME;
	struct {
		DWORD LowPart;
		LONG HighPart;
	} u;
#endif //MIDL_PASS
	LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER* PLARGE_INTEGER;

#if defined(MIDL_PASS)
typedef struct _ULARGE_INTEGER {
#else // MIDL_PASS
typedef union _ULARGE_INTEGER {
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} DUMMYSTRUCTNAME;
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} u;
#endif //MIDL_PASS
	ULONGLONG QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER* PULARGE_INTEGER;

typedef BYTE  BOOLEAN;
typedef BOOLEAN *PBOOLEAN;

#define UNREFERENCED_PARAMETER(P)          (VOID)(P)

//
// Image Format
//


#define IMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define IMAGE_OS2_SIGNATURE                 0x454E      // NE
#define IMAGE_OS2_SIGNATURE_LE              0x454C      // LE
#define IMAGE_VXD_SIGNATURE                 0x454C      // LE
#define IMAGE_NT_SIGNATURE                  0x00004550  // PE00

typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
	WORD   e_magic;                     // Magic number
	WORD   e_cblp;                      // Bytes on last page of file
	WORD   e_cp;                        // Pages in file
	WORD   e_crlc;                      // Relocations
	WORD   e_cparhdr;                   // Size of header in paragraphs
	WORD   e_minalloc;                  // Minimum extra paragraphs needed
	WORD   e_maxalloc;                  // Maximum extra paragraphs needed
	WORD   e_ss;                        // Initial (relative) SS value
	WORD   e_sp;                        // Initial SP value
	WORD   e_csum;                      // Checksum
	WORD   e_ip;                        // Initial IP value
	WORD   e_cs;                        // Initial (relative) CS value
	WORD   e_lfarlc;                    // File address of relocation table
	WORD   e_ovno;                      // Overlay number
	WORD   e_res[4];                    // Reserved words
	WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
	WORD   e_oeminfo;                   // OEM information; e_oemid specific
	WORD   e_res2[10];                  // Reserved words
	LONG   e_lfanew;                    // File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_OS2_HEADER {      // OS/2 .EXE header
	WORD   ne_magic;                    // Magic number
	CHAR   ne_ver;                      // Version number
	CHAR   ne_rev;                      // Revision number
	WORD   ne_enttab;                   // Offset of Entry Table
	WORD   ne_cbenttab;                 // Number of bytes in Entry Table
	LONG   ne_crc;                      // Checksum of whole file
	WORD   ne_flags;                    // Flag word
	WORD   ne_autodata;                 // Automatic data segment number
	WORD   ne_heap;                     // Initial heap allocation
	WORD   ne_stack;                    // Initial stack allocation
	LONG   ne_csip;                     // Initial CS:IP setting
	LONG   ne_sssp;                     // Initial SS:SP setting
	WORD   ne_cseg;                     // Count of file segments
	WORD   ne_cmod;                     // Entries in Module Reference Table
	WORD   ne_cbnrestab;                // Size of non-resident name table
	WORD   ne_segtab;                   // Offset of Segment Table
	WORD   ne_rsrctab;                  // Offset of Resource Table
	WORD   ne_restab;                   // Offset of resident name table
	WORD   ne_modtab;                   // Offset of Module Reference Table
	WORD   ne_imptab;                   // Offset of Imported Names Table
	LONG   ne_nrestab;                  // Offset of Non-resident Names Table
	WORD   ne_cmovent;                  // Count of movable entries
	WORD   ne_align;                    // Segment alignment shift count
	WORD   ne_cres;                     // Count of resource segments
	BYTE   ne_exetyp;                   // Target Operating system
	BYTE   ne_flagsothers;              // Other .EXE flags
	WORD   ne_pretthunks;               // offset to return thunks
	WORD   ne_psegrefbytes;             // offset to segment ref. bytes
	WORD   ne_swaparea;                 // Minimum code swap area size
	WORD   ne_expver;                   // Expected Windows version number
} IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;

typedef struct _IMAGE_VXD_HEADER {      // Windows VXD header
	WORD   e32_magic;                   // Magic number
	BYTE   e32_border;                  // The byte ordering for the VXD
	BYTE   e32_worder;                  // The word ordering for the VXD
	DWORD  e32_level;                   // The EXE format level for now = 0
	WORD   e32_cpu;                     // The CPU type
	WORD   e32_os;                      // The OS type
	DWORD  e32_ver;                     // Module version
	DWORD  e32_mflags;                  // Module flags
	DWORD  e32_mpages;                  // Module # pages
	DWORD  e32_startobj;                // Object # for instruction pointer
	DWORD  e32_eip;                     // Extended instruction pointer
	DWORD  e32_stackobj;                // Object # for stack pointer
	DWORD  e32_esp;                     // Extended stack pointer
	DWORD  e32_pagesize;                // VXD page size
	DWORD  e32_lastpagesize;            // Last page size in VXD
	DWORD  e32_fixupsize;               // Fixup section size
	DWORD  e32_fixupsum;                // Fixup section checksum
	DWORD  e32_ldrsize;                 // Loader section size
	DWORD  e32_ldrsum;                  // Loader section checksum
	DWORD  e32_objtab;                  // Object table offset
	DWORD  e32_objcnt;                  // Number of objects in module
	DWORD  e32_objmap;                  // Object page map offset
	DWORD  e32_itermap;                 // Object iterated data map offset
	DWORD  e32_rsrctab;                 // Offset of Resource Table
	DWORD  e32_rsrccnt;                 // Number of resource entries
	DWORD  e32_restab;                  // Offset of resident name table
	DWORD  e32_enttab;                  // Offset of Entry Table
	DWORD  e32_dirtab;                  // Offset of Module Directive Table
	DWORD  e32_dircnt;                  // Number of module directives
	DWORD  e32_fpagetab;                // Offset of Fixup Page Table
	DWORD  e32_frectab;                 // Offset of Fixup Record Table
	DWORD  e32_impmod;                  // Offset of Import Module Name Table
	DWORD  e32_impmodcnt;               // Number of entries in Import Module Name Table
	DWORD  e32_impproc;                 // Offset of Import Procedure Name Table
	DWORD  e32_pagesum;                 // Offset of Per-Page Checksum Table
	DWORD  e32_datapage;                // Offset of Enumerated Data Pages
	DWORD  e32_preload;                 // Number of preload pages
	DWORD  e32_nrestab;                 // Offset of Non-resident Names Table
	DWORD  e32_cbnrestab;               // Size of Non-resident Name Table
	DWORD  e32_nressum;                 // Non-resident Name Table Checksum
	DWORD  e32_autodata;                // Object # for automatic data object
	DWORD  e32_debuginfo;               // Offset of the debugging information
	DWORD  e32_debuglen;                // The length of the debugging info. in bytes
	DWORD  e32_instpreload;             // Number of instance pages in preload section of VXD file
	DWORD  e32_instdemand;              // Number of instance pages in demand load section of VXD file
	DWORD  e32_heapsize;                // Size of heap - for 16-bit apps
	BYTE   e32_res3[12];                // Reserved words
	DWORD  e32_winresoff;
	DWORD  e32_winreslen;
	WORD   e32_devid;                   // Device ID for VxD
	WORD   e32_ddkver;                  // DDK version for VxD
} IMAGE_VXD_HEADER, *PIMAGE_VXD_HEADER;

//
// File header format.
//

typedef struct _IMAGE_FILE_HEADER {
	WORD    Machine;
	WORD    NumberOfSections;
	DWORD   TimeDateStamp;
	DWORD   PointerToSymbolTable;
	DWORD   NumberOfSymbols;
	WORD    SizeOfOptionalHeader;
	WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER             20

#define IMAGE_FILE_RELOCS_STRIPPED           0x0001  // Relocation info stripped from file.
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002  // File is executable  (i.e. no unresolved externel references).
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004  // Line nunbers stripped from file.
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008  // Local symbols stripped from file.
#define IMAGE_FILE_AGGRESIVE_WS_TRIM         0x0010  // Agressively trim working set
#define IMAGE_FILE_LARGE_ADDRESS_AWARE       0x0020  // App can handle >2gb addresses
#define IMAGE_FILE_BYTES_REVERSED_LO         0x0080  // Bytes of machine word are reversed.
#define IMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200  // Debugging info stripped from file in .DBG file
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   0x0400  // If Image is on removable media, copy and run from the swap file.
#define IMAGE_FILE_NET_RUN_FROM_SWAP         0x0800  // If Image is on Net, copy and run from the swap file.
#define IMAGE_FILE_SYSTEM                    0x1000  // System File.
#define IMAGE_FILE_DLL                       0x2000  // File is a DLL.
#define IMAGE_FILE_UP_SYSTEM_ONLY            0x4000  // File should only be run on a UP machine
#define IMAGE_FILE_BYTES_REVERSED_HI         0x8000  // Bytes of machine word are reversed.

#define IMAGE_FILE_MACHINE_UNKNOWN           0
#define IMAGE_FILE_MACHINE_I386              0x014c  // Intel 386.
#define IMAGE_FILE_MACHINE_R3000             0x0162  // MIPS little-endian, 0x160 big-endian
#define IMAGE_FILE_MACHINE_R4000             0x0166  // MIPS little-endian
#define IMAGE_FILE_MACHINE_R10000            0x0168  // MIPS little-endian
#define IMAGE_FILE_MACHINE_WCEMIPSV2         0x0169  // MIPS little-endian WCE v2
#define IMAGE_FILE_MACHINE_ALPHA             0x0184  // Alpha_AXP
#define IMAGE_FILE_MACHINE_SH3               0x01a2  // SH3 little-endian
#define IMAGE_FILE_MACHINE_SH3DSP            0x01a3
#define IMAGE_FILE_MACHINE_SH3E              0x01a4  // SH3E little-endian
#define IMAGE_FILE_MACHINE_SH4               0x01a6  // SH4 little-endian
#define IMAGE_FILE_MACHINE_SH5               0x01a8  // SH5
#define IMAGE_FILE_MACHINE_ARM               0x01c0  // ARM Little-Endian
#define IMAGE_FILE_MACHINE_THUMB             0x01c2
#define IMAGE_FILE_MACHINE_AM33              0x01d3
#define IMAGE_FILE_MACHINE_POWERPC           0x01F0  // IBM PowerPC Little-Endian
#define IMAGE_FILE_MACHINE_POWERPCFP         0x01f1
#define IMAGE_FILE_MACHINE_IA64              0x0200  // Intel 64
#define IMAGE_FILE_MACHINE_MIPS16            0x0266  // MIPS
#define IMAGE_FILE_MACHINE_ALPHA64           0x0284  // ALPHA64
#define IMAGE_FILE_MACHINE_MIPSFPU           0x0366  // MIPS
#define IMAGE_FILE_MACHINE_MIPSFPU16         0x0466  // MIPS
#define IMAGE_FILE_MACHINE_AXP64             IMAGE_FILE_MACHINE_ALPHA64
#define IMAGE_FILE_MACHINE_TRICORE           0x0520  // Infineon
#define IMAGE_FILE_MACHINE_CEF               0x0CEF
#define IMAGE_FILE_MACHINE_EBC               0x0EBC  // EFI Byte Code
#define IMAGE_FILE_MACHINE_AMD64             0x8664  // AMD64 (K8)
#define IMAGE_FILE_MACHINE_M32R              0x9041  // M32R little-endian
#define IMAGE_FILE_MACHINE_CEE               0xC0EE

//
// Directory format.
//

typedef struct _IMAGE_DATA_DIRECTORY {
	DWORD   VirtualAddress;
	DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

//
// Optional header format.
//

typedef struct _IMAGE_OPTIONAL_HEADER {
	//
	// Standard fields.
	//

	WORD    Magic;
	BYTE    MajorLinkerVersion;
	BYTE    MinorLinkerVersion;
	DWORD   SizeOfCode;
	DWORD   SizeOfInitializedData;
	DWORD   SizeOfUninitializedData;
	DWORD   AddressOfEntryPoint;
	DWORD   BaseOfCode;
	DWORD   BaseOfData;

	//
	// NT additional fields.
	//

	DWORD   ImageBase;
	DWORD   SectionAlignment;
	DWORD   FileAlignment;
	WORD    MajorOperatingSystemVersion;
	WORD    MinorOperatingSystemVersion;
	WORD    MajorImageVersion;
	WORD    MinorImageVersion;
	WORD    MajorSubsystemVersion;
	WORD    MinorSubsystemVersion;
	DWORD   Win32VersionValue;
	DWORD   SizeOfImage;
	DWORD   SizeOfHeaders;
	DWORD   CheckSum;
	WORD    Subsystem;
	WORD    DllCharacteristics;
	DWORD   SizeOfStackReserve;
	DWORD   SizeOfStackCommit;
	DWORD   SizeOfHeapReserve;
	DWORD   SizeOfHeapCommit;
	DWORD   LoaderFlags;
	DWORD   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_NT_HEADERS {
	DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

// Subsystem Values

#define IMAGE_SUBSYSTEM_UNKNOWN              0   // Unknown subsystem.
#define IMAGE_SUBSYSTEM_NATIVE               1   // Image doesn't require a subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_GUI          2   // Image runs in the Windows GUI subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_CUI          3   // Image runs in the Windows character subsystem.
#define IMAGE_SUBSYSTEM_OS2_CUI              5   // image runs in the OS/2 character subsystem.
#define IMAGE_SUBSYSTEM_POSIX_CUI            7   // image runs in the Posix character subsystem.
#define IMAGE_SUBSYSTEM_NATIVE_WINDOWS       8   // image is a native Win9x driver.
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI       9   // Image runs in the Windows CE subsystem.
#define IMAGE_SUBSYSTEM_EFI_APPLICATION      10  //
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  11   //
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER   12  //
#define IMAGE_SUBSYSTEM_EFI_ROM              13
#define IMAGE_SUBSYSTEM_XBOX                 14
#define IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION 16

// DllCharacteristics Entries

//      IMAGE_LIBRARY_PROCESS_INIT            0x0001     // Reserved.
//      IMAGE_LIBRARY_PROCESS_TERM            0x0002     // Reserved.
//      IMAGE_LIBRARY_THREAD_INIT             0x0004     // Reserved.
//      IMAGE_LIBRARY_THREAD_TERM             0x0008     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040     // DLL can move.
#define IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY    0x0080     // Code Integrity Image
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT    0x0100     // Image is NX compatible
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200     // Image understands isolation and doesn't want it
#define IMAGE_DLLCHARACTERISTICS_NO_SEH       0x0400     // Image does not use SEH.  No SE handler may reside in this image
#define IMAGE_DLLCHARACTERISTICS_NO_BIND      0x0800     // Do not bind this image.
//                                            0x1000     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER   0x2000     // Driver uses WDM model
//                                            0x4000     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE     0x8000

// Directory Entries

#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

//
// Section header format.
//

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
	BYTE    Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		DWORD   PhysicalAddress;
		DWORD   VirtualSize;
	} Misc;
	DWORD   VirtualAddress;
	DWORD   SizeOfRawData;
	DWORD   PointerToRawData;
	DWORD   PointerToRelocations;
	DWORD   PointerToLinenumbers;
	WORD    NumberOfRelocations;
	WORD    NumberOfLinenumbers;
	DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER          40

//
// Section characteristics.
//
//      IMAGE_SCN_TYPE_REG                   0x00000000  // Reserved.
//      IMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
//      IMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
//      IMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
#define IMAGE_SCN_TYPE_NO_PAD                0x00000008  // Reserved.
//      IMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.

#define IMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  // Section contains initialized data.
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  // Section contains uninitialized data.

#define IMAGE_SCN_LNK_OTHER                  0x00000100  // Reserved.
#define IMAGE_SCN_LNK_INFO                   0x00000200  // Section contains comments or some other type of information.
//      IMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
#define IMAGE_SCN_LNK_REMOVE                 0x00000800  // Section contents will not become part of image.
#define IMAGE_SCN_LNK_COMDAT                 0x00001000  // Section contents comdat.
//                                           0x00002000  // Reserved.
//      IMAGE_SCN_MEM_PROTECTED - Obsolete   0x00004000
#define IMAGE_SCN_NO_DEFER_SPEC_EXC          0x00004000  // Reset speculative exceptions handling bits in the TLB entries for this section.
#define IMAGE_SCN_GPREL                      0x00008000  // Section content can be accessed relative to GP
#define IMAGE_SCN_MEM_FARDATA                0x00008000
//      IMAGE_SCN_MEM_SYSHEAP  - Obsolete    0x00010000
#define IMAGE_SCN_MEM_PURGEABLE              0x00020000
#define IMAGE_SCN_MEM_16BIT                  0x00020000
#define IMAGE_SCN_MEM_LOCKED                 0x00040000
#define IMAGE_SCN_MEM_PRELOAD                0x00080000

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000  //
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000  //
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000  //
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000  //
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000  // Default alignment if no others are specified.
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000  //
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000  //
#define IMAGE_SCN_ALIGN_128BYTES             0x00800000  //
#define IMAGE_SCN_ALIGN_256BYTES             0x00900000  //
#define IMAGE_SCN_ALIGN_512BYTES             0x00A00000  //
#define IMAGE_SCN_ALIGN_1024BYTES            0x00B00000  //
#define IMAGE_SCN_ALIGN_2048BYTES            0x00C00000  //
#define IMAGE_SCN_ALIGN_4096BYTES            0x00D00000  //
#define IMAGE_SCN_ALIGN_8192BYTES            0x00E00000  //
// Unused                                    0x00F00000
#define IMAGE_SCN_ALIGN_MASK                 0x00F00000

#define IMAGE_SCN_LNK_NRELOC_OVFL            0x01000000  // Section contains extended relocations.
#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000  // Section can be discarded.
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000  // Section is not cachable.
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000  // Section is not pageable.
#define IMAGE_SCN_MEM_SHARED                 0x10000000  // Section is shareable.
#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.

//
// TLS Chaacteristic Flags
//
#define IMAGE_SCN_SCALE_INDEX                0x00000001  // Tls index is scaled

//
// DLL support.
//

//
// Export Format
//

typedef struct _IMAGE_EXPORT_DIRECTORY {
	DWORD   Characteristics;
	DWORD   TimeDateStamp;
	WORD    MajorVersion;
	WORD    MinorVersion;
	DWORD   Name;
	DWORD   Base;
	DWORD   NumberOfFunctions;
	DWORD   NumberOfNames;
	DWORD   AddressOfFunctions;     // RVA from base of image
	DWORD   AddressOfNames;         // RVA from base of image
	DWORD   AddressOfNameOrdinals;  // RVA from base of image
} IMAGE_EXPORT_DIRECTORY, * PIMAGE_EXPORT_DIRECTORY;

//
// Import Format
//

typedef struct _IMAGE_IMPORT_BY_NAME {
	WORD    Hint;
	BYTE    Name[1];
} IMAGE_IMPORT_BY_NAME, * PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA64 {
	union {
		ULONGLONG ForwarderString;  // PBYTE 
		ULONGLONG Function;         // PDWORD
		ULONGLONG Ordinal;
		ULONGLONG AddressOfData;    // PIMAGE_IMPORT_BY_NAME
	} u1;
} IMAGE_THUNK_DATA64;
typedef IMAGE_THUNK_DATA64* PIMAGE_THUNK_DATA64;

typedef struct _IMAGE_THUNK_DATA32 {
	union {
		DWORD ForwarderString;      // PBYTE 
		DWORD Function;             // PDWORD
		DWORD Ordinal;
		DWORD AddressOfData;        // PIMAGE_IMPORT_BY_NAME
	} u1;
} IMAGE_THUNK_DATA32;
typedef IMAGE_THUNK_DATA32* PIMAGE_THUNK_DATA32;

#define IMAGE_ORDINAL_FLAG64 0x8000000000000000
#define IMAGE_ORDINAL_FLAG32 0x80000000
#define IMAGE_ORDINAL64(Ordinal) (Ordinal & 0xffff)
#define IMAGE_ORDINAL32(Ordinal) (Ordinal & 0xffff)
#define IMAGE_SNAP_BY_ORDINAL64(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG64) != 0)
#define IMAGE_SNAP_BY_ORDINAL32(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG32) != 0)


typedef struct _IMAGE_IMPORT_DESCRIPTOR {
	union {
		DWORD   Characteristics;            // 0 for terminating null import descriptor
		DWORD   OriginalFirstThunk;         // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
	} DUMMYUNIONNAME;
	DWORD   TimeDateStamp;                  // 0 if not bound,
											// -1 if bound, and real date\time stamp
											//     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
											// O.W. date/time stamp of DLL bound to (Old BIND)

	DWORD   ForwarderChain;                 // -1 if no forwarders
	DWORD   Name;
	DWORD   FirstThunk;                     // RVA to IAT (if bound this IAT has actual addresses)
} IMAGE_IMPORT_DESCRIPTOR;
typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED *PIMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_RESOURCE_DIRECTORY {
	DWORD   Characteristics;
	DWORD   TimeDateStamp;
	WORD    MajorVersion;
	WORD    MinorVersion;
	WORD    NumberOfNamedEntries;
	WORD    NumberOfIdEntries;
	//  IMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
	union {
		struct {
			DWORD NameOffset : 31;
			DWORD NameIsString : 1;
		} DUMMYSTRUCTNAME;
		DWORD   Name;
		WORD    Id;
	} DUMMYUNIONNAME;
	union {
		DWORD   OffsetToData;
		struct {
			DWORD   OffsetToDirectory : 31;
			DWORD   DataIsDirectory : 1;
		} DUMMYSTRUCTNAME2;
	} DUMMYUNIONNAME2;
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;

typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
	DWORD   OffsetToData;
	DWORD   Size;
	DWORD   CodePage;
	DWORD   Reserved;
} IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;

#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

// from VerRsrc.h
typedef struct tagVS_FIXEDFILEINFO
{
	DWORD   dwSignature;            /* e.g. 0xfeef04bd */
	DWORD   dwStrucVersion;         /* e.g. 0x00000042 = "0.42" */
	DWORD   dwFileVersionMS;        /* e.g. 0x00030075 = "3.75" */
	DWORD   dwFileVersionLS;        /* e.g. 0x00000031 = "0.31" */
	DWORD   dwProductVersionMS;     /* e.g. 0x00030010 = "3.10" */
	DWORD   dwProductVersionLS;     /* e.g. 0x00000031 = "0.31" */
	DWORD   dwFileFlagsMask;        /* = 0x3F for version "0.42" */
	DWORD   dwFileFlags;            /* e.g. VFF_DEBUG | VFF_PRERELEASE */
	DWORD   dwFileOS;               /* e.g. VOS_DOS_WINDOWS16 */
	DWORD   dwFileType;             /* e.g. VFT_DRIVER */
	DWORD   dwFileSubtype;          /* e.g. VFT2_DRV_KEYBOARD */
	DWORD   dwFileDateMS;           /* e.g. 0 */
	DWORD   dwFileDateLS;           /* e.g. 0 */
} VS_FIXEDFILEINFO;

// from WinBase.h
#define INVALID_HANDLE_VALUE	-1

#define FillMemory RtlFillMemory
#define ZeroMemory RtlZeroMemory

// from tchar.h
#define __T(x)      x
#define _T(x)       __T(x)

/* Formatted i/o */

#define _tprintf        printf
#define _tprintf_l      _printf_l
#define _tprintf_s      printf_s
#define _tprintf_s_l    _printf_s_l
#define _tprintf_p      _printf_p
#define _tprintf_p_l    _printf_p_l
#define _tcprintf       _cprintf
#define _tcprintf_l     _cprintf_l
#define _tcprintf_s     _cprintf_s
#define _tcprintf_s_l   _cprintf_s_l
#define _tcprintf_p     _cprintf_p
#define _tcprintf_p_l   _cprintf_p_l
#define _vtcprintf      _vcprintf
#define _vtcprintf_l    _vcprintf_l
#define _vtcprintf_s    _vcprintf_s
#define _vtcprintf_s_l  _vcprintf_s_l
#define _vtcprintf_p    _vcprintf_p
#define _vtcprintf_p_l  _vcprintf_p_l
#define _ftprintf       fprintf
#define _ftprintf_l     _fprintf_l
#define _ftprintf_s     fprintf_s
#define _ftprintf_s_l   _fprintf_s_l
#define _ftprintf_p     _fprintf_p
#define _ftprintf_p_l   _fprintf_p_l
#define _stprintf       sprintf
#define _stprintf_l     _sprintf_l
#define _stprintf_s     sprintf_s
#define _stprintf_s_l   _sprintf_s_l
#define _stprintf_p     _sprintf_p
#define _stprintf_p_l   _sprintf_p_l
#define _sctprintf      _scprintf
#define _sctprintf_l    _scprintf_l
#define _sctprintf_p    _scprintf_p
#define _sctprintf_p_l  _scprintf_p_l
#define _sntprintf      _snprintf
#define _sntprintf_l    _snprintf_l
#define _sntprintf_s    _snprintf_s
#define _sntprintf_s_l  _snprintf_s_l
#define _vtprintf       vprintf
#define _vtprintf_l     _vprintf_l
#define _vtprintf_s     vprintf_s
#define _vtprintf_s_l   _vprintf_s_l
#define _vtprintf_p     _vprintf_p
#define _vtprintf_p_l   _vprintf_p_l
#define _vftprintf      vfprintf
#define _vftprintf_l    _vfprintf_l
#define _vftprintf_s    vfprintf_s
#define _vftprintf_s_l  _vfprintf_s_l
#define _vftprintf_p    _vfprintf_p
#define _vftprintf_p_l  _vfprintf_p_l
#define _vstprintf      vsprintf
#define _vstprintf_l    _vsprintf_l
#define _vstprintf_s    vsprintf_s
#define _vstprintf_s_l  _vsprintf_s_l
#define _vstprintf_p    _vsprintf_p
#define _vstprintf_p_l  _vsprintf_p_l
#define _vsctprintf     _vscprintf
#define _vsctprintf_l   _vscprintf_l
#define _vsctprintf_p   _vscprintf_p
#define _vsctprintf_p_l _vscprintf_p_l
#define _vsntprintf     _vsnprintf
#define _vsntprintf_l   _vsnprintf_l
#define _vsntprintf_s   _vsnprintf_s
#define _vsntprintf_s_l _vsnprintf_s_l

#define _tscanf         scanf
#define _tscanf_l       _scanf_l
#define _tscanf_s       scanf_s
#define _tscanf_s_l     _scanf_s_l
#define _tcscanf        _cscanf
#define _tcscanf_l      _cscanf_l
#define _tcscanf_s      _cscanf_s
#define _tcscanf_s_l    _cscanf_s_l
#define _ftscanf        fscanf
#define _ftscanf_l      _fscanf_l
#define _ftscanf_s      fscanf_s
#define _ftscanf_s_l    _fscanf_s_l
#define _stscanf        sscanf
#define _stscanf_l      _sscanf_l
#define _stscanf_s      sscanf_s
#define _stscanf_s_l    _sscanf_s_l
#define _sntscanf       _snscanf
#define _sntscanf_l     _snscanf_l
#define _sntscanf_s     _snscanf_s
#define _sntscanf_s_l   _snscanf_s_l
#define _vtscanf        vscanf
#define _vtscanf_s      vscanf_s
#define _vftscanf       vfscanf
#define _vftscanf_s     vfscanf_s
#define _vstscanf       vsscanf
#define _vstscanf_s     vsscanf_s


/* Unformatted i/o */

#define _fgettc          fgetc
#define _fgettc_nolock   _fgetc_nolock
#define _fgettchar       _fgetchar
#define _fgetts          fgets
#define _fputtc          fputc
#define _fputtc_nolock   _fputc_nolock
#define _fputtchar       _fputchar
#define _fputts          fputs
#define _cputts          _cputs
#define _gettc           getc
#define _gettc_nolock    _getc_nolock
#define _gettch          _getch
#define _gettch_nolock   _getch_nolock
#define _gettche         _getche
#define _gettche_nolock  _getche_nolock
#define _gettchar        getchar
#define _gettchar_nolock _getchar_nolock
#define _getts_s         gets_s
#define _cgetts_s        _cgets_s
#define _puttc           putc
#define _puttc_nolock    _fputc_nolock
#define _puttchar        putchar
#define _puttchar_nolock _putchar_nolock
#define _puttch          _putch
#define _puttch_nolock   _putch_nolock
#define _putts           puts
#define _ungettc         ungetc
#define _ungettc_nolock  _ungetc_nolock
#define _ungettch        _ungetch
#define _ungettch_nolock _ungetch_nolock

/* String conversion functions */

#define _tcstod     strtod
#define _tcstof     strtof
#define _tcstol     strtol
#define _tcstold    strtold
#define _tcstoll    strtoll
#define _tcstoul    strtoul
#define _tcstoull   strtoull
#define _tcstoimax  strtoimax
#define _tcstoumax  strtoumax
#define _ttof       atof
#define _tstof      atof
#define _tstol      atol
#define _tstoll     atoll
#define _tstoi      atoi
#define _tstoi64    _atoi64
#define _tcstod_l     _strtod_l
#define _tcstof_l     _strtof_l
#define _tcstol_l     _strtol_l
#define _tcstold_l    _strtold_l
#define _tcstoll_l    _strtoll_l
#define _tcstoul_l    _strtoul_l
#define _tcstoull_l   _strtoull_l
#define _tcstoimax_l  _strtoimax_l
#define _tcstoumax_l  _strtoumax_l
#define _tstof_l      _atof_l
#define _tstol_l      _atol_l
#define _tstoll_l     _atoll_l
#define _tstoi_l      _atoi_l
#define _tstoi64_l    _atoi64_l

#define _itot_s     _itoa_s
#define _ltot_s     _ltoa_s
#define _ultot_s    _ultoa_s
#define _itot       _itoa
#define _ltot       _ltoa
#define _ultot      _ultoa
#define _ttoi       atoi
#define _ttol       atol
#define _ttoll      atoll

#define _ttoi64     _atoi64
#define _tcstoi64   _strtoi64
#define _tcstoi64_l   _strtoi64_l
#define _tcstoui64  _strtoui64
#define _tcstoui64_l  _strtoui64_l
#define _i64tot_s   _i64toa_s
#define _ui64tot_s  _ui64toa_s
#define _i64tot     _i64toa
#define _ui64tot    _ui64toa

/* String functions */

/* Note that _mbscat, _mbscpy and _mbsdup are functionally equivalent to
strcat, strcpy and strdup, respectively. */

#define _tcscat     strcat
#define _tcscat_s   strcat_s
#define _tcscpy     strcpy
#define _tcscpy_s   strcpy_s
#define _tcsdup     _strdup
#define _tcslen     strlen
#define _tcsnlen    strnlen
#define _tcsxfrm    strxfrm
#define _tcsxfrm_l    _strxfrm_l
#define _tcserror   strerror
#define _tcserror_s   strerror_s
#define __tcserror  _strerror
#define __tcserror_s  _strerror_s

#ifdef _DEBUG
#define _tcsdup_dbg _strdup_dbg
#endif  /* _DEBUG */

/* Execute functions */

#define _texecl     _execl
#define _texecle    _execle
#define _texeclp    _execlp
#define _texeclpe   _execlpe
#define _texecv     _execv
#define _texecve    _execve
#define _texecvp    _execvp
#define _texecvpe   _execvpe

#define _tspawnl    _spawnl
#define _tspawnle   _spawnle
#define _tspawnlp   _spawnlp
#define _tspawnlpe  _spawnlpe
#define _tspawnv    _spawnv
#define _tspawnve   _spawnve
#define _tspawnvp   _spawnvp
#define _tspawnvpe  _spawnvpe

#define _tsystem    system


/* Time functions */

#define _tasctime   asctime
#define _tctime     ctime
#define _tctime32   _ctime32
#define _tctime64   _ctime64
#define _tstrdate   _strdate
#define _tstrtime   _strtime
#define _tutime     _utime
#define _tutime32   _utime32
#define _tutime64   _utime64
#define _tcsftime   strftime
#define _tcsftime_l _strftime_l

#define _tasctime_s   asctime_s
#define _tctime_s     ctime_s
#define _tctime32_s   _ctime32_s
#define _tctime64_s   _ctime64_s
#define _tstrdate_s   _strdate_s
#define _tstrtime_s   _strtime_s

/* Directory functions */

#define _tchdir             _chdir
#define _tgetcwd            _getcwd
#define _tgetdcwd           _getdcwd
#define _tgetdcwd_nolock    _getdcwd_nolock
#define _tmkdir             _mkdir
#define _trmdir             _rmdir

#ifdef _DEBUG
#define _tgetcwd_dbg        _getcwd_dbg
#define _tgetdcwd_dbg       _getdcwd_dbg
#define _tgetdcwd_lk_dbg    _getdcwd_lk_dbg
#endif  /* _DEBUG */

/* Environment/Path functions */

#define _tfullpath      _fullpath
#define _tgetenv        getenv
#define _tgetenv_s      getenv_s
#define _tdupenv_s      _dupenv_s
#define _tmakepath      _makepath
#define _tmakepath_s    _makepath_s
#define _tpgmptr        _pgmptr
#define _get_tpgmptr    _get_pgmptr
#define _tputenv        _putenv
#define _tputenv_s      _putenv_s
#define _tsearchenv     _searchenv
#define _tsearchenv_s   _searchenv_s
#define _tsplitpath     _splitpath
#define _tsplitpath_s   _splitpath_s

#ifdef _DEBUG
#define _tfullpath_dbg  _fullpath_dbg
#define _tdupenv_s_dbg  _dupenv_s_dbg
#endif  /* _DEBUG */

/* Stdio functions */

#define _tfdopen    _fdopen
#define _tfsopen    _fsopen
#define _tfopen     fopen
#define _tfopen_s   fopen_s
#define _tfreopen   freopen
#define _tfreopen_s freopen_s
#define _tperror    perror
#define _tpopen     _popen
#define _ttempnam   _tempnam
#define _ttmpnam    tmpnam
#define _ttmpnam_s  tmpnam_s

#ifdef _DEBUG
#define _ttempnam_dbg   _tempnam_dbg
#endif  /* _DEBUG */


/* Io functions */

#define _tchmod     _chmod
#define _tcreat     _creat
#define _tfindfirst      _findfirst
#define _tfindfirst32    _findfirst32
#define _tfindfirst64    _findfirst64
#define _tfindfirsti64   _findfirsti64
#define _tfindfirst32i64 _findfirst32i64
#define _tfindfirst64i32 _findfirst64i32
#define _tfindnext       _findnext
#define _tfindnext32     _findnext32
#define _tfindnext64     _findnext64
#define _tfindnexti64    _findnexti64
#define _tfindnext32i64  _findnext32i64
#define _tfindnext64i32  _findnext64i32
#define _tmktemp            _mktemp
#define _tmktemp_s          _mktemp_s

#define _topen      _open
#define _taccess    _access
#define _taccess_s  _access_s

#define _tremove    remove
#define _trename    rename
#define _tsopen     _sopen
#define _tsopen_s   _sopen_s
#define _tunlink    _unlink

#define _tfinddata_t      _finddata_t
#define _tfinddata32_t    _finddata32_t
#define _tfinddata64_t    __finddata64_t
#define _tfinddatai64_t   _finddatai64_t
#define _tfinddata32i64_t _finddata32i64_t
#define _tfinddata64i32_t _finddata64i32_t

/* ctype functions */
#define _istascii       __isascii
#define _istcntrl       iscntrl
#define _istcntrl_l     _iscntrl_l
#define _istxdigit      isxdigit
#define _istxdigit_l    _isxdigit_l

/* Stat functions */
#define _tstat      _stat
#define _tstat32    _stat32
#define _tstat32i64 _stat32i64
#define _tstat64    _stat64
#define _tstat64i32 _stat64i32
#define _tstati64   _stati64


/* Setlocale functions */

#define _tsetlocale setlocale

/* String functions */

#define _tcschr         strchr
#define _tcscspn        strcspn
#define _tcsncat        strncat
#define _tcsncat_s      strncat_s
#define _tcsncat_l      _strncat_l
#define _tcsncat_s_l    _strncat_s_l
#define _tcsncpy        strncpy
#define _tcsncpy_s      strncpy_s
#define _tcsncpy_l      _strncpy_l
#define _tcsncpy_s_l    _strncpy_s_l
#define _tcspbrk        strpbrk
#define _tcsrchr        strrchr
#define _tcsspn         strspn
#define _tcsstr         strstr
#define _tcstok         strtok
#define _tcstok_s       strtok_s
#define _tcstok_l       _strtok_l
#define _tcstok_s_l     _strtok_s_l

#define _tcsnset        _strnset
#define _tcsnset_s      _strnset_s
#define _tcsnset_l      _strnset_l
#define _tcsnset_s_l    _strnset_s_l
#define _tcsrev         _strrev
#define _tcsset         _strset
#define _tcsset_s       _strset_s
#define _tcsset_l       _strset_l
#define _tcsset_s_l     _strset_s_l

#define _tcscmp         strcmp
#define _tcsicmp        _stricmp
#define _tcsicmp_l      _stricmp_l
#define _tcsnccmp       strncmp
#define _tcsncmp        strncmp
#define _tcsncicmp      _strnicmp
#define _tcsncicmp_l    _strnicmp_l
#define _tcsnicmp       _strnicmp
#define _tcsnicmp_l     _strnicmp_l

#define _tcscoll        strcoll
#define _tcscoll_l      _strcoll_l
#define _tcsicoll       _stricoll
#define _tcsicoll_l     _stricoll_l
#define _tcsnccoll      _strncoll
#define _tcsnccoll_l    _strncoll_l
#define _tcsncoll       _strncoll
#define _tcsncoll_l     _strncoll_l
#define _tcsncicoll     _strnicoll
#define _tcsncicoll_l   _strnicoll_l
#define _tcsnicoll      _strnicoll
#define _tcsnicoll_l    _strnicoll_l

/* "logical-character" mappings */

#define _tcsclen        strlen
#define _tcscnlen       strnlen
#define _tcsclen_l(_String, _Locale) strlen(_String)
#define _tcscnlen_l(_String, _Max_count, _Locale) strnlen((_String), (_Max_count))
#define _tcsnccat       strncat
#define _tcsnccat_s     strncat_s
#define _tcsnccat_l     _strncat_l
#define _tcsnccat_s_l   _strncat_s_l
#define _tcsnccpy       strncpy
#define _tcsnccpy_s     strncpy_s
#define _tcsnccpy_l     _strncpy_l
#define _tcsnccpy_s_l   _strncpy_s_l
#define _tcsncset       _strnset
#define _tcsncset_s     _strnset_s
#define _tcsncset_l     _strnset_l
#define _tcsncset_s_l   _strnset_s_l

typedef char            _TCHAR;

// from WinIoCtl.h
#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028
#define FILE_DEVICE_BATTERY             0x00000029
#define FILE_DEVICE_BUS_EXTENDER        0x0000002a
#define FILE_DEVICE_MODEM               0x0000002b
#define FILE_DEVICE_VDM                 0x0000002c
#define FILE_DEVICE_MASS_STORAGE        0x0000002d
#define FILE_DEVICE_SMB                 0x0000002e
#define FILE_DEVICE_KS                  0x0000002f
#define FILE_DEVICE_CHANGER             0x00000030
#define FILE_DEVICE_SMARTCARD           0x00000031
#define FILE_DEVICE_ACPI                0x00000032
#define FILE_DEVICE_DVD                 0x00000033
#define FILE_DEVICE_FULLSCREEN_VIDEO    0x00000034
#define FILE_DEVICE_DFS_FILE_SYSTEM     0x00000035
#define FILE_DEVICE_DFS_VOLUME          0x00000036
#define FILE_DEVICE_SERENUM             0x00000037
#define FILE_DEVICE_TERMSRV             0x00000038
#define FILE_DEVICE_KSEC                0x00000039
#define FILE_DEVICE_FIPS                0x0000003A
#define FILE_DEVICE_INFINIBAND          0x0000003B
#define FILE_DEVICE_VMBUS               0x0000003E
#define FILE_DEVICE_CRYPT_PROVIDER      0x0000003F
#define FILE_DEVICE_WPD                 0x00000040
#define FILE_DEVICE_BLUETOOTH           0x00000041
#define FILE_DEVICE_MT_COMPOSITE        0x00000042
#define FILE_DEVICE_MT_TRANSPORT        0x00000043
#define FILE_DEVICE_BIOMETRIC		0x00000044
#define FILE_DEVICE_PMI                 0x00000045

typedef enum _MEDIA_TYPE {
	Unknown,                // Format is unknown
	F5_1Pt2_512,            // 5.25", 1.2MB,  512 bytes/sector
	F3_1Pt44_512,           // 3.5",  1.44MB, 512 bytes/sector
	F3_2Pt88_512,           // 3.5",  2.88MB, 512 bytes/sector
	F3_20Pt8_512,           // 3.5",  20.8MB, 512 bytes/sector
	F3_720_512,             // 3.5",  720KB,  512 bytes/sector
	F5_360_512,             // 5.25", 360KB,  512 bytes/sector
	F5_320_512,             // 5.25", 320KB,  512 bytes/sector
	F5_320_1024,            // 5.25", 320KB,  1024 bytes/sector
	F5_180_512,             // 5.25", 180KB,  512 bytes/sector
	F5_160_512,             // 5.25", 160KB,  512 bytes/sector
	RemovableMedia,         // Removable media other than floppy
	FixedMedia,             // Fixed hard disk media
	F3_120M_512,            // 3.5", 120M Floppy
	F3_640_512,             // 3.5" ,  640KB,  512 bytes/sector
	F5_640_512,             // 5.25",  640KB,  512 bytes/sector
	F5_720_512,             // 5.25",  720KB,  512 bytes/sector
	F3_1Pt2_512,            // 3.5" ,  1.2Mb,  512 bytes/sector
	F3_1Pt23_1024,          // 3.5" ,  1.23Mb, 1024 bytes/sector
	F5_1Pt23_1024,          // 5.25",  1.23MB, 1024 bytes/sector
	F3_128Mb_512,           // 3.5" MO 128Mb   512 bytes/sector
	F3_230Mb_512,           // 3.5" MO 230Mb   512 bytes/sector
	F8_256_128,             // 8",     256KB,  128 bytes/sector
	F3_200Mb_512,           // 3.5",   200M Floppy (HiFD)
	F3_240M_512,            // 3.5",   240Mb Floppy (HiFD)
	F3_32M_512              // 3.5",   32Mb Floppy
} MEDIA_TYPE, *PMEDIA_TYPE;

#define _NTSCSI_USER_MODE_
#include "scsi.h"
#include "ntddcdrm.h"
#include "ntddmmc.h"

typedef struct _DISK_GEOMETRY {
	LARGE_INTEGER Cylinders;
	MEDIA_TYPE MediaType;
	DWORD TracksPerCylinder;
	DWORD SectorsPerTrack;
	DWORD BytesPerSector;
} DISK_GEOMETRY, *PDISK_GEOMETRY;

//
// Support for GUID Partition Table (GPT) disks.
//

//
// There are currently two ways a disk can be partitioned. With a traditional
// AT-style master boot record (PARTITION_STYLE_MBR) and with a new, GPT
// partition table (PARTITION_STYLE_GPT). RAW is for an unrecognizable
// partition style. There are a very limited number of things you can
// do with a RAW partititon.
//

typedef enum _PARTITION_STYLE {
	PARTITION_STYLE_MBR,
	PARTITION_STYLE_GPT,
	PARTITION_STYLE_RAW
} PARTITION_STYLE;

//
// The DISK_GEOMETRY_EX structure is returned on issuing an
// IOCTL_DISK_GET_DRIVE_GEOMETRY_EX ioctl.
//

typedef enum _DETECTION_TYPE {
	DetectNone,
	DetectInt13,
	DetectExInt13
} DETECTION_TYPE;

typedef struct _DISK_INT13_INFO {
	WORD   DriveSelect;
	DWORD MaxCylinders;
	WORD   SectorsPerTrack;
	WORD   MaxHeads;
	WORD   NumberDrives;
} DISK_INT13_INFO, *PDISK_INT13_INFO;

typedef struct _DISK_EX_INT13_INFO {
	WORD   ExBufferSize;
	WORD   ExFlags;
	DWORD ExCylinders;
	DWORD ExHeads;
	DWORD ExSectorsPerTrack;
	DWORD64 ExSectorsPerDrive;
	WORD   ExSectorSize;
	WORD   ExReserved;
} DISK_EX_INT13_INFO, *PDISK_EX_INT13_INFO;

#if (_MSC_VER >= 1200)
#pragma warning(push)
#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

typedef struct _DISK_DETECTION_INFO {
	DWORD SizeOfDetectInfo;
	DETECTION_TYPE DetectionType;
	union {
		struct {

			//
			// If DetectionType == DETECTION_INT13 then we have just the Int13
			// information.
			//

			DISK_INT13_INFO Int13;

			//
			// If DetectionType == DETECTION_EX_INT13, then we have the
			// extended int 13 information.
			//

			DISK_EX_INT13_INFO ExInt13;     // If DetectionType == DetectExInt13
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;
} DISK_DETECTION_INFO, *PDISK_DETECTION_INFO;

#ifndef GUID_DEFINED
#define GUID_DEFINED
#if defined(__midl)
typedef struct {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	byte           Data4[8];
} GUID;
#else
typedef struct _GUID {
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[8];
} GUID;
#endif
#endif

typedef struct _DISK_PARTITION_INFO {
	DWORD SizeOfPartitionInfo;
	PARTITION_STYLE PartitionStyle;                 // PartitionStyle = RAW, GPT or MBR
	union {
		struct {                                                        // If PartitionStyle == MBR
			DWORD Signature;                                // MBR Signature
			DWORD CheckSum;                                 // MBR CheckSum
		} Mbr;
		struct {                                                        // If PartitionStyle == GPT
			GUID DiskId;
		} Gpt;
	} DUMMYUNIONNAME;
} DISK_PARTITION_INFO, *PDISK_PARTITION_INFO;

#if (_MSC_VER >= 1200)
#pragma warning(pop)
#endif

//
// The Geometry structure is a variable length structure composed of a
// DISK_GEOMETRY_EX structure followed by a DISK_PARTITION_INFO structure
// followed by a DISK_DETECTION_DATA structure.
//

#if (NTDDI_VERSION < NTDDI_WIN2003)
#define DiskGeometryGetPartition(Geometry)\
                        ((PDISK_PARTITION_INFO)((Geometry)+1))

#define DiskGeometryGetDetect(Geometry)\
                        ((PDISK_DETECTION_INFO)(((PBYTE)DiskGeometryGetPartition(Geometry)+\
                                        DiskGeometryGetPartition(Geometry)->SizeOfPartitionInfo)))
#else
#define DiskGeometryGetPartition(Geometry)\
                        ((PDISK_PARTITION_INFO)((Geometry)->Data))

#define DiskGeometryGetDetect(Geometry)\
                        ((PDISK_DETECTION_INFO)(((DWORD_PTR)DiskGeometryGetPartition(Geometry)+\
                                        DiskGeometryGetPartition(Geometry)->SizeOfPartitionInfo)))
#endif
typedef struct _DISK_GEOMETRY_EX {
	DISK_GEOMETRY Geometry;                                 // Standard disk geometry: may be faked by driver.
	LARGE_INTEGER DiskSize;                                 // Must always be correct
	BYTE  Data[1];                                                  // Partition, Detect info
} DISK_GEOMETRY_EX, *PDISK_GEOMETRY_EX;

//
// IOCTL_STORAGE_GET_MEDIA_TYPES_EX will return an array of DEVICE_MEDIA_INFO
// structures, one per supported type, embedded in the GET_MEDIA_TYPES struct.
//

typedef enum _STORAGE_MEDIA_TYPE {
	//
	// Following are defined in ntdddisk.h in the MEDIA_TYPE enum
	//
	// Unknown,                // Format is unknown
	// F5_1Pt2_512,            // 5.25", 1.2MB,  512 bytes/sector
	// F3_1Pt44_512,           // 3.5",  1.44MB, 512 bytes/sector
	// F3_2Pt88_512,           // 3.5",  2.88MB, 512 bytes/sector
	// F3_20Pt8_512,           // 3.5",  20.8MB, 512 bytes/sector
	// F3_720_512,             // 3.5",  720KB,  512 bytes/sector
	// F5_360_512,             // 5.25", 360KB,  512 bytes/sector
	// F5_320_512,             // 5.25", 320KB,  512 bytes/sector
	// F5_320_1024,            // 5.25", 320KB,  1024 bytes/sector
	// F5_180_512,             // 5.25", 180KB,  512 bytes/sector
	// F5_160_512,             // 5.25", 160KB,  512 bytes/sector
	// RemovableMedia,         // Removable media other than floppy
	// FixedMedia,             // Fixed hard disk media
	// F3_120M_512,            // 3.5", 120M Floppy
	// F3_640_512,             // 3.5" ,  640KB,  512 bytes/sector
	// F5_640_512,             // 5.25",  640KB,  512 bytes/sector
	// F5_720_512,             // 5.25",  720KB,  512 bytes/sector
	// F3_1Pt2_512,            // 3.5" ,  1.2Mb,  512 bytes/sector
	// F3_1Pt23_1024,          // 3.5" ,  1.23Mb, 1024 bytes/sector
	// F5_1Pt23_1024,          // 5.25",  1.23MB, 1024 bytes/sector
	// F3_128Mb_512,           // 3.5" MO 128Mb   512 bytes/sector
	// F3_230Mb_512,           // 3.5" MO 230Mb   512 bytes/sector
	// F8_256_128,             // 8",     256KB,  128 bytes/sector
	// F3_200Mb_512,           // 3.5",   200M Floppy (HiFD)
	//

	DDS_4mm = 0x20,            // Tape - DAT DDS1,2,... (all vendors)
	MiniQic,                   // Tape - miniQIC Tape
	Travan,                    // Tape - Travan TR-1,2,3,...
	QIC,                       // Tape - QIC
	MP_8mm,                    // Tape - 8mm Exabyte Metal Particle
	AME_8mm,                   // Tape - 8mm Exabyte Advanced Metal Evap
	AIT1_8mm,                  // Tape - 8mm Sony AIT
	DLT,                       // Tape - DLT Compact IIIxt, IV
	NCTP,                      // Tape - Philips NCTP
	IBM_3480,                  // Tape - IBM 3480
	IBM_3490E,                 // Tape - IBM 3490E
	IBM_Magstar_3590,          // Tape - IBM Magstar 3590
	IBM_Magstar_MP,            // Tape - IBM Magstar MP
	STK_DATA_D3,               // Tape - STK Data D3
	SONY_DTF,                  // Tape - Sony DTF
	DV_6mm,                    // Tape - 6mm Digital Video
	DMI,                       // Tape - Exabyte DMI and compatibles
	SONY_D2,                   // Tape - Sony D2S and D2L
	CLEANER_CARTRIDGE,         // Cleaner - All Drive types that support Drive Cleaners
	CD_ROM,                    // Opt_Disk - CD
	CD_R,                      // Opt_Disk - CD-Recordable (Write Once)
	CD_RW,                     // Opt_Disk - CD-Rewriteable
	DVD_ROM,                   // Opt_Disk - DVD-ROM
	DVD_R,                     // Opt_Disk - DVD-Recordable (Write Once)
	DVD_RW,                    // Opt_Disk - DVD-Rewriteable
	MO_3_RW,                   // Opt_Disk - 3.5" Rewriteable MO Disk
	MO_5_WO,                   // Opt_Disk - MO 5.25" Write Once
	MO_5_RW,                   // Opt_Disk - MO 5.25" Rewriteable (not LIMDOW)
	MO_5_LIMDOW,               // Opt_Disk - MO 5.25" Rewriteable (LIMDOW)
	PC_5_WO,                   // Opt_Disk - Phase Change 5.25" Write Once Optical
	PC_5_RW,                   // Opt_Disk - Phase Change 5.25" Rewriteable
	PD_5_RW,                   // Opt_Disk - PhaseChange Dual Rewriteable
	ABL_5_WO,                  // Opt_Disk - Ablative 5.25" Write Once Optical
	PINNACLE_APEX_5_RW,        // Opt_Disk - Pinnacle Apex 4.6GB Rewriteable Optical
	SONY_12_WO,                // Opt_Disk - Sony 12" Write Once
	PHILIPS_12_WO,             // Opt_Disk - Philips/LMS 12" Write Once
	HITACHI_12_WO,             // Opt_Disk - Hitachi 12" Write Once
	CYGNET_12_WO,              // Opt_Disk - Cygnet/ATG 12" Write Once
	KODAK_14_WO,               // Opt_Disk - Kodak 14" Write Once
	MO_NFR_525,                // Opt_Disk - Near Field Recording (Terastor)
	NIKON_12_RW,               // Opt_Disk - Nikon 12" Rewriteable
	IOMEGA_ZIP,                // Mag_Disk - Iomega Zip
	IOMEGA_JAZ,                // Mag_Disk - Iomega Jaz
	SYQUEST_EZ135,             // Mag_Disk - Syquest EZ135
	SYQUEST_EZFLYER,           // Mag_Disk - Syquest EzFlyer
	SYQUEST_SYJET,             // Mag_Disk - Syquest SyJet
	AVATAR_F2,                 // Mag_Disk - 2.5" Floppy
	MP2_8mm,                   // Tape - 8mm Hitachi
	DST_S,                     // Ampex DST Small Tapes
	DST_M,                     // Ampex DST Medium Tapes
	DST_L,                     // Ampex DST Large Tapes
	VXATape_1,                 // Ecrix 8mm Tape
	VXATape_2,                 // Ecrix 8mm Tape
#if (NTDDI_VERSION < NTDDI_WINXP)
	STK_EAGLE,                 // STK Eagle
#else
	STK_9840,                  // STK 9840
#endif
	LTO_Ultrium,               // IBM, HP, Seagate LTO Ultrium
	LTO_Accelis,               // IBM, HP, Seagate LTO Accelis
	DVD_RAM,                   // Opt_Disk - DVD-RAM
	AIT_8mm,                   // AIT2 or higher
	ADR_1,                     // OnStream ADR Mediatypes
	ADR_2,
	STK_9940,                  // STK 9940
	SAIT,                      // SAIT Tapes
	VXATape                    // VXA (Ecrix 8mm) Tape
}STORAGE_MEDIA_TYPE, *PSTORAGE_MEDIA_TYPE;

#define MEDIA_ERASEABLE         0x00000001
#define MEDIA_WRITE_ONCE        0x00000002
#define MEDIA_READ_ONLY         0x00000004
#define MEDIA_READ_WRITE        0x00000008

#define MEDIA_WRITE_PROTECTED   0x00000100
#define MEDIA_CURRENTLY_MOUNTED 0x80000000

//
// Define the different storage bus types
// Bus types below 128 (0x80) are reserved for Microsoft use
//

typedef enum _STORAGE_BUS_TYPE {
	BusTypeUnknown = 0x00,
	BusTypeScsi,
	BusTypeAtapi,
	BusTypeAta,
	BusType1394,
	BusTypeSsa,
	BusTypeFibre,
	BusTypeUsb,
	BusTypeRAID,
	BusTypeiScsi,
	BusTypeSas,
	BusTypeSata,
	BusTypeSd,
	BusTypeMmc,
	BusTypeVirtual,
	BusTypeFileBackedVirtual,
	BusTypeMax,
	BusTypeMaxReserved = 0x7F
} STORAGE_BUS_TYPE, * PSTORAGE_BUS_TYPE;

typedef struct _DEVICE_MEDIA_INFO {
	union {
		struct {
			LARGE_INTEGER Cylinders;
			STORAGE_MEDIA_TYPE MediaType;
			DWORD TracksPerCylinder;
			DWORD SectorsPerTrack;
			DWORD BytesPerSector;
			DWORD NumberMediaSides;
			DWORD MediaCharacteristics; // Bitmask of MEDIA_XXX values.
		} DiskInfo;

		struct {
			LARGE_INTEGER Cylinders;
			STORAGE_MEDIA_TYPE MediaType;
			DWORD TracksPerCylinder;
			DWORD SectorsPerTrack;
			DWORD BytesPerSector;
			DWORD NumberMediaSides;
			DWORD MediaCharacteristics; // Bitmask of MEDIA_XXX values.
		} RemovableDiskInfo;

		struct {
			STORAGE_MEDIA_TYPE MediaType;
			DWORD   MediaCharacteristics; // Bitmask of MEDIA_XXX values.
			DWORD   CurrentBlockSize;
			STORAGE_BUS_TYPE BusType;

			//
			// Bus specific information describing the medium supported.
			//

			union {
				struct {
					BYTE  MediumType;
					BYTE  DensityCode;
				} ScsiInformation;
			} BusSpecificData;

		} TapeInfo;
	} DeviceSpecific;
} DEVICE_MEDIA_INFO, *PDEVICE_MEDIA_INFO;

typedef struct _GET_MEDIA_TYPES {
	DWORD DeviceType;              // FILE_DEVICE_XXX values
	DWORD MediaInfoCount;
	DEVICE_MEDIA_INFO MediaInfo[1];
} GET_MEDIA_TYPES, *PGET_MEDIA_TYPES;



typedef ULONG DVD_SESSION_ID, *PDVD_SESSION_ID;

/*++

IOCTL_DVD_READ_STRUCTURE

Issues a READ_DVD_STRUCTURE command to the drive.

Input:

a DVD_READ_STRUCTURE describing what information is requested

Output:

a DVD Layer Descriptor as defined below

--*/

typedef enum DVD_STRUCTURE_FORMAT {
	DvdPhysicalDescriptor,     // 0x00
	DvdCopyrightDescriptor,    // 0x01
	DvdDiskKeyDescriptor,      // 0x02
	DvdBCADescriptor,          // 0x03
	DvdManufacturerDescriptor, // 0x04
	DvdMaxDescriptor           // 0x05
} DVD_STRUCTURE_FORMAT, *PDVD_STRUCTURE_FORMAT;

/////////////////////////////////////////////////////////////

typedef struct DVD_READ_STRUCTURE {
	LARGE_INTEGER BlockByteOffset;
	DVD_STRUCTURE_FORMAT Format;
	DVD_SESSION_ID SessionId;
	UCHAR LayerNumber;
} DVD_READ_STRUCTURE, *PDVD_READ_STRUCTURE;

typedef struct _DVD_DESCRIPTOR_HEADER {
	USHORT Length;
	UCHAR Reserved[2];
#if !defined(__midl)
	UCHAR Data[0];
#endif
} DVD_DESCRIPTOR_HEADER, *PDVD_DESCRIPTOR_HEADER;

// format 0x00 - DvdPhysicalDescriptor
typedef struct _DVD_LAYER_DESCRIPTOR {
	UCHAR BookVersion : 4;      // in MMC 5 :   Part Version
	UCHAR BookType : 4;         //              Disk Category
	UCHAR MinimumRate : 4;      //              Maximum Rate
	UCHAR DiskSize : 4;
	UCHAR LayerType : 4;
	UCHAR TrackPath : 1;
	UCHAR NumberOfLayers : 2;
	UCHAR Reserved1 : 1;
	UCHAR TrackDensity : 4;
	UCHAR LinearDensity : 4;
	ULONG StartingDataSector;   //              3bytes + 1 zeroed byte
	ULONG EndDataSector;        //              3bytes + 1 zeroed byte
	ULONG EndLayerZeroSector;   //              3bytes + 1 zeroed byte
	UCHAR Reserved5 : 7;
	UCHAR BCAFlag : 1;
	// The large Media Specific field is not declared here to enable stack allocation
} DVD_LAYER_DESCRIPTOR, *PDVD_LAYER_DESCRIPTOR;

typedef struct _DVD_FULL_LAYER_DESCRIPTOR {
	DVD_LAYER_DESCRIPTOR commonHeader;
	UCHAR MediaSpecific[2031];
} DVD_FULL_LAYER_DESCRIPTOR, *PDVD_FULL_LAYER_DESCRIPTOR;


// format 0x01 - DvdCopyrightDescriptor
typedef struct _DVD_COPYRIGHT_DESCRIPTOR {
	UCHAR CopyrightProtectionType;
	UCHAR RegionManagementInformation;
	USHORT Reserved;
} DVD_COPYRIGHT_DESCRIPTOR, *PDVD_COPYRIGHT_DESCRIPTOR;


// format 0x02 - DvdDiskKeyDescriptor
typedef struct _DVD_DISK_KEY_DESCRIPTOR {
	UCHAR DiskKeyData[2048];
} DVD_DISK_KEY_DESCRIPTOR, *PDVD_DISK_KEY_DESCRIPTOR;


// format 0x03 - DvdBCADescriptor
typedef struct _DVD_BCA_DESCRIPTOR {
	UCHAR BCAInformation[0];
} DVD_BCA_DESCRIPTOR, *PDVD_BCA_DESCRIPTOR;

// format 0x04 - DvdManufacturerDescriptor
typedef struct _DVD_MANUFACTURER_DESCRIPTOR {
	UCHAR ManufacturingInformation[2048];
} DVD_MANUFACTURER_DESCRIPTOR, *PDVD_MANUFACTURER_DESCRIPTOR;


// format 0x05 - not defined in enum
typedef struct _DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR {
	union {
		struct {
			UCHAR CopyProtectionMode : 4;
			UCHAR ContentGenerationManagementSystem : 2;
			UCHAR CopyProtectedSector : 1;
			UCHAR CopyProtectedMaterial : 1;
		} Dvdrom;
		struct {
			UCHAR Reserved0001 : 4;
			UCHAR ContentGenerationManagementSystem : 2;
			UCHAR Reserved0002 : 1;
			UCHAR CopyProtectedMaterial : 1;
		} DvdRecordable_Version1;
		struct {
			UCHAR Reserved0003;
		} Dvdram;
		struct {
			UCHAR Reserved0004 : 2;
			UCHAR ADP_TY : 2; // what is this mean?
			UCHAR Reserved0005 : 4;
		} DvdRecordable;
		UCHAR CPR_MAI;
	};
	UCHAR Reserved0[3];
} DVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR, *PDVD_COPYRIGHT_MANAGEMENT_DESCRIPTOR;

// format 0x06 (media ID) is unstructured in public spec
// format 0x07 (media key block) is unstructured in public spec
// format 0x08 (DVD-RAM DDS) is unstructured in public spec

// format 0x09 - not defined in enum
// This is valid for DVD-RAM and also HD DVD-RAM
typedef struct _DVD_RAM_MEDIUM_STATUS {
	UCHAR Reserved0 : 1;
	UCHAR PersistentWriteProtect : 1;
	UCHAR CartridgeWriteProtect : 1;
	UCHAR MediaSpecificWriteInhibit : 1;
	UCHAR Reserved1 : 2;
	UCHAR CartridgeNotSealed : 1;
	UCHAR MediaInCartridge : 1;
	UCHAR DiscTypeIdentification;
	UCHAR Reserved2;
	UCHAR MediaSpecificWriteInhibitInformation;
} DVD_RAM_MEDIUM_STATUS, *PDVD_RAM_MEDIUM_STATUS;

// format 0x0A - not defined in enum
typedef struct _DVD_RAM_SPARE_AREA_INFORMATION {
	UCHAR FreePrimarySpareSectors[4];
	UCHAR FreeSupplementalSpareSectors[4];
	UCHAR AllocatedSupplementalSpareSectors[4];
} DVD_RAM_SPARE_AREA_INFORMATION, *PDVD_RAM_SPARE_AREA_INFORMATION;

// format 0x0B - not defined in enum
typedef struct _DVD_RAM_RECORDING_TYPE {
	UCHAR Reserved0 : 4;
	UCHAR RealTimeData : 1;
	UCHAR Reserved1 : 3;
	UCHAR Reserved2[3];
} DVD_RAM_RECORDING_TYPE, *PDVD_RAM_RECORDING_TYPE;

// format 0x0C (RMD in last border-out) is unstructured in public spec
// format 0x0D - not defined in enum
typedef struct _DVD_RECORDING_MANAGEMENT_AREA_DATA {
	UCHAR LastRecordedRMASectorNumber[4];
#if !defined(__midl)
	UCHAR RMDBytes[0];
#endif
} DVD_RECORDING_MANAGEMENT_AREA_DATA, *PDVD_RECORDING_MANAGEMENT_AREA_DATA;

// format 0x0E - not define in enum
typedef struct _DVD_PRERECORDED_INFORMATION {
	UCHAR FieldID_1;
	UCHAR DiscApplicationCode;
	UCHAR DiscPhysicalCode;
	UCHAR LastAddressOfDataRecordableArea[3];
	UCHAR ExtensionCode : 4; // -R for general/authoring v2.0
	UCHAR PartVers1on : 4; // -R for general/authoring v2.0
	UCHAR Reserved0;
	UCHAR FieldID_2;
	UCHAR OpcSuggestedCode;
	UCHAR WavelengthCode;
	UCHAR WriteStrategyCode[4];
	UCHAR Reserved2;
	UCHAR FieldID_3;
	UCHAR ManufacturerId_3[6];
	UCHAR Reserved3;
	UCHAR FieldID_4;
	UCHAR ManufacturerId_4[6];
	UCHAR Reserved4;
	UCHAR FieldID_5;
	UCHAR ManufacturerId_5[6];
	UCHAR Reserved5;
	UCHAR Reserved99[24];
} DVD_PRERECORDED_INFORMATION, *PDVD_PRERECORDED_INFORMATION;

// format 0x0F - not defined in enum
typedef struct _DVD_UNIQUE_DISC_IDENTIFIER {
	UCHAR Reserved0[2];
	UCHAR RandomNumber[2];
	UCHAR Year[4];   // ASCII?
	UCHAR Month[2];  // ASCII?
	UCHAR Day[2];    // ASCII?
	UCHAR Hour[2];   // ASCII?
	UCHAR Minute[2]; // ASCII?
	UCHAR Second[2]; // ASCII?
} DVD_UNIQUE_DISC_IDENTIFIER, *PDVD_UNIQUE_DISC_IDENTIFIER;

// format 0x10 - not define in enum - use DVD_LAYER_DESCRIPTOR structure above
// format 0x11 (ADIP information) is unstructured in public spec
// formats 0x12, 0x15 are is unstructured in public spec
// formats 0x13, 0x14, 0x16 through 0x18 are not yet defined

// format 0x19 - not defined in enum
typedef struct _HD_DVD_R_MEDIUM_STATUS {
	UCHAR ExtendedTestZone : 1;
	UCHAR Reserved1 : 7;
	UCHAR NumberOfRemainingRMDsInRDZ;
	UCHAR NumberOfRemainingRMDsInCurrentRMZ[2];
} HD_DVD_R_MEDIUM_STATUS, *PHD_DVD_R_MEDIUM_STATUS;

// format 0x1A (HD DVD-R - Last recorded RMD in the latest R) is unstructured in public spec
// formats 0x1B through 0x1F are not yet defined

// format 0x20 - not define in enum
typedef struct _DVD_DUAL_LAYER_RECORDING_INFORMATION {
	UCHAR Reserved0 : 7;
	UCHAR Layer0SectorsImmutable : 1;
	UCHAR Reserved1[3];
	UCHAR Layer0Sectors[4];
} DVD_DUAL_LAYER_RECORDING_INFORMATION, *PDVD_DUAL_LAYER_RECORDING_INFORMATION;

// format 0x21 - not define in enum
typedef struct _DVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS {
	UCHAR Reserved0 : 7;
	UCHAR InitStatus : 1;
	UCHAR Reserved1[3];
	UCHAR ShiftedMiddleAreaStartAddress[4];
} DVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS, *PDVD_DUAL_LAYER_MIDDLE_ZONE_START_ADDRESS;

// format 0x22 - not define in enum
typedef struct _DVD_DUAL_LAYER_JUMP_INTERVAL_SIZE {
	UCHAR Reserved1[4];
	UCHAR JumpIntervalSize[4];
} DVD_DUAL_LAYER_JUMP_INTERVAL_SIZE, *PDVD_DUAL_LAYER_JUMP_INTERVAL_SIZE;

// format 0x23 - not define in enum
typedef struct _DVD_DUAL_LAYER_MANUAL_LAYER_JUMP {
	UCHAR Reserved1[4];
	UCHAR ManualJumpLayerAddress[4];
} DVD_DUAL_LAYER_MANUAL_LAYER_JUMP, *PDVD_DUAL_LAYER_MANUAL_LAYER_JUMP;

// format 0x24 - not define in enum
typedef struct _DVD_DUAL_LAYER_REMAPPING_INFORMATION {
	UCHAR Reserved1[4];
	UCHAR RemappingAddress[4];
} DVD_DUAL_LAYER_REMAPPING_INFORMATION, *PDVD_DUAL_LAYER_REMAPPING_INFORMATION;

// formats 0x25 through 0x2F are not yet defined

// format 0x30 - not defined in enum (common header)
typedef struct _DVD_DISC_CONTROL_BLOCK_HEADER {
	UCHAR ContentDescriptor[4];
	union {
		struct {
			UCHAR ReservedDoNotUse_UseAsByteInstead_0[3];
			UCHAR RecordingWithinTheUserDataArea : 1;
			UCHAR ReadingDiscControlBlocks : 1;
			UCHAR FormattingTheMedium : 1;
			UCHAR ModificationOfThisDiscControlBlock : 1;
			UCHAR ReservedDoNotUse_UseAsByteInstead_1 : 4;
		};
		UCHAR AsByte[4];
	} ProhibitedActions;
	UCHAR VendorId[32]; // actually "non-specified" data
						// UCHAR DCBData[32728];
} DVD_DISC_CONTROL_BLOCK_HEADER, *PDVD_DISC_CONTROL_BLOCK_HEADER;

// publicly defined DCB types
typedef enum _DISC_CONTROL_BLOCK_TYPE {
	FormattingDiscControlBlock = 0x46444300, // 'FDC\0'
	WriteInhibitDiscControlBlock = 0x57444300, // 'WDC\0'
	SessionInfoDiscControlBlock = 0x53444300, // 'SDC\0'
	DiscControlBlockList = 0xFFFFFFFF
} DISC_CONTROL_BLOCK_TYPE, *PDISC_CONTROL_BLOCK_TYPE;

// format 0x30 - not defined in enum -- Format DCB, not in MMC.

// format 0x30 - not defined in enum -- Write Inhibit DCB
typedef struct _DVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT {
	DVD_DISC_CONTROL_BLOCK_HEADER header;
	UCHAR UpdateCount[4];
	union {
		struct {
			UCHAR ReservedDoNotUse_UseAsByteInstead_0[3];
			UCHAR WriteProtectStatus : 2;
			UCHAR ReservedDoNotUse_UseAsByteInstead_1 : 5;
			UCHAR UpdateRequiresPassword : 1;
		};
		UCHAR AsByte[4];
	} WriteProtectActions;
	UCHAR Reserved0[16];
	UCHAR UpdatePassword[32];
	UCHAR Reserved1[32672];
} DVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT, *PDVD_DISC_CONTROL_BLOCK_WRITE_INHIBIT;

// format 0x30 - not defined in enum - Session DCB
typedef struct _DVD_DISC_CONTROL_BLOCK_SESSION_ITEM {
	UCHAR AsByte[16]; // not publicly defined?
} DVD_DISC_CONTROL_BLOCK_SESSION_ITEM, *PDVD_DISC_CONTROL_BLOCK_SESSION_ITEM;
typedef struct _DVD_DISC_CONTROL_BLOCK_SESSION {
	DVD_DISC_CONTROL_BLOCK_HEADER header;
	UCHAR SessionNumber[2];
	UCHAR Reserved0[22];
	UCHAR DiscID[32];
	UCHAR Reserved1[32];
	DVD_DISC_CONTROL_BLOCK_SESSION_ITEM SessionItem[504];
	UCHAR Reserved2[24576]; // 3 Repetitions of bytes 0 through 8191
} DVD_DISC_CONTROL_BLOCK_SESSION, *PDVD_DISC_CONTROL_BLOCK_SESSION;

// format 0x30 - not defined in enum - DCB list
typedef struct _DVD_DISC_CONTROL_BLOCK_LIST_DCB {
	UCHAR DcbIdentifier[4];
} DVD_DISC_CONTROL_BLOCK_LIST_DCB, *PDVD_DISC_CONTROL_BLOCK_LIST_DCB;
typedef struct _DVD_DISC_CONTROL_BLOCK_LIST {
	DVD_DISC_CONTROL_BLOCK_HEADER header;
	UCHAR Reserved0;
	UCHAR ReadabldDCBs;
	UCHAR Reserved1;
	UCHAR WritableDCBs;
#if !defined(__midl)
	DVD_DISC_CONTROL_BLOCK_LIST_DCB Dcbs[0];
#endif
} DVD_DISC_CONTROL_BLOCK_LIST, *PDVD_DISC_CONTROL_BLOCK_LIST;

// format 0x31 (MTA ECC Block) is unstructured in public spec
// formats 0x32 through 0xBF are not yet defined

// format 0xC0 - not defined in enum
typedef struct _DVD_WRITE_PROTECTION_STATUS {
	UCHAR SoftwareWriteProtectUntilPowerdown : 1;
	UCHAR MediaPersistentWriteProtect : 1;
	UCHAR CartridgeWriteProtect : 1;
	UCHAR MediaSpecificWriteProtect : 1;
	UCHAR Reserved0 : 4;
	UCHAR Reserved1[3];
} DVD_WRITE_PROTECTION_STATUS, *PDVD_WRITE_PROTECTION_STATUS;

// formats 0xC1 through 0x7F are not yet defined
// format 0x80 (AACS volume identifier) is unstructured in public spec
// format 0x81 (Pre-Recorded AACS media serial number) is unstructured in public spec
// format 0x82 (AACS media identifier) is unstructured in public spec
// format 0x83 (AACS media key block) is unstructured in public spec
// formats 0x84 through 0x8F are not yet defined

// format 0x90 - not defined in enum
typedef struct _DVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE {
	UCHAR TypeCodeOfFormatLayer[2];
} DVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS, *PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS;
typedef struct _DVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS {
	UCHAR NumberOfRecognizedFormatLayers;
	UCHAR OnlineFormatlayer : 2;
	UCHAR Reserved1 : 2;
	UCHAR DefaultFormatLayer : 2;
	UCHAR Reserved2 : 2;
	// DVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE TypeCodes[0];
} DVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE, *PDVD_LIST_OF_RECOGNIZED_FORMAT_LAYERS_TYPE_CODE;

// formats 0x91 through 0xFE are not yet defined

// format 0xFF - not defined in enum
typedef struct _DVD_STRUCTURE_LIST_ENTRY {
	UCHAR FormatCode;
	UCHAR Reserved0 : 6;
	UCHAR Readable : 1;
	UCHAR Sendable : 1;
	UCHAR FormatLength[2];
} DVD_STRUCTURE_LIST_ENTRY, *PDVD_STRUCTURE_LIST_ENTRY;

// BD Disc Structures

// format 0x00 (BD Disc Information) is unstructured in public spec
// format 0x08 (BD Disc Definition Structure) is unstructured in public spec
// format 0x09 (BD Cartridge Status) is identical to DVD_RAM_MEDIUM_STATUS but
// only CartridgeWriteProtect, CartridgeNotSealed and MediaInCartridge are
// valid. Other fields are reserved.

// format 0x09 (BD Spare Area Information) - not defined in enum
typedef struct _DVD_BD_SPARE_AREA_INFORMATION {
	UCHAR Reserved1[4];
	UCHAR NumberOfFreeSpareBlocks[4];
	UCHAR NumberOfAllocatedSpareBlocks[4];
} DVD_BD_SPARE_AREA_INFORMATION, *PDVD_BD_SPARE_AREA_INFORMATION;

// format 0x12 (BD Raw Defect List). DFL is not fully defined in public spec

// format 0x30 (BD Physical Access Control).
typedef struct _BD_PAC_HEADER {
	UCHAR PACId[3];
	UCHAR PACFormatNumber;
	UCHAR PACUpdateCount[4];
	UCHAR UnknownPACRules[4];
	UCHAR UnkownPACEntireDiscFlag;
	UCHAR Reserved1[2];
	UCHAR NumberOfSegments;
	UCHAR Segments[8][32];
	UCHAR Reserved2[112];
} BD_PAC_HEADER, *PBD_PAC_HEADER;

// Primary PAC is unstructured in public spec

// Disc Write Protect PAC
typedef struct _BD_DISC_WRITE_PROTECT_PAC {
	BD_PAC_HEADER Header;
	UCHAR KnownPACEntireDiscFlags;
	UCHAR Reserved1[3];
	UCHAR WriteProtectControlByte;
	UCHAR Reserved2[7];
	UCHAR WriteProtectPassword[32];
} BD_DISC_WRITE_PROTECT_PAC, *PBD_DISC_WRITE_PROTECT_PAC;


typedef struct _DVD_RPC_KEY {
	UCHAR UserResetsAvailable : 3;
	UCHAR ManufacturerResetsAvailable : 3;
	UCHAR TypeCode : 2;
	UCHAR RegionMask;
	UCHAR RpcScheme;
	UCHAR Reserved02;
} DVD_RPC_KEY, *PDVD_RPC_KEY;

typedef struct _DVD_SET_RPC_KEY {
	UCHAR PreferredDriveRegionCode;
	UCHAR Reserved[3];
} DVD_SET_RPC_KEY, *PDVD_SET_RPC_KEY;

typedef struct _DVD_ASF { // Authentication Success Flag
	UCHAR Reserved0[3];
	UCHAR SuccessFlag : 1;
	UCHAR Reserved1 : 7;
} DVD_ASF, *PDVD_ASF;

typedef struct _DVD_REGION {
	UCHAR CopySystem;
	UCHAR RegionData;                      // current media region (not playable when set)
	UCHAR SystemRegion;                    // current drive region (playable when set)
	UCHAR ResetCount;                      // number of resets available
} DVD_REGION, *PDVD_REGION;

/////////////////////////////////////////////////////////////
// AACS-related structures
// (mostly opaque data, but useful for allocation)

// The AACS layer number refers to the layer of the disc a structure
// is read from.  This can only be a single byte in the CDB, so limit
// the value to 0..255.
typedef ULONG  AACS_LAYER_NUMBER, *PAACS_LAYER_NUMBER;
typedef ULONG CAACS_LAYER_NUMBER, *PCAACS_LAYER_NUMBER;


// The AACS Certificate (opaque data structure) is used to validate
// the host to the logical unit, as well as to validate the logical
// unit to the host.
typedef struct _AACS_CERTIFICATE {
	UCHAR Nonce[20];
	UCHAR Certificate[92];
} AACS_CERTIFICATE, *PAACS_CERTIFICATE;
typedef const AACS_CERTIFICATE   CAACS_CERTIFICATE;
typedef const AACS_CERTIFICATE *PCAACS_CERTIFICATE;

// The AACS challenge key (opaque data structure) is used to setup
// a shared bus key for AACS-protected structure transfer.
typedef struct _AACS_CHALLENGE_KEY {
	UCHAR EllipticCurvePoint[40];
	UCHAR Signature[40];
} AACS_CHALLENGE_KEY, *PAACS_CHALLENGE_KEY;
typedef const AACS_CHALLENGE_KEY   CAACS_CHALLENGE_KEY;
typedef const AACS_CHALLENGE_KEY *PCAACS_CHALLENGE_KEY;

// The VolumeID is one of the unique identifiers on AACS protected media
typedef struct _AACS_VOLUME_ID {
	UCHAR VolumeID[16];
	UCHAR MAC[16]; // MessageAuthenticationCode
} AACS_VOLUME_ID, *PAACS_VOLUME_ID;
typedef const AACS_VOLUME_ID   CAACS_VOLUME_ID;
typedef const AACS_VOLUME_ID *PCAACS_VOLUME_ID;

// The prerecorded Serial Number is one of the unique identifiers on AACS protected media
typedef struct _AACS_SERIAL_NUMBER {
	UCHAR PrerecordedSerialNumber[16];
	UCHAR MAC[16]; // MessageAuthenticationCode
} AACS_SERIAL_NUMBER, *PAACS_SERIAL_NUMBER;
typedef const AACS_SERIAL_NUMBER   CAACS_SERIAL_NUMBER;
typedef const AACS_SERIAL_NUMBER *PCAACS_SERIAL_NUMBER;

// The MediaID is one of the unique identifiers on AACS protected media
typedef struct _AACS_MEDIA_ID {
	UCHAR MediaID[16];
	UCHAR MAC[16]; // MessageAuthenticationCode
} AACS_MEDIA_ID, *PAACS_MEDIA_ID;
typedef const AACS_MEDIA_ID   CAACS_MEDIA_ID;
typedef const AACS_MEDIA_ID *PCAACS_MEDIA_ID;

// When sending a certificate or challenge key, need to wrap
// the data structure with a DVD_SESSION_ID.
typedef struct _AACS_SEND_CERTIFICATE {
	DVD_SESSION_ID SessionId;
	AACS_CERTIFICATE Certificate;
} AACS_SEND_CERTIFICATE, *PAACS_SEND_CERTIFICATE;
typedef const AACS_SEND_CERTIFICATE   CAACS_SEND_CERTIFICATE;
typedef const AACS_SEND_CERTIFICATE *PCAACS_SEND_CERTIFICATE;

// When sending a certificate or challenge key, need to wrap
// the data structure with a DVD_SESSION_ID.
typedef struct _AACS_SEND_CHALLENGE_KEY {
	DVD_SESSION_ID SessionId;
	AACS_CHALLENGE_KEY ChallengeKey;
} AACS_SEND_CHALLENGE_KEY, *PAACS_SEND_CHALLENGE_KEY;
typedef const AACS_SEND_CHALLENGE_KEY   CAACS_SEND_CHALLENGE_KEY;
typedef const AACS_SEND_CHALLENGE_KEY *PCAACS_SEND_CHALLENGE_KEY;


// The AACS binding nonce (opaque data structure) is used to
// protect individual content.
typedef struct _AACS_BINDING_NONCE {
	UCHAR BindingNonce[16];
	UCHAR MAC[16]; // MessageAuthenticationCode
} AACS_BINDING_NONCE, *PAACS_BINDING_NONCE;
typedef const AACS_BINDING_NONCE   CAACS_BINDING_NONCE;
typedef const AACS_BINDING_NONCE *PCAACS_BINDING_NONCE;


// This structure is sent when reading a binding nonce
// either from the medium or when having the logical unit
// generate a new binding nonce for a set of sectors
// NOTE: This structure must be identically aligned for 32/64 bit builds
//       
typedef struct _AACS_READ_BINDING_NONCE {
	DVD_SESSION_ID        SessionId;
	ULONG  NumberOfSectors; // spec only provides one byte
	ULONGLONG             StartLba;

	// 32-bit HANDLE is 32 bits, 64-bit HANDLE is 64 bits
	union {
		HANDLE                Handle;
		ULONGLONG             ForceStructureLengthToMatch64bit;
	};
} AACS_READ_BINDING_NONCE, *PAACS_READ_BINDING_NONCE;


// from stdlib.h
#define _MAX_PATH   PATH_MAX
#define _MAX_DRIVE  NAME_MAX
#define _MAX_DIR    NAME_MAX
#define _MAX_FNAME  NAME_MAX
#define _MAX_EXT    NAME_MAX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/statvfs.h>
#include <linux/cdrom.h>
#include <linux/iso_fs.h>
#include <linux/mmc/ioctl.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <wchar.h>
#include <locale.h>
#include <libgen.h>

#define __wchar_t wchar_t

#define _strnicmp strncasecmp


#define CP_ACP 1

#define NO_ERROR 0L

typedef struct _SCSI_ADDRESS {
	ULONG Length;
	UCHAR PortNumber;
	UCHAR PathId;
	UCHAR TargetId;
	UCHAR Lun;
}SCSI_ADDRESS, *PSCSI_ADDRESS;

typedef enum _STORAGE_QUERY_TYPE {
	PropertyStandardQuery = 0,          // Retrieves the descriptor
	PropertyExistsQuery,                // Used to test whether the descriptor is supported
	PropertyMaskQuery,                  // Used to retrieve a mask of writeable fields in the descriptor
	PropertyQueryMaxDefined     // use to validate the value
} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;

//
// define some initial property id's
//

typedef enum _STORAGE_PROPERTY_ID {
	StorageDeviceProperty = 0,
	StorageAdapterProperty,
	StorageDeviceIdProperty,
	StorageDeviceUniqueIdProperty,              // See storduid.h for details
	StorageDeviceWriteCacheProperty,
	StorageMiniportProperty,
	StorageAccessAlignmentProperty,
	StorageDeviceSeekPenaltyProperty,
	StorageDeviceTrimProperty,
	StorageDeviceWriteAggregationProperty
} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;

//
// Query structure - additional parameters for specific queries can follow
// the header
//

typedef struct _STORAGE_PROPERTY_QUERY {

	//
	// ID of the property being retrieved
	//

	STORAGE_PROPERTY_ID PropertyId;

	//
	// Flags indicating the type of query being performed
	//

	STORAGE_QUERY_TYPE QueryType;

	//
	// Space for additional parameters if necessary
	//

	BYTE  AdditionalParameters[1];

} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;

//
// Standard property descriptor header.  All property pages should use this
// as their first element or should contain these two elements
//

typedef struct _STORAGE_DESCRIPTOR_HEADER {

	DWORD Version;

	DWORD Size;

} STORAGE_DESCRIPTOR_HEADER, *PSTORAGE_DESCRIPTOR_HEADER;

//
// Adapter properties
//
// This descriptor can be retrieved from a target device object of from the
// device object for the bus.  Retrieving from the target device object will
// forward the request to the underlying bus
//

typedef struct _STORAGE_ADAPTER_DESCRIPTOR {

	DWORD Version;

	DWORD Size;

	DWORD MaximumTransferLength;

	DWORD MaximumPhysicalPages;

	DWORD AlignmentMask;

	BOOLEAN AdapterUsesPio;

	BOOLEAN AdapterScansDown;

	BOOLEAN CommandQueueing;

	BOOLEAN AcceleratedTransfer;

#if (NTDDI_VERSION < NTDDI_WINXP)
	BOOLEAN BusType;
#else
	BYTE  BusType;
#endif

	WORD   BusMajorVersion;

	WORD   BusMinorVersion;

} STORAGE_ADAPTER_DESCRIPTOR, *PSTORAGE_ADAPTER_DESCRIPTOR;

#define IOCTL_SCSI_PASS_THROUGH_DIRECT	SG_IO
#define IOCTL_DISK_GET_DRIVE_GEOMETRY	SG_IO
#define IOCTL_DISK_GET_MEDIA_TYPES		SG_IO
#define IOCTL_SCSI_GET_ADDRESS			SG_IO
#define IOCTL_STORAGE_QUERY_PROPERTY	SG_IO
#define IOCTL_STORAGE_GET_MEDIA_TYPES_EX	SG_IO
#define IOCTL_DISK_GET_DRIVE_GEOMETRY_EX	SG_IO

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER {
	sg_io_hdr_t io_hdr;
	unsigned char Dummy[18];
	SENSE_DATA SenseData;
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

#define MOVEFILE_REPLACE_EXISTING 0

int Beep(int fz, int time);

int GetCurrentDirectory(size_t size, char *buf);

int GetTempPath(size_t size, char* buf);

// https://groups.google.com/forum/#!topic/gnu.gcc.help/0dKxhmV4voE
// Abstract:   split a path into its parts
// Parameters: Path: Object to split
//             Drive: Logical drive , only for compatibility , not considered
//             Directory: Directory part of path
//             Filename: File part of path
//             Extension: Extension part of path (includes the leading point)
// Returns:    Directory Filename and Extension are changed
// Comment:    Note that the concept of an extension is not available in Linux,
//             nevertheless it is considered

void _splitpath(const char* Path, char* Drive, char* Directory, char* Filename, char* Extension);

// Abstract:   Make a path out of its parts
// Parameters: Path: Object to be made
//             Drive: Logical drive , only for compatibility , not considered
//             Directory: Directory part of path
//             Filename: File part of path
//             Extension: Extension part of path (includes the leading point)
// Returns:    Path is changed
// Comment:    Note that the concept of an extension is not available in Linux,
//             nevertheless it is considered

void _makepath(char* Path, const char* Drive, const char* Directory, const char* File, const char* Extension);

// http://stackoverflow.com/questions/3218201/find-a-replacement-for-windows-pathappend-on-gnu-linux
int PathAppend(char* path, char const* more);

int PathSet(char* path, char const* fullpath);

// https://www.quora.com/How-do-I-check-if-a-file-already-exists-using-C-file-I-O
int PathFileExists(const char *filename);

// http://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
int MakeSureDirectoryPathExists(const char *dir);

int PathRemoveFileSpec(char* path);

int PathRemoveExtension(char* path);

// http://jajagacchi.jugem.jp/?eid=123
int PathRenameExtension(char* path, const char* ext);

int MoveFileEx(const char* srcFile, const char* dstFile, int flag);

int CopyFile(const char* srcFile, const char* dstFile, int flag);

int _fseeki64(FILE* fp, ULONGLONG ofs, int origin);

off_t _ftelli64(FILE* fp);

// http://pdfbjj.blogspot.com/2012/04/linuxgetmodulefilename.html
int GetModuleFileName(void* a, char* path, unsigned long size);

int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr
	, int cchWideChar, LPSTR lpMultiByteStr, int cchMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);

int MultiByteToWideChar(UINT CodePage, DWORD dwFlags,
	LPCSTR lpMultiByteStr, int cchMultiByte, LPWSTR lpWideCharStr, int cchWideChar);

void SetLastError(int errcode);

int GetLastError(void);

int CloseHandle(int fd);

int DeviceIoControl(int fd, unsigned long ioCtlCode, void* inbuf
	, unsigned long a, void* b, unsigned long c, unsigned long* d, void* e);

int ReadFile(int fd, void* inbuf, unsigned long size, unsigned long* d, void* e);

#define FILE_BEGIN SEEK_SET
off_t SetFilePointer(int fd, off_t pos, void* a, int origin);

off64_t SetFilePointerEx(int fd, LARGE_INTEGER pos, void* a, int origin);

unsigned int Sleep(unsigned long seconds);
#endif

int GetDiskFreeSpaceEx(LPCSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailableToCaller, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);
