// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenFailure.h
//
// PURPOSE : Interface screen for handling mission failure
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_FAILURE_H_)
#define _SCREEN_FAILURE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenFailure : public CBaseScreen
{
public:
	CScreenFailure();
	virtual ~CScreenFailure();

	// Build the screen
    virtual LTBOOL	Build();
    virtual void	OnFocus(LTBOOL bFocus);
	virtual void	Escape();

    virtual LTBOOL	HandleKeyUp(int key, int rep);
    virtual LTBOOL	OnLButtonUp(int x, int y);
    virtual LTBOOL	OnRButtonUp(int x, int y);

    virtual LTBOOL	Render(HSURFACE hDestSurf);


protected:

	CLTGUITextCtrl		*m_pString;
	CLTGUITextCtrl		*m_pHelp;
	
	bool				m_bSentAutoLoadMessage;


};

#endif // !defined(_SCREEN_FAILURE_H_)