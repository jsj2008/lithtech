// ----------------------------------------------------------------------- //
//
// MODULE  : DeathMatchMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "DeathMatchMissionMgr.h"
#include "PlayerObj.h"
#include "ServerSaveLoadMgr.h"
#include "MissionButeMgr.h"

extern CVarTrack g_vtNetFriendlyFire;

// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CDeathMatchMissionMgr::CDeathMatchMissionMgr()
{
}

CDeathMatchMissionMgr::~CDeathMatchMissionMgr()
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CDeathMatchMissionMgr::Init()
{
	if( !CServerMissionMgr::Init( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CDeathMatchMissionMgr::Term()
{
	CServerMissionMgr::Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Save()
//
//	PURPOSE:	Saves state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::Save( ILTMessage_Write& msg, uint32 dwSaveFlags )
{
	if( !CServerMissionMgr::Save( msg, dwSaveFlags ))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Load()
//
//	PURPOSE:	Loads state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::Load( ILTMessage_Read& msg, uint32 dwSaveFlags )
{
	if( !CServerMissionMgr::Load( msg, dwSaveFlags ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::Update
//
//	PURPOSE:	Frame update.
//
// --------------------------------------------------------------------------- //

void CDeathMatchMissionMgr::Update( )
{
	//check for end of level...

	//check score limit
	if ( m_ServerSettings.m_nScoreLimit > 0 && m_fStartTime >= 0.0f )
	{
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj* pPlayerObj = *iter;
			if (pPlayerObj->GetPlayerScore()->GetScore() >= m_ServerSettings.m_nScoreLimit)
			{
				g_pLTServer->CPrint("ScoreLimit reached.");
				NextRound();
			}
			iter++;
		}
	}

	CServerMissionMgr::Update( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::FinishExitLevel
//
//	PURPOSE:	Finish the exitlevel we started.
//
// --------------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::FinishExitLevel( )
{
	TRACE( "CDeathMatchMissionMgr::FinishExitLevel:\n" );

	// No longer waiting for exitlevel messages back from client.
	m_bExitingLevel = false;

	// Clear out the playertracker.
	m_PlayerTracker.Term( );

	// Tell the players to handle an exit level.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		pPlayerObj->HandleExit( true );
		iter++;
	}

	// Load new level with no restoring of keepalives or save games.
	if( !g_pServerSaveLoadMgr->LoadNewLevel( m_sCurrentWorldName ))
		return false;

	// Unpause the server now that we're done switching levels.
	g_pGameServerShell->PauseGame( LTFALSE );	


	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::StartGame
//
//	PURPOSE:	Start the game.
//
// --------------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::StartGame(ILTMessage_Read& msg )
{

	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = false;
	sms.m_bFriendlyFire = true;
	sms.m_nRunSpeed = sgo.GetDeathmatch().m_nRunSpeed;
	sms.m_nScoreLimit = sgo.GetDeathmatch().m_nScoreLimit;
	sms.m_nTimeLimit = sgo.GetDeathmatch().m_nTimeLimit;
	sms.m_nRounds = sgo.GetDeathmatch().m_nRounds;
	sms.m_nFragScore = sgo.GetDeathmatch().m_nFragScore;
	sms.m_nTagScore = sgo.GetDeathmatch().m_nTagScore;

	SetServerSettings(sms);

	if( !g_pMissionButeMgr->Init( MISSION_DM_FILE ))
		return false;

	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDeathMatchMissionMgr::EndGame
//
//	PURPOSE:	End the game.
//
// --------------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::EndGame( )
{
	// Call base.
	if( !CServerMissionMgr::EndGame( ))
		return false;

	// Can't have custom level or no campaign in deathmatch.
	if( m_bCustomLevel || m_Campaign.size( ) == 0 )
	{
		ASSERT( !"CDeathMatchMissionMgr::EndGame:  Can't have custom level or no campaign in deathmatch." );
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

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleMultiplayerOptions
//
//	PURPOSE:	Handle updating the host options
//
// --------------------------------------------------------------------------- //

bool CDeathMatchMissionMgr::HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg )
{
	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	sgo.GetDeathmatch().Read(&msg);

	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = false;
	sms.m_bFriendlyFire = true;
	sms.m_nRunSpeed = sgo.GetDeathmatch().m_nRunSpeed;
	sms.m_nScoreLimit = sgo.GetDeathmatch().m_nScoreLimit;
	sms.m_nTimeLimit = sgo.GetDeathmatch().m_nTimeLimit;
	sms.m_nRounds = sgo.GetDeathmatch().m_nRounds;
	sms.m_nFragScore = sgo.GetDeathmatch().m_nFragScore;
	sms.m_nTagScore = sgo.GetDeathmatch().m_nTagScore;

	SetServerSettings(sms);

	return true;

}
