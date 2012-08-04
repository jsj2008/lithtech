// MenuSaveGame.h: interface for the CMenuSaveGame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUSAVEGAME_H__31608232_5BE6_11D2_BDA1_0060971BDC6D__INCLUDED_)
#define AFX_MENUSAVEGAME_H__31608232_5BE6_11D2_BDA1_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuSaveGame : public CMenuBase  
{
public:
	CMenuSaveGame();
	virtual ~CMenuSaveGame();

	// Build the menu
	void	Build();			

	// Add the menu items
	void	InitSavedGameSlots();

protected:

	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENUSAVEGAME_H__31608232_5BE6_11D2_BDA1_0060971BDC6D__INCLUDED_)
