// ----------------------------------------------------------------------- //
//
// MODULE  : SharedVoting.h
//
// PURPOSE : Provide common data definitions for User Voting
//
// CREATED : 12/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHAREDVOTING_H__
#define __SHAREDVOTING_H__

enum VoteType
{
	eVote_Kick,
	eVote_TeamKick,
	eVote_Ban,
	eVote_NextRound,
	eVote_NextMap,
	eVote_SelectMap,

	eVote_None,
	kNumVoteTypes = eVote_None
};



enum VoteAction
{
	eVote_Start,
	eVote_Cast,
	eVote_Pass,
	eVote_Fail,				// too many no votes
	eVote_Expire,			// too much time expired
	eVote_Cancel_Players,	// not enough valid players to vote
	eVote_Cancel_InProgress, // another vote is already in progress

	eVote_NoAction,
	kNumVoteActions = eVote_NoAction
};

class VoteData
{
public:
	VoteData()
	{
		m_eVoteType = eVote_None;
		m_nVoteID = (uint8)-1;
		m_nCallerID = (uint32)-1;
		m_nTargetID = (uint32)-1;
	};

	~VoteData() {};

	VoteType	m_eVoteType;
	uint8		m_nVoteID;
	uint32		m_nCallerID; //the client ID of the person calling the vote
	
	uint32		m_nTargetID; //for vote kicking m_nTargetID is the client ID of the person to be kicked

	uint8		m_nYesVotes;
	uint8		m_nNoVotes;

	uint8		m_nVotesNeeded; //number of yes votes need to pass vote

};


#endif  // __SHAREDVOTING_H__
