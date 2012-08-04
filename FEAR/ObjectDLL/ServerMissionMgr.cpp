// ----------------------------------------------------------------------- //
//
// MODULE  : ServerMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "ServerMissionMgr.h"
#include "MsgIDs.h"
#include "WinUtil.h"
#include "MissionDB.h"
#include "ServerSaveLoadMgr.h"
#include "PlayerObj.h"
#include "DisplayTimer.h"
#include "Spawner.h"
#include "WorldProperties.h"
#include "AIDB.h"
#include "ltprofileutils.h"
#include "GameModeMgr.h"
#include "GameStartPoint.h"
#include "PlayerObj.h"
#include "ServerConnectionMgr.h"
#include "TeamMgr.h"
#include "iltfilemgr.h"
#include "ServerDB.h"
#include "igamespy.h"

CServerMissionMgr* g_pServerMissionMgr = NULL;
VarTrack g_vtDifficultyFactorPlayerIncrease;

// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CServerMissionMgr::CServerMissionMgr()
{
	m_bCustomLevel = false;
    m_nCurrentMission = (uint32)-1;
    m_nCurrentLevel = (uint32)-1;
	m_nCurrentRound	= 0;
	m_nCurCampaignIndex = (uint32)-1;

	m_bTransitionLevels = false;
	m_bNewMission = false;
	m_bPlayerTrackerAborted = false;

	m_bMissionFailed = false;

	m_tmrStartTime.SetEngineTimer( SimulationTimer::Instance( ));
	m_tmrState.SetEngineTimer( SimulationTimer::Instance( ));
	m_eEndRoundCondition = GameModeMgr::eEndRoundCondition_None;
	m_nWinningTeam = INVALID_TEAM;
	m_hWinningPlayer = NULL;
	m_bHaveMinimumPlayersToPlay = false;
	m_bLevelStarted = false;

	m_eServerGameState = EServerGameState_None;
}

