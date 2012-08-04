// FolderControls.h: interface for the CFolderControls class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_CONTROLS_H_
#define _FOLDER_CONTROLS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderControls : public CBaseFolder
{
public:
	CFolderControls();
	virtual ~CFolderControls();

	// Build the folder
    LTBOOL   Build();
    void OnFocus(LTBOOL bFocus);

	void ConfirmSetting(LTBOOL bConfirm);

protected:
	void	ClearBindings();
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

    LTBOOL				m_bUseJoystick;     // TRUE if the joystick should be used
	CLTGUITextItemCtrl	*m_pJoystickCtrl;

	int m_nConfirm;


};

#endif // _FOLDER_CONTROLS_H_