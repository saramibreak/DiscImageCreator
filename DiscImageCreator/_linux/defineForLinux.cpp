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
	char* CopyOfPath = (char*)Path;
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
	if (fp != NULL) fclose(fp);
	return (fp != NULL);
}

// http://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix
int MakeSureDirectoryPathExists(const char *dir)
{
	char tmp[256];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;
	for (p = tmp + 1; *p; p++)
		if (*p == '/') {
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	mkdir(tmp, S_IRWXU);
	return true;
}

int PathRemoveFileSpec(char* path)
{
	size_t length = strlen(path);
	for (size_t j = length - 1; j > 0; j--) {
		if (path[j] == '/') {
			path[j] = '\0';
			break;
		}
	}
	return 1;
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
	ssize_t v = readlink("/proc/self/exe", path, size);
	if (v == -1) {
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


off_t SetFilePointerEx(int fd, LARGE_INTEGER pos, void* a, int origin)
{
	UNREFERENCED_PARAMETER(a);
	off64_t ofs = pos.QuadPart;
	return lseek64(fd, ofs, origin);
}

unsigned int Sleep(unsigned long seconds)
{
	return sleep((unsigned int)seconds / 1000);
}
