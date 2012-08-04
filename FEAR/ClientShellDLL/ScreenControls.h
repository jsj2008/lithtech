// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenControls.h
//
// PURPOSE : Interface screen for setting control options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_CONTROLS_H_
#define _SCREEN_CONTROLS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenControls : public CBaseScreen
{
public:
	CScreenControls();
	virtual ~CScreenControls();

	// Build the folder
    bool   Build();
    void OnFocus(bool bFocus);

	void ConfirmSetting(bool bConfirm);

protected:

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

    bool				m_bUseJoystick;     // TRUE if the joystick should be used
	CLTGUITextCtrl		*m_pJoystickCtrl;

	int m_nConfirm;


};

#endif // _SCREEN_CONTROLS_H_