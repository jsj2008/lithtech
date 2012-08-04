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
#include "MissionDB.h"
#include "ScreenLoad.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientSaveLoadMgr.h"
#include "MissionMgr.h"
#include "ClientConnectionMgr.h"

#include "sys/win/mpstrconv.h"

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
	const char* pszFont = NULL;
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

bool CScreenLoad::Build()
{
	CreateTitle("IDS_TITLE_LOADGAME");

	kNameWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,0);
	kTimeWidth = g_pLayoutDB->GetListColumnWidth(m_hLayout,0,1);
	kSmallFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);
	pszFont = g_pLayoutDB->GetListFont(m_hLayout,0);
	kIndent = 2 * kSmallFontSize;

	if (!CBaseScreen::Build())
	{
		return false;
	} 

	m_DefaultPos = m_ScreenRect.m_vMin;
	
	return true;
}


void CScreenLoad::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		m_vfLastScale.Init();
		BuildSavedLevelLists();
		UseBack(true);
	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelLists();
		UseBack(false);
	}
	CBaseScreen::OnFocus(bFocus);
}

uint32 CScreenLoad::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= CMD_CUSTOM && dwCommand <= CMD_CUSTOM+3+kMaxSave)
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
		else if (dwCommand == CMD_CUSTOM+2)
		{
			bLoaded = bLoaded && g_pMissionMgr->StartGameFromCheckpointSave( );
		}
		else
		{
            uint32 slot = (dwCommand - CMD_CUSTOM) - 2;
			bLoaded = bLoaded && g_pMissionMgr->StartGameFromSaveSlot( slot );
		}

		if ( !bLoaded )
		{
			MBCreate mb;
	        g_pInterfaceMgr->ShowMessageBox("IDS_LOADGAMEFAILED",&mb);
			return 0;
		}

		return 1;
	}
	else
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

};


CLTGUIColumnCtrl* CScreenLoad::BuildSaveControls( char const* pszIniKey, uint32 nCommandId, const char* szControlHelpStringId, 
									bool bUserName )
{
	// Check inputs.
	if( !pszIniKey || !pszIniKey[0] )
	{
		ASSERT( !"CScreenLoad::BuildSaveControls:  Invalid inputs." );
		return NULL;
	}

	wchar_t wszSaveTitle[SLMGR_MAX_INISTR_LEN+1];
	char szWorldName[MAX_PATH*2];
	time_t saveTime;
	SaveGameData saveGameData;

	if( !g_pClientSaveLoadMgr->ReadSaveINI( pszIniKey, wszSaveTitle, LTARRAYSIZE( wszSaveTitle ), 
		szWorldName, LTARRAYSIZE( szWorldName ), &saveTime ))
		return NULL;

	ParseSaveString( szWorldName, wszSaveTitle, saveTime, &saveGameData, bUserName );
	if( !saveGameData.wszUserName[0] )
		return NULL;

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kNameWidth+kTimeWidth,kSmallFontSize);
	cs.nCommandID = nCommandId;
	cs.pCommandHandler = this;
	cs.szHelpID = szControlHelpStringId;

	CLTGUIColumnCtrl* pColCtrl = CreateColumnCtrl(cs,false,pszFont,kSmallFontSize );

	// The world name column
	pColCtrl->AddColumn(saveGameData.wszUserName, kNameWidth, true);

	// The column that contains the date/time
	pColCtrl->AddColumn(MPA2W(saveGameData.szTime).c_str(), kTimeWidth, true);

	return pColCtrl;
		
}

void CScreenLoad::BuildSavedLevelLists()
{

	uint32 nFontSize = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);
	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kNameWidth,nFontSize);

	if (g_pClientSaveLoadMgr->QuickSaveExists())
	{

		AddTextItem("IDS_LOADQUICKSAVE", cs, true);
		CLTGUIColumnCtrl*  pColCtrl = BuildSaveControls( QUICKSAVE_INIKEY, CMD_CUSTOM, "IDS_HELP_LOADQUICKSAVE", false );
		AddControl(pColCtrl);
		LTVector2n pos = pColCtrl->GetBasePos();
		pos.x += kIndent;
		pColCtrl->SetBasePos(pos);
	}

	if (g_pClientSaveLoadMgr->ReloadSaveExists())
	{
		
		AddTextItem( "IDS_LOADCURRENT", cs, true);
		CLTGUIColumnCtrl*  pColCtrl = BuildSaveControls( RELOADLEVEL_INIKEY, CMD_CUSTOM+1, "IDS_HELP_RELOAD", false );
		AddControl(pColCtrl);
		LTVector2n pos = pColCtrl->GetBasePos();
		pos.x += kIndent;
		pColCtrl->SetBasePos(pos);
	}

	if (g_pClientSaveLoadMgr->CheckpointSaveExists())
	{
		
		AddTextItem( "IDS_LOADCHECKPOINT", cs, true);
		CLTGUIColumnCtrl*  pColCtrl = BuildSaveControls( CHECKPOINTSAVE_INIKEY, CMD_CUSTOM+2, "IDS_HELP_LOADCHECKPOINTSAVE", false );
		AddControl(pColCtrl);
		LTVector2n pos = pColCtrl->GetBasePos();
		pos.x += kIndent;
		pColCtrl->SetBasePos(pos);
	}
	
	
	AddTextItem("IDS_LOAD_USERGAME", cs, true);
	

	CLTGUIListCtrl_create listCs;
	listCs.rnBaseRect.m_vMin = m_DefaultPos;
	listCs.rnBaseRect.Left() += kIndent;
	listCs.rnBaseRect.Right() = (m_ScreenRect.Right() - 32);
	listCs.rnBaseRect.Bottom() = (m_ScreenRect.Bottom());
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 

	CLTGUIListCtrl *pList = AddList(listCs);

	TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
	pList->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
	pList->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
	pList->SetScrollWrap(false);

	for (int i = 0; i < kMaxSave; i++)
	{
		if (g_pClientSaveLoadMgr->SlotSaveExists( i+1 ))
		{
			CLTGUIColumnCtrl*  pColCtrl = BuildSaveControls( g_pClientSaveLoadMgr->GetSlotSaveKey( i+1 ), CMD_CUSTOM+3+i, "IDS_HELP_LOADGAME", true );
			pList->AddControl(pColCtrl);
		}
	}

	LTVector2n listpos = pList->GetBasePos();
	uint32 listH = pList->GetBaseHeight();
	uint32 listW = pList->GetBaseWidth();
	uint16 nLast = pList->GetNumControls() - 1;
	pList->CalculatePositions();
	CLTGUICtrl *pCtrl = pList->GetControl(nLast);
	if (pCtrl)
	{
		LTVector2n pos = pCtrl->GetBasePos();
		pos.y += pCtrl->GetBaseHeight();
		listH = (pos.y - listpos.y) + 4;

		pList->SetSize( LTVector2n(listW,listH) );

	}

}

