// FolderMouse.h: interface for the CFolderMouse class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_MOUSE_H_
#define _FOLDER_MOUSE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderMouse : public CBaseFolder
{
public:
	CFolderMouse();
	virtual ~CFolderMouse();

	// Build the folder
    LTBOOL   Build();
    void OnFocus(LTBOOL bFocus);

protected:
	void	ClearBindings();
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);


    LTBOOL	m_bInvertY;
    LTBOOL	m_bMouseLook;
	int		m_nInputRate;
	int		m_nSensitivity;
	int	  	m_nVehicleTurn;

	CSliderCtrl			*m_pInputCtrl;
	CSliderCtrl			*m_pSensitivityCtrl;

};

#endif // _FOLDER_MOUSE_H_