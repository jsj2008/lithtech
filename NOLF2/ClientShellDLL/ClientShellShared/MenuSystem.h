// ----------------------------------------------------------------------- //
//
// MODULE  : MenuSystem.h
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_SYSTEM_H_)
#define _MENU_SYSTEM_H_

#include "BaseMenu.h"


class CMenuSystem : public CBaseMenu
{
public:

	CMenuSystem( );

	virtual LTBOOL	Init();

	virtual void OnFocus(LTBOOL bFocus);

	// Handle a command
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);

private:

	CLTGUITextCtrl*	m_pSaveCtrl;
	CLTGUITextCtrl*	m_pLoadCtrl;
	CLTGUITextCtrl*	m_pPlayerCtrl;
	CLTGUITextCtrl*	m_pTeamCtrl;
	CLTGUITextCtrl*	m_pHostCtrl;
	CLTGUITextCtrl*	m_pServerCtrl;
};

#endif //!defined(_MENU_SYSTEM_H_)