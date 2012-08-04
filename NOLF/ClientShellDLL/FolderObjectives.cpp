// FolderObjectives.cpp: implementation of the CFolderObjectives class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderObjectives.h"
#include "BaseSelectionFolder.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "MissionMgr.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"
#include "SoundMgr.h"
#include "FolderWeapons.h"
#include "FolderGadgets.h"
#include "FolderMods.h"
#include "FolderGear.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
    uint32 dwNumStaticControls = 0;
	int		nContinueIndex  = kNoSelection;
	uint32 CMD_SKIP = FOLDER_CMD_CUSTOM+1;
	uint32 CMD_START = FOLDER_CMD_CUSTOM+2;
	
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderObjectives::CFolderObjectives()
{
    m_pMissionCtrl = LTNULL;
    m_pSpacerCtrl = LTNULL;
	m_pSkipCtrl	= LTNULL;
	m_pSelectCtrl = LTNULL;
	m_eNextFolder = FOLDER_ID_NONE;
}

CFolderObjectives::~CFolderObjectives()
{

}


LTBOOL CFolderObjectives::Build()
{

	CreateTitle(IDS_TITLE_OBJECTIVES);


    m_pMissionCtrl=AddTextItem(IDS_SPACER,LTNULL,0,LTTRUE, GetMediumFont());
	m_pMissionCtrl->SetFixedWidth(480);
    m_pMissionCtrl->Enable(LTFALSE);

	AddBlankLine();

    CLTGUITextItemCtrl* pCtrl=AddTextItem(IDS_OBJECTIVES,LTNULL,0,LTTRUE, GetMediumFont());
    pCtrl->Enable(LTFALSE);

	CLTGUIFont *pFont = GetMediumFont();

    m_pSpacerCtrl = CreateTextItem(" ",LTNULL,LTNULL, LTTRUE, pFont);
    m_pSpacerCtrl->Enable(LTFALSE);

	dwNumStaticControls = m_controlArray.GetSize();

	return 	CBaseFolder::Build();
;
}

void CFolderObjectives::Term()
{
	if (m_controlArray.FindElement(m_pSpacerCtrl) >= m_controlArray.GetSize())
	{
		debug_delete(m_pSpacerCtrl);
        m_pSpacerCtrl = LTNULL;
	}
}

void CFolderObjectives::Escape()
{
	CBaseFolder::Escape();
}

uint32 CFolderObjectives::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_START)
	{
		int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();

		g_pGameClientShell->StartMission(missionNum);
		return 1;

	}
	else if (dwCommand == CMD_SKIP)
	{
		if (m_pFolderMgr)
			m_pFolderMgr->SkipOutfitting();
		g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_INVENTORY);

		return 1;

	}
	else if (dwCommand == FOLDER_CMD_CONTINUE)
	{
		g_pInterfaceMgr->SwitchToFolder(m_eNextFolder);
		return 1;

	}

	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
}


void CFolderObjectives::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pMissionCtrl->RemoveAll();
		UseContinue(FOLDER_ID_NONE);

		int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();

		MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
		if (!pMission) return;

		int missionId = pMission->nNameId;

        HSTRING hTxt=g_pLTClient->FormatString(missionId);
		m_pMissionCtrl->AddString(hTxt);
        g_pLTClient->FreeString(hTxt);

		//add objectives
		BuildObjectivesList();

		//add spacer
		AddFreeControl(m_pSpacerCtrl);


		UseBack(LTFALSE);
	    int nHelp = LTNULL;
		m_eNextFolder = GetNextSelectionFolder(FOLDER_ID_OBJECTIVES,&nHelp);
		UseContinue(FOLDER_ID_NONE);

		
		if (m_eNextFolder == FOLDER_ID_INVENTORY)
		{
			m_pSelectCtrl = CreateTextItem( IDS_START_MISSION, CMD_START, IDS_HELP_START, LTFALSE, GetMediumFont());
			LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_OBJECTIVES,"StartPos");
			AddFixedControl(m_pSelectCtrl,pos);
			SetSelection(GetIndex(m_pSelectCtrl));
		}
		else
		{
			LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_OBJECTIVES,"SkipPos");

			m_pSkipCtrl = CreateTextItem( IDS_SKIP_OUTFITTING,CMD_SKIP, IDS_HELP_SKIP_OUTFITTING, LTFALSE, GetMediumFont());
			AddFixedControl(m_pSkipCtrl,pos);
			SetSelection(GetIndex(m_pSkipCtrl));

			pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_OBJECTIVES,"SelectPos");
			
			m_pSelectCtrl =  CreateTextItem( IDS_DO_OUTFITTING, FOLDER_CMD_CONTINUE, IDS_HELP_DO_OUTFITTING, LTFALSE, GetMediumFont());
			AddFixedControl(m_pSelectCtrl,pos);
		}

		UseBack();
	}
	else
	{
		SetSelection(kNoSelection);
		while (m_controlArray.GetSize() > dwNumStaticControls)
		{
			CLTGUICtrl* pCtrl = m_controlArray[dwNumStaticControls];
			m_controlArray.Remove(dwNumStaticControls);

			if (pCtrl != m_pSpacerCtrl)
				debug_delete(pCtrl);


		}
		if (m_pSkipCtrl)
		{
			RemoveFixedControl(m_pSkipCtrl);
			debug_delete(m_pSkipCtrl);
			m_pSkipCtrl = LTNULL;
 
		}

		if (m_pSelectCtrl)
		{
			RemoveFixedControl(m_pSelectCtrl);
			debug_delete(m_pSelectCtrl);
			m_pSelectCtrl = LTNULL;
 
		}


	}
	CBaseFolder::OnFocus(bFocus);

}


// build a list of objectives for a new mission
void CFolderObjectives::BuildObjectivesList()
{
	int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
	if (!pMission) return;

	CLTGUIFont *pFont = GetSmallFont();

	for (int i = 0; i < pMission->nNumObjectives ; i++)
	{
        uint32 objID = pMission->aObjectiveIds[i];

		CGroupCtrl *pGroup = debug_new(CGroupCtrl);
        CLTGUITextItemCtrl *pCtrl = CreateTextItem((int)objID,LTNULL,LTNULL,LTTRUE,pFont);
		if (pCtrl)
		{
            pCtrl->Enable(LTFALSE);
			pCtrl->SetFixedWidth(440);
			int nIndex= 0;
			CBitmapCtrl *pCheck = debug_new(CBitmapCtrl);
            pCheck->Create(g_pLTClient,"interface\\check-off.pcx");

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
