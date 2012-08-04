// ----------------------------------------------------------------------- //
//
// MODULE  : SaveLoadMgr.cpp
//
// PURPOSE : Manages the Saving and Loading of games..
//
// CREATED : 12/06/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "MsgIds.h"
#include "WinUtil.h"
#include <time.h>
#include "SaveLoadMgr.h"
#include "VersionMgr.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::CSaveLoadMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CSaveLoadMgr::CSaveLoadMgr( )
{
	m_bUseMultiplayerFolders = false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::~CSaveLoadMgr
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CSaveLoadMgr::~CSaveLoadMgr( )
{
	Term( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::Init
//
//  PURPOSE:	Initializes the object.
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::Init( char const* pszProfileName, bool bUseMultiplayerFolders )
{
	// Verify inputs.
	if( !pszProfileName || !pszProfileName[0] )
	{
		ASSERT( !"CSaveLoadMgr::Init:  Invalid profile name." );
		return false;
	}

	// Start fresh.
	Term( );

	m_sProfileName = pszProfileName;
	m_bUseMultiplayerFolders = bUseMultiplayerFolders;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::Term
//
//  PURPOSE:	Terminates the object.
//
// ----------------------------------------------------------------------- //

void CSaveLoadMgr::Term( )
{
	m_sProfileName.Empty( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::BuildProfileSaveDir
//
//  PURPOSE:	Develop a save directory with the give base name...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::BuildProfileSaveDir( )
{
	// Create the profile root save dir 

	if( !CWinUtil::DirExist( GetProfileSaveDir( ) ))
	{
		if( !CWinUtil::CreateDir( GetProfileSaveDir( ) ))
			return false;
	}

	if( !CWinUtil::DirExist( GetRootSaveDir( ) ))
	{
		if( !CWinUtil::CreateDir( GetRootSaveDir( ) ))
			return false;
	}

	// Create the profile save working dir...

	if( !CWinUtil::DirExist( GetSaveWorkingDir( ) ))
	{
		if( !CWinUtil::CreateDir( GetSaveWorkingDir( ) ))
			return false;
	}

	// Create the profile save quicksave dir...

	if( !CWinUtil::DirExist( GetQuickSaveDir( ) ))
	{
		if( !CWinUtil::CreateDir( GetQuickSaveDir( ) ))
			return false;
	}

	// Create the profile save reload dir...
	
	if( !CWinUtil::DirExist( GetReloadLevelDir( ) ))
	{
		if( !CWinUtil::CreateDir( GetReloadLevelDir( ) ))
			return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::DeleteSpecificSaveDir
//
//  PURPOSE:	Remove the passed in profiles save dir...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::DeleteSpecificSaveDir( const char *pName )
{
	if( !pName ) return false;

	// Delete the profile save root dir.  This deletes all the files and sub-folders.
	if( CWinUtil::DirExist( GetProfileSaveDir( pName ) ))
	{
		if( !CWinUtil::RemoveDir( GetProfileSaveDir( pName ) ))
			return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::WriteContinueINI
//
//  PURPOSE:	Write out the continue information.
//
// ----------------------------------------------------------------------- //

void CSaveLoadMgr::WriteContinueINI( char const* pszSaveKey, char const* pszSaveDir, char const* pszSaveFile )
{
	if( !pszSaveKey || !pszSaveDir || !pszSaveFile )
	{
		ASSERT( !"CSaveLoadMgr::WriteContinueINI:  Invalid inputs." );
		return;
	}

	CString sIniString;
	sIniString.Format( "%s|%s|%s", pszSaveKey, pszSaveDir, pszSaveFile );

	CWinUtil::WinWritePrivateProfileString( GAME_NAME, CONTINUE_INIKEY, sIniString, GetSaveINIFile( ));

	// Flush the file.
	CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, GetSaveINIFile( ));

	return;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::ReadContinueINI
//
//  PURPOSE:	Read in the continue information.
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::ReadContinueINI( char* pszSaveKey, uint32 nSaveKeySize, char* pszSaveDir, uint32 nSaveDirSize,
									char* pszSaveFile, uint32 nSaveFileSize )
{

	// Get the continue ini string.  This has the key, dir and savefile that was used for the last save.
	char szIniString[SLMGR_MAX_INISTR_LEN];
	CWinUtil::WinGetPrivateProfileString( GAME_NAME, CONTINUE_INIKEY, "", szIniString, ARRAY_LEN( szIniString ), 
		GetSaveINIFile( ));

	// Pull out the individual components.
	char* pszKey = strtok( szIniString, "|" );
	char* pszDir = strtok( NULL, "|" );
	char* pszFile = strtok( NULL, "|" );

	// Give them the key if they want it.
	if( pszSaveKey && nSaveKeySize > 0 && pszKey )
	{
		strncpy( pszSaveKey, pszKey, nSaveKeySize );
		pszSaveKey[nSaveKeySize-1] = 0;
	}
	// Give them the dir if they want it.
	if( pszSaveDir && nSaveDirSize > 0 && pszDir )
	{
		strncpy( pszSaveDir, pszDir, nSaveDirSize );
		pszSaveDir[nSaveDirSize-1] = 0;
	}
	// Give them the file if they want it.
	if( pszSaveFile && nSaveFileSize > 0 && pszFile )
	{
		strncpy( pszSaveFile, pszFile, nSaveFileSize );
		pszSaveFile[nSaveFileSize-1] = 0;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::WriteSaveINI
//
//  PURPOSE:	Write the save info to a specific key in the .ini file
//
// ----------------------------------------------------------------------- //

void CSaveLoadMgr::WriteSaveINI( const char *pKey, const char* pszSaveTitle, const char* pszWorldName )
{
	if( !pKey || !pszWorldName || !pszSaveTitle )
	{
		ASSERT( !"CSaveLoadMgr::WriteSaveINI:  Invalid inputs." );
		return;
	}

	time_t	tmSys;
	time( &tmSys );

	// Stamp the key with the World name, save game name and time...
	
	CString sIniString;
	sIniString.Format( "%s|%s|%ld", pszSaveTitle, pszWorldName, (long)tmSys );

	CWinUtil::WinWritePrivateProfileString( GAME_NAME, pKey, sIniString, GetSaveINIFile( ));

	// Flush the file.
	CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, GetSaveINIFile( ));

	return;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::ReadSaveINI
//
//  PURPOSE:	Retrieve the settings for the save game specified by the key...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::ReadSaveINI( const char *pKey, char* pszSaveTitle, uint32 nSaveTitleSize, 
							   char* pszWorldName, uint32 nWorldNameSize,
							   time_t* pSaveTime )
{
	if( !pKey || !pKey[0] )
	{
		ASSERT( !"CSaveLoadMgr::ReadSaveINI: Invalid inputs." );
		return false;
	}
	
	char szIniString[MAX_PATH*2];
	CWinUtil::WinGetPrivateProfileString( GAME_NAME, pKey, "", szIniString, ARRAY_LEN( szIniString ), 
		GetSaveINIFile( ));

	if( !szIniString[0] )
		return false;

	char* pTitle = strtok(szIniString,"|");
    char* pWorldName = strtok(LTNULL,"|");
	char* pTimeStr = strtok(LTNULL,"|");

	if( pszSaveTitle && nSaveTitleSize > 0 && pTitle )
	{
		strncpy( pszSaveTitle, pTitle, nSaveTitleSize );
		pszSaveTitle[nSaveTitleSize-1] = 0;
	}
	if( pszWorldName && nWorldNameSize > 0 && pWorldName )
	{
		strncpy( pszWorldName, pWorldName, nWorldNameSize );
		pszWorldName[nWorldNameSize-1] = 0;
	}
	if( pSaveTime && pTimeStr )
	{
		*pSaveTime = ( time_t )atol( pTimeStr );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::CopyWorkingDir
//
//  PURPOSE:	Copy all files in the profile working dir to the destination working dir..
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::CopyWorkingDir( const char *pDestDir )
{
	if( !pDestDir ) return false;
	if( !_stricmp( GetSaveWorkingDir( ), pDestDir )) return false;

	char szDest[MAX_PATH] = {0};
	sprintf( szDest, "%s\\%s", pDestDir, WORKING_DIR );

	// We don't want to keep old saved levels around...
	CWinUtil::EmptyDir( szDest );
	
	if( !CWinUtil::CopyDir( GetSaveWorkingDir( ), szDest ))
		return false;
	
	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::MoveToWorkingDir
//
//  PURPOSE:	Clear out the working dir and copy the source working dir to it
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::CopyToWorkingDir( const char *pSrcDir )
{
	if( !pSrcDir ) return false;
	if( !_stricmp( GetSaveWorkingDir( ), pSrcDir )) return false;

	char szSrc[MAX_PATH] = {0};
	sprintf( szSrc, "%s\\%s", pSrcDir, WORKING_DIR );

	// Clear out the working dir save files...

	ClearWorkingDir();

	// Copy over the working save files...

	if( !CWinUtil::CopyDir( szSrc, GetSaveWorkingDir( ) ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::ClearWorkingDir
//
//  PURPOSE:	Delete all files in the working directory...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::ClearWorkingDir( )
{
	// Clear out the working dir save files...

	if( !CWinUtil::EmptyDir( GetSaveWorkingDir( ) ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::SaveExists
//
//  PURPOSE:	Does the save file and .ini entry exist...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::SaveExists( char const* pszSaveIniKey, char const* pszSaveFile )
{
	if( !pszSaveIniKey || !pszSaveIniKey[0] || !pszSaveFile || !pszSaveFile[0] )
	{
		ASSERT( !"CSaveLoadMgr::SaveExists: Invalid inputs." );
		return false;
	}

	char szSaveTitle[SLMGR_MAX_INISTR_LEN+1];
	char szWorldName[MAX_PATH*2];
	if( !ReadSaveINI( pszSaveIniKey, szSaveTitle, ARRAY_LEN( szSaveTitle ), 
		szWorldName, ARRAY_LEN( szWorldName ), NULL ))
		return false;

	if( !szSaveTitle[0] || !szWorldName[0] )
		return false;
	
	if( !CWinUtil::FileExist( pszSaveFile ))
		return false;

	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::QuickSaveExists
//
//  PURPOSE:	Does the quick save file and .ini entry exist...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::QuickSaveExists( )
{
	return SaveExists( QUICKSAVE_INIKEY, GetQuickSaveFile( ));
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::ReloadSaveExists
//
//  PURPOSE:	Does the Reload save file and .ini entry exist...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::ReloadSaveExists( )
{
	return SaveExists( RELOADLEVEL_INIKEY, GetReloadLevelFile( ));
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::SlotSaveExists
//
//  PURPOSE:	Does this slot save file and .ini entry exist...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::SlotSaveExists( uint32 nSlot )
{
	return SaveExists( GetSlotSaveKey( nSlot ), GetSlotSaveFile( nSlot ));
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::CanContinueGame
//
//  PURPOSE:	Test if we are allowed to continue a game...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::CanContinueGame( )
{
	// Is the .ini file actually there...

	if( !CWinUtil::FileExist( GetSaveINIFile( ) ))
		return false;

	char szSaveKey[SLMGR_MAX_INIKEY_LEN];
	char szSaveFile[MAX_PATH];

	if( !ReadContinueINI( szSaveKey, ARRAY_LEN( szSaveKey ), NULL, 0, szSaveFile, ARRAY_LEN( szSaveFile )))
		return false;

	if( !SaveExists( szSaveKey, szSaveFile ))
		return false;

	return true;
}