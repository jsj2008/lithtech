// FolderAwards.cpp: implementation of the CFolderAwards class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderAwards.h"
#include "FolderCommands.h"
#include "FolderMgr.h"
#include "MissionMgr.h"
#include "ClientRes.h"
#include "GameClientShell.h"
#include "MsgIds.h"

extern CGameClientShell* g_pGameClientShell;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderAwards::CFolderAwards()
{
}


CFolderAwards::~CFolderAwards()
{
}

LTBOOL CFolderAwards::Build()
{
	CreateTitle(IDS_TITLE_AWARDS);

    LTBOOL bSuccess = CBaseFolder::Build();
    UseMain(LTTRUE);
    UseBack(LTFALSE);
	UseContinue(FOLDER_ID_BRIEFING,IDS_HELP_NEXTMISSION);
	return bSuccess;
}

void CFolderAwards::Term()
{

}


uint32 CFolderAwards::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == FOLDER_CMD_CONTINUE)
	{
		int mis = g_pGameClientShell->GetCurrentMission();
		if (mis != 0)
			g_pInterfaceMgr->GetMissionData()->NewMission(mis);
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


void CFolderAwards::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		if (!g_pGameClientShell->IsMultiplayerGame() && !HasAwards())
		{
			// Request the player data to be updated (This may take a
			// frame or two to happen in a single-player game...in
			// multiplayer games the values are updated constantly...

			// if we already have awards then the request and the processing have already been done

            HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_SUMMARY);
            g_pLTClient->EndMessage(hMessage);
		}
		else
		{
			UpdateData();
		}
		SetSelection(GetIndex(m_pContinue));

	}
	else
	{
		SetSelection(kNoSelection);
		RemoveFree();
	}
	CBaseFolder::OnFocus(bFocus);
}


void CFolderAwards::UpdateData()
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


	g_pLTClient->CPrint("best:%0.2f,old best:%0.2f",pMSummary->fBestRank, pMSummary->fOldBestRank);
	if (pMSummary->fBestRank > pMSummary->fOldBestRank)
	{
		RANKBONUS oldBonus;
		RANKBONUS newBonus;
		pMission->GetRankBonus(pMSummary->fBestRank,&newBonus);
		pMission->GetRankBonus(pMSummary->fOldBestRank,&oldBonus);
		if (newBonus.nPerturbPoints > oldBonus.nPerturbPoints)
		{
			pCtrl = AddTextItem(IDS_BONUS_PERTURB,LTNULL,0, LTTRUE, GetMediumFont());
			pCtrl->Enable(LTFALSE);
		}

		if (newBonus.nAmmoPoints > oldBonus.nAmmoPoints)
		{
			pCtrl = AddTextItem(IDS_BONUS_AMMO,LTNULL,0, LTTRUE, GetMediumFont());
			pCtrl->Enable(LTFALSE);
		}

		if (newBonus.nArmorPoints > oldBonus.nArmorPoints)
		{
			pCtrl = AddTextItem(IDS_BONUS_ARMOR,LTNULL,0, LTTRUE, GetMediumFont());
			pCtrl->Enable(LTFALSE);
		}

		if (newBonus.nDamagePoints > oldBonus.nDamagePoints)
		{
			pCtrl = AddTextItem(IDS_BONUS_DAMAGE,LTNULL,0, LTTRUE, GetMediumFont());
			pCtrl->Enable(LTFALSE);
		}

		if (newBonus.nHealthPoints > oldBonus.nHealthPoints)
		{
			pCtrl = AddTextItem(IDS_BONUS_HEALTH,LTNULL,0, LTTRUE, GetMediumFont());
			pCtrl->Enable(LTFALSE);
		}

		if (newBonus.nReputationPoints > oldBonus.nReputationPoints)
		{
			pCtrl = AddTextItem(IDS_BONUS_REPUTATION,LTNULL,0, LTTRUE, GetMediumFont());
			pCtrl->Enable(LTFALSE);
		}

		if (newBonus.nStealthPoints > oldBonus.nStealthPoints)
		{
			pCtrl = AddTextItem(IDS_BONUS_STEALTH,LTNULL,0, LTTRUE, GetMediumFont());
			pCtrl->Enable(LTFALSE);
		}

	}

	if (pCtrl)
		AddBlankLine();


