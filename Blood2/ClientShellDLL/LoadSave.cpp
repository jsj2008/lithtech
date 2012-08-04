//----------------------------------------------------------
//
// MODULE  : LOADSAVE.CPP
//
// PURPOSE : Load & Save game info
//
// CREATED : 4/19/98
//
//----------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include "LoadSave.h"
#include "ClientUtilities.h"
#include "time.h"
#include <mbstring.h>

// Declare the static data
SavedInfo CSavedGameInfo::gQuickSaveInfo;
SavedInfo CSavedGameInfo::gCurrentSaveInfo;
SavedInfo CSavedGameInfo::gSaveSlotInfo[MAX_SAVESLOTS];



static const char g_szEmptySlot[] = "<EMPTY SLOT>";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSavedGameInfo::CSavedGameInfo		
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSavedGameInfo::CSavedGameInfo()
{
	// Initialize the slot list and fill it with a placeholder title
	for (int nIndex=0; nIndex < MAX_SAVESLOTS; nIndex++)
	{
		memset(&gSaveSlotInfo[nIndex], sizeof(SavedInfo), 0);
		_mbscpy((unsigned char*)gSaveSlotInfo[nIndex].szName, (const unsigned char*)g_szEmptySlot);
	}
	memset(&gCurrentSaveInfo, sizeof(SavedInfo), 0);
	_mbscpy((unsigned char*)gCurrentSaveInfo.szName, (const unsigned char*)g_szEmptySlot);
	memset(&gQuickSaveInfo, sizeof(SavedInfo), 0);
	_mbscpy((unsigned char*)gQuickSaveInfo.szName, (const unsigned char*)g_szEmptySlot);

	m_nReservedSlot = SLOT_NONE;
}

	
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSavedGameInfo::Load (...)
//
//	PURPOSE:	Goes through the save game slots and loads the save info.
//
// ----------------------------------------------------------------------- //

