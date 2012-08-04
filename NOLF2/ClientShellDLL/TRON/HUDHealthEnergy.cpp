// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHealthEnergy.cpp
//
// PURPOSE : HUDItem to display health cache and energy
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "HUDHealthEnergy.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "TronLayoutMgr.h"
#include "TronPlayerMgr.h"

//******************************************************************************************
//**
//** HUD health cache and energy display
//**
//******************************************************************************************

namespace
{
	const uint32 argbTransWhite		= 0x80FFFFFF;
}

CHUDHealthEnergy::CHUDHealthEnergy()
{
	// Make this something new
	m_UpdateFlags = kHUDHealth | kHUDEnergy;
	m_fScale = 0.0f;
}


LTBOOL CHUDHealthEnergy::Init()
{
	UpdateLayout();
	m_hHUDTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\hud.dtx");
	m_hHUDHubTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\hudhub.dtx");

	// Build the drawprims

	// HUD center bit
	g_pDrawPrim->SetRGBA(&m_HUDHub,argbWhite);

	// health bg
	g_pDrawPrim->SetRGBA(&m_HealthBG,argbTransWhite);
	// energy bg
	g_pDrawPrim->SetRGBA(&m_EnergyBG,argbTransWhite);
	// health bar
	g_pDrawPrim->SetRGBA(&m_HealthBar,0xC0FFFFFF);
	// energy bar
	g_pDrawPrim->SetRGBA(&m_EnergyBar,0xC0FFFFFF);
	// energy transfer
	g_pDrawPrim->SetRGBA(&m_EnergyTransfer, 0xC0FFFFFF);
	// health blink
	g_pDrawPrim->SetRGBA(&m_HealthBlinkBar,argbTransWhite);

	m_fHealthBlinkTime = 0.0f;
	m_fLayoutHealthBlinkTime = 2.0f;
	m_bHealthBlink = false;

	m_iHealthWidth = 160;
	m_iEnergyWidth = 160;

	return LTTRUE;
}

void CHUDHealthEnergy::Term()
{
	if (m_pHealthStr)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pHealthStr);
		m_pHealthStr = LTNULL;
	}
	if (m_pEnergyStr)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pEnergyStr);
		m_pEnergyStr = LTNULL;
	}
}

void CHUDHealthEnergy::Render()
{
	SetRenderState();

	// hub center bit
	g_pDrawPrim->SetTexture(m_hHUDHubTex);
	g_pDrawPrim->DrawPrim(&m_HUDHub);

	g_pDrawPrim->SetTexture(m_hHUDTex);

	// reduce blink time, if any.
	// hack turn it off for now
	if (m_fHealthBlinkTime > 0.0f)
	{
		// reduce blink time
		// determine if it means that the blink is actually happening this frame
		m_fHealthBlinkTime = 0.0f;
	}
	m_bHealthBlink = false;

	// draw backgrounds
	g_pDrawPrim->DrawPrim(&m_HealthBG);
	g_pDrawPrim->DrawPrim(&m_EnergyBG);
	
	// draw bars
	if (m_bHealthBlink)
		g_pDrawPrim->DrawPrim(&m_HealthBlinkBar);
	else
		g_pDrawPrim->DrawPrim(&m_HealthBar);

	g_pDrawPrim->DrawPrim(&m_EnergyBar);
	
	// draw text
	if (m_pHealthStr)
		m_pHealthStr->Render();
	if (m_pEnergyStr)
		m_pEnergyStr->Render();

	if (g_pTronPlayerMgr->IsTransferringEnergy())
	{
		g_pDrawPrim->DrawPrim(&m_EnergyTransfer);
	}
}

void CHUDHealthEnergy::Update()
{
	uint32 oldHealth = m_iHealth;
	uint32 oldEnergy = m_iEnergy;

	uint32 oldHealthMax = m_iHealthMax;
	uint32 oldEnergyMax = m_iEnergyMax;

	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	m_iHealth = g_pPlayerStats->GetHealth();
	m_iEnergy = g_pTronPlayerStats->GetEnergy();
	m_iHealthMax = g_pPlayerStats->GetMaxHealth();
 	m_iEnergyMax = g_pTronPlayerStats->GetMaxEnergy();

	m_bHealthBlink = ((oldHealth != m_iHealth) || (oldHealthMax != m_iHealthMax));
	bool bEnergyBlink = ((oldEnergy != m_iEnergy) || (oldEnergyMax != m_iEnergyMax));

	// font-y stuff
	char buf[6];

	if (m_bHealthBlink)
	{
		// TODO Set healthblink time
		// Update the health bar dims (left justified)
		float fWidth = (float)(m_iHealthWidth *((float)m_iHealth / (float)m_iHealthMax));
		m_HealthBar.verts[1].x = m_HealthBar.verts[2].x =
			m_HealthBlinkBar.verts[1].x = m_HealthBlinkBar.verts[2].x =
				m_HealthBar.verts[0].x + fWidth;

		// Update the string
		sprintf(buf, "%d", m_iHealth);
		if (m_pHealthStr)
		{
			m_pHealthStr->SetText(buf);
		}
	}
	if (bEnergyBlink)
	{
		// Update the energy bar dims
		float fWidth = (float)(m_iEnergyWidth *((float)m_iEnergy / (float)m_iEnergyMax));
		m_EnergyBar.verts[0].x = m_EnergyBar.verts[3].x =
				m_EnergyBar.verts[1].x - fWidth;

		// Update the string
		sprintf(buf, "%d", m_iEnergy);
		if (m_pEnergyStr)
		{
			m_pEnergyStr->SetText(buf);
		}
	}
	// If an energy transfer is in progress, show how much energy it is taking
	if (g_pTronPlayerMgr->IsTransferringEnergy())
	{
		float fEnergyRequired = (float)g_pTronPlayerMgr->GetEnergyRequired();
		float fPercentTransferred = g_pTronPlayerMgr->GetPercentEnergyTransferred();
		float fCurrentEnergyDrain = fEnergyRequired * fPercentTransferred;
		float fDrainWidth = m_iEnergyWidth * (fCurrentEnergyDrain/(float)m_iEnergyMax);
		// left edge
		m_EnergyTransfer.verts[0].x = m_EnergyTransfer.verts[3].x =
			m_EnergyBar.verts[0].x;
		m_EnergyTransfer.verts[1].x = m_EnergyTransfer.verts[2].x =
			m_EnergyBar.verts[0].x + fDrainWidth;
	}
}

