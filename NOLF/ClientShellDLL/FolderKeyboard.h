// FolderKeyboard.h: interface for the CFolderKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _FOLDER_KEYBOARD_H_
#define _FOLDER_KEYBOARD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseFolder.h"

class CFolderKeyboard : public CBaseFolder
{
public:
	CFolderKeyboard();
	virtual ~CFolderKeyboard();

	// Build the folder
    LTBOOL   Build();
    void OnFocus(LTBOOL bFocus);

protected:
	void	ClearBindings();
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	CToggleCtrl		*m_pLookCtrl;

    LTBOOL          m_bLookspring;
	int				m_nNormalTurn;
	int	  			m_nFastTurn;
	int	  			m_nVehicleTurn;
	int				m_nLookUp;


    LTBOOL   m_bMouseLook;

};

#endif // _FOLDER_KEYBOARD_H_