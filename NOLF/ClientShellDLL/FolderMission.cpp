// FolderMission.cpp: implementation of the CFolderMission class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMission.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "MissionMgr.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
    const uint32 kReturn = 1;
    uint32 dwNumStaticControls = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMission::CFolderMission()
{
    m_pMissionCtrl = LTNULL;
}

CFolderMission::~CFolderMission()
{
}


LTBOOL CFolderMission::Build()
{
	CreateTitle(IDS_TITLE_STATUS);

    HSTRING hTxt = LTNULL;

    m_pMissionCtrl=AddTextItem(IDS_SPACER,LTNULL,0,LTTRUE, GetLargeFont());
	m_pMissionCtrl->SetFixedWidth(480);
    m_pMissionCtrl->Enable(LTFALSE);

    CLTGUITextItemCtrl* pCtrl = AddTextItem(IDS_SPACER,LTNULL,0,LTTRUE, GetLargeFont());
    pCtrl->Enable(LTFALSE);

//	AddPageBreak();

    pCtrl=AddTextItem(IDS_OBJECTIVES,LTNULL,0,LTTRUE, GetLargeFont());
	pCtrl->SetFixedWidth(480);
    pCtrl->Enable(LTFALSE);


	dwNumStaticControls = m_controlArray.GetSize();

	return CBaseFolder::Build();

}

void CFolderMission::Term()
{

}

void CFolderMission::Escape()
{
	if (g_pGameClientShell->IsInWorld())
	{
		// Go back to the game...

		CBaseFolder::Escape();
	}
	else
	{
		// Go to the player summary screen...

//		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_SUMMARY);
	}
}

uint32 CFolderMission::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{

	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
}


void CFolderMission::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pMissionCtrl->RemoveAll();

		// Use the current mission if we're in the world, else use
		// the last mission (since that is the one we want to see the
		// summary of)...

		int missionNum = g_pGameClientShell->GetCurrentMission();
		if (g_pGameClientShell->IsInWorld())
		{
			UseContinue(FOLDER_ID_NONE);
			UseBack(LTTRUE);
			SetSelection(GetIndex(m_pBack));
		}
		else
		{
			missionNum--;

			// This should only happen if we're wrapping the mission
			// number...in this case show info for the last mission...

			if (missionNum < 0)
			{
				missionNum = g_pMissionMgr->GetNumMissions()-1;
			}
			UseContinue(FOLDER_ID_SUMMARY);
			m_pContinue->SetHelpID(IDS_HELP_CONTINUE);
            UseMain(LTTRUE);
			UseBack(LTFALSE);
			SetSelection(GetIndex(m_pContinue));
		}

		MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
		if (!pMission) return;

		int missionId = pMission->nNameId;

        HSTRING hTxt=g_pLTClient->FormatString(missionId);
		m_pMissionCtrl->AddString(hTxt);
        g_pLTClient->FreeString(hTxt);


		//add objectives
		BuildObjectivesList();


	}
	else
	{
		SetSelection(kNoSelection);
		while (m_controlArray.GetSize() > dwNumStaticControls)
		{
			CLTGUICtrl* pCtrl = m_controlArray[dwNumStaticControls];
			m_controlArray.Remove(dwNumStaticControls);


		}

	}
	CBaseFolder::OnFocus(bFocus);
}


LTBOOL CFolderMission::HandleKeyDown(int key, int rep)
{

	if (g_pGameClientShell->IsInWorld())
	{
		if (key == VK_F1)
		{
			Escape();
            return LTTRUE;
		}
	}
	else
	{
		if (key == VK_ESCAPE)
		{
			Escape();
            return LTTRUE;
		}
	}

	return CBaseFolder::HandleKeyDown(key,rep);
}

// build a list of objectives for the current mission
void CFolderMission::BuildObjectivesList()
{
	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();

	ObjectivesList* pObjList = pStats->GetObjectives();
	ObjectivesList* pCompObjList = pStats->GetCompletedObjectives();

	CLTGUIFont *pFont = GetSmallFont();
	for (int i = pObjList->nNumObjectives-1; i >= 0 ; i--)
	{
        uint32 objID = pObjList->dwObjectives[i];

		CGroupCtrl *pGroup = debug_new(CGroupCtrl);
        CLTGUITextItemCtrl *pCtrl = CreateTextItem((int)objID, LTNULL, LTNULL, LTTRUE, pFont);
		if (pCtrl)
		{
            pCtrl->Enable(LTFALSE);
			pCtrl->SetFixedWidth(440);
			int nIndex= 0;
			CBitmapCtrl *pCheck = debug_new(CBitmapCtrl);
			if (pCompObjList->Have(objID,nIndex))
			{
                pCheck->Create(g_pLTClient,"interface\\check-on.pcx");
			}
			else
			{
                pCheck->Create(g_pLTClient,"interface\\check-off.pcx");
			}

			int strHeight = pCtrl->GetHeight();
			int bmpHeight = pCheck->GetHeight();
			int height = Max(strHeight,bmpHeight) + 4;


			pGroup->Create(480,height);

            LTIntPt offset(40, (height - strHeight) / 2 );
			pGroup->AddControl(pCtrl,offset);

			offset.x = 0;
			offset.y = (height - bmpHeight) / 2;
			pGroup->AddControl(pCheck,offset);
            pGroup->Enable(LTFALSE);

			AddFreeControl(pGroup);
		}


	}
}