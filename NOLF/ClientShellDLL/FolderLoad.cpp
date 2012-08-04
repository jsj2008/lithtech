// FolderLoad.cpp: implementation of the CFolderLoad class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "MissionMgr.h"
#include "FolderLoad.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

namespace
{
	const int kMaxSave = 10;
	int kColumnWidth = 350;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderLoad::CFolderLoad()
{

}

CFolderLoad::~CFolderLoad()
{

}

LTBOOL CFolderLoad::Build()
{
	CreateTitle(IDS_TITLE_LOADGAME);
	return CBaseFolder::Build();
}


void CFolderLoad::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildSavedLevelList();
	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelList();
	}
	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderLoad::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= FOLDER_CMD_CUSTOM && dwCommand <= FOLDER_CMD_CUSTOM+1+kMaxSave)
	{
		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		char strKey[32];
		char strFilename[128];

		if (dwCommand == FOLDER_CMD_CUSTOM)
		{
			sprintf (strKey, "SaveGame00");
			strncpy (strFilename, QUICKSAVE_FILENAME, 127);
		}
		else if (dwCommand == FOLDER_CMD_CUSTOM+1)
		{
			sprintf (strKey, "Reload");
			strncpy (strFilename, RELOADLEVEL_FILENAME, 127);
		}
		else
		{
            uint32 slot = (dwCommand - FOLDER_CMD_CUSTOM) - 1;
			sprintf (strKey, "SaveGame%02d", slot);
			sprintf (strFilename, "Save\\Slot%02d.sav", slot);
		}

		CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
		if (!*strSaveGameSetting) return 0;

		char* pWorldName = strtok(strSaveGameSetting,"|");

		if (g_pGameClientShell->LoadGame(pWorldName, strFilename))
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
			return 1;
		}
		else
		{
            HSTRING hString = g_pLTClient->FormatString(IDS_LOADGAMEFAILED);
	        g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
			g_pLTClient->FreeString(hString);
			return 0;
		}



	}
	else
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

};

void CFolderLoad::BuildSavedLevelList()
{
	char strSaveGameSetting[256];
	memset (strSaveGameSetting, 0, 256);

	if (CWinUtil::FileExist(QUICKSAVE_FILENAME))
	{
		SaveGameData sQuickSave;
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, "SaveGame00", "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
		ParseSaveString(strSaveGameSetting,&sQuickSave,LTFALSE);
		if (strlen(sQuickSave.szUserName) > 0)
		{
			CLTGUITextItemCtrl *pCtrl = AddTextItem(IDS_QUICKLOAD,LTNULL,LTNULL,LTTRUE,GetMediumFont());
			pCtrl->Enable(LTFALSE);

			char szStr[256];
			sprintf(szStr,"    %s",sQuickSave.szUserName);
			HSTRING hLoad=g_pLTClient->CreateString(szStr);
			CLTGUIColumnTextCtrl* pColCtrl = AddColumnText(FOLDER_CMD_CUSTOM, IDS_HELP_QUICKLOAD, LTFALSE, GetSmallFont());

			// The world name column
			pColCtrl->AddColumn(hLoad, kColumnWidth, LTF_JUSTIFY_LEFT);

			if (strlen(sQuickSave.szTime) > 0)
			{
				// The column that contains the date/time
				HSTRING hTime=g_pLTClient->CreateString(sQuickSave.szTime);
				pColCtrl->AddColumn(hTime, 230, LTF_JUSTIFY_LEFT);
				g_pLTClient->FreeString(hTime);
			}
			g_pLTClient->FreeString(hLoad);
			
		}
	}

	if (CWinUtil::FileExist(RELOADLEVEL_FILENAME))
	{
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, "Reload", "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
		SaveGameData sAutoSave;
		ParseSaveString(strSaveGameSetting,&sAutoSave,LTFALSE);
		if (strlen(sAutoSave.szUserName) > 0)
		{
			CLTGUITextItemCtrl *pCtrl = AddTextItem(IDS_LOADCURRENT,LTNULL,LTNULL,LTTRUE,GetMediumFont());
			pCtrl->Enable(LTFALSE);

			char szStr[256];
			sprintf(szStr,"    %s",sAutoSave.szUserName);
			HSTRING hLoad=g_pLTClient->CreateString(szStr);
			CLTGUIColumnTextCtrl* pColCtrl = AddColumnText(FOLDER_CMD_CUSTOM+1, IDS_HELP_RELOAD, LTFALSE, GetSmallFont());

			// The world name column
			pColCtrl->AddColumn(hLoad, kColumnWidth, LTF_JUSTIFY_LEFT);

			if (strlen(sAutoSave.szTime) > 0)
			{
				// The column that contains the date/time
				HSTRING hTime=g_pLTClient->CreateString(sAutoSave.szTime);
				pColCtrl->AddColumn(hTime, 230, LTF_JUSTIFY_LEFT);
				g_pLTClient->FreeString(hTime);
			}
			g_pLTClient->FreeString(hLoad);
		}
	}
	
	CLTGUITextItemCtrl *pCtrl = AddTextItem(IDS_LOAD_USERGAME,LTNULL,LTNULL,LTTRUE,GetMediumFont());
	pCtrl->Enable(LTFALSE);
	for (int i = 0; i < kMaxSave; i++)
	{

		char strFilename[128];
		sprintf (strFilename, "Save\\Slot%02d.sav", i+1);
		if (CWinUtil::FileExist(strFilename))
		{

			SaveGameData sSave;
			char szKey[32] = "";
			sprintf (szKey, "SaveGame%02d", i+1);
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, szKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
			ParseSaveString(strSaveGameSetting,&sSave);
			if (strlen(sSave.szUserName) > 0)
			{
				char szStr[256];
				sprintf(szStr,"    %s",sSave.szUserName);
				HSTRING hLoad=g_pLTClient->CreateString(szStr);

				CLTGUIColumnTextCtrl* pCtrl = AddColumnText(FOLDER_CMD_CUSTOM+2+i, IDS_HELP_LOADGAME, LTFALSE, GetSmallFont());

				// The world name column
				pCtrl->AddColumn(hLoad, kColumnWidth, LTF_JUSTIFY_LEFT);

				if (strlen(sSave.szTime) > 0)
				{
					// The column that contains the date/time
					HSTRING hTime=g_pLTClient->CreateString(sSave.szTime);
					pCtrl->AddColumn(hTime, 230, LTF_JUSTIFY_LEFT);
					g_pLTClient->FreeString(hTime);
				}
				g_pLTClient->FreeString(hLoad);
			}
		}

	}


}

