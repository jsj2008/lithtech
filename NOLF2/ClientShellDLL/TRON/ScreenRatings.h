// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenRatings.h
//
// PURPOSE : Screen for managing player's performance ratings
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_RATINGS_H_
#define _SCREEN_RATINGS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "BaseScaleFX.h"
#include "ArcCtrl.h"
#include "RatingMgr.h"
#include "ScreenSubroutines.h"
#include "SubroutineMgr.h"

class CScreenRatings : public CBaseScreen
{
public:
	CScreenRatings();
	virtual ~CScreenRatings();

	// Build the screen
    LTBOOL   Build();

	// This is called when the screen gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);

	void	Escape();
	void	Compile();

	void	ClearScreen();

	void	AddSubroutine(PlayerSubroutine * pSub);

	LTBOOL	OnMouseMove(int x, int y);

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    LTBOOL	Render(HSURFACE hDestSurf);

	void	SetCursorMode(SubCursorMode eMode);

	int			m_nAdditives;
	int			m_nAdditiveLibSize;

	SubCursorMode m_eCursorMode;

	ADDITIVE	* m_pHotAdd;
	ADDITIVE	* m_pCursorAdd;

	CArcCtrl	m_SystemMemoryCtrl;
	CArcCtrl	m_AdditiveCtrl;
};

#endif // _SCREEN_RATINGS_H_