// ----------------------------------------------------------------------- //
//
// MODULE  : SinglePlayerMissionMgr.cpp
//
// PURPOSE : Implementation of class to handle deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "SinglePlayerMissionMgr.h"
#include "MusicMgr.h"
#include "PlayerObj.h"
#include "MsgIds.h"
#include "MissionButeMgr.h"

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CSinglePlayerMissionMgr::StartGame
//
//	PURPOSE:	Default start game.
//
// --------------------------------------------------------------------------- //

bool CSinglePlayerMissionMgr::StartGame( ILTMessage_Read& msg )
{
	if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
		return false;

	if( !CServerMissionMgr::StartGame( msg ))
		return false;

	g_pMusicMgr->Enable( );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CSinglePlayerMissionMgr::EndGame
//
//	PURPOSE:	End the game.
//
// --------------------------------------------------------------------------- //

bool CSinglePlayerMissionMgr::EndGame( )
{
	// Call base.
	if( !CServerMissionMgr::EndGame( ))
		return false;

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

	return true;
}