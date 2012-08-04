// FolderStats.cpp: implementation of the CFolderStats class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderStats.h"
#include "FolderIntel.h"
#include "FolderCommands.h"
#include "FolderMgr.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "MsgIds.h"

extern CGameClientShell* g_pGameClientShell;
namespace
{
    const uint32 CMD_INTEL = FOLDER_CMD_CUSTOM+1;
	LTBOOL bOKtoDismiss= LTFALSE;
}

extern LTBOOL g_bLockStats;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderStats::CFolderStats()
{
	m_nKey = 0;
}

CFolderStats::~CFolderStats()
{

}

LTBOOL CFolderStats::Build()
{
	CreateTitle(IDS_TITLE_PLAYER);

    UseArrows(LTTRUE);
	UseBack(LTTRUE,LTFALSE,LTTRUE);

	return CBaseFolder::Build();
}

void CFolderStats::Term()
{

}

void CFolderStats::Escape()
{
	if (g_pGameClientShell->IsInWorld())
	{
		g_pInterfaceMgr->ChangeState(GS_PLAYING);
	}
	else
	{
		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_MAIN);
	}
}

uint32 CFolderStats::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_INTEL)
	{
		CMissionData* pMissionData = g_pInterfaceMgr->GetMissionData();
		if (!pMissionData) return 0;
		int nMissionNum = pMissionData->GetMissionNum();

		CFolderIntel *pIntel = (CFolderIntel *)m_pFolderMgr->GetFolderFromID(FOLDER_ID_INTEL);
		pIntel->SetMissionNum(nMissionNum);

		m_pFolderMgr->SetCurrentFolder(FOLDER_ID_INTEL);
		return 1;
	}

	return CBaseFolder::OnCommand(dwCommand, dwParam1, dwParam2);
}


void CFolderStats::OnFocus(LTBOOL bFocus)
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


		m_nKey = GetCommandKey(COMMAND_ID_FRAGCOUNT);
		bOKtoDismiss = !IsKeyDown(m_nKey);
		SetSelection(GetIndex(m_pBack));

	}
	else
	{
		SetSelection(kNoSelection);
		RemoveFree();
	}
	CBaseFolder::OnFocus(bFocus);
}


LTBOOL CFolderStats::HandleKeyDown(int key, int rep)
{
	if (key == m_nKey && bOKtoDismiss)
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::HandleKeyDown(key, rep);
}

LTBOOL CFolderStats::HandleKeyUp(int key)
{
	bOKtoDismiss = LTTRUE;
	return CBaseFolder::HandleKeyUp(key);
}

void CFolderStats::UpdateData()
{
	CPlayerSummaryMgr *pPSummary = g_pGameClientShell->GetPlayerSummary();
	if (!pPSummary) return;
	CMissionData* pMissionData = g_pInterfaceMgr->GetMissionData();
	if (!pMissionData) return;
	int nMissionNum = pMissionData->GetMissionNum();
	MISSION* pMission = g_pMissionMgr->GetMission(nMissionNum);
	MISSIONSUMMARY* pMSummary = pPSummary->GetMissionSummary(nMissionNum);

	pPSummary->ReadRankData();

	RemoveFree();

    CLTGUITextItemCtrl* pCtrl = LTNULL;
    HSTRING hTxt = LTNULL;

	//time in mission
	char str[128];
    LTFLOAT fSec = pMSummary->fTotalMissionTime;
    uint32 nMin = (uint32)fSec / 60;
    fSec -= (60.0f * (LTFLOAT)nMin);
    uint32 nHour = nMin / 60;
	nMin -= 60 * nHour;
	sprintf(str,"%02d:%02d:%.1f",nHour,nMin,fSec);

    hTxt=g_pLTClient->FormatString(IDS_STAT_TIME,str);
    pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetLargeFont());
    pCtrl->Enable(LTFALSE);
    g_pLTClient->FreeString(hTxt);

	AddBlankLine();

	//accuracy
    pCtrl = AddTextItem(IDS_STAT_ACCURACY,LTNULL,0,LTTRUE, GetLargeFont());
    pCtrl->Enable(LTFALSE);
    LTFLOAT fAccuracy = 0.0f;
	if (pMSummary->dwNumShotsFired)
	{
        fAccuracy = (LTFLOAT) pMSummary->dwNumHits / (LTFLOAT) pMSummary->dwNumShotsFired;
		int nAccuracy =(int)( 100.0f * fAccuracy);

		//shots fired
        hTxt=g_pLTClient->FormatString(IDS_STAT_SHOTS,
				pMSummary->dwNumShotsFired,pMSummary->dwNumHits, nAccuracy);
        pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetSmallFont());
        pCtrl->Enable(LTFALSE);
        g_pLTClient->FreeString(hTxt);

		//hit locations
        uint32 h = pMSummary->dwHitLocations[HL_HEAD];
        uint32 t = pMSummary->dwHitLocations[HL_TORSO];
        uint32 a = pMSummary->dwHitLocations[HL_ARM];
        uint32 l = pMSummary->dwHitLocations[HL_LEG];
        hTxt=g_pLTClient->FormatString(IDS_STAT_HITS,h,t,(a+l));
        pCtrl= AddTextItem(hTxt,LTNULL,0,LTTRUE, GetSmallFont());
        pCtrl->Enable(LTFALSE);
        g_pLTClient->FreeString(hTxt);

		AddBlankLine();

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

	AddBlankLine();

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


	AddBlankLine();

	//inteligence
	int cur = pMSummary->m_nMissionCurNumIntel;
	if (cur)
	{
        pCtrl = AddTextItem(IDS_STAT_INTEL,LTNULL,0, LTTRUE, GetLargeFont());
        pCtrl->Enable(LTFALSE);
		
        hTxt=g_pLTClient->FormatString(IDS_STAT_FOUND,cur);
        pCtrl= AddTextItem(hTxt,LTNULL,0, LTTRUE, GetSmallFont());
        pCtrl->Enable(LTFALSE);
	}
	if (cur || pMSummary->m_nMissionMaxIntel)
		AddTextItem(IDS_VIEW_GALLERY,CMD_INTEL,IDS_HELP_VIEW_GALLERY, LTFALSE, GetSmallFont());

	CalculateLastDrawn();
	CheckArrows();
}



