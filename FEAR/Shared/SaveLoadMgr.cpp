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

#include "Stdafx.h"
#include "MsgIDs.h"
#include <time.h>
#include "SaveLoadMgr.h"
#include "VersionMgr.h"
#include "WinUtil.h"
#include "ltprofileutils.h"
#include "sys/win/mpstrconv.h"
#include "iltfilemgr.h"

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
	// If this isn't mp, then we need a profile name to use for saving save/load files.
	if( !bUseMultiplayerFolders && ( !pszProfileName || !pszProfileName[0] ))
	{
		ASSERT( !"CSaveLoadMgr::Init:  Invalid profile name." );
		return false;
	}

	// Start fresh.
	Term( );

	m_sProfileName           = pszProfileName;
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
	m_sProfileName.clear();
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
	char szAbsDir[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetProfileSaveDir( GetProfileName( )), szAbsDir, LTARRAYSIZE( szAbsDir ));
	if( !CWinUtil::DirExist( szAbsDir ))
	{
		if( !CWinUtil::CreateDir( szAbsDir ))
			return false;
	}

	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetRootSaveDir( GetProfileName( )), szAbsDir, LTARRAYSIZE( szAbsDir ));
	if( !CWinUtil::DirExist( szAbsDir ))
	{
		if( !CWinUtil::CreateDir( szAbsDir ))
			return false;
	}

	// Create the profile save working dir...
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetSaveWorkingDir( GetProfileName( )), szAbsDir, LTARRAYSIZE( szAbsDir ));
	if( !CWinUtil::DirExist( szAbsDir ) )
	{
		if( !CWinUtil::CreateDir( szAbsDir ) ) 
			return false;
	}

	// Create the profile save quicksave dir...
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetQuickSaveDir( GetProfileName( )), szAbsDir, LTARRAYSIZE( szAbsDir ));
	if( !CWinUtil::DirExist( szAbsDir ) )
	{
		if( !CWinUtil::CreateDir( szAbsDir ) )
			return false;
	}

	// Create the profile save reload dir...
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetReloadLevelDir( GetProfileName( )), szAbsDir, LTARRAYSIZE( szAbsDir ));
	if( !CWinUtil::DirExist( szAbsDir ) )
	{
		if( !CWinUtil::CreateDir( szAbsDir ) )
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
	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetProfileSaveDir( pName ), szAbsFile, LTARRAYSIZE( szAbsFile ));
	if( CWinUtil::DirExist( szAbsFile ))
	{
		if( !CWinUtil::RemoveDir( szAbsFile ))
		{
			return false;
		}
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

	char pszBuffer[1024];
	LTSNPrintF(pszBuffer, LTARRAYSIZE(pszBuffer), "%s|%s|%s", pszSaveKey, pszSaveDir, pszSaveFile );

	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetSaveINIFile( GetProfileName() ), szAbsFile, LTARRAYSIZE( szAbsFile ));

	CWinUtil::WinWritePrivateProfileString( GAME_NAME, CONTINUE_INIKEY, pszBuffer, szAbsFile );

	// Flush the file.
	CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, szAbsFile );

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

	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetSaveINIFile( GetProfileName()), szAbsFile, LTARRAYSIZE( szAbsFile ));

	// Get the continue ini string.  This has the key, dir and savefile that was used for the last save.
	char szIniString[SLMGR_MAX_INISTR_LEN];
	LTProfileUtils::ReadString( GAME_NAME,
	                                          CONTINUE_INIKEY,
	                                          "",
	                                          szIniString,
	                                          ARRAY_LEN( szIniString ), 
	                                          szAbsFile );

	// Pull out the individual components.
	char* pszKey  = strtok( szIniString, "|" );
	char* pszDir  = strtok( NULL, "|" );
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

