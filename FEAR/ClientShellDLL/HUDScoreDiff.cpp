// ----------------------------------------------------------------------- //
//
// MODULE  : HUDScoreDiff.cpp
//
// PURPOSE : Implementation of CHUDScoreDiff to display player scores
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDScoreDiff.h"
#include "InterfaceMgr.h"
#include "ClientConnectionMgr.h"
#include "HUDMessageQueue.h"
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"
#include "CharacterFX.h"


CHUDScoreDiff::CHUDScoreDiff()
{
	m_UpdateFlags = kHUDScores;
	m_eLevel = kHUDRenderText;


}


bool CHUDScoreDiff::Init()
{
	UpdateLayout();

	EnableFade(false);
	
	SetSourceString( LoadString("HUD_Score_Chars"));

	return true;

}

void CHUDScoreDiff::Term()
{
}

void CHUDScoreDiff::Render()
{
	if (!IsMultiplayerGameClient()) return;

	SetRenderState();
	m_Text.Render();
}

void CHUDScoreDiff::Update()
{
	if (!IsMultiplayerGameClient()) return;

	if (GameModeMgr::Instance( ).m_grbEliminationWin)
	{
		CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
		if (!pCIMgr) return;

		CLIENT_INFO* pCI = pCIMgr->GetFirstClient();

		int32 nEnemiesLeft = 0;
		while (pCI)
		{
			if (pCI && pCI != g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient())
			{
				CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID(pCI->nID);
				if (pCharacter &&  !pCharacter->IsPlayerDead() && !pCharacter->m_cs.bIsSpectating )
				{
					if (GameModeMgr::Instance( ).m_grbUseTeams)
					{
						if (!pCIMgr->IsLocalTeam(pCI->nTeamID)) 
						{
							++nEnemiesLeft;
						}
					}
					else
					{
						++nEnemiesLeft;
					}
				}
			}
			pCI = pCI->pNext;
		}
		wchar_t wsScore[16] = L"";
		FormatString("HUD_Score_Format",wsScore,LTARRAYSIZE(wsScore),nEnemiesLeft);
		m_Text.SetText(wsScore);
		m_Text.SetColor(m_cTextColor);
	}
	else
	{
		int32 nLocalScore = 0;
		int32 nOtherScore = 0;
		if (GameModeMgr::Instance( ).m_grbUseTeams)
		{
			uint8 nTeam = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalTeam();
			CTeam* pTeam = CTeamMgr::Instance().GetTeam(nTeam);
			if (pTeam)
			{
				nLocalScore = pTeam->GetScore();
			}

			nTeam = 1-nTeam;
			pTeam = CTeamMgr::Instance().GetTeam(nTeam);
			if (pTeam)
			{
				nOtherScore = pTeam->GetScore();
			}
		}
		else
		{
			CLIENT_INFO* pCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
			if (pCI)
			{
				nLocalScore = pCI->sScore.GetScore();
			}

			pCI = g_pInterfaceMgr->GetClientInfoMgr()->GetFirstClient();
			if (pCI && pCI == g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient())
			{
				pCI = pCI->pNext;
			}
			if (pCI)
			{
				nOtherScore = pCI->sScore.GetScore();
			}
		}
		wchar_t wsScore[16] = L"";
		int32 nDiff = (nLocalScore-nOtherScore);

		FormatString("HUD_Score_Format",wsScore,LTARRAYSIZE(wsScore),nDiff);
		if (nDiff > 0)
		{
			m_Text.SetColor(m_cWinningTextColor);
			FormatString("HUD_Score_Format_Advantage",wsScore,LTARRAYSIZE(wsScore),nDiff);
		}
		else if (nDiff == 0)
		{
			m_Text.SetColor(m_cTextColor);
		}
		else
		{
			m_Text.SetColor(m_cLosingTextColor);
		}

		m_Text.SetText(wsScore);

	}

}


void CHUDScoreDiff::ScaleChanged()
{
	CHUDItem::ScaleChanged();
}


void CHUDScoreDiff::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDScoreDiff");
	}

	CHUDItem::UpdateLayout();

	m_cWinningTextColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);
	m_cLosingTextColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,1);

}

