// FolderSave.cpp: implementation of the CFolderSave class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "MissionMgr.h"
#include "FolderSave.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

#include "WinUtil.h"
#include <stdio.h>
#include <time.h>

namespace
{
	const int kMaxSave = 10;
	const int CMD_OVERWRITE = FOLDER_CMD_CUSTOM+100;
	const int CMD_EDIT_NAME = CMD_OVERWRITE+1;
	int	 g_nSaveSlot = -1;
	int	 g_nSaveIndex = -1;
	void OverwriteCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderSave *pThisFolder = (CFolderSave *)pData;
		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(CMD_OVERWRITE,0,0);
	}

	CLTGUIColumnTextCtrl* g_pColCtrl = LTNULL;
	HSTRING g_hOldName = LTNULL;
	HSTRING g_hOldTime = LTNULL;

	int kColumnWidth = 330;
	int kGap = 20;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderSave::CFolderSave()
{
	m_pEdit = LTNULL;
}

CFolderSave::~CFolderSave()
{

}

LTBOOL CFolderSave::Build()
{
	CreateTitle(IDS_TITLE_SAVEGAME);

    m_pEdit = CreateEditCtrl("", CMD_EDIT_NAME, LTNULL, m_szSaveName, sizeof(m_szSaveName), 0, LTTRUE, GetSmallFont());
	m_pEdit->EnableCursor();
    m_pEdit->Enable(LTFALSE);
	m_pEdit->SetAlignment(LTF_JUSTIFY_LEFT);

	return CBaseFolder::Build();
}


void CFolderSave::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildSavedLevelList();
		SetSelection(0);
	}
	else
	{
		SetSelection(kNoSelection);
		ClearSavedLevelList();
	}
	CBaseFolder::OnFocus(bFocus);
}

uint32 CFolderSave::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= FOLDER_CMD_CUSTOM && dwCommand <= FOLDER_CMD_CUSTOM+kMaxSave)
	{
        uint32 slot = dwCommand - FOLDER_CMD_CUSTOM;
		char strSaveGameSetting[256];
		char strKey[32];
		sprintf (strKey, "SaveGame%02d", slot);
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);

		char strFilename[128];
		sprintf (strFilename, "Save\\Slot%02d.sav", slot);

		g_nSaveSlot = slot;
		g_nSaveIndex = (int)dwParam1;
		if (strlen (strSaveGameSetting) > 0 && CWinUtil::FileExist(strFilename))
		{
            HSTRING hString = g_pLTClient->FormatString(IDS_CONFIRMSAVE);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,OverwriteCallBack,this);
			g_pLTClient->FreeString(hString);
			return 1;
		}
		if (dwCommand > FOLDER_CMD_CUSTOM)
		{
			m_szSaveName[0] = LTNULL;		
			NameSaveGame(g_nSaveSlot,g_nSaveIndex);
		}
		else if (!SaveGame(g_nSaveSlot))
		{
			HSTRING hString = g_pLTClient->FormatString(IDS_SAVEGAMEFAILED);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
			g_pLTClient->FreeString(hString);
		}


	}
	else if (dwCommand == CMD_OVERWRITE)
	{
		NameSaveGame(g_nSaveSlot,g_nSaveIndex);
	}
	else if (dwCommand == CMD_EDIT_NAME)
	{
		if (GetCapture())
		{
            m_pEdit->Select(LTFALSE);
			UpdateData(LTTRUE);
            SetCapture(LTNULL);
			RemoveFixedControl(m_pEdit);

			ForceMouseUpdate();
			if (!SaveGame(g_nSaveSlot))
			{
				HSTRING hString = g_pLTClient->FormatString(IDS_SAVEGAMEFAILED);
				g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
				g_pLTClient->FreeString(hString);
				return 0;
			}

		}
	}
	else
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

	return 1;
};

