// ----------------------------------------------------------------------- //
//
// MODULE  : TeamMgr.cpp
//
// PURPOSE : TeamMgr - shared mission summary stuff
//
// CREATED : 11/19/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "TeamMgr.h"
#include "MsgIDs.h"

#ifdef _SERVERBUILD
#include "iltserver.h"
extern ILTServer *g_pLTServer;
#include "../ObjectDLL/ServerMissionMgr.h"
#include "../ObjectDLL/PlayerObj.h"
#endif

// ----------------------------------------------------------------------- //
//
// CTeam - class to manage a team's data
//
// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::CTeam
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTeam::CTeam()
{
	m_nID = INVALID_TEAM;
	m_nScore = 0;
	m_nRoundScore = 0;
	m_nModel = (uint32)NULL;
	m_bAttackingTeam = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::Init
//
//	PURPOSE:	Init data
//
// ----------------------------------------------------------------------- //

void CTeam::Init(uint8 nID, uint32 nModel)
{
	m_nID = nID;
	m_nScore = 0;
	m_nRoundScore = 0;
	m_nModel = nModel;
	m_players.clear();
	m_bAttackingTeam = false;
}

void CTeam::Init(uint8 nID, ILTMessage_Read *pMsg)
{
	m_nID = nID;
	m_players.clear();
	m_bAttackingTeam = false;
	ReadData(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::WriteData
//
//	PURPOSE:	Write the data to be sent to the client
//
// ----------------------------------------------------------------------- //

void CTeam::WriteData(ILTMessage_Write *pMsg,bool bScoreOnly)
{
	if (!pMsg) return;

	if (bScoreOnly)
		pMsg->Writeuint8(MTEAM_SCORE);
	else
		pMsg->Writeuint8(MTEAM_FULL);

    pMsg->Writeint32(m_nScore);
    pMsg->Writeint32(m_nRoundScore);

	if (!bScoreOnly)
	{
		pMsg->Writeuint32(m_nModel);
		pMsg->Writebool( GetAttacking( ));
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::ReadData
//
//	PURPOSE:	Read the data sent to the client
//
// ----------------------------------------------------------------------- //

void CTeam::ReadData(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	uint8 msgType = pMsg->Readuint8();

    m_nScore	= pMsg->Readint32();
    m_nRoundScore = pMsg->Readuint32();

	if (msgType == MTEAM_FULL)
	{
		m_nModel	= pMsg->Readuint32();
		SetAttacking( pMsg->Readbool( ));
	}

}

#ifdef _SERVERBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddPlayer
//
//	PURPOSE:	Add a player
//
// ----------------------------------------------------------------------- //

void CTeam::AddPlayer(uint32 nClientID)
{
	// Get the player from the clientID...

	HCLIENT hClient = g_pLTServer->GetClientHandle( nClientID );
	if( !hClient )
	{
		g_pLTServer->CPrint( "CTeam::AddPlayer() - FAILED: Invalid ClientID (%i) Team %d", nClientID, GetID());
		return;
	}

	m_players.insert(nClientID);

	CPlayerObj* pPlayer = GetPlayerFromClientId(nClientID);
	
	// use LTSNPrintF to form the string in order to protect against embedded formatting
	char szMsg[256] = {0};
	LTSNPrintF(szMsg, LTARRAYSIZE(szMsg), "Player '%ls' joined Team %d.", pPlayer->GetNetUniqueName(), GetID());
	g_pLTServer->CPrintNoArgs(szMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::RemovePlayer
//
//	PURPOSE:	Remove a player
//
// ----------------------------------------------------------------------- //

void CTeam::RemovePlayer(uint32 nClientID)
{
	PlayerIDSet::iterator iter = m_players.find(nClientID);
	if (iter != m_players.end())
	{
		m_players.erase(iter);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::AddScore
//
//	PURPOSE:	Adjust the team score
//
// ----------------------------------------------------------------------- //

void CTeam::AddScore(int32 nScore)
{
	// Ignore if round victory condition has already been met.
	if( g_pServerMissionMgr->IsEndRoundConditionMet())
		return;

	m_nScore += nScore;

	g_pServerMissionMgr->CheckScoreLimitWin();

	UpdateClient(true);

	DebugCPrint(2, "Team%i Score: %i", m_nID, m_nScore );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::WonRound
//
//	PURPOSE:	Record round win.
//
// ----------------------------------------------------------------------- //

void CTeam::WonRound( )
{
	m_nRoundScore++;
	UpdateClient(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::UpdateClient
//
//	PURPOSE:	Send team info to client
//
// ----------------------------------------------------------------------- //

void CTeam::UpdateClient(bool bScoreOnly, HCLIENT hClients /*= NULL*/)
{
	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_TEAM_INFO);
    cMsg.Writeuint8(m_nID);
	WriteData(cMsg,bScoreOnly);
	g_pLTServer->SendToClient(cMsg.Read(), hClients, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::NewRound
//
//	PURPOSE:	Prepares for a new round.
//
// ----------------------------------------------------------------------- //

void CTeam::NewRound( )
{
	m_nScore = 0;
	UpdateClient(true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::NewLevel
//
//	PURPOSE:	Prepares for a new level.
//
// ----------------------------------------------------------------------- //

void CTeam::NewLevel( )
{
	m_nRoundScore = 0;
	m_nScore = 0;
	UpdateClient(true);
}


#endif

// ----------------------------------------------------------------------- //
//
// CTeamMgr - class to manage multiple teams
//
// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

CTeamMgr& CTeamMgr::Instance()
{
	static CTeamMgr sTeamMgr;
	return sTeamMgr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::CTeamMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTeamMgr::CTeamMgr()
{
	m_teams.reserve( MAX_TEAMS );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::CTeamMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTeamMgr::~CTeamMgr()
{
	TeamArray::iterator iter = m_teams.begin();
	while (iter != m_teams.end())
	{
		debug_delete( *iter);
		iter++;
	}
	m_teams.clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::GetTeam()
//
//	PURPOSE:	Retrieve the specified team 
//
// ----------------------------------------------------------------------- //

CTeam* CTeamMgr::GetTeam(uint8 nID)
{
	TeamArray::iterator iter = m_teams.begin();
	while (iter != m_teams.end() && ((*iter)->GetID() != nID) )
		iter++;

	if (iter == m_teams.end())
	{
		return NULL;
	}
	else
	{
		return (*iter);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::GetNumTeams()
//
//	PURPOSE:	Retrieve the number of teams
//
// ----------------------------------------------------------------------- //

uint8 CTeamMgr::GetNumTeams()
{
	return ( uint8 )m_teams.size();
}

#ifdef _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::UpdateTeam()
//
//	PURPOSE:	Create/update a team from a message (Client only)
//
// ----------------------------------------------------------------------- //

void CTeamMgr::UpdateTeam(ILTMessage_Read *pMsg)
{
    uint8 nID = pMsg->Readuint8();

	CTeam* pTeam = GetTeam(nID);

	if (!pTeam)
	{
		CTeam* pTeam = debug_new(CTeam);
		pTeam->Init(nID,pMsg);
		m_teams.push_back(pTeam);
	}
	else
	{
		pTeam->ReadData(pMsg);
	}
	g_pHUDMgr->QueueUpdate(kHUDScores);

}

#endif // _CLIENTBUILD

#ifdef _SERVERBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddTeam()
//
//	PURPOSE:	Create a new team (Server only)
//
// ----------------------------------------------------------------------- //
uint8 CTeamMgr::AddTeam(uint32 nModel)
{
	CTeam* pTeam = debug_new(CTeam);
	if( !pTeam )
		return INVALID_TEAM;

	pTeam->Init(m_teams.size(),nModel);

	m_teams.push_back(pTeam);

	DebugCPrint(1, "Added team %d with model (%i)", pTeam->GetID( ), nModel );

	return m_teams.size();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddPlayer()
//
//	PURPOSE:	Add a player to a team
//
// ----------------------------------------------------------------------- //
void CTeamMgr::AddPlayer(uint32 nClientID, uint8 nTeamID)
{
	RemovePlayer(nClientID);

	CTeam* pTeam = GetTeam(nTeamID);
	if (pTeam)
	{
		pTeam->AddPlayer(nClientID);
	}
}

CTeam*	CTeamMgr::GetTeamWithLeastPlayers()
{
	uint8 nLeastNumPlayers = (uint8)-1;
	int32 nScore = 0;
	CTeam* pTeam = NULL;

	TeamArray::const_iterator iter;
	for( iter = m_teams.begin(); iter != m_teams.end(); ++iter )
	{
		//choose this team, if the team has fewer players, or the same number but a lower score 
		if( (*iter)->GetNumPlayers() < nLeastNumPlayers ||  
			((*iter)->GetNumPlayers() == nLeastNumPlayers && (*iter)->GetScore() < nScore)  )
		{
			pTeam = (*iter);
			nLeastNumPlayers = pTeam->GetNumPlayers();
			nScore = pTeam->GetScore();
		}
	}

	return pTeam;
}

void CTeamMgr::RemovePlayer(uint32 nClientID)
{
	TeamArray::iterator iter = m_teams.begin();
	while (iter != m_teams.end())
	{
		(*iter)->RemovePlayer(nClientID);
		iter++;
	}
}

uint8 CTeamMgr::GetTeamIdOfPlayer( uint32 nClientID )
{
	// Iterate over the teams looking for the client.
	TeamArray::iterator iter = m_teams.begin();
	for( ; iter != m_teams.end(); iter++ )
	{
		CTeam* pTeam = *iter;
		if( !pTeam )
			continue;

		// Iterate over the players looking for the client.
		PlayerIDSet const& players = pTeam->GetPlayerIDSet( );
		PlayerIDSet::const_iterator iter = players.find(nClientID);
		if( iter == players.end( ))
			continue;

		// This is our team.
		return pTeam->GetID( );
	}

	return INVALID_TEAM;
}


void CTeamMgr::UpdateClient(HCLIENT hClient)
{
	UpdateClient( hClient, false );
}


void CTeamMgr::UpdateClient( HCLIENT hClient, bool bScoreOnly )
{
	TeamArray::iterator iter = m_teams.begin();
	while (iter != m_teams.end())
	{
		(*iter)->UpdateClient( bScoreOnly, hClient );
		iter++;
	}
}

void CTeamMgr::SetAttackingTeam( uint8 nTeamId, bool bValue )
{
	CTeam *pTeam = GetTeam( nTeamId );
	if( !pTeam )
		return;

	// Iterate over all the teams and set one of them as the attacker.
	TeamArray::iterator iter = m_teams.begin();
	for( ; iter != m_teams.end(); iter++ )
	{
		CTeam* pTeam = *iter;
		pTeam->SetAttacking(( pTeam->GetID( ) == nTeamId ));
	}

	// Tell the clients about the change.
	UpdateClient( NULL, false );
}

 

bool CTeamMgr::GetAttackingTeam( uint8 nTeamId )
{
	CTeam *pTeam = GetTeam( nTeamId );
	if( !pTeam )
		return false;

	return pTeam->GetAttacking( );
}


void CTeamMgr::AddToScore( uint8 nTeamID, int32 nScore /* = 1  */ )
{
	CTeam *pTeam = GetTeam( nTeamID );
	if( !pTeam )
		return;

	pTeam->AddScore( nScore );
}

void CTeamMgr::WonRound( uint8 nTeamID )
{
	CTeam *pTeam = GetTeam( nTeamID );
	if( !pTeam )
		return;

	pTeam->WonRound( );
}

void CTeamMgr::NewRound( )
{
	TeamArray::iterator iter;
	for( iter = m_teams.begin(); iter != m_teams.end(); ++iter )
	{
		(*iter)->NewRound( );
	}
}

void CTeamMgr::NewLevel( )
{
	TeamArray::iterator iter;
	for( iter = m_teams.begin(); iter != m_teams.end(); ++iter )
	{
		(*iter)->NewLevel( );
	}
}

#endif // _SERVERBUILD
