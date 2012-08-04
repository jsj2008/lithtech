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

#include "stdafx.h"
#include "ServerSaveLoadMgr.h"
#include "MsgIds.h"
#include "WinUtil.h"
#include "TransitionMgr.h"
#include "PlayerObj.h"
#include "WorldProperties.h"
#include "ServerMissionMgr.h"

// 
// Globals...
//

CServerSaveLoadMgr *g_pServerSaveLoadMgr = LTNULL;
	


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::CServerSaveLoadMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CServerSaveLoadMgr::CServerSaveLoadMgr( )
{
	ASSERT( g_pServerSaveLoadMgr == LTNULL );

	m_eSaveDataState = eSaveDataStateNone;
	g_pServerSaveLoadMgr = this;
	m_bPlayerTrackerAborted = false;
	m_bWaitingForAutoLoadResponses = false;
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

	ASSERT( g_pServerSaveLoadMgr != LTNULL );

	g_pServerSaveLoadMgr = LTNULL;
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

	if( !LoadLevel( pszNewLevel, true ))
		return false;

	if( !RunWorld( ))
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
	if( !LoadLevel( m_sSaveDataNewLevel, true ))
		return false;

	// Make sure mission mgr has the right settings for this level.
	g_pServerMissionMgr->SetMissionBasedOnLevel( m_sSaveDataNewLevel );

	if( !RunWorld( ))
		return false;

	// Don't need newlevel name anymore.
	m_sSaveDataNewLevel.Empty( );

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

bool CServerSaveLoadMgr::LoadSaveFile( char const* pszSaveIniKey, char const* pszSaveDir, 
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
	if( !SlotSaveExists( nSlot ))
		return false;

	// Load the save file.
	if( !LoadSaveFile( GetSlotSaveKey( nSlot ), GetSlotSaveDir( nSlot ), GetSlotSaveFile( nSlot )))
		return false;

	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::SaveGameSlot
//
//  PURPOSE:	Save a named game to a specific slot.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::SaveGameSlot( int nSlot, const char* pszSaveName )
{
	if( !CanSaveGame() )
		return false;

	// Are we trying to save to the Quick save slot?
	if( nSlot == 0 ) 
	{
		ASSERT( !"CServerSaveLoadMgr::SaveGameSlot:  Invalid slot specified." );
		return false;
	}

	if( !pszSaveName )
	{
		ASSERT( !"CServerSaveLoadMgr::SaveGameSlot:  Invalid savename specified." );
		return false;
	}

	// Make sure all the folders are setup.
	if( !BuildProfileSaveDir( ))
		return false;

	// Create the slot save dir if it doesn't yet exist...

	if( !CWinUtil::FileExist( GetSlotSaveDir( nSlot ) ))
	{
		if( !CWinUtil::CreateDir( GetSlotSaveDir( nSlot ) ))
			return false;
	}

	m_sSaveGameDir = GetSlotSaveDir( nSlot );
	m_sSaveGameFile = GetSlotSaveFile( nSlot );
	m_sSaveGameKey.Format( "%s%02d", SLOTSAVE_INIKEY, nSlot );
	m_sSaveGameTitle = pszSaveName;

	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	// Ask the client for the save data and wait for ack.
	if( !AskClientsForSaveData( eSaveDataSaveGameSlot ))
		return false;

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
	if( !LoadSaveFile( QUICKSAVE_INIKEY, GetQuickSaveDir( ), GetQuickSaveFile( )))
		return false;

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
		return false;

	// Make sure all the folders are setup.
	if( !BuildProfileSaveDir( ))
		return false;

	// Create the quicksave dir if it doesn't yet exist...

	if( !CWinUtil::DirExist( GetQuickSaveDir( ) ))
	{
		if( !CWinUtil::CreateDir( GetQuickSaveDir( ) ))
			return false;
	}

	m_sSaveGameDir = GetQuickSaveDir( );
	m_sSaveGameFile = GetQuickSaveFile( );
	m_sSaveGameKey = QUICKSAVE_INIKEY;
	m_sSaveGameTitle.Format( "%i,%i", nMission, nLevel );

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
	if( !LoadSaveFile( RELOADLEVEL_INIKEY, GetReloadLevelDir( ), GetReloadLevelFile( )))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CServerSaveLoadMgr::AutoSave
//
//  PURPOSE:	Handle automatic saving.
//
// ----------------------------------------------------------------------- //

bool CServerSaveLoadMgr::AutoSave( int nMission, int nLevel )
{
	if( !CanSaveGame())
		return false;

	// Make sure all the folders are setup.
	if( !BuildProfileSaveDir( ))
		return false;

	m_sSaveGameDir = GetReloadLevelDir( );
	m_sSaveGameFile = GetReloadLevelFile( );
	m_sSaveGameKey = RELOADLEVEL_INIKEY;
	m_sSaveGameTitle.Format( "%i,%i", nMission, nLevel );

	g_pGameServerShell->SetLGFlags( LOAD_RESTORE_GAME );

	// Ask the client for the save data and wait for ack.
	if( !AskClientsForSaveData( eSaveDataAutoSave ))
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
	if( !CanContinueGame())
		return false;

	char szSaveKey[SLMGR_MAX_INIKEY_LEN];
	char szSaveDir[MAX_PATH];
	char szSaveFile[MAX_PATH];

	// Get the save info from the continue info.
	if( !ReadContinueINI( szSaveKey, ARRAY_LEN( szSaveKey ), szSaveDir, ARRAY_LEN( szSaveDir ), 
		szSaveFile, ARRAY_LEN( szSaveFile )))
		return false;

	// Load the save file.
	if( !LoadSaveFile( szSaveKey, szSaveDir, szSaveFile ))
		return false;

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
	// [KLS 8/28/02] Don't save if we're doing a performance test...
	if (g_pGameServerShell->IsPerformanceTest()) 
		return false;


	// Handle single player.
	if( !IsMultiplayerGame( ))
	{
		// Get the first and only player and check if he's still alive.
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		if( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj* pPlayerObj = *iter;
			if( !pPlayerObj )
				return true;
			if( pPlayerObj->IsDead( ))
				return false;
		}

		return true;
	}
	// Handle multiplayer.
	else
	{
		bool bCanSave = false;

		switch( g_pGameServerShell->GetGameType( ))
		{
			case eGameTypeCooperative:
				bCanSave = true;
				break;

			default:
				bCanSave = false;
				break;
		}

		return bCanSave;
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
    LTRESULT dResult = g_pLTServer->LoadWorld(( char* )pszFilename, flags);
	if( dResult != LT_OK )
		return false;

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

	// Restore time if not keepalives.
	int nFlags = ( bKeepAlives ) ? 0 : RESTOREOBJECTS_RESTORETIME;

	// Restore objects from file.
    LTRESULT dResult = g_pLTServer->RestoreObjects(( char* )pszFilename, g_pGameServerShell->GetLGFlags( ), nFlags );
	if (dResult != LT_OK)
	{
		ASSERT( !"CServerSaveLoadMgr::RestoreObjectsFromSaveFile: Could not restore objects." );
		return false;
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
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

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
//		ASSERT( !"CServerSaveLoadMgr::AskClientsForSaveData:  Already in a save state." );
		return false;
	}


	// Get the playerobj for this client.
    void *pData = g_pLTServer->GetClientUserData(hSender);
	CPlayerObj* pPlayerObjSender = (CPlayerObj*)pData;
	if( !pPlayerObjSender )
	{
		ASSERT( !"CServerSaveLoadMgr::HandleSaveData: Invalid playerobj sender." );
		return false;
	}

	//can the client save now?
	if (!msg.Readbool())
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
		case eSaveDataAutoSave:
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

	if( !SaveKeepAlives( GetKeepAliveFile( )))
		return false;

	// Load the level with objects.
	if( !LoadLevel( m_sSaveDataNewLevel, true ))
		return false;

	// Restore the objects with keepalives.
	if( !RestoreObjectsFromSaveFile( GetKeepAliveFile( ), true ))
		return false;

	// Make sure mission mgr has the right settings for this level.
	g_pServerMissionMgr->SetMissionBasedOnLevel( m_sSaveDataNewLevel );

	if( !RunWorld( ))
		return false;

	// Don't need newlevel name anymore.
	m_sSaveDataNewLevel.Empty( );

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

	// Create a save file name based on the world title.
	CString sSaveGameFile = GetWorldSaveFile( pszCurWorld );

	// Create a load file name based on the new world title.
	CString sLoadGameFile = GetWorldSaveFile( m_sSaveDataNewLevel );

	// See if we have already visited the level we are transitioning to.
	bool bRestoreLevel = !!CWinUtil::FileExist( sLoadGameFile );

	// Save the old world off.
	if( !g_pTransMgr->PreTransitionLoad( sSaveGameFile ))
	{
		ASSERT( !"CServerSaveLoadMgr::FinishTransitionLevels: Could not save world transitioned from." );
		return false;
	}

	// Always consider our level switch to be "new level".  TransitionMgr will
	// handle setting the flags for the different states.
	g_pGameServerShell->SetLGFlags( LOAD_NEW_LEVEL );

	// Load the level and only load the world objects if we don't have a save game for it.
	if( !LoadLevel( m_sSaveDataNewLevel, !bRestoreLevel ))
		return false;

	// Load the transition objects and any save data for the new level.
	// If we've been to the world before, then use the loadgame file, otherwise pass null.
	if( !g_pTransMgr->PostTransitionLoad( bRestoreLevel ? sLoadGameFile : NULL ))
	{
		ASSERT( !"CServerSaveLoadMgr::FinishTransitionLevels: Could not load world transitioned to." );
		return false;
	}

	// Make sure mission mgr has the right settings for this level.
	g_pServerMissionMgr->SetMissionBasedOnLevel( m_sSaveDataNewLevel );

	if( !RunWorld( ))
		return false;

	// Don't need newlevel name anymore.
	m_sSaveDataNewLevel.Empty( );

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

    HOBJECT hObj = LTNULL;

	// Save depends on the global WorldProperties object being valid (i.e.,
	// every level MUST have a WorldProperties object).
	if (!g_pWorldProperties)
	{
		ASSERT( !"CGameServerShell::FinishSaveGame:  Could not find worldproperties." );
		return false;
	}

	// Copy in the save dir.
	if( !CopyWorkingDir( m_sSaveGameDir )) 
		return false;

	// Create a save list.
    ObjectList* pSaveList = g_pLTServer->CreateObjectList();
	if( !pSaveList )
	{
		ASSERT( !"CGameServerShell::FinishSaveGame:  Could not create save list object." );
		return false;
	}

	// Add active objects to the list...
    hObj = g_pLTServer->GetNextObject(LTNULL);
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

    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
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
        LTRESULT dResult = g_pLTServer->SaveObjects(( char* )( char const* )m_sSaveGameFile, pSaveList, LOAD_RESTORE_GAME,
												 SAVEOBJECTS_SAVEGAMECONSOLE);
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
		WriteContinueINI( m_sSaveGameKey, m_sSaveGameDir, m_sSaveGameFile );

		// Log that we made the save.
		WriteSaveINI( m_sSaveGameKey, m_sSaveGameTitle, g_pGameServerShell->GetCurLevel( ));
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
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SAVE_GAME );
	cMsg.Writebool(false);
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

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