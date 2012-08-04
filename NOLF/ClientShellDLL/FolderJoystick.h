// FolderJoystick.h: interface for the CFolderJoystick class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_JOYSTICK_H_
#define _FOLDER_JOYSTICK_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CJoystickAxisTurn;
class CJoystickAxisLook;
class CJoystickAxisStrafe;
class CJoystickAxisMove;


class CFolderJoystick : public CBaseFolder
{
public:
	CFolderJoystick();
	virtual ~CFolderJoystick();

	// Build the folder
    LTBOOL   Build();

	// Change in focus
    void    OnFocus(LTBOOL bFocus);

	// Override the left/right input
    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnEnter();
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);

	LTBOOL	IsJoystickConfigured();

    virtual LTBOOL   NextPage(LTBOOL bChangeSelection = LTTRUE);
    virtual LTBOOL   PreviousPage(LTBOOL bChangeSelection = LTTRUE);


protected:
	// Build the axis folders
	void	BuildAxisFolders();


	void	UpdateAxes();
	// Update the enable/disabled status of the controls
	void	UpdateEnable();

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:
    LTBOOL                       m_bUseJoystick;     // TRUE if the joystick should be used

	CJoystickAxisTurn		*m_pAxisTurn;		// The turn left/right axis
	CJoystickAxisLook		*m_pAxisLook;		// The look up/down axis
	CJoystickAxisStrafe		*m_pAxisStrafe;		// The strafe left/right axis
	CJoystickAxisMove		*m_pAxisMove;		// The move forward/backward axis

	int			m_nCurrPage;

};

#endif // _FOLDER_MOUSE_H_