// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMutePlayer.h
//
// PURPOSE : Interface screen to mute specific players
//
// CREATED : 06/21/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCREENMUTEPLAYER_H__
#define __SCREENMUTEPLAYER_H__


#include "BaseScreen.h"


class CScreenMutePlayer : public CBaseScreen
{
public:
	CScreenMutePlayer();
	virtual ~CScreenMutePlayer();

	// Build the screen
    bool  Build();

    void    OnFocus(bool bFocus);

	void	UpdateLists();


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	

	CLTGUITextCtrl*			m_pAdd;
	CLTGUITextCtrl*			m_pRemove;
	CLTGUITextCtrl*			m_pAddAll;
	CLTGUITextCtrl*			m_pRemoveAll;
	CLTGUIListCtrlEx*		m_pUnmuted;
	CLTGUIScrollBar*		m_pUnmutedScrollBar;
	CLTGUIListCtrlEx*		m_pMuted;
	CLTGUIScrollBar*		m_pMutedScrollBar;

};

#endif  // __SCREENMUTEPLAYER_H__
