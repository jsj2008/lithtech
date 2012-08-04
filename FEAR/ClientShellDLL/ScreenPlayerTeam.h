// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayerTeam.h
//
// PURPOSE : Interface screen for team selection
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREENPLAYERTEAM_H_
#define _SCREENPLAYERTEAM_H_

#include "BaseScreen.h"

class CScreenPlayerTeam : public CBaseScreen
{
public:
	CScreenPlayerTeam();
	virtual ~CScreenPlayerTeam();


	// Build the screen
    bool   Build();
	void	Term();

    void    OnFocus(bool bFocus);

	virtual void    Escape();

    virtual bool   OnMouseMove(int x, int y);
    virtual bool   OnUp();
    virtual bool   OnDown();
    virtual bool   OnLButtonDown(int x, int y);
    virtual bool   OnLButtonUp(int x, int y);


	// Returns false if the screen should exit as a result of this update
	virtual bool	UpdateInterfaceSFX();


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	virtual void	CreateInterfaceSFX();
	virtual void	RemoveInterfaceSFX();

	void	SelectTeam(uint8 nTeam);
	void	UpdateTeam();

	CLTGUITextCtrl*	m_pTeams[2];
	CLTGUITextCtrl*	m_pCounts[2];
	CLTGUITextCtrl*	m_pScores[2];
	CLTGUIListCtrl* m_pPlayers[2];
	CLTGUITextCtrl*	m_pAuto;

	uint8	m_nTeam;

	StopWatchTimer	m_UpdateTimer;
	StopWatchTimer	m_AutoSelectTimer;

};


#endif // _SCREENPLAYERTEAM_H_