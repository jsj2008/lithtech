// FolderEscape.cpp: implementation of the CFolderEscape class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderEscape.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "WinUtil.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CFolderEscape *pThisFolder = (CFolderEscape *)pData;
		if (bReturn && pThisFolder)
			pThisFolder->SendCommand(FOLDER_CMD_EXIT,0,0);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderEscape::CFolderEscape()
{
    m_pResumeCtrl = LTNULL;
    m_pSaveCtrl = LTNULL;
    m_pLoadCtrl = LTNULL;

}

CFolderEscape::~CFolderEscape()
{

}

// Build the folder
LTBOOL CFolderEscape::Build()
{

	CreateTitle(IDS_TITLE_GAME);

	m_pResumeCtrl = AddTextItem(IDS_RESUME, FOLDER_CMD_RESUME, IDS_HELP_RESUME);

	m_pLoadCtrl = AddTextItem(IDS_LOADGAME,	FOLDER_CMD_LOAD_GAME, IDS_HELP_LOAD);

	m_pSaveCtrl = AddTextItem(IDS_SAVEGAME,	FOLDER_CMD_SAVE_GAME, IDS_HELP_SAVE);

	AddTextItem(IDS_OPTIONS, FOLDER_CMD_OPTIONS, IDS_HELP_OPTIONS);
	AddTextItem(IDS_MAIN, FOLDER_CMD_MAIN, IDS_HELP_MAIN);
	AddTextItem(IDS_EXIT, FOLDER_CMD_QUIT, IDS_HELP_EXIT);

	UseBack(LTFALSE);

 	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderEscape::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_SAVE_GAME:
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_SAVE);
			break;
		}
	case FOLDER_CMD_LOAD_GAME:
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_LOAD);
			break;
		}
	case FOLDER_CMD_OPTIONS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_OPTIONS);
			break;
		}
	case FOLDER_CMD_QUIT:
		{
            HSTRING hString = g_pLTClient->FormatString(IDS_SUREWANTQUIT);
			g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,QuitCallBack,this);
			g_pLTClient->FreeString(hString);
			break;
		}
	case FOLDER_CMD_EXIT:
		{
#ifdef _DEMO
			g_pInterfaceMgr->ShowDemoScreens(LTTRUE);
#else
            g_pLTClient->Shutdown();
#endif
			break;
		}
	case FOLDER_CMD_RESUME:
		{
			Escape();
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CFolderEscape::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		LTBOOL bInGame = g_pGameClientShell->IsInWorld() && hPlayerObj && !g_pGameClientShell->IsPlayerDead();
		m_pResumeCtrl->Enable(bInGame);

		LTBOOL bEnable = ( g_pGameClientShell->GetGameType() == SINGLE && 
			!g_pGameClientShell->IsUsingExternalCamera() && 
			g_pGameClientShell->IsPlayerInWorld() && 
			!g_pGameClientShell->IsPlayerDead() &&
			!g_pGameClientShell->GetMoveMgr()->IsZipCordOn() );

		m_pSaveCtrl->Enable(bEnable);

		char strSaveGameSetting[256];
		memset (strSaveGameSetting, 0, 256);
		char strKey[32];

		sprintf (strKey, "Reload");
		CWinUtil::WinGetPrivateProfileString (GAME_NAME, strKey, "", strSaveGameSetting, 256, SAVEGAMEINI_FILENAME);
		m_pLoadCtrl->Enable( (strlen (strSaveGameSetting) > 0) );
	}
	CBaseFolder::OnFocus(bFocus);
}