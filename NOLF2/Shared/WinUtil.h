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

	static BOOL GetMoviesPath (char* strPath);
	static BOOL DirExist (char const* strPath);
	static BOOL CreateDir (char const* strPath);
	static BOOL FileExist (char const* strPath);

	static BOOL	CopyDir( char const* pSrc, char const* pDest );
	static BOOL	EmptyDir( char const* pDir );
	static BOOL RemoveDir( char const* pDir );

	static DWORD WinGetPrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, DWORD nSize, const char* lpFileName);
	static DWORD WinWritePrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpString, const char* lpFileName);

	static void DebugOut (char const* str);
	static void DebugBreak();
	static float GetTime();

	static char* GetFocusWindow();

	static void WriteToDebugFile (char const* strText);
};

#endif
