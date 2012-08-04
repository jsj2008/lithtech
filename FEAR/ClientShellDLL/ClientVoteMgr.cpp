// ----------------------------------------------------------------------- //
//
// MODULE  : ClientVoteMgr.cpp
//
// PURPOSE : Manage client-side of player voting
//
// CREATED : 12/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ClientVoteMgr.h"
#include "HUDMessageQueue.h"
#include "HUDTransmission.h"
#include "MissionMgr.h"
#include "GameModeMgr.h"

// ----------------------------------------------------------------------- //
// constructor/destructor
// ----------------------------------------------------------------------- //

ClientVoteMgr::ClientVoteMgr() :
	m_bHasVoted(false)
{
}

ClientVoteMgr::~ClientVoteMgr()
{
}

ClientVoteMgr& ClientVoteMgr::Instance()
{
	static ClientVoteMgr instance;
	return instance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::Init
//
//	PURPOSE:	Handle initial set up
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::Init()
{
	m_VoteTimer.SetEngineTimer(RealTimeTimer::Instance());
	m_VoteDelayTimer.SetEngineTimer(RealTimeTimer::Instance());
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::OnExitWorld()
//
//	PURPOSE:	Handle exiting a world
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::OnExitWorld()
{
	ClearVote();
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::CallVoteKick
//
//	PURPOSE:	Initiate a vote to kick or ban a player
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::CallVoteKick(VoteType eVoteType, uint32 nClientID)
{
	if (IsVoteInProgress())
	{
		g_pTransmission->Show("ScreenVote_VoteInProgress");
		return;
	}
	if (IsVoteDelayed())
	{
		wchar_t szMsg[256];
		szMsg[0] = '\0';
		FormatString("ScreenVote_VoteDelayed",szMsg,LTARRAYSIZE(szMsg),(uint32)m_VoteDelayTimer.GetTimeLeft());
		g_pTransmission->Show(szMsg);
		return;
	}


	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_VOTE );
	cMsg.WriteBits( eVote_Start, FNumBitsExclusive<kNumVoteActions>::k_nValue );
	cMsg.WriteBits( eVoteType, FNumBitsExclusive<kNumVoteTypes>::k_nValue );
	cMsg.Writeuint32( nClientID );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::CallVoteMap
//
//	PURPOSE:	Initiate a vote to cycle to a particular map
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::CallVoteMap(uint32 nIndex)
{
	if (IsVoteInProgress())
	{
		g_pTransmission->Show("ScreenVote_VoteInProgress");
		return;
	}
	if (IsVoteDelayed())
	{
		wchar_t szMsg[256];
		szMsg[0] = '\0';
		FormatString("ScreenVote_VoteDelayed",szMsg,LTARRAYSIZE(szMsg),(uint32)m_VoteDelayTimer.GetTimeLeft());
		g_pTransmission->Show(szMsg);
		return;
	}


	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_VOTE );
	cMsg.WriteBits( eVote_Start, FNumBitsExclusive<kNumVoteActions>::k_nValue );
	cMsg.WriteBits( eVote_SelectMap, FNumBitsExclusive<kNumVoteTypes>::k_nValue );
	cMsg.Writeuint32( nIndex );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::CallVoteNext
//
//	PURPOSE:	Initiate a vote to got to the next round or mao
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::CallVoteNext(VoteType eVoteType)
{
	if (IsVoteInProgress())
	{
		g_pTransmission->Show("ScreenVote_VoteInProgress");
		return;
	}
	if (IsVoteDelayed())
	{
		wchar_t szMsg[256];
		szMsg[0] = '\0';
		FormatString("ScreenVote_VoteDelayed",szMsg,LTARRAYSIZE(szMsg),(uint32)m_VoteDelayTimer.GetTimeLeft());
		g_pTransmission->Show(szMsg);
		return;
	}


	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_VOTE );
	cMsg.WriteBits( eVote_Start, FNumBitsExclusive<kNumVoteActions>::k_nValue );
	cMsg.WriteBits( eVoteType, FNumBitsExclusive<kNumVoteTypes>::k_nValue );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::CastVote
//
//	PURPOSE:	Vote yes or no on current vote
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::CastVote(bool bVote)
{
	if (!IsVoteInProgress() || m_bHasVoted)
	{
		return;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_VOTE );
	cMsg.WriteBits( eVote_Cast, FNumBitsExclusive<kNumVoteActions>::k_nValue );
	cMsg.Writeuint8( m_CurrentVote.m_nVoteID );
	cMsg.Writebool( bVote );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	m_bHasVoted = true;
	g_pHUDMgr->QueueUpdate(kHUDVote);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::HandleMsgVote
//
//	PURPOSE:	Handle a voting message from the server
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::HandleMsgVote(ILTMessage_Read* pMsg)
{
	VoteAction eVoteAction = (VoteAction)pMsg->ReadBits( FNumBitsExclusive<kNumVoteActions>::k_nValue );
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);


	switch(eVoteAction)
	{
	case eVote_Start:
		HandleVoteStart(pMsg);
		break;
	case eVote_Cast:
		if (IsVoteInProgress())
		{
			uint32 nVoteID = pMsg->Readuint8();
			if (nVoteID == m_CurrentVote.m_nVoteID)
			{
				m_CurrentVote.m_nYesVotes = pMsg->Readuint8();
				m_CurrentVote.m_nNoVotes = pMsg->Readuint8();
				m_CurrentVote.m_nVotesNeeded = pMsg->Readuint8();
			}
			g_pHUDMgr->QueueUpdate(kHUDVote);
		}

		break;
	case eVote_Pass:
		if (nLocalID == m_CurrentVote.m_nCallerID)
		{
			//start a timer to prevent us from spamming votes
			double fTimeOut = (double)(GameModeMgr::Instance().m_ServerSettings.m_nVoteDelay); 
			m_VoteDelayTimer.Start(fTimeOut);
		}
		HandleVotePass();
		break;
	case eVote_Fail:
		g_pTransmission->Show("Vote_Failed");
		g_pGameMsgs->AddMessage("Vote_Failed");
		if (nLocalID == m_CurrentVote.m_nCallerID)
		{
			//start a timer to prevent us from spamming votes
			double fTimeOut = (double)(GameModeMgr::Instance().m_ServerSettings.m_nVoteDelay); 
			m_VoteDelayTimer.Start(fTimeOut);
		}
		ClearVote();
		break;
	case eVote_Expire:
		g_pTransmission->Show("Vote_TimedOut");
		g_pGameMsgs->AddMessage("Vote_TimedOut");
		if (nLocalID == m_CurrentVote.m_nCallerID)
		{
			//start a timer to prevent us from spamming votes
			double fTimeOut = (double)(GameModeMgr::Instance().m_ServerSettings.m_nVoteDelay); 
			m_VoteDelayTimer.Start(fTimeOut);
		}
		ClearVote();
		break;
	case eVote_Cancel_Players:
		g_pTransmission->Show("Vote_NotEnoughPlayers");
		g_pGameMsgs->AddMessage("Vote_NotEnoughPlayers");
		m_VoteDelayTimer.Stop();
		ClearVote();
		break;
	case eVote_Cancel_InProgress:
		g_pTransmission->Show("ScreenVote_VoteInProgress");
		g_pGameMsgs->AddMessage("ScreenVote_VoteInProgress");
		m_VoteDelayTimer.Stop();
		break;
	}

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::HandleVoteStart
//
//	PURPOSE:	Handle a voting message from the server
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::HandleVoteStart(ILTMessage_Read* pMsg)
{
	m_CurrentVote.m_nVoteID = pMsg->Readuint8();
	VoteType eVoteType = (VoteType)pMsg->ReadBits( FNumBitsExclusive<kNumVoteTypes>::k_nValue );
	m_CurrentVote.m_eVoteType = eVoteType;
	m_CurrentVote.m_nCallerID = pMsg->Readuint32();
	m_CurrentVote.m_nTargetID = pMsg->Readuint32();
	m_CurrentVote.m_nVotesNeeded = pMsg->Readuint8();
	m_VoteTimer.Start(pMsg->Readdouble());

	m_CurrentVote.m_nYesVotes = 1; //count the person who called the vote

	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);

	//if I'm the one who called the vote, I've already voted yes
	m_bHasVoted = (nLocalID == m_CurrentVote.m_nCallerID);

	wchar_t wszMsg[256] = L"";
	wchar_t wszTxt[256] = L"";
	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr)
	{
		LTERROR("ClientInfoMgr not available");
		return;
	}

	switch(eVoteType)
	{
	case eVote_Kick:
		m_sTargetName = pCIMgr->GetPlayerName(m_CurrentVote.m_nTargetID);

		FormatString("Vote_StartKick",wszMsg,LTARRAYSIZE(wszMsg),pCIMgr->GetPlayerName(m_CurrentVote.m_nCallerID),pCIMgr->GetPlayerName(m_CurrentVote.m_nTargetID));
		FormatString("Vote_Kick",wszTxt,LTARRAYSIZE(wszTxt),pCIMgr->GetPlayerName(m_CurrentVote.m_nTargetID));

		break;
	case eVote_TeamKick:
		m_sTargetName = pCIMgr->GetPlayerName(m_CurrentVote.m_nTargetID);

		FormatString("Vote_StartTeamKick",wszMsg,LTARRAYSIZE(wszMsg),pCIMgr->GetPlayerName(m_CurrentVote.m_nCallerID),pCIMgr->GetPlayerName(m_CurrentVote.m_nTargetID));
		FormatString("Vote_TeamKick",wszTxt,LTARRAYSIZE(wszTxt),pCIMgr->GetPlayerName(m_CurrentVote.m_nTargetID));

		break;
	case eVote_Ban:
		m_sTargetName = pCIMgr->GetPlayerName(m_CurrentVote.m_nTargetID);

		FormatString("Vote_StartBan",wszMsg,LTARRAYSIZE(wszMsg),pCIMgr->GetPlayerName(m_CurrentVote.m_nCallerID),pCIMgr->GetPlayerName(m_CurrentVote.m_nTargetID));
		FormatString("Vote_Ban",wszTxt,LTARRAYSIZE(wszTxt),m_sTargetName.c_str());

		break;
	case eVote_NextRound:
		m_sTargetName = L"";

		FormatString("Vote_StartNextRound",wszMsg,LTARRAYSIZE(wszMsg),pCIMgr->GetPlayerName(m_CurrentVote.m_nCallerID));
		FormatString("Vote_NextRound",wszTxt,LTARRAYSIZE(wszTxt));

		break;
	case eVote_NextMap:
		m_sTargetName = L"";

		FormatString("Vote_StartNextMap",wszMsg,LTARRAYSIZE(wszMsg),pCIMgr->GetPlayerName(m_CurrentVote.m_nCallerID));
		FormatString("Vote_NextMap",wszTxt,LTARRAYSIZE(wszTxt));

		break;
	case eVote_SelectMap:
		m_sTargetName = g_pMissionMgr->GetMapList()[m_CurrentVote.m_nTargetID].c_str();

		FormatString("Vote_StartSelectMap",wszMsg,LTARRAYSIZE(wszMsg),pCIMgr->GetPlayerName(m_CurrentVote.m_nCallerID),m_sTargetName.c_str());
		FormatString("Vote_SelectMap",wszTxt,LTARRAYSIZE(wszTxt),m_sTargetName.c_str());

		break;
	}

	g_pGameMsgs->AddMessage(wszMsg);
	g_pTransmission->Show(wszMsg);
	m_sVoteString = wszTxt;

	g_pHUDMgr->QueueUpdate(kHUDVote);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::ClearVote
//
//	PURPOSE:	Clear current vote
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::ClearVote()
{
	m_CurrentVote.m_eVoteType = eVote_None;
	if (g_pHUDMgr)
	{
		g_pHUDMgr->QueueUpdate(kHUDVote);
	}
	m_VoteTimer.Stop();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientVoteMgr::HandleVotePass
//
//	PURPOSE:	Current vote succeded
//
// ----------------------------------------------------------------------- //

void ClientVoteMgr::HandleVotePass()
{
	wchar_t wszMsg[256] = L"";

	switch(m_CurrentVote.m_eVoteType)
	{
	case eVote_Kick:
		FormatString("Vote_Pass_Kick",wszMsg,LTARRAYSIZE(wszMsg),m_sTargetName.c_str());
		g_pGameMsgs->AddMessage(wszMsg);
		break;
	case eVote_TeamKick:
		FormatString("Vote_Pass_TeamKick",wszMsg,LTARRAYSIZE(wszMsg),m_sTargetName.c_str());
		g_pGameMsgs->AddMessage(wszMsg);
		break;
	case eVote_Ban:
		FormatString("Vote_Pass_Ban",wszMsg,LTARRAYSIZE(wszMsg),m_sTargetName.c_str());
		g_pGameMsgs->AddMessage(wszMsg);
		break;
	case eVote_NextRound:
		g_pGameMsgs->AddMessage( LoadString("Vote_Pass_NextRound") );
		break;
	case eVote_NextMap:
		g_pGameMsgs->AddMessage( LoadString("Vote_Pass_NextMap") );
		break;
	case eVote_SelectMap:
		FormatString("Vote_Pass_SelectMap",wszMsg,LTARRAYSIZE(wszMsg),m_sTargetName.c_str());
		g_pGameMsgs->AddMessage(wszMsg);
		break;
	}


	ClearVote();
}
