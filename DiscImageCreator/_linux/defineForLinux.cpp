#if defined(__linux__)
#include "defineForLinux.h"
#endif

int Beep(int fz, int time)
{
	UNREFERENCED_PARAMETER(fz);
	UNREFERENCED_PARAMETER(time);
	return TRUE;
}

int GetCurrentDirectory(size_t size, char *buf)
{
	if (!getcwd(buf, size)) {
		return FALSE;
	}
	return TRUE;
}

static size_t build_temp_path_narrow(char *buf, size_t bufSize)
{
    const char *tmp = getenv("TMPDIR");
    struct stat st;

    if (!tmp || !*tmp ||
        stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode)) {
        tmp = "/tmp";
    }

    size_t len = strlen(tmp);
    int need_slash = (len > 0 && tmp[len - 1] != '/');

    size_t required = len + (need_slash ? 1 : 0) + 1;
    if (bufSize == 0) {
        return 0;
    }

    if (required > bufSize) {
        if (len >= bufSize) {
            len = bufSize - 1;
            need_slash = 0;
        } else if (len + 1 >= bufSize) {
            need_slash = 0;
        }
    }

    memcpy(buf, tmp, len);
    size_t pos = len;

    if (need_slash && pos < bufSize - 1) {
        buf[pos++] = '/';
    }

    buf[pos] = '\0';
    return pos;
}

DWORD GetTempPathA(DWORD nBufferLength, char *lpBuffer)
{
    char tmpBuf[PATH_MAX];
    size_t len = build_temp_path_narrow(tmpBuf, sizeof(tmpBuf));
    if (len == 0) {
        return 0;
    }

    size_t required = len + 1;

    if (!lpBuffer || nBufferLength == 0) {
        return (DWORD)required;
    }

    if (nBufferLength < required) {
        size_t copy_len = nBufferLength - 1;
        memcpy(lpBuffer, tmpBuf, copy_len);
        lpBuffer[copy_len] = '\0';
        return (DWORD)required;
    }

    memcpy(lpBuffer, tmpBuf, len + 1);
    return (DWORD)len;
}

DWORD GetTempPathW(DWORD nBufferLength, wchar_t *lpBuffer)
{
    char tmpBuf[PATH_MAX];
    size_t lenNarrow = build_temp_path_narrow(tmpBuf, sizeof(tmpBuf));
    if (lenNarrow == 0) {
        return 0;
    }

    size_t requiredCharsNoNull = mbstowcs(NULL, tmpBuf, 0);
    if (requiredCharsNoNull == (size_t)-1) {
        return 0;
    }
    size_t required = requiredCharsNoNull + 1;

    if (!lpBuffer || nBufferLength == 0) {
        return (DWORD)required;
    }

    if (nBufferLength < required) {
        size_t toConvert = nBufferLength - 1;
        size_t written = mbstowcs(lpBuffer, tmpBuf, toConvert);
        if (written == (size_t)-1) {
            return 0;
        }
        lpBuffer[written] = L'\0';
        return (DWORD)required;
    }

    size_t written = mbstowcs(lpBuffer, tmpBuf, required);
    if (written == (size_t)-1) {
        return 0;
    }
    lpBuffer[written] = L'\0';
    return (DWORD)written;
}

DWORD GetTempPath(DWORD nBufferLength, LPTSTR lpBuffer)
{
#ifdef _UNICODE
    return GetTempPathW(nBufferLength, lpBuffer);
#else
    return GetTempPathA(nBufferLength, lpBuffer);
#endif
}

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

void _splitpath(const char* Path, char* Drive, char* Directory, char* Filename, char* Extension)
{
	const char* CopyOfPath = Path;
	int Counter = 0;
	int Last = 0;
	int Rest = 0;

	// no drives available in linux .
	// extensions are not common in linux
	// but considered anyway
	if (Drive != NULL) {
		if (*CopyOfPath == '/') {
			strncpy(Drive, Path, 1);
			CopyOfPath++;
			Counter++;
		}
		else {
			Drive = NULL;
		}
	}

	while (*CopyOfPath != '\0')
	{
		// search for the last slash
		while (*CopyOfPath != '/' && *CopyOfPath != '\0')
		{
			CopyOfPath++;
			Counter++;
		}
		if (*CopyOfPath == '/')
		{
			CopyOfPath++;
			Counter++;
			Last = Counter;
		}
		else
			Rest = Counter - Last;
	}
	if (Directory != NULL) {
		// directory is the first part of the path until the
		// last slash appears
		if (Drive) {
			strncpy(Directory, Path + 1, Last - 1);
		}
		else {
			strncpy(Directory, Path, Last);
		}
		// strncpy doesnt add a '\0'
		Directory[Last] = '\0';
	}
	char ext[256] = { 0 };
	char* pExt = ext;
	if (Filename != NULL) {
		// Filename is the part behind the last slahs
		strcpy(Filename, CopyOfPath -= Rest);
		strncpy(pExt, Filename, sizeof(ext) - 1);
		PathRemoveExtension(Filename);
	}
	if (Extension != NULL) {
		// get extension if there is any
		while (*pExt != '\0')
		{
			// the part behind the point is called extension in windows systems
			// at least that is what i thought apperantly the '.' is used as part
			// of the extension too .
			if (*pExt == '.')
			{
				while (*pExt != '\0')
				{
//					*Extension = *Filename;
					strncpy(Extension, pExt, sizeof(ext));
//					*Filename = '\0';
					break;
//					Extension++;
//					Filename++;
				}
			}
			if (*pExt != '\0')
			{
				pExt++;
			}
		}
//		*Extension = '\0';
	}
	return;
}

