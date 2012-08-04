// ----------------------------------------------------------------------- //
//
// MODULE  : SharedScoring.cpp
//
// PURPOSE : SharedScoring - shared mission summary stuff
//
// CREATED : 10/17/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "SharedScoring.h"
#include "MsgIDs.h"
#ifdef _SERVERBUILD
#include "../ObjectDLL/ServerMissionMgr.h"
#include "../ObjectDLL/PlayerObj.h"
#include "BroadcastDB.h"
#endif
#include "GameModeMgr.h"
#include "TeamMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::CPlayerScore
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerScore::CPlayerScore()
{
	m_nClientID = ( uint32 )-1;
	m_nScore = 0;
	memset(m_nEvents,0,sizeof(m_nEvents));

	m_bInitted = false;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::Init
//
//	PURPOSE:	Clear data
//
// ----------------------------------------------------------------------- //

void CPlayerScore::Init(uint32 nClientID)
{
	m_nClientID = nClientID;
	m_nScore = 0;
	memset(m_nEvents,0,sizeof(m_nEvents));

#ifdef _SERVERBUILD
	ClearTeamKills();
	m_nSequentialKills = 0;
	m_nMultiKillCount = 0;
	m_MultiKillTimer.SetEngineTimer(SimulationTimer::Instance());
#endif


	m_bInitted = true;

	SERVER_CODE
	(
		UpdateClients();
	)
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::WriteData
//
//	PURPOSE:	Write the data to be sent to the client
//
// ----------------------------------------------------------------------- //

void CPlayerScore::WriteData(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    pMsg->Writeint32(m_nScore);
	for (uint8 i = 0; i < kNumScoreEvents; ++i)
	{
		pMsg->Writeuint32(m_nEvents[i]);
	}
    
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::ReadData
//
//	PURPOSE:	Read the data sent to the client
//
// ----------------------------------------------------------------------- //

void CPlayerScore::ReadData(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    m_nScore	= pMsg->Readint32();
	for (uint8 i = 0; i < kNumScoreEvents; ++i)
	{
		m_nEvents[i] = pMsg->Readuint32();
	}
}

#ifdef _SERVERBUILD

void CPlayerScore::AddEvent(ScoreEvent eType )
{
	m_nEvents[eType]++;
	int32 nPlayerValue = 0;
	int32 nTeamValue = 0;
	uint8 nTeamId = INVALID_TEAM;

	GameModeMgr& gameModeMgr = GameModeMgr::Instance( );

	// See if we are using teams.
	if( gameModeMgr.m_grbUseTeams )
	{
		CPlayerObj* pPlayerObj = GetPlayerFromClientId( m_nClientID );
		if( pPlayerObj )
		{
			nTeamId = pPlayerObj->GetTeamID();
		}
	}

	switch(eType) 
	{
	case eKill:
		m_nSequentialKills++;
		CheckSequentialKills();

		if (m_MultiKillTimer.IsStarted() && !m_MultiKillTimer.IsTimedOut())
		{
			m_nMultiKillCount++;
		}
		else
		{
			m_nMultiKillCount = 1;
		}
		m_MultiKillTimer.Start(0.5f);

		nPlayerValue = (int32)gameModeMgr.m_grnFragScorePlayer;
		nTeamValue = (int32)gameModeMgr.m_grnFragScoreTeam;
		break;
	case eDeath:
		m_nSequentialKills = 0;
		nPlayerValue = gameModeMgr.m_grnDeathScorePlayer;
		nTeamValue = gameModeMgr.m_grnDeathScoreTeam;
		break;
	case eTeamKill:
		m_nTeamKills++;
		nPlayerValue = gameModeMgr.m_grnTKScore;
		nTeamValue = 0;
		break;
	case eSuicide:
		m_nSequentialKills = 0;
		nPlayerValue = gameModeMgr.m_grnSuicideScorePlayer;
		nTeamValue = gameModeMgr.m_grnSuicideScoreTeam;
		break;
	}
	AddScore(nPlayerValue);

	if( nTeamId != INVALID_TEAM )
	{
		CTeamMgr::Instance().AddToScore( nTeamId, nTeamValue );
	}
}

void CPlayerScore::AddScore(int32 nBonus)
{
	// Ignore if round victory condition has already been met.
	if( g_pServerMissionMgr->IsEndRoundConditionMet())
		return;

	m_nScore += nBonus;

	g_pServerMissionMgr->CheckScoreLimitWin();

	UpdateClients();
}

void CPlayerScore::AddObjectiveScore(int32 nBonus)
{
	// Ignore if round victory condition has already been met.
	if( g_pServerMissionMgr->IsEndRoundConditionMet())
		return;

	// Objective scores are not counted individually.
	m_nEvents[eObjective] += nBonus;

	AddScore( nBonus );
}

void CPlayerScore::UpdateClients(HCLIENT hClients /*= NULL*/)
{
	if (!m_bInitted) 
		return;

	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_PLAYER_SCORE);
	cMsg.Writeuint32(m_nClientID);    

	WriteData(cMsg);
	g_pLTServer->SendToClient(cMsg.Read(), hClients, MESSAGE_GUARANTEED);
}


void CPlayerScore::Update()
{
	if (m_MultiKillTimer.IsStarted() && !m_MultiKillTimer.IsTimedOut())
	{
		CheckMultiKill();
	}
}


void CPlayerScore::CheckSequentialKills()
{
	HRECORD hRec = DATABASE_CATEGORY( BroadcastGlobal ).GetGlobalRecord();
	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hRec,"SequentialKills");
	uint32 nNum = g_pLTDatabase->GetNumValues(hAtt);

	for (uint32 n = 0; n < nNum; ++n)
	{
		uint32 nThreshold = DATABASE_CATEGORY( BroadcastGlobal ).GETSTRUCTATTRIB(SequentialKills, hAtt, n, Threshold);
		HRECORD hBroadcast = DATABASE_CATEGORY( BroadcastGlobal ).GETSTRUCTATTRIB(SequentialKills, hAtt, n, Broadcast);

		if (nThreshold == m_nSequentialKills)
		{
			HRECORD hBroadcast = DATABASE_CATEGORY( BroadcastGlobal ).GETSTRUCTATTRIB(SequentialKills, hAtt, n, Broadcast);
			if (hBroadcast)
			{
				CPlayerObj* pPlayerObj = GetPlayerFromClientId( m_nClientID );
				if( pPlayerObj )
				{
					PlayerBroadcastInfo pbi;
					pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hBroadcast );
					pbi.bForceClient = true;
					pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hBroadcast, Priority);
					pPlayerObj->HandleBroadcast( pbi );

					const char * pszStringID = DATABASE_CATEGORY( BroadcastGlobal ).GETSTRUCTATTRIB(SequentialKills, hAtt, n, MessageID);

					if (pszStringID)
					{
						CAutoMessage cClientMsg;
						cClientMsg.Writeuint8(MID_PLAYER_EVENT);
						cClientMsg.Writeuint8(kPEReward);
						cClientMsg.Writeuint32(m_nClientID);
						cClientMsg.Writeuint32(IndexFromStringID(pszStringID));
						g_pLTServer->SendToClient(cClientMsg.Read(), NULL, MESSAGE_GUARANTEED);
					}


				}
			}
			return;
		}
	}

}

