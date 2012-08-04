// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPreload.h
//
// PURPOSE : Interface screen to be displayed  before loading a level
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenPreload.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientConnectionMgr.h"
#include "MissionMgr.h"


namespace
{
}

/*
static void FailCDKey( )
{
	MBCreate mb;
	g_pInterfaceMgr->ShowMessageBox(IDS_CDKEY_INVALID,&mb);
	g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_MAIN);
}
*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPreload::CScreenPreload()
{
//	m_eValidatingCDKeyState = kValidatingCDKeyState_None;
	m_bFirstUpdate = false;
}

CScreenPreload::~CScreenPreload()
{

}


// Build the screen
bool CScreenPreload::Build()
{

	CLTGUICtrl_create cs;
	AddTextItem(L"Pre-Load",cs,true);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;

	UseBack(false);
	return true;
}


void CScreenPreload::OnFocus(bool bFocus)
{
	CBaseScreen::OnFocus(bFocus);
	m_bVisited = false;
}


void CScreenPreload::Escape()
{
	g_pInterfaceMgr->LoadFailed(SCREEN_ID_MAIN, LoadString( "Screen_LoadAborted" ));
}

bool CScreenPreload::UpdateInterfaceSFX()
{
	if( m_bFirstUpdate )
	{
		m_bFirstUpdate = false;
		FirstUpdate( );
	}


	if (!CBaseScreen::UpdateInterfaceSFX())
		return false;

	bool bSuccess = g_pMissionMgr->FinishExitLevel();

	bSuccess = bSuccess && g_pMissionMgr->FinishStartGame();

	if (!bSuccess)
	{
		if (IsMultiplayerGameClient())
			g_pInterfaceMgr->ConnectionFailed(g_pClientConnectionMgr->GetLastConnectionResult());
		else
			g_pInterfaceMgr->LoadFailed();
	}

	return true;
}

void CScreenPreload::FirstUpdate( )
{
	//set up chain FX here
}

void CScreenPreload::CreateInterfaceSFX()
{
	// Flag that this is will be the first update.
	m_bFirstUpdate = true;
}