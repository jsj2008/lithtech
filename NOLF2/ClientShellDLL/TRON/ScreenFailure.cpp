// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenFailure.cpp
//
// PURPOSE : Interface screen for setting crosshair options
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenFailure.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "TRONInterfaceMgr.h"
#include "SaveLoadMgr.h"
#include "GameClientShell.h"
#include "ClientSaveLoadMgr.h"
#include "MissionMgr.h"


namespace
{
	float g_fDuration = 0.0f;
	float g_fMinDelay = 3.0f;
	float g_fDelay = 10.0f;
	LTRect	stringRect(20,240,620,400);
	uint8	stringSize = 24;
	LTIntPt helpPos(160,440);
	uint8	helpSize = 12;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenFailure::CScreenFailure()
{
	m_pString = LTNULL;
}

CScreenFailure::~CScreenFailure()
{

}


// Build the screen
LTBOOL CScreenFailure::Build()
{
	CreateTitle(NULL);

	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"FailStringRect"))
	{
		stringRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_FAILURE,"FailStringRect");
	}
	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"FailStringSize"))
	{
		stringSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_FAILURE,"FailStringSize");
	}

	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"HelpStringPos"))
	{
		helpPos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_FAILURE,"HelpStringPos");
	}
	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"HelpStringSize"))
	{
		helpSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_FAILURE,"HelpStringSize");
	}

	LTIntPt stringPos;
	stringPos.x = (stringRect.left + stringRect.right) / 2;
	stringPos.y = stringRect.top;
	m_pString = AddTextItem("failed",NULL,NULL,stringPos,LTTRUE);
	if (m_pString)
	{
		m_pString->SetFont(NULL,stringSize);
		m_pString->SetFixedWidth(stringRect.right - stringRect.left);
		CUIFormattedPolyString *pStr = m_pString->GetString();
		if (pStr)
		{
			pStr->SetAlignmentH(CUI_HALIGN_CENTER);
		}
	}

	if (g_pLayoutMgr->HasCustomValue(SCREEN_ID_FAILURE,"Delay"))
	{
		g_fDelay = g_pLayoutMgr->GetScreenCustomFloat(SCREEN_ID_FAILURE,"Delay");
	}

	CLTGUITextCtrl *pString = AddTextItem(IDS_HELP_FAILURE,NULL,NULL,helpPos,LTTRUE);
	if (pString)
	{
		pString->SetFont(NULL,helpSize);
	}


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);
	return LTTRUE;
}


void CScreenFailure::OnFocus(LTBOOL bFocus)
{

	if (bFocus)
	{
		g_fDuration = 0.0f;
		if (m_pString)
		{
			CUIFormattedPolyString *pStr = m_pString->GetString();
			if (pStr)
				pStr->SetText(LoadTempString(g_pInterfaceMgr->GetFailStringID()));
		}
	}
	else
	{
		if (m_pString)
		{
			CUIFormattedPolyString *pStr = m_pString->GetString();
			if (pStr)
				pStr->SetText("");
		}
	}
	CBaseScreen::OnFocus(bFocus);
}


void CScreenFailure::Escape()
{

	OnFocus(LTFALSE);

	//hack to rebuild our screen history
	m_pScreenMgr->ClearHistory();
	m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
	m_pScreenMgr->AddScreenToHistory(SCREEN_ID_SINGLE);

	g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
	m_pScreenMgr->SetCurrentScreen(SCREEN_ID_LOAD);
}

LTBOOL CScreenFailure::HandleKeyDown(int key, int rep)
{
	if (key == VK_F9)
	{
		g_pMissionMgr->StartGameFromQuickSave();
		return LTTRUE;
	}
	else if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;

}
LTBOOL CScreenFailure::OnLButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}
LTBOOL CScreenFailure::OnRButtonDown(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}


LTBOOL CScreenFailure::Render(HSURFACE hDestSurf)
{
	CBaseScreen::Render(hDestSurf);
	g_fDuration += g_pGameClientShell->GetFrameTime();
	if (g_fDuration >= g_fDelay)
		Escape();
	return LTTRUE;

}




