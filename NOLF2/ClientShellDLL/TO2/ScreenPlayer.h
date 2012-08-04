// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayer.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_PLAYER_H_
#define _SCREEN_PLAYER_H_

#include "BaseScreen.h"
#include "LayoutMgr.h"

class CScreenPlayer : public CBaseScreen
{
public:
	CScreenPlayer();
	virtual ~CScreenPlayer();


	// Build the screen
    LTBOOL   Build();
	void	Term();

    void    OnFocus(LTBOOL bFocus);

	virtual LTBOOL OnLeft();
	virtual LTBOOL OnRight();
	virtual LTBOOL OnUp();
	virtual LTBOOL OnDown();
    virtual LTBOOL OnMouseMove(int x, int y);
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);


	void	NextModel();
	void	PrevModel();

protected:
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

	void	UpdateBandwidth();

	void	UpdateChar();

    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	CLTGUIColumnCtrl*	m_pName;
	CLTGUITextCtrl*		m_pModel;
	CLTGUITextCtrl*		m_pSkills;
	CLTGUIButton*		m_pLeft;
	CLTGUIButton*		m_pRight;

	int			m_nCurrentModel;
	std::string m_sPlayerName;

	CLTGUICycleCtrl*	m_pBandwidthCycle;
	CLTGUIColumnCtrl*	m_pBandwidth;

	uint8				m_nBandwidth;
	CString				m_sBandwidth;

};


#endif // _SCREEN_PLAYER_H_