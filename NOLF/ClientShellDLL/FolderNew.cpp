// FolderNew.cpp: implementation of the CFolderNew class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderMgr.h"
#include "FolderBriefing.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "MissionMgr.h"
#include "FolderNew.h"

#include "InterfaceMgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	char	sSelectStr[32] = "";
	int		nOldFirstDrawn;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderNew::CFolderNew()
{
}

CFolderNew::~CFolderNew()
{

}



void CFolderNew::Term()
{
	ClearMissionList();
	CBaseSelectionFolder::Term();
}



// Build the folder
LTBOOL CFolderNew::Build()
{

	CreateTitle(IDS_TITLE_NEWGAME);

	if (strlen(sSelectStr) == 0)
	{
        HSTRING hTemp = g_pLTClient->FormatString(IDS_HELP_MISSION);
        char *pTemp = g_pLTClient->GetStringData(hTemp);
		strncpy(sSelectStr,pTemp,ARRAY_LEN(sSelectStr));
        g_pLTClient->FreeString(hTemp);
	}

    LTBOOL success = CBaseSelectionFolder::Build();
	UseContinue(FOLDER_ID_NONE);

	return success;
}

void CFolderNew::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		BuildMissionList();
		SetSelection(g_pGameClientShell->GetPlayerSummary()->GetNextMission());
	}
	else
	{
		SetSelection(kNoSelection);
		ClearMissionList();
	}
	CBaseSelectionFolder::OnFocus(bFocus);
}

void CFolderNew::BuildMissionList()
{
	int nextMission = g_pGameClientShell->GetPlayerSummary()->GetNextMission();
	int nCurrentMission = -1;
	for (int mis = 0; mis <= nextMission; ++mis)
	{
		MISSION* pMission = g_pMissionMgr->GetMission(mis);
        uint32 cmd = (uint32)mis + FOLDER_CMD_CUSTOM;
		if (!pMission) break;

		int nameId = pMission->nNameId;
		int nListWidth = m_ListRect.right - m_ListRect.left;
        HSTRING hStr = g_pLTClient->FormatString(nameId);

		CLTGUIFont *pFont;
		if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 0)
			pFont = GetSmallFont();
		else if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID) == 1)
			pFont = GetMediumFont();
		else
			pFont = GetLargeFont();

		CBitmapCtrl *pCheck = debug_new(CBitmapCtrl);
		if (g_pLayoutMgr->GetListFontSize((eFolderID)m_nFolderID))
            pCheck->Create(g_pLTClient,"interface\\check-off.pcx","interface\\check-on.pcx");
		else
            pCheck->Create(g_pLTClient,"interface\\small_check-off.pcx","interface\\small_check-on.pcx");
		pCheck->Select((mis < nextMission));

		int nWidth = pCheck->GetWidth();
        CStaticTextCtrl *pCtrl = CreateStaticTextItem(hStr,cmd,LTNULL,(nListWidth - nWidth),0,LTFALSE,pFont);
        pCtrl->Enable(LTTRUE);


		int nHeight = Max(pCtrl->GetHeight(),pCheck->GetHeight());

		CGroupCtrl *pGroup = AddGroup(nListWidth,nHeight,cmd);
        g_pLTClient->FreeString(hStr);

        LTIntPt offset;

        pGroup->AddControl(pCheck,offset,LTFALSE);
		offset.x = nWidth;
        pGroup->AddControl(pCtrl,offset,LTTRUE);

//		AddPhotoBitmap(g_pInterfaceResMgr->GetSharedSurface(pMission->szPhoto));


	}
}

LTBOOL CFolderNew::UpdateSelection()
{
	LTBOOL bChanged = CBaseSelectionFolder::UpdateSelection();
	if (bChanged)
	{
		CGroupCtrl *pGroup = (CGroupCtrl *)GetControl(m_nLastListItem);
		int nMissionID = (pGroup->GetHelpID() - FOLDER_CMD_CUSTOM);

		MISSION* pMission = g_pMissionMgr->GetMission(nMissionID);

		m_pName->RemoveAll();
		m_pName->AddString(pMission->nNameId);

		int briefingId = pMission->nDescriptionId;
		HSTRING hStr;
		if (briefingId)
            hStr=g_pLTClient->FormatString(briefingId);
		else
            hStr=g_pLTClient->CreateString("place mission briefing here");
		m_pDescription->SetString(hStr);
        g_pLTClient->FreeString(hStr);

		SetPhotoBitmap(pMission->szPhoto);
		
		if (m_nFirstDrawn != nOldFirstDrawn)
		{
			nOldFirstDrawn = m_nFirstDrawn;
			for (int i = m_nFirstDrawn; i < m_nLastDrawn; i++)
			{
				CGroupCtrl *pGroup = (CGroupCtrl *)GetControl(i);
				int nMissionID = (pGroup->GetHelpID() - FOLDER_CMD_CUSTOM);

				MISSION* pMission = g_pMissionMgr->GetMission(nMissionID);
				AddPhotoBitmap(g_pInterfaceResMgr->GetSharedSurface(pMission->szPhoto));
			}
		}

	}
	return bChanged;	

}

void CFolderNew::ClearMissionList()
{
	// Terminate the ctrls
	while (m_controlArray.GetSize() > 0)
	{
		m_controlArray[0]->Destroy();
		debug_delete(m_controlArray[0]);
		m_controlArray.Remove(0);
	}
}



uint32 CFolderNew::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
    if (dwCommand >= FOLDER_CMD_CUSTOM && dwCommand < FOLDER_CMD_CUSTOM + (uint32)g_pMissionMgr->GetNumMissions())
	{
		int mis = (int)(dwCommand-FOLDER_CMD_CUSTOM);
		g_pInterfaceMgr->GetMissionData()->NewMission(mis);
		g_pInterfaceMgr->GetPlayerStats()->PrepareInventory();
		CFolderBriefing* pBriefing = (CFolderBriefing*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_BRIEFING);
		pBriefing->SetPostMission(LTFALSE);
		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_BRIEFING);
	}
	else
		return CBaseSelectionFolder::OnCommand(dwCommand,dwParam1,dwParam2);


	return 1;
};


HSTRING CFolderNew::GetHelpString(uint32 dwHelpId, int nControlIndex)
{
	char pStr[512] = "";
	if (nControlIndex >= 0 && nControlIndex < (int)m_controlArray.GetSize())
	{
		MISSION* pMission = g_pMissionMgr->GetMission(dwHelpId-FOLDER_CMD_CUSTOM);
        HSTRING hStr = LTNULL;
		if (pMission)
		{

			int nameId = pMission->nNameId;

            HSTRING hTemp = g_pLTClient->FormatString(nameId);
            char *pName = g_pLTClient->GetStringData(hTemp);


			sprintf(pStr,"%s: %s",m_sSelectStr,pName);

            g_pLTClient->FreeString(hTemp);

            hStr = g_pLTClient->CreateString(pStr);

		}

		return hStr;

	}
	else
		return CBaseSelectionFolder::GetHelpString(dwHelpId,nControlIndex);
}