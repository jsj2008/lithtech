// ----------------------------------------------------------------------- //
//
// MODULE  : HUDVote.h
//
// PURPOSE : HUD element to display the status of a vote
//
// CREATED : 12/02/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDVOTE_H__
#define __HUDVOTE_H__

class CHUDVote : public CHUDItem
{
public:
	CHUDVote();
	virtual ~CHUDVote() {};

	virtual bool	Init();
	virtual void	Term();

	virtual void	Render();
	virtual void	Update();
	virtual void	ScaleChanged();

	virtual void	UpdateLayout();
	virtual void	UpdateFlash();

private:
	void			UpdateTextPos();

	bool	m_bVisible;

	CLTGUIString	m_Count;
	CLTGUIString	m_Timer;
	CLTGUIString	m_Hotkeys;

	uint32			m_cNormalTextColor;
	uint32			m_cVotedTextColor;

	uint8			m_nLastMinutes;
	uint8			m_nLastSeconds;

	HTEXTURESTRING	m_hTimerTexture;

};

#endif  // __HUDVOTE_H__
