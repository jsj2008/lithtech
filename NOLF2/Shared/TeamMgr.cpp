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

#include "stdafx.h"
#include "TeamMgr.h"
#include "MsgIDs.h"

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
	m_nModel = eModelIdInvalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeam::Init
//
//	PURPOSE:	Init data
//
// ----------------------------------------------------------------------- //

void CTeam::Init(uint8 nID, const char *szName, uint32 nModel)
{
	m_nID = nID;
	m_nScore = 0;
	m_nRoundScore = 0;
	m_sName = szName;
	m_nModel = nModel;
	m_players.clear();
}

void CTeam::Init(uint8 nID, ILTMessage_Read *pMsg)
{
	m_nID = nID;
	m_players.clear();
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
		pMsg->WriteString(m_sName.c_str());
		pMsg->Writeuint32(m_nModel);
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
		char szTeamName[256];
		pMsg->ReadString(szTeamName,sizeof(szTeamName));
		m_sName = szTeamName;

		m_nModel	= pMsg->Readuint32();
	}

}

#ifndef _CLIENTBUILD

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
		g_pLTServer->CPrint( "CTeam::AddPlayer() - FAILED: Invalid ClientID (%i) Team '%s'", nClientID, GetName() );
		return;
	}

	m_players.insert(nClientID);
g_pLTServer->CPrint( "Added ClientID (%i) to team '%s'", nClientID, GetName() );
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
	m_nScore += nScore;
	UpdateClient(true);

g_pLTServer->CPrint( "Team%i: '%s' Score: %i", m_nID, GetName(), m_nScore );
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

void CTeam::UpdateClient(bool bScoreOnly, HCLIENT hClients /*= LTNULL*/)
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
	return m_teams.size();
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

}

#else // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTeamMgr::AddTeam()
//
//	PURPOSE:	Create a new team (Server only)
//
// ----------------------------------------------------------------------- //
uint8 CTeamMgr::AddTeam(const char *szName, uint32 nModel)
{
	CTeam* pTeam = debug_new(CTeam);
	if( !pTeam )
		return INVALID_TEAM;

	pTeam->Init(m_teams.size(),szName,nModel);

	m_teams.push_back(pTeam);

g_pLTServer->CPrint( "Added team '%s' with model (%i)", szName, nModel );

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
	CTeam* pTeam = GetTeam(nTeamID);
	if (pTeam)
	{
		RemovePlayer(nClientID);
		pTeam->AddPlayer(nClientID);
	}
	else
	{
		// Invalid team specified so just auto assign the player to the team with the least amount of people...
		pTeam = GetTeamWithLeastPlayers();

		if( pTeam )
		{
			RemovePlayer( nClientID );
			pTeam->AddPlayer( nClientID );
		}
	}

	ASSERT( pTeam && "No valid team to add player to!" );
}

CTeam*	CTeamMgr::GetTeamWithLeastPlayers()
{
	uint8 nLeastNumPlayers = _UI8_MAX;
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
	TeamArray::iterator iter = m_teams.begin();
	while (iter != m_teams.end())
	{
		(*iter)->UpdateClient(false,hClient);
		iter++;
	}
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

#endif // _CLIENTBUILD
