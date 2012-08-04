// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSaveLoadMgr.cpp
//
// PURPOSE : Manages the Saving and Loading of games for server.
//
// CREATED : 02/07/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "ServerSaveLoadMgr.h"
#include "MsgIDs.h"
#include "WinUtil.h"
#include "TransitionMgr.h"
#include "PlayerObj.h"
#include "WorldProperties.h"
#include "ServerMissionMgr.h"
#include "ltfileoperations.h"
#include "iltfilemgr.h"
#include "ServerConnectionMgr.h"

// 
// Globals...
//

//ILTLoadingProgress for access to the loading bar
#include "iltloadingprogress.h"
static ILTLoadingProgress* g_pILTLoadingProgress;
define_holder(ILTLoadingProgress, g_pILTLoadingProgress);

CServerSaveLoadMgr *g_pServerSaveLoadMgr = NULL;
	


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::CServerSaveLoadMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CServerSaveLoadMgr::CServerSaveLoadMgr( )
{
	ASSERT( g_pServerSaveLoadMgr == NULL );

	m_eSaveDataState = eSaveDataStateNone;
	g_pServerSaveLoadMgr = this;
	m_bPlayerTrackerAborted = false;
	m_bWaitingForAutoLoadResponses = false;
	m_bLoadingLevel = false;
	m_bCanSaveOverride = true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::~CServerSaveLoadMgr
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CServerSaveLoadMgr::~CServerSaveLoadMgr( )
{
	Term( );

	ASSERT( g_pServerSaveLoadMgr != NULL );

	g_pServerSaveLoadMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::Init
//
//  PURPOSE:	Initializes the object given a profile name.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::Init( char const* pszProfileName, bool bUseMultiplayerFolders )
{
	// Start fresh.
	Term( );

	// Tell the base class to init first.
	if( !CSaveLoadMgr::Init( pszProfileName, bUseMultiplayerFolders ))
		return false;

	m_eSaveDataState = eSaveDataStateNone;
	m_bPlayerTrackerAborted = false;

	// Clear working dir since we're starting fresh.
	ClearWorkingDir( );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::Term
//
//  PURPOSE:	Terminates the object.
//
// ----------------------------------------------------------------------- //

void CServerSaveLoadMgr::Term( )
{
	CSaveLoadMgr::Term( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::LoadNewLevel
//
//  PURPOSE:	Load new level with no restoring of keepalives or save games.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::LoadNewLevel( char const* pszNewLevel )
{
	if( !pszNewLevel )
	{
		ASSERT( !"CServerSaveLoadMgr::LoadNewLevel:  Invalid filename." );
		return false;
	}

	g_pGameServerShell->SetLGFlags( LOAD_NEW_GAME );

	if( !LoadLevel( pszNewLevel, true ) )
		return false;

	if( !RunWorld() )
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::SwitchLevels
//
//  PURPOSE:	Switch levels with keepalives
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::SwitchLevels( char const* pszNewLevel, bool bDoKeepAliveSave )
{
	TRACE( "CServerSaveLoadMgr::SwitchLevels\n" );

	if( !pszNewLevel )
	{
		ASSERT( !"CServerSaveLoadMgr::SwitchLevels:  Invalid filename." );
		return false;
	}

	// Make sure all the folders are setup.
	if( bDoKeepAliveSave )
		if( !BuildProfileSaveDir( ))
			return false;

	// Remember our new level to go to.
	m_sSaveDataNewLevel = pszNewLevel;

	// If we do saves, we need to ask the client.
	if( bDoKeepAliveSave )
	{
		// Ask the client for the save data and wait for ack.
		if( !AskClientsForSaveData( eSaveDataStateSwitchLevels ))
			return false;

		return true;
	}

	// Load the level with objects.
	if( !LoadLevel( m_sSaveDataNewLevel.c_str(), true ))
		return false;

	// Make sure mission mgr has the right settings for this level.
	g_pServerMissionMgr->SetMissionBasedOnLevel( m_sSaveDataNewLevel.c_str() );

	if( !RunWorld( ))
		return false;

	// Don't need newlevel name anymore.
	m_sSaveDataNewLevel.clear( );

	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::TransitionLevels
//
//  PURPOSE:	Transition between levels with transition volumes.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::TransitionLevels( char const* pszNewLevel )
{
	TRACE( "CServerSaveLoadMgr::SwitchLevels\n" );

	if( !pszNewLevel )
	{
		ASSERT( !"CServerSaveLoadMgr::TransitionLevels:  Invalid filename." );
		return false;
	}

	// Make sure all the folders are setup.
	if( !BuildProfileSaveDir( ))
		return false;

	// Remember our new level to go to.
	m_sSaveDataNewLevel = pszNewLevel;

	g_pGameServerShell->SetLGFlags( LOAD_NEW_LEVEL );

	// Ask the client for the save data and wait for ack.
	if( !AskClientsForSaveData( eSaveDataStateTransitionLevels ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::LoadSaveFile
//
//  PURPOSE:	Load a game from save type
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::LoadSaveFile( char const* pszSaveIniKey,
                                       char const* pszSaveDir, 
                                       char const* pszSaveFile )
{
	// Check inputs.
	if( !pszSaveIniKey || !pszSaveIniKey[0] || !pszSaveDir || !pszSaveDir[0] || !pszSaveFile || !pszSaveFile[0] )
	{
		ASSERT( !"CServerSaveLoadMgr::LoadSaveFile: Invalid inputs." );
		return false;
	}

	char szWorldName[MAX_PATH*2];
	if( !ReadSaveINI( pszSaveIniKey, NULL, 0, szWorldName, ARRAY_LEN( szWorldName ), NULL ))
		return false;

	if( !szWorldName[0] )
		return false;

	if( !CopyToWorkingDir( pszSaveDir ))
		return false;

	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	// Make sure mission mgr has the right settings for this level.
	g_pServerMissionMgr->ExitLevelToSavedGame( szWorldName );

	if( !LoadLevel( szWorldName, false ))
		return false;

	if( !RestoreObjectsFromSaveFile( pszSaveFile, false ))
		return false;

	if( !RunWorld( ))
		return false;

	// Write this save as the continue save.
	WriteContinueINI( pszSaveIniKey, pszSaveDir, pszSaveFile );

	return true;
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::LoadGameSlot
//
//  PURPOSE:	Load a game from a specific slot.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::LoadGameSlot( int nSlot )
{
	if( !SlotSaveExists( nSlot ) )
	{
		return false;
	}

	// Load the save file.
	if( !LoadSaveFile( GetSlotSaveKey( nSlot ),
	                   GetSlotSaveDir( nSlot, GetProfileName() ),
	                   GetSlotSaveFile( nSlot, GetProfileName() ) ) )
	{
		return false;
	}

	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::SaveGameSlot
//
//  PURPOSE:	Save a named game to a specific slot.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::SaveGameSlot( int nSlot, const wchar_t* pwszSaveName )
{
	if( !CanSaveGame() )
	{
		HandleSaveFailed();
		return false;
	}

	// Are we trying to save to the Quick save slot?
	if( nSlot == 0 ) 
	{
		LTERROR( "Invalid slot specified." );
		return false;
	}

	if( !pwszSaveName )
	{
		LTERROR( "Invalid savename specified." );
		return false;
	}

	// Make sure all the folders are setup.
	if( !BuildProfileSaveDir() )
	{
		return false;
	}

	// Create the slot save dir if it doesn't yet exist...
	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetSlotSaveDir( nSlot, GetProfileName() ), szAbsFile, LTARRAYSIZE( szAbsFile ));
	if( !CWinUtil::FileExist( szAbsFile ) )
	{
		if( !CWinUtil::CreateDir( szAbsFile ))
		{
			return false;
		}
	}

	// these paths are stored relative.
	m_sSaveGameDir = GetSlotSaveDir( nSlot, GetProfileName() );
	m_sSaveGameFile = GetSlotSaveFile( nSlot, GetProfileName() );

	char pszSaveGameKey[256];
	LTSNPrintF(pszSaveGameKey, LTARRAYSIZE(pszSaveGameKey), "%s%02d", SLOTSAVE_INIKEY, nSlot );
	m_sSaveGameKey = pszSaveGameKey;

	m_wsSaveGameTitle = pwszSaveName;

	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	// Ask the client for the save data and wait for ack.
	if( !AskClientsForSaveData( eSaveDataSaveGameSlot ) )
	{
		return false;
	}

	return true;	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::QuickLoad
//
//  PURPOSE:	Handle the loading of the quick save file.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::QuickLoad( )
{
	if( !QuickSaveExists( ))
		return false;

	// Load the save file.
	if( !LoadSaveFile( QUICKSAVE_INIKEY,
	                   GetQuickSaveDir( GetProfileName() ),
	                   GetQuickSaveFile( GetProfileName() ) ) )
	{
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::QuickSave
//
//  PURPOSE:	Handle quick saving of the game.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::QuickSave( int nMission, int nLevel )
{
	if( !CanSaveGame())
	{
		HandleSaveFailed();
		return false;
	}

	// Make sure all the folders are setup.
	if( !BuildProfileSaveDir( ))
		return false;

	// Create the quicksave dir if it doesn't yet exist...
	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetQuickSaveDir( GetProfileName()), szAbsFile, LTARRAYSIZE( szAbsFile ));
	if( !CWinUtil::DirExist( szAbsFile ))
	{
		if( !CWinUtil::CreateDir( szAbsFile ) ) 
			return false;
	}

	// these paths are stored relative.
	m_sSaveGameDir = GetQuickSaveDir( GetProfileName() );
	m_sSaveGameFile = GetQuickSaveFile( GetProfileName() );
	m_sSaveGameKey = QUICKSAVE_INIKEY;

	wchar_t wszSaveGameTitle[256];
	LTSNPrintF(wszSaveGameTitle, LTARRAYSIZE(wszSaveGameTitle), L"%i,%i", nMission, nLevel );
	m_wsSaveGameTitle = wszSaveGameTitle;

	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	// Ask the client for the save data and wait for ack.
	if( !AskClientsForSaveData( eSaveDataSaveGameSlot ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::ReloadLevel
//
//  PURPOSE:	Reload the current level.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::ReloadLevel( )
{
	if( !ReloadSaveExists() ) return false;

	// Load the save file.
	if( !LoadSaveFile( RELOADLEVEL_INIKEY,
	                   GetReloadLevelDir( GetProfileName() ),
	                   GetReloadLevelFile( GetProfileName() ) ) )
	{
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::LoadCheckpointSave
//
//  PURPOSE:	Load the checkpointsave
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::LoadCheckpointSave( )
{
	if( !CheckpointSaveExists() ) return false;

	// Load the save file.
	if( !LoadSaveFile( CHECKPOINTSAVE_INIKEY,
	                   GetCheckpointSaveDir( GetProfileName() ),
	                   GetCheckpointSaveFile( GetProfileName() ) ) )
	{
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::ReloadSave
//
//  PURPOSE:	Handle automatic saving of beginning of level.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::ReloadSave( int nMission, int nLevel )
{
	if( !CanSaveGame())
		return false;

	// Make sure all the folders are setup.
	if( !BuildProfileSaveDir( ))
		return false;

	// these paths are stored relative.
	m_sSaveGameDir = GetReloadLevelDir( GetProfileName());
	m_sSaveGameFile = GetReloadLevelFile( GetProfileName());
	m_sSaveGameKey = RELOADLEVEL_INIKEY;

	wchar_t wszSaveGameTitle[256];
	LTSNPrintF(wszSaveGameTitle, LTARRAYSIZE(wszSaveGameTitle), L"%i,%i", nMission, nLevel );
	m_wsSaveGameTitle = wszSaveGameTitle;

	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	// Ask the client for the save data and wait for ack.
	if( !AskClientsForSaveData( eSaveDataReloadSave ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::SaveCheckpointSave
//
//  PURPOSE:	Handle automatic saving of beginning of level.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::SaveCheckpointSave( int nMission, int nLevel )
{
	if( !CanSaveGame())
		return false;

	// Make sure all the folders are setup.
	if( !BuildProfileSaveDir( ))
		return false;

	// these paths are stored relative.
	m_sSaveGameDir = GetCheckpointSaveDir( GetProfileName());
	m_sSaveGameFile = GetCheckpointSaveFile( GetProfileName());
	m_sSaveGameKey = CHECKPOINTSAVE_INIKEY;

	wchar_t wszSaveGameTitle[256];
	LTSNPrintF(wszSaveGameTitle, LTARRAYSIZE(wszSaveGameTitle), L"%i,%i", nMission, nLevel );
	m_wsSaveGameTitle = wszSaveGameTitle;

	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	// Ask the client for the save data and wait for ack.
	if( !AskClientsForSaveData( eSaveDataCheckpointSave ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::ContinueGame
//
//  PURPOSE:	Load the last saved game.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::ContinueGame()
{
	// Are we allowed to continue?
	if( !CanContinueGame() )
	{
		return false;
	}

	char szSaveKey[SLMGR_MAX_INIKEY_LEN];
	char szSaveDir[MAX_PATH];
	char szSaveFile[MAX_PATH];

	// Get the save info from the continue info.
	if( !ReadContinueINI( szSaveKey,  ARRAY_LEN( szSaveKey ),
	                      szSaveDir,  ARRAY_LEN( szSaveDir ), 
	                      szSaveFile, ARRAY_LEN( szSaveFile ) ) )
	{
		return false;
	}

	// Load the save file.
	if( !LoadSaveFile( szSaveKey, szSaveDir, szSaveFile ) )
	{
		return false;
	}

	return true;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::OnMessage
//
//	PURPOSE:	Handle messages.
//
// --------------------------------------------------------------------------- //

bool CServerSaveLoadMgr::OnMessage( HCLIENT hSender, ILTMessage_Read& msg )
{
	msg.SeekTo(0);
	uint8 messageID = msg.Readuint8();

	switch (messageID)
	{
		case MID_LOAD_GAME:					HandleLoadGame					( hSender, msg );	return true; break;
		case MID_SAVE_DATA:					HandleSaveData					( hSender, msg );	return true; break;
		case MID_CLIENT_READY_FOR_AUTOLOAD:	HandleClientReadyForAutoLoad	( hSender, msg );	return true; break;	
		default:
			break;
	}

	return false;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::OnPlayerTrackerAbort
//
//	PURPOSE:	Handle when we have to abort a playertracker.
//
// --------------------------------------------------------------------------- //

void CServerSaveLoadMgr::OnPlayerTrackerAbort( )
{
	// Tell update to handle the playertrackerabort.  We can't continue
	// with the savedata right now because the player is still in the
	// process of getting deleted.
	m_bPlayerTrackerAborted = true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::Update
//
//	PURPOSE:	Frame update.
//
// --------------------------------------------------------------------------- //

void CServerSaveLoadMgr::Update( )
{
	// Update the playertracker.
	m_SaveDataPlayerTracker.Update( );

	m_AutoLoadResponsePlayerTracker.Update();

	// Since all the players dropped out, we can just finish what we
	// were doing.
	if( m_bPlayerTrackerAborted )
	{
		// Clear the abort signal.
		m_bPlayerTrackerAborted = false;
		
		if( m_SaveDataPlayerTracker.Aborted() )
		{
			FinishSaveData( );
		}
		
		if( m_AutoLoadResponsePlayerTracker.Aborted() )
		{
			FinishAutoLoad( );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::CanSaveGame
//
//  PURPOSE:	Test if we are allowed to save.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::CanSaveGame()	const
{ 
	//saving is currently not supported in Xenon
#if defined (PLATFORM_XENON)
	return false;
#endif

    // [KLS 8/28/02] Don't save if we're doing a performance test...
	if (g_pGameServerShell->IsPerformanceTest()) 
		return false;

	// Check the override.
	if( !GetCanSaveOverride())
		return false;

	// Handle single player.
	if( !IsMultiplayerGameServer( ))
	{

		// Get the first and only player and check if he's still alive.
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		if( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj* pPlayerObj = *iter;
			if( !pPlayerObj )
				return true;
			if( !pPlayerObj->IsAlive( ))
				return false;
		}

		return true;
	}
	// Handle multiplayer.
	else
	{
		return false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::LoadLevel
//
//	PURPOSE:	Load a level.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::LoadLevel( char const* pszFilename, bool bLoadWorldObjects )
{
	// Check inputs.
	if( !pszFilename || !pszFilename[0] )
	{
		ASSERT( !"CServerSaveLoadMgr::LoadLevel: Invalid filename." );
		return false;
	}

	// Load the new level.
	int flags = bLoadWorldObjects ? LOADWORLD_LOADWORLDOBJECTS : 0;
	flags |= LOADWORLD_NORELOADGEOMETRY;
    
	m_bLoadingLevel = true;
	LTRESULT dResult = g_pLTServer->LoadWorld(( char* )pszFilename, flags);
	m_bLoadingLevel = false;

	if( dResult != LT_OK )
	{
		// Tell all the clients the server has bad assets.
		ServerConnectionMgr::GameClientDataList& lst = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		for( ServerConnectionMgr::GameClientDataList::iterator iter = lst.begin( ); iter != lst.end( ); iter++ )
		{
			GameClientData* pGameClientData = *iter;
			ServerConnectionMgr::Instance( ).BootWithReason( *pGameClientData, eClientConnectionError_InvalidAssets, NULL );
		}
		LTERROR( "Failed to load level." );
		return false;
	}

	g_pGameServerShell->SetCurLevel( pszFilename );
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::SaveKeepAlives
//
//	PURPOSE:	Saves the keepalive objects.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::SaveKeepAlives( char const* pszFilename )
{
	// Check inputs.
	if( !pszFilename || !pszFilename[0] )
	{
		ASSERT( !"CServerSaveLoadMgr::SaveKeepAlives: Invalid filename." );
		return false;
	}

	// Create an object list to store the keepalive objects in.
    ObjectList* pKeepAliveList = g_pLTServer->CreateObjectList();
	if( !pKeepAliveList )
	{
		ASSERT( !"CServerSaveLoadMgr::SaveKeepAlives: Could not make object list." );
		return false;
	}

	// Ask each player for the list of keepalives.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		
		// Only use this object if it is the player and
		// if he has a controlling client.  The player could be missing a controlling
		// client if this is MP save/load and the player didn't come back.
		if( pPlayerObj && pPlayerObj->GetClient( ))
			pPlayerObj->BuildKeepAlives( pKeepAliveList );

		iter++;
	}

	// Check if there are any objects to restore.
	if( !pKeepAliveList->m_nInList )
	{
		// Free the list.
		g_pLTServer->RelinquishList( pKeepAliveList );
		pKeepAliveList = NULL;

		// Not a failure case.
		return true;
	}

	// Save the objects to the file.
    LTRESULT dResult = g_pLTServer->SaveObjects(( char* )pszFilename, pKeepAliveList, g_pGameServerShell->GetLGFlags( ), 0 );
	if (dResult != LT_OK)
	{
		// Free the list.
		g_pLTServer->RelinquishList( pKeepAliveList );
		pKeepAliveList = NULL;

		ASSERT( !"CServerSaveLoadMgr::SaveKeepAlives: Could not save objects." );
		return false;
	}

	// Free the list.
	g_pLTServer->RelinquishList( pKeepAliveList );
	pKeepAliveList = NULL;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::RestoreObjectsFromSaveFile
//
//	PURPOSE:	Restore objects from save file.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::RestoreObjectsFromSaveFile( char const* pszFilename, bool bKeepAlives )
{
	// Check inputs.
	if( !pszFilename || !pszFilename[0] )
	{
		ASSERT( !"CServerSaveLoadMgr::RestoreObjectsFromSaveFile: Invalid filename." );
		return false;
	}

	// Restore objects from file.
    LTRESULT dResult = g_pLTServer->RestoreObjects(( char* )pszFilename, g_pGameServerShell->GetLGFlags( ), 0 );
	if (dResult != LT_OK)
	{
		ASSERT( !"CServerSaveLoadMgr::RestoreObjectsFromSaveFile: Could not restore objects." );
		return false;
	}

	//Update the progress bar to show that the Objects have been loaded
	if ( g_pILTLoadingProgress )
	{
		g_pILTLoadingProgress->UpdateProgress(eLoadingProgressTask_Objects, 1.0f);
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::RunWorld
//
//	PURPOSE:	Runs the loaded world.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::RunWorld( )
{
	// Start the world...
    LTRESULT dResult = g_pLTServer->RunWorld();
	if( dResult != LT_OK )
	{
		ASSERT( !"CServerSaveLoadMgr::RunWorld: Couldn't Run world." );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::AskClientsForSaveData
//
//  PURPOSE:	Send a message to all the clients and ask for save data.
//				Keep track of who we sent to so we can match responses.
//				We have to wait for everyone in the list unless someone drops.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::AskClientsForSaveData( SaveDataState eSaveDataState )
{
	// Protect against overlapping save requests.  Server should be
	// paused during a save, so new requests should not be getting in.
	if( m_eSaveDataState != eSaveDataStateNone )
	{
		ASSERT( !"CServerSaveLoadMgr::AskClientsForSaveData:  Already in a save state." );
		return false;
	}

	// Initialize the PlayerTracker to the current set of players.
	m_bPlayerTrackerAborted = false;
	if( !m_SaveDataPlayerTracker.Init( *this ))
	{
		// No players.
		return false;
	}

	// Set our state for what to do when all data comes in.
	m_eSaveDataState = eSaveDataState;

	// Ask the players for their save data.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_DATA );
	cMsg.Writeuint8( m_eSaveDataState );
	CLTMsgRef_Read cSendMsg = cMsg.Read();

	// Don't just broadcast the list.  That would send messages to clients that aren't fully connected yet.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		if( pPlayerObj )
		{
			// Make sure the player has a client.
			if( pPlayerObj->GetClient( ))
			{
				g_pLTServer->SendToClient( cSendMsg, pPlayerObj->GetClient( ), MESSAGE_GUARANTEED );
			}
		}
		else
		{
			ASSERT( !"CServerSaveLoadMgr::AskClientsForSaveData:  Invalid playerobj entry." );
		}

		iter++;
	}

	// Need to pause the server while we wait for client save data.
	g_pGameServerShell->PauseGame( true );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::HandleLoadGame
//
//	PURPOSE:	Handle load game message.
//
// --------------------------------------------------------------------------- //

bool CServerSaveLoadMgr::HandleLoadGame( HCLIENT hSender, ILTMessage_Read& msg )
{
	// Ignore load requests from non-local clients.
	if( !( g_pLTServer->GetClientInfoFlags( hSender ) & CIF_LOCAL ))
		return false;

	bool bSuccess = false;

	LoadGameType eLoadGameType = ( LoadGameType )msg.Readuint8( );
	switch( eLoadGameType )
	{
		case kLoadGameTypeSlot:
		{
			int nSlot = msg.Readuint8( );
			bSuccess = LoadGameSlot( nSlot );
		}
		break;
		case kLoadGameTypeQuick:
		{
			bSuccess = QuickLoad( );
		}
		break;
		case kLoadGameTypeReload:
		{
			bSuccess = ReloadLevel( );
		}
		break;
		case kLoadGameTypeCheckpointSave:
		{
			bSuccess = LoadCheckpointSave( );
		}
		break;
		case kLoadGameTypeContinue:
		{
			bSuccess = ContinueGame( );
		}
		break;
		default:
		{
			ASSERT( !"CServerSaveLoadMgr::HandleLoadGame: Invalid loadgametype." );
			bSuccess = false;
		}
	}

	if (!bSuccess)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_LOAD_FAILED );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	}

	return bSuccess;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::HandleSaveData
//
//	PURPOSE:	Handle save data message.
//
// --------------------------------------------------------------------------- //

bool CServerSaveLoadMgr::HandleSaveData( HCLIENT hSender, ILTMessage_Read& msg )
{
	TRACE( "CServerSaveLoadMgr::HandleSaveData: hSender(0x%X)\n", hSender );

	// ignore this if we're not waiting for data
	if( m_eSaveDataState == eSaveDataStateNone )
	{
		return false;
	}


	// Get the playerobj for this client.
	CPlayerObj* pPlayerObjSender = GetPlayerFromHClient(hSender);
	if( !pPlayerObjSender )
	{
		ASSERT( !"CServerSaveLoadMgr::HandleSaveData: Invalid playerobj sender." );
		return false;
	}

	//can the client and server save now?
	if (!msg.Readbool() || !CanSaveGame())
	{
		HandleSaveFailed();
		return false;
	}

	// Read the message in.
	CLTMsgRef_Read pClientData = msg.SubMsg(msg.Tell());

	CPlayerObj* pPlayerObj = m_SaveDataPlayerTracker.RemoveClient( hSender );
	if( pPlayerObj )
	{
		// Set the clientdata for this player.  We must remember to release the message
		// when we're done with it.
		pPlayerObj->SetClientSaveData(pClientData);
	}

	// Check if we are still waiting for more save data.
	if( !m_SaveDataPlayerTracker.IsEmpty( ))
		return true;

	return FinishSaveData( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::HandleClientReadyForAutoLoad
//
//	PURPOSE:	Handle ready for auto load message.
//
// --------------------------------------------------------------------------- //

bool CServerSaveLoadMgr::HandleClientReadyForAutoLoad( HCLIENT hSender, ILTMessage_Read& msg )
{
	if( !m_bWaitingForAutoLoadResponses )
		return false;
	
	m_AutoLoadResponsePlayerTracker.RemoveClient( hSender );

	if( m_AutoLoadResponsePlayerTracker.IsEmpty() )
		FinishAutoLoad();
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::ClearClientSaveData
//
//  PURPOSE:	The players were given the clientsave data in HandleSaveData.
//				When we're done with the data, call this to clear them out.
//
// ----------------------------------------------------------------------- //

void CServerSaveLoadMgr::ClearClientSaveData( )
{
	// Tell all the players to clear their client save data.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		if( pPlayerObj )
		{
			pPlayerObj->SetClientSaveData( NULL );
		}

		iter++;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::FinishSaveData
//
//	PURPOSE:	Finish the savedata state we started.
//
// --------------------------------------------------------------------------- //

bool CServerSaveLoadMgr::FinishSaveData( )
{
	TRACE( "CServerSaveLoadMgr::FinishSaveData\n" );

	// Default to success.
	bool bRet = true;

	// Put the current save data state into a temporary variable.  Then clear
	// out the savedata state member variable.  We clear it before we call
	// any more functions, because the functions called could change the state.
	SaveDataState eSaveDataState = m_eSaveDataState;
	m_eSaveDataState = eSaveDataStateNone;

	// Finish the task we started with.
	switch( eSaveDataState )
	{
		case eSaveDataStateSwitchLevels:
			bRet = FinishSwitchLevels( );
			break;
		case eSaveDataStateTransitionLevels:
			bRet = FinishTransitionLevels( );
			break;
		case eSaveDataSaveGameSlot:
		case eSaveDataQuickSave:
		case eSaveDataReloadSave:
		case eSaveDataCheckpointSave:
			bRet = FinishSaveGame( );
			break;
		default:
			ASSERT( !"CServerSaveLoadMgr::HandleSaveData: Invalid savedata state." );
			bRet = false;
			break;
	}

	// Unpause the server now that we've finished getting save data.  This matches
	// the pause in AskClientForSaveData.
	g_pGameServerShell->PauseGame( false );

	return bRet;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::FinishSwitchLevels
//
//	PURPOSE:	Finish the switch levels task.  Called after all client save
//				data has come in.
//
// --------------------------------------------------------------------------- //

bool CServerSaveLoadMgr::FinishSwitchLevels( )
{
	TRACE( "CServerSaveLoadMgr::FinishSwitchLevels\n" );

	// This is a new level in terms of the level objects loaded from the level.
	g_pGameServerShell->SetLGFlags( LOAD_NEW_LEVEL );

	if( !SaveKeepAlives( GetKeepAliveFile( GetProfileName() ) ) )
		return false;

	// Load the level with objects.
	if( !LoadLevel( m_sSaveDataNewLevel.c_str(), true ))
		return false;

	// Restore the objects with keepalives.
	if( !RestoreObjectsFromSaveFile( GetKeepAliveFile( GetProfileName()), true ) )
		return false;

	// Tell the persistent timers about the load.
	SerializeableEngineTimer::OnLoad( true );

	// Make sure mission mgr has the right settings for this level.
	g_pServerMissionMgr->SetMissionBasedOnLevel( m_sSaveDataNewLevel.c_str() );

	if( !RunWorld( ))
		return false;

	// Don't need newlevel name anymore.
	m_sSaveDataNewLevel.clear( );

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::FinishTransitionLevels
//
//	PURPOSE:	Finish the transition levels task.  Called after all client save
//				data has come in.
//
// --------------------------------------------------------------------------- //

bool CServerSaveLoadMgr::FinishTransitionLevels( )
{
	TRACE( "CServerSaveLoadMgr::FinishTransitionLevels\n" );

	// Get the current world.
	char const* pszCurWorld = g_pGameServerShell->GetCurLevel( );

	char szAbsFile[MAX_PATH*2];
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetWorldSaveFile( m_sSaveDataNewLevel.c_str(), g_pServerSaveLoadMgr->GetProfileName() ), 
		szAbsFile, LTARRAYSIZE( szAbsFile ));

	// See if we have already visited the level we are transitioning to.
	bool bRestoreLevel = CWinUtil::FileExist( szAbsFile );

	// Save the old world off.
	if( !g_pTransMgr->PreTransitionLoad( GetWorldSaveFile( pszCurWorld, g_pServerSaveLoadMgr->GetProfileName() ) ) )
	{
		ASSERT( !"CServerSaveLoadMgr::FinishTransitionLevels: Could not save world transitioned from." );
		return false;
	}

	// Always consider our level switch to be "new level".  TransitionMgr will
	// handle setting the flags for the different states.
	g_pGameServerShell->SetLGFlags( LOAD_NEW_LEVEL );

	// Load the level and only load the world objects if we don't have a save game for it.
	if( !LoadLevel( m_sSaveDataNewLevel.c_str(), !bRestoreLevel ))
		return false;

	// Load the transition objects and any save data for the new level.
	// If we've been to the world before, then use the loadgame file, otherwise pass null.
	if( !g_pTransMgr->PostTransitionLoad( bRestoreLevel ? GetWorldSaveFile( m_sSaveDataNewLevel.c_str(), g_pServerSaveLoadMgr->GetProfileName() ) : NULL ) )
	{
		ASSERT( !"CServerSaveLoadMgr::FinishTransitionLevels: Could not load world transitioned to." );
		return false;
	}

	// Make sure mission mgr has the right settings for this level.
	g_pServerMissionMgr->SetMissionBasedOnLevel( m_sSaveDataNewLevel.c_str() );

	if( !RunWorld( ))
		return false;

	// Don't need newlevel name anymore.
	m_sSaveDataNewLevel.clear( );

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::FinishSaveGame
//
//  PURPOSE:	Finish the save game.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::FinishSaveGame( )
{
	TRACE( "CServerSaveLoadMgr::FinishSaveGame\n" );

    HOBJECT hObj = NULL;

	// Save depends on the global WorldProperties object being valid (i.e.,
	// every level MUST have a WorldProperties object).
	if (!g_pWorldProperties)
	{
		ASSERT( !"CGameServerShell::FinishSaveGame:  Could not find worldproperties." );
		return false;
	}

	// Copy in the save dir.
	if( !CopyWorkingDir( m_sSaveGameDir.c_str() ) ) 
		return false;

	// Create a save list.
    ObjectList* pSaveList = g_pLTServer->CreateObjectList();
	if( !pSaveList )
	{
		ASSERT( !"CGameServerShell::FinishSaveGame:  Could not create save list object." );
		return false;
	}

	// Add active objects to the list...
    hObj = g_pLTServer->GetNextObject(NULL);
	while (hObj)
	{
		if (hObj != g_pWorldProperties->m_hObject)
		{
			// Only use this object if it's not the player or if it is the player and
			// if he has a controlling client.  The player could be missing a controlling
			// client if this is MP save/load and the player didn't come back.
			CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObj ));
			if( !pPlayerObj || pPlayerObj->GetClient( ))
				g_pLTServer->AddObjectToList(pSaveList, hObj);
		}

        hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Add inactive objects to the list...

    hObj = g_pLTServer->GetNextInactiveObject(NULL);
	while (hObj)
	{
		if (hObj != g_pWorldProperties->m_hObject)
		{
			// Only use this object if it's not the player or if it is the player and
			// if he has a controlling client.  The player could be missing a controlling
			// client if this is MP save/load and the player didn't come back.
			CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hObj ));
			if( !pPlayerObj || pPlayerObj->GetClient( ))
	            g_pLTServer->AddObjectToList(pSaveList, hObj);
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}


	// Make sure the WorldProperties object is saved FIRST, this way all the global
	// data will be available for the other objects when they get restored.
	// (ServerDE::AddObjectsToList() adds to the front of the list, so we
	// need to add it last ;)...

    g_pLTServer->AddObjectToList(pSaveList, g_pWorldProperties->m_hObject);

	bool bRet = true;

	if( pSaveList->m_nInList > 0 )
	{
        LTRESULT dResult = g_pLTServer->SaveObjects(m_sSaveGameFile.c_str(), pSaveList, LOAD_RESTORE_GAME,
												 SAVEOBJECTS_SAVEGAMECONSOLE | SAVEOBJECTS_SAVEGAMESTATE);
		if (dResult != LT_OK)
		{
            ASSERT( !"CGameServerShell::FinishSaveGame: Couldn't save objects!" );
			bRet = false;
		}
	}

    g_pLTServer->RelinquishList( pSaveList );
	pSaveList = NULL;

	if( bRet )
	{
		// Write out the continue game info.
		WriteContinueINI( m_sSaveGameKey.c_str(),
		                  m_sSaveGameDir.c_str(),
		                  m_sSaveGameFile.c_str() );

		// Log that we made the save.
		WriteSaveINI( m_sSaveGameKey.c_str(), m_wsSaveGameTitle.c_str(), g_pGameServerShell->GetCurLevel( ));

		// If we just did a reloadlevel save, then toss our checkpointsave.
		if( LTStrICmp( m_sSaveGameKey.c_str( ), RELOADLEVEL_INIKEY ) == 0 )
		{
			char szAbsFile[MAX_PATH*2];
			g_pLTBase->FileMgr()->GetAbsoluteUserFileName( GetCheckpointSaveDir( GetProfileName() ), szAbsFile, LTARRAYSIZE( szAbsFile ));

			LTFileOperations::DeleteFile( szAbsFile );

			CWinUtil::RemoveDir( szAbsFile );
			WriteSaveINI( CHECKPOINTSAVE_INIKEY, L"", "" );
		}
	}

	// Don't need client save data anymore.
	ClearClientSaveData( );

	return bRet;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::HandleSaveFailed
//
//	PURPOSE:	Handle the case that a client couldn't save
//
// --------------------------------------------------------------------------- //

void CServerSaveLoadMgr::HandleSaveFailed()
{
	// Only send error if not autosave.  Autosaves aren't controllable by player, so
	// they don't need notification.
	if( m_eSaveDataState != eSaveDataReloadSave )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SAVE_GAME );
		cMsg.Writebool(false);
		// Abort if we are doing a keepalive save.
		bool bAbort = ( m_eSaveDataState == eSaveDataStateSwitchLevels );
		cMsg.Writebool( bAbort );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}

	// the save failed so return to a normal state so the game can continue
	m_bPlayerTrackerAborted = false;
	m_SaveDataPlayerTracker.Term();

	m_eSaveDataState = eSaveDataStateNone;
	g_pGameServerShell->PauseGame( false );

}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::WaitForAutoLoadResponses
//
//	PURPOSE:	Let the 
//
// --------------------------------------------------------------------------- //

void CServerSaveLoadMgr::WaitForAutoLoadResponses( )
{
	if( m_bWaitingForAutoLoadResponses )
		return;

	m_bWaitingForAutoLoadResponses = true;

	// Build the list of current clients for the tracker...

	m_AutoLoadResponsePlayerTracker.Init( *this );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::HandleSaveFailed
//
//	PURPOSE:	Handle the case that a client couldn't save
//
// --------------------------------------------------------------------------- //

bool CServerSaveLoadMgr::FinishAutoLoad()
{
	m_bWaitingForAutoLoadResponses = false;
	m_AutoLoadResponsePlayerTracker.Term();

	return ContinueGame();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::PreStartWorld
//
//	PURPOSE:	Called before exiting a level to another level.
//
// --------------------------------------------------------------------------- //

void CServerSaveLoadMgr::PreStartWorld()
{
	// Clear out the cansave override between levels in case it doesn't get cleaned up properly by content.
	SetCanSaveOverride( true );
}