//	Generate Awards
	if (pMission->bAllowAwards)
	{
		MISSIONAWARDS *pAwards = g_pMissionMgr->GetMissionAwards();

		int awardNum = 0;
        pCtrl = AddTextItem(IDS_AWARDS,LTNULL,0, LTTRUE, GetLargeFont());
        pCtrl->Enable(LTFALSE);

        if (pAwards->nNumHighAmmoAwards && pMission->nHighAmmo > -1 && pMSummary->dwNumShotsFired >= (uint32)pMission->nHighAmmo)
		{
			awardNum = GetRandom(0,pAwards->nNumHighAmmoAwards-1);
            pCtrl = AddTextItem(pAwards->aHighAmmoAwards[awardNum],LTNULL,0, LTTRUE, GetSmallFont());
            pCtrl->Enable(LTFALSE);
		}
        if (pAwards->nNumLowAmmoAwards && pMission->nLowAmmo > -1 && pMSummary->dwNumShotsFired <= (uint32)pMission->nLowAmmo)
		{
			awardNum = GetRandom(0,pAwards->nNumLowAmmoAwards-1);
            pCtrl = AddTextItem(pAwards->aLowAmmoAwards[awardNum],LTNULL,0, LTTRUE, GetSmallFont());
            pCtrl->Enable(LTFALSE);
		}
	    LTFLOAT fAccuracy = 0.0f;
		if (pMSummary->dwNumShotsFired)
		{
			fAccuracy = (LTFLOAT) pMSummary->dwNumHits / (LTFLOAT) pMSummary->dwNumShotsFired;
		}

		if (pAwards->nNumAccuracyAwards && fAccuracy > pAwards->fAccuracyPct)
		{
			awardNum = GetRandom(0,pAwards->nNumAccuracyAwards-1);
            pCtrl = AddTextItem(pAwards->aAccuracyAwards[awardNum],LTNULL,0, LTTRUE, GetSmallFont());
            pCtrl->Enable(LTFALSE);
		}
		if (pAwards->nNumMarksmanAwards && pMSummary->dwNumShotsFired)
		{

            LTFLOAT fMarksman = (LTFLOAT) pMSummary->dwHitLocations[HL_HEAD] / (LTFLOAT) pMSummary->dwNumShotsFired;
			if (fMarksman > pAwards->fMarksmanPct)
			{
				awardNum = GetRandom(0,pAwards->nNumMarksmanAwards-1);
                pCtrl = AddTextItem(pAwards->aMarksmanAwards[awardNum],LTNULL,0, LTTRUE, GetSmallFont());
                pCtrl->Enable(LTFALSE);
			}
		}
		if (pAwards->nNumNonInjuryAwards && g_pInterfaceMgr->GetPlayerStats()->GetMissionDamage() == 0)
		{

			awardNum = GetRandom(0,pAwards->nNumNonInjuryAwards-1);
            pCtrl = AddTextItem(pAwards->aNonInjuryAwards[awardNum],LTNULL,0, LTTRUE, GetSmallFont());

            pCtrl->Enable(LTFALSE);
		}
		if (pAwards->nNumNotShotAwards && pMSummary->dwNumTimesHit == 0)
		{

			awardNum = GetRandom(0,pAwards->nNumNotShotAwards-1);
            pCtrl = AddTextItem(pAwards->aNotShotAwards[awardNum],LTNULL,0, LTTRUE, GetSmallFont());

            pCtrl->Enable(LTFALSE);
		}
        if (pAwards->nNumStealthAwards && pMission->nMaxDetect > -1 && (pMSummary->dwNumTimesDetected + pMSummary->dwNumBodies) <= (uint32)pMission->nMaxDetect)
		{
			awardNum = GetRandom(0,pAwards->nNumStealthAwards-1);
            pCtrl = AddTextItem(pAwards->aStealthAwards[awardNum],LTNULL,0, LTTRUE, GetSmallFont());
            pCtrl->Enable(LTFALSE);
		}

	}
}



