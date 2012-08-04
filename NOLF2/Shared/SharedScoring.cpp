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

#include "stdafx.h"
#include "SharedScoring.h"
#include "MsgIDs.h"
#ifndef _CLIENTBUILD
#include "ServerMissionMgr.h"
#include "PlayerObj.h"
#include "ServerUtilities.h"
#endif

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerScore::CPlayerScore
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerScore::CPlayerScore()
{
	m_nClientID = -1;
	m_nScore = 0;
	m_nFrags = 0;
	m_nTags = 0;

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
	m_nFrags = 0;
	m_nTags = 0;

	m_bInitted = true;

#ifndef _CLIENTBUILD
	UpdateClients();
#endif
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
    pMsg->Writeint32(m_nFrags);
    pMsg->Writeint32(m_nTags);

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
    m_nFrags	= pMsg->Readint32();
    m_nTags		= pMsg->Readint32();

}

#ifndef _CLIENTBUILD

void CPlayerScore::AddFrag()
{
	m_nFrags++;
	AddBonus( g_pServerMissionMgr->GetServerSettings().m_nFragScore );
}

void CPlayerScore::RemoveFrag()
{
	m_nFrags--;
	AddBonus( -g_pServerMissionMgr->GetServerSettings().m_nFragScore );
}

void CPlayerScore::AddTag()
{
	m_nTags++;
	AddBonus( g_pServerMissionMgr->GetServerSettings().m_nTagScore );
}

void CPlayerScore::RemoveTag()
{
	m_nTags--;
	AddBonus( -g_pServerMissionMgr->GetServerSettings().m_nTagScore );
}


void CPlayerScore::AddBonus(int nBonus)
{
	m_nScore += nBonus;

	if( IsTeamGameType( ))
	{
		CPlayerObj* pPlayerObj = GetPlayerFromClientId( m_nClientID );
		if( pPlayerObj )
		{
			CTeamMgr::Instance().AddToScore( pPlayerObj->GetTeamID(), nBonus );
		}
	}

	UpdateClients();
}

void CPlayerScore::UpdateClients(HCLIENT hClients /*= LTNULL*/)
{

	if (!m_bInitted) 
		return;

	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_PLAYER_SCORE);
	cMsg.Writeuint32(m_nClientID);    

	WriteData(cMsg);
	g_pLTServer->SendToClient(cMsg.Read(), hClients, MESSAGE_GUARANTEED);
}

#endif