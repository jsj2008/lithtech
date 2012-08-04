// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPlayerList.h
//
// PURPOSE : Implementation of CHUDPlayerList to display list of teammates
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDPlayerList.h"
#include "ClientInfoMgr.h"
#include "SFXMgr.h"
#include "CharacterFX.h"
#include "GameModeMgr.h"

//******************************************************************************************
//** HUD Player List
//******************************************************************************************
CHUDPlayerList::CHUDPlayerList()
{
	m_UpdateFlags = kHUDPlayers;
	m_bVisible = false;
}


bool CHUDPlayerList::Init()
{
	UpdateLayout();
	EnableFade(false);

	LTVector2n pos = m_vBasePos;

	for (uint8 i = 0; i < MAX_MULTI_PLAYERS; i++)
	{
		m_Players[i].SetFont(m_sTextFont);
		m_Players[i].SetColor(argbWhite);
		m_Players[i].SetAlignment(m_eTextAlignment);
		m_Players[i].SetDropShadow(1);
		m_Players[i].SetPos(pos);
		pos.y += m_sTextFont.m_nHeight;
	}

	ScaleChanged();

	return true;
}

void CHUDPlayerList::Term()
{
	for (uint8 i = 0; i < MAX_MULTI_PLAYERS; i++)
	{
		m_Players[i].FlushTexture();
	}
}

void CHUDPlayerList::Render()
{
	if (!m_bVisible) return;

	for (uint8 i = 0; i < m_nCount; i++)
	{
		m_Players[i].Render();

	}

}
void CHUDPlayerList::Update()
{
	if (!GameModeMgr::Instance( ).m_grbUseTeams)
	{
		m_bVisible = false;
		return;
	}
	m_bVisible = true;

	CClientInfoMgr *pCI = g_pInterfaceMgr->GetClientInfoMgr();
	
	CLIENT_INFO* pLocal = pCI->GetLocalClient();

	
	m_nCount = 0;
	CLIENT_INFO* pClient = pCI->GetFirstClient();

	while (pClient && m_nCount < MAX_MULTI_PLAYERS)
	{
		if (pClient->nTeamID == pLocal->nTeamID)
		{
			if (LTStrCmp(m_Players[m_nCount].GetText(),pClient->sName.c_str()) != 0)
				m_Players[m_nCount].SetText(pClient->sName.c_str());

			CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID(pClient->nID);
			if (!pCharacter || pCharacter->IsPlayerDead() || pCharacter->m_cs.bIsSpectating )
				m_Players[m_nCount].SetColor(m_cDeadColor);
			else if (pClient->nID == pLocal->nID)
				m_Players[m_nCount].SetColor(m_cTextColor);
			else
			{
				if (g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam(pClient->nTeamID))
					m_Players[m_nCount].SetColor(m_cTeamColor[0]);
				else
					m_Players[m_nCount].SetColor(m_cTeamColor[1]);
			}


			m_nCount++;

		}
		pClient = pClient->pNext;
	}

}

void CHUDPlayerList::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDPlayerList");
	}

	CHUDItem::UpdateLayout();

	m_cDeadColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);


	m_cTeamColor[0] = g_pLayoutDB->GetTeamColor(0);
	m_cTeamColor[1] = g_pLayoutDB->GetTeamColor(1);

}

void CHUDPlayerList::ScaleChanged()
{
	LTVector2 vfScale = g_pInterfaceResMgr->GetScreenScale();

	LTVector2n pos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(pos);

	pos.x -= (m_vIconSize.x + 2);

	uint32 nOffset = LTMAX(m_sTextFont.m_nHeight,m_vIconSize.y);

	for (uint8 i = 0; i < MAX_MULTI_PLAYERS; i++)
	{
		m_Players[i].SetPos(pos);
		pos.y += nOffset;
	}

}
