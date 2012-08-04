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

	virtual bool	Init( CMenuMgr& menuMgr );

	virtual void OnFocus(bool bFocus);

	// Handle a command
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);

	// Render the control
	virtual void Render ();

private:

	CLTGUITextCtrl*	m_pSaveCtrl;
	CLTGUITextCtrl*	m_pLoadCtrl;
	CLTGUITextCtrl*	m_pPlayerCtrl;
	CLTGUITextCtrl*	m_pTeamCtrl;
	CLTGUITextCtrl*	m_pWpnCtrl;
	CLTGUITextCtrl*	m_pStoryCtrl;
	CLTGUITextCtrl*	m_pHostCtrl;
	CLTGUITextCtrl*	m_pServerCtrl;
	CLTGUITextCtrl*	m_pServerIPCtrl;
	CLTGUITextCtrl*	m_pVoteCtrl;
	CLTGUITextCtrl*	m_pMuteCtrl;
};

#endif //!defined(_MENU_SYSTEM_H_)