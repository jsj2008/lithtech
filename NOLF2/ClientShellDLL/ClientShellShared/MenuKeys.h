// ----------------------------------------------------------------------- //
//
// MODULE  : MenuKeys.h
//
// PURPOSE : In-game key item menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_KEYS_H_)
#define _MENU_KEYS_H_

#include "BaseMenu.h"

class CMenuKeys : public CBaseMenu
{
public:

	virtual LTBOOL	Init();
	virtual void	Term();

	virtual void OnFocus(LTBOOL bFocus);


	// Handle a command
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);

protected:

	CSubMenu		m_Popup;
	CLTGUITextCtrl	m_Name;
	CLTGUITextCtrl	m_Description;
	CLTGUIButton	m_Image;

};

#endif //!defined(_MENU_KEYS_H_)