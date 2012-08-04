// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenFailure.h
//
// PURPOSE : Interface screen for handling end of mission 
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_FAILURE_H_)
#define _SCREEN_FAILURE_H_

#include "BaseScreen.h"

class CScreenFailure : public CBaseScreen
{
public:
	CScreenFailure();
	virtual ~CScreenFailure();

	// Build the screen
    virtual bool	Build();
    virtual void	OnFocus(bool bFocus);
	virtual void	Escape();

    virtual bool	HandleKeyDown(int key, int rep);
    virtual bool	OnLButtonDown(int x, int y);
    virtual bool	OnRButtonDown(int x, int y);

    virtual bool	Render();


protected:


	CLTGUITextCtrl		*m_pTips;
	CLTGUITextCtrl		*m_pContinue;
	
	float				m_fDuration;
	
	bool				m_bFlash;		
	double				m_fFlashTime;
	
	bool				m_bRestart;

	static uint32		sm_nTipIndex;
};

#endif // !defined(_SCREEN_END_MISSION_H_)