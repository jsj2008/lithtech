// ----------------------------------------------------------------------- //
//
// MODULE  : MissionMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2001-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MissionMgr.h"
#include "GameClientShell.h"
#include "VarTrack.h"
#include "MsgIds.h"
#include "WinUtil.h"
#include "MissionDB.h"
#include "PlayerMgr.h"
#include "ClientSaveLoadMgr.h"
#include "ClientConnectionMgr.h"
#include "ProfileMgr.h"
#include "ScreenPreload.h"
#include "HUDMessageQueue.h"
#include "HUDTransmission.h"
#include "HUDEndRoundMessage.h"
#include "PlayerCamera.h"
#include "ResourceExtensions.h"
#include "BindMgr.h"
#include "iltfilemgr.h"
#include "GameModeMgr.h"
#include "ltprofileutils.h"
#include "HUDScores.h"
#include "MissionDB.h"
#include "ScreenVote.h"

extern VarTrack g_vtScreenFadeInTime;

VarTrack	g_vtPerformanceTestLevelName;

CMissionMgr* g_pMissionMgr = NULL;

// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CMissionMgr::CMissionMgr()
{
	m_bCustomLevel = false;
    m_nCurrentMission = -1;
    m_nCurrentLevel = -1;
	m_bExitingLevel = false;
	m_bExitingMission = false;
	m_eStartGameState = eStartGameUnknown;
	m_nSaveSlot = -1;
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
    m_nNewMission = -1;
    m_nNewLevel = -1;
	m_bGameOver = false;
	m_eServerGameState = EServerGameState_None;
}