void CPlayerScore::CheckMultiKill()
{
	HRECORD hRec = DATABASE_CATEGORY( BroadcastGlobal ).GetGlobalRecord();
	HATTRIBUTE hAtt = g_pLTDatabase->GetAttribute(hRec,"MultiKills");
	uint32 nNum = g_pLTDatabase->GetNumValues(hAtt);
	uint32 nLastThreshold = 0;

	HRECORD hBroadcast = NULL;
	for (uint32 n = 0; n < nNum; ++n)
	{
		uint32 nThreshold = DATABASE_CATEGORY( BroadcastGlobal ).GETSTRUCTATTRIB(MultiKills, hAtt, n, Threshold);
		if (nThreshold > nLastThreshold && nThreshold <= m_nMultiKillCount)
		{
			nLastThreshold = nThreshold;
			hBroadcast = DATABASE_CATEGORY( BroadcastGlobal ).GETSTRUCTATTRIB(MultiKills, hAtt, n, Broadcast);
		}
	}

	if (hBroadcast)
	{
		CPlayerObj* pPlayerObj = GetPlayerFromClientId( m_nClientID );
		if( pPlayerObj )
		{
			PlayerBroadcastInfo pbi;
			pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hBroadcast );
			pbi.bForceClient = true;
			pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);
			pbi.hTarget = pPlayerObj->GetClient();

			pPlayerObj->HandleBroadcast( pbi );
		}
	}

}

#endif
