// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMgr.cpp
//
// PURPOSE : Interface screen manager
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "TO2ScreenMgr.h"

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
#include "ScreenPlayer.h"
#include "ScreenPlayerSkills.h"
#include "ScreenHost.h"
#include "ScreenJoin.h"
#include "ScreenJoinLAN.h"
#include "ScreenPlayerTeam.h"

//under options
#include "ScreenDisplay.h"
#include "ScreenAudio.h"
#include "ScreenControls.h"
#include "ScreenGame.h"
#include "ScreenPerformance.h"

//under multi/host
#include "ScreenHostOptions.h"
#include "ScreenHostDMOptions.h"
#include "ScreenHostTDMOptions.h"
#include "ScreenHostDDOptions.h"
#include "ScreenHostMission.h"
#include "ScreenHostLevels.h"
#include "ScreenHostWeapons.h"
#include "ScreenTeam.h"

//under options/game
#include "ScreenCrosshair.h"

//under options/controls
#include "ScreenMouse.h"
#include "ScreenKeyboard.h"
#include "ScreenJoystick.h"
#include "ScreenConfigure.h"


#include "ScreenFailure.h"
#include "ScreenEndMission.h"
#include "ScreenEndCoopMission.h"
#include "ScreenEndDMMission.h"
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

CTO2ScreenMgr::CTO2ScreenMgr()
{
	m_eCurrentScreenID = SCREEN_ID_NONE;
	m_eLastScreenID = SCREEN_ID_NONE;
}

CTO2ScreenMgr::~CTO2ScreenMgr()
{

}


//////////////////////////////////////////////////////////////////////
// Function name	: CTO2ScreenMgr::Init
// Description	    :
// Return type      : LTBOOL
//////////////////////////////////////////////////////////////////////

LTBOOL CTO2ScreenMgr::Init()
{
	//build screen array
	m_screenArray.reserve(SCREEN_ID_UNASSIGNED);

	for (int nID = SCREEN_ID_MAIN; nID < SCREEN_ID_UNASSIGNED; ++nID)
	{
		AddScreen((eScreenID)nID);
	}

    return CScreenMgr::Init();
}


void CTO2ScreenMgr::SwitchToScreen(CBaseScreen *pNewScreen)
{

	CScreenMgr::SwitchToScreen(pNewScreen);

	// Do any special case work for each screen
	if (m_eCurrentScreenID == SCREEN_ID_MAIN)
	{
		m_nHistoryLen = 0;
		m_eScreenHistory[0] = SCREEN_ID_NONE;
	}

}




void CTO2ScreenMgr::AddScreen(eScreenID screenID)
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
	case SCREEN_ID_HOST_OPTIONS:
		pScreen = debug_new(CScreenHostOptions);
		break;
	case SCREEN_ID_HOST_DM_OPTIONS:
		pScreen = debug_new(CScreenHostDMOptions);
		break;
	case SCREEN_ID_HOST_TDM_OPTIONS:
		pScreen = debug_new(CScreenHostTDMOptions);
		break;
	case SCREEN_ID_HOST_DD_OPTIONS:
		pScreen = debug_new(CScreenHostDDOptions);
		break;
	case SCREEN_ID_HOST_LEVELS:
		pScreen = debug_new(CScreenHostLevels);
		break;
	case SCREEN_ID_HOST_WEAPONS:
		pScreen = debug_new(CScreenHostWeapons);
		break;
	case SCREEN_ID_HOST_MISSION:
		pScreen = debug_new(CScreenHostMission);
		break;
	case SCREEN_ID_TEAM:
		pScreen = debug_new(CScreenTeam);
		break;
	case SCREEN_ID_JOIN:
		pScreen = debug_new(CScreenJoin);
		break;
	case SCREEN_ID_JOIN_LAN:
		pScreen = debug_new(CScreenJoinLAN);
		break;
	case SCREEN_ID_PLAYER:
		pScreen = debug_new(CScreenPlayer);
		break;
	case SCREEN_ID_PLAYER_SKILLS:
		pScreen = debug_new(CScreenPlayerSkills);
		break;
	case SCREEN_ID_PLAYER_TEAM:
		pScreen = debug_new(CScreenPlayerTeam);
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
	case SCREEN_ID_JOYSTICK:
		pScreen = debug_new(CScreenJoystick);
		break;
	case SCREEN_ID_CONFIGURE:
		pScreen = debug_new(CScreenConfigure);
		break;
	case SCREEN_ID_FAILURE:
		pScreen = debug_new(CScreenFailure);
		break;
	case SCREEN_ID_END_MISSION:
		pScreen = debug_new(CScreenEndMission);
		break;
	case SCREEN_ID_END_COOP_MISSION:
		pScreen = debug_new(CScreenEndCoopMission);
		break;
	case SCREEN_ID_END_DM_MISSION:
		pScreen = debug_new(CScreenEndDMMission);
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


const char* CTO2ScreenMgr::GetScreenName(eScreenID id)
{
	return s_aScreenName[id];
}

// Returns a screen ID (uint16) based on a name
uint16 CTO2ScreenMgr::GetScreenIDFromName(char * pName)
{
	for (uint16 i=0; i < SCREEN_ID_UNASSIGNED; ++i)
	{
		if (!strcmpi(pName, s_aScreenName[i]))
			return (i);
	}
	return 999;
}
