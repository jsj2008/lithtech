// FolderGallery.cpp: implementation of the CFolderGallery class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderGallery.h"
#include "FolderIntel.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "WinUtil.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderGallery::CFolderGallery()
{

}

CFolderGallery::~CFolderGallery()
{

}

// Build the folder
LTBOOL CFolderGallery::Build()
{

	CreateTitle(IDS_TITLE_GALLERY);

 	// Make sure to call the base class
	return CBaseFolder::Build();
}

uint32 CFolderGallery::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= FOLDER_CMD_CUSTOM)
	{
		int nMissionNum = (int)(dwCommand - FOLDER_CMD_CUSTOM);
		CFolderIntel *pIntel = (CFolderIntel *)m_pFolderMgr->GetFolderFromID(FOLDER_ID_INTEL);
		pIntel->SetMissionNum(nMissionNum);

		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_INTEL);
		return 1;

	}
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
};


void CFolderGallery::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildMissionList();
		SetSelection(g_pGameClientShell->GetPlayerSummary()->GetNextMission());
	}
	else
	{
		SetSelection(kNoSelection);
		RemoveFree();
	}
	CBaseFolder::OnFocus(bFocus);
}


void CFolderGallery::BuildMissionList()
{
	int nextMission = g_pGameClientShell->GetPlayerSummary()->GetNextMission();
	int nCurrentMission = -1;
	for (int mis = 0; mis <= nextMission; ++mis)
	{
		MISSION* pMission = g_pMissionMgr->GetMission(mis);
        uint32 cmd = (uint32)mis + FOLDER_CMD_CUSTOM;
		if (!pMission) break;

		int nameId = pMission->nNameId;
		CLTGUIFont *pFont;
		if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
			pFont = GetSmallFont();
		else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
			pFont = GetMediumFont();
		else
			pFont = GetLargeFont();

		int numItems = g_pGameClientShell->GetIntelItemMgr()->GetNumItems(mis);

		//if this is the last (and therefore uncompleted) mission only display it if items have been collected
		if (mis < nextMission || numItems)
		{
			CStaticTextCtrl *pCtrl = AddStaticTextItem(nameId,cmd,IDS_HELP_VIEW_GALLERY,0,0,LTFALSE,pFont);
	        pCtrl->Enable((numItems > 0));
		}
		
	}

}

