#include "Windows.h"
#include "stdio.h"
#include "sys\stat.h"
#include "winutil.h"
#include <time.h>
#include <direct.h>
#include <mbstring.h>

#define IsKeyDown(key)  (GetAsyncKeyState(key) & 0x80000000)
#define WasKeyDown(key) (GetAsyncKeyState(key) & 0x00000001)
#define WisKeyDown(key) (GetAsyncKeyState(key) & 0x80000001)

BOOL CWinUtil::GetMoviesPath (char* strPath)
{
	char chDrive   = 'A';
	strPath[0] = '\0';

	char strTemp[256];
	if (_getcwd (strTemp, 255))
	{
		char strFile[270];
		if (strTemp[_mbstrlen(strTemp) - 1] != '\\') _mbscat((unsigned char*)strTemp, (const unsigned char*)"\\");
		_mbscpy((unsigned char*)strFile, (const unsigned char*)strTemp);
		_mbscat((unsigned char*)strFile, (const unsigned char*)"intro.smk");

		if (FileExist (strFile))
		{
			_mbscpy((unsigned char*)strPath, (const unsigned char*)strTemp);
			return TRUE;
		}
	}

	while (chDrive <= 'Z')
	{
		sprintf(strPath, "%c:\\", chDrive);
			
		if (GetDriveType(strPath) == DRIVE_CDROM)
		{
			_mbscat((unsigned char*)strPath, (const unsigned char*)"Movies\\");
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
	if (strPath[_mbstrlen(strPath) - 1] == '\\')
	{
		strPath[_mbstrlen(strPath) - 1] = '\0';
		bRemovedBackSlash = TRUE;
	}

	UINT oldErrorMode = SetErrorMode (SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	struct stat statbuf;
	int error = stat (strPath, &statbuf);
	SetErrorMode (oldErrorMode);
	if (error != -1) bDirExists = TRUE;

	if (bRemovedBackSlash)
	{
		strPath[_mbstrlen(strPath)] = '\\';
	}

	return bDirExists;
}

BOOL CWinUtil::CreateDir (char* strPath)
{
	if (DirExist (strPath)) return TRUE;
	if (strPath[_mbstrlen(strPath) - 1] == ':') return FALSE;		// special case

	char strPartialPath[MAX_PATH];
	strPartialPath[0] = '\0';

	char* token = strtok (strPath, "\\");
	while (token)
	{
		_mbscat((unsigned char*)strPartialPath, (const unsigned char*)token);
		if (!DirExist (strPartialPath) && strPartialPath[_mbstrlen(strPartialPath) - 1] != ':')
		{
			if (!CreateDirectory (strPartialPath, NULL)) return FALSE;
		}
		_mbscat((unsigned char*)strPartialPath, (const unsigned char*)"\\");
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
	FILE* pFile = fopen ("c:\\B2Debug.txt", "a+t");
	if (!pFile) return;

	time_t seconds;
	time (&seconds);
	struct tm* timedate = localtime (&seconds);
	if (!timedate) return;
	
	char strTimeDate[128];
	sprintf (strTimeDate, "[%02d/%02d/%02d %02d:%02d:%02d]  ", timedate->tm_mon + 1, timedate->tm_mday, (timedate->tm_year + 1900) % 100, timedate->tm_hour, timedate->tm_min, timedate->tm_sec);
	fwrite (strTimeDate, _mbstrlen(strTimeDate), 1, pFile);

	fwrite (strText, _mbstrlen(strText), 1, pFile);
	fwrite ("\n", 1, 1, pFile);

	fclose (pFile);
}

char CWinUtil::GetCdRomDriveWithGame()
{
	// Try to find a CD ROM drive with the game EXE on it...

	char chDrive   = 'A';
	char sDir[256];

	while (chDrive <= 'Z')
	{
		sprintf(sDir, "%c:\\", chDrive);
			
		if (GetDriveType(sDir) == DRIVE_CDROM)
		{
			char sBuf[256];
			sprintf(sBuf, "%c:\\GAME\\BLOOD2.EXE", chDrive);

			if (FileExist(sBuf))
			{
				return(chDrive);
			}
		}

		chDrive++;
	}


	// If we get here, we didn't find one...

	return(0);
}

void CWinUtil::TimedKeyWait(int nKey1, int nKey2, int nKey3, float fWaitTime)
{
	float fStartTime = GetTime();
	float fEndTime   = fStartTime + fWaitTime;

	while (GetTime() < fEndTime)
	{
		if (nKey1 > 0 && IsKeyDown(nKey1)) return;
		if (nKey2 > 0 && IsKeyDown(nKey2)) return;
		if (nKey3 > 0 && IsKeyDown(nKey3)) return;
	}
}

void CWinUtil::RemoveMessages(int nMsg, int nMax)
{
	HWND hWnd = GetActiveWindow();
	if (!hWnd) return;

	MSG msg;

	for (int i = 0; i < nMax; i++)
	{
		if (!PeekMessage(&msg, hWnd, nMsg, nMsg, PM_REMOVE))
		{
			return;
		}
	}
}

void CWinUtil::RemoveKeyDownMessages(int nMax)
{
	HWND hWnd = GetActiveWindow();
	if (!hWnd) return;

	MSG msg;

	for (int i = 0; i < nMax; i++)
	{
		if (!PeekMessage(&msg, hWnd, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE))
		{
			return;
		}
	}
}

void CWinUtil::Sleep(DWORD dwSleep)
{
	::Sleep(dwSleep);
}





