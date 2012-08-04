// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMulti.cpp
//
// PURPOSE : Interface screen for hosting and joining multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMulti.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "GameClientShell.h"
#include "ClientSaveLoadMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMulti::CScreenMulti()
{
}

CScreenMulti::~CScreenMulti()
{

}

// Build the screen
LTBOOL CScreenMulti::Build()
{

	CreateTitle(IDS_TITLE_MULTI);

	AddTextItem(IDS_PLAYER_SETUP, CMD_PLAYER, IDS_HELP_PLAYER);
	AddTextItem(IDS_JOIN, CMD_JOIN, IDS_HELP_JOIN);
	AddTextItem(IDS_JOIN_LAN, CMD_JOIN_LAN, IDS_HELP_JOIN_LAN);
	AddTextItem(IDS_HOST, CMD_HOST, IDS_HELP_HOST);

 	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenMulti::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_PLAYER:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PLAYER);
			break;
		}
	case CMD_JOIN:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOIN);
			break;
		}
	case CMD_JOIN_LAN:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_JOIN_LAN);
			break;
		}
	case CMD_HOST:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_HOST);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenMulti::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// Switch save/load to use mp folders.  If we go back to the
		// mainscreen, it will switch back to sp folders.
		g_pClientSaveLoadMgr->SetUseMultiplayerFolders( true );

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
}

