// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostVoting.h
//
// PURPOSE : Screen to set server options related to user voting
//
// CREATED : 12/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENHOSTVOTING_H__
#define __SCREENHOSTVOTING_H__

#include "SharedVoting.h"

class CScreenHostVoting : public CBaseScreen
{
public:
	CScreenHostVoting();
	virtual ~CScreenHostVoting();

	// Build the screen
	virtual bool	Build();

	virtual void    OnFocus(bool bFocus);


protected:

	bool			m_bAllowVote[kNumVoteTypes];
	int				m_nMinPlayersForVote;
	int				m_nMinPlayersForTeamVote;
	int				m_nVoteLifetime;		//seconds
	int				m_nVoteBanDuration;		//minutes
	int				m_nVoteDelay;		//seconds

};




#endif  // __SCREENHOSTVOTING_H__
