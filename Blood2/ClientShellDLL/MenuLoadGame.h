// MenuLoadGame.h: interface for the CMenuLoadGame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENULOADGAME_H__31608231_5BE6_11D2_BDA1_0060971BDC6D__INCLUDED_)
#define AFX_MENULOADGAME_H__31608231_5BE6_11D2_BDA1_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuLoadGame : public CMenuBase  
{
public:
	CMenuLoadGame();
	virtual ~CMenuLoadGame();

	// Build the menu
	void	Build();		

	// Add the menu items
	void	InitSavedGameSlots();

protected:

	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENULOADGAME_H__31608231_5BE6_11D2_BDA1_0060971BDC6D__INCLUDED_)
