// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenCrosshair.h
//
// PURPOSE : Interface screen for setting crosshair options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_SCREEN_CROSSHAIR_H_)
#define _SCREEN_CROSSHAIR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenCrosshair : public CBaseScreen
{
public:
	CScreenCrosshair();
	virtual ~CScreenCrosshair();

	// Build the screen
    LTBOOL   Build();
    void    OnFocus(LTBOOL bFocus);
    LTBOOL   Render(HSURFACE hDestSurf);

    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);

protected:
	void	GetConsoleVariables();
	void	SetConsoleVariables();

protected:

	CLTGUICycleCtrl		*m_pStyle;

    LTBOOL          m_bCrosshair;
	int				m_nAlpha;
	int				m_nColorR;
	int				m_nColorG;
	int				m_nColorB;
	uint8			m_nStyle;
    LTBOOL          m_bDynamic;

};

#endif // !defined(_SCREEN_CROSSHAIR_H_)