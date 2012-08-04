// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoystick.h
//
// PURPOSE : Interface screen for joystick/gamepad settings
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_JOYSTICK_H_
#define _SCREEN_JOYSTICK_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenJoystick : public CBaseScreen
{
public:
	CScreenJoystick();
	virtual ~CScreenJoystick();

	// Build the screen
    bool   Build();
    void OnFocus(bool bFocus);

protected:
	void	ClearBindings();
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	void	LoadBindings();
	void	SaveBindings();

	Tuint8List m_aAxisBindings;
	uint32 m_nJoystickDevice, m_nFirstAxis;
};

#endif // _SCREEN_KEYBOARD_H_