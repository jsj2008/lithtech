// FolderMultiSummary.cpp: implementation of the CFolderMultiSummary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderMultiSummary.h"
#include "BaseSelectionFolder.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "MissionMgr.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"
#include "SoundMgr.h"
#include "MsgIDs.h"
extern CGameClientShell* g_pGameClientShell;


namespace
{
	int kName	= 180;
	int kScore  = 60;
	int kGap	= 32;

	CLTGUIColumnTextCtrl *pLocalCtrl = LTNULL;
	CLTGUIColumnTextCtrl *pTeamCtrl = LTNULL;
	CLTGUIColumnTextCtrl *pLastCtrl = LTNULL;

	char szLocalName[32];
	char szLocalScore[32];
	int nLocalCol;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMultiSummary::CFolderMultiSummary()
{

}

CFolderMultiSummary::~CFolderMultiSummary()
{

}


LTBOOL CFolderMultiSummary::Build()
{

	CreateTitle(IDS_TITLE_SUMMARY);
	m_bEscaped = LTFALSE;
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_MP_SUMMARY,"ColumnWidth"))
		kName = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_MP_SUMMARY,"ColumnWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_MP_SUMMARY,"ScoreWidth"))
		kScore = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_MP_SUMMARY,"ScoreWidth");
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_MP_SUMMARY,"GapWidth"))
		kGap = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_MP_SUMMARY,"GapWidth");


	UseBack(LTFALSE);
	UseMain(LTFALSE);
	UseContinue(LTFALSE);

	return 	CBaseFolder::Build();
;
}


void CFolderMultiSummary::Escape()
{
	if (m_bEscaped) return;
	m_bEscaped = LTTRUE;
	CStaticTextCtrl *pBrief = AddStaticTextItem(IDS_STARTNEXTLEVEL,LTNULL,LTNULL,480,0,LTTRUE,GetSmallFont());
	if (pBrief)
	{
		pBrief->Enable(LTFALSE);
	}
    HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_EXITLEVEL);
    g_pLTClient->EndMessage(hMessage);
}

uint32 CFolderMultiSummary::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
}


void CFolderMultiSummary::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_bEscaped = LTFALSE;
		int missionId = g_pGameClientShell->GetMPMissionName();
		LevelEnd le = g_pGameClientShell->GetLevelEnd();
		int	endString = g_pGameClientShell->GetLevelEndString();

		if (missionId)
		{
			CLTGUITextItemCtrl* pCtrl =AddTextItem(missionId,LTNULL,0,LTTRUE,GetLargeFont());
			pCtrl->SetFixedWidth(480);
			pCtrl->Enable(LTFALSE);
		}

		int leID = 0;
		switch (le)
		{
		case LE_TEAM1_WIN:
			leID = IDS_WIN_TEAM1;
			break;
		case LE_TEAM2_WIN:
			leID = IDS_WIN_TEAM2;
			break;
		case LE_DRAW:
			leID = IDS_WIN_DRAW;
			break;
		case LE_TIMELIMIT:
			leID = IDS_END_TIME;
			break;
		case LE_FRAGLIMIT:
			leID = IDS_END_FRAG;
			break;
		case LE_UNKNOWN:
		default:
			leID = IDS_END_UNKNOWN;
			break;
		}
		if (missionId) AddBlankLine();

		CStaticTextCtrl *pBrief = AddStaticTextItem(leID,LTNULL,LTNULL,480,0,LTTRUE,GetLargeFont());
		if (pBrief)
		{
			pBrief->Enable(LTFALSE);
		}

		AddBlankLine();

		if (endString)
		{
			CLTGUITextItemCtrl* pCtrl =AddTextItem(endString,LTNULL,0,LTTRUE,GetLargeFont());
			pCtrl->SetFixedWidth(480);
			pCtrl->Enable(LTFALSE);
			AddBlankLine();
		}

