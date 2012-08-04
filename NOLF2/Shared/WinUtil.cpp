#include "stdafx.h"
#include "stdio.h"
#include "sys\stat.h"
#include "winutil.h"
#include <time.h>
#include <direct.h>
#include <IO.h>
#include "ltbasedefs.h"

BOOL CWinUtil::GetMoviesPath (char* strPath)
{
	char chDrive   = 'A';
	strPath[0] = '\0';

	char strTemp[256];
	if (_getcwd (strTemp, 255))
	{
		char strFile[270];
		if (strTemp[strlen(strTemp) - 1] != '\\') strcat (strTemp, "\\");
		SAFE_STRCPY(strFile, strTemp);
		strcat (strFile, "intro.smk");

		if (FileExist (strFile))
		{
			SAFE_STRCPY(strPath, strTemp);
			return TRUE;
		}
	}

	while (chDrive <= 'Z')
	{
		sprintf(strPath, "%c:\\", chDrive);

		if (GetDriveType(strPath) == DRIVE_CDROM)
		{
			strcat(strPath, "Movies\\");
			if (DirExist (strPath)) return TRUE;
		}

		chDrive++;
	}

	strPath[0] = '\0';

	return FALSE;
}

BOOL CWinUtil::DirExist (char const* strPath)
{
	if (!strPath || !*strPath) return FALSE;

	BOOL bDirExists = FALSE;

	char szPath[MAX_PATH];
	SAFE_STRCPY( szPath, strPath );

	if (szPath[strlen(szPath) - 1] == '\\')
	{
		szPath[strlen(szPath) - 1] = '\0';
	}

	UINT oldErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	struct stat statbuf;
	int error = stat (szPath, &statbuf);
	SetErrorMode (oldErrorMode);
	if (error != -1) bDirExists = TRUE;

	return bDirExists;
}

BOOL CWinUtil::CreateDir (char const* strPath)
{
	if (DirExist (strPath)) return TRUE;
	if (strPath[strlen(strPath) - 1] == ':') return FALSE;		// special case

	char strPartialPath[MAX_PATH];
	strPartialPath[0] = '\0';

	// Make a copy of the input since we're going to change it.
	char szPath[MAX_PATH];
	SAFE_STRCPY( szPath, strPath );

	char* token = strtok (szPath, "\\");
	while (token)
	{
		strcat (strPartialPath, token);
		if (!DirExist (strPartialPath) && strPartialPath[strlen(strPartialPath) - 1] != ':')
		{
			if (!CreateDirectory (strPartialPath, NULL)) return FALSE;
		}
		strcat (strPartialPath, "\\");
		token = strtok (NULL, "\\");
	}

	return TRUE;
}

BOOL CWinUtil::FileExist (char const* strPath)
{
	OFSTRUCT ofs;
	HFILE hFile = OpenFile (strPath, &ofs, OF_EXIST);
	if (hFile == HFILE_ERROR) return FALSE;

	return TRUE;
}

BOOL CWinUtil::CopyDir( char const* pSrc, char const* pDest )
{
	if( !pSrc || !pDest ) return FALSE;

	if( !DirExist( pSrc )) return FALSE;

	// CreateDir does not preserve the path...
	char szDir[MAX_PATH] = {0};
	strcpy( szDir, pDest );

	if( !DirExist( szDir ))
		CreateDir( szDir );

	char szDestFile[MAX_PATH] = {0};
	char szFiles[MAX_PATH] = {0};
	sprintf(szFiles, "%s\\*.*", pSrc);

	struct _finddata_t file;
	long hFile;
	StringSet fileList;

	// find first file
	if((hFile = _findfirst(szFiles, &file)) != -1L)
	{
		do
		{
			sprintf( szFiles, "%s\\%s", pSrc, file.name );
			if( FileExist( szFiles ))
				fileList.insert(file.name);
		}
		while(_findnext(hFile, &file) == 0);
	}
	_findclose(hFile);

	StringSet::iterator iter = fileList.begin();
	while (iter != fileList.end())
	{
		sprintf(szFiles,"%s\\%s",pSrc,iter->c_str());
		sprintf(szDestFile, "%s\\%s", pDest, iter->c_str());
		
		if( !CopyFile( szFiles, szDestFile, FALSE ))
			return FALSE;

		iter++;
	}
	
	return TRUE;
}

BOOL CWinUtil::EmptyDir( char const* pDir )
{
	if( !pDir ) return FALSE;

	if( !DirExist( pDir )) return FALSE;

	char szDelFile[MAX_PATH] = {0};
	char szFiles[MAX_PATH] = {0};
	sprintf( szFiles, "%s\\*.*", pDir);

	struct _finddata_t file;
	long hFile;
	StringSet fileList;

	// find first file...
	if( (hFile = _findfirst( szFiles, &file )) != -1L)
	{
		do
		{
			sprintf( szFiles, "%s\\%s", pDir, file.name );
			fileList.insert( szFiles );
		}
		while (_findnext( hFile, &file ) == 0 );
	}
	_findclose( hFile );

	// Remove each file...

	StringSet::iterator iter = fileList.begin();
	while( iter != fileList.end() )
	{
		remove( iter->c_str() );
		iter++;
	}

	return TRUE;
}

BOOL CWinUtil::RemoveDir( char const* pDir )
{
	if( !pDir ) return FALSE;

	// Make sure it exists and is empty...

	if( !DirExist( pDir )) return FALSE;
	if( !EmptyDir( pDir )) return FALSE;

	// Ok, delete it...

	_rmdir( pDir );

	return TRUE;
}

DWORD CWinUtil::WinGetPrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, DWORD nSize, const char* lpFileName)
{
	return GetPrivateProfileString (lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
}

DWORD CWinUtil::WinWritePrivateProfileString (const char* lpAppName, const char* lpKeyName, const char* lpString, const char* lpFileName)
{
	return WritePrivateProfileString (lpAppName, lpKeyName, lpString, lpFileName);
}

void CWinUtil::DebugOut (char const* str)
{
	OutputDebugString (str);
}

void CWinUtil::DebugBreak()
{
	::DebugBreak();
}

float CWinUtil::GetTime()
{
	return (float)GetTickCount() / 1000.0f;
}

char* CWinUtil::GetFocusWindow()
{
	static char strText[128];

	HWND hWnd = GetFocus();
	if (!hWnd)
	{
		hWnd = GetForegroundWindow();
		if (!hWnd) return NULL;
	}

	GetWindowText (hWnd, strText, 127);
	return strText;
}

void CWinUtil::WriteToDebugFile (char const* strText)
{
	FILE* pFile = fopen ("c:\\shodebug.txt", "a+t");
	if (!pFile) return;

	time_t seconds;
	time (&seconds);
	struct tm* timedate = localtime (&seconds);
	if (!timedate) return;

	char strTimeDate[128];
	sprintf (strTimeDate, "[%02d/%02d/%02d %02d:%02d:%02d]  ", timedate->tm_mon + 1, timedate->tm_mday, (timedate->tm_year + 1900) % 100, timedate->tm_hour, timedate->tm_min, timedate->tm_sec);
	fwrite (strTimeDate, strlen(strTimeDate), 1, pFile);

	fwrite (strText, strlen(strText), 1, pFile);
	fwrite ("\n", 1, 1, pFile);

	fclose (pFile);
}
