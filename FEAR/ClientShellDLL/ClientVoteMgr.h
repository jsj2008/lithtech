// ----------------------------------------------------------------------- //
//
// MODULE  : ClientVoteMgr.h
//
// PURPOSE : Manage client-side of player voting
//
// CREATED : 12/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTVOTEMGR_H__
#define __CLIENTVOTEMGR_H__

#include "SharedVoting.h"

class ClientVoteMgr
{
private:

	ClientVoteMgr();
	~ClientVoteMgr();

public:

	static ClientVoteMgr& Instance();
	void	Init();
	virtual void	OnExitWorld();

	bool	IsVoteInProgress() const {return (m_CurrentVote.m_eVoteType != eVote_None); }
	bool	IsVoteDelayed() const {return (m_VoteDelayTimer.IsStarted() && !m_VoteDelayTimer.IsTimedOut()); }
	const VoteData& GetVoteData() const {return m_CurrentVote;	}
	double	GetVoteTimeLeft() const {return m_VoteTimer.GetTimeLeft();}
	const wchar_t* GetVoteString() const {return m_sVoteString.c_str(); }

	bool	HasVoted() const { return (IsVoteInProgress() && m_bHasVoted); }

	void	CallVoteKick(VoteType eVoteType, uint32 nClientID);
	void	CallVoteMap(uint32 nIndex);
	void	CallVoteNext(VoteType eVoteType);

	void	CastVote(bool bVote);

	void	HandleMsgVote		(ILTMessage_Read*);

private:
	void	HandleVoteStart		(ILTMessage_Read*);

	void	HandleVotePass();
	void	ClearVote();

	VoteData	m_CurrentVote;
	bool		m_bHasVoted;
	std::wstring m_sVoteString;
	std::wstring m_sTargetName;
	StopWatchTimer m_VoteTimer;

	StopWatchTimer m_VoteDelayTimer;



private:
	PREVENT_OBJECT_COPYING( ClientVoteMgr );


};

#endif  // __CLIENTVOTEMGR_H__