void CFolderLoad::ClearSavedLevelList()
{
	unsigned int i;
	for (i=0; i < m_controlArray.GetSize(); i++)
	{
		m_controlArray[i]->Destroy();
		debug_delete(m_controlArray[i]);
	}
	m_controlArray.SetSize(0);
}



void CFolderLoad::ParseSaveString(char* pszSaveStr, SaveGameData *pSG, LTBOOL bUserName)
{
	int mission = -1;
	int level = -1;

	if (!pszSaveStr || strlen (pszSaveStr) == 0) return;

	char* pWorldName = strtok(pszSaveStr,"|");
    char* pName = strtok(LTNULL,"|");
	char* pTimeStr = strtok(LTNULL,"|");

	char* pMissionNum = LTNULL;
	char* pLevelNum = LTNULL;
	if (bUserName)
	{
		strncpy(pSG->szUserName,pName,128);
	}
	else
	{
		pSG->szUserName[0] = LTNULL;
		pMissionNum = strtok(pName,",");
		pLevelNum = strtok(LTNULL,",");
	}

	strncpy(pSG->szWorldName,pWorldName,128);

	if (pMissionNum)
		mission = atoi(pMissionNum);
	if (pLevelNum)
		level = atoi(pLevelNum);

	struct tm* pTimeDate = LTNULL;
	if (pTimeStr)
	{
		time_t nSeconds = (time_t) atol (pTimeStr);
		pTimeDate = localtime (&nSeconds);
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
	}

	if (strlen(pSG->szUserName) == 0)
	{
		if (mission >= 0)
		{
			MISSION* pMission = g_pMissionMgr->GetMission(mission);
			if (pMission)
			{
				int missionId = pMission->nNameId;
				HSTRING hTxt=g_pLTClient->FormatString(missionId);
				if (level <= 0)
				{
					sprintf(pSG->szUserName,"%s",g_pLTClient->GetStringData(hTxt));
				}
				else
				{
					HSTRING hTxt2=g_pLTClient->FormatString(IDS_SCENENUMBER,level + 1);
					sprintf(pSG->szUserName,"%s, %s",g_pLTClient->GetStringData(hTxt),g_pLTClient->GetStringData(hTxt2));
					g_pLTClient->FreeString(hTxt2);
				}
				g_pLTClient->FreeString(hTxt);

			}

		}
		else
		{
			strncpy(pSG->szUserName,pWorldName,128);
		}
	}


};


LTBOOL CFolderLoad::HandleKeyDown(int key, int rep)
{

	if (key == VK_F9)
	{
		SendCommand(FOLDER_CMD_CUSTOM,0,0);
        return LTTRUE;
	}
    return CBaseFolder::HandleKeyDown(key,rep);

}
