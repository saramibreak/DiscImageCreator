#pragma once

#ifdef _WIN32
#ifdef UNICODE
#define _tcscasestr strcasestrW
#else
#define _tcscasestr strcasestr
#endif

char* strcasestr(const char* haystack, const char* needle);
TCHAR* strcasestrW(const TCHAR* haystack, const TCHAR* needle);
#endif