void CScreenLoad::ClearSavedLevelLists()
{
	RemoveAll();
}



void CScreenLoad::ParseSaveString(char const* pszWorldName, wchar_t const* pwszTitle, time_t const& saveTime, SaveGameData *pSG, bool bUserName)
{
	int mission = -1;
	int level = -1;

	// Check inputs.
	if (!pszWorldName || !pszWorldName[0] || !pwszTitle )
		return;

	char* pMissionNum = NULL;
	char* pLevelNum = NULL;
	if (bUserName)
	{
		LTStrCpy( pSG->wszUserName,pwszTitle, LTARRAYSIZE( pSG->wszUserName ));
	}
	else
	{
		pSG->wszUserName[0] = '\0';
		char szTitle[SLMGR_MAX_INISTR_LEN+1];
		LTStrCpy( szTitle, MPW2A( pwszTitle ).c_str( ), LTARRAYSIZE(szTitle) );
		pMissionNum = strtok(szTitle,",");
		pLevelNum = strtok(NULL,",");
	}

	LTStrCpy( pSG->szWorldName, pszWorldName, LTARRAYSIZE( pSG->szWorldName ));

	if (pMissionNum)
		mission = atoi(pMissionNum);
	if (pLevelNum)
		level = atoi(pLevelNum);

	struct tm* pTimeDate = NULL;
	pTimeDate = localtime (&saveTime);
	if (pTimeDate)
	{
		if (g_pInterfaceResMgr->IsEnglish())
		{
			LTSNPrintF(pSG->szTime, LTARRAYSIZE( pSG->szTime ), "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mon + 1, pTimeDate->tm_mday, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
		}
		else
		{
			LTSNPrintF(pSG->szTime, LTARRAYSIZE( pSG->szTime ), "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mday, pTimeDate->tm_mon + 1, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
		}
	}

	if( !pSG->wszUserName[0] )
	{
		if (mission >= 0)
		{
			HRECORD hMission = g_pMissionDB->GetMission(mission);
			if (hMission)
			{
				const char* szMissionId = g_pMissionDB->GetString(hMission,MDB_Name);
				const wchar_t* wszMission = LoadString( szMissionId );
				HRECORD hLevel = g_pMissionDB->GetLevel(hMission,level);

				const wchar_t* wszLevel = NULL;
				if (hLevel)
				{
					wszLevel = LoadString(g_pMissionDB->GetString(hLevel,MDB_Name));
				}

				if( wszLevel )
				{
					// strip whitespace
					while( (*wszLevel == ' ' || *wszLevel == '\t') && *wszLevel != '\0' )
						wszLevel++;
					LTStrCpy( pSG->wszUserName, wszLevel, LTARRAYSIZE( pSG->wszUserName ));
				}
				else
					LTStrCpy( pSG->wszUserName, wszMission, LTARRAYSIZE( pSG->wszUserName ));
			}
		}

		//if it's still blank, use the worldname
		if( !pSG->wszUserName[0] )
		{
			LTStrCpy( pSG->wszUserName, MPA2W( pszWorldName ).c_str( ), LTARRAYSIZE( pSG->wszUserName ));
		}
	}
};


bool CScreenLoad::HandleKeyDown(int key, int rep)
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	if (key == VK_F9)
	{
		SendCommand(CMD_CUSTOM,0,0);
        return true;
	}
#endif // PLATFORM_XENON
    return CBaseScreen::HandleKeyDown(key,rep);

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ExitLoadScreenCB
//
//  PURPOSE:	Callback for the load mission screen messagebox...
//
// ----------------------------------------------------------------------- //

static void ExitLoadScreenCB(bool bReturn, void *pData, void* pUserData)
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
	if( IsMultiplayerGameClient( ) && g_pLTClient->IsConnected())
	{
		if( m_pScreenMgr->GetFromHistory( 0 ) == SCREEN_ID_HOST )
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = ExitLoadScreenCB;
			mb.pUserData = this;
			g_pInterfaceMgr->ShowMessageBox("IDS_SHUTDOWNSERVER",&mb);
			return;
		}
	}
	
	CBaseScreen::Escape();
}
