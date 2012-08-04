// ----------------------------------------------------------------------- //
//
// MODULE  : TronScreenMgr.cpp
//
// PURPOSE : Interface screen manager
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "TronScreenMgr.h"

//screens
#include "BaseScreen.h"
#include "ScreenMain.h"

//under main
#include "ScreenSingle.h"
#include "ScreenMulti.h"
#include "ScreenOptions.h"
#include "ScreenProfile.h"

//under single
#include "ScreenLoad.h"
#include "ScreenSave.h"

//under multi
#include "ScreenHost.h"
#include "ScreenJoin.h"
#include "ScreenJoinLAN.h"

//under options
#include "ScreenDisplay.h"
#include "ScreenAudio.h"
#include "ScreenControls.h"
#include "ScreenGame.h"
#include "ScreenPerformance.h"

//under multi/host
#include "ScreenHostLevels.h"

//under options/game
#include "ScreenCrosshair.h"

//under options/controls
#include "ScreenMouse.h"
#include "ScreenKeyboard.h"
#include "ScreenConfigure.h"

//Player management screens
#include "ScreenSubroutines.h"
#include "ScreenRatings.h"

#include "ScreenFailure.h"
#include "ScreenEndMission.h"
#include "ScreenPreload.h"
#include "ScreenPostload.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


static char s_aScreenName[SCREEN_ID_UNASSIGNED+1][32] =
{
#define INCLUDE_AS_STRING
#include "ScreenEnum.h"
#undef INCLUDE_AS_STRING

};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTronScreenMgr::CTronScreenMgr()
{
	m_eCurrentScreenID = SCREEN_ID_NONE;
	m_eLastScreenID = SCREEN_ID_NONE;

	// This is a temporary wav file sound until we
	// get a real direct music solution in... 
	// Which is already hooked up in interfacemgr
	// (SetMenuMusic())
	m_hMenuSnd = LTNULL;
}

CTronScreenMgr::~CTronScreenMgr()
{
	KillMenuMusic();
}


//////////////////////////////////////////////////////////////////////
// Function name	: CTronScreenMgr::Init
// Description	    :
// Return type      : LTBOOL
//////////////////////////////////////////////////////////////////////

LTBOOL CTronScreenMgr::Init()
{
	//build screen array
	m_screenArray.reserve(SCREEN_ID_UNASSIGNED);

	for (int nID = SCREEN_ID_MAIN; nID < SCREEN_ID_UNASSIGNED; ++nID)
	{
		AddScreen((eScreenID)nID);
	}

    return CScreenMgr::Init();
}


void CTronScreenMgr::SwitchToScreen(CBaseScreen *pNewScreen)
{

	CScreenMgr::SwitchToScreen(pNewScreen);

	// Do any special case work for each screen
	if (m_eCurrentScreenID == SCREEN_ID_MAIN)
	{
		m_nHistoryLen = 0;
		m_eScreenHistory[0] = SCREEN_ID_NONE;

		// add a custom cursor
		g_pCursorMgr->UseSprite("Interface\\cursor.dtx");
		g_pCursorMgr->UseGlowSprite("Interface\\cursor_glow.dtx");
		g_pCursorMgr->SetCenter(32,32);
	}

}




void CTronScreenMgr::AddScreen(eScreenID screenID)
{
    CBaseScreen* pScreen = LTNULL;
	switch (screenID)
	{
	case SCREEN_ID_MAIN:
		pScreen = debug_new(CScreenMain);
		break;
	case SCREEN_ID_SINGLE:
		pScreen = debug_new(CScreenSingle);
		break;
	case SCREEN_ID_MULTI:
		pScreen = debug_new(CScreenMulti);
		break;
	case SCREEN_ID_OPTIONS:
		pScreen = debug_new(CScreenOptions);
		break;
	case SCREEN_ID_PROFILE:
		pScreen = debug_new(CScreenProfile);
		break;
	case SCREEN_ID_LOAD:
		pScreen = debug_new(CScreenLoad);
		break;
	case SCREEN_ID_SAVE:
		pScreen = debug_new(CScreenSave);
		break;
	case SCREEN_ID_HOST:
		pScreen = debug_new(CScreenHost);
		break;
	case SCREEN_ID_HOST_LEVELS:
		pScreen = debug_new(CScreenHostLevels);
		break;
	case SCREEN_ID_JOIN:
		pScreen = debug_new(CScreenJoin);
		break;
	case SCREEN_ID_JOIN_LAN:
		pScreen = debug_new(CScreenJoinLAN);
		break;
	case SCREEN_ID_AUDIO:
		pScreen = debug_new(CScreenAudio);
		break;
	case SCREEN_ID_DISPLAY	:
		pScreen = debug_new(CScreenDisplay);
		break;
	case SCREEN_ID_GAME	:
		pScreen = debug_new(CScreenGame);
		break;
	case SCREEN_ID_PERFORMANCE	:
		pScreen = debug_new(CScreenPerformance);
		break;
	case SCREEN_ID_CROSSHAIR:
		pScreen = debug_new(CScreenCrosshair);
		break;
	case SCREEN_ID_CONTROLS:
		pScreen = debug_new(CScreenControls);
		break;
	case SCREEN_ID_MOUSE:
		pScreen = debug_new(CScreenMouse);
		break;
	case SCREEN_ID_KEYBOARD:
		pScreen = debug_new(CScreenKeyboard);
		break;
	case SCREEN_ID_CONFIGURE:
		pScreen = debug_new(CScreenConfigure);
		break;
	case SCREEN_ID_SUBROUTINES:
		pScreen = debug_new(CScreenSubroutines);
		break;
//	case SCREEN_ID_RATINGS:
//		pScreen = debug_new(CScreenRatings);
//		break;
	case SCREEN_ID_FAILURE:
		pScreen = debug_new(CScreenFailure);
		break;
	case SCREEN_ID_END_MISSION:
		pScreen = debug_new(CScreenEndMission);
		break;
	case SCREEN_ID_PRELOAD:
		pScreen = debug_new(CScreenPreload);
		break;
	case SCREEN_ID_POSTLOAD:
		pScreen = debug_new(CScreenPostload);
		break;
	}

	if (pScreen)
	{
		pScreen->Init(screenID);
		CScreenMgr::AddScreen(pScreen);
	}

}


const char* CTronScreenMgr::GetScreenName(eScreenID id)
{
	return s_aScreenName[id];
}

// Returns a screen ID (uint16) based on a name
uint16 CTronScreenMgr::GetScreenIDFromName(char * pName)
{
	for (uint16 i=0; i < SCREEN_ID_UNASSIGNED; ++i)
	{
		if (!strcmpi(pName, s_aScreenName[i]))
			return (i);
	}
	return 999;
}

void CTronScreenMgr::ExitScreens()
{
	KillMenuMusic();
	CScreenMgr::ExitScreens();
}

void CTronScreenMgr::PlayMenuMusic()
{
	if(!m_hMenuSnd)
	{
		m_hMenuSnd = g_pClientSoundMgr->PlaySoundLocal("Music/TronTheme.wav", SOUNDPRIORITY_PLAYER_HIGH, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE);
	}
}

void CTronScreenMgr::KillMenuMusic()
{
	if(m_hMenuSnd)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hMenuSnd);
		m_hMenuSnd = NULL;
	}
}
