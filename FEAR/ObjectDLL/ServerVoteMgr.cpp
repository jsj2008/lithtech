// ----------------------------------------------------------------------- //
//
// MODULE  : ServerVoteMgr.cpp
//
// PURPOSE : Manage server side of player voting
//
// CREATED : 12/02/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "ServerVoteMgr.h"
#include "ServerConnectionMgr.h"
#include "ServerMissionMgr.h"
#include "igamespy.h"
#include "GameModeMgr.h"
#include "BanUserMgr.h"
#include "PlayerObj.h"

// ----------------------------------------------------------------------- //
// constructor/destructor
// ----------------------------------------------------------------------- //

ServerVoteMgr::ServerVoteMgr()
{
	// Reserve enough space for the maximum number of players.
	m_lstEligibleVoter.reserve( MAX_MULTI_PLAYERS );
	m_lstVoterCastYes.reserve( MAX_MULTI_PLAYERS );
	m_lstVoterCastNo.reserve( MAX_MULTI_PLAYERS );
}

ServerVoteMgr::~ServerVoteMgr()
{
	m_delegateRemoveClient.Detach();
}

ServerVoteMgr& ServerVoteMgr::Instance()
{
	static ServerVoteMgr instance;
	return instance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::Init
//
//	PURPOSE:	Handle initial set up
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::Init()
{
	m_VoteTimer.SetEngineTimer(RealTimeTimer::Instance());

	// Attach to the removeclient event.
	m_delegateRemoveClient.Attach( this, g_pGameServerShell, g_pGameServerShell->RemoveClient );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::Update
//
//	PURPOSE:	Handle periodic update
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::Update()
{
	if (m_VoteTimer.IsStarted() && m_VoteTimer.IsTimedOut())
	{
		m_VoteTimer.Stop();
		m_CurrentVote.m_eVoteType = eVote_None;

		// Inform clients that vote timed out
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_VOTE );
		cMsg.WriteBits( eVote_Expire, FNumBitsExclusive<kNumVoteActions>::k_nValue );
		SendToEligibleVoters( *cMsg.Read( ));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::HandleMsgVote
//
//	PURPOSE:	Handle voting message from client
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::HandleMsgVote(HCLIENT hSender, ILTMessage_Read* pMsg)
{

	VoteAction eVoteAction = (VoteAction)pMsg->ReadBits( FNumBitsExclusive<kNumVoteActions>::k_nValue );

	switch(eVoteAction)
	{
	case eVote_Start:
		HandleVoteStart(hSender,pMsg);
		break;
	case eVote_Cast:
		HandleVoteCast(hSender,pMsg);
		break;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SendCancelVoteInProgress
//
//	PURPOSE:	Sends a cancel message when vote is in progress.
//
// ----------------------------------------------------------------------- //

static void SendCancelVoteInProgress( HCLIENT hTarget )
{
	// Tell the player why their vote didn't start
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_VOTE );
	cMsg.WriteBits( eVote_Cancel_InProgress, FNumBitsExclusive<kNumVoteActions>::k_nValue );
	g_pLTServer->SendToClient( cMsg.Read(), hTarget, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SendCancelNotEnoughPlayers
//
//	PURPOSE:	Sends a cancel message when not enough players to call vote
//
// ----------------------------------------------------------------------- //

static void SendCancelNotEnoughPlayers( HCLIENT hTarget )
{
	// Tell the player why their vote didn't start
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_VOTE );
	cMsg.WriteBits( eVote_Cancel_Players, FNumBitsExclusive<kNumVoteActions>::k_nValue );
	g_pLTServer->SendToClient( cMsg.Read(), hTarget, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::HandleVoteStart
//
//	PURPOSE:	Handle client trying to initiate voting
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::HandleVoteStart(HCLIENT hSender, ILTMessage_Read* pMsg)
{
	if (!hSender)
	{
		return;
	}

	if (IsVoteInProgress())
	{
		// Tell the player why their vote didn't start
		SendCancelVoteInProgress( hSender );
		return;

	}

	VoteType eVoteType = (VoteType)pMsg->ReadBits( FNumBitsExclusive<kNumVoteTypes>::k_nValue );

	//check to see if we have a real client
	uint32 nCallerID = g_pLTServer->GetClientID( hSender );
	HCLIENT hCallerClient = g_pLTServer->GetClientHandle( nCallerID );
	if( !hCallerClient )
		return;
	GameClientData* pCallerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( hCallerClient );
	if( !pCallerGameClientData )
		return;

	//check to see if the client has a live player...
	if (!GameModeMgr::Instance( ).m_grbAllowDeadVoting)
	{
		CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->HandleToObject( pCallerGameClientData->GetPlayer( ));
		if( !pPlayerObj || !pPlayerObj->IsAlive( ))
		{
			return;
		}
	};
	

	

	// Make sure we start fresh.
	ClearVote( );

	switch(eVoteType)
	{
	case eVote_Kick:
	case eVote_TeamKick:
	case eVote_Ban:
		{
			uint32 nTargetID = pMsg->Readuint32();
			HCLIENT hTargetClient = g_pLTServer->GetClientHandle( nTargetID );
			if( !hTargetClient )
				return;
			GameClientData* pTargetGameClientData = ServerConnectionMgr::Instance().GetGameClientData( hTargetClient );
			if( !pTargetGameClientData )
				return;
	
			// Iterate through all the clients and see if anyone is ready to vote.
			ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
			ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
			for( ; iter != gameClientDataList.end( ); iter++ )
			{
				GameClientData* pGameClientData = *iter;
				if( !pGameClientData->GetClient( ))
					continue;

				// Skip clients that aren't ready to play yet.
				if( pGameClientData->GetClientConnectionState() != eClientConnectionState_InWorld )
					continue;

				// Client must have reached the inworld state.
				if( !pGameClientData->IsClientInWorld( ))
					continue;

				// Restrict elibible voters if it's a team kick.
				if( eVoteType == eVote_TeamKick )
				{
					if( pCallerGameClientData->GetLastTeamId() != pGameClientData->GetLastTeamId( ))
						continue;
				}

				// Add to the eligible voter list.
				m_lstEligibleVoter.push_back( pGameClientData->GetClient( ));
			}

			// Check if we have a quorum of voters.
			uint32 nQuorum = ( eVoteType == eVote_TeamKick ) ? GameModeMgr::Instance().m_ServerSettings.m_nMinPlayersForTeamVote :
				GameModeMgr::Instance().m_ServerSettings.m_nMinPlayersForVote;

			//figure out how many votes are needed to pass
			uint8	nVotesNeeded = ((m_lstEligibleVoter.size() + 2) / 2); //the "+ 2" is here to ensure a majority not just half

			//if we have less than the "minimum" number of players, the vote must pass unanimously
			if( m_lstEligibleVoter.size( ) < nQuorum )
			{
				nVotesNeeded = m_lstEligibleVoter.size( );
			}

			// Put the caller on the list of voters already cast.
			m_lstVoterCastYes.push_back( pCallerGameClientData->GetClient( ));

			float fDuration = GameModeMgr::Instance().m_ServerSettings.m_nVoteLifetime;
			m_VoteTimer.Start(fDuration);

			m_CurrentVote.m_eVoteType = eVoteType;
			m_CurrentVote.m_nVoteID = m_CurrentVote.m_nVoteID++; //increment vote ID so that each vote is more or less unique... will wrap after 256 votes
			m_CurrentVote.m_nTargetID = nTargetID;
			m_CurrentVote.m_nCallerID = nCallerID;
			m_CurrentVote.m_nNoVotes = 0;
			m_CurrentVote.m_nYesVotes = 1;
			m_CurrentVote.m_nVotesNeeded = nVotesNeeded;

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_VOTE );
			cMsg.WriteBits( eVote_Start, FNumBitsExclusive<kNumVoteActions>::k_nValue );
			cMsg.Writeuint8( m_CurrentVote.m_nVoteID );
			cMsg.WriteBits( eVoteType, FNumBitsExclusive<kNumVoteTypes>::k_nValue );
			cMsg.Writeuint32( nCallerID );
			cMsg.Writeuint32( nTargetID );
			cMsg.Writeuint8( m_CurrentVote.m_nVotesNeeded );
			cMsg.Writedouble( m_VoteTimer.GetTimeLeft( ));

			// Send the vote start info to the eligible voters.
			SendToEligibleVoters( *cMsg.Read( ));

			// Check if we've already achieved necessary votes.
			CheckVoteStatus();
		}
		break;
	case eVote_NextRound:
	case eVote_NextMap:
		{
	
			// Iterate through all the clients and see if anyone is ready to vote.
			ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
			ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
			for( ; iter != gameClientDataList.end( ); iter++ )
			{
				GameClientData* pGameClientData = *iter;
				if( !pGameClientData->GetClient( ))
					continue;

				// Skip clients that aren't ready to play yet.
				if( pGameClientData->GetClientConnectionState() != eClientConnectionState_InWorld )
					continue;

				// Client must have reached the inworld state.
				if( !pGameClientData->IsClientInWorld( ))
					continue;

				// Add to the eligible voter list.
				m_lstEligibleVoter.push_back( pGameClientData->GetClient( ));
			}

			// Check if we have a quorum of voters.
			uint32 nQuorum = GameModeMgr::Instance().m_ServerSettings.m_nMinPlayersForVote;

			//figure out how many votes are needed to pass
			uint8	nVotesNeeded = ((m_lstEligibleVoter.size() + 2) / 2); //the "+ 2" is here to ensure a majority not just half
			//if we have less than the "minimum" number of players, the vote must pass unanimously
			if( m_lstEligibleVoter.size( ) < nQuorum )
			{
				nVotesNeeded = m_lstEligibleVoter.size( );
			}

			// Put the caller on the list of voters already cast.
			m_lstVoterCastYes.push_back( pCallerGameClientData->GetClient( ));

			float fDuration = GameModeMgr::Instance().m_ServerSettings.m_nVoteLifetime;
			m_VoteTimer.Start(fDuration);

			m_CurrentVote.m_eVoteType = eVoteType;
			m_CurrentVote.m_nVoteID = m_CurrentVote.m_nVoteID++; //increment vote ID so that each vote is more or less unique... will wrap after 256 votes
			m_CurrentVote.m_nTargetID = 0;
			m_CurrentVote.m_nCallerID = nCallerID;
			m_CurrentVote.m_nNoVotes = 0;
			m_CurrentVote.m_nYesVotes = 1;
			m_CurrentVote.m_nVotesNeeded = nVotesNeeded;

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_VOTE );
			cMsg.WriteBits( eVote_Start, FNumBitsExclusive<kNumVoteActions>::k_nValue );
			cMsg.Writeuint8( m_CurrentVote.m_nVoteID );
			cMsg.WriteBits( eVoteType, FNumBitsExclusive<kNumVoteTypes>::k_nValue );
			cMsg.Writeuint32( nCallerID );
			cMsg.Writeuint32( 0 );
			cMsg.Writeuint8( m_CurrentVote.m_nVotesNeeded );
			cMsg.Writedouble( m_VoteTimer.GetTimeLeft( ));

			// Send the vote start info to the eligible voters.
			SendToEligibleVoters( *cMsg.Read( ));

			// Check if we've already achieved necessary votes.
			CheckVoteStatus();
		}
		break;
	case eVote_SelectMap:
		{
			uint32 nMapIndex = pMsg->Readuint32();
			uint32 nCallerID = g_pLTServer->GetClientID( hSender );
			HCLIENT hCallerClient = g_pLTServer->GetClientHandle( nCallerID );
			if( !hCallerClient )
				return;
			GameClientData* pCallerGameClientData = ServerConnectionMgr::Instance().GetGameClientData( hCallerClient );
			if( !pCallerGameClientData )
				return;

			// Iterate through all the clients and see if anyone is ready to vote.
			ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
			ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
			for( ; iter != gameClientDataList.end( ); iter++ )
			{
				GameClientData* pGameClientData = *iter;
				if( !pGameClientData->GetClient( ))
					continue;

				// Skip clients that aren't ready to play yet.
				if( pGameClientData->GetClientConnectionState() != eClientConnectionState_InWorld )
					continue;

				// Client must have reached the inworld state.
				if( !pGameClientData->IsClientInWorld( ))
					continue;

				// Add to the eligible voter list.
				m_lstEligibleVoter.push_back( pGameClientData->GetClient( ));
			}

			// Check if we have a quorum of voters.
			uint32 nQuorum = GameModeMgr::Instance().m_ServerSettings.m_nMinPlayersForVote;
			//figure out how many votes are needed to pass
			uint8	nVotesNeeded = ((m_lstEligibleVoter.size() + 2) / 2); //the "+ 2" is here to ensure a majority not just half

			//if we have less than the "minimum" number of players, the vote must pass unanimously
			if( m_lstEligibleVoter.size( ) < nQuorum )
			{
				nVotesNeeded = m_lstEligibleVoter.size( );
			}

			// Put the caller on the list of voters already cast.
			m_lstVoterCastYes.push_back( pCallerGameClientData->GetClient( ));

			float fDuration = GameModeMgr::Instance().m_ServerSettings.m_nVoteLifetime;
			m_VoteTimer.Start(fDuration);

			m_CurrentVote.m_eVoteType = eVoteType;
			m_CurrentVote.m_nVoteID = m_CurrentVote.m_nVoteID++; //increment vote ID so that each vote is more or less unique... will wrap after 256 votes
			m_CurrentVote.m_nTargetID = nMapIndex;
			m_CurrentVote.m_nCallerID = nCallerID;
			m_CurrentVote.m_nNoVotes = 0;
			m_CurrentVote.m_nYesVotes = 1;
			m_CurrentVote.m_nVotesNeeded = nVotesNeeded;

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_VOTE );
			cMsg.WriteBits( eVote_Start, FNumBitsExclusive<kNumVoteActions>::k_nValue );
			cMsg.Writeuint8( m_CurrentVote.m_nVoteID );
			cMsg.WriteBits( eVoteType, FNumBitsExclusive<kNumVoteTypes>::k_nValue );
			cMsg.Writeuint32( nCallerID );
			cMsg.Writeuint32( nMapIndex );
			cMsg.Writeuint8( m_CurrentVote.m_nVotesNeeded );
			cMsg.Writedouble( m_VoteTimer.GetTimeLeft( ));

			// Send the vote start info to the eligible voters.
			SendToEligibleVoters( *cMsg.Read( ));

			// Check if we've already achieved necessary votes.
			CheckVoteStatus();
		}
		break;

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::HandleVoteCast
//
//	PURPOSE:	Handle client trying to vote
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::HandleVoteCast(HCLIENT hVoterClient, ILTMessage_Read* pMsg)
{
	if (!IsVoteInProgress())
	{
		return;
	}

	uint32 nVoteID = pMsg->Readuint8();
	bool bVote = pMsg->Readbool();


	if (nVoteID != m_CurrentVote.m_nVoteID)
	{
		return;
	}

	VoterList::iterator iterVoter;

	// Check if they are eligible.
	if( !IsClientEligible( hVoterClient, iterVoter ))
		return;

	// Check if they already voted.
	if( IsClientVoteYes( hVoterClient, iterVoter ) || IsClientVoteNo( hVoterClient, iterVoter ))
		return;

	if (bVote)
	{
		m_lstVoterCastYes.push_back( hVoterClient );
		m_CurrentVote.m_nYesVotes++;
	}
	else
	{
		m_lstVoterCastNo.push_back( hVoterClient );
		m_CurrentVote.m_nNoVotes++;
	}

	// Check the status of the vote.
	CheckVoteStatus( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::CheckVoteStatus
//
//	PURPOSE:	Checks the vote status and sends out to clients.
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::CheckVoteStatus( )
{
	//check to see if this requires a unanimous vote
	bool bUnanimous = m_CurrentVote.m_nVotesNeeded >= m_lstEligibleVoter.size();

	if (m_CurrentVote.m_nYesVotes >= m_CurrentVote.m_nVotesNeeded)
	{
		HandleVotePass();
	}
	else if (m_CurrentVote.m_nNoVotes >= m_CurrentVote.m_nVotesNeeded )
	{
		HandleVoteFail();
	}
	else if (bUnanimous && m_CurrentVote.m_nNoVotes > 0)
	{
		HandleVoteFail();
	}
	else
	{
		//Update clients
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_VOTE );
		cMsg.WriteBits( eVote_Cast, FNumBitsExclusive<kNumVoteActions>::k_nValue );
		cMsg.Writeuint8( m_CurrentVote.m_nVoteID );
		cMsg.Writeuint8( m_CurrentVote.m_nYesVotes );
		cMsg.Writeuint8( m_CurrentVote.m_nNoVotes );
		cMsg.Writeuint8( m_CurrentVote.m_nVotesNeeded );
		// Send the vote status info to the eligible voters.
		SendToEligibleVoters( *cMsg.Read( ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::HandleVotePass
//
//	PURPOSE:	Handle vote passing.
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::HandleVotePass( )
{
	// Tell everyone the vote passed.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_VOTE );
	cMsg.WriteBits( eVote_Pass, FNumBitsExclusive<kNumVoteActions>::k_nValue );
	// Send the vote status info to the eligible voters.
	SendToEligibleVoters( *cMsg.Read( ));

	switch( m_CurrentVote.m_eVoteType )
	{
	case eVote_Kick:
	case eVote_TeamKick:
		{
			// Get the client.
			HCLIENT hClient = g_pLTServer->GetClientHandle( m_CurrentVote.m_nTargetID );			
			if( hClient )
			{
				// Kick the client.
				g_pLTServer->KickClient( hClient );
			}
		}
		break;
	case eVote_Ban:
		{
			// get the user's unique CD Key hash
			const char* pszUserCDKeyHash = NULL;
			g_pGameServerShell->GetGameSpyServer()->GetUserCDKeyHash(m_CurrentVote.m_nTargetID, pszUserCDKeyHash);	

			// get the user's player name
			const wchar_t* pwszPlayerName;
			HCLIENT hClient = g_pLTServer->GetClientHandle( m_CurrentVote.m_nTargetID );			
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( hClient );
			if (!pGameClientData)
			{
				pwszPlayerName = L"";
			}
			else
			{
				pwszPlayerName = pGameClientData->GetUniqueName( );
			}

			// add them to the temp ban list
			float fDuration = GameModeMgr::Instance( ).m_ServerSettings.m_nVoteBanDuration * 60.0f;
			BanUserMgr::Instance().AddTempBan( pszUserCDKeyHash, MPW2A(pwszPlayerName).c_str(), fDuration );
		}
		break;
	case eVote_NextRound:
		{
			g_pServerMissionMgr->NextRound( );
		}
		break;
	case eVote_NextMap:
		{
			g_pServerMissionMgr->NextMission( );

		}
		break;
	case eVote_SelectMap:
		{
			// Check to make sure the mission index is within range.
			Campaign& campaign = g_pServerMissionMgr->GetCampaign( );
			if( m_CurrentVote.m_nTargetID < campaign.size( ))
			{
				// Do the switch.
				g_pServerMissionMgr->SwitchToCampaignIndex( m_CurrentVote.m_nTargetID );
			}

		}
		break;

	}

	// Clear the current vote.
	ClearVote( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::HandleVoteFail
//
//	PURPOSE:	Handle vote failing.
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::HandleVoteFail( )
{
	// Tell everyone the vote failed.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_VOTE );
	cMsg.WriteBits( eVote_Fail, FNumBitsExclusive<kNumVoteActions>::k_nValue );
	// Send the vote status info to the eligible voters.
	SendToEligibleVoters( *cMsg.Read( ));

	// Clear the current vote.
	ClearVote( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ServerVoteMgr::OnRemoveClient
//
//	PURPOSE:	Handle RemoveClient event
//
// ----------------------------------------------------------------------- //

void ServerVoteMgr::OnRemoveClient( ServerVoteMgr* pServerVoteMgr, CGameServerShell* pGameServerShell, EventCaster::NotifyParams& notifyParams )
{
	// Don't worry about it if there isn't a vote.
	if (!pServerVoteMgr->IsVoteInProgress())
	{
		return;
	}

	CGameServerShell::RemoveClientNotifyParams& removeClientNotifyParams = ( CGameServerShell::RemoveClientNotifyParams& )notifyParams;
	HCLIENT hClientRemoved = removeClientNotifyParams.m_hClient;

	//handle any vote type specific functionality
	switch( pServerVoteMgr->m_CurrentVote.m_eVoteType )
	{
	case eVote_Kick:
	case eVote_TeamKick:
	case eVote_Ban:
		{
		// Check if the client that left was the target.
		HCLIENT hTargetClient = g_pLTServer->GetClientHandle( pServerVoteMgr->m_CurrentVote.m_nTargetID );
		if( hTargetClient == hClientRemoved )
		{
			pServerVoteMgr->HandleVoteFail();
			return;
		}
		break;
		}
	};


	VoterList::iterator iterVoter;

	// Check if they are not eligible to vote.
	if( !pServerVoteMgr->IsClientEligible( hClientRemoved, iterVoter ))
		return;

	// Remove them from the eligible list.
	pServerVoteMgr->m_lstEligibleVoter.erase( iterVoter );

	// Check if we no longer have a quorem.
	uint32 nQuorum = ( pServerVoteMgr->m_CurrentVote.m_eVoteType == eVote_TeamKick ) ? GameModeMgr::Instance().m_ServerSettings.m_nMinPlayersForTeamVote :
		GameModeMgr::Instance().m_ServerSettings.m_nMinPlayersForVote;

		//figure out how many votes are needed to pass
	uint8	nVotesNeeded = ((pServerVoteMgr->m_lstEligibleVoter.size() + 2) / 2); //the "+ 2" is here to ensure a majority not just half

	//if we have less than the "minimum" number of players, the vote must pass unanimously
	if( pServerVoteMgr->m_lstEligibleVoter.size( ) < nQuorum )
	{
		nVotesNeeded = pServerVoteMgr->m_lstEligibleVoter.size( );
	}

	// Update the votes needed.
	pServerVoteMgr->m_CurrentVote.m_nVotesNeeded = nVotesNeeded;

	// Remove the client's vote from the poll.
	if( pServerVoteMgr->IsClientVoteYes( hClientRemoved, iterVoter ))
	{
		pServerVoteMgr->m_lstVoterCastYes.erase( iterVoter );
		pServerVoteMgr->m_CurrentVote.m_nYesVotes--;
	}
	else if( pServerVoteMgr->IsClientVoteNo( hClientRemoved, iterVoter ))
	{
		pServerVoteMgr->m_lstVoterCastNo.erase( iterVoter );
		pServerVoteMgr->m_CurrentVote.m_nNoVotes--;
	}

	// Check the current vote status.
	pServerVoteMgr->CheckVoteStatus();
}
