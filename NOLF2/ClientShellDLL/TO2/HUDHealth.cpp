// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHealth.cpp
//
// PURPOSE : HUDItem to display player health
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "HUDHealth.h"

extern CGameClientShell* g_pGameClientShell;

//******************************************************************************************
//**
//** HUD Health display
//**
//******************************************************************************************

CHUDHealth::CHUDHealth()
{
	m_UpdateFlags = kHUDHealth | kHUDArmor;
	m_hHealth = LTNULL;
	m_hArmor = LTNULL;
}

LTBOOL CHUDHealth::Init()
{

	//health icon
	m_hHealth = g_pInterfaceResMgr->GetTexture("interface\\hud\\icon_health.dtx");
	g_pDrawPrim->SetRGBA(&m_Poly[0],argbWhite);
	SetupQuadUVs(m_Poly[0], m_hHealth, 0.0f,0.0f,1.0f,1.0f);

	//armor icon
	m_hArmor = g_pInterfaceResMgr->GetTexture("interface\\hud\\icon_armor.dtx");
	g_pDrawPrim->SetRGBA(&m_Poly[1],argbWhite);
	SetupQuadUVs(m_Poly[1], m_hArmor, 0.0f,0.0f,1.0f,1.0f);


	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pHealthStr = g_pFontManager->CreatePolyString(pFont,"",0.0f, 0.0f);
	m_pArmorStr = g_pFontManager->CreatePolyString(pFont,"",0.0f, 0.0f);


	UpdateLayout();

	//health bar
	m_HealthBar.Init(g_pInterfaceResMgr->GetTexture("interface\\hud\\healthbar.dtx"));

	//armor bar
	m_ArmorBar.Init(g_pInterfaceResMgr->GetTexture("interface\\hud\\armorbar.dtx"));

	m_pHealthStr->SetColor(m_HealthColor);
	m_pArmorStr->SetColor(m_ArmorColor);

	return LTTRUE;
}

void CHUDHealth::Term()
{
	if (m_pHealthStr)
	{
		g_pFontManager->DestroyPolyString(m_pHealthStr);
        m_pHealthStr=LTNULL;
	}
	if (m_pArmorStr)
	{
		g_pFontManager->DestroyPolyString(m_pArmorStr);
        m_pArmorStr=LTNULL;
	}

}

void CHUDHealth::Render()
{
	SetRenderState();

	if (m_bUseHealthBar)
	{
		m_HealthBar.Render();
		m_ArmorBar.Render();


	}

	if (m_bUseHealthText)
	{
		m_pHealthStr->Render();
		m_pArmorStr->Render();
	}

	if (m_bUseHealthIcon)
	{
		// draw our icons
		g_pDrawPrim->SetTexture(m_hHealth);
		g_pDrawPrim->DrawPrim(&m_Poly[0]);
		g_pDrawPrim->SetTexture(m_hArmor);
		g_pDrawPrim->DrawPrim(&m_Poly[1]);
	}

}

