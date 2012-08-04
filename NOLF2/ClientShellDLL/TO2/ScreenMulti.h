// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMulti.h
//
// PURPOSE : Interface screen for hosting and joining multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_MULTI_H_
#define _SCREEN_MULTI_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenMulti : public CBaseScreen
{
public:
	CScreenMulti();
	virtual ~CScreenMulti();

	// Build the screen
    LTBOOL   Build();

	LTBOOL	Render(HSURFACE hDestSurf);

    void    OnFocus(LTBOOL bFocus);

protected:
    uint32	OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	uint32	HandleCallback(uint32 dwParam1, uint32 dwParam2);
	void	ChangeCDKey();
	void	RequestMOTD();
	void	RequestValidate();
	LTBOOL	LaunchSierraUp();
	void	CreateDMMissionFile();
//	void	CreateTDMMissionFile();
	void	CreateDDMissionFile();

	void	Update();

	CLTGUIColumnCtrl* m_pCDKeyCtrl;

	std::string m_sCurCDKey;
	std::string m_sLastValidCDKey;
	std::string	m_sGameMOTD;
	std::string	m_sSysMOTD;

	enum EState {
		eState_Startup,
		eState_VersionCheck,
		eState_NoCDKey,
		eState_ValidateCDKey,
		eState_MOTD,
		eState_Ready,
	};
	EState m_eCurState;

	CLTGUIWindow*	m_pWait;
	CLTGUICtrl*		m_pJoin;
	CLTGUICtrl*		m_pHost;

	CLTGUITextCtrl*		m_pStatusCtrl;
	CLTGUITextCtrl*		m_pUpdate;
	CLTGUILargeText*	m_pSysMOTD;
	CLTGUIFrame*		m_pSysFrame;
	CLTGUILargeText*	m_pGameMOTD;
	CLTGUIFrame*		m_pGameFrame;
	CLTGUITextCtrl*		m_pWaitText;
	

};

#endif // _SCREEN_MULTI_H_