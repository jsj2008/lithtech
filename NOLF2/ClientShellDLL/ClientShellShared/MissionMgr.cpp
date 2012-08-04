// ----------------------------------------------------------------------- //
//
// MODULE  : MissionMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "MissionMgr.h"
#include "GameClientShell.h"
#include "VarTrack.h"
#include "MsgIds.h"
#include "WinUtil.h"
#include "MissionButeMgr.h"
#include "PlayerMgr.h"
#include "ClientSaveLoadMgr.h"
#include "clientresshared.h"
#include "ClientMultiplayerMgr.h"
#include "ProfileMgr.h"
#include "ScreenPreload.h"

extern VarTrack g_vtScreenFadeInTime;

CMissionMgr* g_pMissionMgr = LTNULL;

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
}

CMissionMgr::~CMissionMgr()
{
	Term( );

	g_pMissionMgr = LTNULL;
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

	m_sCurrentWorldName.Empty( );
	m_sNewWorldName.Empty( );

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
	g_pMissionMgr = LTNULL;
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
	int nFirstMission = 0;

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// If this is a multiplayer game, then get use the campaign file to determine
	// the first mission.
	if( IsMultiplayerGame( ))
	{
		// Get the first mission to play from the missions file.
		char szMission[4];
		CUserProfile* pUserProfile = g_pProfileMgr->GetCurrentProfile( );
		char const* pszCampaignFile = GetCampaignFile( pUserProfile->m_ServerGameOptions );
		if (!CWinUtil::FileExist( pszCampaignFile ))
			return false;

		CWinUtil::WinGetPrivateProfileString( "MissionList", "Mission0", "0", szMission, 
			ARRAY_LEN( szMission ), pszCampaignFile );
		nFirstMission = atoi( szMission );
	}

	// Get the level name for the first mission/level.
	char const* pszLevelFilename = GetLevelFromMission( nFirstMission, 0 );
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

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// [KLS 8/30/02] Make sure the server info is setup correctly...
	g_pClientMultiplayerMgr->SetupServerSinglePlayer();

	m_eStartGameState = eStartGameFromLevel;
	// Set our new level name.
	if( !SetNewLevel("worlds\\RetailSinglePlayer\\performance" ))
		return false;

	return true;
}



bool CMissionMgr::FinishStartGameFromLevel()
{
	m_eStartGameState = eStartGameStarted;

	// Make sure we have a server started.
	if( !g_pClientMultiplayerMgr->StartClientServer( ))
	{
		g_pInterfaceMgr->LoadFailed( );
		return false;
	}

	// Send the start game message.
	if( !SendStartGameMessage( ))
		return false;

	// Tell the server to start with this level.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_START_LEVEL );
	cMsg.WriteString( m_sCurrentWorldName );
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
	// Check if there was a save available.
	if( !g_pClientSaveLoadMgr->QuickSaveExists( ))
		return false;

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( QUICKSAVE_INIKEY, NULL, 0, szWorldName, ARRAY_LEN( szWorldName ),
		NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );

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
	bOk = bOk && g_pClientMultiplayerMgr->StartClientServer( );

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
		szWorldName, ARRAY_LEN( szWorldName ),	NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );

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
	bOk = bOk && g_pClientMultiplayerMgr->StartClientServer( );

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
		szWorldName, ARRAY_LEN( szWorldName ),	NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );

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
	bOk = bOk && g_pClientMultiplayerMgr->StartClientServer( );

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

	// Starting new game.
	g_pInterfaceMgr->StartingNewGame( );

	// Get the savekey in the continue info.
	char szSaveKey[SLMGR_MAX_INIKEY_LEN];
	if( !g_pClientSaveLoadMgr->ReadContinueINI( szSaveKey, ARRAY_LEN( szSaveKey ), NULL, 0, NULL, 0 ))
		return false;

	// Get the worldname for the save.
	char szWorldName[MAX_PATH];
	if( !g_pClientSaveLoadMgr->ReadSaveINI( szSaveKey, NULL, 0, 
		szWorldName, ARRAY_LEN( szWorldName ),	NULL ))
		return false;

	// Starting new mission.
	m_bNewMission = true;
	m_bRestoringLevel = false;
	m_bServerWaiting = true;
	ClearMissionInfo( );

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
	bOk = bOk && g_pClientMultiplayerMgr->StartClientServer( );

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
	if( !g_pClientMultiplayerMgr->StartClientServer( ))
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
		case MID_EXIT_LEVEL:		HandleExitLevel			( msg );	return true;
		case MID_END_GAME:			HandleEndGame			( msg );	return true;
	}

	return false;
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
	g_pInterfaceMgr->MissionFailed(IDS_YOUWEREKILLED);

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
	msg.ReadString( m_sNewWorldName.GetBuffer( MAX_PATH ), MAX_PATH );
	m_sNewWorldName.ReleaseBuffer( );
	m_bExitingMission = msg.Readbool( );
	m_bRestoringLevel = msg.Readbool( );
	m_bServerWaiting  = msg.Readbool( );

	int nMissionId, nLevel;

	// Check if this is a valid mission level.
	if (g_pMissionButeMgr->IsMissionLevel( m_sNewWorldName, nMissionId, nLevel))
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
		//  if we are the host, we are already on the preload screen
		if (g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) )
			g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);
		return true;

	}


	m_bNewMission = m_bExitingMission;

	// Check if we didn't get a new world.
	if( m_sNewWorldName.IsEmpty( ))
	{
		// Just go back to the main menu.
		g_pInterfaceMgr->ChangeState( GS_SCREEN );
		return true;
	}

	// Consider ourselves to be exiting.
	m_bExitingLevel = true;

	// Change to the exiting level state.
	g_pInterfaceMgr->ChangeState(GS_EXITINGLEVEL);

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::HandleEndGame
//
//	PURPOSE:	Handle the server telling us to prepare for exit level.
//
// --------------------------------------------------------------------------- //