// Abstract:   Make a path out of its parts
// Parameters: Path: Object to be made
//             Drive: Logical drive , only for compatibility , not considered
//             Directory: Directory part of path
//             Filename: File part of path
//             Extension: Extension part of path (includes the leading point)
// Returns:    Path is changed
// Comment:    Note that the concept of an extension is not available in Linux,
//             nevertheless it is considered

void _makepath(char* Path, const char* Drive, const char* Directory,
	const char* File, const char* Extension)
{
	while (*Drive != '\0' && Drive != NULL)
	{
		*Path = *Drive;
		Path++;
		Drive++;
	}
	while (*Directory != '\0' && Directory != NULL)
	{
		*Path = *Directory;
		Path++;
		Directory++;
	}
	while (*File != '\0' && File != NULL)
	{
		*Path = *File;
		Path++;
		File++;
	}
	while (*Extension != '\0' && Extension != NULL)
	{
		*Path = *Extension;
		Path++;
		Extension++;
	}
	*Path = '\0';
	return;
}

int PathSet(char* path, char const* fullpath)
{
	strcpy(path, fullpath);

	return 1; /* not sure when this function would 'fail' */
}

// http://stackoverflow.com/questions/3218201/find-a-replacement-for-windows-pathappend-on-gnu-linux
int PathAppend(char* path, char const* more)
{
	size_t pathlen = strlen(path);

	while (*more == '/') {
		/* skip path separators at start of `more` */
		++more;
	}

	/*
	* if there's anything to add to the path, make sure there's
	* a path separator at the end of it
	*/

	if (*more && (pathlen > 0) && (path[pathlen - 1] != '/')) {
		strcat(path, "/");
	}

	strcat(path, more);

	return 1; /* not sure when this function would 'fail' */
}

// https://www.quora.com/How-do-I-check-if-a-file-already-exists-using-C-file-I-O
int PathFileExists(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp != NULL) {
		fclose(fp);
	}
	return (fp != NULL);
}

int parse_uid_gid_from_sudo(uid_t *out_uid, gid_t *out_gid)
{
	const char *su = getenv("SUDO_UID");
	const char *sg = getenv("SUDO_GID");
	if (!su || !sg) {
		return 0;
	}

    char *end = NULL;
    unsigned long u = strtoul(su, &end, 10);
	if (!end || *end != '\0') {
		return 0;
	}

    end = NULL;
    unsigned long g = strtoul(sg, &end, 10);
	if (!end || *end != '\0') {
		return 0;
	}

    *out_uid = (uid_t)u;
    *out_gid = (gid_t)g;
    return 1;
}

int MakeSureDirectoryPathExists(const char *dir)
{
	char tmp[PATH_MAX];
	char* p = NULL;
	size_t len;

	if (!dir) {
		errno = EINVAL;
		return 0;
	}
	if (strlen(dir) >= sizeof(tmp)) {
		errno = ENAMETOOLONG;
		return 0;
	}
	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	if (len == 0) {
		errno = EINVAL;
		return 0;
	}
	if (tmp[len - 1] == '/' && len > 1) {
		tmp[len - 1] = '\0';
	}
	uid_t target_uid = (uid_t)-1;
	gid_t target_gid = (gid_t)-1;
	int need_chown = 0;
	if (geteuid() == 0) {
		if (parse_uid_gid_from_sudo(&target_uid, &target_gid)) {
			need_chown = 1;
		}
	}
	for (p = tmp + 1; *p; p++) {
		if (*p == '/') {
			*p = '\0';
			if (mkdir(tmp, 0755) == 0) {
				if (need_chown) {
					if (chown(tmp, target_uid, target_gid) != 0) {
						fprintf(stderr, "Failed to chown");
					}
				}
			} else if (errno != EEXIST) {
			    return 0;
			}
			*p = '/';
		}
	}
	if (mkdir(tmp, 0755) == 0) {
		if (need_chown) {
			if (chown(tmp, target_uid, target_gid) != 0) {
				fprintf(stderr, "Failed to chown");
			}
		}
	} else if (errno != EEXIST) {
	    return 0;
	}

	return 1;
}

