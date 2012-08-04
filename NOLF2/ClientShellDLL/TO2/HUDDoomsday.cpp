// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDoomsday.cpp
//
// PURPOSE : HUDItem to display status of doomsday pieces
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "TO2PlayerMgr.h"
#include "GameClientShell.h"
#include "DoomsdayPieceFX.h"

//******************************************************************************************
//**
//** HUD Carry Icon display
//**
//******************************************************************************************
namespace
{
	float fAlpha = 1.0f;
	float fBlink = -2.0f;
}

CHUDDoomsday::CHUDDoomsday()
{
	m_eLevel = kHUDRenderDead;
	m_UpdateFlags = kHUDDoomsday;
	for (int i = 0; i < kNumDDIcons; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_hIcon[i][j] = LTNULL;
		}
		m_bBlink[i] = false;
		m_nTeam[i] = INVALID_TEAM;
	};
}


LTBOOL CHUDDoomsday::Init()
{

	m_hIcon[eDD_Transmitter][0] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_trans_b.dtx");
	m_hIcon[eDD_Core][0] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_core_b.dtx");
	m_hIcon[eDD_Battery][0] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_bat_b.dtx");

	m_hIcon[eDD_Transmitter][1] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_trans_r.dtx");
	m_hIcon[eDD_Core][1] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_core_r.dtx");
	m_hIcon[eDD_Battery][1] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_bat_r.dtx");

	m_hIcon[eDD_Transmitter][2] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_trans_n.dtx");
	m_hIcon[eDD_Core][2] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_core_n.dtx");
	m_hIcon[eDD_Battery][2] = g_pInterfaceResMgr->GetTexture("interface\\hud\\doom_bat_n.dtx");


	for (int i = 0; i < kNumDDIcons; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Poly[i],argbWhite);
		SetupQuadUVs(m_Poly[i], m_hIcon[i][2], 0.0f,0.0f,1.0f,1.0f);
	}

	UpdateLayout();

	return LTTRUE;
}

void CHUDDoomsday::Term()
{

}

void CHUDDoomsday::Render()
{

	if (g_pGameClientShell->GetGameType() != eGameTypeDoomsDay) return;

	if (!g_pRadar->GetDraw()) return;

	SetRenderState();

	// calculate a simple fade-in/out cycle
	fAlpha += (fBlink * g_pLTClient->GetFrameTime());
	if (fAlpha > 1.0f)
	{
		fAlpha = 1.0f;
		fBlink = -fBlink;
	}
	else if (fAlpha < 0.3f)
	{
		fAlpha = 0.3f;
		fBlink = -fBlink;
	}
	uint8 nAlpha = (uint8)(255.0f * fAlpha);

	//calculate the fade color
	uint32 nFade =  SET_ARGB(nAlpha,0xFF,0xFF,0xFF);


	
	for (int i = 0; i < kNumDDIcons; i++)
	{
		//if the matching piece is unowned, use normal color and unowned icon
		if (m_nTeam[i] == INVALID_TEAM)
		{
			g_pDrawPrim->SetRGBA(&m_Poly[i],argbWhite);
			g_pDrawPrim->SetTexture(m_hIcon[i][2]);
		}
		else
		{
			//if the matching piece is carried, use the current fade color
			if (m_bBlink[i])
			{
				g_pDrawPrim->SetRGBA(&m_Poly[i],nFade);
			}
			else
			{
				g_pDrawPrim->SetRGBA(&m_Poly[i],argbWhite);
			}
			//use the appropriate team icon
			g_pDrawPrim->SetTexture(m_hIcon[i][m_nTeam[i]]);

		}

		// draw the icon
		g_pDrawPrim->DrawPrim(&m_Poly[i]);
	}
	
}

void CHUDDoomsday::Update()
{
	if (g_pGameClientShell->GetGameType() != eGameTypeDoomsDay) return;

	//clear status
	for (int i = 0; i < kNumDDIcons; i++)
	{
		m_bBlink[i] = false;
		m_nTeam[i] = INVALID_TEAM;
	}

	//step through each doomsday piece in the world
	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_DOOMSDAYPIECE_ID);
	if (pList)
	{
		int nNumDD = pList->GetSize();
		for (int i=0; i < nNumDD; i++)
		{
			if ((*pList)[i])
			{
				CDoomsdayPieceFX* pDDP = (CDoomsdayPieceFX*)(*pList)[i];
				int nIcon = kNumDDIcons;

				// find out which icon matched the piece
				switch (pDDP->GetType())
				{
					case kDoomsDay_transmitter:
						nIcon = eDD_Transmitter;
						break;
					case kDoomsDay_battery:
						nIcon = eDD_Battery;
						break;
					case kDoomsDay_Core:
						nIcon = eDD_Core;
						break;
				}

				// if we've got a match (which should be always)
				if (nIcon < kNumDDIcons)
				{
					//set the status for the icon
					m_bBlink[nIcon] = pDDP->IsCarried();
					m_nTeam[nIcon] = pDDP->GetTeam();
				}
			}

		}
	}

	float fx = (float)(m_BasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float fy = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetXRatio();
	float fw = (float)(m_nSize) * g_pInterfaceResMgr->GetXRatio();
	float fgap = fw * 1.25f;

	for (int i = 0; i < kNumDDIcons; i++)
	{
		g_pDrawPrim->SetXYWH(&m_Poly[i],fx,fy,fw,fw);
		fy += fgap;
	}

}

void CHUDDoomsday::UpdateLayout()
{
	char *pTag = "Doomsday";
	m_BasePos	= g_pLayoutMgr->GetPoint(pTag,"BasePos");
	m_nSize		= (uint8)g_pLayoutMgr->GetInt(pTag,"IconSize");

	if (!m_nSize)
	{
		m_BasePos		= LTIntPt(500,10);
		m_nSize			= 32;
	}

}



