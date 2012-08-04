// FolderSummary.cpp: implementation of the CFolderSummary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderSummary.h"
#include "FolderCommands.h"
#include "FolderMgr.h"
#include "FolderAwards.h"
#include "FolderBriefing.h"
#include "MissionMgr.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "MsgIds.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
    LTIntPt  nullPt(0,0);
	int nContinueId = FOLDER_ID_NONE;
	int nContinueHelpId = 0;


}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderSummary::CFolderSummary()
{
}


CFolderSummary::~CFolderSummary()
{
}

LTBOOL CFolderSummary::Build()
{
	CreateTitle(IDS_TITLE_SUMMARY);

    LTBOOL bSuccess = CBaseFolder::Build();
    UseMain(LTTRUE);
    UseBack(LTFALSE);
	return bSuccess;
}

void CFolderSummary::Term()
{

}


uint32 CFolderSummary::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == FOLDER_CMD_CONTINUE && m_nFolderID != FOLDER_ID_FAILURE && nContinueId != FOLDER_ID_AWARDS)
	{
		int mis = g_pGameClientShell->GetCurrentMission();
		if (mis != 0)
		{
			g_pInterfaceMgr->GetMissionData()->NewMission(mis);
			g_pInterfaceMgr->GetPlayerStats()->PrepareInventory();
		}
		else
		{
			//game over man
#ifdef _DEMO
			g_pLTClient->PauseSounds();
			g_pInterfaceMgr->ShowDemoScreens(LTFALSE);
#else
			g_pInterfaceMgr->SwitchToFolder(FOLDER_ID_MAIN);
#endif
			return 1;
		}
	}

	return CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);
}


void CFolderSummary::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		if (!g_pGameClientShell->IsMultiplayerGame())
		{
			// Request the player data to be updated (This may take a
			// frame or two to happen in a single-player game...in
			// multiplayer games the values are updated constantly...

            HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_SUMMARY);
            g_pLTClient->EndMessage(hMessage);
		}
		else
		{
			UpdateData();
		}
		CFolderBriefing* pBriefing = (CFolderBriefing*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_BRIEFING);
		pBriefing->SetPostMission(LTTRUE);

	}
	else
	{
		SetSelection(kNoSelection);
		UseContinue(FOLDER_ID_NONE);
		RemoveFree();
	}
	CBaseFolder::OnFocus(bFocus);
}


