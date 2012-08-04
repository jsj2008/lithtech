// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMain.h
//
// PURPOSE : Top level interface screen
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_MAIN_H_
#define _SCREEN_MAIN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenMain : public CBaseScreen
{
public:
	CScreenMain();
	virtual ~CScreenMain();

	// Build the screen
    bool   Build();

	// This is called when the screen gets or loses focus
    virtual void    OnFocus(bool bFocus);

	virtual void    Escape();

    bool   DoMultiplayer(bool bMinimize);


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    bool   Render();

	void	AutoDetectPerformanceSettings();

	CLTGUITextCtrl* m_pResume;

};

#endif // _SCREEN_MAIN_H_