// MenuJoystick.h: interface for the CMenuJoystick class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUJOYSTICK_H__8B896D02_6918_11D2_BDAE_0060971BDC6D__INCLUDED_)
#define AFX_MENUJOYSTICK_H__8B896D02_6918_11D2_BDAE_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuJoystickAxisTurn;
class CMenuJoystickAxisLook;
class CMenuJoystickAxisStrafe;
class CMenuJoystickAxisMove;

class CMenuJoystick : public CMenuBase  
{
public:
	CMenuJoystick();
	virtual ~CMenuJoystick();

	// Build the menu
	void	Build();			

	// Change in focus
	void	OnFocus(DBOOL bFocus);

protected:
	// Build the axis menus
	void	BuildAxisMenus();

	// Override the left/right input
	void	OnLeft();
	void	OnRight();

	// Update the enable/disabled status of the controls
	void	UpdateEnable();

protected:
	DBOOL						m_bUseJoystick;		// TRUE if the joystick should be used
	DBOOL						m_bUsePovHat;		// TRUE if useing the POV hat is enabled
	CLTGUIOnOffCtrl				*m_pUsePovHat;		// pointer to the UsePovHat control

	CMenuJoystickAxisTurn		*m_pAxisTurn;		// The turn left/right axis
	CMenuJoystickAxisLook		*m_pAxisLook;		// The look up/down axis
	CMenuJoystickAxisStrafe		*m_pAxisStrafe;		// The strafe left/right axis
	CMenuJoystickAxisMove		*m_pAxisMove;		// The move forward/backward axis
};

#endif // !defined(AFX_MENUJOYSTICK_H__8B896D02_6918_11D2_BDAE_0060971BDC6D__INCLUDED_)
