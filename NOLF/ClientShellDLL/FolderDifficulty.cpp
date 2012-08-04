// FolderDifficulty.cpp: implementation of the CFolderDifficulty class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderDifficulty.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "WinUtil.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	enum eLocalCmds
	{
		CMD_EASY = FOLDER_CMD_CUSTOM+1,
		CMD_MEDIUM,
		CMD_HARD,
		CMD_INSANE
	};
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderDifficulty::CFolderDifficulty()
{
}

CFolderDifficulty::~CFolderDifficulty()
{

}

// Build the folder
LTBOOL CFolderDifficulty::Build()
{

	CreateTitle(IDS_TITLE_DIFFICULTY);

	AddTextItem(IDS_NEW_EASY,	CMD_EASY,	IDS_HELP_EASY);
	AddTextItem(IDS_NEW_MEDIUM, CMD_MEDIUM, IDS_HELP_MEDIUM);
	AddTextItem(IDS_NEW_HARD,	CMD_HARD,	IDS_HELP_HARD);
	AddTextItem(IDS_NEW_INSANE, CMD_INSANE, IDS_HELP_INSANE);

	LTIntPt textPos(360,360);
	int nTextWidth = 180;
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_DIFFICULTY,"TextPos"))
		textPos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_DIFFICULTY,"TextPos");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_DIFFICULTY,"TextWidth"))
		nTextWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_DIFFICULTY,"TextWidth");

	CStaticTextCtrl *pCtrl = CreateStaticTextItem(IDS_DIFF_REMINDER, LTNULL, LTNULL, nTextWidth, 0, LTTRUE, GetMediumFont());
	pCtrl->Enable(LTFALSE);
	AddFixedControl(pCtrl,textPos,LTFALSE);

 	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderDifficulty::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_EASY:
		{
			g_pGameClientShell->SetDifficulty(GD_EASY);
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_NEW);
			break;
		}
	case CMD_MEDIUM:
		{
			g_pGameClientShell->SetDifficulty(GD_NORMAL);
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_NEW);
			break;
		}
	case CMD_HARD:
		{
			g_pGameClientShell->SetDifficulty(GD_HARD);
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_NEW);
			break;
		}
	case CMD_INSANE:
		{
			g_pGameClientShell->SetDifficulty(GD_VERYHARD);
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_NEW);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CFolderDifficulty::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		SetSelection(g_pGameClientShell->GetDifficulty());
	}
	CBaseFolder::OnFocus(bFocus);
}