void CSaveLoadMgr::WriteSaveINI( const char *pKey, const wchar_t* pwszSaveTitle, const char* pszWorldName )
{
	if( !pKey || !pszWorldName || !pwszSaveTitle )
	{
		LTERROR( "Invalid inputs." );
		return;
	}

	time_t tmSys;
	time( &tmSys );

	// Stamp the key with the World name, save game name and time...

	wchar_t wszKey[256];
	LTStrCpy( wszKey, MPA2W( pKey ).c_str( ), LTARRAYSIZE( wszKey ));
	wchar_t wszGameName[256];
	LTStrCpy( wszGameName, MPA2W( GAME_NAME ).c_str(), LTARRAYSIZE( wszGameName ));

	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetSaveINIFile( GetProfileName() ), szAbsFile, LTARRAYSIZE( szAbsFile ));

	wchar_t wszIniFile[256];
	LTStrCpy( wszIniFile, MPA2W( szAbsFile ).c_str( ), LTARRAYSIZE( wszIniFile ));
	wchar_t wszFullKey[256];

	LTSNPrintF( wszFullKey, LTARRAYSIZE( wszFullKey ), L"%s_%s", wszKey, L"Title" );
	CWinUtil::WinWritePrivateProfileString( wszGameName, wszFullKey, pwszSaveTitle, wszIniFile );
	LTSNPrintF( wszFullKey, LTARRAYSIZE( wszFullKey ), L"%s_%s", wszKey, L"World" );
	CWinUtil::WinWritePrivateProfileString( wszGameName, wszFullKey, MPA2W( pszWorldName ).c_str( ), wszIniFile );
	LTSNPrintF( wszFullKey, LTARRAYSIZE( wszFullKey ), L"%s_%s", wszKey, L"Time" );
	wchar_t wszTime[256];
	LTSNPrintF( wszTime, LTARRAYSIZE( wszTime ), L"%ld", (long)tmSys );
	CWinUtil::WinWritePrivateProfileString( wszGameName, wszFullKey, wszTime, wszIniFile );

	// Flush the file.
	CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, wszIniFile );

	return;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::ReadSaveINI
