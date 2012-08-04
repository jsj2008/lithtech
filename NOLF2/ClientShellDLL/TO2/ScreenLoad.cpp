// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenLoad.cpp
//
// PURPOSE : Interface screen for loading saved games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "MissionButeMgr.h"
#include "ScreenLoad.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientSaveLoadMgr.h"
#include "MissionMgr.h"
#include "ClientMultiplayerMgr.h"


#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

namespace
{
	const int kMaxSave = SLMGR_MAX_SAVE_SLOTS;
	int kNameWidth = 0;
	int kTimeWidth = 0;
	int kIndent = 0;
	int kSmallFontSize = 4;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenLoad::CScreenLoad()
{

}

CScreenLoad::~CScreenLoad()
{

}

LTBOOL CScreenLoad::Build()
{
	CreateTitle(IDS_TITLE_LOADGAME);

	kNameWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_LOAD,"NameWidth");
	kTimeWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_LOAD,"TimeWidth");
	kIndent = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_LOAD,"Indent");
	kSmallFontSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_LOAD,"SmallFontSize");

	return CBaseScreen::Build();
}


void CScreenLoad::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildSavedLevelLists();
		UseBack(LTTRUE);
	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelLists();
		UseBack(LTFALSE);
	}
	CBaseScreen::OnFocus(bFocus);
}

uint32 CScreenLoad::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= CMD_CUSTOM && dwCommand <= CMD_CUSTOM+1+kMaxSave)
	{
		bool bLoaded = true;

		if (dwCommand == CMD_CUSTOM)
		{
			// This is the Quick load slot but we might not be in a game so don't QuickLoad...
			bLoaded = bLoaded && g_pMissionMgr->StartGameFromQuickSave( );
		}
		else if (dwCommand == CMD_CUSTOM+1)
		{
			bLoaded = bLoaded && g_pMissionMgr->StartGameFromReload( );
		}
		else
		{
            uint32 slot = (dwCommand - CMD_CUSTOM) - 1;
			bLoaded = bLoaded && g_pMissionMgr->StartGameFromSaveSlot( slot );
		}

		if ( !bLoaded )
		{
			MBCreate mb;
	        g_pInterfaceMgr->ShowMessageBox(IDS_LOADGAMEFAILED,&mb);
			return 0;
		}

		return 1;
	}
	else
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

};


CLTGUIColumnCtrl* CScreenLoad::BuildSaveControls( char const* pszIniKey, uint32 nCommandId, uint32 nControlHelpStringId, 
									bool bUserName )
{
	// Check inputs.
	if( !pszIniKey || !pszIniKey[0] )
	{
		ASSERT( !"CScreenLoad::BuildSaveControls:  Invalid inputs." );
		return NULL;
	}

	char szSaveTitle[SLMGR_MAX_INISTR_LEN+1];
	char szWorldName[MAX_PATH*2];
	time_t saveTime;
	SaveGameData saveGameData;

	if( !g_pClientSaveLoadMgr->ReadSaveINI( pszIniKey, szSaveTitle, ARRAY_LEN( szSaveTitle ), 
		szWorldName, ARRAY_LEN( szWorldName ), &saveTime ))
		return NULL;

	ParseSaveString( szWorldName, szSaveTitle, saveTime, &saveGameData, bUserName );
	if( !saveGameData.szUserName[0] )
		return NULL;

	CLTGUIColumnCtrl* pColCtrl = CreateColumnCtrl(nCommandId, nControlHelpStringId);
	pColCtrl->SetFont(LTNULL,kSmallFontSize);

	// The world name column
	pColCtrl->AddColumn(saveGameData.szUserName, kNameWidth);

	// The column that contains the date/time
	pColCtrl->AddColumn(saveGameData.szTime, kTimeWidth);

	return pColCtrl;
		
}