//		BuildObjectivesList();
		CClientInfoMgr *pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
		char score[32];
		uint32 nLocalID;
		g_pLTClient->GetLocalClientID(&nLocalID);
		pTeamCtrl = LTNULL;
		pLocalCtrl = LTNULL;
		pLastCtrl = LTNULL;
		nLocalCol = 0;
		if (pCIMgr)
		{
			if (g_pGameClientShell->GetGameType() == COOPERATIVE_ASSAULT)
			{
				HSTRING hEmpty = g_pLTClient->CreateString(" ");
				CLTGUIColumnTextCtrl *pCtrl=AddColumnText(LTNULL,LTNULL,LTTRUE,GetMediumFont());
				pCtrl->Enable(LTFALSE);
				pTeamCtrl = pCtrl;

				TEAM_INFO *pTeam = pCIMgr->GetTeam(1);
				HSTRING hName = g_pLTClient->CreateString(pTeam->szName);
				pCtrl->AddColumn(hName, kName, LTF_JUSTIFY_LEFT);
				g_pLTClient->FreeString(hName);

				sprintf(score,"%d",pTeam->nScore);
				HSTRING hScore = g_pLTClient->CreateString(score);
				pCtrl->AddColumn(hScore, kScore, LTF_JUSTIFY_RIGHT);
				g_pLTClient->FreeString(hScore);

				pCtrl->AddColumn(hEmpty, kGap, LTF_JUSTIFY_LEFT);

				pTeam = pCIMgr->GetTeam(2);
				hName = g_pLTClient->CreateString(pTeam->szName);
				pCtrl->AddColumn(hName, kName, LTF_JUSTIFY_LEFT);
				g_pLTClient->FreeString(hName);
				
				sprintf(score,"%d",pTeam->nScore);
				hScore = g_pLTClient->CreateString(score);
				pCtrl->AddColumn(hScore, kScore, LTF_JUSTIFY_RIGHT);
				g_pLTClient->FreeString(hScore);
				
				AddBlankLine();

				int nNumClients[2] = {0,0};
				CLTGUIColumnTextCtrl *pCtrls[16];
				int nNumCtrls = 0;
				memset(pCtrls,0,sizeof(pCtrls));
				CLIENT_INFO *ptr = pCIMgr->GetFirstClient();
				while (ptr)
				{
					sprintf(score,"%d (%d)",ptr->nFrags, (uint32)ptr->m_Ping);
					if (ptr->team == 1 || ptr->team == 2)
					{
						int nCtrl = nNumClients[ptr->team-1];
						if (nCtrl >= nNumCtrls)
						{
							pCtrls[nNumCtrls] = AddColumnText(LTNULL,LTNULL,LTTRUE,GetSmallFont());
							pCtrls[nNumCtrls]->Enable(LTFALSE);
							pCtrls[nNumCtrls]->AddColumn(hEmpty, kName, LTF_JUSTIFY_LEFT);
							pCtrls[nNumCtrls]->AddColumn(hEmpty, kScore, LTF_JUSTIFY_RIGHT);
							pCtrls[nNumCtrls]->AddColumn(hEmpty, kGap, LTF_JUSTIFY_LEFT);
							pCtrls[nNumCtrls]->AddColumn(hEmpty, kName, LTF_JUSTIFY_LEFT);
							pCtrls[nNumCtrls]->AddColumn(hEmpty, kScore, LTF_JUSTIFY_RIGHT);
							nNumCtrls++;
						}

						hScore = g_pLTClient->CreateString(score);

						int nNameCol = 3*(ptr->team-1);
						int nScoreCol = nNameCol+1;
						pCtrls[nCtrl]->SetString(nNameCol,ptr->hstrName);
						pCtrls[nCtrl]->SetString(nScoreCol,hScore);
						g_pLTClient->FreeString(hScore);

						if (ptr->nID == nLocalID)
						{
							pLocalCtrl = pCtrls[nCtrl];
							strcpy(szLocalName,g_pLTClient->GetStringData(ptr->hstrName));
							strcpy(szLocalScore,score);
							nLocalCol = nNameCol;
						}

						nNumClients[ptr->team-1]++;


					}


					ptr = ptr->pNext;
				}
				pLastCtrl = pCtrls[nNumCtrls-1] ;
				g_pLTClient->FreeString(hEmpty);
	
			}
			else
			{
				CLIENT_INFO *ptr = pCIMgr->GetFirstClient();
				while (ptr)
				{
					CLTGUIColumnTextCtrl *pCtrl = AddColumnText(LTNULL,LTNULL,LTTRUE,GetSmallFont());
					pCtrl->Enable(LTFALSE);
					pCtrl->AddColumn(ptr->hstrName, kName, LTF_JUSTIFY_LEFT);
					
					sprintf(score,"%d (%d)",ptr->nFrags, (uint32)ptr->m_Ping);
					HSTRING hScore = g_pLTClient->CreateString(score);
					pCtrl->AddColumn(hScore, kScore, LTF_JUSTIFY_RIGHT);
					g_pLTClient->FreeString(hScore);

					if (ptr->nID == nLocalID)
					{
						pLocalCtrl = pCtrl;
						strcpy(szLocalName,g_pLTClient->GetStringData(ptr->hstrName));
						strcpy(szLocalScore,score);

					}

					ptr = ptr->pNext;
				}
			}
		}
		AddBlankLine();
		UseBack(LTFALSE);
	}
	else
	{
		RemoveFree();
	}
	CBaseFolder::OnFocus(bFocus);

}

