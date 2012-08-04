// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTeamScores.cpp
//
// PURPOSE : HUD Element to display team scores
//
// CREATED : 03/23/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDTeamScores.h"
#include "GameModeMgr.h"


CHUDTeamScores::CHUDTeamScores()
{
	m_bDraw = false;
	m_bUpdated = false;
	m_eLevel = kHUDRenderDead;
}


bool CHUDTeamScores::Init()
{

	m_UpdateFlags = kHUDScores;


	return true;
}
void CHUDTeamScores::Term()
{
}

void CHUDTeamScores::Render()
{
	if (!m_bDraw)
	{
		return;
	}

	for (uint16 i = 0; i < MAX_TEAMS; ++i )
	{
		m_Scores[i].Render();
	}


}
void CHUDTeamScores::Update()
{
	if (!m_bUpdated)
	{
		UpdateLayout();
		m_bUpdated = true;
	}
	if (!m_bDraw)
	{
		return;
	}

	int32 nLocalScore = 0;
	int32 nOtherScore = 0;
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

	wchar_t wsScore[16] = L"";
	FormatString("HUD_Score_Format",wsScore,LTARRAYSIZE(wsScore),nLocalScore);
	m_Scores[0].SetText(wsScore);

	FormatString("HUD_Score_Format",wsScore,LTARRAYSIZE(wsScore),nOtherScore);
	m_Scores[1].SetText(wsScore);


}

void CHUDTeamScores::UpdateLayout()
{
	m_bDraw = true;
	if (!GameModeMgr::Instance().m_grbUseTeams || LTStrEmpty(GameModeMgr::Instance().m_grsHUDTeamScoreLayout))
	{
		m_bDraw = false;
		return;
	}

	HRECORD hLayout = g_pLayoutDB->GetHUDRecord(GameModeMgr::Instance().m_grsHUDTeamScoreLayout);
	//if we haven't initialized our layout info
	if (m_hLayout != hLayout)
	{
		m_hLayout = hLayout;
	}

	if (!m_hLayout)
	{
		m_bDraw = false;
		return;
	}


	CHUDItem::UpdateLayout();

	SetSourceString( LoadString("HUD_Score_Chars"));

	for (uint16 i = 0; i < MAX_TEAMS; ++i )
	{
		m_Scores[i].SetFont(m_sTextFont);
		m_Scores[i].SetColor(g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,i));
		m_Scores[i].SetSourceString(m_hSourceString);
		m_Scores[i].SetDropShadow(1);
	}

	m_Scores[0].SetAlignment(kRight);
	m_vScoreOffset[0] = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDTextOffset,0);

	m_Scores[1].SetAlignment(kLeft);
	m_vScoreOffset[1].x = -m_vScoreOffset[0].x;
	m_vScoreOffset[1].y = m_vScoreOffset[0].y;

	ScaleChanged();

}

void CHUDTeamScores::ScaleChanged()
{
	for (uint8 i = 0; i < MAX_TEAMS; ++i )
	{
		LTVector2n vPos = m_vBasePos + m_vScoreOffset[i];
		g_pInterfaceResMgr->ScaleScreenPos(vPos);
		m_Scores[i].SetPos(vPos);
	}

}