void CFolderSave::BuildSavedLevelList()
{
	AddTextItem(IDS_QUICKSAVE,FOLDER_CMD_CUSTOM,IDS_HELP_QUICKSAVE,LTFALSE,GetMediumFont());

	// Create an empty string
    HSTRING hEmpty=g_pLTClient->CreateString(" ");


	int nFirstEmpty = -1;
	
	CLTGUITextItemCtrl *pCtrl = AddTextItem(IDS_LOAD_USERGAME,LTNULL,LTNULL,LTTRUE,GetMediumFont());
	pCtrl->Enable(LTFALSE);

 	for (int i = 0; i < kMaxSave; i++)
	{
		LTBOOL bEmpty = LTTRUE;

		char strFilename[128];
		sprintf (strFilename, "Save\\Slot%02d.sav", i+1);
		if (CWinUtil::FileExist(strFilename))
		{

			char strSaveGameSetting[256];
			memset (strSaveGameSetting, 0, 256);
			char strKey[32];
			sprintf (strKey, "SaveGame%02d", i+1);
			CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);

			int mission = -1;
			int level = -1;
			struct tm* pTimeDate = LTNULL;
			char strWorldName[128] = "";
			char strUserName[128] = "";
			char strTime[128] = "";

			if (strlen (strSaveGameSetting) > 0)
			{
				char* pWorldName = strtok(strSaveGameSetting,"|");
				char* pNameStr = strtok(NULL,"|");
				char* pTimeStr = strtok(NULL,"|");

				time_t nSeconds = (time_t) atol (pTimeStr);
				pTimeDate = localtime (&nSeconds);
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
				sprintf(strWorldName,"    %s",pWorldName);
				sprintf(strUserName,"    %s",pNameStr);

			}


			HSTRING hWorld = LTNULL;
			HSTRING hTime = LTNULL;
			if (strlen(strUserName) > 0)
			{
				hWorld = g_pLTClient->CreateString(strUserName);
				bEmpty = LTFALSE;
			}
			else if (strlen(strWorldName) > 0)
			{
				hWorld=g_pLTClient->CreateString(strWorldName);
				bEmpty = LTFALSE;
			}


			if (!bEmpty && strlen(strTime) > 0)
			{
				// Set the text in the control
				hTime=g_pLTClient->CreateString(strTime);
			}


			if (hWorld)
			{
				CLTGUIColumnTextCtrl* pCtrl = AddColumnText(FOLDER_CMD_CUSTOM+1+i, IDS_HELP_SAVEGAME, LTFALSE, GetSmallFont());
				pCtrl->SetParam1( m_controlArray.GetSize() - 1);
				// The world name column

				pCtrl->AddColumn(hWorld, kColumnWidth, LTF_JUSTIFY_LEFT);

				// This is a spacer
				pCtrl->AddColumn(hEmpty, kGap, LTF_JUSTIFY_LEFT);

				if (hTime)
				{
					// The column that contains the date/time
					pCtrl->AddColumn(hTime, 230, LTF_JUSTIFY_LEFT);
					g_pLTClient->FreeString(hTime);
				}
				g_pLTClient->FreeString(hWorld);
			}


		}


		if (bEmpty && nFirstEmpty < 0) 
			nFirstEmpty = i;

	}

	if (nFirstEmpty >= 0)
	{
		CLTGUIColumnTextCtrl* pCtrl = AddColumnText(FOLDER_CMD_CUSTOM+1+nFirstEmpty, IDS_HELP_SAVEGAME, LTFALSE, GetSmallFont());
		pCtrl->SetParam1( m_controlArray.GetSize() - 1);
		// The world name column

		pCtrl->AddColumn(IDS_EMPTY, kColumnWidth, LTF_JUSTIFY_LEFT);

		// This is a spacer
		pCtrl->AddColumn(hEmpty, kGap, LTF_JUSTIFY_LEFT);

		// The column that contains the date/time
		pCtrl->AddColumn(hEmpty, 230, LTF_JUSTIFY_LEFT);
	}

    g_pLTClient->FreeString(hEmpty);


}

void CFolderSave::ClearSavedLevelList()
{
	unsigned int i;
	for (i=0; i < m_controlArray.GetSize(); i++)
	{
		m_controlArray[i]->Destroy();
		debug_delete(m_controlArray[i]);
	}
	m_controlArray.SetSize(0);
}