int PathRemoveFileSpec(char* path)
{
	if (path == NULL)
		return 0;

	size_t len = strlen(path);
	if (len == 0)
		return 0;

	// root "/" is ignore
	if (len == 1 && path[0] == '/')
		return 0;

	// last '/' is skip
	size_t i = len;
	while (i > 0 && path[i - 1] == '/')
		--i;

	if (i == 0) {
		// if string is "////" , it changed to "/"
		path[0] = '/';
		path[1] = '\0';
		return 1;
	}

	// search last '/' before i
	for (size_t j = i; j > 0; --j) {
		if (path[j - 1] == '/') {
			if (j == 1) {
				// "/foo" -> "/"
				path[1] = '\0';
			}
			else {
				// "/foo/bar.txt" -> "/foo"
				// "dir/file"      -> "dir"
				path[j - 1] = '\0';
			}
			return 1;
		}
	}

	// '/' is nothing
	return 0;
}

int PathRemoveExtension(char* path)
{
	size_t length = strlen(path);
	for (size_t j = length - 1; j > 0; j--) {
		if (path[j] == '.') {
			path[j] = '\0';
			break;
		}
	}
	return 1;
}

// http://jajagacchi.jugem.jp/?eid=123
int PathRenameExtension(char* path, const char* ext)
{
	PathRemoveExtension(path);
	strncat(path, ext, strlen(path));
	return 1;
}

const char* PathFindExtensionA(const char* path)
{
	if (!path) {
		return NULL;
	}

	const char* base = path;
	for (const char* p = path; *p; ++p) {
		if (*p == '/' || *p == '\\') {
			base = p + 1;
		}
	}

	const char* last_dot = NULL;
	for (const char* p = base; *p; ++p) {
		if (*p == '.') {
			last_dot = p;
		}
	}

	if (!last_dot) {
		const char* end = base;
		while (*end) {
			++end;
		}
		return end;
	}

	return last_dot;
}

int MoveFileEx(const char* srcFile, const char* dstFile, int flag)
{
	FILE* fpR = NULL;
	FILE* fpW = NULL;
	if (flag == 0) {
		fpR = fopen(srcFile, "rb");
		fpW = fopen(dstFile, "wb");
	}
	else {
		fpR = fopen(srcFile, "r");
		fpW = fopen(dstFile, "w");
	}

	if (fpR == NULL || fpW == NULL) {
		if (fpR) {
			fclose(fpR);
		}
		if (fpW) {
			fclose(fpW);
		}
		return FALSE;
	}

	fseek(fpR, 0, SEEK_END);
	long size = ftell(fpR);
	rewind(fpR);

	unsigned char buf[2352] = { 0 };
	for (int i = 0; i < size; i += 2352) {
		size_t readsize = fread(buf, sizeof(unsigned char), sizeof(buf), fpR);
		if (readsize < sizeof(buf)) {
			fclose(fpR);
			fclose(fpW);
			return FALSE;
		}
		fwrite(buf, sizeof(unsigned char), readsize, fpW);
	}
	fclose(fpR);
	fclose(fpW);
	return TRUE;
}

int CopyFile(const char* srcFile, const char* dstFile, int flag)
{
	return MoveFileEx(srcFile, dstFile, flag);
}

int _fseeki64(FILE* fp, ULONGLONG ofs, int origin)
{
	return fseeko(fp, ofs, origin);
}

off_t _ftelli64(FILE* fp)
{
	return ftello(fp);
}

