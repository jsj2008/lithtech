// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenVote.h
//
// PURPOSE : Interface screen for player voting
//
// CREATED : 12/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENVOTE_H__
#define __SCREENVOTE_H__

#include "SharedVoting.h"

class CScreenVote : public CBaseScreen
{
public:
	CScreenVote();
	virtual ~CScreenVote();

	// Build the screen
	virtual bool Build();

	virtual void OnFocus(bool bFocus);

	virtual void Escape();

	void	UpdateMapList( );


protected:
	
	virtual uint32 OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	void	UpdatePlayerList();

protected:
	VoteType		m_eVoteType;

	CLTGUITextCtrl* m_pVoteKick;
	CLTGUITextCtrl* m_pVoteTeamKick;
	CLTGUITextCtrl* m_pVoteBan;


	CLTGUITextCtrl* m_pVoteNextRound;
	CLTGUITextCtrl* m_pVoteNextMap;
	CLTGUITextCtrl* m_pVoteMap;

	CLTGUIListCtrl* m_pPlayers;
	CLTGUIListCtrl* m_pMaps;


};

#endif  // __SCREENVOTE_H__