bool CMissionMgr::HandleEndGame( ILTMessage_Read& msg  )
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
	else if (IsMultiplayerGame())
	{
		switch( g_pGameClientShell->GetGameType( ))
		{
			case eGameTypeCooperative:
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_END_COOP_MISSION);
				break;
			case eGameTypeDeathmatch:
			case eGameTypeTeamDeathmatch:
			case eGameTypeDoomsDay:
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_END_DM_MISSION);
				break;
		}
	}
	else
	{
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_END_MISSION);

		//record the completion
		CRegMgr* pRegMgr = g_pVersionMgr->GetRegMgr();
		if (pRegMgr->IsValid())
		{
			pRegMgr->Set("EndGame",1);
		}

	}

	return true;
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
	if( !SetNewLevel( m_sNewWorldName ))
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
	
	g_pChatMsgs->ClearHistory();
	g_pRewardMsgs->ClearHistory();
	g_pPickupMsgs->ClearHistory();
	g_pTransmission->Hide();
	
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
	if( !pszWorldName || !pszWorldName[0] )
	{
		ASSERT( !"CMissionMgr::SetNewLevel:  Invalid world name." );
		return false;
	}

	m_sCurrentWorldName = pszWorldName;

	// See if the loaded world is a custom level or not...

	int nMissionId, nLevel;

	// Check if this is a valid mission level.
	if (g_pMissionButeMgr->IsMissionLevel( pszWorldName, nMissionId, nLevel))
	{
		// Check if we're switching missions.
		if( m_bNewMission )
		{
			// Starting new mission.
			ClearMissionInfo( );
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
	g_pLTClient->ClearInput();
	
	// Toss any cameras.
	g_pPlayerMgr->TurnOffAlternativeCamera( CT_FULLSCREEN );

	g_pGameClientShell->GetPlayerMgr()->ResetCamera();

	// Consider the world not loaded.
	g_pGameClientShell->SetWorldNotLoaded( );

	// Clear the disconnect flags.
	g_pInterfaceMgr->SetIntentionalDisconnect( false );

	if (GS_LOADINGLEVEL == g_pInterfaceMgr->GetGameState())
	{
		g_pInterfaceMgr->UpdateLoadScreenInfo();
		return true;
	}

	// Change to the loading level state.
	if (!m_bServerWaiting)
	{
		//  if we are the host, we can ignore this
		if (g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ) )
			g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);

		return true;

	}
	else if( g_pInterfaceMgr->ShouldSkipPreLoad() )
	{
		// Skipping pre-load, go right into level loading...

		g_pInterfaceMgr->ChangeState( GS_LOADINGLEVEL );
	}
	else
	{
		CScreenPreload *pPreload = (CScreenPreload *) (g_pInterfaceMgr->GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
  		if (pPreload)
  		{
  			pPreload->SetWaitingToExit(false);
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);
  		}
  		else
  			ASSERT(!"No Preload screen available.");
		
	}

	return true;
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionMgr::GetLevelFromMission
//
//	PURPOSE:	Get the level name from mission info.
//
// --------------------------------------------------------------------------- //

char const* CMissionMgr::GetLevelFromMission( int nMission, int nLevel )
{
	MISSION* pMission = g_pMissionButeMgr->GetMission( nMission );
	if( !pMission )
	{
		ASSERT( !"CMissionMgr::GetLevelFromMission: Invalid mission." );
		return NULL;
	}

	if( nLevel < 0 || nLevel > pMission->nNumLevels )
	{
		ASSERT( !"CMissionMgr::GetLevelFromMission: Invalid mission info." );
		return NULL;
	}

	return pMission->aLevels[nLevel].szLevel;
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
	SAFE_STRCPY(szTmp,pszNewWorldName);
	strtok(szTmp,".");

	// Check if we already set ourselves up for this world.
//	if( !m_sCurrentWorldName.IsEmpty( ) && m_sCurrentWorldName.CompareNoCase( szTmp ) == 0 )
//		return true;

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
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	m_bGameOver = false;

	// Tell the server to start with the specified bute.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_START_GAME );

	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	
	return true;
}