// http://pdfbjj.blogspot.com/2012/04/linuxgetmodulefilename.html
int GetModuleFileName(void* a, char* path, unsigned long size)
{
	UNREFERENCED_PARAMETER(a);
#if defined(__linux__)
	ssize_t v = readlink("/proc/self/exe", path, size);
	if (v == -1) {
#elif defined(__APPLE__) && defined(__MACH__)
	if (_NSGetExecutablePath(path, (uint32_t*)&size) != 0) {
#endif
		return FALSE;
	}
	return TRUE;
}

int WideCharToMultiByte(
	UINT CodePage,
	DWORD dwFlags,
	LPCWSTR lpWideCharStr,
	int cchWideChar,
	LPSTR lpMultiByteStr,
	int cchMultiByte,
	LPCSTR lpDefaultChar,
	LPBOOL lpUsedDefaultChar)
{
	UNREFERENCED_PARAMETER(CodePage);
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(cchMultiByte);
	UNREFERENCED_PARAMETER(lpDefaultChar);
	UNREFERENCED_PARAMETER(lpUsedDefaultChar);
	size_t ret = wcstombs(lpMultiByteStr, lpWideCharStr, cchWideChar);
	if (ret == (size_t)-1) {
		return 0;
	}
	return cchWideChar;
}

int MultiByteToWideChar(
	UINT CodePage,
	DWORD dwFlags,
	LPCSTR lpMultiByteStr,
	int cchMultiByte,
	LPWSTR lpWideCharStr,
	int cchWideChar)
{
	UNREFERENCED_PARAMETER(CodePage);
	UNREFERENCED_PARAMETER(dwFlags);
	UNREFERENCED_PARAMETER(cchWideChar);
	size_t ret = mbstowcs(lpWideCharStr, lpMultiByteStr, cchMultiByte);
	if (ret == (size_t)-1) {
		return 0;
	}
	return cchMultiByte;
}

void SetLastError(int errcode)
{
	errno = errcode;
}

int GetLastError(void)
{
	return errno;
}

#if defined(__linux__)
int CloseHandle(int fd)
{
	int ret = close(fd);
	ret = ret != -1 ? TRUE : FALSE;
	return ret;
}

int DeviceIoControl(int fd, unsigned long ioCtlCode, void* inbuf, unsigned long a, void* b, unsigned long c, unsigned long* d, void* e)
{
	UNREFERENCED_PARAMETER(a);
	UNREFERENCED_PARAMETER(b);
	UNREFERENCED_PARAMETER(c);
	UNREFERENCED_PARAMETER(d);
	UNREFERENCED_PARAMETER(e);
	PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER p = (PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)inbuf;
	if (p == NULL) {
		// This Linux shim only implements the SCSI pass-through path, which
		// requires a PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER input buffer.
		// Callers that pass no buffer (e.g. the disk-geometry ioctls used by
		// DiskGetMediaTypes for floppy media) would otherwise dereference NULL
		// and crash; report failure instead.
		SetLastError(EINVAL);
		return FALSE;
	}
	int ret = ioctl(fd, ioCtlCode, &(p->io_hdr));
	memcpy(&(p->SenseData), p->Dummy, 18);
	ret = ret != -1 ? TRUE : FALSE;
	return ret;
}

int ReadFile(int fd, void* inbuf, unsigned long size, unsigned long* d, void* e)
{
	UNREFERENCED_PARAMETER(e);
	int ret = 0;
	*d = read(fd, inbuf, size);
	ret = ret != -1 ? TRUE : FALSE;
	return ret;
}

off_t SetFilePointer(int fd, off_t pos, void* a, int origin)
{
	UNREFERENCED_PARAMETER(a);
	return lseek(fd, pos, origin);
}

off64_t SetFilePointerEx(int fd, LARGE_INTEGER pos, void* a, int origin)
{
	UNREFERENCED_PARAMETER(a);
	off64_t ofs = pos.QuadPart;
	return lseek64(fd, ofs, origin);
}
#elif defined(__APPLE__) && defined(__MACH__)
// https://developer.apple.com/library/archive/documentation/DeviceDrivers/Conceptual/WorkingWithSAM/WWS_SAMDevInt/WWS_SAM_DevInt.html#//apple_ref/doc/uid/TP30000387-SW1
IOCFPlugInInterface** plugInInterface;
MMCDeviceInterface** mmcInterface;
SCSITaskDeviceInterface** interface;

// Copies an IOKit string property, without the padding spaces it is stored with.
void CopyTrimmedProperty(CFStringRef src, char* dst, size_t dstSize)
{
	dst[0] = 0;
	if (!src || CFGetTypeID(src) != CFStringGetTypeID()
		|| !CFStringGetCString(src, dst, (CFIndex)dstSize, kCFStringEncodingUTF8)) {
		return;
	}
	size_t len = strlen(dst);
	while (len > 0 && isspace((unsigned char)dst[len - 1])) {
		dst[--len] = 0;
	}
}

// The BSD name ("disk4") is not a property of the SCSI peripheral device. It belongs to the
// IOMedia object that the block storage stack publishes below it, so it has to be searched
// for recursively - and it only exists while a disc is loaded.
void GetDriveBSDName(io_service_t service, char* dst, size_t dstSize)
{
	CFStringRef name = (CFStringRef)IORegistryEntrySearchCFProperty(service, kIOServicePlane
		, CFSTR(kIOBSDNameKey), kCFAllocatorDefault, kIORegistryIterateRecursively);
	CopyTrimmedProperty(name, dst, dstSize);
	if (name) {
		CFRelease(name);
	}
}

// Vendor and product of the drive itself ("PLEXTOR DVDR   PX-760A"). Unlike the BSD name
// this is there whether or not a disc is loaded, so it can also name an empty drive.
void GetDriveName(io_service_t service, char* dst, size_t dstSize)
{
	dst[0] = 0;
	CFDictionaryRef characteristics = (CFDictionaryRef)IORegistryEntrySearchCFProperty(
		service, kIOServicePlane, CFSTR(kIOPropertyDeviceCharacteristicsKey)
		, kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents);
	if (!characteristics) {
		return;
	}
	if (CFGetTypeID(characteristics) == CFDictionaryGetTypeID()) {
		char vendor[16] = {};
		char product[64] = {};
		CopyTrimmedProperty((CFStringRef)CFDictionaryGetValue(
			characteristics, CFSTR(kIOPropertyVendorNameKey)), vendor, sizeof(vendor));
		CopyTrimmedProperty((CFStringRef)CFDictionaryGetValue(
			characteristics, CFSTR(kIOPropertyProductNameKey)), product, sizeof(product));
		// ATA drives report no vendor, everything is in the product name.
		snprintf(dst, dstSize, "%s%s%s", vendor, vendor[0] && product[0] ? " " : "", product);
	}
	CFRelease(characteristics);
}

void OutputAvailableDrives(PAUTHORING_DEVICE pDevices, int nDeviceNum)
{
	if (nDeviceNum == 0) {
		fprintf(stderr, "No drive supports SCSITaskAuthoringDevice\n");
		return;
	}
	fprintf(stderr, "Available drive:\n");
	for (int i = 0; i < nDeviceNum; i++) {
		fprintf(stderr, "\t%-9s %s\n"
			, pDevices[i].bsdName[0] ? pDevices[i].bsdName : "(no disc)", pDevices[i].driveName);
	}
}

SCSITaskInterface** GetSCSITaskInterface(char* path)
{
	const char* requested = path;
	const char* slash = strrchr(path, '/');
	if (slash) {
		requested = slash + 1;   // "/dev/disk2" -> "disk2"
	}
	// Create the dictionaries
	CFMutableDictionaryRef matchingDict = CFDictionaryCreateMutable(
		kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,	&kCFTypeDictionaryValueCallBacks);
	CFMutableDictionaryRef subDict      = CFDictionaryCreateMutable(
		kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,	&kCFTypeDictionaryValueCallBacks);

	// Create a dictionary with the "SCSITaskDeviceCategory" key = "SCSITaskAuthoringDevice"
	CFDictionarySetValue(subDict
		, CFSTR(kIOPropertySCSITaskDeviceCategory), CFSTR(kIOPropertySCSITaskAuthoringDevice));

	// Add the dictionary to the main dictionary with the key "IOPropertyMatch" to
	// narrow the search to the above dictionary.
	CFDictionarySetValue(matchingDict, CFSTR(kIOPropertyMatchKey), subDict);

	CFRelease(subDict);

	io_iterator_t iterator = 0;
	kern_return_t err = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iterator);
	if (err != KERN_SUCCESS || !iterator) {
		fprintf(stderr, "IOServiceGetMatchingServices failed: 0x%x\n", err);
		return NULL;
	}
	// The matching dictionary already asked for SCSITaskAuthoringDevice, so everything the
	// iterator returns is one. Collect them all: the requested drive has to be picked out of
	// them, and if it is not among them the others are worth reporting.
	AUTHORING_DEVICE devices[MAX_AUTHORING_DEVICE_NUM] = {};
	int nDeviceNum = 0;
	io_service_t tmp;
	while ((tmp = IOIteratorNext(iterator))) {
		if (nDeviceNum == MAX_AUTHORING_DEVICE_NUM) {
			IOObjectRelease(tmp);
			continue;
		}
		devices[nDeviceNum].service = tmp;
		GetDriveBSDName(tmp, devices[nDeviceNum].bsdName, sizeof(devices[nDeviceNum].bsdName));
		GetDriveName(tmp, devices[nDeviceNum].driveName, sizeof(devices[nDeviceNum].driveName));
		nDeviceNum++;
	}
	IOObjectRelease(iterator);

	// A loaded disc gives the drive a BSD name, which is unambiguous. Prefer it.
	int nSelected = -1;
	for (int i = 0; i < nDeviceNum; i++) {
		if (devices[i].bsdName[0] && strcmp(devices[i].bsdName, requested) == 0) {
			nSelected = i;
			break;
		}
	}
	// An empty drive has no BSD name, so eject/close/start/stop would have nothing to
	// address it by. Match on the drive name for those, and refuse if it is not unique.
	if (nSelected == -1) {
		int nMatchNum = 0;
		for (int i = 0; i < nDeviceNum; i++) {
			if (devices[i].driveName[0] && strcasestr(devices[i].driveName, requested)) {
				nSelected = i;
				nMatchNum++;
			}
		}
		if (nMatchNum > 1) {
			fprintf(stderr, "Ambiguous drive: %s\n", requested);
			OutputAvailableDrives(devices, nDeviceNum);
			for (int i = 0; i < nDeviceNum; i++) {
				IOObjectRelease(devices[i].service);
			}
			return NULL;
		}
	}
	if (nSelected == -1) {
		fprintf(stderr, "Not found the drive: %s\n", requested);
		OutputAvailableDrives(devices, nDeviceNum);
		for (int i = 0; i < nDeviceNum; i++) {
			IOObjectRelease(devices[i].service);
		}
		return NULL;
	}

	io_service_t service = devices[nSelected].service;
	for (int i = 0; i < nDeviceNum; i++) {
		if (i != nSelected) {
			IOObjectRelease(devices[i].service);
		}
	}
	io_name_t service_class = "";
	io_string_t service_path = "";
	IOObjectGetClass(service, service_class);
	IORegistryEntryGetPath(service, kIOServicePlane, service_path);
	printf("Class: %s\nPath: %s\n", service_class, service_path);

	CFTypeRef prop = IORegistryEntryCreateCFProperty(service, CFSTR("IOCFPlugInTypes"), kCFAllocatorDefault, 0);
	if (!prop) {
	    fprintf(stderr, "Not found IOCFPlugInTypes\n");
	} else {
	    fprintf(stderr, "IOCFPlugInTypes:\n");
	    CFShow(prop);
	    CFRelease(prop);
	}

	// Unmount the drive that is about to be opened. There is nothing to unmount when it
	// holds no disc, and ObtainExclusiveAccess fails on a drive that is still mounted.
	if (devices[nSelected].bsdName[0]) {
		char unmountCmd[256];
		snprintf(unmountCmd, sizeof(unmountCmd),
			"diskutil unmountDisk %s > /dev/null", devices[nSelected].bsdName);
		printf("unmountCmd: %s\n", unmountCmd);
		if (system(unmountCmd) != 0) {
			fprintf(stderr, "Failed: system(unmountDisk)\n");
		}
	}

	SInt32        score;
	HRESULT       herr;

	// Create the IOCFPlugIn interface so we can query it.
	if (noErr != (err = IOCreatePlugInInterfaceForService(
		service, kIOMMCDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plugInInterface, &score))) {
		fprintf(stderr, "Failed: IOCreatePlugInInterfaceForService returned %x\n", err);
		return NULL;
	}
	// Query the interface for the MMCDeviceInterface.
	if (S_OK != (herr = (*plugInInterface)->QueryInterface(
		plugInInterface, CFUUIDGetUUIDBytes(kIOMMCDeviceInterfaceID), (LPVOID*)(&mmcInterface)))) {
		fprintf(stderr, "Failed: QueryInterface returned %x\n", herr);
		return NULL;
	}
	if (NULL == (interface = (*mmcInterface)->GetSCSITaskDeviceInterface(mmcInterface))) {;
		fprintf(stderr, "Failed: GetSCSITaskDeviceInterface returned NULL\n");
		return NULL;
	}
	if (noErr != (err = (*interface)->ObtainExclusiveAccess(interface))) {
		fprintf(stderr, "Failed: ObtainExclusiveAccess returned %x\n", err);
		return NULL;
	}
	// Create a task now that we have exclusive access
	return (*interface)->CreateSCSITask(interface);
}

