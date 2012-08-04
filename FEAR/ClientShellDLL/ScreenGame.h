// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenGame.h
//
// PURPOSE : Interface screen for setting gameplay options
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_GAME_H_
#define _SCREEN_GAME_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenGame : public CBaseScreen
{
public:
	CScreenGame();
	virtual ~CScreenGame();

	// Build the folder
    bool   Build();
    void    OnFocus(bool bFocus);


protected:


    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

protected:
    bool	            m_bSubtitles;
    bool				m_bGore;
	uint8				m_nDifficulty;
	bool				m_bAlwaysRun;
	bool				m_bCrouchToggle;
	uint8				m_nLayout;
	int					m_nHeadBob;
	int					m_nMsgDur;
	uint8				m_nAutoWeaponSwitch;	
	int					m_nHUDFade;
	bool				m_bSlowMoFX;

	bool				m_bMPAutoWeaponSwitch;
	bool				m_bSPAutoWeaponSwitch;

	CLTGUICycleCtrl		*m_pDifficultyCtrl;			// The difficulty control
	CLTGUITextCtrl		*m_pFade;

};

#endif // _SCREEN_GAME_H_