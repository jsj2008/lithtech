// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAmmo.cpp
//
// PURPOSE : HUDItem to display player ammo
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "PlayerMgr.h"

//******************************************************************************************
//**
//** HUD Ammo display
//**
//******************************************************************************************

CHUDAmmo::CHUDAmmo()
{
	m_UpdateFlags = kHUDAmmo;
	m_bDraw = LTFALSE;
	m_bInfinite = LTFALSE;
}

LTBOOL CHUDAmmo::Init()
{
	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pStr = g_pFontManager->CreatePolyString(pFont,"",0.0f, 0.0f);

	UpdateLayout();

	//armor bar
	m_Bar.Init(g_pInterfaceResMgr->GetTexture("interface\\hud\\ammobar.dtx"));

	m_pStr->SetColor(m_TextColor);

	m_hFull = g_pInterfaceResMgr->GetTexture("interface\\hud\\ammo_full.dtx");
	m_hEmpty = g_pInterfaceResMgr->GetTexture("interface\\hud\\ammo_empty.dtx");

	uint32 a,r,g,b, fadeColor;
	GET_ARGB(m_TextColor,a,r,g,b);

	fadeColor = SET_ARGB(a,0xFF,0xFF,0xFF);
	g_pDrawPrim->SetRGBA(&m_Poly[0],fadeColor);
	g_pDrawPrim->SetRGBA(&m_Poly[1],fadeColor);

	return LTTRUE;
}

void CHUDAmmo::Term()
{
 	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}
}

