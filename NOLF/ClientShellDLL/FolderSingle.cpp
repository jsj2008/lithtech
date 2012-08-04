// FolderSingle.cpp: implementation of the CFolderSingle class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderSingle.h"
#include "FolderCustomLevel.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "WinUtil.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


namespace
{
	const uint32 CMD_GALLERY = FOLDER_CMD_CUSTOM + 1;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderSingle::CFolderSingle()
{
    m_pSaveCtrl = LTNULL;
    m_pLoadCtrl = LTNULL;

}

CFolderSingle::~CFolderSingle()
{

}

// Build the folder
LTBOOL CFolderSingle::Build()
{

	CreateTitle(IDS_TITLE_GAME);

	AddTextItem(IDS_NEWGAME, FOLDER_CMD_NEW_GAME, IDS_HELP_NEW);

	m_pLoadCtrl = AddTextItem(IDS_LOADGAME,	FOLDER_CMD_LOAD_GAME, IDS_HELP_LOAD);

	m_pSaveCtrl = AddTextItem(IDS_SAVEGAME,	FOLDER_CMD_SAVE_GAME, IDS_HELP_SAVE);

	CFolderCustomLevel *pCustom = (CFolderCustomLevel *)m_pFolderMgr->GetFolderFromID(FOLDER_ID_CUSTOM_LEVEL);
	
	if (pCustom->Build() && pCustom->GetNumFiles() > 0)
		AddTextItem(IDS_WARP, FOLDER_CMD_CUSTOM_LEVEL, IDS_HELP_CUSTOM);



#ifndef _DEMO
#ifndef _FINAL

	CButeMgr buteMgr;

	if ( buteMgr.Parse("NOLF\\Attributes\\FavoriteLevels.txt") )
	{
		CLTGUITextItemCtrl *pCtrl = AddTextItem(IDS_FAVORITES,		FOLDER_CMD_FAVORITE_LEVEL, IDS_HELP_FAVORITES);
	}
#endif
#endif

	AddTextItem(IDS_GALLERY,CMD_GALLERY,IDS_HELP_GALLERY);

 	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderSingle::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_NEW_GAME:
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_DIFFICULTY);
			break;
		}
	case CMD_GALLERY:
		{
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_GALLERY);
			break;
		}
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
	case FOLDER_CMD_CUSTOM_LEVEL:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_CUSTOM_LEVEL);
			break;
		}
	case FOLDER_CMD_FAVORITE_LEVEL:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_FAVORITE_LEVEL);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CFolderSingle::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
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