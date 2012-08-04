// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenTeam.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_TEAM_H_
#define _SCREEN_TEAM_H_

#include "BaseScreen.h"

class CScreenTeam : public CBaseScreen
{
public:
	CScreenTeam();
	virtual ~CScreenTeam();


	// Build the screen
    bool   Build();
	void	Term();

    void    OnFocus(bool bFocus);
/*
	virtual bool OnLeft();
	virtual bool OnRight();
	virtual bool OnUp();
	virtual bool OnDown();
    virtual bool OnMouseMove(int x, int y);
    bool   OnLButtonUp(int x, int y);
    bool   OnRButtonUp(int x, int y);


	void	NextModel();
	void	PrevModel();
*/

protected:
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);
//	void	UpdateChar();

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	CLTGUIColumnCtrl*	m_pName;
//	CLTGUITextCtrl*		m_pModel;
//	CLTGUIButton*		m_pLeft;
//	CLTGUIButton*		m_pRight;

//	INT_CHAR m_Model;

//	int			m_nCurrentModel;
//	int			m_nSkipModel;
};


#endif // _SCREEN_TEAM_H_