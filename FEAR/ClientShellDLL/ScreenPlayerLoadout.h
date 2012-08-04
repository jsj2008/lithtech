// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayerLoadout.h
//
// PURPOSE : Interface screen for team selection
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREENPLAYERLOADOUT_H_
#define _SCREENPLAYERLOADOUT_H_

#include "BaseScreen.h"

class CScreenPlayerLoadout : public CBaseScreen
{
public:
	CScreenPlayerLoadout();
	virtual ~CScreenPlayerLoadout();


	// Build the screen
    virtual bool	Build();

    virtual void    OnFocus(bool bFocus);

	virtual void    Escape();
	virtual bool	UpdateInterfaceSFX();

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	void	SelectLoadout(uint8 nLoadout);
	bool	CheckRestrictions(HRECORD hLoadout);

	uint8	m_nLoadout;

	StringSet m_setRestrictedWeapons;
	StringSet m_setRestrictedGear;

	StopWatchTimer	m_AutoSelectTimer;
	CLTGUITextCtrl*	m_pLevelName;
	CLTGUIListCtrl* m_pList;


};


#endif // _SCREENPLAYERLOADOUT_H_