// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsDayMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "DoomsDayMissionMgr.h"
#include "PlayerObj.h"
#include "ServerSaveLoadMgr.h"
#include "MissionButeMgr.h"

extern CVarTrack g_vtNetFriendlyFire;

// --------------------------------------------------------------------------- //
// Constructor & Destructor
// --------------------------------------------------------------------------- //
CDoomsDayMissionMgr::CDoomsDayMissionMgr()
{
	m_nActivatingTeam = INVALID_TEAM;
	m_bNextRound = false;
}

CDoomsDayMissionMgr::~CDoomsDayMissionMgr()
{
	Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::Init
//
//	PURPOSE:	Init the mgr
//
// ----------------------------------------------------------------------- //
bool CDoomsDayMissionMgr::Init()
{
	if( !CServerMissionMgr::Init( ))
		return false;

	// Add the teams to the team mgr...
	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	uint8 nNumTeams = sgo.GetDoomsday().m_nNumTeams;

	for( uint8 i = 0; i < nNumTeams; ++i )
	{
		CTeamMgr::Instance().AddTeam( sgo.GetDoomsday().m_sTeamName[i].c_str(),
									  sgo.GetDoomsday().m_nTeamModel[i] );
	}

	m_nActivatingTeam = INVALID_TEAM;
	m_bNextRound = false;

	return !!CTeamMgr::Instance().GetNumTeams();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::Term
//
//	PURPOSE:	Term the mgr
//
// ----------------------------------------------------------------------- //
void CDoomsDayMissionMgr::Term()
{
	CServerMissionMgr::Term( );

	m_nActivatingTeam = INVALID_TEAM;
	m_bNextRound = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::Save()
//
//	PURPOSE:	Saves state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::Save( ILTMessage_Write& msg, uint32 dwSaveFlags )
{
	if( !CServerMissionMgr::Save( msg, dwSaveFlags ))
		return false;

	ILTMessage_Write* pMsg = &msg;

	SAVE_BYTE( m_nActivatingTeam );
	SAVE_bool( m_bNextRound );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::Load()
//
//	PURPOSE:	Loads state of misionmgr.
//
// ----------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::Load( ILTMessage_Read& msg, uint32 dwSaveFlags )
{
	if( !CServerMissionMgr::Load( msg, dwSaveFlags ))
		return false;

	ILTMessage_Read* pMsg = &msg;

	LOAD_BYTE( m_nActivatingTeam );
	LOAD_bool( m_bNextRound );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::Update
//
//	PURPOSE:	Frame update.
//
// --------------------------------------------------------------------------- //

void CDoomsDayMissionMgr::Update( )
{
	CServerMissionMgr::Update( );

	// Need to do next round stuff in update and not in other functions to
	// guarantee we aren't being called by an object.  Since nextround can
	// delete all the objects, this would cause a crash.
	if( m_bNextRound )
	{
		m_bNextRound = false;
		NextRound( );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::FinishExitLevel
//
//	PURPOSE:	Finish the exitlevel we started.
//
// --------------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::FinishExitLevel( )
{
	TRACE( "CDoomsDayMissionMgr::FinishExitLevel:\n" );

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

	// Clear team info.
	m_nActivatingTeam = INVALID_TEAM;
	m_bNextRound = false;

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::StartGame
//
//	PURPOSE:	Start the game.
//
// --------------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::StartGame(ILTMessage_Read& msg )
{
	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = false;
	sms.m_bFriendlyFire = sgo.GetDoomsday().m_bFriendlyFire;
	sms.m_nRunSpeed = sgo.GetDoomsday().m_nRunSpeed;
	sms.m_nScoreLimit = 0;
	sms.m_nTimeLimit = sgo.GetDoomsday().m_nTimeLimit;
	sms.m_nRounds = sgo.GetDoomsday().m_nRounds;
	sms.m_nFragScore = sgo.GetDoomsday().m_nFragScore;
	sms.m_nTagScore = sgo.GetDoomsday().m_nTagScore;
	sms.m_nReviveScore = sgo.GetDoomsday().m_nRevivingScore;

	SetServerSettings(sms);


	if( !g_pMissionButeMgr->Init( MISSION_DD_FILE ))
		return false;

	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::EndGame
//
//	PURPOSE:	End the game.
//
// --------------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::EndGame( )
{
	// Call base.
	if( !CServerMissionMgr::EndGame( ))
		return false;

	// Can't have custom level or no campaign in deathmatch.
	if( m_bCustomLevel || m_Campaign.size( ) == 0 )
	{
		ASSERT( !"CDoomsDayMissionMgr::EndGame:  Can't have custom level or no campaign in doomsday." );
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
//	ROUTINE:	CDoomsDayMissionMgr::HandleMultiplayerOptions
//
//	PURPOSE:	Handle updating the host options
//
// --------------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg )
{

	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	sgo.GetDoomsday().Read(&msg);

	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = false;
	sms.m_bFriendlyFire = sgo.GetDoomsday().m_bFriendlyFire;
	sms.m_nRunSpeed = sgo.GetDoomsday().m_nRunSpeed;
	sms.m_nScoreLimit = 0;
	sms.m_nTimeLimit = sgo.GetDoomsday().m_nTimeLimit;
	sms.m_nRounds = sgo.GetDoomsday().m_nRounds;
	sms.m_nFragScore = sgo.GetDoomsday().m_nFragScore;
	sms.m_nTagScore = sgo.GetDoomsday().m_nTagScore;
	sms.m_nReviveScore = sgo.GetDoomsday().m_nRevivingScore;

	SetServerSettings(sms);


	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::SetDeviceFiring
//
//	PURPOSE:	Called when activating team starts device.
//
// --------------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::SetDeviceFiring( uint8 nActivatingTeam )
{
	m_nActivatingTeam = nActivatingTeam;
	g_pLTServer->CPrint( "Device completed." );
	CTeamMgr::Instance( ).WonRound( m_nActivatingTeam );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::SetDeviceEffectComplete
//
//	PURPOSE:	Called when activating team completes effect.
//
// --------------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::SetDeviceEffectComplete( uint8 nActivatingTeam )
{
	m_nActivatingTeam = nActivatingTeam;

	// Tell ourselves to go to the next round on our next update.
	m_bNextRound = true;

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::CanRespawn
//
//	PURPOSE:	Called to check if gametype allows respawning.
//
// --------------------------------------------------------------------------- //

bool CDoomsDayMissionMgr::CanRespawn( CPlayerObj const& player ) const
{
	// Once the device starts, the other teams can't respawn.
	if( m_nActivatingTeam != INVALID_TEAM && player.GetTeamID( ) != m_nActivatingTeam )
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsDayMissionMgr::LevelStarted()
//
//	PURPOSE:	Called when the clients have finished loading and have started
//				playing a level
//
// ----------------------------------------------------------------------- //

void CDoomsDayMissionMgr::LevelStarted()
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