CMissionMgr::~CMissionMgr()
{
	Term( );

	g_pMissionMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::Init()
{
	// Start fresh.
	Term( );

	g_pMissionMgr = this;

	m_bCustomLevel = false;
    m_nCurrentMission = -1;
    m_nCurrentLevel = -1;
	m_bExitingLevel = false;
	m_bExitingMission = false;
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
    m_nNewMission = -1;
    m_nNewLevel = -1;
	m_eServerGameState = EServerGameState_None;

	m_sCurrentWorldName.clear( );
	m_sNewWorldName.clear( );

	g_vtPerformanceTestLevelName.Init(g_pLTClient, "PerformanceTestLevel", "worlds\\Release\\performance", 0.0f);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CMissionMgr::Term()
{
	g_pMissionMgr = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameNew
//
//	PURPOSE:	Start the game from the beginning.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameNew( )
{
	// Default to use the first mission.
	HRECORD hMissionRec = NULL;

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// If this is a multiplayer game, then get the first mission.
	StringSet setRequiredMapFeatures;

	if( IsMultiplayerGameClient( ))
	{
		// Get the first mission index from the options file.  If none specified, then just 
		// use the index to the first possible mission.
		char szMissionName[MAX_PATH*2];
		GameModeMgr::Instance( ).GetMissionByIndex( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ),
			0, szMissionName, LTARRAYSIZE( szMissionName ));
		hMissionRec = g_pLTDatabase->GetRecord( g_pMissionDB->GetMissionCat( ), szMissionName );
		GameModeMgr& gameModeMgr = GameModeMgr::Instance( );
		DelimitedStringToStringContainer(gameModeMgr.m_grsRequiredMapFeatures.GetValue(),setRequiredMapFeatures,",");
	}

	// Use the first valid mission in missionorder.
	if( !hMissionRec )
	{
		HATTRIBUTE hMissions = g_pMissionDB->GetAttribute( g_pMissionDB->GetMissionOrderRec(), MDB_Missions );
		uint32 nNumMissions = g_pMissionDB->GetNumValues(hMissions);
		uint32 nIndex = 0;
		do 
		{
			hMissionRec = g_pLTDatabase->GetRecordLink( hMissions, nIndex, NULL );
			if (!g_pMissionDB->CheckMPLevelRequirements(hMissionRec,setRequiredMapFeatures))
			{
				nIndex++;
				hMissionRec = NULL;
			}
			
		} while(nIndex <= nNumMissions && !hMissionRec);

		//if we still don't have one, there is trouble
		if (!hMissionRec)
		{
			LTERROR("No valid maps found for this game mode.");
			return false;
		}
		
	}

	// Use the first level of the mission.
	HRECORD hLevel = g_pMissionDB->GetLevel( hMissionRec, 0 );
	if( !hLevel )
	{
		LTERROR_PARAM1( "%s: Invalid mission.", __FUNCTION__ );
		return false;
	}

	// Get the level name for the first mission/level.
	char const* pszLevelFilename = g_pMissionDB->GetWorldName(hLevel,true);
	if( !pszLevelFilename )
		return false;

	// Start from the first level.
	if( !StartGameFromLevel( pszLevelFilename ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromLevel
//
//	PURPOSE:	Start the game from a level
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromLevel( char const* pszFilename )
{
	// Check inputs.
	if( !pszFilename || !pszFilename[0] )
	{
		ASSERT( !"CMissionMgr::StartGameFromLevel: Invalid filename." );
		return false;
	}

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );
	m_sCurrentWorldName = "";

	m_eStartGameState = eStartGameFromLevel;
	// Set our new level name.
	if( !SetNewLevel( pszFilename ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartPerformanceLevel
//
//	PURPOSE:	Start the game from the performance level
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartPerformanceLevel()
{
	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );
	m_sCurrentWorldName = "";

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Make sure the server info is setup correctly...
	g_pClientConnectionMgr->SetupServerSinglePlayer();

	m_eStartGameState = eStartGameFromLevel;
	// Set our new level name.
	if( !SetNewLevel(g_vtPerformanceTestLevelName.GetStr()) )
		return false;

	return true;
}



bool CMissionMgr::FinishStartGameFromLevel()
{
	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	if( !g_pClientConnectionMgr->StartClientServer( ))
	{
		return false;
	}

	// Send the start game message.
	if( !SendStartGameMessage( ))
		return false;

	// Tell the server to start with this level.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_START_LEVEL );
	cMsg.WriteString( m_sCurrentWorldName.c_str());
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromQuickSave
//
//	PURPOSE:	Start the game from quicksave.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromQuickSave( )
{

	//make sure mission bute mgr is initialized to the single player butes
	if (!g_pMissionDB->IsUsingDB(DB_Default_File) )
	{
		// Initialize to the sp mission bute.
		if( !g_pMissionDB->Init( DB_Default_File ))
		{
			return false;
		}
	}

	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->QuickSaveExists( ))
		return false;

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( QUICKSAVE_INIKEY, NULL, 0, szWorldName, LTARRAYSIZE( szWorldName ), NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );
	m_sCurrentWorldName = "";

	bool bOk = true;

	// Set this as the level we'll be playing.
	bOk = SetNewLevel( szWorldName );


	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}
	m_eStartGameState = eStartGameFromQuickSave;

	return true;
}


bool CMissionMgr::FinishStartGameFromQuickSave( )
{
	bool bOk = true;

	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	bOk = bOk && g_pClientConnectionMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the quickload.
	bOk = bOk && g_pClientSaveLoadMgr->QuickLoad( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromSaveSlot
//
//	PURPOSE:	Start the game from save slot.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromSaveSlot( int nSlot )
{
	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->SlotSaveExists( nSlot ))
		return false;

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( g_pClientSaveLoadMgr->GetSlotSaveKey( nSlot ), NULL, 0, 
		szWorldName, LTARRAYSIZE( szWorldName ), NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );
	m_sCurrentWorldName = "";

	bool bOk = true;

	// Set this as the level we'll be playing.
	bOk = bOk && SetNewLevel( szWorldName );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	m_nSaveSlot = nSlot;
	m_eStartGameState = eStartGameFromSaveSlot;

	return true;
}


bool CMissionMgr::FinishStartGameFromSaveSlot( )
{
	bool bOk = true;
	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	bOk = bOk && g_pClientConnectionMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the load.
	bOk = bOk && g_pClientSaveLoadMgr->LoadGameSlot( m_nSaveSlot );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromReload
//
//	PURPOSE:	Start the game from reload.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromReload( )
{
	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->ReloadSaveExists( ))
		return false;

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( RELOADLEVEL_INIKEY, NULL, 0, 
		szWorldName, LTARRAYSIZE( szWorldName ), NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );
	m_sCurrentWorldName = "";

	bool bOk = true;

	// Set this as the level we'll be playing.
	bOk = bOk && SetNewLevel( szWorldName );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	m_eStartGameState = eStartGameFromReload;

	return true;
}

bool CMissionMgr::FinishStartGameFromReload( )
{
	m_eStartGameState = eStartGameStarted;

	bool bOk = true;

	// Make sure we have a server started.
	bOk = bOk && g_pClientConnectionMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the load.
	bOk = bOk && g_pClientSaveLoadMgr->ReloadLevel( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromCheckpointSave
//
//	PURPOSE:	Start the game from checkpointsave.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromCheckpointSave( )
{
	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->CheckpointSaveExists( ))
		return false;

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( CHECKPOINTSAVE_INIKEY, NULL, 0, 
		szWorldName, LTARRAYSIZE( szWorldName ), NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );
	m_sCurrentWorldName = "";

	bool bOk = true;

	// Set this as the level we'll be playing.
	bOk = bOk && SetNewLevel( szWorldName );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	m_eStartGameState = eStartGameFromCheckpointSave;

	return true;
}

bool CMissionMgr::FinishStartGameFromCheckpointSave( )
{
	m_eStartGameState = eStartGameStarted;

	bool bOk = true;

	// Make sure we have a server started.
	bOk = bOk && g_pClientConnectionMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the load.
	bOk = bOk && g_pClientSaveLoadMgr->LoadCheckpointSave( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameFromContinue
//
//	PURPOSE:	Start the game from continue.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameFromContinue( )
{
	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->CanContinueGame( ))
		return false;

	//make sure mission bute mgr is initialized to the single player butes
	if (!g_pMissionDB->IsUsingDB(DB_Default_File) )
	{
		// Initialize to the sp mission bute.
		if( !g_pMissionDB->Init( DB_Default_File ))
		{
			return false;
		}
	}

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Get the savekey in the continue info.
	char szSaveKey[SLMGR_MAX_INIKEY_LEN];
	if( !g_pClientSaveLoadMgr->ReadContinueINI( szSaveKey, ARRAY_LEN( szSaveKey ), NULL, 0, NULL, 0 ))
		return false;

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( szSaveKey, NULL, 0, 
		szWorldName, LTARRAYSIZE( szWorldName ), NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );
	m_sCurrentWorldName = "";

	bool bOk = true;

	// Set this as the level we'll be playing.
	bOk = bOk && SetNewLevel( szWorldName );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}
	m_eStartGameState = eStartGameFromContinue;

	return true;
}

bool CMissionMgr::FinishStartGameFromContinue( )
{
	m_eStartGameState = eStartGameStarted;

	bool bOk = true;

	// Make sure we have a server started.
	bOk = bOk && g_pClientConnectionMgr->StartClientServer( );

	// Send the start game message.
	bOk = bOk && SendStartGameMessage( );

	// Do the load.
	bOk = bOk && g_pClientSaveLoadMgr->ContinueGame( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::StartGameAsClient
//
//	PURPOSE:	Start the game from remote server.
//
// ----------------------------------------------------------------------- //
bool CMissionMgr::StartGameAsClient( )
{
	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );
	m_sCurrentWorldName = "";

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// We don't know our level yet, so just go to a join loading screen.
	if( !SetLoadingLevel( ))
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}
	m_eStartGameState = eStartGameAsClient;
	return true;
}

bool CMissionMgr::FinishStartGameAsClient( )
{
	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	if( !g_pClientConnectionMgr->StartClientServer( ))
		return false;

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::OnMessage()
//
//	PURPOSE:	Handle client messages
//
// ----------------------------------------------------------------------- //

bool CMissionMgr::OnMessage( uint8 messageID, ILTMessage_Read & msg )
{

	switch(messageID)
	{
		case MID_SERVERGAMESTATE:	HandleServerGameState	( msg );	return true;
		case MID_EXIT_LEVEL:		HandleExitLevel			( msg );	return true;
		case MID_END_GAME:			HandleEndGame			( msg );	return true;
		case MID_MAPLIST:			HandleMapList			( msg );	return true;
	}

	return false;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleServerGameState
//
//	PURPOSE:	Called when the server sends its EServerGameState.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleServerGameState( ILTMessage_Read& msg  )
{
	m_eServerGameState = ( EServerGameState )msg.Readuint8( );

	switch( GetServerGameState( ))
	{
	case EServerGameState_ShowScore:
		{
			// Make sure the user score screen is not up.
			g_pScores->Show(false);

			// Show the end of round menu.
			g_pInterfaceMgr->GetSystemMenuMgr( ).SetCurrentMenu( MENU_ID_ENDROUND );
			g_pInterfaceMgr->GetSystemMenuMgr( ).Open();
		}
		break;
	case EServerGameState_PlayingSuddenDeath:
		{
			g_pInterfaceMgr->GetSystemMenuMgr( ).ExitMenus();

			// Do the transmission.
			const char* pszFilename = g_pClientSoundMgr->GetSoundFilenameFromId( "Dialogue", "IDS_SuddenDeath" );
			if( !LTStrEmpty( pszFilename ))
			{
				g_pClientSoundMgr->PlaySoundLocal(pszFilename,SOUNDPRIORITY_PLAYER_HIGH);
			}

			g_pEndRoundMessage->Show( LoadString( "IDS_SuddenDeath" ));
		}
		break;
	default:
		{
			g_pInterfaceMgr->GetSystemMenuMgr( ).ExitMenus();
		}
		break;
	}

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleMissionFailed
//
//	PURPOSE:	Handle mission failure
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleMissionFailed()
{
	g_pPlayerMgr->ClearPlayerModes();
	// [KLS 7/28/02] - Interface mgr handles forcing screen fading in
	// if necessary, doing it here may cause problems...
	///g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
	g_pInterfaceMgr->MissionFailed("IDS_YOUWEREKILLED");

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleExitLevel
//
//	PURPOSE:	Handle the server telling us to prepare for exit level.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleExitLevel( ILTMessage_Read& msg  )
{
	char pszWorldNameBuffer[MAX_PATH + 1];
	msg.ReadString( pszWorldNameBuffer, LTARRAYSIZE(pszWorldNameBuffer) );
	m_sNewWorldName = pszWorldNameBuffer;

	m_bExitingMission = msg.Readbool( );
	m_bRestoringLevel = msg.Readbool( );
	m_bServerWaiting  = msg.Readbool( );


	// Handle exiting the level when not for restoring a new level.
	if( !IsMultiplayerGameClient( ) && !m_bRestoringLevel )
	{
		// Clear out any modes that we don't want to carry over to the next level.
		g_pPlayerMgr->PreExitWorld();

		// This is a very special case check.  Normally, the player will be alive when then go to the next
		// level, or if they died, they can go to a level if they restart it.
		if(g_pPlayerMgr->IsPlayerDead())
		{
			// [KLS 7/28/02] - Interface mgr handles forcing screen fading in
			// if necessary, doing it here may cause problems...
			///g_pInterfaceMgr->ForceScreenFadeIn(g_vtScreenFadeInTime.GetFloat());
			g_pInterfaceMgr->MissionFailed("IDS_YOUWEREKILLED");
			return true;
		}
	}

	uint32 nMissionId, nLevel;

	// Check if this is a valid mission level.
	if (g_pMissionDB->IsMissionLevel( m_sNewWorldName.c_str( ), nMissionId, nLevel))
	{
		m_nNewMission = nMissionId;
		m_nNewLevel = nLevel;
	}
	else
	{
		m_nNewMission = -1;
		m_nNewLevel = -1;
	}

	//m_bServerWaiting should only be false in the case where we are loading a game,
	if (!m_bServerWaiting)
	{
		ILTClientSoundMgr *pSoundMgr = (ILTClientSoundMgr *)g_pLTClient->SoundMgr();

		pSoundMgr->RemoveAllUnusedSoundInstances();

		g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);
		return true;
	}



	m_bNewMission = m_bExitingMission;
	if( IsMultiplayerGameClient( ) )
	{
		if (!g_pClientConnectionMgr->IsClientLoggedIn())
		{
			return true;
		}
	}


	// Check if we didn't get a new world.
	if( m_sNewWorldName.empty( ))
	{
		// Just go back to the main menu.
		g_pInterfaceMgr->ChangeState( GS_SCREEN );
		return true;
	}

	// Consider ourselves to be exiting.
	m_bExitingLevel = true;

	// Make sure the loading screen isn't up anymore.  We won't be able to switch
	// out of it if it is.
	g_pInterfaceMgr->HideLoadScreen( );

	// If this is a multiplayer game, and just going between rounds, then just don't
	// bother going to a exiting level screen.
	if( IsMultiplayerGameClient( ) && !m_bNewMission )
	{
		FinishExitLevel( );
	}
	else
	{
		// Change to the exiting level state.
		g_pInterfaceMgr->ChangeState(GS_EXITINGLEVEL);
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleEndGame
//
//	PURPOSE:	Handle the server telling us to end the game.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleEndGame( ILTMessage_Read& /*msg*/  )
{
	// Go to the main menu.
	g_pInterfaceMgr->ChangeState(GS_SCREEN);

	m_bGameOver = true;

	if (g_pGameClientShell->IsRunningPerformanceTest())
	{
		//since our history was cleared by loading the level... rebuild it
		g_pInterfaceMgr->GetScreenMgr()->AddScreenToHistory( SCREEN_ID_MAIN );
		g_pInterfaceMgr->GetScreenMgr()->AddScreenToHistory( SCREEN_ID_OPTIONS );
	
		//we're done performance testing go back to performance screen
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PERFORMANCE);
	}
	else if (IsMultiplayerGameClient())
	{
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
	}
	else
	{
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_END_MISSION);

		LTProfileUtils::WriteUint32( "Game", "EndGame", 1, g_pVersionMgr->GetGameSystemIniFile());

	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleMapList
//
//	PURPOSE:	Handle the server telling us about the map rotation
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleMapList( ILTMessage_Read& msg  )
{
	ClearMapList();

	uint8 nNumMissions = msg.Readuint8( );

	char szMission[128] = "";
	wchar_t szDisplay[128] = L"";

	uint16 nMissionIndex = 0;
	for( int i = 0; i < nNumMissions; i++ )
	{
		msg.ReadString( szMission, ARRAY_LEN( szMission ));
		g_pMissionDB->GetMissionDisplayName(szMission,szDisplay,LTARRAYSIZE(szDisplay));

		m_sMapList.push_back(szDisplay);

	}

	CScreenVote *pScreen = static_cast<CScreenVote *>(g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_VOTE));
	if (pScreen)
	{
		if (!pScreen->IsBuilt())
		{
			pScreen->Build();
		}
		pScreen->UpdateMapList( );
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::ClearMapList
//
//	PURPOSE:	Clear the server's map list when we disconnect
//
// --------------------------------------------------------------------------- //

void CMissionMgr::ClearMapList(  )
{
	m_sMapList.clear();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::FinishExitLevel
//
//	PURPOSE:	Called when interfacemgr is finished doing exit level tasks.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::FinishExitLevel( )
{
	// We already handled the exiting level.
	if( !m_bExitingLevel )
		return true;

	m_bExitingLevel = false;
	m_bExitingMission = false;

	// Tell the server we're done exiting.

	if (m_bServerWaiting)
	{
		SendEmptyServerMsg( MID_EXIT_LEVEL, MESSAGE_GUARANTEED );
	}

	// Set our new level information.
	if( !SetNewLevel( m_sNewWorldName.c_str( )))
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::FinishStartGame
//
//	PURPOSE:	Called when interfacemgr is finished doing start game tasks.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::FinishStartGame( )
{
	g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);

	if( g_pChatMsgs )
	{
		g_pChatMsgs->ClearHistory();
	}

	if( g_pTransmission )
	{
		g_pTransmission->Hide();
	}

	if( g_pEndRoundMessage )
	{
		g_pEndRoundMessage->Hide();
	}

	switch (m_eStartGameState)
	{
	case eStartGameFromLevel:
		return FinishStartGameFromLevel( );
	case eStartGameFromQuickSave:
		return FinishStartGameFromQuickSave( );
	case eStartGameFromSaveSlot:
		return FinishStartGameFromSaveSlot();
	case eStartGameFromReload:
		return FinishStartGameFromReload( );
	case eStartGameFromCheckpointSave:
		return FinishStartGameFromCheckpointSave( );
	case eStartGameFromContinue:
		return FinishStartGameFromContinue( );
	case eStartGameAsClient:
		return FinishStartGameAsClient( );
	case eStartGameStarted:
		return true;
	default:
		g_pInterfaceMgr->LoadFailed( );
		return false;

	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::SetNewLevel
//
//	PURPOSE:	Initialize new level information.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::SetNewLevel( char const* pszWorldName )
{
	// Check inputs.
	if( !pszWorldName || !pszWorldName[0] || !WorldExists(pszWorldName) )
	{
		ASSERT( !"CMissionMgr::SetNewLevel:  Invalid world name." );
		return false;
	}

	if( !SetCurrentWorld( pszWorldName ))
		return false;

	// Go to the loading state.
	if( !SetLoadingLevel( ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::SetLoadingLevel
//
//	PURPOSE:	Go to loading level state.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::SetLoadingLevel( )
{
	// Clear the input buffer.
	CBindMgr::GetSingleton().ClearAllCommands();

	// Reset the camera...
	g_pPlayerMgr->GetPlayerCamera()->ResetCamera();

	// Consider the world not loaded.
	g_pGameClientShell->SetWorldNotLoaded( );

	// Clear the disconnect flags.
	g_pInterfaceMgr->SetIntentionalDisconnect( false );

	// Not exiting level.
	m_bExitingLevel = false;

	if (GS_LOADINGLEVEL == g_pInterfaceMgr->GetGameState())
	{
		g_pInterfaceMgr->UpdateLoadScreenInfo();
		return true;
	}

	// Change to the loading level state.
	if (!m_bServerWaiting)
	{
		//  if we are the host, we can ignore this
		if (g_pClientConnectionMgr->IsConnectedToRemoteServer( ) )
			g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);

		return true;

	}

	// For mp, switching between rounds on the same level doesn't change the screen.
	if( IsMultiplayerGameClient( ) && !m_bNewMission )
	{
		// Skipping pre-load, go right into level loading...
		g_pInterfaceMgr->ChangeState( GS_LOADINGLEVEL );
		return true;
	}

	g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);
	
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::PreLoadWorld
//
//	PURPOSE:	Handle pre-load world.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::PreLoadWorld( char const* pszNewWorldName )
{
	// Check inputs.
	if( !pszNewWorldName )
	{
		ASSERT( !"CMissionMgr::PreLoadWorld: Invalid inputs." );
		return false;
	}

	char szTmp[256];
	LTStrCpy(szTmp, pszNewWorldName, LTARRAYSIZE(szTmp));
	strtok(szTmp,".");

	// If we just joined a remote server, this is the first time we
	// know the new world name.
	if( !SetNewLevel( szTmp ))
		return false;

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::ClearMissionInfo
//
//	PURPOSE:	Clear the mission information when a new mission is started.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::ClearMissionInfo( )
{
	g_pPlayerStats->ClearMissionInfo();

	m_bCustomLevel = false;
    m_nCurrentMission = -1;
    m_nCurrentLevel = -1;
    m_nNewMission = -1;
    m_nNewLevel = -1;

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::SendStartGameMessage
//
//	PURPOSE:	Clear the mission information when a new mission is started.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::SendStartGameMessage( )
{
	m_bGameOver = false;

	// Tell the server to start with the specified bute.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_START_GAME );

	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	
	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::ClientHandshaking
//
//	PURPOSE:	Handles a client getting handshaking info from server.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::ClientHandshaking( char const* pszWorldName )
{
	if( pszWorldName && pszWorldName[0] )
	{
		if( !SetCurrentWorld( pszWorldName ))
			return false;
	}

	if (GS_LOADINGLEVEL == g_pInterfaceMgr->GetGameState())
	{
		g_pInterfaceMgr->UpdateLoadScreenInfo();
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::SetCurrentWorld
//
//	PURPOSE:	Sets our current world value.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::SetCurrentWorld( char const* pszWorldName )
{
	// Check inputs.
	if( !pszWorldName || !pszWorldName[0] )
	{
		ASSERT( !"CMissionMgr::SetNewLevel:  Invalid world name." );
		return false;
	}


	m_sCurrentWorldName = pszWorldName;

	// See if the loaded world is a custom level or not...

	uint32 nMissionId, nLevel;

	// Check if this is a valid mission level.
	if (g_pMissionDB->IsMissionLevel( pszWorldName, nMissionId, nLevel))
	{
		// Check if this is a valid mission level.
		if( !IsCustomLevel( ))
		{
			// Check if we're switching missions.
			if( m_bNewMission )
			{
				// Starting new mission.
				ClearMissionInfo( );
			}
		}

		m_nCurrentMission	= nMissionId;
		m_nCurrentLevel		= nLevel;
		m_nNewMission	= nMissionId;
		m_nNewLevel		= nLevel;
        m_bCustomLevel    = false;
	}
	// This is a custom level.
	else
	{
        m_bCustomLevel = true;
		m_nCurrentMission = -1;
		m_nCurrentLevel = -1;
		m_bNewMission = true;
		m_nNewMission = -1;
		m_nNewLevel = -1;
	}

	return true;
}


bool CMissionMgr::WorldExists(const char *pszWorldName)
{
	std::string sWorldFile = pszWorldName;
	sWorldFile += ".";
	sWorldFile += RESEXT_WORLD_PACKED;

	ILTInStream	*pDBStream = g_pLTBase->FileMgr()->OpenFile( sWorldFile.c_str() );
	if( !pDBStream )
	{
		return false;
	}

	// Free up the stream...

	pDBStream->Release( );
	return true;
}
