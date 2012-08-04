#include "Stdafx.h"
#include "stdio.h"
#include "sys/stat.h"
#include "WinUtil.h"
#include <time.h>

#if !defined(PLATFORM_LINUX)
#include <direct.h>
#include <IO.h>
#else
#include </usr/include/unistd.h>
#endif

#include "ltbasedefs.h"
#include "CommonUtilities.h"
#include "ltfileoperations.h"

bool CWinUtil::DirExist (char const* strPath)
{
	if (!strPath || !*strPath) return false;

	bool bDirExists = false;

	char szPath[MAX_PATH + 1];
	LTStrCpy( szPath, strPath, LTARRAYSIZE(szPath) );

	//if (szPath[LTStrLen(szPath) - 1] == '\\')
	if (LTStrCmp(&szPath[LTStrLen(szPath) - 1], FILE_PATH_SEPARATOR) == 0)
	{
		szPath[LTStrLen(szPath) - 1] = '\0';
	}

#if !defined(PLATFORM_XENON) && !defined(PLATFORM_LINUX)
	UINT oldErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
#endif // !PLATFORM_XENON
	struct stat statbuf;
	int error = stat (szPath, &statbuf);
#if !defined(PLATFORM_XENON)  && !defined(PLATFORM_LINUX)
	SetErrorMode (oldErrorMode);
#endif // !PLATFORM_XENON
	if (error != -1) bDirExists = true;

	return bDirExists;
}

bool CWinUtil::CreateDir (char const* strPath)
{
	if (DirExist (strPath)) return true;
	if (strPath[LTStrLen(strPath) - 1] == ':') return false;		// special case

	char strPartialPath[MAX_PATH + 1];
	strPartialPath[0] = '\0';

	// Make a copy of the input since we're going to change it.
	char szPath[MAX_PATH + 1];
	LTStrCpy( szPath, strPath, LTARRAYSIZE(szPath) );

	char* token = strtok (szPath, FILE_PATH_SEPARATOR);
	while (token)
	{
		LTStrCat (strPartialPath, token, LTARRAYSIZE(strPartialPath));
		if (!DirExist (strPartialPath) && strPartialPath[LTStrLen(strPartialPath) - 1] != ':')
		{
#if defined(PLATFORM_LINUX)
			mkdir(strPartialPath, 0);
#else
			if (!CreateDirectoryA (strPartialPath, NULL)) return false;	
#endif
		}
		LTStrCat (strPartialPath, FILE_PATH_SEPARATOR, LTARRAYSIZE(strPartialPath));
		token = strtok (NULL, FILE_PATH_SEPARATOR);
	}

	return true;
}

bool CWinUtil::FileExist (char const* strPath)
{
#if defined(PLATFORM_LINUX)
	struct stat statbuf;
	int error = stat (strPath, &statbuf);
	return error == 0 ? true : false;
#else
	return (INVALID_FILE_ATTRIBUTES != ::GetFileAttributesA(strPath));
#endif
}

bool CWinUtil::CopyDir( char const* pSrc, char const* pDest )
{
	if( !pSrc || !pDest ) return false;

	if( !DirExist( pSrc )) return false;

	// CreateDir does not preserve the path...
	char szDir[MAX_PATH] = {0};
	LTStrCpy( szDir, pDest, LTARRAYSIZE(szDir) );

	if( !DirExist( szDir ))
		CreateDir( szDir );

	char szDestFile[MAX_PATH] = {0};
	char szFiles[MAX_PATH] = {0};
	LTSNPrintF(szFiles, LTARRAYSIZE(szFiles), "%s%s*.*", pSrc, FILE_PATH_SEPARATOR);

	LTFINDFILEINFO file;
	LTFINDFILEHANDLE hFile;
	StringSet fileList;

	// find first file
	if (LTFileOperations::FindFirst(szFiles, hFile, &file))
	{
		do
		{
			LTSNPrintF( szFiles, LTARRAYSIZE(szFiles), "%s%s%s", pSrc, FILE_PATH_SEPARATOR, file.name );
			if( FileExist( szFiles ))
			{
				// Don't include subdirs.
				if (!file.bIsSubdir)
					fileList.insert(file.name);
			}
		}
		while (LTFileOperations::FindNext(hFile, &file));

		LTFileOperations::FindClose(hFile);
	}

	StringSet::iterator iter = fileList.begin();
	while (iter != fileList.end())
	{
		LTSNPrintF(szFiles, LTARRAYSIZE(szFiles), "%s%s%s",pSrc, FILE_PATH_SEPARATOR, iter->c_str());
		LTSNPrintF(szDestFile, LTARRAYSIZE(szDestFile), "%s%s%s", pDest, FILE_PATH_SEPARATOR, iter->c_str());

#if !defined(PLATFORM_LINUX)
		if( !CopyFileA( szFiles, szDestFile, false ))
			return false;
#endif

		iter++;
	}
	
	return true;
}

