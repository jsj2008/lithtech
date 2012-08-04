// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSave.cpp
//
// PURPOSE : Interface screen for saving games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "MissionButeMgr.h"
#include "ScreenSave.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"


#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

namespace
{
	const int kMaxSave = SLMGR_MAX_SAVE_SLOTS;
	int	 g_nSaveSlot = -1;
	int	 g_nSaveIndex = -1;
	void OverwriteCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenSave *pThisScreen = (CScreenSave *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_SAVE);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OVERWRITE,0,0);
	}
	CLTGUIColumnCtrl* g_pColCtrl = LTNULL;

	char g_szOldName[40]	= { '\0' };
	char g_szOldTime[40]	= { '\0' };

	int kNameWidth = 0;
	int kTimeWidth = 0;
	int kIndent = 0;
	int kSmallFontSize = 4;

	void EditCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenSave *pThisScreen = (CScreenSave *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_SAVE);

		if(pThisScreen)
		{
			if (bReturn)
			{
				pThisScreen->SendCommand(CMD_EDIT_NAME,(uint32)pData,0);
			}
			else
			{
				//the user cancelled we need to restore the original text, which has been saved in the
				//globals above.
				if(g_pColCtrl)
				{
					g_pColCtrl->SetString(1, g_szOldName);
					g_pColCtrl->SetString(2, g_szOldTime);
				}
			}
		}
	};


}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSave::CScreenSave()
{
}

CScreenSave::~CScreenSave()
{

}

LTBOOL CScreenSave::Build()
{
	CreateTitle(IDS_TITLE_SAVEGAME);

	kNameWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SAVE,"NameWidth");
	kTimeWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SAVE,"TimeWidth");
	kIndent = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SAVE,"Indent");
	kSmallFontSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_SAVE,"SmallFontSize");

	return CBaseScreen::Build();
}


void CScreenSave::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildSavedLevelList();
		SetSelection(0);
		UseBack(LTTRUE);
	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelList();
		UseBack(LTFALSE);
	}
	CBaseScreen::OnFocus(bFocus);
}

uint32 CScreenSave::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= CMD_CUSTOM && dwCommand <= CMD_CUSTOM+kMaxSave)
	{
        uint32 slot = dwCommand - CMD_CUSTOM;

		g_nSaveSlot = slot;
		g_nSaveIndex = (int)dwParam1;

		if (g_pClientSaveLoadMgr->SlotSaveExists( g_nSaveSlot ))
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = OverwriteCallBack,
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_CONFIRMSAVE,&mb);
			return 1;
		}
		if (g_nSaveSlot > 0)
		{
			m_szSaveName[0] = LTNULL;		
			NameSaveGame(g_nSaveSlot,g_nSaveIndex);
		}
		else if (!SaveGame(g_nSaveSlot))
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(IDS_SAVEGAMEFAILED,&mb);
		}


	}
	else if (dwCommand == CMD_OVERWRITE)
	{
		NameSaveGame(g_nSaveSlot,g_nSaveIndex);
	}
	else if (dwCommand == CMD_EDIT_NAME)
	{
		char *pStr = (char *)dwParam1;
		if (strlen(pStr))
		{
			SAFE_STRCPY(m_szSaveName,pStr);
		}
		ForceMouseUpdate();
		if (!SaveGame(g_nSaveSlot))
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(IDS_SAVEGAMEFAILED,&mb);
			return 0;
		}
	}

	else
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);

	return 1;
};

