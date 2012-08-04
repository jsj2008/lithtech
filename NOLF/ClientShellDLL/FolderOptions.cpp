// FolderOptions.cpp: implementation of the CFolderOptions class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderOptions.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderOptions::CFolderOptions()
{
}

CFolderOptions::~CFolderOptions()
{

}

// Build the folder
LTBOOL CFolderOptions::Build()
{

	CreateTitle(IDS_TITLE_OPTIONS);

	AddTextItem(IDS_SCREEN,			FOLDER_CMD_DISPLAY,		IDS_HELP_DISPLAY);
	AddTextItem(IDS_SOUND,			FOLDER_CMD_AUDIO,		IDS_HELP_SOUND);
	AddTextItem(IDS_CONTROLS,		FOLDER_CMD_CONTROLS,	IDS_HELP_CONTROLS);
	AddTextItem(IDS_GAME_OPTIONS,	FOLDER_CMD_GAME,		IDS_HELP_GAME_OPTIONS);
	AddTextItem(IDS_PERFORMANCE,	FOLDER_CMD_PERFORMANCE,	IDS_HELP_PERFORMANCE);

	// Make sure to call the base class
	if (! CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);
	return LTTRUE;
}

uint32 CFolderOptions::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_DISPLAY:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_DISPLAY);
			break;
		}
	case FOLDER_CMD_AUDIO:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_AUDIO);
			break;
		}
	case FOLDER_CMD_GAME:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_GAME);
			break;
		}
	case FOLDER_CMD_PERFORMANCE:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_PERFORMANCE);
			break;
		}
	case FOLDER_CMD_CONTROLS:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_CONTROLS);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CFolderOptions::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
	}
	CBaseFolder::OnFocus(bFocus);
}

