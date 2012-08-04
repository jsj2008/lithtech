// FolderBriefing.cpp: implementation of the CFolderBriefing class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderBriefing.h"
#include "BaseSelectionFolder.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "MissionMgr.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"
#include "SoundMgr.h"
extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderBriefing::CFolderBriefing()
{
    m_pMissionCtrl = LTNULL;
    m_pBriefTextCtrl = LTNULL;
	m_bPostMission = LTFALSE;
	m_hSnd = LTNULL;

}

CFolderBriefing::~CFolderBriefing()
{

}


LTBOOL CFolderBriefing::Build()
{

	CreateTitle(IDS_TITLE_BRIEFING);


    m_pMissionCtrl=AddTextItem(IDS_SPACER,LTNULL,0,LTTRUE,GetMediumFont());
	m_pMissionCtrl->SetFixedWidth(480);
    m_pMissionCtrl->Enable(LTFALSE);

	AddBlankLine();

    CLTGUITextItemCtrl* pCtrl=AddTextItem(IDS_BRIEFING,LTNULL,0,LTTRUE,GetMediumFont());
    pCtrl->Enable(LTFALSE);

    m_pBriefTextCtrl = AddStaticTextItem(IDS_SPACER,LTNULL,LTNULL,480,0,LTTRUE,GetSmallFont());
	if (m_pBriefTextCtrl)
	{
        m_pBriefTextCtrl->Enable(LTFALSE);
	}

	return 	CBaseFolder::Build();
;
}


void CFolderBriefing::Escape()
{
	if (m_bPostMission)
	{
		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MAIN);
	}
	CBaseFolder::Escape();
}

uint32 CFolderBriefing::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
}


void CFolderBriefing::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pMissionCtrl->RemoveAll();
		m_pBriefTextCtrl->RemoveString();
		UseBack(LTFALSE);
		UseContinue(FOLDER_ID_OBJECTIVES);

		int missionNum = g_pInterfaceMgr->GetMissionData()->GetMissionNum();

		MISSION* pMission = g_pMissionMgr->GetMission(missionNum);
		if (!pMission) return;

		int missionId = pMission->nNameId;
		int briefId = pMission->nBriefingId;

        HSTRING hTxt=g_pLTClient->FormatString(missionId);
		m_pMissionCtrl->AddString(hTxt);
        g_pLTClient->FreeString(hTxt);

		m_pBriefTextCtrl->SetString(briefId);

		char szFile[256]= "";
		sprintf(szFile,"voice\\%d.wav",briefId);
		m_hSnd = g_pClientSoundMgr->PlayInterfaceSound(szFile, PLAYSOUND_GETHANDLE);

		UseMain(m_bPostMission);
		UseBack(!m_bPostMission);
		SetSelection(GetIndex(m_pContinue));

	}
	else
	{
		if (m_hSnd)
		{
			g_pLTClient->KillSound(m_hSnd);
			m_hSnd = LTNULL;
		}
		m_pMissionCtrl->RemoveAll();
		m_pBriefTextCtrl->RemoveString();
		SetSelection(kNoSelection);
	}
	CBaseFolder::OnFocus(bFocus);

}

