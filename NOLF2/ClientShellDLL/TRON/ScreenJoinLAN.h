// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoinLAN.h
//
// PURPOSE : Interface screen to search for and join LAN games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#ifndef _SCREEN_JOIN_LAN_H_
#define _SCREEN_JOIN_LAN_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"

class CScreenJoinLAN : public CBaseScreen
{
public:
	CScreenJoinLAN();
	virtual ~CScreenJoinLAN();

	// Build the screen
    LTBOOL   Build();

	LTBOOL	 Render(HSURFACE hDestSurf);

    void    OnFocus(LTBOOL bFocus);
	LTBOOL	HandleKeyDown(int key, int rep);

	void	JoinCurGame();

protected:
    uint32  OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);

	LTBOOL	InitSessions();
	void	FindServers();
	LTBOOL	SetService();
	LTBOOL	IsCurrentGame(CString sAddress);
	LTBOOL	DoJoinGame(CString sAddress);

	LTBOOL	IsValidPort(uint16 nPort);

	int		m_nFrameDelay;

	char m_szPort[8];

	CLTGUITextCtrl*		m_pFind;
	CLTGUIColumnCtrl*	m_pPort;
	CLTGUIListCtrl*		m_pServers;

	CMoArray<CString>	m_lstSessions;

};

inline LTBOOL CScreenJoinLAN::IsValidPort(uint16 nPort)
{
	return (nPort > 0);
}


#endif // _SCREEN_MULTI_H_