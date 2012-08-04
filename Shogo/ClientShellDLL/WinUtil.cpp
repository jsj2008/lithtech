#include "Windows.h"
#include "stdio.h"
#include "sys\stat.h"
#include "winutil.h"
#include <time.h>
#include <direct.h>
#include "clientheaders.h"

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

BOOL CWinUtil::DirExist (char* strPath)
{
	if (!strPath || !*strPath) return FALSE;

	BOOL bDirExists = FALSE;

	BOOL bRemovedBackSlash = FALSE;
	if (strPath[strlen(strPath) - 1] == '\\')
	{
		strPath[strlen(strPath) - 1] = '\0';
		bRemovedBackSlash = TRUE;
	}

	UINT oldErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	struct stat statbuf;
	int error = stat (strPath, &statbuf);
	SetErrorMode (oldErrorMode);
	if (error != -1) bDirExists = TRUE;

	if (bRemovedBackSlash)
	{
		strPath[strlen(strPath)] = '\\';
	}

	return bDirExists;
}

BOOL CWinUtil::CreateDir (char* strPath)
{
	if (DirExist (strPath)) return TRUE;
	if (strPath[strlen(strPath) - 1] == ':') return FALSE;		// special case

	char strPartialPath[MAX_PATH];
	strPartialPath[0] = '\0';

	char* token = strtok (strPath, "\\");
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

BOOL CWinUtil::FileExist (char* strPath)
{
	OFSTRUCT ofs;
	HFILE hFile = OpenFile (strPath, &ofs, OF_EXIST);
	if (hFile == HFILE_ERROR) return FALSE;

	return TRUE;
}

DWORD CWinUtil::WinGetPrivateProfileString (char* lpAppName, char* lpKeyName, char* lpDefault, char* lpReturnedString, DWORD nSize, char* lpFileName)
{
	return GetPrivateProfileString (lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
}

DWORD CWinUtil::WinWritePrivateProfileString (char* lpAppName, char* lpKeyName, char* lpString, char* lpFileName)
{
	return WritePrivateProfileString (lpAppName, lpKeyName, lpString, lpFileName);
}

void CWinUtil::DebugOut (char* str)
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

void CWinUtil::WriteToDebugFile (char *strText)
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