void CFolderSummary::UpdateData()
{
	CPlayerSummaryMgr *pPSummary = g_pGameClientShell->GetPlayerSummary();
	if (!pPSummary) return;

	CMissionData* pMissionData = g_pInterfaceMgr->GetMissionData();
	if (!pMissionData) return;

	int nMissionNum = pMissionData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(nMissionNum);
	if (!pMission) return;

	MISSIONSUMMARY* pMSummary = pPSummary->GetMissionSummary(nMissionNum);
	if (!pMSummary) return;

	pPSummary->ReadRankData();

	RemoveFree();

    CLTGUITextItemCtrl* pCtrl = LTNULL;

	CLTGUIColumnTextCtrl *pRanks=AddColumnText(LTNULL, LTNULL, LTTRUE, GetLargeFont());

	// The rank header
	pRanks->AddColumn(IDS_STAT_RANK, 200, LTF_JUSTIFY_LEFT);
	if (m_nFolderID != FOLDER_ID_FAILURE)
	{
		pRanks->AddColumn(IDS_SUMMARY_BEST, 200, LTF_JUSTIFY_LEFT);
	}
    pRanks->Enable(LTFALSE);

	pRanks=AddColumnText(LTNULL, LTNULL, LTTRUE, GetSmallFont());

	//rank
    LTFLOAT fRank = 0.0f;
	int nRankId = 0;
	if (m_nFolderID == FOLDER_ID_FAILURE)
	{
		pRanks->AddColumn(IDS_STAT_FAILED, 200, LTF_JUSTIFY_LEFT);
	}
	else
	{
		fRank = pMSummary->fCurRank;
		nRankId = g_pMissionMgr->GetMissionRating()->GetRankId(fRank);
		pRanks->AddColumn(nRankId, 200, LTF_JUSTIFY_LEFT);
	}


	if (m_nFolderID != FOLDER_ID_FAILURE)
	{
		//best rank
		fRank = pMSummary->fBestRank;
		nRankId = g_pMissionMgr->GetMissionRating()->GetRankId(fRank);
		pRanks->AddColumn(nRankId, 200, LTF_JUSTIFY_LEFT);
	}
    pRanks->Enable(LTFALSE);


	//time in mission
	char str[128];
    LTFLOAT fSec = pMSummary->fTotalMissionTime;
    uint32 nMin = (uint32)fSec / 60;
    fSec -= (60.0f * (LTFLOAT)nMin);
    uint32 nHour = nMin / 60;
	nMin -= 60 * nHour;
	sprintf(str,"%02d:%02d:%.1f",nHour,nMin,fSec);

    HSTRING hTxt=g_pLTClient->FormatString(IDS_STAT_TIME,str);
    pCtrl= AddTextItem(hTxt,LTNULL,0,LTTRUE, GetLargeFont());
    pCtrl->Enable(LTFALSE);
    g_pLTClient->FreeString(hTxt);


	//accuracy
	if (pMission->bAllowAwards)
	{
		pCtrl = AddTextItem(IDS_STAT_ACCURACY,LTNULL,0, LTTRUE, GetLargeFont());
		pCtrl->Enable(LTFALSE);
		LTFLOAT fAccuracy = 0.0f;
		if (pMSummary->dwNumShotsFired)
		{
			fAccuracy = (LTFLOAT) pMSummary->dwNumHits / (LTFLOAT) pMSummary->dwNumShotsFired;
			int nAccuracy =(int)( 100.0f * fAccuracy);

			//shots fired
			hTxt=g_pLTClient->FormatString(IDS_STAT_SHOTS,
					pMSummary->dwNumShotsFired,pMSummary->dwNumHits, nAccuracy);
			pCtrl= AddTextItem(hTxt,LTNULL,0,LTTRUE, GetSmallFont());
			pCtrl->Enable(LTFALSE);
			g_pLTClient->FreeString(hTxt);

			//hit locations
			uint32 h = pMSummary->dwHitLocations[HL_HEAD];
			uint32 t = pMSummary->dwHitLocations[HL_TORSO];
			uint32 a = pMSummary->dwHitLocations[HL_ARM];
			uint32 l = pMSummary->dwHitLocations[HL_LEG];
			hTxt=g_pLTClient->FormatString(IDS_STAT_HITS,h,t,(a+l));
			pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetSmallFont());
			pCtrl->Enable(LTFALSE);
			g_pLTClient->FreeString(hTxt);

			// casualties
			pCtrl = AddTextItem(IDS_STAT_CASUALTIES,LTNULL,0, LTTRUE, GetLargeFont());
			pCtrl->Enable(LTFALSE);
			hTxt=g_pLTClient->FormatString(IDS_STAT_KILLS, pMSummary->dwNumEnemyKills,
											pMSummary->dwNumFriendKills, pMSummary->dwNumNeutralKills);
			pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetSmallFont());
			pCtrl->Enable(LTFALSE);
			g_pLTClient->FreeString(hTxt);


		}
		else
		{
			hTxt=g_pLTClient->FormatString(IDS_STAT_NOSHOTS);
			pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetSmallFont());
			pCtrl->Enable(LTFALSE);
			g_pLTClient->FreeString(hTxt);
		}
	

		//stealth
		pCtrl = AddTextItem(IDS_STAT_STEALTH,LTNULL,0, LTTRUE, GetLargeFont());
		pCtrl->Enable(LTFALSE);

		hTxt=g_pLTClient->FormatString(IDS_STAT_DETECTED,pMSummary->dwNumTimesDetected,pMSummary->dwNumDisturbances);
		pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetSmallFont());
		pCtrl->Enable(LTFALSE);
		g_pLTClient->FreeString(hTxt);
		hTxt=g_pLTClient->FormatString(IDS_STAT_DETECTED2,pMSummary->dwNumBodies,pMSummary->dwNumTimesHit);
		pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetSmallFont());
		pCtrl->Enable(LTFALSE);
		g_pLTClient->FreeString(hTxt);
	}


	//inteligence
    pCtrl = AddTextItem(IDS_STAT_INTEL,LTNULL,0, LTTRUE, GetLargeFont());
    pCtrl->Enable(LTFALSE);

	int cur = pMSummary->m_nMissionCurNumIntel;
	int total = pMSummary->m_nMissionTotalIntel;
    hTxt=g_pLTClient->FormatString(IDS_SUMMARY_FOUND,cur,total);
    pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetSmallFont());
    pCtrl->Enable(LTFALSE);



	if (m_nFolderID == FOLDER_ID_FAILURE)
	{
		nContinueId = FOLDER_ID_LOAD;
		nContinueHelpId = IDS_HELP_LOAD;
	}
	else
	{
		//	Generate Awards
		CFolderAwards* pAwards = (CFolderAwards*)m_pFolderMgr->GetFolderFromID(FOLDER_ID_AWARDS);
		pAwards->UpdateData();
		if (pAwards->HasAwards())
		{
			nContinueId = FOLDER_ID_AWARDS;
			nContinueHelpId = IDS_HELP_AWARDS;
		}
		else
		{
			nContinueId = FOLDER_ID_BRIEFING;
			nContinueHelpId = IDS_HELP_NEXTMISSION;
		}
	}

	UseContinue(nContinueId,nContinueHelpId);
	CalculateLastDrawn();
	CheckArrows();
	SetSelection(GetIndex(m_pContinue));

}


LTBOOL CFolderSummary::HandleKeyDown(int key, int rep)
{
	if (key == VK_F9)
	{
		g_pGameClientShell->QuickLoad();
        return LTTRUE;
	}
    return CBaseFolder::HandleKeyDown(key,rep);

}
