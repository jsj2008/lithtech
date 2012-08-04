// FolderFavoriteLevels.cpp: implementation of the CFolderFavoriteLevels class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderFavoriteLevels.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderFavoriteLevels::CFolderFavoriteLevels()
{
}

CFolderFavoriteLevels::~CFolderFavoriteLevels()
{
}

// Build the folder
LTBOOL CFolderFavoriteLevels::Build()
{


	CreateTitle(IDS_TITLE_CUSTOMLEVELS);

	CButeMgr buteMgr;

	if ( buteMgr.Parse("NOLF\\Attributes\\FavoriteLevels.txt") )
	{
		for ( int iLevel = 0 ; ; iLevel++ )
		{
			char szLevel[64];
			sprintf(szLevel, "FavoriteLevel%d", iLevel);

			if ( buteMgr.Exist(szLevel) )
			{
				CString csName = buteMgr.GetString(szLevel, "Name", (CString&)"(no name)");
				AddTextItem((char *)(LPCSTR)csName, FOLDER_CMD_CUSTOM+iLevel, IDS_HELP_CUSTOMLEVEL);
			}
			else
			{
				break;
			}
		}
	}

	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderFavoriteLevels::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= FOLDER_CMD_CUSTOM)
	{
        uint32 dwSelected = dwCommand-FOLDER_CMD_CUSTOM;

		CButeMgr buteMgr;

		if ( buteMgr.Parse("NOLF\\Attributes\\FavoriteLevels.txt") )
		{
			char szLevel[64];
			sprintf(szLevel, "FavoriteLevel%d", dwSelected);

			if ( buteMgr.Exist(szLevel) )
			{
				CString csFile = buteMgr.GetString(szLevel, "File", (CString&)"");

				if (g_pGameClientShell->LoadWorld((char*)(LPCSTR)csFile))
				{
					g_pInterfaceMgr->ChangeState(GS_PLAYING);
					return 1;
				}
			}
		}
	}
	else
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);

	return 0;
};