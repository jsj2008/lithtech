// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHost.h
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_HOST_H_
#define _SCREEN_HOST_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "NetDefs.h"

class CScreenHost : public CBaseScreen
{
public:
	CScreenHost();
	virtual ~CScreenHost();

	// Build the screen
    LTBOOL   Build();
    LTBOOL   OnLeft();
    LTBOOL   OnRight();
    LTBOOL   OnLButtonUp(int x, int y);
    LTBOOL   OnRButtonUp(int x, int y);

    void    OnFocus(LTBOOL bFocus);
    void	Escape();

	virtual bool UpdateInterfaceSFX();
	void	ReadyLaunch(LTBOOL bReady);

protected:
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);
	void	CreateDefaultCampaign();

    LTBOOL	LaunchGame();
    LTBOOL	SetService( );
	void	RunServerOptions();
	void	HandleLaunch();

	void	UpdateBandwidth();
	void	UpdateGameType();
	void	SaveOptions();


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	CString	m_sSessionName;
	CString	m_sCampaignName;
	CString	m_sPassword;
	CString	m_sPort;
	LTBOOL	m_bUsePassword;
	LTBOOL	m_bDedicated;
	CString	m_sScmdPassword;
	LTBOOL	m_bAllowScmdCommands;

	CLTGUIColumnCtrl*	m_pName;

	CLTGUITextCtrl*		m_pLoadCtrl;

	CLTGUIColumnCtrl*	m_pCampaign;

	CLTGUIColumnCtrl*	m_pPassword;
	CLTGUIToggle*		m_pPassToggle;

	CLTGUIColumnCtrl*	m_pScmdPassword;
	CLTGUIToggle*		m_pScmdPassToggle;

	CLTGUIColumnCtrl*	m_pPort;

	CLTGUICycleCtrl*	m_pBandwidthCycle;
	CLTGUIColumnCtrl*	m_pBandwidth;

	CLTGUITextCtrl*		m_pWeapons;

	GameType			m_eGameType;

	uint8				m_nBandwidth;
	CString				m_sBandwidth;

	LTBOOL				m_bReadyToLaunch;
};



#endif // _SCREEN_HOST_H_