#pragma once

#ifdef _WIN32
char* strcasestr(const char* haystack, const char* needle);
TCHAR* strcasestrW(const TCHAR* haystack, const TCHAR* needle);
#endif