bool CWinUtil::EmptyDir( char const* pDir )
{
	if( !pDir ) return false;

	if( !DirExist( pDir )) return false;

	char szFiles[MAX_PATH] = {0};
	LTSNPrintF( szFiles, LTARRAYSIZE( szFiles ), "%s%s*", pDir, FILE_PATH_SEPARATOR);

	LTFINDFILEINFO file;
	LTFINDFILEHANDLE hFile;
	StringSet fileList;
	StringSet dirList;

	// find first file...
	if (LTFileOperations::FindFirst(szFiles, hFile, &file))
	{
		do
		{
			if (file.bIsSubdir)
			{
				if (!LTStrEquals(file.name,".") && !LTStrEquals(file.name,".."))
				{
					LTSNPrintF( szFiles, LTARRAYSIZE( szFiles ), "%s%s%s", pDir, FILE_PATH_SEPARATOR, file.name );
					dirList.insert( szFiles );
				}
			}
			else
			{
				LTSNPrintF( szFiles, LTARRAYSIZE( szFiles ), "%s%s%s", pDir, FILE_PATH_SEPARATOR, file.name );
				fileList.insert( szFiles );
			}
		}
		while (LTFileOperations::FindNext(hFile, &file));
		LTFileOperations::FindClose(hFile);
	}

	// Remove each file...

	StringSet::iterator iter = fileList.begin();
	while( iter != fileList.end() )
	{
		remove( iter->c_str() );
		iter++;
	}

	iter = dirList.begin();
	while( iter != dirList.end() )
	{
		RemoveDir( iter->c_str() );
		iter++;
	}

	return true;
}

bool CWinUtil::RemoveDir( char const* pDir )
{
	if( !pDir ) return false;

	// Make sure it exists and is empty...

	if( !DirExist( pDir )) return false;
	if( !EmptyDir( pDir )) return false;

	// Ok, delete it...
#if defined(PLATFORM_LINUX)
	rmdir( pDir );
#else
	_rmdir( pDir );
#endif 

	return true;
}

DWORD CWinUtil::WinGetPrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, DWORD nSize, const char* lpFileName)
{
// XENON: Currently disabled in Xenon builds.  Always returns the default string
#if defined(PLATFORM_XENON) || defined(PLATFORM_LINUX)
	// Use the default string on Xenon
	if (lpDefault == NULL)
		lpDefault = "";
	LTStrCpy(lpReturnedString, lpDefault, nSize);
	return LTStrLen(lpReturnedString);
#else // !PLATFORM_XENON
	return GetPrivateProfileStringA (lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
#endif // !PLATFORM_XENON
}

DWORD CWinUtil::WinWritePrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpString, const char* lpFileName)
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON) && !defined(PLATFORM_LINUX)
	return WritePrivateProfileStringA (lpAppName, lpKeyName, lpString, lpFileName);
#else // PLATFORM_XENON
	return (lpString != NULL) ? LTStrLen(lpString) : 0;
#endif // PLATFORM_XENON
}

DWORD CWinUtil::WinGetPrivateProfileString (const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpDefault, wchar_t* lpReturnedString, DWORD nSize, const wchar_t* lpFileName)
{
	// XENON: Currently disabled in Xenon builds.  Always returns the default string
#if defined(PLATFORM_XENON) || defined(PLATFORM_LINUX)
	// Use the default string on Xenon
	if (lpDefault == NULL)
		lpDefault = L"";
	LTStrCpy(lpReturnedString, lpDefault, nSize);
	return LTStrLen(lpReturnedString);
#else // !PLATFORM_XENON
	return GetPrivateProfileStringW (lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
#endif // !PLATFORM_XENON
}

DWORD CWinUtil::WinWritePrivateProfileString (const wchar_t* lpAppName, const wchar_t* lpKeyName, const wchar_t* lpString, const wchar_t* lpFileName)
{
	// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON) &&  !defined(PLATFORM_LINUX)
	return WritePrivateProfileStringW (lpAppName, lpKeyName, lpString, lpFileName);
#else // PLATFORM_XENON
	return (lpString != NULL) ? LTStrLen(lpString) : 0;
#endif // PLATFORM_XENON
}

#if !defined(PLATFORM_LINUX)
void CWinUtil::DebugOut (char const* str)
{
	OutputDebugStringA (str);
}

float CWinUtil::GetTime()
{
	return (float)GetTickCount() / 1000.0f;
}
#endif
