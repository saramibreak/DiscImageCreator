// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
#ifdef _DEBUG
#include <vld.h>
#endif
#pragma warning(disable:4200 4710 4711 5045)
#pragma warning(push)
#pragma warning(disable:4091 4191 4365 4514 4668 4768 4820 4917 5039)
#pragma comment(lib, "Advapi32.lib")
#include <stddef.h>
#include <stdio.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")
#include <conio.h>
#include <locale.h>
#include <tchar.h>
#include <time.h>
#if 0
#include <TlHelp32.h>
#endif
#include <windows.h>
#ifdef UNICODE
#include <fcntl.h>
#include <io.h>
#include <Shlobj.h>
#endif

// extract cab file
#include <setupapi.h>
#pragma comment (lib, "setupapi.lib")

// XML
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>
#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

// SPTI(needs Windows Driver Kit(wdk))
#include <ntddcdrm.h> // inc\api
#include <ntddcdvd.h> // inc\api
#include <ntddmmc.h> // inc\api
#include <ntddscsi.h> // inc\api
#define _NTSCSI_USER_MODE_
#include <scsi.h> // inc\ddk
#undef _NTSCSI_USER_MODE_

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#pragma warning(pop)