void CScreenLoad::BuildSavedLevelLists()
{
	if (g_pClientSaveLoadMgr->QuickSaveExists())
	{
		CLTGUITextCtrl *pCtrl = AddTextItem( IDS_QUICKLOAD,LTNULL,LTNULL,kDefaultPos,LTTRUE);
		pCtrl->Enable(LTFALSE);
		CLTGUIColumnCtrl*  pColCtrl = BuildSaveControls( QUICKSAVE_INIKEY, CMD_CUSTOM, IDS_HELP_QUICKLOAD, false );
		AddControl(pColCtrl);
		m_nextPos.y += kSmallFontSize;
	}

	if (g_pClientSaveLoadMgr->ReloadSaveExists())
	{
		
		CLTGUITextCtrl *pCtrl = AddTextItem( IDS_LOADCURRENT,LTNULL,LTNULL,kDefaultPos,LTTRUE);
		pCtrl->Enable(LTFALSE);
		CLTGUIColumnCtrl*  pColCtrl = BuildSaveControls( RELOADLEVEL_INIKEY, CMD_CUSTOM+1, IDS_HELP_RELOAD, false );
		AddControl(pColCtrl);
		m_nextPos.y += kSmallFontSize;
	}
	
	CLTGUITextCtrl *pCtrl = AddTextItem(IDS_LOAD_USERGAME,LTNULL,LTNULL,kDefaultPos,LTTRUE);
	pCtrl->Enable(LTFALSE);

	LTIntPt pos = m_nextPos;
	pos.x += kIndent;
	uint16 nHeight = (GetPageBottom() - pos.y);
	uint16 nWidth = (GetPageRight() - pos.x) - 32;
	CLTGUIListCtrl *pList = AddList(pos,nHeight,true,nWidth);
	pList->SetFrameWidth(2);
	pList->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	pList->SetIndent(LTIntPt(4,4));
	pList->SetScrollWrap(LTFALSE);
	for (int i = 0; i < kMaxSave; i++)
	{
		if (g_pClientSaveLoadMgr->SlotSaveExists( i+1 ))
		{
			CLTGUIColumnCtrl*  pColCtrl = BuildSaveControls( g_pClientSaveLoadMgr->GetSlotSaveKey( i+1 ), CMD_CUSTOM+2+i, IDS_HELP_LOADGAME, true );
			pList->AddControl(pColCtrl);

		}
	}
}

void CScreenLoad::ClearSavedLevelLists()
{
	RemoveAll();
}



void CScreenLoad::ParseSaveString(char const* pszWorldName, char const* pszTitle, time_t const& saveTime, SaveGameData *pSG, bool bUserName)
{
	int mission = -1;
	int level = -1;

	// Check inputs.
	if (!pszWorldName || !pszWorldName[0] || !pszTitle )
		return;

	char* pMissionNum = LTNULL;
	char* pLevelNum = LTNULL;
	if (bUserName)
	{
		strncpy(pSG->szUserName,pszTitle,128);
	}
	else
	{
		pSG->szUserName[0] = LTNULL;
		char szTitle[SLMGR_MAX_INISTR_LEN+1];
		SAFE_STRCPY( szTitle, pszTitle );
		pMissionNum = strtok(szTitle,",");
		pLevelNum = strtok(LTNULL,",");
	}

	strncpy(pSG->szWorldName,pszWorldName,128);

	if (pMissionNum)
		mission = atoi(pMissionNum);
	if (pLevelNum)
		level = atoi(pLevelNum);

	struct tm* pTimeDate = LTNULL;
	pTimeDate = localtime (&saveTime);
	if (pTimeDate)
	{
		if (g_pInterfaceResMgr->IsEnglish())
		{
			sprintf (pSG->szTime, "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mon + 1, pTimeDate->tm_mday, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
		}
		else
		{
			sprintf (pSG->szTime, "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mday, pTimeDate->tm_mon + 1, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
		}
	}

	if (strlen(pSG->szUserName) == 0)
	{
		if (mission >= 0)
		{
			MISSION* pMission = g_pMissionButeMgr->GetMission(mission);
			if (pMission)
			{
				int missionId = pMission->nNameId;
				char szMission[64];
				char szLevel[64] = "";
				LoadString(missionId,szMission,sizeof(szMission));
				LEVEL* pLevel = g_pMissionButeMgr->GetLevel(mission,level);
				if (pLevel)
				{
					LoadString(pLevel->nNameId,szLevel,sizeof(szLevel));
				}

				if(strlen(szLevel))
					sprintf(pSG->szUserName,"%s",szLevel);
				else
					sprintf(pSG->szUserName,"%s",szMission);
			}
		}
		else
		{
			strncpy(pSG->szUserName,pszWorldName,128);
		}
	}


};


LTBOOL CScreenLoad::HandleKeyDown(int key, int rep)
{

	if (key == VK_F9)
	{
		SendCommand(CMD_CUSTOM,0,0);
        return LTTRUE;
	}
    return CBaseScreen::HandleKeyDown(key,rep);

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ExitLoadScreenCB
//
//  PURPOSE:	Callback for the load mission screen messagebox...
//
// ----------------------------------------------------------------------- //

static void ExitLoadScreenCB(LTBOOL bReturn, void *pData)
{
	// If they confirm, then go back.
	if (bReturn)
	{
		g_pInterfaceMgr->GetScreenMgr( )->PreviousScreen( );
	}
}



void CScreenLoad::Escape()
{
	// If we're playing mp, and we're about to go to the host screen, then warn
	// the user.
	if( IsMultiplayerGame( ) && g_pLTClient->IsConnected())
	{
		if( m_pScreenMgr->GetFromHistory( 0 ) == SCREEN_ID_HOST )
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = ExitLoadScreenCB;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_SHUTDOWNSERVER,&mb);
			return;
		}
	}
	
	CBaseScreen::Escape();
}
