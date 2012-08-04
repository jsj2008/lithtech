// ----------------------------------------------------------------------- //
//
// MODULE  : ServerVoteMgr.h
//
// PURPOSE : Manage server side of player voting
//
// CREATED : 12/02/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERVOTEMGR_H__
#define __SERVERVOTEMGR_H__

#include "SharedVoting.h"
#include "EventCaster.h"

class ServerVoteMgr
{
private:

	ServerVoteMgr();
	~ServerVoteMgr();

public:

	static ServerVoteMgr& Instance();
	void	Init();

	VoteType VoteInProgress() const {return m_CurrentVote.m_eVoteType;}
	bool	IsVoteInProgress() const {return (m_CurrentVote.m_eVoteType != eVote_None); }

	void	Update();

	void	HandleMsgVote		(HCLIENT, ILTMessage_Read*);

private:

	void	HandleVoteStart		(HCLIENT, ILTMessage_Read*);
	void	HandleVoteCast		(HCLIENT, ILTMessage_Read*);

	void	HandleVotePass();
	void	HandleVoteFail();

	// Checks the vote status and sends out to clients.
	void	CheckVoteStatus( );

	// Defines a list type to track voters.
	typedef std::vector< HCLIENT > VoterList;

	// Check if client is an eligible voter.
	bool	IsClientEligible( HCLIENT hClient, VoterList::iterator& iterVoter )
	{
		iterVoter = std::find( m_lstEligibleVoter.begin( ), m_lstEligibleVoter.end( ), hClient );
		if( iterVoter == m_lstEligibleVoter.end( ))
			return false;

		return true;
	}

	// Has client voted yes.
	bool	IsClientVoteYes( HCLIENT hClient, VoterList::iterator& iterVoter )
	{
		iterVoter = std::find( m_lstVoterCastYes.begin( ), m_lstVoterCastYes.end( ), hClient );
		if( iterVoter == m_lstVoterCastYes.end( ))
			return false;

		return true;
	}

	// Has client voted no.
	bool	IsClientVoteNo( HCLIENT hClient, VoterList::iterator& iterVoter )
	{
		iterVoter = std::find( m_lstVoterCastNo.begin( ), m_lstVoterCastNo.end( ), hClient );
		if( iterVoter == m_lstVoterCastNo.end( ))
			return false;

		return true;
	}

	// Declare our delegate to receive removeclient events.
	static void OnRemoveClient( ServerVoteMgr* pServerVoteMgr, CGameServerShell* pGameServerShell, EventCaster::NotifyParams& notifyParams );
	Delegate< ServerVoteMgr, CGameServerShell, ServerVoteMgr::OnRemoveClient > m_delegateRemoveClient;

	// Clears out the current vote.
	void	ClearVote( )
	{
		m_CurrentVote.m_eVoteType = eVote_None;
		m_VoteTimer.Stop( );
		ltstd::reset_vector( m_lstVoterCastYes );
		ltstd::reset_vector( m_lstVoterCastNo );
		ltstd::reset_vector( m_lstEligibleVoter );
	}

	// Sends a message to all the eligible voters.
	void SendToEligibleVoters( ILTMessage_Read& msg )
	{
		for( VoterList::iterator iter = m_lstEligibleVoter.begin(); iter != m_lstEligibleVoter.end( ); iter++ )
		{
			g_pLTServer->SendToClient( &msg, *iter, MESSAGE_GUARANTEED );
		}
	}

	VoteData	m_CurrentVote;
	StopWatchTimer	m_VoteTimer;

	// List of eligible voters.
	VoterList m_lstEligibleVoter;

	// List of voters that have voted yes.
	VoterList m_lstVoterCastYes;

	// List of voters that have voted no.
	VoterList m_lstVoterCastNo;

private:
	PREVENT_OBJECT_COPYING( ServerVoteMgr );


};


#endif  // __SERVERVOTEMGR_H__
