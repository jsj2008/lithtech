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

	virtual bool UpdateInterfaceSFX();
	void	ReadyLaunch(LTBOOL bReady);

protected:
	void	HandleCallback(uint32 dwParam1, uint32 dwParam2);

    LTBOOL	LaunchGame();
    LTBOOL	SetService( );
	void	RunServerOptions();
	void	HandleLaunch();

	LTBOOL	IsValidPort(uint16 nPort);
	LTBOOL	IsValidBandwidth(uint32 nBandwidth);

	void	UpdateBandwidth();


protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
	CString	m_sSessionName;
	CString	m_sCampaignName;
	CString	m_sPassword;
	CString	m_sPort;
	LTBOOL	m_bUsePassword;

	CLTGUIColumnCtrl*	m_pName;

//	CLTGUITextCtrl*		m_pLoadCtrl;

	CLTGUIColumnCtrl*	m_pCampaign;

	CLTGUIColumnCtrl*	m_pPassword;
	CLTGUIToggle*		m_pPassToggle;

	CLTGUIColumnCtrl*	m_pPort;

	CLTGUICycleCtrl*	m_pBandwidthCycle;
	CLTGUIColumnCtrl*	m_pBandwidth;

	uint8				m_nBandwidth;
	CString				m_sBandwidth;

	LTBOOL				m_bReadyToLaunch;
};


inline LTBOOL CScreenHost::IsValidPort(uint16 nPort)
{
	//TODO: what is the valid range of ports?
	return (nPort > 0);
}

inline LTBOOL CScreenHost::IsValidBandwidth(uint32 nBandwidth) 
{
	return (nBandwidth >= 33600 && nBandwidth <= 10000000);
}

#endif // _SCREEN_HOST_H_