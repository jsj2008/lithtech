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
#include "TO2InterfaceMgr.h"
#include "SaveLoadMgr.h"
#include "GameClientShell.h"
#include "ClientSaveLoadMgr.h"
#include "ClientMultiplayerMgr.h"
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
	m_pHelp = NULL;
	m_bSentAutoLoadMessage = false;
	g_fDuration = 0.0f;
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

	if( !g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
	{
		m_pHelp = AddTextItem(IDS_HELP_FAILURE,NULL,NULL,helpPos,LTTRUE);
		if (m_pHelp)
		{
			m_pHelp->SetFont(NULL,helpSize);
		}
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
			m_pString->SetString(LoadTempString(g_pInterfaceMgr->GetFailStringID()));
		}
		if (m_pHelp)
		{
			m_pHelp->Show(LTFALSE);
		}

		m_bSentAutoLoadMessage = false;
	}
	else
	{
		if (m_pString)
		{
			m_pString->SetString(" ");
		}

		//for co-op hide the MB if they are forced to leave the screen before choosing
		g_pInterfaceMgr->CloseMessageBox(LTFALSE);
	}
	CBaseScreen::OnFocus(bFocus);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ExitFailureScreenCB
//
//  PURPOSE:	Callback for the Exit failure mission screen messagebox...
//
// ----------------------------------------------------------------------- //

static void ExitFailureScreenCB(LTBOOL bReturn, void *pData)
{
	// If they confirm, then go back.
	if (bReturn)
	{
		CScreenMgr* pScreenMgr = g_pInterfaceMgr->GetScreenMgr( );

		//hack to rebuild our screen history
		pScreenMgr->ClearHistory();
		pScreenMgr->AddScreenToHistory( SCREEN_ID_MAIN );
		pScreenMgr->AddScreenToHistory( SCREEN_ID_MULTI );

		g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
		g_pInterfaceMgr->GetScreenMgr( )->PreviousScreen( );
	}
}



void CScreenFailure::Escape()
{
	// Only go back to the load screen if local client or single player.
	if( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
	{
		MBCreate mb;
		mb.eType = LTMB_YESNO;
		mb.pFn = ExitFailureScreenCB;
		mb.pData = this;
		g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
	}
	else
	{
		//hack to rebuild our screen history
		m_pScreenMgr->ClearHistory();

		g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);

		m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);

		// Put mp history if mp game.
		if( IsMultiplayerGame( ))
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MULTI);
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_HOST);
		}
		// Put sp history if sp game.
		else
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_SINGLE);
		}

		OnFocus(LTFALSE);

		m_pScreenMgr->SetCurrentScreen(SCREEN_ID_LOAD);
	}
}

LTBOOL CScreenFailure::HandleKeyUp(int key, int rep)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;

}
LTBOOL CScreenFailure::OnLButtonUp(int x, int y)
{
	if (g_fDuration > g_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}
LTBOOL CScreenFailure::OnRButtonUp(int x, int y)
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
	
	// Let the serve know we are ready to begin an automatic load...

	if( g_fDuration > g_fMinDelay && !m_bSentAutoLoadMessage )
	{
		SendEmptyServerMsg( MID_CLIENT_READY_FOR_AUTOLOAD );
		m_bSentAutoLoadMessage = true;
	}

	// Automatically trigger an escape if we're a local client.
	if( !g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
	{

		if (g_fDuration > g_fMinDelay && m_pHelp && !m_pHelp->IsVisible())
		{
			g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\pressanykey.wav");
			m_pHelp->Show(LTTRUE);
		}

		if (g_fDuration >= g_fDelay)
			Escape();
	}
	return LTTRUE;

}




