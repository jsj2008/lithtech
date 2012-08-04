// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenOptions.cpp
//
// PURPOSE : Interface screen for navigation to various option setting screens
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
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
	int32 kTextWidth = 200;


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
bool CScreenOptions::Build()
{

	CreateTitle("IDS_TITLE_OPTIONS");

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	cs.nCommandID = CMD_DISPLAY;
	cs.szHelpID = "IDS_HELP_DISPLAY";
	AddTextItem("IDS_DISPLAY", cs );

	cs.nCommandID = CMD_AUDIO;
	cs.szHelpID = "IDS_HELP_SOUND";
	AddTextItem("IDS_SOUND", cs );

	cs.nCommandID = CMD_PERFORMANCE;
	cs.szHelpID = "IDS_HELP_PERFORMANCE";
	AddTextItem("IDS_PERFORMANCE", cs);

	cs.nCommandID = CMD_CONTROLS;
	cs.szHelpID = "IDS_HELP_CONTROLS";
	AddTextItem("IDS_CONTROLS", cs);

	cs.nCommandID = CMD_GAME;
	cs.szHelpID = "IDS_HELP_GAME_OPTIONS";
	AddTextItem("IDS_GAME_OPTIONS", cs );

	cs.nCommandID = CMD_WEAPONS;
	cs.szHelpID = "IDS_HELP_WEAPONS";
	AddTextItem( "IDS_WEAPONS", cs );


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(true,true);
	return true;
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
	case CMD_WEAPONS:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_WEAPONS);
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
void    CScreenOptions::OnFocus(bool bFocus)
{
	if (bFocus)
	{
        UpdateData(false);
	}
	else
	{
		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
}

