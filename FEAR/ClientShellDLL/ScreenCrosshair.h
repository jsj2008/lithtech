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


#include "BaseScreen.h"

class CScreenCrosshair : public CBaseScreen
{
public:
	CScreenCrosshair();
	virtual ~CScreenCrosshair();

	// Build the screen
    bool   Build();
    void   OnFocus(bool bFocus);
    bool   Render();

    bool   OnLeft();
    bool   OnRight();
    bool   OnLButtonUp(int x, int y);
    bool   OnRButtonUp(int x, int y);

protected:
	void	GetConsoleVariables();
	void	SetConsoleVariables();	

protected:

	int				m_nColorA;
	int				m_nColorR;
	int				m_nColorG;
	int				m_nColorB;
	int				m_nCrosshairSize;

	CLTGUIFrame*	m_pPlaceholder;

};

#endif // !defined(_SCREEN_CROSSHAIR_H_)