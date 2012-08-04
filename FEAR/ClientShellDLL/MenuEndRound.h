// ----------------------------------------------------------------------- //
//
// MODULE  : MenuEndRound.h
//
// PURPOSE : End of round menu.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(__MENUENDROUND_H__)
#define __MENUENDROUND_H__

#include "BaseMenu.h"

class ScoresCtrl;

class CMenuEndRound : public CBaseMenu
{
public:

	CMenuEndRound( );

	virtual bool	Init( CMenuMgr& menuMgr );

	virtual	bool	OnMouseMove(int x, int y) { return false; }

	HRECORD			GetMenuRecord();

	virtual void    Show ( bool bShow );

private:

	ScoresCtrl* m_pScoresCtrl;
	CLTGUITextCtrl* m_pTitleCtrl;
	CLTGUITextCtrl* m_pEndRoundCondition;
};

#endif //!defined(__MENUENDROUND_H__)