int CloseHandle(SCSITaskInterface** task)
{
	// Release the task interface
	(*task)->Release(task);
	(*interface)->ReleaseExclusiveAccess(interface);

	// Release the SCSITaskDeviceInterface.
	(*interface)->Release(interface);
	(*mmcInterface)->Release(mmcInterface);

	IODestroyPlugInInterface(plugInInterface);
	return 1;
}

int DeviceIoControl(SCSITaskInterface** task, unsigned long aa, void* inbuf, unsigned long a, void* b, unsigned long c, unsigned long* d, void* e)
{
	UNREFERENCED_PARAMETER(aa);
	UNREFERENCED_PARAMETER(a);
	UNREFERENCED_PARAMETER(b);
	UNREFERENCED_PARAMETER(c);
	UNREFERENCED_PARAMETER(d);
	UNREFERENCED_PARAMETER(e);
	PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER p = (PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER)inbuf;
	IOReturn ret = (*task)->ExecuteTaskSync(task, &(p->tmpSenseData), &(p->taskStatus), &(p->transferCount));
	memcpy(&(p->SenseData), &(p->tmpSenseData), 18);
	ret = ret != -1 ? TRUE : FALSE;
	return ret;
}

int ReadFile(SCSITaskInterface** task, void* inbuf, unsigned long size, unsigned long* d, void* e)
{
	// TODO
	UNREFERENCED_PARAMETER(e);
	int ret = 1;
	ret = ret != -1 ? TRUE : FALSE;
	return ret;
}

