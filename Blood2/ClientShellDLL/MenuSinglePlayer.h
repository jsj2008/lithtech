// MenuSinglePlayer.h: interface for the CMenuSinglePlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUSINGLEPLAYER_H__D7668B33_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUSINGLEPLAYER_H__D7668B33_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuSinglePlayer : public CMenuBase  
{
public:
	CMenuSinglePlayer();
	virtual ~CMenuSinglePlayer();

	// Build the menu
	void	Build();

	// Enable/Disable the save option
	void	EnableSave(DBOOL bEnable);

protected:
	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);

protected:
	CLTGUICtrl	*m_pSaveCtrl;
};

#endif // !defined(AFX_MENUSINGLEPLAYER_H__D7668B33_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