void CScreenSave::BuildSavedLevelList()
{
	AddTextItem(IDS_QUICKSAVE,CMD_CUSTOM,IDS_HELP_QUICKSAVE);

	int nFirstEmpty = -1;
	
	CLTGUITextCtrl *pCtrl = AddTextItem(IDS_LOAD_USERGAME,LTNULL,LTNULL,kDefaultPos,LTTRUE);
	pCtrl->Enable(LTFALSE);

 	for (int i = 0; i < kMaxSave; i++)
	{
		LTBOOL bEmpty = LTTRUE;

		if (g_pClientSaveLoadMgr->SlotSaveExists( i+1 ))
		{
			char strSaveGameSetting[256] = {0};

			int mission = -1;
			int level = -1;
			struct tm* pTimeDate = LTNULL;
			char strTime[128] = "";

			char szSaveTitle[SLMGR_MAX_INISTR_LEN+1];
			char szWorldName[MAX_PATH*2];
			time_t saveTime;

			if( g_pClientSaveLoadMgr->ReadSaveINI( g_pClientSaveLoadMgr->GetSlotSaveKey( i+1 ), 
				szSaveTitle, ARRAY_LEN( szSaveTitle ), szWorldName, ARRAY_LEN( szWorldName ), &saveTime ))
			{
				pTimeDate = localtime (&saveTime);
				if (pTimeDate)
				{
					if (g_pInterfaceResMgr->IsEnglish())
					{
						sprintf (strTime, "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mon + 1, pTimeDate->tm_mday, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
					}
					else
					{
						sprintf (strTime, "%02d/%02d/%02d %02d:%02d:%02d", pTimeDate->tm_mday, pTimeDate->tm_mon + 1, (pTimeDate->tm_year + 1900) % 100, pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec);
					}

				}

			}


			char *pWorld = LTNULL;
			char *pTime = LTNULL;
			if ( szSaveTitle[0] )
			{
				pWorld = szSaveTitle;
				bEmpty = LTFALSE;
			}
			else if ( szWorldName[0] )
			{
				pWorld = szWorldName;
				bEmpty = LTFALSE;
			}


			if (pWorld)
			{
				CLTGUIColumnCtrl* pCtrl = AddColumnCtrl(CMD_CUSTOM+1+i, IDS_HELP_SAVEGAME);
				pCtrl->SetFont(LTNULL,kSmallFontSize);
				pCtrl->SetParam1( m_controlArray.size() - 1);

				// This is a spacer
				pCtrl->AddColumn("", kIndent, LTTRUE);

				// The world name column
				pCtrl->AddColumn(pWorld, kNameWidth);

				if (strlen(strTime))
				{
					// The column that contains the date/time
					pCtrl->AddColumn(strTime, kTimeWidth);
				}
			}


		}


		if (bEmpty && nFirstEmpty < 0) 
			nFirstEmpty = i;

	}

	if (nFirstEmpty >= 0)
	{
		CLTGUIColumnCtrl* pCtrl = AddColumnCtrl(CMD_CUSTOM+1+nFirstEmpty, IDS_HELP_SAVEGAME);
		pCtrl->SetFont(LTNULL,kSmallFontSize);
		pCtrl->SetParam1( m_controlArray.size() - 1);
		// This is a spacer
		pCtrl->AddColumn("", kIndent, LTTRUE);

		// The world name column
		char szTmp[64];
		FormatString(IDS_EMPTY,szTmp,sizeof(szTmp));
		pCtrl->AddColumn(szTmp, kNameWidth);


		// The column that contains the date/time
		pCtrl->AddColumn("", kTimeWidth);
	}

}

void CScreenSave::ClearSavedLevelList()
{
	RemoveAll();

}

LTBOOL CScreenSave::SaveGame(uint32 slot)
{
	if (slot > 0)
	{
		if( !g_pClientSaveLoadMgr->SaveGameSlot( slot, m_szSaveName ))
			return false;
		}
	else
	{
		if( !g_pClientSaveLoadMgr->QuickSave() )
			return false;
	}

//	g_pInterfaceMgr->GetMessageMgr()->AddLine(IDS_GAMESAVED);
	g_pInterfaceMgr->ChangeState(GS_PLAYING);
	return LTTRUE;
}


LTBOOL CScreenSave::HandleKeyDown(int key, int rep)
{

	if (key == VK_F6)
	{
		SendCommand(CMD_CUSTOM,0,0);
        return LTTRUE;
	}
    return CBaseScreen::HandleKeyDown(key,rep);

}


void CScreenSave::NameSaveGame(uint32 slot, int index)
{
	sprintf(m_szSaveName,"%s", g_pMissionMgr->GetCurrentWorldName());

	if (!g_pMissionMgr->IsCustomLevel())
	{
		int missionNum = g_pMissionMgr->GetCurrentMission();
		int sceneNum = g_pMissionMgr->GetCurrentLevel();
		MISSION* pMission = g_pMissionButeMgr->GetMission(missionNum);
		if (pMission)
		{
			int missionId = pMission->nNameId;
			char szMission[40] = "";
			char szLevel[40] = "";
			LoadString(missionId,szMission,sizeof(szMission));
			LEVEL* pLevel = g_pMissionButeMgr->GetLevel(missionNum,sceneNum);
			if (pLevel)
			{
				LoadString(pLevel->nNameId,szLevel,sizeof(szLevel));
			}

			if(strlen(szLevel))
				sprintf(m_szSaveName,"%s",szLevel);
			else
				sprintf(m_szSaveName,"%s",szMission);
		}
	}
	

	g_pColCtrl = (CLTGUIColumnCtrl*)GetControl(index);
	if (!g_pColCtrl)
		return;
	LTIntPt pos = g_pColCtrl->GetPos();

	SAFE_STRCPY(g_szOldName,g_pColCtrl->GetString(1));
	SAFE_STRCPY(g_szOldTime,g_pColCtrl->GetString(2));

	g_pColCtrl->SetString(1, "  ");
	g_pColCtrl->SetString(2, "  ");

	//show edit box here	
	MBCreate mb;
	mb.eType = LTMB_EDIT;
	mb.pFn = EditCallBack;
	mb.pString = m_szSaveName;
	mb.nMaxChars = 40;
	g_pInterfaceMgr->ShowMessageBox(IDS_ENTER_NAME,&mb);

}