LTBOOL CFolderSave::SaveGame(uint32 slot)
{
 	char strFilename[128];
	if (slot > 0)
		sprintf (strFilename, "Save\\Slot%02d.sav", slot);
	else
	{
		strncpy(strFilename, QUICKSAVE_FILENAME, 127);
		int missionNum = -1;
		int sceneNum = -1;

		if (!g_pGameClientShell->IsCustomLevel())
		{
			missionNum = g_pGameClientShell->GetCurrentMission();
			sceneNum = g_pGameClientShell->GetCurrentLevel();
		}

		sprintf (m_szSaveName, "%d,%d", missionNum, sceneNum);

	}

	if (!g_pGameClientShell->SaveGame (strFilename))
	{
		return LTFALSE;
	}

	time_t seconds;
	time (&seconds);

	char strKey[32];
	sprintf (strKey, "SaveGame%02d", slot);
	char strSaveGame[256];

	sprintf (strSaveGame, "%s|%s|%ld",g_pGameClientShell->GetCurrentWorldName(), m_szSaveName, (long)seconds);
	CWinUtil::WinWritePrivateProfileString (GAME_NAME, strKey, strSaveGame, SAVEGAMEINI_FILENAME);

	g_pInterfaceMgr->GetMessageMgr()->AddLine(IDS_GAMESAVED);
	g_pInterfaceMgr->UseCursor(LTFALSE);
	g_pInterfaceMgr->ChangeState(GS_PLAYING);
	return LTTRUE;
}


LTBOOL CFolderSave::HandleKeyDown(int key, int rep)
{

	if (key == VK_F6)
	{
		SendCommand(FOLDER_CMD_CUSTOM,0,0);
        return LTTRUE;
	}
    return CBaseFolder::HandleKeyDown(key,rep);

}


void CFolderSave::NameSaveGame(uint32 slot, int index)
{
	sprintf(m_szSaveName,"%s", g_pGameClientShell->GetCurrentWorldName());

	if (!g_pGameClientShell->IsCustomLevel())
	{
		int missionNum = g_pGameClientShell->GetCurrentMission();
		int sceneNum = g_pGameClientShell->GetCurrentLevel();
		MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
		if (pMission)
		{
			int missionId = pMission->nNameId;
			HSTRING hTxt=g_pLTClient->FormatString(missionId);
			if (sceneNum <= 0)
			{
				sprintf(m_szSaveName,"%s",g_pLTClient->GetStringData(hTxt));
			}
			else
			{
				HSTRING hTxt2=g_pLTClient->FormatString(IDS_SCENENUMBER,sceneNum + 1);
				sprintf(m_szSaveName,"%s, %s",g_pLTClient->GetStringData(hTxt),g_pLTClient->GetStringData(hTxt2));
				g_pLTClient->FreeString(hTxt2);
			}
			g_pLTClient->FreeString(hTxt);

		}
	}

	g_pColCtrl = (CLTGUIColumnTextCtrl*)GetControl(index);
	if (!g_pColCtrl)
		return;
	LTIntPt pos = g_pColCtrl->GetPos();

	g_hOldName = g_pColCtrl->GetString(0);
	g_hOldTime = g_pColCtrl->GetString(2);
	HSTRING hTxt = g_pLTClient->CreateString("  ");
	g_pColCtrl->SetString(0, hTxt);
	g_pColCtrl->SetString(2, hTxt);
	g_pLTClient->FreeString(hTxt);

	
	AddFixedControl(m_pEdit,pos);
	m_pEdit->SetText(m_szSaveName);

	m_pEdit->UpdateData(LTFALSE);
	SetCapture(m_pEdit);
	m_pEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
    m_pEdit->Select(LTTRUE);

}

void CFolderSave::Escape()
{
	if (GetCapture())
	{
        m_pEdit->Select(LTFALSE);
		g_pColCtrl->SetString(0, g_hOldName);
		g_pColCtrl->SetString(2, g_hOldTime);
		SetCapture(LTNULL);
		RemoveFixedControl(m_pEdit);
	}
	else
	{
		CBaseFolder::Escape();
	}

}

/******************************************************************/
LTBOOL	CFolderSave::OnLButtonUp(int x, int y)
{

	if (GetCapture())
	{
		return m_pEdit->OnEnter();
	}
	return CBaseFolder::OnLButtonUp(x,y);
}
LTBOOL	CFolderSave::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::OnRButtonUp(x,y);
}
