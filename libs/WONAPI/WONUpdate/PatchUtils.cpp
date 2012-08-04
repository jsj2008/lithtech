//----------------------------------------------------------------------------------
// PatchUtils.cpp : implementaion file.
//----------------------------------------------------------------------------------
#include "PatchUtils.h"
#include "WONCommon/StringUtil.h"


using namespace WONAPI;


//----------------------------------------------------------------------------------
// Let other process have a crack at the processer.
//----------------------------------------------------------------------------------
void ReleaseControl(DWORD tSpin)
{
	MSG Msg;

	DWORD tWaitTil = GetTickCount() + tSpin;

	do
	{
		while (PeekMessage(&Msg, NULL, 0, 0, TRUE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
	while (GetTickCount() < tWaitTil);
}

//----------------------------------------------------------------------------------
// Generate a temporary file name to hold a recipe file.
//----------------------------------------------------------------------------------
std::string GenerateTempFileName(std::string sPrefix, std::string sExt)
{
	char sFileName[MAX_PATH];
	char sTempDir[MAX_PATH];
	GetTempPath(MAX_PATH, sTempDir);
	GetTempFileName(sTempDir, sPrefix.c_str(), 0, sFileName);
	DeleteFile(sFileName); // Sometimes this file is actually created.

	std::string sFile = sFileName;
	int nPos = sFile.rfind('.');
	if (nPos != -1)
		sFile.erase(nPos, sFile.length() - nPos);
	sFile += sExt;

	return sFileName;
}

//----------------------------------------------------------------------------------
// Return the size of the file (in bytes).
//----------------------------------------------------------------------------------
DWORD GetFileSize(const std::string& sFileName)
{
	if (sFileName.length() == 0)
		return 0;

	DWORD nFileSize = 0;
	HANDLE hFile = CreateFile(sFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL); 
	if (hFile != INVALID_HANDLE_VALUE) //lint !e1924
	{
		nFileSize = ::GetFileSize(hFile, 0);
		CloseHandle(hFile);
	}
	return nFileSize;
}

//----------------------------------------------------------------------------------
// Does the supplied file exist?
//----------------------------------------------------------------------------------
bool FileExists(const std::string& sFileName)
{
	if (sFileName.length() == 0)
		return false;

	bool bFound = false;
	WIN32_FIND_DATA FD;
	HANDLE hFile = FindFirstFile(sFileName.c_str(), &FD);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		bFound = true;
		FindClose(hFile);
	}

	return bFound;
}

//----------------------------------------------------------------------------------
// Does the supplied directory exist?
//----------------------------------------------------------------------------------
bool DirectoryExists(const std::string& sDir)
{
	UINT nLen = sDir.length();

	if (nLen == 0)
		return false;

	// Check for a disk.
	int nColen = sDir.find(':');
	if (nLen == 2 && nColen == 1)
	{
		UINT nType = GetDriveType(sDir.c_str());
		return nType != DRIVE_UNKNOWN;
	}

	DWORD nAttribs = GetFileAttributes(sDir.c_str());
	if (nAttribs == 0xFFFFFFFF)
		return false;

	return (nAttribs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

//----------------------------------------------------------------------------------
// Recursive directory creation.
//----------------------------------------------------------------------------------
bool CreateDirectoryRecursive(const std::string& sDir)
{
	// If it already exists, we're done, bail out.
	if (DirectoryExists(sDir))
		return true;

	// Try to create the parent (or some ancestor) first.
	std::string sNew = sDir;
	UINT nPos = sNew.rfind('\\');

	if (nPos != 0xFFFFFFFF)
	{
		sNew.erase(nPos, sNew.length() - nPos);
		if (CreateDirectoryRecursive(sNew))
			return CreateDirectory(sDir.c_str(), 0) != FALSE;
	}

	return false;
}

//----------------------------------------------------------------------------------
// Convert a number to a string that includes commas.
// i.e. 1234567 -> '1,234,567'
//----------------------------------------------------------------------------------
GUIString NumToStrWithCommas(int nVal)
{
	TCHAR sOutput[32];

	if (nVal == 0)
		return "0";

	if (nVal < 0)
	{
		GUIString sVal = TEXT("-");
		sVal.append(NumToStrWithCommas(-nVal));
		return sVal;
	}

	int nOnes = nVal % 1000;
	int nTousands = (nVal / 1000) % 1000;
	int nMillions = (nVal / 1000000) % 1000;
	int nBillions = nVal / 1000000000;

	// Fetch the internationalized numeric separator.
	setlocale(LC_ALL, "");
	lconv* pLocalConv = localeconv();
	char* pSep = pLocalConv->mon_thousands_sep;
	TCHAR cSep = *pSep;

	if (nBillions)
		wsprintf(sOutput, "%d%c%03d%c%03d%c%03d", nBillions, cSep, nMillions, cSep, nTousands, cSep, nOnes);
	else if (nMillions)
		wsprintf(sOutput, "%d%c%03d%c%03d", nMillions, cSep, nTousands, cSep, nOnes);
	else if (nTousands)
		wsprintf(sOutput, "%d%c%03d", nTousands, cSep, nOnes);
	else
		wsprintf(sOutput, "%d", nOnes);

	return sOutput;
}

//----------------------------------------------------------------------------------
// Replace all instances of sKey with sValue within sString.  The search is 
// NOT case sensitive.
//----------------------------------------------------------------------------------
bool ReplaceSubString(GUIString& sString, const GUIString& sKey, const GUIString& sValue)
{
	bool bRet = false;

	GUIString sLowerString = sString;
	sLowerString.toLowerCase();

	GUIString sLowerKey = sKey;
	sLowerKey.toLowerCase();

	int nPos = sLowerString.find(sLowerKey);
	while (nPos != -1)
	{
		// Replace the 'key' with the 'value'.
		sString.erase(nPos, sKey.length());
		sString.insert(nPos, sValue);

		// Look for another copy of the key.
		sLowerString = sString;
		sLowerString.toLowerCase();
		nPos = sLowerString.find(sLowerKey);
	}

	return bRet;
}