off_t SetFilePointer(SCSITaskInterface** task, off_t pos, void* a, int origin)
{
	// TODO
	UNREFERENCED_PARAMETER(a);
	return 1;
}

off64_t SetFilePointerEx(SCSITaskInterface** task, LARGE_INTEGER pos, void* a, int origin)
{
	// TODO
	UNREFERENCED_PARAMETER(a);
	off64_t ofs = pos.QuadPart;
	return 1;
}
#endif

unsigned int Sleep(unsigned long seconds)
{
	return sleep((unsigned int)seconds / 1000);
}

int GetDiskFreeSpaceEx(
	LPCSTR lpDirectoryName,
	PULARGE_INTEGER lpFreeBytesAvailableToCaller,
	PULARGE_INTEGER lpTotalNumberOfBytes,
	PULARGE_INTEGER lpTotalNumberOfFreeBytes
) {
	struct statvfs64 buf;
	buf.f_bsize = 0;
	buf.f_frsize = 0;
	buf.f_blocks = 0; 
	buf.f_bfree = 0; 
	buf.f_bavail = 0; 
	buf.f_files = 0;
	buf.f_ffree = 0; 
	buf.f_favail = 0; 
	buf.f_fsid = 0; 
	buf.f_flag = 0; 
	buf.f_namemax = 0;

	int rc = statvfs64(lpDirectoryName, &buf);
	if (rc < 0) {
		fprintf(stderr, "Error: statvfs() %s: %s\n", strerror(errno), lpDirectoryName);
		return -1;
	}

	printf(
		"          Block Size: %lu\n"
		"       Flagment Size: %lu\n"
		"       All Block Num: %lu\n"
		"      Free Block Num: %lu\n"
		" Available Block Num: %lu\n"
		"          I Node Num: %lu\n"
		"     Free I Node Num: %lu\n"
		"Available I Node Num: %lu\n"
		"      File System ID: %lu\n"
		"          Mount Flag: %lu\n"
		" Max Filename Length: %lu\n"
		, buf.f_bsize, buf.f_frsize
		, buf.f_blocks, buf.f_bfree, buf.f_bavail
		, buf.f_files, buf.f_ffree, buf.f_favail
		, buf.f_fsid, buf.f_flag, buf.f_namemax
	);
	lpFreeBytesAvailableToCaller->QuadPart = buf.f_frsize * buf.f_bavail;
	lpTotalNumberOfBytes->QuadPart = buf.f_frsize * buf.f_blocks;
	lpTotalNumberOfFreeBytes->QuadPart = buf.f_frsize * buf.f_bfree;

	return 0;
}

