// ----------------------------------------------------------------------- //
//
// MODULE  : TeamDeathMatchMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle team deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "TeamDeathMatchMissionMgr.h"
	#include "TeamMgr.h"
	#include "MissionButeMgr.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::CTeamDeathMatchMissionMgr
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTeamDeathMatchMissionMgr::CTeamDeathMatchMissionMgr()
:	CDeathMatchMissionMgr	()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::~CTeamDeathMatchMissionMgr
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTeamDeathMatchMissionMgr::~CTeamDeathMatchMissionMgr()
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //

bool CTeamDeathMatchMissionMgr::Init()
{
	if( !CDeathMatchMissionMgr::Init() )
		return false;

	// Add the teams to the team mgr...

	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );

	uint8 nNumTeams = sgo.GetTeamDeathmatch().m_nNumTeams;

	for( uint8 i = 0; i < nNumTeams; ++i )
	{
		CTeamMgr::Instance().AddTeam( sgo.GetTeamDeathmatch().m_sTeamName[i].c_str(),
									  sgo.GetTeamDeathmatch().m_nTeamModel[i] );
	}

	return !!CTeamMgr::Instance().GetNumTeams();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::Update
//
//	PURPOSE:	Frame update.  Check for end level condition...
//
// ----------------------------------------------------------------------- //

void CTeamDeathMatchMissionMgr::Update()
{
	// Check the teams to see if any of them have reached the score limit...
	
	if( m_ServerSettings.m_nScoreLimit > 0 && m_fStartTime >= 0.0f )
	{
		for( uint8 i = 0; i < CTeamMgr::Instance().GetNumTeams(); ++i )
		{
			CTeam *pTeam = CTeamMgr::Instance().GetTeam( i );
			if( pTeam && (pTeam->GetScore() >= m_ServerSettings.m_nScoreLimit) )
			{
				CTeamMgr::Instance( ).WonRound( i );
				g_pLTServer->CPrint( "ScoreLimit reached." );
				NextRound();
			}
		}
	}


	// Skip the CDeathMatchMissionMgr::Update() but be sure to call up...

	CServerMissionMgr::Update( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::FinishExitLevel
//
//	PURPOSE:	Finish the exitlevel we started....
//
// ----------------------------------------------------------------------- //

bool CTeamDeathMatchMissionMgr::FinishExitLevel()
{
	if( !CDeathMatchMissionMgr::FinishExitLevel() )
		return false;

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::StartGame
//
//	PURPOSE:	Start the game.
//
// --------------------------------------------------------------------------- //

bool CTeamDeathMatchMissionMgr::StartGame(ILTMessage_Read& msg )
{
	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = false;
	sms.m_bFriendlyFire = sgo.GetTeamDeathmatch().m_bFriendlyFire;
	sms.m_nRunSpeed = sgo.GetTeamDeathmatch().m_nRunSpeed;
	sms.m_nScoreLimit = sgo.GetTeamDeathmatch().m_nScoreLimit;
	sms.m_nTimeLimit = sgo.GetTeamDeathmatch().m_nTimeLimit;
	sms.m_nRounds = sgo.GetTeamDeathmatch().m_nRounds;
	sms.m_nFragScore = sgo.GetTeamDeathmatch().m_nFragScore;
	sms.m_nTagScore = sgo.GetTeamDeathmatch().m_nTagScore;;

	SetServerSettings(sms);

	if( !g_pMissionButeMgr->Init( MISSION_DM_FILE ))
		return false;

	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamDeathMatchMissionMgr::LevelStarted()
//
//	PURPOSE:	Called when the clients have finished loading and have started
//				playing a level
//
// ----------------------------------------------------------------------- //

void CTeamDeathMatchMissionMgr::LevelStarted()
{
	CServerMissionMgr::LevelStarted( );

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



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerMissionMgr::HandleMultiplayerOptions
//
//	PURPOSE:	Handle updating the host options
//
// --------------------------------------------------------------------------- //

bool CTeamDeathMatchMissionMgr::HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg )
{

	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	sgo.GetTeamDeathmatch().Read(&msg);

	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = false;
	sms.m_bFriendlyFire = sgo.GetTeamDeathmatch().m_bFriendlyFire;
	sms.m_nRunSpeed = sgo.GetTeamDeathmatch().m_nRunSpeed;
	sms.m_nScoreLimit = sgo.GetTeamDeathmatch().m_nScoreLimit;
	sms.m_nTimeLimit = sgo.GetTeamDeathmatch().m_nTimeLimit;
	sms.m_nRounds = sgo.GetTeamDeathmatch().m_nRounds;
	sms.m_nFragScore = sgo.GetTeamDeathmatch().m_nFragScore;
	sms.m_nTagScore = sgo.GetTeamDeathmatch().m_nTagScore;;

	SetServerSettings(sms);

	return true;

}
