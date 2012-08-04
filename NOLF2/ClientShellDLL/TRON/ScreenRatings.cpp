// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenRatings.cpp
//
// PURPOSE : Screen for managing player's performance Ratings
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenRatings.h"
#include "ScreenSubroutines.h"
#include "ScreenMgr.h"
#include "LayoutMgr.h"
#include "ScreenCommands.h"
//#include "WinUtil.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"
#include "RatingMgr.h"
#include "TRONPlayerStats.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
	const int CMD_COMPILE = CMD_CUSTOM + 1;
	const int CMD_SUBROUTINES = CMD_CUSTOM + 2;

	const float MEMORY_INNER_RADIUS = 120;
	const float MEMORY_OUTER_RADIUS = 170;

	const float LIBRARY_INNER_RADIUS = 186;
	const float LIBRARY_OUTER_RADIUS = 230;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenRatings::CScreenRatings()
{
}

CScreenRatings::~CScreenRatings()
{
}


// Build the screen
LTBOOL CScreenRatings::Build()
{
	CreateTitle(IDS_TITLE_RATINGS);

	// Add a "Subroutines" button
	AddTextItem(IDS_SUBROUTINES, CMD_SUBROUTINES, IDS_HELP_SUBROUTINES);

	// Add a "Compile" button
	AddTextItem(IDS_COMPILE, CMD_COMPILE, IDS_HELP_COMPILE);

	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE);

	LTIntPt center;
	center.x = 320;
	center.y = 240;

	m_SystemMemoryCtrl.Init(center, MEMORY_INNER_RADIUS, MEMORY_OUTER_RADIUS, 24);

	m_AdditiveCtrl.Init(center, LIBRARY_INNER_RADIUS, LIBRARY_OUTER_RADIUS, 6);
	m_AdditiveCtrl.SetArc(45.0f, 135.0f);
	m_AdditiveCtrl.SetNumHighlightSegments(1);

	m_nAdditives = 0;
	m_nAdditiveLibSize = 0;
	// TODO mark all slots as "unavailable"
	// TODO set the cursor mode
	return LTTRUE;

}

void CScreenRatings::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// TODO activate all the additive sprites
		m_pHotAdd = LTNULL;
		m_pCursorAdd = LTNULL;
		m_eCursorMode = CursorPickup;
	}
	else
	{
		// Hide all sprites
	}
	CBaseScreen::OnFocus(bFocus);
}

void CScreenRatings::ClearScreen()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenRatings::OnCommand
//
//	PURPOSE:	Handle incoming commands from controls
//
// ----------------------------------------------------------------------- //

uint32 CScreenRatings::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{

	case CMD_SUBROUTINES:
		{
			//you can enable the following line when screenmgr is smart enough to not
			// create a history for this switch
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_SUBROUTINES);
		}
		break;

	case CMD_COMPILE:
		{
			Compile();
			{
				// Compile the elements on the other screen, too.
				CScreenSubroutines *pSubScreen = (CScreenSubroutines *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_SUBROUTINES);
				pSubScreen->Compile();
			}
			g_pSubroutineMgr->Compile();
			Escape();
			break;
		}

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::OnMouseMove
//
//	PURPOSE:	Pass on any mouse updates to the relevant controls
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenRatings::OnMouseMove(int x, int y)
{
	ADDITIVE * pOldAdd = m_pHotAdd;
	m_pHotAdd = LTNULL;

	// TODO eventually we need to keep track of which arcs are visible at any time
	m_SystemMemoryCtrl.OnMouseMove(x, y);
	m_AdditiveCtrl.OnMouseMove(x, y);

	m_pHotAdd = (ADDITIVE *)m_SystemMemoryCtrl.GetHotObject();
	if (!m_pHotAdd)
		m_pHotAdd = (ADDITIVE *)m_AdditiveCtrl.GetHotObject();

	if (m_pHotAdd != pOldAdd)
	{
		// The heat has changed.
		if (m_pHotAdd)
		{
			// rebuild the "hot" window
		}
	}
	return CBaseScreen::OnMouseMove(x, y);
}

// Screen specific rendering
LTBOOL   CScreenRatings::Render(HSURFACE hDestSurf)
{
	LTBOOL lResult = CBaseScreen::Render(hDestSurf);

	m_SystemMemoryCtrl.Render();
	
	// FIXME eventually hide these
	m_AdditiveCtrl.Render();

    return lResult;

}


void CScreenRatings::Escape()
{
	//quit or return to game?
    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (g_pGameClientShell->IsWorldLoaded() && hPlayerObj && 
		(!g_pPlayerMgr->IsPlayerDead() || IsMultiplayerGame( )))
	{
		g_pInterfaceMgr->ChangeState(GS_PLAYING);
	}
	else
	{
		CBaseScreen::Escape();
	}
}

void CScreenRatings::Compile()
{
}

void CScreenRatings::SetCursorMode(SubCursorMode eMode)
{
	switch (m_eCursorMode)
	{
	case CursorPickup:
		break;

	case CursorAdditiveDrop:
		break;
	}

	m_eCursorMode = eMode;

	switch (m_eCursorMode)
	{
	case CursorPickup:
		break;

	case CursorAdditiveDrop:
		break;
	}
}

void CScreenRatings::AddSubroutine(PlayerSubroutine * pSubroutine)
{
}