/*
// build a list of objectives for the current mission
void CFolderMultiSummary::BuildObjectivesList()
{
	CPlayerStats* pStats = g_pInterfaceMgr->GetPlayerStats();

	ObjectivesList* pObjList = pStats->GetObjectives();
	ObjectivesList* pCompObjList = pStats->GetCompletedObjectives();

	CLTGUIFont *pFont = GetSmallFont();
	for (int i = pObjList->nNumObjectives-1; i >= 0 ; i--)
	{
        uint32 objID = pObjList->dwObjectives[i];

		CGroupCtrl *pGroup = new CGroupCtrl;
        CLTGUITextItemCtrl *pCtrl = CreateTextItem((int)objID, LTNULL, LTNULL, LTTRUE, pFont);
		if (pCtrl)
		{
            pCtrl->Enable(LTFALSE);
			pCtrl->SetFixedWidth(440);
			int nIndex= 0;
			CBitmapCtrl *pCheck = new CBitmapCtrl;
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
*/

LTBOOL CFolderMultiSummary::Render(HSURFACE hDestSurf)
{
	if (!hDestSurf)
	{
        return LTFALSE;
	}

	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();

	CClientInfoMgr *pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	if (pTeamCtrl && pLastCtrl && pCIMgr)
	{
		LTIntPt firstPos = pTeamCtrl->GetPos();
		LTIntPt lastPos = pLastCtrl->GetPos();
		firstPos.x += xo;
		firstPos.y += yo;
		lastPos.x += xo;
		lastPos.y += yo + GetSmallFont()->GetHeight();

		LTRect rcTeam(firstPos.x-8, firstPos.y-8, firstPos.x+kName+kScore+8, lastPos.y+8);

		float fOldAlpha;
		g_pLTClient->GetSurfaceAlpha(pCIMgr->GetTeam(1)->hBanner, fOldAlpha);
		g_pLTClient->SetSurfaceAlpha(pCIMgr->GetTeam(1)->hBanner,0.6f);
		g_pLTClient->SetSurfaceAlpha(pCIMgr->GetTeam(2)->hBanner,0.6f);

		g_pLTClient->ScaleSurfaceToSurface(hDestSurf, pCIMgr->GetTeam(1)->hBanner, &rcTeam, LTNULL);

		rcTeam.left = rcTeam.right + kGap - 16;
		rcTeam.right = rcTeam.left + kName + kScore + 16;

		g_pLTClient->ScaleSurfaceToSurface(hDestSurf, pCIMgr->GetTeam(2)->hBanner, &rcTeam, LTNULL);
		g_pLTClient->SetSurfaceAlpha(pCIMgr->GetTeam(1)->hBanner,fOldAlpha);
		g_pLTClient->SetSurfaceAlpha(pCIMgr->GetTeam(2)->hBanner,fOldAlpha);


	}

	if (!CBaseFolder::Render(hDestSurf))
		return LTFALSE;

	if (pLocalCtrl)
	{
		LTIntPt localPos = pLocalCtrl->GetPos();
		localPos.x += xo;
		localPos.y += yo;
		if (nLocalCol)
			localPos.x += kName+kScore+kGap;

		CLTGUIFont *pFont = GetSmallFont();
		pFont->Draw(szLocalName,hDestSurf,localPos.x,localPos.y,LTF_JUSTIFY_LEFT,kWhite);	
		pFont->Draw(szLocalScore,hDestSurf,localPos.x+kName+kScore,localPos.y,LTF_JUSTIFY_RIGHT,kWhite);	

	}

	return LTTRUE;

}