//
//  PURPOSE:	Retrieve the settings for the save game specified by the key...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::ReadSaveINI( const char *pKey, wchar_t* pwszSaveTitle, uint32 nSaveTitleSize, 
							   char* pszWorldName, uint32 nWorldNameSize,
							   time_t* pSaveTime )
{
	if( !pKey || !pKey[0] )
	{
		LTERROR( "Invalid inputs." );
		return false;
	}

	// Convert key to wide string for further string operations.
	wchar_t wszKey[256];
	LTStrCpy( wszKey, MPA2W( pKey ).c_str( ), LTARRAYSIZE( wszKey ));
	wchar_t wszGameName[256];
	LTStrCpy( wszGameName, MPA2W( GAME_NAME ).c_str(), LTARRAYSIZE( wszGameName ));
	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetSaveINIFile( GetProfileName()), szAbsFile, LTARRAYSIZE( szAbsFile ));
	wchar_t wszIniFile[MAX_PATH*2];
	LTStrCpy( wszIniFile, MPA2W( szAbsFile ).c_str( ), LTARRAYSIZE( wszIniFile ));
	wchar_t wszFullKey[256];

	if( pwszSaveTitle )
	{
		LTSNPrintF( wszFullKey, LTARRAYSIZE( wszFullKey ), L"%s_%s", wszKey, L"Title" );
		LTProfileUtils::ReadString( wszGameName, wszFullKey, L"", pwszSaveTitle, nSaveTitleSize, 
			wszIniFile );
	}
	if( pszWorldName )
	{
		wchar_t wszWorldName[MAX_PATH*2];
		LTSNPrintF( wszFullKey, LTARRAYSIZE( wszFullKey ), L"%s_%s", wszKey, L"World" );
		LTProfileUtils::ReadString( wszGameName, wszFullKey, L"", wszWorldName, LTARRAYSIZE( wszWorldName ), 
			wszIniFile );
		LTStrCpy( pszWorldName, MPW2A( wszWorldName ).c_str( ), nWorldNameSize );
	}
	if( pSaveTime )
	{
		wchar_t wszTime[MAX_PATH*2];
		LTSNPrintF( wszFullKey, LTARRAYSIZE( wszFullKey ), L"%s_%s", wszKey, L"Time" );
		LTProfileUtils::ReadString( wszGameName, wszFullKey, L"", wszTime, LTARRAYSIZE( wszTime ), 
			wszIniFile );
		*pSaveTime = ( time_t )atol( MPW2A( wszTime ).c_str( ));
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

	char szRelSrc[MAX_PATH*2];
	LTStrCpy( szRelSrc, GetSaveWorkingDir( GetProfileName( )), LTARRAYSIZE( szRelSrc ));

	if( LTStrIEquals( szRelSrc, pDestDir )) 
		return false;

	// Create absolute path for copy operation.
	char szDest[MAX_PATH*2] = "";
	LTFileOperations::GetUserDirectory( szDest, LTARRAYSIZE( szDest ) );
	LTStrCat( szDest, pDestDir, LTARRAYSIZE( szDest ) );
	LTStrCat( szDest, FILE_PATH_SEPARATOR WORKING_DIR FILE_PATH_SEPARATOR, LTARRAYSIZE( szDest ) );

	char szSrc[MAX_PATH*2] = "";
	LTFileOperations::GetUserDirectory( szSrc, LTARRAYSIZE( szSrc ));
	LTStrCat( szSrc, szRelSrc, LTARRAYSIZE( szSrc ));

	// We don't want to keep old saved levels around...
	CWinUtil::EmptyDir( szDest );
	
	if( !CWinUtil::CopyDir( szSrc, szDest ))
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

	char szRelDest[MAX_PATH*2];
	LTStrCpy( szRelDest, GetSaveWorkingDir( GetProfileName( )), LTARRAYSIZE( szRelDest ));

	if( LTStrIEquals( szRelDest, pSrcDir )) 
		return false;

	char szSrc[MAX_PATH*2] = {0};
	LTFileOperations::GetUserDirectory( szSrc, LTARRAYSIZE( szSrc ) );
	LTStrCat( szSrc, pSrcDir, LTARRAYSIZE( szSrc ));

	char szDest[MAX_PATH*2] = "";
	LTFileOperations::GetUserDirectory( szDest, LTARRAYSIZE( szDest ));
	LTStrCat( szDest, szRelDest, LTARRAYSIZE( szDest ));

	// Clear out the working dir save files...
	ClearWorkingDir();

	// Copy over the working save files...
	if( !CWinUtil::CopyDir( szSrc, szDest ))
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
	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetSaveWorkingDir( GetProfileName() ), szAbsFile, LTARRAYSIZE( szAbsFile ));

	if( !CWinUtil::EmptyDir( szAbsFile ))
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

	char szWorldName[MAX_PATH*2];
	if( !ReadSaveINI( pszSaveIniKey, NULL, 0, szWorldName, LTARRAYSIZE( szWorldName ), NULL ))
		return false;

	if( !szWorldName[0] )
		return false;
	
	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( pszSaveFile, szAbsFile, LTARRAYSIZE( szAbsFile ));

	if( !CWinUtil::FileExist( szAbsFile ))
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
	return SaveExists( QUICKSAVE_INIKEY, GetQuickSaveFile( GetProfileName() ) );
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
	return SaveExists( RELOADLEVEL_INIKEY, GetReloadLevelFile( GetProfileName() ) );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSaveLoadMgr::CheckpointSaveExists
//
//  PURPOSE:	Does the checkpointsave file and .ini entry exist...
//
// ----------------------------------------------------------------------- //

bool CSaveLoadMgr::CheckpointSaveExists( )
{
	return SaveExists( CHECKPOINTSAVE_INIKEY, GetCheckpointSaveFile( GetProfileName() ) );
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
	return SaveExists( GetSlotSaveKey( nSlot ), GetSlotSaveFile( nSlot, GetProfileName() ) );
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
	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetSaveINIFile( GetProfileName() ), szAbsFile, LTARRAYSIZE( szAbsFile ));
	if( !CWinUtil::FileExist( szAbsFile ) )
		return false;

	char szSaveKey[SLMGR_MAX_INIKEY_LEN];
	char szSaveFile[MAX_PATH];

	if( !ReadContinueINI( szSaveKey, ARRAY_LEN( szSaveKey ), NULL, 0, szSaveFile, ARRAY_LEN( szSaveFile )))
		return false;

	if( !SaveExists( szSaveKey, szSaveFile ))
		return false;

	return true;
}