static void split_dir_and_pattern(const char* path, char* dir, char* pat)
{
	const char* slash = strrchr(path, '/');
	if (!slash) {
		strcpy(dir, ".");
		strcpy(pat, path);
	}
	else {
		size_t dlen = (size_t)(slash - path);
		if (dlen == 0) {
			strcpy(dir, "/");
		}
		else {
			memcpy(dir, path, dlen);
			dir[dlen] = '\0';
		}
		strcpy(pat, slash + 1);
	}
}

static void fill_find_data(const char* dirpath,
	const char* name,
	WIN32_FIND_DATA* fd)
{
	char full[PATH_MAX];
	struct stat st;

	snprintf(full, sizeof(full), "%s/%s", dirpath, name);
	if (stat(full, &st) != 0) {
		fd->dwFileAttributes = 0;
		fd->nFileSize = 0;
		strncpy(fd->cFileName, name, sizeof(fd->cFileName) - 1);
		fd->cFileName[sizeof(fd->cFileName) - 1] = '\0';
		return;
	}

	DWORD attr = 0;
	if (S_ISDIR(st.st_mode)) {
		attr |= FILE_ATTRIBUTE_DIRECTORY;
	}
	if ((st.st_mode & S_IWUSR) == 0) {
		attr |= FILE_ATTRIBUTE_READONLY;
	}

	fd->dwFileAttributes = attr;
	fd->nFileSize = (LONGLONG)st.st_size;
	strncpy(fd->cFileName, name, sizeof(fd->cFileName) - 1);
	fd->cFileName[sizeof(fd->cFileName) - 1] = '\0';
}