CServerMissionMgr::~CServerMissionMgr()
{
	Term( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CServerMissionMgr::Init()
{
	g_pServerMissionMgr = this;
	m_bTransitionLevels = false;
	m_bCustomLevel  = false;
	m_nCurrentMission = (uint32)-1;
	m_nCurCampaignIndex = (uint32)-1;
	m_nCurrentLevel = (uint32)-1;
	m_nCurrentRound = 0;
	m_sCurrentWorldName = "";
	m_bNewMission = false;
	m_bPlayerTrackerAborted = false;
	m_bMissionFailed = false;
	SetServerGameState( EServerGameState_None );

	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		CTeamMgr::Instance().AddTeam( 0 );
		CTeamMgr::Instance().AddTeam( 1 );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CServerMissionMgr::Term()
{
	g_pServerMissionMgr = NULL;
	SetServerGameState( EServerGameState_None );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::GetLevelFromMission
//
//	PURPOSE:	Get the level name from mission info.
//
// --------------------------------------------------------------------------- //

char const* CServerMissionMgr::GetLevelFromMission( uint32 nMission, uint32 nLevel )
{
	HRECORD hLevel = g_pMissionDB->GetLevel( nMission, nLevel );
	if( !hLevel )
	{
		ASSERT( !"CServerMissionMgr::GetLevelFromMission: Invalid mission info." );
		return NULL;
	}

	return g_pMissionDB->GetWorldName(hLevel,true);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetMissionBasedOnLevel
//
//	PURPOSE:	Sets mission info based on level.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::SetMissionBasedOnLevel( char const* pszFilename )
{
	// Check inputs.
	if( !pszFilename || !pszFilename[0] )
	{
		ASSERT( !"CServerMissionMgr::SetMissionBasedOnLevel: Invalid filename." );
		return false;
	}

	// See if the loaded world is a custom level or not...
	uint32 nMissionId, nLevel;
	if( g_pMissionDB->IsMissionLevel(( char* )pszFilename, nMissionId, nLevel ))
	{
		m_nCurrentMission	= nMissionId;
		m_nCurrentLevel		= nLevel;
		m_bCustomLevel    = false;
	}
	else
	{
		m_nCurrentMission	= (uint32)-1;
		m_nCurrentLevel		= (uint32)-1;
        m_bCustomLevel	= true;
	}

	// Save our world name.
	m_sCurrentWorldName = pszFilename;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::FindNextCampaignIndex
//
//	PURPOSE:	Finds the next occurrance of the mission in the campaign.
//
// --------------------------------------------------------------------------- //

uint32 CServerMissionMgr::FindNextCampaignIndex( uint32 nStartingCampaignIndex, uint32 nMissionIndex )
{
	uint32 nCampaignIndex = (uint32)-1;

	// Check inputs.
	if( nMissionIndex >= g_pMissionDB->GetNumMissions( ))
		return (uint32)-1;

	// Starting from a campaign index, find the next occurance of the mission.
	for( uint32 nIndex = nStartingCampaignIndex + 1; nIndex < m_Campaign.size( ); nIndex++ )
	{
		if( m_Campaign[nIndex] == nMissionIndex )
		{
			return nIndex;
		}
	}

	return (uint32)-1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Save()
//
//	PURPOSE:	Saves state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::Save( ILTMessage_Write& msg, uint32 dwSaveFlags )
{
	msg.Writeuint8( m_Campaign.size( ));
	for( Campaign::iterator iter = m_Campaign.begin( ); iter != m_Campaign.end( ); iter++ )
	{
		uint32 nMissionIndex = *iter;
		HRECORD hMission = g_pLTDatabase->GetRecordByIndex( g_pMissionDB->GetMissionCat( ), nMissionIndex );
		msg.WriteString( g_pMissionDB->GetRecordName( hMission ));
	}
	msg.Writeuint8( m_nCurCampaignIndex );

	msg.Writeuint8(m_ServerSettings.m_nMPDifficulty);
	msg.Writefloat(m_ServerSettings.m_fPlayerDiffFactor);


	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Load()
//
//	PURPOSE:	Loads state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::Load( ILTMessage_Read& msg, uint32 dwSaveFlags )
{
	m_Campaign.clear( );
	uint8 nNumCampaignEntries = msg.Readuint8( );
	char szMissionName[MAX_PATH*2];
	for( uint8 nCampaignIndex = 0; nCampaignIndex < nNumCampaignEntries; ++nCampaignIndex )
	{
		msg.ReadString( szMissionName, LTARRAYSIZE( szMissionName ));
		HRECORD hMissionRec = g_pLTDatabase->GetRecord( g_pMissionDB->GetMissionCat(), szMissionName );
		uint8 nMissionIndex = g_pLTDatabase->GetRecordIndex( hMissionRec );
		m_Campaign.push_back( nMissionIndex );
	}
	m_nCurCampaignIndex = msg.Readuint8( );

	ServerMissionSettings ss = m_ServerSettings;
	ss.m_nMPDifficulty = msg.Readuint8();
	ss.m_fPlayerDiffFactor = msg.Readfloat();
	SetServerSettings(ss);

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::OnMessage
//
//	PURPOSE:	Handle missionmgr messages.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::OnMessage( HCLIENT hSender, ILTMessage_Read& msg )
{
	msg.SeekTo(0);
	uint8 messageID = msg.Readuint8();

	switch (messageID)
	{
		case MID_START_GAME:			HandleStartGame			( hSender, msg );	return true;
		case MID_START_LEVEL:			HandleStartLevel		( hSender, msg );	return true;
		case MID_EXIT_LEVEL:			HandleExitLevel			( hSender, msg );	return true;
		case MID_MULTIPLAYER_OPTIONS:	HandleMultiplayerOptions( hSender, msg );	return true;

		default:
			break;
	}

	return false;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleStartGame
//
//	PURPOSE:	Handle start game message.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::HandleStartGame( HCLIENT hSender, ILTMessage_Read& msg )
{
	ServerMissionSettings ss = m_ServerSettings;
	ss.m_nMPDifficulty = (uint8)g_pGameServerShell->GetDifficulty();
	SetServerSettings(ss);

	// Start the game.
	if( !StartGame(msg ))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SERVERAPP_FAILEDTOSTART );
		CLTMsgRef_Read cMsgRef = cMsg.Read(); 
		g_pLTServer->SendToServerApp( *cMsgRef );

		return false;
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleStartLevel
//
//	PURPOSE:	Start a level from scratch.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::HandleStartLevel( HCLIENT hSender, ILTMessage_Read& msg )
{
	char szStartLevel[MAX_PATH];

	msg.ReadString( szStartLevel, ARRAY_LEN( szStartLevel ));

	// Starting level from scratch, so this is a new mission for us.
	m_bNewMission = true;

	// Setup mission based on level.
	bool bOk = SetMissionBasedOnLevel( szStartLevel );

	if( bOk )
	{
		// Setup the campaign index.
		if( !m_bCustomLevel )
			m_nCurCampaignIndex = FindNextCampaignIndex( (uint32)-1, m_nCurrentMission );

		// Load the level.
		bOk = g_pServerSaveLoadMgr->LoadNewLevel( szStartLevel );
	}

	// If we failed, tell the server app, if we have one.
	if( !bOk )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( SERVERAPP_FAILEDTOLOADWORLD );
		cMsg.WriteString( szStartLevel );
		CLTMsgRef_Read cMsgRef = cMsg.Read(); 
		g_pLTServer->SendToServerApp( *cMsgRef );
	}

	return bOk;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleMultiplayerOptions
//
//	PURPOSE:	Handle updating the host options
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg )
{
	GameModeMgr::Instance().ReadFromMsg( msg );

	GameRulesUpdated.DoNotify();
	UpdateTimeLimit();

	// Resend the new options to all the other clients.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_MULTIPLAYER_OPTIONS );
	GameModeMgr::Instance().WriteToMsg( cMsg );
	SendToClientsExcept( *cMsg.Read( ), hSender, MESSAGE_GUARANTEED );

	return true;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::LevelStarted()
//
//	PURPOSE:	Called when the clients have finished loading and have started
//				playing a level
//
// ----------------------------------------------------------------------- //

void CServerMissionMgr::LevelStarted()
{
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		// Check if this is a new level, or just another round.
		if( m_bNewMission )
		{
			CTeamMgr::Instance( ).NewLevel( );
		}
		else
		{
			CTeamMgr::Instance( ).NewRound( );
		}
	}

	// Start the grace period timer that allows people to join and
	// respawn.
	if( GameModeMgr::Instance().m_grnJoinGracePeriod > 0 )
	{
		SetServerGameState( EServerGameState_GracePeriod );
	}
	// No grace period, just start the game.
	else
	{
		SetServerGameState( EServerGameState_Playing );

	}

	m_bLevelStarted = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevelSwitch()
//
//	PURPOSE:	Called when it's time to go to the next level in the mission.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevelSwitch( )
{
	// Check if we're already exiting a level.
	if( GetServerGameState() == EServerGameState_ExitingLevel )
		return true;

	// If this was a custom level, then the game is over.
	if( m_bCustomLevel )
	{
		// Let the endgame code figure out what we should do.
		if( !EndGame( ))
			return false;

		return true;
	}

	// Get the mission.
	HRECORD hMission = g_pMissionDB->GetMission( m_nCurrentMission );
	if( !hMission )
	{
        ASSERT( !"CServerMissionMgr::ExitLevelSwitch:  Invalid mission." );
		return false;
	}

	// Go to the next level.
	m_nCurrentLevel++;
	m_nCurrentRound = 0;

	// Default to not a new mission.
	m_bNewMission = false;

	// See if we finished a mission.
	if( m_nCurrentLevel >= g_pMissionDB->GetNumValues(hMission,MDB_Levels) )
	{
		// Start level index over and advance the mission index.
		m_nCurrentLevel = 0;

		// See if we weren't playing a campaign.
		if( m_nCurCampaignIndex == ( uint32 )-1 )
		{
			if( !EndGame( ))
				return false;

			return true;
		}

		m_nCurCampaignIndex++;

		// See if we reached the end of our campaign.
		if( m_nCurCampaignIndex >= m_Campaign.size( ))
		{
			// Let the endgame code figure out what we should do.
			if( !EndGame( ))
				return false;

			return true;
		}

		// Get the missionid from the campaign.
		m_nCurrentMission = m_Campaign[m_nCurCampaignIndex];

		// Flag that this is a new mission so we can tell everyone else.
		m_bNewMission = true;
	}

	// Get the worldname from the mission/level indices.
	char const* pszFilename = GetLevelFromMission( m_nCurrentMission, m_nCurrentLevel );
	if( !pszFilename )
		return false;

	// Do the exit level.
	if( !ExitLevel( pszFilename, false ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::NextMission()
//
//	PURPOSE:	Called to force transition to the next mission.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::NextMission( )
{
	// Check if we're already exiting a level.
	if( GetServerGameState() == EServerGameState_ExitingLevel )
		return true;

	// If this was a custom level, then the game is over.
	if( m_bCustomLevel )
	{
		// Let the endgame code figure out what we should do.
		if( !EndGame( ))
			return false;

		return true;
	}

	// Get the mission.
	HRECORD hMission = g_pMissionDB->GetMission(m_nCurrentMission);
	if( !hMission )
	{
        ASSERT( !"CServerMissionMgr::NextMission:  Invalid mission." );
		return false;
	}

	// Start level index over and advance the mission index.
	m_nCurrentLevel = 0;
	m_nCurrentRound = 0;

	// See if we weren't playing a campaign.
	if( m_nCurCampaignIndex == ( uint32 )-1 )
	{
		if( !EndGame( ))
			return false;

		return true;
	}

	m_nCurCampaignIndex++;

	// See if we reached the end of our campaign.
	if( m_nCurCampaignIndex >= m_Campaign.size( ))
	{
		// Let the endgame code figure out what we should do.
		if( !EndGame( ))
			return false;

		return true;
	}

	// Get the missionid from the campaign.
	m_nCurrentMission = m_Campaign[m_nCurCampaignIndex];

	// Flag that this is a new mission so we can tell everyone else.
	m_bNewMission = true;

	// Get the worldname from the mission/level indices.
	char const* pszFilename = GetLevelFromMission( m_nCurrentMission, m_nCurrentLevel );
	if( !pszFilename )
		return false;

	// Do the exit level.
	if( !ExitLevel( pszFilename, false ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::NextRound()
//
//	PURPOSE:	Called to force switch to next round, if on last round will call NextMission()...
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::NextRound( )
{
	// write a scoring entry if necessary
	if (IsMultiplayerGameServer())
	{
		if (GameModeMgr::Instance().m_ServerSettings.m_bEnableScoringLog)
		{
			WriteScoresToLog();
		}
	}

	// Check if we're already exiting a level.
	
	if( GetServerGameState() == EServerGameState_ExitingLevel )
		return true;


	// Get the mission...

	HRECORD hMission = g_pMissionDB->GetMission(m_nCurrentMission);
	if( !hMission )
	{
        ASSERT( !"CServerMissionMgr::NextMission:  Invalid mission." );
		return false;
	}

	//check for team switching
	if( GameModeMgr::Instance().m_grbUseTeams && GameModeMgr::Instance().m_grbSwitchTeamsBetweenRounds)
	{
		// Iterate through the clients and find the player.
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
		for( ; iter != gameClientDataList.end( ); iter++ )
		{
			GameClientData* pGameClientData = *iter;
			if( !pGameClientData->GetClient( ))
				continue;
			uint8 nTeam = pGameClientData->GetLastTeamId(); 
			if (nTeam != INVALID_TEAM)
			{
				nTeam = (nTeam + 1) % MAX_TEAMS;
				pGameClientData->SetRequestedTeam(nTeam);
			}
		}
	}

	// Advance to the next round...
	if( GetEndRoundCondition() != GameModeMgr::eEndRoundCondition_Restart )
	{
		++m_nCurrentRound;
		EndRound.DoNotify();
	}

	// Default to not a new mission.
	m_bNewMission = false;

	// See if we finished the rounds for this level...
	
 	if( m_nCurrentRound >= GameModeMgr::Instance( ).m_grnNumRounds )
	{
		// Just go to the next mission...

		if( !NextMission() )
			return false;

		EndMap.DoNotify();
		return true;
	}

	if( !ExitLevel( m_sCurrentWorldName.c_str(), false ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SwitchToCampaignIndex
//
//	PURPOSE:	Called to force transition to the specific mission within campaign.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::SwitchToCampaignIndex( uint32 nCampaignIndex )
{
	m_nCurCampaignIndex = nCampaignIndex - 1;
	return NextMission( );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevelToLevel()
//
//	PURPOSE:	Called when you want to go to any arbitrary level.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevelToLevel( char const* pszNewLevel )
{
	// Check if we're already exiting a level.
	if( GetServerGameState() == EServerGameState_ExitingLevel )
		return true;

	// Verify inputs.
	if( !pszNewLevel )
	{
		ASSERT( !"CServerMissionMgr::ExitLevelToLevel: Invalid inputs." );
		return false;
	}

	// Default to this not being a mission change.
	m_bNewMission = false;

	// Check if the new level is a new mission.
	uint32 nCurMission = m_nCurrentMission;
	if( !SetMissionBasedOnLevel( pszNewLevel ))
		return false;

	// If we switch to a custom level, then consider it a new mission.
	if( m_bCustomLevel )
	{
		m_bNewMission = true;
		m_nCurCampaignIndex = (uint32)-1;
	}
	// If it's not a custom world, check if our mission changed.
	else
	{
		m_bNewMission = ( nCurMission != m_nCurrentMission );

		// If we had a mission switch, advance our campaign index.
		if( m_bNewMission )
		{
			// See if we can find the campaign index from our previous position.
			m_nCurCampaignIndex = FindNextCampaignIndex( m_nCurCampaignIndex, m_nCurrentMission );
		}
	}

	// Do the exit level.
	if( !ExitLevel( pszNewLevel, false ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevelToSavedGame()
//
//	PURPOSE:	Called before you load a game to notify clients
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevelToSavedGame( char const* pszNewLevel )
{
	// Check if we're already exiting a level.
	if( GetServerGameState() == EServerGameState_ExitingLevel )
		return true;

	// Verify inputs.
	if( !pszNewLevel )
	{
		ASSERT( !"CServerMissionMgr::ExitLevelToSavedGame: Invalid inputs." );
		return false;
	}

	// Default to this not being a mission change.
	m_bNewMission = false;

	// Check if the new level is a new mission.
	uint32 nCurMission = m_nCurrentMission;
	if( !SetMissionBasedOnLevel( pszNewLevel ))
		return false;

	// If we switch to a custom level, then consider it a new mission.
	if( m_bCustomLevel )
	{
		m_bNewMission = true;
		m_nCurCampaignIndex = (uint32)-1;
	}
	// If it's not a custom world, check if our mission changed.
	else
	{
		m_bNewMission = ( nCurMission != m_nCurrentMission );

		// If we had a mission switch, advance our campaign index.
		if( m_bNewMission )
		{
			// See if we can find the campaign index from our previous position.
			m_nCurCampaignIndex = FindNextCampaignIndex( m_nCurCampaignIndex, m_nCurrentMission );
		}
	}

	// Tell clients we're exiting. Don't wait for their response.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_EXIT_LEVEL );
	cMsg.WriteString( m_sCurrentWorldName.c_str() );
	cMsg.Writebool( m_bNewMission );
	cMsg.Writebool( true );
	cMsg.Writebool( false );
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevelTransition()
//
//	PURPOSE:	Called when level transition is needed.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevelTransition( uint32 nNewLevel )
{
	// Check if we're already exiting a level.
	if( GetServerGameState() == EServerGameState_ExitingLevel )
		return true;
	
	// Default to this not being a mission change.
	m_bNewMission = false;

	// Get the next level name.
	char const* pszFilename = GetLevelFromMission( m_nCurrentMission, nNewLevel );
	if( !pszFilename )
		return false;

	// Do the exit level.
	if( !ExitLevel( pszFilename, true ))
		return false;

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::ExitLevel()
//
//	PURPOSE:	Called by exitlevelXXX routines to go to a new level.
//
// ----------------------------------------------------------------------- //

bool CServerMissionMgr::ExitLevel( char const* pszNewLevel, bool bTransitionToLevel )
{
	// Verify inputs.
	if( !pszNewLevel )
	{
		ASSERT( !"CServerMissionMgr::ExitLevel: Invalid inputs." );
		return false;
	}

	// Check if we're already exiting a level.
	if( GetServerGameState() == EServerGameState_ExitingLevel )
		return true;

	// Clear out the can save override between levels, in case content forgot.
	g_pServerSaveLoadMgr->SetCanSaveOverride( true );

	// If this is a single player game, then don't allow the level to exit if the player
	// is dead or dying.  They'll get a mission failure instead.
	if( !IsMultiplayerGameServer( ))
	{
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		if( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj const* pPlayerObj = *iter;
			if( pPlayerObj )
			{
				if( pPlayerObj->IsDead())
					return false;
			}
		}
	}

	// Make sure there is a worldproperties object.
	if (!g_pWorldProperties)
	{
		ASSERT( !"CServerMissionMgr::ExitLevel:  Could not find worldproperties." );
		return false;
	}
	
	// Tell worldproperties to send its level end command.
	g_pWorldProperties->SendLevelEndCmd( );

	// Record that we are transitioning.
	m_bTransitionLevels = bTransitionToLevel;

	// Make sure mission information is setup for the new level.  The
	// campaign index should have already been updated.
	if( !SetMissionBasedOnLevel( pszNewLevel ))
		return false;

	SetServerGameState( EServerGameState_ExitingLevel );

	// Initialize the PlayerTracker to the current set of players.  
	m_bPlayerTrackerAborted = false;
	if( !m_PlayerTracker.Init( *this ))
	{
		// No players, just go to the next level.
		if (!FinishExitLevel( ))
		{
			return false;
		}

		// make sure the server is paused until the next player joins
		g_pGameServerShell->PauseGame(true);

		return true;
	}

	// Tell the client.
	if( !SendExitLevelMessage( ))
		return false;

	// Pause the server while we switch levels.
	g_pGameServerShell->PauseGame( true );

	// Not in level any more, stop timing.
	m_tmrStartTime.Stop( );
	m_bHaveMinimumPlayersToPlay = false;

	
	return true;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleExitLevel
//
//	PURPOSE:	Handle the client's response to exit level.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::HandleExitLevel( HCLIENT hSender, ILTMessage_Read& msg )
{
	// Ignore the message if we're not waiting for exit.
	if( GetServerGameState() != EServerGameState_ExitingLevel )
		return true;

	// Get the playerobj for this client.
	CPlayerObj* pPlayerObjSender = GetPlayerFromHClient( hSender );
	if( !pPlayerObjSender )
	{
		ASSERT( !"CServerMissionMgr::HandleExitLevel: Invalid playerobj sender." );
		return false;
	}

	// Remove this player from the tracker.
	m_PlayerTracker.RemoveClient( hSender );

	// Don't bother waiting for all players to send exit level.  Just finish exiting
	// on the first one.  We don't want one player to be able to hold up the whole server.
	return FinishExitLevel( );
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::OnPlayerTrackerAbort
//
//	PURPOSE:	Handle when we have to abort a playertracker.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::OnPlayerTrackerAbort( )
{
	// Tell update to handle the playertrackerabort.  We can't continue
	// with the savedata right now because the player is still in the
	// process of getting deleted.
	m_bPlayerTrackerAborted = true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::Update
//
//	PURPOSE:	Frame update.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::Update( )
{
	switch( GetServerGameState( ))
	{
	case EServerGameState_None:
		break;
	case EServerGameState_Loading:
		break;
	case EServerGameState_GracePeriod:
		{
			// Check if the grace timer ended.
			if(!m_tmrState.IsStarted() || m_tmrState.IsTimedOut())
			{
				SetServerGameState( EServerGameState_Playing );

				// Respawn existing players.  If no one respawns, then go to the next round.
				bool bRespawns = false;
				CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
				for( ; iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
				{
					CPlayerObj* pPlayerObj = const_cast< CPlayerObj* >( *iter );
					if ( pPlayerObj->GetClient() && pPlayerObj->IsSpectating())
					{
						GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
						if( !pGameClientData )
							continue;

						// Skip clients that are not ready to play yet.
						if( !pGameClientData->IsReady( ))
							continue;

						pPlayerObj->Respawn( );
						bRespawns = true;
					}
				}

				// If no players, then just go to the next round.
				if( !bRespawns )
				{
					NextRound( );
				}
			}
		}
		break;
	case EServerGameState_Playing:
		{
			// See if we've reached the time limit.
			CheckTimeLimitWin( );
		}
		break;
	case EServerGameState_PlayingSuddenDeath:
		{
			// See if we've reached the time limit.
			CheckTimeLimitWin( );
		}
		break;
	case EServerGameState_EndingRound:
		{
			// Go to the next round if our end round time is up.
			if( !m_tmrState.IsStarted( ) || m_tmrState.IsTimedOut( ))
			{
				// Skip showing the score if it's a restart.
				if( m_eEndRoundCondition == GameModeMgr::eEndRoundCondition_Restart )
				{
					m_tmrState.Stop( );
					NextRound();
				}
				else
				{
					SetServerGameState( EServerGameState_ShowScore );
				}
			}
		}
		break;
	case EServerGameState_ShowScore:
		{
			// Go to the next round if our end round time is up.
			if( !m_tmrState.IsStarted( ) || m_tmrState.IsTimedOut( ))
			{
				m_tmrState.Stop( );
				NextRound();
			}
		}
		break;
	case EServerGameState_ExitingLevel:
		{
			// Update the playertracker.
			m_PlayerTracker.Update( );

			// Since all the players dropped out, we can just finish what we
			// were doing.
			if( m_bPlayerTrackerAborted )
			{
				// Clear the abort signal.
				m_bPlayerTrackerAborted = false;
				FinishExitLevel( );
			}
		}
		break;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::FinishExitLevel
//
//	PURPOSE:	Finish the exitlevel we started.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::FinishExitLevel( )
{
	// No longer waiting for exitlevel messages back from client.
	SetServerGameState( EServerGameState_Loading );

	// Clear out the playertracker.
	m_PlayerTracker.Term( );

	// Haven't started new level yet.
	m_bLevelStarted = false;

	// Tell the players to handle an exit level.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		pPlayerObj->HandleExit( m_bNewMission );
		iter++;
	}
	
	// Have client reselect team and loadout between maps but not between rounds.
	if( IsMultiplayerGameServer( ) && m_bNewMission )
	{
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
		for( ; iter != gameClientDataList.end( ); iter++ )
		{
			GameClientData* pGameClientData = *iter;
			if( !pGameClientData )
				continue;

			pGameClientData->SetLastTeamId( INVALID_TEAM );
			pGameClientData->SetRequestedTeam( INVALID_TEAM );
			pGameClientData->SetLoadout( (uint8)-1 );
		}
	}

	// Save the keepalives between levels if we're not switching to the same level.
	bool bSaveKeepAlives = !IsMultiplayerGameServer( ) && !LTStrEquals( m_sCurrentWorldName.c_str(), g_pGameServerShell->GetCurLevel());
	
	m_bTransitionLevels = m_bTransitionLevels && !IsMultiplayerGameServer( );

	if( m_bNewMission )
	{
		// Clear working save dir since we're starting fresh.
		g_pServerSaveLoadMgr->ClearWorkingDir( );

		// If this is mp, then consider this to be a new game.  Saves between
		// missions is only available in sp which allows skills to be saved.
		if( IsMultiplayerGameServer( ))
		{
			// Tell the gameservershell it's a new game.
			g_pGameServerShell->SetLGFlags( LOAD_NEW_GAME );
			bSaveKeepAlives = false;
		}
	}

	// Default to failure.
	bool bRet = false;

	// Do transition.
	if( m_bTransitionLevels )
	{
		m_bTransitionLevels = false;
		bRet = g_pServerSaveLoadMgr->TransitionLevels( m_sCurrentWorldName.c_str() );
	}
	// Do switch.
	else
	{
		// Don't carry slowmo across levels.
		g_pGameServerShell->ExitSlowMo( false, 0.0f );

		bRet = g_pServerSaveLoadMgr->SwitchLevels( m_sCurrentWorldName.c_str(), bSaveKeepAlives );
	}

	// Unpause the server now that we're done switching levels.
	g_pGameServerShell->PauseGame( false );	

	return bRet;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SendExitLevelMessage
//
//	PURPOSE:	Tell client to exit level.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::SendExitLevelMessage( )
{
	// Tell the players to prepare to exit a level.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		pPlayerObj->HandlePreExit();
		g_pGameServerShell->SendPlayerInfoMsgToClients(NULL,pPlayerObj,MID_PI_UPDATE);
		iter++;
	}

	CTeamMgr::Instance().UpdateClient();

	// See if we have already visited the level we are going to.
	char szRelLoadGameFile[MAX_PATH*2];
	char szAbsLoadGameFile[MAX_PATH*2];
	LTStrCpy( szRelLoadGameFile, g_pServerSaveLoadMgr->GetWorldSaveFile( m_sCurrentWorldName.c_str(), g_pServerSaveLoadMgr->GetProfileName() ), LTARRAYSIZE( szRelLoadGameFile ));
	g_pLTBase->FileMgr()->GetAbsoluteUserFileName( szRelLoadGameFile, szAbsLoadGameFile, LTARRAYSIZE( szAbsLoadGameFile ));
	bool bRestoringLevel = CWinUtil::FileExist( szAbsLoadGameFile );

	// Tell client's we're exiting.  Wait for their response.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_EXIT_LEVEL );
	cMsg.WriteString( m_sCurrentWorldName.c_str() );
	cMsg.Writebool( m_bNewMission );
	cMsg.Writebool( bRestoringLevel );
	cMsg.Writebool( true );
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::EndGame
//
//	PURPOSE:	The game has ended.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::EndGame( )
{
	// Make sure there is a worldproperties object.
	if (!g_pWorldProperties)
	{
		ASSERT( !"CServerMissionMgr::EndGame:  Could not find worldproperties." );
	}
	else
	{
		// Tell worldproperties to send its level end command.
		g_pWorldProperties->SendLevelEndCmd( );
	}

	if( IsMultiplayerGameServer( ))
	{
		// Can't have custom level or no campaign in deathmatch.
		if( m_bCustomLevel || m_Campaign.size( ) == 0 )
		{
			LTERROR( "Can't have custom level or no campaign in mp." );
			return false;
		}

		// Start from the beginning.
		m_nCurrentLevel = 0;
		m_nCurCampaignIndex = 0;
		m_nCurrentMission = m_Campaign[m_nCurCampaignIndex];
		m_bNewMission = true;

		// Get the worldname from the mission/level indices.
		char const* pszFilename = GetLevelFromMission( m_nCurrentMission, m_nCurrentLevel );
		if( !pszFilename )
			return false;

		// Exit to the level.
		if( !ExitLevel( pszFilename, false ))
			return false;
	}
	else
	{
		// Tell the players to prepare to exit a level.
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj* pPlayerObj = *iter;
			pPlayerObj->HandlePreExit();
			iter++;
		}

		// Tell the client the game is over.
		SendEmptyClientMsg( MID_END_GAME, NULL, MESSAGE_GUARANTEED );
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::StartGame
//
//	PURPOSE:	Default start game.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::StartGame(ILTMessage_Read& msg )
{
	char path[MAX_PATH*2];
	if( IsMultiplayerGameServer( ))
	{
		LTFileOperations::GetUserDirectory(path, LTARRAYSIZE(path));
		LTStrCat( path, MDB_MP_File, LTARRAYSIZE( path ));
		if( !g_pMissionDB->Init( path ))
			return false;
	}
	else
	{
		if( !g_pMissionDB->Init( DB_Default_File ))
			return false;
	}

	// Setup the campaign from the campaign file.
	SetupCampaign( );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetupCampaign
//
//	PURPOSE:	Setup the campaign list of missions.  The campaign
//				defines which missions we will play.  Having a blank
//				campaign file means that all missions will be used.
//
// --------------------------------------------------------------------------- //

bool CServerMissionMgr::SetupCampaign( )
{
	// Clear all missions from our campaign.
	m_Campaign.clear( );

	// Read the missionid's for this campaign.
	char szMissionName[MAX_PATH*2];
	uint32 nIndex = 0;
	for( ;; )
	{
		if( !GameModeMgr::Instance( ).GetMissionByIndex( g_pGameServerShell->GetNetGameInfo( ).m_sServerOptionsFile.c_str( ),
			nIndex, szMissionName, LTARRAYSIZE( szMissionName )))
			break;

		// If there wasn't a missionname, then we reached the end.
		if( LTStrEmpty( szMissionName ))
			break;

		// Get the mission id.
		HRECORD hMissionRec = g_pLTDatabase->GetRecord( g_pMissionDB->GetMissionCat( ), szMissionName );
		// Make sure it's a valid mission.
		if( hMissionRec )
			m_Campaign.push_back( g_pLTDatabase->GetRecordIndex( hMissionRec ));

		nIndex++;
	}

	// If no missions are defined, then just use all the missions.
	if( m_Campaign.empty( ))
	{
		StringSet setRequiredMapFeatures;
		if (IsMultiplayerGameServer())
		{
			GameModeMgr& gameModeMgr = GameModeMgr::Instance( );
			DelimitedStringToStringContainer(gameModeMgr.m_grsRequiredMapFeatures.GetValue(),setRequiredMapFeatures,",");
		}

		// Use the missionorder record that defines the order of missions to play.
		HATTRIBUTE hMissions = g_pMissionDB->GetAttribute( g_pMissionDB->GetMissionOrderRec(), MDB_Missions );
		uint32 nNumMissions = g_pLTDatabase->GetNumValues( hMissions );
		// Iterate through all the missions referenced in the missionorder and add them to the campaign.
		for( uint8 nOrderIndex = 0; nOrderIndex < nNumMissions; nOrderIndex++ )
		{
			HRECORD hMissionRec = g_pLTDatabase->GetRecordLink( hMissions, nOrderIndex, NULL );
			if( !hMissionRec )
				continue;

			if (IsMultiplayerGameServer() && !g_pMissionDB->CheckMPLevelRequirements(hMissionRec,setRequiredMapFeatures))
			{
				continue;
			}

			// Use the index into the mission category, not the missionorder category.  Missionorder category is 
			// just needed to define the order of the campaign.
			m_Campaign.push_back( g_pLTDatabase->GetRecordIndex( hMissionRec ));
		}

		return true;
	}

	return true;
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetMissionFailed()
//
//	PURPOSE:	Set or clear mission failed state
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::SetMissionFailed(bool bFail)
{
	if (bFail != m_bMissionFailed)
	{
		g_pGameServerShell->PauseGame(bFail);

		if( bFail )
		{
			// Send a message to the serverapp letting it know the mission failed.
			// If a ServerApp doesn't exist then nothing happens, otherwise GameServerShell::ServerAppMessageFn()
			// will recieve a message and handle it.
	
			CAutoMessage cMsg;
			cMsg.Writeuint8( SERVERAPP_MISSIONFAILED );

			CLTMsgRef_Read cMsgRef = cMsg.Read(); 
			g_pLTServer->SendToServerApp( *cMsgRef );
		}
}
	m_bMissionFailed = bFail; 
};


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetServerSettings
//
//	PURPOSE:	Sets friendly fire info.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::SetServerSettings(ServerMissionSettings& ServerSettings)
{ 
	if (!g_vtDifficultyFactorPlayerIncrease.IsInitted())
	{
		g_vtDifficultyFactorPlayerIncrease.Init( g_pLTServer, "DifficultyFactorPlayerIncrease", NULL, g_pAIDB->GetAIConstantsRecord()->fDifficultyFactorPlayerIncrease );
	}

	m_ServerSettings = ServerSettings;

	if (GameModeMgr::Instance( ).m_grbUsesDifficulty)
	{
		g_pGameServerShell->SetDifficulty((GameDifficulty)ServerSettings.m_nMPDifficulty);
	}
	else
		g_pGameServerShell->SetDifficulty(GD_NORMAL);
	

	g_vtDifficultyFactorPlayerIncrease.SetFloat(ServerSettings.m_fPlayerDiffFactor);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CurrentWinningTeamData
//
//	PURPOSE:	Data structure to gather information on winning status for
//				Team style games.
//
// --------------------------------------------------------------------------- //
struct CurrentWinningTeamData
{
	CurrentWinningTeamData( )
	{
		m_nTotalLivePlayers = 0;
		m_nLivePlayers[0] = 0;
		m_nLivePlayers[1] = 0;
		m_nWinningTeam = INVALID_TEAM;
	}

	uint8 m_nTotalLivePlayers;
	uint8 m_nLivePlayers[2];
	uint8 m_nWinningTeam;
};

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	FindCurrentWinningTeam
//
//	PURPOSE:	Finds the current winning team.
//
// --------------------------------------------------------------------------- //
static bool FindCurrentWinningTeam( CurrentWinningTeamData& data )
{ 
	// Check if not using teams.
	if( !GameModeMgr::Instance().m_grbUseTeams )
	{
		LTERROR( "Invalid team mode." );
		return false;
	}

	// Initialize to no winner.
	data.m_nWinningTeam = INVALID_TEAM;

	// If elimination, then team wins if other team eliminated.
	if( GameModeMgr::Instance().m_grbEliminationWin )
	{
		// Check which players are eliminated off teams.
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
		for( ; iter != gameClientDataList.end( ); iter++ )
		{
			GameClientData* pGameClientData = *iter;
			if( !pGameClientData )
				continue;

			CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->HandleToObject( pGameClientData->GetPlayer( ));
			if( !pPlayerObj )
				continue;

			// Ignore them if they're not alive.
			if( !pPlayerObj->IsAlive())
				continue;

			// Record their life without team designation.
			data.m_nTotalLivePlayers++;

			// Record their life for their team.
			if( pPlayerObj->GetTeamID() != INVALID_TEAM )
			{
				data.m_nLivePlayers[pPlayerObj->GetTeamID()]++;
			}
		}

		// Check if team 0 has been eliminated.
		if( !data.m_nLivePlayers[0] )
		{
			// Team 1 wins.
			data.m_nWinningTeam = 1;
		}
		// Check if team 1 has been eliminated.
		else if( !data.m_nLivePlayers[1] )
		{
			// Team 0 wins.
			data.m_nWinningTeam = 0;
		}
	}

	// No clear winner yet, check by score.
	if( data.m_nWinningTeam == INVALID_TEAM )
	{
		// Check if one team has a higher score. Or if a tie, leave team winner invalid.
		int32 nTeamScore0 = CTeamMgr::Instance().GetTeam(0)->GetScore();
		int32 nTeamScore1 = CTeamMgr::Instance().GetTeam(1)->GetScore();
		if( nTeamScore0 > nTeamScore1 )
		{
			data.m_nWinningTeam = 0;
		}
		else if( nTeamScore0 < nTeamScore1 )
		{
			data.m_nWinningTeam = 1;
		}
	}

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CurrentWinningPlayerSort
//
//	PURPOSE:	Functor to determine who is currently winning a DM game.
//
// --------------------------------------------------------------------------- //
struct CurrentWinningPlayerSort
{
public:

	bool operator()( GameClientData* pX, GameClientData* pY ) const
	{
		CPlayerObj* pPlayerObjX = ( CPlayerObj* )g_pLTServer->HandleToObject( pX->GetPlayer( ));
		CPlayerObj* pPlayerObjY = ( CPlayerObj* )g_pLTServer->HandleToObject( pY->GetPlayer( ));

		if( !pPlayerObjY )
		{
			return true;
		}
		else if( !pPlayerObjX )
		{
			return false;
		}

		if( GameModeMgr::Instance().m_grbEliminationWin )
		{
			bool bAliveX = ( pPlayerObjX->IsAlive( ));
			bool bAliveY = ( pPlayerObjY->IsAlive( ));

			// if X is alive and Y isn't, X is better
			if( bAliveX && !bAliveY )
			{
				return true;
			}
			//if Y is alive and X isn't, Y is better
			if( bAliveY && !bAliveX )
			{
				return false;
			}
		}

		// Check scores to see who is better.
		return pX->GetPlayerScore()->GetScore() > pY->GetPlayerScore()->GetScore();
	}
};


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CurrentWinningPlayerData
//
//	PURPOSE:	Data structure to gather information on winning status for
//				DM style games.
//
// --------------------------------------------------------------------------- //
struct CurrentWinningPlayerData
{
	CurrentWinningPlayerData( )
	{
		m_nTotalPlayers = 0;
		m_nLivePlayers = 0;
		m_pWinningGameClientData = NULL;
	}

	uint8 m_nTotalPlayers;
	uint8 m_nLivePlayers;
	GameClientData* m_pWinningGameClientData;
};

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	FindCurrentWinningPlayerDM
//
//	PURPOSE:	Finds the current winning player of a DM style game.
//
// --------------------------------------------------------------------------- //
static bool FindCurrentWinningPlayerDM( CurrentWinningPlayerData& data )
{ 
	// Check if using teams.
	if( GameModeMgr::Instance().m_grbUseTeams )
	{
		LTERROR( "Invalid team mode." );
		return false;
	}

	// Sort the players.
	ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );

	std::vector< GameClientData* > lstSortedPlayers;
	lstSortedPlayers.reserve( gameClientDataList.size( ));
	ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
	for( ; iter != gameClientDataList.end( ); iter++ )
	{
		GameClientData* pGameClientData = *iter;
		if( !pGameClientData )
			continue;

		lstSortedPlayers.push_back( pGameClientData );
	}
	std::stable_sort( lstSortedPlayers.begin(), lstSortedPlayers.end( ), CurrentWinningPlayerSort( ));

	// Count the total number of live players.
	for( uint32 nIndex = 0; nIndex < lstSortedPlayers.size( ); nIndex++ )
	{
		CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->HandleToObject( lstSortedPlayers[nIndex]->GetPlayer( ));
		if( pPlayerObj && pPlayerObj->IsAlive( ))
		{
			data.m_nLivePlayers++;
		}
	}
	data.m_nTotalPlayers = lstSortedPlayers.size( );

	// Check if list is empty.
	if( lstSortedPlayers.empty( ))
		return true;

	// If only one client, then he's the winner.
	if( lstSortedPlayers.size( ) == 1 )
	{
		data.m_pWinningGameClientData = lstSortedPlayers[0];
		return true;
	}

	// Check for tie.  We know we have more than 1 client at this point.  If tie, then leave winning NULL.
	GameClientData* pWinningGameClientData = lstSortedPlayers[0];
	GameClientData* pSecondGameClientData = lstSortedPlayers[1];
	CurrentWinningPlayerSort sort;
	// If both first and second are equivalent, then there's a tie.
	if( !sort( pWinningGameClientData, pSecondGameClientData ) && !sort( pSecondGameClientData, pWinningGameClientData ))
		return true;

	data.m_pWinningGameClientData = lstSortedPlayers[0];
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::CheckScoreLimitWin
//
//	PURPOSE:	Checks if an eliminationwin has been achieved.
//
// --------------------------------------------------------------------------- //

struct ScoreLimitTeam
{
	CTeam* m_pTeam;
	uint8 m_nTeamId;

	// Return true if this score is higher than other score.
	bool operator<( ScoreLimitTeam const& other ) const
	{
		return ( m_pTeam->GetScore() > other.m_pTeam->GetScore());
	}
};

struct ScoreLimitPlayer
{
	GameClientData* m_pGameClientData;

	// Return true if this score is higher than other score.
	bool operator<( ScoreLimitPlayer const& other ) const
	{
		return ( m_pGameClientData->GetPlayerScore( )->GetScore() > other.m_pGameClientData->GetPlayerScore( )->GetScore());
	}
};

void CServerMissionMgr::CheckScoreLimitWin( )
{ 
	if( IsEndRoundConditionMet( ))
		return;

	// Check if there is a score limit or the timer hasn't started yet.
	if( GameModeMgr::Instance( ).m_grnScoreLimit == 0 || !m_tmrStartTime.IsStarted( ))
		return;

	// Check score limit for teams.
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		// Check if there is an invalid number of teams.
		if( CTeamMgr::Instance().GetNumTeams() < 2 )
			return;

		// Find the team with the highest score.
		std::vector< ScoreLimitTeam > lstScoreLimitTeam;
		for( uint8 i = 0; i < CTeamMgr::Instance().GetNumTeams(); ++i )
		{
			ScoreLimitTeam scoreLimitTeam;
			scoreLimitTeam.m_pTeam = CTeamMgr::Instance().GetTeam( i );
			scoreLimitTeam.m_nTeamId = i;
			lstScoreLimitTeam.push_back( scoreLimitTeam );
		}

		std::sort( lstScoreLimitTeam.begin( ), lstScoreLimitTeam.end( ));
		CTeam* pWinningTeam = NULL;
		uint8 nWinningTeamId = INVALID_TEAM;
		// Check if there wasn't a tie for first.
		if( lstScoreLimitTeam[0] < lstScoreLimitTeam[1] )
		{
			pWinningTeam = lstScoreLimitTeam[0].m_pTeam;
			nWinningTeamId = lstScoreLimitTeam[0].m_nTeamId;
		}

		// Check if score limit reached or if we're in sudden death.
		if( pWinningTeam && ( pWinningTeam->GetScore() >= ( int32 )GameModeMgr::Instance( ).m_grnScoreLimit ||
			m_eServerGameState == EServerGameState_PlayingSuddenDeath ) )
		{
			SetTeamWon( nWinningTeamId );
			g_pLTServer->CPrint( "ScoreLimit reached." );
			if( m_eServerGameState == EServerGameState_PlayingSuddenDeath )
			{
				SetEndRoundCondition( GameModeMgr::eEndRoundCondition_SuddenDeathScore );
			}
			else
			{
				SetEndRoundCondition( GameModeMgr::eEndRoundCondition_ScoreLimit );
			}
		}
	}
	// Check score limit for dm style.
	else
	{
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );

		// Check if there is an invalid number of players.
		if( gameClientDataList.size( ) < 1 )
			return;

		// Find the player with the highest score.
		std::vector< ScoreLimitPlayer > lstScoreLimitPlayer;
		ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
		for( ; iter != gameClientDataList.end( ); iter++ )
		{
			ScoreLimitPlayer scoreLimitPlayer;
			scoreLimitPlayer.m_pGameClientData = *iter;
			if( !scoreLimitPlayer.m_pGameClientData )
				continue;
			lstScoreLimitPlayer.push_back( scoreLimitPlayer );
		}

		std::sort( lstScoreLimitPlayer.begin( ), lstScoreLimitPlayer.end( ));
		GameClientData* pWinningPlayer = NULL;
		// Check if there wasn't a tie for first.
		if( lstScoreLimitPlayer.size( ) == 1 || lstScoreLimitPlayer[0] < lstScoreLimitPlayer[1] )
		{
			pWinningPlayer = lstScoreLimitPlayer[0].m_pGameClientData;
		}

		// Check if score limit reached or if we're in sudden death.
		if( pWinningPlayer && ( pWinningPlayer->GetPlayerScore()->GetScore() >= ( int32 )GameModeMgr::Instance( ).m_grnScoreLimit ||
			m_eServerGameState == EServerGameState_PlayingSuddenDeath ))
		{
			g_pLTServer->CPrint("ScoreLimit reached.");
			m_hWinningPlayer = pWinningPlayer->GetPlayer();
			SetEndRoundCondition( GameModeMgr::eEndRoundCondition_ScoreLimit );
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::CheckTimeLimitWin
//
//	PURPOSE:	Checks if an timelimit has been achieved.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::CheckTimeLimitWin( )
{ 
	// No winning based on time in sp.
	if( !IsMultiplayerGameServer( ))
		return;

	if( IsEndRoundConditionMet( ))
		return;

	// Check if we haven't started playing yet.
	if( !m_tmrStartTime.IsStarted( ))
		return;

	// Check if there is a time limit.
	if( !m_tmrState.IsStarted( ))
		return;

	double fTimeLeft = m_tmrState.GetTimeLeft();

	// If we ran out of time, then go to the next round.
	if( fTimeLeft > 0.0 )
		return;

	g_pLTServer->CPrint( "TimeLimit reached." );

	bool bFoundWinner = false;

	// Check for teams.
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		CurrentWinningTeamData data;
		FindCurrentWinningTeam( data );
		SetTeamWon( data.m_nWinningTeam );
		if( data.m_nWinningTeam != INVALID_TEAM )
		{
			bFoundWinner = true;
		}
	}
	// Check DM style.
	else
	{
		// Find the player with the highest score and count him as winner.
		CurrentWinningPlayerData data;
		FindCurrentWinningPlayerDM( data );
		if( data.m_pWinningGameClientData )
		{
			m_hWinningPlayer = data.m_pWinningGameClientData->GetPlayer( );
			bFoundWinner = true;
		}
	}

	if( GetServerGameState( ) == EServerGameState_Playing )
	{
		// If we found a winner or if there is no sudden death, we can end the round.
		if( bFoundWinner || GameModeMgr::Instance( ).m_grnSuddenDeathTimeLimit == 0 )
		{
			SetEndRoundCondition( GameModeMgr::eEndRoundCondition_TimeLimit );
		}
		// Go to sudden death.
		else
		{
			SetServerGameState( EServerGameState_PlayingSuddenDeath );
		}
	}
	// Doing sudden death, so we just end in a tie.
	else
	{
		SetEndRoundCondition( GameModeMgr::eEndRoundCondition_TimeLimit );
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::CheckEliminationWin
//
//	PURPOSE:	Checks if an eliminationwin has been achieved.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::CheckEliminationWin( )
{ 
	if( IsEndRoundConditionMet( ))
		return;

	// Make sure that elimination is a condition for a win.
	if( !GameModeMgr::Instance( ).m_grbEliminationWin || !m_tmrStartTime.IsStarted( ))
		return;

	// Check for teams.
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		CurrentWinningTeamData data;
		if( !FindCurrentWinningTeam( data ))
			return;

		// Check if team 0 has been eliminated.
		if( !data.m_nLivePlayers[0] )
		{
			// Team 1 wins.
			SetTeamWon( data.m_nWinningTeam );
			SetEndRoundCondition( GameModeMgr::eEndRoundCondition_Elimination );
		}
		// Check if team 1 has been eliminated.
		else if( !data.m_nLivePlayers[1] )
		{
			// Team 0 wins.
			SetTeamWon( data.m_nWinningTeam );
			SetEndRoundCondition( GameModeMgr::eEndRoundCondition_Elimination );
		}
	}
	// No teams.
	else
	{
		CurrentWinningPlayerData data;
		if( !FindCurrentWinningPlayerDM( data ))
			return;

		// If there aren't 2 people left, then the round is over.
		if( data.m_nLivePlayers < 2 )
		{
			if( data.m_pWinningGameClientData )
			{
				m_hWinningPlayer = data.m_pWinningGameClientData->GetPlayer( );
			}

			SetEndRoundCondition( GameModeMgr::eEndRoundCondition_Elimination );
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetTeamWon
//
//	PURPOSE:	Sets a team as the winner.
//
// --------------------------------------------------------------------------- //
void CServerMissionMgr::SetTeamWon( uint8 nTeamId )
{
	m_nWinningTeam = nTeamId;
	if( m_nWinningTeam != INVALID_TEAM )
	{
		CTeamMgr::Instance( ).WonRound( m_nWinningTeam );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::PostStartWorld
//
//	PURPOSE:	Called when the server finishes loading a world.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::PostStartWorld( )
{
	// Make sure we start with no end round condition.
	SetEndRoundCondition( GameModeMgr::eEndRoundCondition_None );

	//Clear the mission failed flag (and unpause if it had paused us)
	SetMissionFailed(false);

	SetServerGameState( EServerGameState_Loading );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetEndRoundCondition
//
//	PURPOSE:	Sets that the round has ended due to a victory condition.
//
// --------------------------------------------------------------------------- //

void CServerMissionMgr::SetEndRoundCondition( GameModeMgr::EndRoundCondition eEndRoundCondition )
{
	// Set the condition.
	m_eEndRoundCondition = eEndRoundCondition;

	// Check if we're setting a valid condition.
	if( m_eEndRoundCondition != GameModeMgr::eEndRoundCondition_None )
	{
		SetServerGameState( EServerGameState_EndingRound );
	}
	else
	{
		// No end round condition met.
		m_nWinningTeam = INVALID_TEAM;
		m_hWinningPlayer = NULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetDisplayTimer
//
//	PURPOSE:	Sets the time left on the countdown timer.
//
// --------------------------------------------------------------------------- //
bool CServerMissionMgr::SetDisplayTimer( double fCountDownTime )
{
	DisplayTimer* pDisplayTimer = NULL;

	// Check if we need to create one.
	if( !m_hDisplayTimer )
	{
		// Get a valid position in the level.
		LTRotation rRot;
		LTVector vPos( 0.0f, 0.0f, 0.0f );
		if( !GameStartPoint::GetStartPointList().empty( ))
		{
			GameStartPoint* pGameStartPoint = GameStartPoint::GetStartPointList( )[0];
			g_pLTServer->GetObjectPos( pGameStartPoint->m_hObject, &vPos );
		}

		pDisplayTimer = ( DisplayTimer* )SpawnObject( "DisplayTimer", vPos, rRot );
		if( !pDisplayTimer )
		{
			LTERROR( "Could not create a display timer." );
			return false;
		}

		m_hDisplayTimer = pDisplayTimer->m_hObject;
	}
	// We already have a display timer.
	else
	{
		pDisplayTimer = ( DisplayTimer* )g_pLTServer->HandleToObject( m_hDisplayTimer );
		if( !pDisplayTimer )
		{
			LTERROR( "Invalid display timer." );
			return false;
		}
	}

	// Set our time.
	pDisplayTimer->Start( fCountDownTime );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::WriteRoundScoresToLog
//
//	PURPOSE:	Writes the end-of-round scores to the log
//
// --------------------------------------------------------------------------- //
void CServerMissionMgr::WriteScoresToLog()
{
	char szEntryLine[g_nScoringEntryBufferSize] = { 0 };

	// write the map name
	g_pGameServerShell->WriteMultiplayerScoringLogEntry("");
	LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "*** Results for Map: %s", m_sCurrentWorldName.c_str());
	g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);
	g_pGameServerShell->WriteMultiplayerScoringLogEntry("");

	// call the appropriate method based on whether this is a team game
	if (GameModeMgr::Instance().m_grbUseTeams)
	{
		uint8 nNumTeams = CTeamMgr::Instance().GetNumTeams();

		// process the teams
		for (uint8 nIndex = 0; nIndex < nNumTeams; ++nIndex)
		{
			CTeam* pTeam = CTeamMgr::Instance().GetTeam(nIndex);

			// team name
			LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Team: %s", MPW2A(pTeam->GetName()).c_str());
			g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

			// team score
			LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Score: %d", pTeam->GetScore());
			g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

			// space between team and first player
			g_pGameServerShell->WriteMultiplayerScoringLogEntry("");

			// process players
			const PlayerIDSet& cPlayerIDSet = pTeam->GetPlayerIDSet();

			for (PlayerIDSet::const_iterator itPlayer = cPlayerIDSet.begin(); itPlayer != cPlayerIDSet.end(); ++itPlayer)
			{
				WritePlayerEntryToLog(g_pLTServer->GetClientHandle((uint32)*itPlayer));
			}
		}

		// space between teams
		g_pGameServerShell->WriteMultiplayerScoringLogEntry("");
	}
	else
	{
		// iterate over the player list
		// Check each client to see if it any has exceeded the score.
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
		for( ; iter != gameClientDataList.end( ); iter++ )
		{
			GameClientData* pGameClientData = *iter;
			if( !pGameClientData )
				continue;

			WritePlayerEntryToLog(pGameClientData->GetClient());
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::WritePlayerEntryToLog
//
//	PURPOSE:	Writes the end-of-round scores to the log for a player.
//
// --------------------------------------------------------------------------- //
void CServerMissionMgr::WritePlayerEntryToLog(HCLIENT hClient)
{
	char szEntryLine[g_nScoringEntryBufferSize] = { 0 };

	CPlayerObj*     pPlayer			= GetPlayerFromHClient(hClient);
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData(hClient);

	NetClientData   sNetClientData;
	pGameClientData->GetNetClientData(sNetClientData);

	// player name and UID
	const char* pszUserCDKeyHash = NULL;
	g_pGameServerShell->GetGameSpyServer()->GetUserCDKeyHash(g_pLTServer->GetClientID(hClient), pszUserCDKeyHash);
	LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Player: %s (uid: %s)", MPW2A(sNetClientData.m_szName).c_str(), pszUserCDKeyHash);
	g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

	CPlayerScore* pScore = pGameClientData->GetPlayerScore();

	// player score
	LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Score: %d", pScore->GetScore());;
	g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

	// player kills
	LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Kills: %d", pScore->GetEventCount(CPlayerScore::eKill));
	g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

	// player deaths
	LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Deaths: %d", pScore->GetEventCount(CPlayerScore::eDeath));
	g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

	// player team kills
	LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Team Kills: %d", pScore->GetEventCount(CPlayerScore::eTeamKill));
	g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

	// player suicides
	LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Suicides: %d", pScore->GetEventCount(CPlayerScore::eSuicide));
	g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

	// player objective
	LTSNPrintF(szEntryLine, LTARRAYSIZE(szEntryLine), "Objective: %d", pScore->GetEventCount(CPlayerScore::eObjective));
	g_pGameServerShell->WriteMultiplayerScoringLogEntry(szEntryLine);

	// space between players
	g_pGameServerShell->WriteMultiplayerScoringLogEntry("");
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::CheckMinimumPlayersToPlay
//
//	PURPOSE:	Checks if we have enough players to play.
//
// --------------------------------------------------------------------------- //
bool CServerMissionMgr::CheckMinimumPlayersToPlay( )
{
	uint32 nNumClientsOnTeam[MAX_TEAMS];
	memset( nNumClientsOnTeam, 0, sizeof( nNumClientsOnTeam ));
	uint32 nNumClientsOnTeamReady[MAX_TEAMS];
	memset( nNumClientsOnTeamReady, 0, sizeof( nNumClientsOnTeamReady ));
	uint32 nNumClients = 0;
	uint32 nNumClientsReady = 0;

	// Iterate through all the clients and see if anyone is ready to play.
	ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
	ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
	for( ; iter != gameClientDataList.end( ); iter++ )
	{
		GameClientData* pGameClientData = *iter;
		if( !pGameClientData->GetClient( ))
			continue;

		// Find out what team they used to be on.
		nNumClients++;
		if( pGameClientData->GetLastTeamId() < MAX_TEAMS )
		{
			nNumClientsOnTeam[pGameClientData->GetLastTeamId()]++;
		}

		// Skip clients that aren't ready to play yet.
		if( pGameClientData->GetClientConnectionState() != eClientConnectionState_InWorld )
			continue;

		// Client must be ready to play.
		if( !pGameClientData->IsReady( ))
			continue;

		// Count them as ready.
		nNumClientsReady++;
		if( pGameClientData->GetLastTeamId( ) < MAX_TEAMS )
		{
			nNumClientsOnTeamReady[pGameClientData->GetLastTeamId( )]++;
		}
	}

	// If no one is ready, then we can't play.
	if( nNumClientsReady == 0 )
		return false;

	// If this is a team game, make sure that there is at
	// least one person on each team ready to play.
	if( GameModeMgr::Instance( ).m_grbUseTeams)
	{
		for( uint8 i = 0; i < MAX_TEAMS; i++ )
		{
			// If there are people on this team, but no one's ready, we need to wait.
			if( nNumClientsOnTeamReady[i] == 0 )
			{
				return false;
			}
		}
	}
	// Non team games.
	else
	{
		// If we have elimination, then we need at least 2 people to continue.  If
		// we only have one client, then that will have to do.
		if( !GameModeMgr::Instance().m_grbAllowRespawnFromDeath )
		{
			if( nNumClientsReady < 2 )
				return false;
		}
		// Deathmatch like game, just need one client.
		else
		{
			if( nNumClientsReady == 0 )
				return false;
		}
	}

	// Ready to play.
	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::OnPlayerInWorld
//
//	PURPOSE:	Handles when a player changes his status in the world.
//
// --------------------------------------------------------------------------- //
void CServerMissionMgr::OnPlayerInWorld( CPlayerObj& player )
{
	if( IsMultiplayerGameServer( ))
	{
		if( !m_bLevelStarted && GetServerGameState() == EServerGameState_Loading )
		{
			LevelStarted( );
		}

		// Check if we haven't already reached the minimum condition to play.
		if( !m_bHaveMinimumPlayersToPlay )
		{
			// Check if we now have enough to play.
			m_bHaveMinimumPlayersToPlay = CheckMinimumPlayersToPlay();
			if( m_bHaveMinimumPlayersToPlay )
			{
				// If we already completed the grace period, we're going to need to restart so
				// that all players start even.
				if( GetServerGameState( ) == EServerGameState_Playing )
				{
					if( GameModeMgr::Instance().m_grnJoinGracePeriod > 0 )
					{
						SetEndRoundCondition( GameModeMgr::eEndRoundCondition_Restart );
					}
				}
			}
		}
	}
	else if( GetServerGameState() == EServerGameState_Loading )
	{
		LevelStarted( );
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::SetServerGameState
//
//	PURPOSE:	Handles when a player changes his status in the world.
//
// --------------------------------------------------------------------------- //
void CServerMissionMgr::SetServerGameState( EServerGameState eValue )
{
	// Check if state hasn't changed.
	if( GetServerGameState( ) == eValue )
		return;

	m_eServerGameState = eValue;

	// Send the state to the clients.
	SendServerGameState( NULL );

	// Do special processing for the state we're entering.
	switch( GetServerGameState( ))
	{
	case EServerGameState_None:
		{
		}
		break;
	case EServerGameState_Loading:
		{
		}
		break;
	case EServerGameState_GracePeriod:
		{
			// Don't start the start time until grace period is over.
			m_tmrStartTime.Stop( );

			// Start our state timer.
			float fTime = ( float )GameModeMgr::Instance( ).m_grnJoinGracePeriod;
			m_tmrState.Start( fTime );
			SetDisplayTimer( fTime );
		}
		break;
	case EServerGameState_Playing:
		{
			// Consider the game statarted.
			m_tmrStartTime.Start( );

			// Check if we have a time limit.
			if( GameModeMgr::Instance( ).m_grnTimeLimit > 0 )
			{
				float fTimeLeft = ((float)GameModeMgr::Instance( ).m_grnTimeLimit * 60.0f );
				m_tmrState.Start( fTimeLeft );
				SetDisplayTimer( fTimeLeft );
			}
			else
			{
				m_tmrState.Stop( );
			}
			BeginRound.DoNotify();

		}
		break;
	case EServerGameState_PlayingSuddenDeath:
		{
			// Check if we have a sudden death time limit.
			if( GameModeMgr::Instance( ).m_grnSuddenDeathTimeLimit > 0 )
			{
				float fTimeLeft = ((float)GameModeMgr::Instance( ).m_grnSuddenDeathTimeLimit * 60.0f );
				m_tmrState.Start( fTimeLeft );
				SetDisplayTimer( fTimeLeft );
			}
			else
			{
				SetServerGameState( EServerGameState_EndingRound );
			}
		}
		break;
	case EServerGameState_EndingRound:
		{
			// Stay in the level for a short time.
			m_tmrState.Start( GameModeMgr::Instance().m_grfEndRoundMessageTime );

			// Tell clients about the win.
			CAutoMessage cClientMsg;
			cClientMsg.Writeuint8(MID_PLAYER_EVENT);
			cClientMsg.Writeuint8(kPEEndRoundCondition);
			cClientMsg.Writeuint8( m_eEndRoundCondition );
			cClientMsg.Writeuint8( m_nWinningTeam );
			uint8 nClientId = (uint8)-1;
			CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->HandleToObject( m_hWinningPlayer );
			if( pPlayerObj )
			{
				nClientId = g_pLTServer->GetClientID( pPlayerObj->GetClient( ));
			}
			cClientMsg.Writeuint8( nClientId );
			g_pLTServer->SendToClient(cClientMsg.Read(), NULL, MESSAGE_GUARANTEED);
		}
		break;
	case EServerGameState_ShowScore:
		{
			// Stay in the level for a short time.
			m_tmrState.Start( GameModeMgr::Instance().m_grfEndRoundScoreScreenTime );

			uint32 nNumStartPoints = GameStartPoint::GetStartPointList().size( );
			GameStartPoint* pGameStartPoint = ( nNumStartPoints > 0 ) ? 
				GameStartPoint::GetStartPointList()[GetRandom( 0, nNumStartPoints - 1 )] : NULL;
			LTVector vPos;
			LTRotation rRot;
			if( pGameStartPoint )
			{
				g_pLTServer->GetObjectPos(pGameStartPoint->m_hObject, &vPos);
				g_pLTServer->GetObjectRotation( pGameStartPoint->m_hObject, &rRot );
			}

			// Put all players into spectator mode at the first start point.
			CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
			for( ; iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
			{
				CPlayerObj* pPlayerObj = const_cast< CPlayerObj* >( *iter );
				if ( !pPlayerObj->GetClient( ))
					continue;

				pPlayerObj->SetSpectatorMode( eSpectatorMode_Fixed, true );

				if( pGameStartPoint )
				{
					g_pLTServer->SetObjectTransform( pPlayerObj->m_hObject, LTRigidTransform(vPos, rRot));
					pPlayerObj->TeleportClientToServerPos( true );
				}
			}
		}
		break;
	case EServerGameState_ExitingLevel:
		{
		}
		break;
	}
}


void CServerMissionMgr::SendServerGameState( HCLIENT hClient )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SERVERGAMESTATE);
	cMsg.Writeuint8((uint8)GetServerGameState( ));
	g_pLTServer->SendToClient(cMsg.Read( ), hClient, MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::UpdateTimeLimit
//
//	PURPOSE:	Updates the time limit on the server based on game options
//
// --------------------------------------------------------------------------- //
void CServerMissionMgr::UpdateTimeLimit()
{
	// Check if we have a time out.
	if( GameModeMgr::Instance( ).m_grnTimeLimit > 0 )
	{
		// Check if we've started the time limit.
		if( m_tmrStartTime.IsStarted( ))
		{
			double fTimeLeft = ((double)GameModeMgr::Instance( ).m_grnTimeLimit * 60.0f - m_tmrStartTime.GetElapseTime());
			m_tmrState.Start( fTimeLeft );
			SetDisplayTimer( fTimeLeft );
		}
	}
	// No timeout, make sure we kill the timer.
	else
	{
		// Check if we've started the time limit.
		if( m_tmrStartTime.IsStarted( ))
		{
			SetDisplayTimer( 0.0f );
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::GetTimeInRound
//
//	PURPOSE:	Retrieve the time spent in the current round, returns 0 if not in a playing state or no timer is started
//
// --------------------------------------------------------------------------- //
double CServerMissionMgr::GetTimeInRound()
{
	if( ( GameModeMgr::Instance( ).m_grnTimeLimit > 0 ) && m_tmrStartTime.IsStarted( ))
	{
		return m_tmrStartTime.GetElapseTime();
	}

	return 0.0;
}