void CSavedGameInfo::LoadInfo()
{
    WIN32_FIND_DATA fileinfo;
    HANDLE hRc, hFile;
	int nIndex;
	char szPath[MAX_PATH];

#ifdef _DEMO
	return;
#endif

	for (nIndex=SLOT_CURRENT; nIndex<MAX_SAVESLOTS; nIndex++)
	{
		// See if the Save directory exists
		_mbscpy((unsigned char*)szPath, (const unsigned char*)"Save");
		hRc = FindFirstFile(szPath, &fileinfo);
		FindClose(hRc);
		if (hRc == INVALID_HANDLE_VALUE && CreateDirectory(szPath, NULL) == 0)
		{
			// Save directory doesn't exist, and unable to create it.
			DDWORD dwError = GetLastError();
			PrintError("File error #%d creating directory.", dwError);
			return;
		}


		// See if this save slot directory exists
		if (nIndex == SLOT_QUICK)
			_mbscpy((unsigned char*)szPath, (const unsigned char*)"Save\\Quick");
		else if (nIndex == SLOT_CURRENT)
			_mbscpy((unsigned char*)szPath, (const unsigned char*)"Save\\Current");
		else
			sprintf(szPath, "Save\\Save%d", nIndex);
		hRc = FindFirstFile (szPath, &fileinfo );
		FindClose(hRc);
		if (hRc != INVALID_HANDLE_VALUE && fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Now try to open the save info file.
			_mbscat((unsigned char*)szPath, (const unsigned char*)"\\Save.dat");

			hFile = CreateFile(szPath, 
								GENERIC_READ, 
								0, 
								NULL, 
								OPEN_EXISTING, 
								FILE_ATTRIBUTE_NORMAL,
								NULL);

			if ( hFile == INVALID_HANDLE_VALUE )
				continue;

			DDWORD dwBytesRead;
			// WARNING: ASSUMES game options are saved first in the file
			SavedInfo savedinfo;
			if ( !ReadFile(hFile, 
							&savedinfo, 
							(unsigned)sizeof(SavedInfo),
							&dwBytesRead,
							DNULL))
			{
				DDWORD dwError = GetLastError();
				PrintError("File error #%d reading save file.", dwError);
			}
			if (nIndex == SLOT_QUICK)
				memcpy(&gQuickSaveInfo, &savedinfo, (unsigned)sizeof(SavedInfo));
			else if (nIndex == SLOT_CURRENT)
				memcpy(&gCurrentSaveInfo, &savedinfo, (unsigned)sizeof(SavedInfo));
			else
				memcpy(&gSaveSlotInfo[nIndex], &savedinfo, (unsigned)sizeof(SavedInfo));

			CloseHandle(hFile);
		}
    }
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSavedGameInfo::Save
//
//	PURPOSE:	Attempts to save savegame info
//
// ----------------------------------------------------------------------- //

void CSavedGameInfo::SaveInfo(int nIndex, int nGameType, int nCharacter)
{
    WIN32_FIND_DATA fileinfo;
    HANDLE hRc, hFile;
	char szPath[MAX_PATH];
	SavedInfo	savedinfo;

#ifdef _DEMO
	return;
#endif

	// See if the Save directory exists
	_mbscpy((unsigned char*)szPath, (const unsigned char*)"Save");
	hRc = FindFirstFile(szPath, &fileinfo);
	FindClose(hRc);
	if (hRc == INVALID_HANDLE_VALUE && CreateDirectory(szPath, NULL) == 0)
	{
		// Save directory doesn't exist, and unable to create it.
		DDWORD dwError = GetLastError();
		PrintError("File error #%d creating directory.", dwError);
		return;
	}

	// See if this save slot directory exists
	if (nIndex >=0 && nIndex <= MAX_SAVESLOTS)
	{
		sprintf(szPath, "SAVE\\Save%d", nIndex);
		gSaveSlotInfo[nIndex].nGameType = nGameType;
		gSaveSlotInfo[nIndex].nCharacter = nCharacter;
		memcpy(&savedinfo, &gSaveSlotInfo[nIndex], (unsigned)sizeof(SavedInfo));
	}
	else if (nIndex == SLOT_QUICK)
	{
		// Saving quick save info, expect gQuickSaveInfo to be set already
		_mbscpy((unsigned char*)szPath, (const unsigned char*)"SAVE\\Quick");
		gQuickSaveInfo.nGameType = nGameType;
		gQuickSaveInfo.nCharacter = nCharacter;
		memcpy(&savedinfo, &gQuickSaveInfo, (unsigned)sizeof(SavedInfo));
	}
	else if (nIndex == SLOT_CURRENT)
	{
		// Saving current save info, expect gCurrentSaveInfo to be set already
		_mbscpy((unsigned char*)szPath, (const unsigned char*)"SAVE\\Current");
		gCurrentSaveInfo.nGameType = nGameType;
		gCurrentSaveInfo.nCharacter = nCharacter;
		memcpy(&savedinfo, &gCurrentSaveInfo, (unsigned)sizeof(SavedInfo));
	}

	hRc = FindFirstFile( szPath, &fileinfo );
	FindClose(hRc);

	if (hRc == INVALID_HANDLE_VALUE && CreateDirectory(szPath, NULL) == 0)
	{
		// Directory doesn't exist, and unable to create it.
		DDWORD dwError = GetLastError();
		PrintError("File error #%d creating directory.", dwError);
		return;
	}

	// Now try to open the save info file.
	_mbscat((unsigned char*)szPath, (const unsigned char*)"\\Save.dat");
	hFile = CreateFile(szPath, 
						GENERIC_WRITE, 
						0, 
						NULL, 
						CREATE_ALWAYS, 
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		DDWORD dwErrorCode = GetLastError();
		return;
	}

	// Write the data
	DDWORD dwBytesWritten;
	if ( !WriteFile(hFile, 
					&savedinfo, 
					(unsigned)sizeof(SavedInfo),
					&dwBytesWritten,
					NULL))
	{
		DDWORD dwError = GetLastError();
		PrintError("File error #%d writing to save file.", dwError);
		return;
	}
	CloseHandle(hFile);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSavedGameInfo::ClearSaveSlot
//
//	PURPOSE:	Clears all of the files out of the specified save directory
//
// ----------------------------------------------------------------------- //

void CSavedGameInfo::ClearSaveSlot(int nIndex)
{
    WIN32_FIND_DATA fileinfo;
    HANDLE hRc;
	char szBasePath[MAX_PATH];
	char szFilePath[MAX_PATH];

	if (nIndex >=0 && nIndex <= MAX_SAVESLOTS)
		sprintf(szBasePath, "SAVE\\SAVE%d\\", nIndex);
	else if (nIndex == SLOT_QUICK)
		_mbscpy((unsigned char*)szBasePath, (const unsigned char*)"SAVE\\QUICK\\");
	else if (nIndex == SLOT_CURRENT)
		_mbscpy((unsigned char*)szBasePath, (const unsigned char*)"SAVE\\CURRENT\\");
	else 
		return;

	_mbscpy((unsigned char*)szFilePath, (const unsigned char*)szBasePath);
	_mbscat((unsigned char*)szFilePath, (const unsigned char*)"*.*");

    hRc = FindFirstFile( szFilePath, &fileinfo );
	DBOOL bNotDone = DTRUE;
	if (hRc && hRc != INVALID_HANDLE_VALUE)
	{
		while( bNotDone )
		{
			if (!(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) 
			{
				_mbscpy((unsigned char*)szFilePath, (const unsigned char*)szBasePath);
				_mbscat((unsigned char*)szFilePath, (const unsigned char*)fileinfo.cFileName);

				if ( !DeleteFile(szFilePath) )
				{
					// Unable to delete a file
					DDWORD dwError = GetLastError();
					PrintError("File error #%d deleting file.", dwError);
					return;
				}
			}
			bNotDone = FindNextFile( hRc, &fileinfo );
		}
		FindClose(hRc);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSavedGameInfo::CopySlot
//
//	PURPOSE:	Saves current save data to a slot
//
// ----------------------------------------------------------------------- //

void CSavedGameInfo::CopySlot(int nSrcIndex, int nDestIndex)
{
    WIN32_FIND_DATA fileinfo;
    HANDLE hRc;
	char szDestPath[MAX_PATH];
	char szSrcPath[MAX_PATH];
	SavedInfo *siSrc = NULL;
	SavedInfo *siDest = NULL;

	if (nSrcIndex == nDestIndex) return;

	// Set up source path
	if (nSrcIndex >=0 && nSrcIndex <= MAX_SAVESLOTS)
	{
		sprintf(szSrcPath, "SAVE\\SAVE%d", nSrcIndex);
		siSrc = &gSaveSlotInfo[nSrcIndex];
	}
	else if (nSrcIndex == SLOT_QUICK)
	{
		_mbscpy((unsigned char*)szSrcPath, (const unsigned char*)"SAVE\\QUICK");
		siSrc = &gQuickSaveInfo;
	}
	else if (nSrcIndex == SLOT_CURRENT)
	{
		_mbscpy((unsigned char*)szSrcPath, (const unsigned char*)"SAVE\\CURRENT");
		siSrc = &gCurrentSaveInfo;
	}
	else
		return;

	// Set up destination path
	if (nDestIndex >=0 && nDestIndex <= MAX_SAVESLOTS)
	{
		sprintf(szDestPath, "SAVE\\SAVE%d", nDestIndex);
		siDest = &gSaveSlotInfo[nDestIndex];
	}
	else if (nDestIndex == SLOT_QUICK)
	{
		_mbscpy((unsigned char*)szDestPath, (const unsigned char*)"SAVE\\QUICK");
		siDest = &gQuickSaveInfo;
	}
	else if (nDestIndex == SLOT_CURRENT)
	{
		_mbscpy((unsigned char*)szDestPath, (const unsigned char*)"SAVE\\CURRENT");
		siDest = &gCurrentSaveInfo;
	}
	else
		return;

	if (siSrc && siDest)
		memcpy(siDest, siSrc, (unsigned)sizeof(SavedInfo));

	// See if the src slot directory exists, if not create it
	hRc = FindFirstFile( szSrcPath, &fileinfo );
	FindClose(hRc);

	if (hRc == INVALID_HANDLE_VALUE && CreateDirectory(szSrcPath, NULL) == 0)
	{
		// Directory doesn't exist, and unable to create it.
		DDWORD dwError = GetLastError();
		PrintError("File error #%d creating directory.", dwError);
		return;
	}

	// See if the dest slot directory exists, if not create it
	hRc = FindFirstFile( szDestPath, &fileinfo );
	FindClose(hRc);

	if (hRc == INVALID_HANDLE_VALUE && CreateDirectory(szDestPath, NULL) == 0)
	{
		// Directory doesn't exist, and unable to create it.
		DDWORD dwError = GetLastError();
		PrintError("File error #%d creating directory.", dwError);
		return;
	}


	char szTmpSrcPath[MAX_PATH];
	char szTmpDestPath[MAX_PATH];

	_mbscat((unsigned char*)szDestPath, (const unsigned char*)"\\");
	_mbscat((unsigned char*)szSrcPath, (const unsigned char*)"\\");

	_mbscpy((unsigned char*)szTmpSrcPath, (const unsigned char*)szSrcPath);
	_mbscat((unsigned char*)szTmpSrcPath, (const unsigned char*)"*.*");

	// Clear out what's already in the slot
	ClearSaveSlot(nDestIndex);

	// Copy the files
    hRc = FindFirstFile( szTmpSrcPath, &fileinfo );
	DBOOL bNotDone = DTRUE;
	if (hRc && hRc != INVALID_HANDLE_VALUE)
	{
		while( bNotDone )
		{
			if (!(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY)) 
			{
				_mbscpy((unsigned char*)szTmpSrcPath, (const unsigned char*)szSrcPath);
				_mbscat((unsigned char*)szTmpSrcPath, (const unsigned char*)fileinfo.cFileName);

				_mbscpy((unsigned char*)szTmpDestPath, (const unsigned char*)szDestPath);
				_mbscat((unsigned char*)szTmpDestPath, (const unsigned char*)fileinfo.cFileName);

				if ( !CopyFile(szTmpSrcPath, szTmpDestPath, DFALSE))
				{
					// Unable to copy file
					DDWORD dwError = GetLastError();
					PrintError("File error #%d copying file.", dwError);
					return;
				}
			}
			bNotDone = FindNextFile( hRc, &fileinfo );
		}
		FindClose(hRc);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSavedGameInfo::SetSlotName
//
//	PURPOSE:	Sets the name of a slot for saving
//
// ----------------------------------------------------------------------- //

void CSavedGameInfo::SetSlotName(int nIndex, char *pName)
{
	// If this is to set a name for an actual slot, use the 
	// name already in the Current info and just append a timestamp.
	if (nIndex >=0 && nIndex <= MAX_SAVESLOTS || nIndex == SLOT_QUICK)
	{
		char *pNewName = DNULL;
		if (nIndex == SLOT_QUICK)
			pNewName = gQuickSaveInfo.szName;
		else
			pNewName = gSaveSlotInfo[nIndex].szName;
		
		char tmpbuf[30];
		time_t ltime;
		time( &ltime );

		// Add a timestamp in the format [Jan 01 00:00:00]
		sprintf(tmpbuf, " [%.15s]", ctime(&ltime) + 4);

		_mbscpy((unsigned char*)pNewName, (const unsigned char*)gCurrentSaveInfo.szName);
		_mbsncat((unsigned char*)pNewName, (const unsigned char*)tmpbuf, MAX_SAVENAME - _mbstrlen(pNewName) - 1);
		pNewName[MAX_SAVENAME] = '\0';
	}
	// Autosave, so just copy the current name
	else if (nIndex == SLOT_CURRENT)
	{
		_mbsncpy((unsigned char*)gCurrentSaveInfo.szName, (const unsigned char*)pName, MAX_SAVENAME);
		gCurrentSaveInfo.szName[MAX_SAVENAME] = '\0';
	}
}