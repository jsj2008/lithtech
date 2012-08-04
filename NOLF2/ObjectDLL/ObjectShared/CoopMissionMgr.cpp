// ----------------------------------------------------------------------- //
//
// MODULE  : CoopMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle coop missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "CoopMissionMgr.h"
#include "MusicMgr.h"
#include "PlayerObj.h"
#include "MsgIds.h"
#include "ServerSaveLoadMgr.h"
#include "MissionButeMgr.h"

extern CVarTrack g_vtNetFriendlyFire;
extern CVarTrack g_vtDifficultyFactorPlayerIncrease;

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCoopMissionMgr::StartGame
//
//	PURPOSE:	Default start game.
//
// --------------------------------------------------------------------------- //

bool CCoopMissionMgr::StartGame(ILTMessage_Read& msg )
{
	if( !g_pMissionButeMgr->Init( MISSION_COOP_FILE ))
		return false;

	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	ServerGameOptions& sgo = g_pGameServerShell->GetServerGameOptions( );
	ServerMissionSettings sms = GetServerSettings();

	sms.m_bUseSkills = sgo.GetCoop().m_bUseSkills;
	sms.m_bFriendlyFire = sgo.GetCoop().m_bFriendlyFire;
	sms.m_nMPDifficulty = sgo.GetCoop().m_nDifficulty;
	sms.m_fPlayerDiffFactor = sgo.GetCoop().m_fPlayerDiffFactor;
	sms.m_nRunSpeed = 100;
	sms.m_nScoreLimit = 0;
	sms.m_nTimeLimit = 0;

	SetServerSettings(sms);

	g_pMusicMgr->Enable( );
	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCoopMissionMgr::EndGame
//
//	PURPOSE:	End the game.
//
// --------------------------------------------------------------------------- //

bool CCoopMissionMgr::EndGame( )
{
	// Call base.
	if( !CServerMissionMgr::EndGame( ))
		return false;

	// Can't have custom level or no campaign in coop.
	if( m_bCustomLevel || m_Campaign.size( ) == 0 )
	{
		ASSERT( !"CCoopMissionMgr::EndGame:  Can't have custom level or no campaign in coop." );
		return false;
	}

	if( !m_bLoopCampaign )
	{
		// Tell the players to prepare to exit a level.
		CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
		while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
		{
			CPlayerObj* pPlayerObj = *iter;
			pPlayerObj->HandlePreExit();
			iter++;
		}

		// Pause the server while we switch levels.
		g_pGameServerShell->PauseGame( LTTRUE );	


		// Tell the clients the game is over.
		SendEmptyClientMsg( MID_END_GAME, NULL, MESSAGE_GUARANTEED );
		return true;
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

bool CCoopMissionMgr::HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg )
{
	ServerMissionSettings sms = GetServerSettings();

	sms.m_bFriendlyFire = msg.Readbool( );
	sms.m_nMPDifficulty = msg.Readuint8( );
	sms.m_fPlayerDiffFactor = msg.Readfloat();

	SetServerSettings(sms);
	
	return true;
}
