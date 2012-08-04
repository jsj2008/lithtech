// FolderMulti.cpp: implementation of the CFolderMulti class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderMulti.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	const uint32 FOLDER_CMD_JOIN_LAN = FOLDER_CMD_CUSTOM+1;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMulti::CFolderMulti()
{
	int x = 0;
}

CFolderMulti::~CFolderMulti()
{

}

// Build the folder
LTBOOL CFolderMulti::Build()
{

	CreateTitle(IDS_TITLE_MULTI);

	AddTextItem(IDS_PLAYER_SETUP,	FOLDER_CMD_PLAYER,		IDS_HELP_PLAYER);
	AddTextItem(IDS_JOIN,			FOLDER_CMD_JOIN,		IDS_HELP_JOIN);
	AddTextItem(IDS_JOIN_LAN,		FOLDER_CMD_JOIN_LAN,	IDS_HELP_JOIN_LAN);
	AddTextItem(IDS_HOST,			FOLDER_CMD_HOST,		IDS_HELP_HOST);

 	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderMulti::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case FOLDER_CMD_PLAYER:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_PLAYER);
			break;
		}
	case FOLDER_CMD_JOIN:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_JOIN);
			break;
		}
	case FOLDER_CMD_JOIN_LAN:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_JOIN_LAN);
			break;
		}
	case FOLDER_CMD_HOST:
		{
			m_pFolderMgr->SetCurrentFolder(FOLDER_ID_HOST);
			break;
		}
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CFolderMulti::OnFocus(LTBOOL bFocus)
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

