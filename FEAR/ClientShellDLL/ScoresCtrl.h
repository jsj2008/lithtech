//-------------------------------------------------------------------------
//
// MODULE  : ScoresCtrl.h
//
// PURPOSE : Control that shows the current scores.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __SCORESCTRL_H__
#define __SCORESCTRL_H__


#include "LTGUIMgr.h"
#include "EventCaster.h"
#include "HUDScores.h"

class CClientInfoMgr;

class ScoresCtrl : public CLTGUICtrl
{
public:
	ScoresCtrl();
	virtual ~ScoresCtrl() {Destroy();}

// CLTGUICtrl implementation.
public:

	virtual void    Create(const CLTGUICtrl_create& cs);
	virtual void	Destroy();

	virtual void	SetBasePos( const LTVector2n& pos );
	virtual void	SetScale( const LTVector2& vfScale);

	// Render the control
	virtual void	Render ();
	virtual void	RenderTransition(float fTrans) {Render();}

	// Since this is for ingame and not on a screen, we don't need
	// to bother flushing to save memory, since this will impact performance when
	// switching in and out of focus.
	virtual void	FlushTextureStrings() {}
	virtual void	RecreateTextureStrings() {}

	// Show/Hide the control
	virtual void    Show( bool bShow );

protected:

	static void OnScoresChangedEvent( ScoresCtrl* pScoresCtrl, CClientInfoMgr* pClientInfoMgr, EventCaster::NotifyParams& notifyParams )
	{
		pScoresCtrl->m_bScoresUpdated = true;
	}
	Delegate< ScoresCtrl, CClientInfoMgr, OnScoresChangedEvent > m_dlgScoresChanged;

protected:

	// Host the scores hud item.
	CHUDScores m_HudScores;

	// Header text.
	CLTGUITextCtrl m_Header;

	bool m_bScoresUpdated;
};


#endif