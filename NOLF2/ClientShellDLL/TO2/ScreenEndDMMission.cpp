// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEndDMMission.cpp
//
// PURPOSE : Interface screen for handling end of mission 
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenEndDMMission.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ScreenPreload.h"
#include "TO2InterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "HUDMgr.h"



static float	s_fMinDelay = 3.0f;
static float	s_fDelay = 30.0f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenEndDMMission::CScreenEndDMMission()
:	CBaseScreen				( ),
	m_pContinue				( LTNULL ),
	m_fDuration				( 0.0f ),
	m_bFlash				( false ),
	m_fFlashTime			( 0.0f ),
	m_bForceShowContinue	( false )
{
}

CScreenEndDMMission::~CScreenEndDMMission()
{

}


// Build the screen
LTBOOL CScreenEndDMMission::Build()
{
	//CreateTitle(IDS_TITLE_ENDMISSION);

	LTIntPt pos(GetPageLeft(),GetPageBottom());
	m_pContinue = AddTextItem(LoadTempString(IDS_PRESS_ANY_KEY),NULL,NULL,pos,LTTRUE);


	// Make sure to call the base class
	if (! CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);
	return LTTRUE;
}


void CScreenEndDMMission::OnFocus(LTBOOL bFocus)
{
	
	if (bFocus)
	{
		g_pScores->Show(true,true);
		m_pContinue->Show(LTFALSE);

		// Reset our flash data...

		m_bFlash = false;
		m_fFlashTime = g_pLTClient->GetTime() + 0.333f;
		
		// Reset the ammount of time the screen has been active...
		
		m_fDuration = 0.0f;

		// If just going between rounds we want to keep showing this screen while loading...

		if( !g_pMissionMgr->IsExitingMission() && g_pMissionMgr->IsExitingLevel() )
		{
			g_pInterfaceMgr->SetLoadingRenderScreen( this );
		
			CBaseScreen::OnFocus(bFocus);
	
			// Skip the preload and set the postload to this screen...

			g_pInterfaceMgr->SkipPreLoad( true );
			g_pInterfaceMgr->SetPostLoadScreenID( SCREEN_ID_END_DM_MISSION );
			
			// Handle restarting the round...

			g_pMissionMgr->FinishExitLevel( );
			g_pMissionMgr->FinishStartGame( );

			// Be sure to show the contine screen as soon as we are ready...

			m_bForceShowContinue = true;

			return;
		}
	}
	else
	{
		g_pScores->Show(false);
	}

	CBaseScreen::OnFocus(bFocus);
}


void CScreenEndDMMission::Escape()
{

	OnFocus(LTFALSE);

	if (g_pMissionMgr->IsGameOver())
	{
		g_pMissionMgr->ClearGameOver();

#ifdef _TO2DEMO
		g_pInterfaceMgr->ShowDemoScreens(LTFALSE);
#else
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
#endif

	}
	else if( g_pInterfaceMgr->ShouldSkipPreLoad() )
	{
		// No need to skip the preload screen anymore...

		g_pInterfaceMgr->SkipPreLoad( false );

		// Set the post load back to it's default...

		g_pInterfaceMgr->SetPostLoadScreenID( );

		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if( g_pGameClientShell->IsWorldLoaded() && hPlayerObj )
		{
			// Let the client enter the world...

			g_pGameClientShell->SendClientLoadedMessage( );
			g_pInterfaceMgr->ChangeState( GS_PLAYING );
		}
	}
	else
	{
		CScreenPreload *pPreload = (CScreenPreload *) (g_pInterfaceMgr->GetScreenMgr( )->GetScreenFromID(SCREEN_ID_PRELOAD));
		if (pPreload)
		{
			pPreload->SetWaitingToExit(true);
		}
		g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_PRELOAD);
	}

}

LTBOOL CScreenEndDMMission::HandleKeyDown(int key, int rep)
{
	if( m_pContinue->IsVisible() )
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;

}
LTBOOL CScreenEndDMMission::OnLButtonDown(int x, int y)
{
	if (m_fDuration > s_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}
LTBOOL CScreenEndDMMission::OnRButtonDown(int x, int y)
{
	if (m_fDuration > s_fMinDelay)
	{
		Escape();
		return LTTRUE;
	}
	return LTFALSE;
}


LTBOOL CScreenEndDMMission::Render(HSURFACE hDestSurf)
{
	CBaseScreen::Render(hDestSurf);

	g_pScores->Render();

	float fTime = g_pLTClient->GetTime();
	if (fTime > m_fFlashTime)
	{
		m_bFlash = !m_bFlash;
		m_fFlashTime = fTime + 0.333f;
		if (m_bFlash)
		{
			m_pContinue->SetColors(m_SelectedColor,m_SelectedColor,m_SelectedColor);
		}
		else
		{
			m_pContinue->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		}

	}

	m_fDuration += g_pGameClientShell->GetFrameTime();

	if( (m_fDuration > s_fMinDelay && !m_pContinue->IsVisible()) || (m_bForceShowContinue && !g_pInterfaceMgr->IsLoadScreenVisible()) )
	{
		m_bForceShowContinue = false;

		m_pContinue->Show(LTTRUE);
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\pressanykey.wav");
	}


	// [KLS 9/13/02] Only auto switch in Multiplayer...
	if (m_fDuration >= s_fDelay && IsMultiplayerGame())
	{
		Escape();
	}

	return LTTRUE;

}