void CHUDAmmo::Render()
{
	bool bWeaponsEnabled = g_pPlayerMgr->GetClientWeaponMgr()->WeaponsEnabled();
	IClientWeaponBase* pClientWeapon =
	                g_pPlayerMgr->GetCurrentClientWeapon();
	if (!pClientWeapon || !bWeaponsEnabled) return;

	if (!m_bDraw || pClientWeapon->GetState() == W_DESELECT) return;


	SetRenderState();

	if (m_bUseBar && !m_bInfinite)
	{
		g_pDrawPrim->BeginDrawPrim();

		m_Bar.Render();


		float x = (float)(m_BasePos.x + m_ClipOffset.x) * g_pInterfaceResMgr->GetXRatio();
		float y = (float)(m_BasePos.y + m_ClipOffset.y) * g_pInterfaceResMgr->GetYRatio();

		float w = (float)m_ClipUnitSize.x * g_pInterfaceResMgr->GetXRatio();
		float h = (float)m_ClipUnitSize.y * g_pInterfaceResMgr->GetYRatio();

		g_pDrawPrim->SetTexture(m_hFull);
		SetupQuadUVs(m_Poly[0], m_hFull, 0.0f, 0.0f, 1.0f, 1.0f);

		for (uint8 i = 0; i < m_nFull; i++)
		{
			x -= w;
			g_pDrawPrim->SetXYWH(&m_Poly[0],x,y,w,h);
			g_pDrawPrim->DrawPrim(&m_Poly[0],1);
		}
		g_pDrawPrim->SetTexture(m_hEmpty);
		SetupQuadUVs(m_Poly[0], m_hEmpty, 0.0f, 0.0f, 1.0f, 1.0f);

		for (uint8 i = 0; i < m_nEmpty; i++)
		{
			x -= w;			
			g_pDrawPrim->SetXYWH(&m_Poly[0],x,y,w,h);
			g_pDrawPrim->DrawPrim(&m_Poly[0],1);
		}

		g_pDrawPrim->EndDrawPrim();

	}

	if (m_bUseText &&  !m_bInfinite)
	{
		m_pStr->Render();
	}

	//render icon here
	if (m_hIcon)
	{
		g_pDrawPrim->SetTexture(m_hIcon);
		SetupQuadUVs(m_Poly[1], m_hIcon, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->DrawPrim(&m_Poly[1],1);
	}
}

void CHUDAmmo::Update()
{

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(g_pPlayerStats->GetCurrentWeapon());
	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(g_pPlayerStats->GetCurrentAmmo());

	m_bDraw = (pWeapon && pAmmo);

	if (pAmmo && (pAmmo->eInstDamageType == DT_MELEE || pAmmo->eInstDamageType == DT_SWORD) )
		m_bDraw = LTFALSE;

	if (!m_bDraw) return;

	IClientWeaponBase* pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon( );
	int nAmmoInClip = pClientWeapon ? pClientWeapon->GetAmmoInClip() : 0;
	int nAmmo = g_pPlayerStats->GetCurrentAmmoCount() - nAmmoInClip;
	m_nFull = (uint8)nAmmoInClip;
	m_nEmpty = (uint8)pWeapon->nShotsPerClip - m_nFull;

	m_bInfinite = pWeapon->bInfiniteAmmo;

	if (m_bUseBar && !m_bInfinite)
	{
		LTFLOAT fPercent = 100.0f * (LTFLOAT) (nAmmo + nAmmoInClip) /  (LTFLOAT) pAmmo->GetMaxAmount(LTNULL);

  		float x = (float)(m_BasePos.x + m_BarOffset.x) * g_pInterfaceResMgr->GetXRatio();
  		float y = (float)(m_BasePos.y + m_BarOffset.y) * g_pInterfaceResMgr->GetYRatio();
  		float h = (float)m_nBarHeight * g_pInterfaceResMgr->GetYRatio();
  
  		float w = fPercent * m_fBarScale * g_pInterfaceResMgr->GetXRatio();
		float maxW = 100.0f * m_fBarScale * g_pInterfaceResMgr->GetXRatio();

		m_Bar.Update(x,y,w,maxW,h);

	}

	if (m_bUseText && !m_bInfinite)
	{
		char str[32];
		sprintf(str,"%d/%d", nAmmoInClip, nAmmo < 0 ? 0 : nAmmo);

		uint8 h = (uint8)((float)m_nTextHeight * g_pInterfaceResMgr->GetYRatio());

		float x = (float)(m_BasePos.x + m_TextOffset.x) * g_pInterfaceResMgr->GetXRatio();
		float y = (float)(m_BasePos.y + m_TextOffset.y) * g_pInterfaceResMgr->GetYRatio();

		m_pStr->SetText(str);
		m_pStr->SetPosition(x,y);
		m_pStr->SetCharScreenHeight(h);

		
	}


	float x = (float)(m_BasePos.x + m_IconOffset.x) * g_pInterfaceResMgr->GetXRatio();
	float y = (float)(m_BasePos.y + m_IconOffset.y) * g_pInterfaceResMgr->GetYRatio();
	float w = (float)m_nIconSize * g_pInterfaceResMgr->GetXRatio();
	float h = (float)m_nIconSize * g_pInterfaceResMgr->GetYRatio();
	std::string icon = pAmmo->GetNormalIcon();
	m_hIcon = g_pInterfaceResMgr->GetTexture(icon.c_str());
	g_pDrawPrim->SetXYWH(&m_Poly[1],x,y,w,h);

}

void CHUDAmmo::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetAmmoBasePos(nCurrentLayout);

	m_bUseBar		= g_pLayoutMgr->GetUseAmmoBar(nCurrentLayout);
	m_BarOffset		= g_pLayoutMgr->GetAmmoBarOffset(nCurrentLayout);
    m_ClipOffset	= g_pLayoutMgr->GetAmmoClipOffset(nCurrentLayout);
    m_ClipUnitSize	= g_pLayoutMgr->GetAmmoClipUnitSize(nCurrentLayout);

	m_bUseText		= g_pLayoutMgr->GetUseAmmoText(nCurrentLayout);
	m_TextOffset	= g_pLayoutMgr->GetAmmoTextOffset(nCurrentLayout);
    
	m_IconOffset	= g_pLayoutMgr->GetAmmoIconOffset(nCurrentLayout);
	m_nIconSize		= g_pLayoutMgr->GetAmmoIconSize(nCurrentLayout);
	m_TextColor		= g_pLayoutMgr->GetAmmoColor(nCurrentLayout);

	m_nBarHeight		= g_pLayoutMgr->GetBarHeight(nCurrentLayout);
	m_nTextHeight		= g_pLayoutMgr->GetTextHeight(nCurrentLayout);
	m_fBarScale			= g_pLayoutMgr->GetBarScale(nCurrentLayout);

}
