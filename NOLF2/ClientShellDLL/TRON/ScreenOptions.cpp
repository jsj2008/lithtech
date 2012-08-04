// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenOptions.cpp
//
// PURPOSE : Interface screen for navigation to various option setting screens
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenOptions.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


namespace
{

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenOptions::CScreenOptions()
{
}

CScreenOptions::~CScreenOptions()
{

}

// Build the screen
LTBOOL CScreenOptions::Build()
{

	CreateTitle(IDS_TITLE_OPTIONS);

	AddTextItem(IDS_DISPLAY, CMD_DISPLAY, IDS_HELP_DISPLAY);
	AddTextItem(IDS_SOUND, CMD_AUDIO, IDS_HELP_SOUND);
	AddTextItem(IDS_CONTROLS, CMD_CONTROLS, IDS_HELP_CONTROLS);
	AddTextItem(IDS_GAME_OPTIONS, CMD_GAME, IDS_HELP_GAME_OPTIONS);
	AddTextItem(IDS_PERFORMANCE, CMD_PERFORMANCE, IDS_HELP_PERFORMANCE);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CScreenOptions::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_DISPLAY:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_DISPLAY);
			break;
		}
	case CMD_AUDIO:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_AUDIO);
			break;
		}
	case CMD_GAME:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_GAME);
			break;
		}
	case CMD_PERFORMANCE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PERFORMANCE);
			break;
		}
	case CMD_CONTROLS:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_CONTROLS);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenOptions::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
}

