// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMouse.h
//
// PURPOSE : Interface screen for setting mouse options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_MOUSE_H_
#define _SCREEN_MOUSE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenMouse : public CBaseScreen
{
public:
	CScreenMouse();
	virtual ~CScreenMouse();

	// Build the screen
    bool   Build();
    void OnFocus(bool bFocus);

protected:
	void	ClearBindings();
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);


    bool	m_bInvertY;
	int		m_nInputRate;
	int		m_nSensitivity;
	int	  	m_nVehicleTurn;

	CLTGUISlider			*m_pInputCtrl;
	CLTGUISlider			*m_pSensitivityCtrl;

};

#endif // _SCREEN_MOUSE_H_