void CHUDHealth::Update()
{
	if (m_bUseHealthBar)
	{
		float x = (float)(m_HealthBasePos.x + m_HealthBarOffset.x) * g_pInterfaceResMgr->GetXRatio();
		float y = (float)(m_HealthBasePos.y + m_HealthBarOffset.y) * g_pInterfaceResMgr->GetYRatio();

		float w = (float)g_pPlayerStats->GetHealth() * m_fBarScale * g_pInterfaceResMgr->GetXRatio();
		float h = (float)m_nBarHeight * g_pInterfaceResMgr->GetYRatio();
		float maxW = (float)g_pPlayerStats->GetMaxHealth() * m_fBarScale * g_pInterfaceResMgr->GetXRatio();

		m_HealthBar.Update(x,y,w,maxW,h);


		x = (float)(m_HealthBasePos.x + m_ArmorBarOffset.x) * g_pInterfaceResMgr->GetXRatio();
		y = (float)(m_HealthBasePos.y + m_ArmorBarOffset.y) * g_pInterfaceResMgr->GetYRatio();

		w = (float)g_pPlayerStats->GetArmor() * m_fBarScale * g_pInterfaceResMgr->GetXRatio();
		maxW = (float)g_pPlayerStats->GetMaxArmor() * m_fBarScale * g_pInterfaceResMgr->GetXRatio();

		m_ArmorBar.Update(x,y,w,maxW,h);

	}

	if (m_bUseHealthText)
	{
		float x = (float)(m_HealthBasePos.x + m_HealthTextOffset.x) * g_pInterfaceResMgr->GetXRatio();
		float y = (float)(m_HealthBasePos.y + m_HealthTextOffset.y) * g_pInterfaceResMgr->GetYRatio();

		uint8 h = (uint8)((float)m_nTextHeight * g_pInterfaceResMgr->GetYRatio());

		m_pHealthStr->SetPosition(x,y);
		m_pHealthStr->SetCharScreenHeight(h);
		char szTmp[16] = "";
		sprintf(szTmp,"%d",g_pPlayerStats->GetHealth());
		m_pHealthStr->SetText(szTmp);

		x = (float)(m_HealthBasePos.x + m_ArmorTextOffset.x) * g_pInterfaceResMgr->GetXRatio();
		y = (float)(m_HealthBasePos.y + m_ArmorTextOffset.y) * g_pInterfaceResMgr->GetYRatio();

		m_pArmorStr->SetPosition(x,y);
		m_pArmorStr->SetCharScreenHeight(h);
		sprintf(szTmp,"%d",g_pPlayerStats->GetArmor());
		m_pArmorStr->SetText(szTmp);

	}

	if (m_bUseHealthIcon)
	{
		float x = (float)(m_HealthBasePos.x + m_HealthIconOffset.x) * g_pInterfaceResMgr->GetXRatio();
		float y = (float)(m_HealthBasePos.y + m_HealthIconOffset.y) * g_pInterfaceResMgr->GetYRatio();

		float w = (float)m_nHealthIconSize * g_pInterfaceResMgr->GetYRatio();

		g_pDrawPrim->SetXYWH(&m_Poly[0],x,y,w,w);

		x = (float)(m_HealthBasePos.x + m_ArmorIconOffset.x) * g_pInterfaceResMgr->GetXRatio();
		y = (float)(m_HealthBasePos.y + m_ArmorIconOffset.y) * g_pInterfaceResMgr->GetYRatio();

		g_pDrawPrim->SetXYWH(&m_Poly[1],x,y,w,w);

	}

}

void CHUDHealth::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_HealthBasePos		= g_pLayoutMgr->GetHealthBasePos(nCurrentLayout);

	m_bUseHealthBar		= g_pLayoutMgr->GetUseHealthBar(nCurrentLayout);
	m_HealthBarOffset	= g_pLayoutMgr->GetHealthBarOffset(nCurrentLayout);
	m_ArmorBarOffset	= g_pLayoutMgr->GetArmorBarOffset(nCurrentLayout);

	m_bUseHealthText	= g_pLayoutMgr->GetUseHealthText(nCurrentLayout);
	m_HealthTextOffset	= g_pLayoutMgr->GetHealthTextOffset(nCurrentLayout);
	m_ArmorTextOffset	= g_pLayoutMgr->GetArmorTextOffset(nCurrentLayout);

	m_bUseHealthIcon	= g_pLayoutMgr->GetUseHealthIcon(nCurrentLayout);
	m_HealthIconOffset	= g_pLayoutMgr->GetHealthIconOffset(nCurrentLayout);
	m_ArmorIconOffset	= g_pLayoutMgr->GetArmorIconOffset(nCurrentLayout);
	m_nHealthIconSize	= g_pLayoutMgr->GetHealthIconSize(nCurrentLayout);

	m_nBarHeight		= g_pLayoutMgr->GetBarHeight(nCurrentLayout);
	m_nTextHeight		= g_pLayoutMgr->GetTextHeight(nCurrentLayout);
	m_fBarScale			= g_pLayoutMgr->GetBarScale(nCurrentLayout);

	m_HealthColor		= g_pLayoutMgr->GetHealthColor(nCurrentLayout);
	m_ArmorColor		= g_pLayoutMgr->GetArmorColor(nCurrentLayout);

}