static int has_wildcard(const char* path)
{
	return strpbrk(path, "*?[") != NULL;
}

HANDLE FindFirstFileA(const char* lpFileName, WIN32_FIND_DATA* lpFindFileData)
{
	if (!has_wildcard(lpFileName)) {
		struct stat st;
		if (stat(lpFileName, &st) != 0) {
			errno = ENOENT;
			return INVALID_HANDLE_VALUE;
		}

		FIND_HANDLE_INTERNAL* h = (FIND_HANDLE_INTERNAL*)malloc(sizeof(*h));
		if (!h) {
			errno = ENOMEM;
			return INVALID_HANDLE_VALUE;
		}

		memset(h, 0, sizeof(*h));
		h->dir = NULL;
		h->single_done = 0;

		split_dir_and_pattern(lpFileName, h->dirpath, h->pattern);

		fill_find_data(h->dirpath, h->pattern, lpFindFileData);

		return (HANDLE)h;
	}

	char dir[PATH_MAX];
	char pat[PATH_MAX];
	split_dir_and_pattern(lpFileName, dir, pat);

	DIR* d = opendir(dir);
	if (!d) {
		return INVALID_HANDLE_VALUE;
	}

	FIND_HANDLE_INTERNAL* h = (FIND_HANDLE_INTERNAL*)malloc(sizeof(*h));
	if (!h) {
		closedir(d);
		errno = ENOMEM;
		return INVALID_HANDLE_VALUE;
	}

	h->dir = d;
	strncpy(h->dirpath, dir, sizeof(h->dirpath) - 1);
	h->dirpath[sizeof(h->dirpath) - 1] = '\0';
	strncpy(h->pattern, pat, sizeof(h->pattern) - 1);
	h->pattern[sizeof(h->pattern) - 1] = '\0';

	struct dirent* ent;
	while ((ent = readdir(d)) != NULL) {
		if (fnmatch(h->pattern, ent->d_name, 0) == 0) {
			fill_find_data(h->dirpath, ent->d_name, lpFindFileData);
			return (HANDLE)h;
		}
	}

	closedir(d);
	free(h);
	errno = ENOENT;
	return INVALID_HANDLE_VALUE;
}

BOOL FindNextFileA(HANDLE hFindFile, WIN32_FIND_DATA* lpFindFileData)
{
	FIND_HANDLE_INTERNAL* h = (FIND_HANDLE_INTERNAL*)hFindFile;

	if (!h->dir) {
		if (h->single_done) {
			errno = ENOENT;
			return FALSE;
		}
		h->single_done = 1;
		errno = ENOENT;
		return FALSE;
	}

	struct dirent* ent;
	while ((ent = readdir(h->dir)) != NULL) {
		if (fnmatch(h->pattern, ent->d_name, 0) == 0) {
			fill_find_data(h->dirpath, ent->d_name, lpFindFileData);
			return TRUE;
		}
	}

	errno = ENOENT;
	return FALSE;
}

BOOL FindClose(HANDLE hFindFile)
{
	FIND_HANDLE_INTERNAL* h = (FIND_HANDLE_INTERNAL*)hFindFile;
	if (!h) return FALSE;
	if (h->dir) closedir(h->dir);
	free(h);
	return TRUE;
}

BOOL DeleteFileA(const char* lpFileName)
{
	if (unlink(lpFileName) == 0) {
		return TRUE;
	}
	return FALSE;
}

BOOL RemoveDirectoryA(const char* lpPathName)
{
	if (rmdir(lpPathName) == 0) {
		return TRUE;
	}
	return FALSE;
}

BOOL SetFileAttributesA(const char* lpFileName, DWORD dwFileAttributes)
{
	struct stat st;
	if (stat(lpFileName, &st) != 0) {
		return FALSE;
	}

	mode_t mode = st.st_mode;

	if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
		mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
	}
	else {
		mode |= S_IWUSR;
	}

	if (chmod(lpFileName, mode) != 0) {
		return FALSE;
	}
	return TRUE;
}
