#ifndef __WINUTIL_H
#define __WINUTIL_H

#ifndef BOOL
	typedef int		    BOOL;
	#define FALSE		    0
	#define TRUE		    1
#endif

#ifndef DWORD
	typedef unsigned long	DWORD;
#endif

class CWinUtil
{
public:

	static bool DirExist (char const* strPath);
	static bool CreateDir (char const* strPath);
	static bool FileExist (char const* strPath);

	static bool	CopyDir( char const* pSrc, char const* pDest );
	static bool	EmptyDir( char const* pDir );
	static bool RemoveDir( char const* pDir );

	static DWORD WinGetPrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, DWORD nSize, const char* lpFileName);
	static DWORD WinWritePrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpString, const char* lpFileName);

	static DWORD WinGetPrivateProfileString (const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpDefault, wchar_t* lpReturnedString, DWORD nSize, const wchar_t* lpFileName);
	static DWORD WinWritePrivateProfileString (const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpString, const wchar_t* lpFileName);

	static void DebugOut (char const* str);
	static float GetTime();
};

#endif