void CHUDHealthEnergy::UpdateLayout()
{
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_iFontNum = ((CTronLayoutMgr *)g_pLayoutMgr)->GetHealthEnergyFont(nCurrentLayout);

	// load HealthBGRect
	// load HealthBGTexRect
	// load HealthBarRect
	// load HealthBarTexRect
	// load HealthBlinkTime

	// load EnergyBGRect
	// load EnergyBGTexRect
	// load EnergyBarRect
	// load EnergyBarTexRect

	// load HUDHubRect
	g_pDrawPrim->SetXYWH(&m_HUDHub, (320.0f * xs) - 63, (480.0f * ys) - 32, 128, 32);
	g_pDrawPrim->SetUVWH(&m_HUDHub, 0.0f, 0.0f, 1.0f, 1.0f);

	// load the healthbar width into m_iHealthWidth
	m_iHealthWidth = 160;

	// load the energybar width into m_iEnergyWidth
	m_iEnergyWidth = 160;

	// Get HealthBlinkTime
	m_fLayoutHealthBlinkTime = 2.0f;

	// get HealthBGPos
	g_pDrawPrim->SetXYWH(&m_HealthBG, (320.0f * xs) + 12, (480.0f * ys) - 19, 248, 20);

	// get HealthBGTexCoords
	g_pDrawPrim->SetUVWH(&m_HealthBG, 261.0f / 512.0f, 467.0f / 512.0f, 248.0f / 512.0f, 20.0f / 512.0f);

	// get HealthBarPos
	g_pDrawPrim->SetXYWH(&m_HealthBar, m_HealthBG.verts[0].x + 61, m_HealthBG.verts[0].y + 5, (float)m_iHealthWidth, 10);
	g_pDrawPrim->SetXYWH(&m_HealthBlinkBar, m_HealthBG.verts[0].x + 61, m_HealthBG.verts[0].y + 5, (float)m_iHealthWidth, 10);

	// get HealthBarTexCoords
	g_pDrawPrim->SetUVWH(&m_HealthBar, 349.0f / 512.0f, 441.0f / 512.0f, 160.0f / 512.0f, 10.0f / 512.0f);

	// get HealthBlinkBarTexCoords
	g_pDrawPrim->SetUVWH(&m_HealthBlinkBar, 349.0f / 512.0f, 441.0f / 512.0f, 160.0f / 512.0f, 10.0f / 512.0f);

	// get EnergyBGPos
	g_pDrawPrim->SetXYWH(&m_EnergyBG, (320.0f * xs) - 260, (480.0f * ys) - 19, 248, 20);

	// get EnergyBGTexCoords
	g_pDrawPrim->SetUVWH(&m_EnergyBG, 261.0f / 512.0f, 490.0f / 512.0f, 248.0f / 512.0f, 20.0f / 512.0f);

	// get EnergyBarPos
	g_pDrawPrim->SetXYWH(&m_EnergyBar, m_EnergyBG.verts[0].x + 28, m_EnergyBG.verts[0].y + 5, (float)m_iEnergyWidth, 10);

	g_pDrawPrim->SetXYWH(&m_EnergyTransfer, m_EnergyBG.verts[0].x + 28, m_EnergyBG.verts[0].y + 5, (float)m_iEnergyWidth, 10);

	// get EnergyBarTexCoords
	g_pDrawPrim->SetUVWH(&m_EnergyBar, 349.0f / 512.0f, 454.0f / 512.0f, 160.0f / 512.0f, 10.0f / 512.0f);

	CUIFont * pFont = g_pInterfaceResMgr->GetFont(m_iFontNum);
	float x = m_HealthBG.verts[0].x + 27;
	float y = m_HealthBG.verts[0].y;
	if (m_pHealthStr)
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pHealthStr);
	m_pHealthStr = g_pLTClient->GetFontManager()->CreatePolyString(pFont, "100", x, y);
	m_pHealthStr->SetColor(0xC0FFFFFF);
	x = m_EnergyBG.verts[0].x + 200;
	y = m_EnergyBG.verts[0].y;
	if (m_pEnergyStr)
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pEnergyStr);
	m_pEnergyStr = g_pLTClient->GetFontManager()->CreatePolyString(pFont, "100", x, y);
	m_pEnergyStr->SetColor(0xC0FFFFFF);
}

