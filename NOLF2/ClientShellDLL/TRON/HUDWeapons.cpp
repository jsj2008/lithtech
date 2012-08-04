// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeapons.cpp
//
// PURPOSE : HUDItem to display player air meter
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "HUDWeapons.h"
#include "ProfileMgr.h"
#include "TronInterfaceMgr.h"


//******************************************************************************************
//**
//** HUD Air display
//**
//******************************************************************************************

CHUDWeapons::CHUDWeapons()
{
	m_UpdateFlags = kHUDWeapons;
	memset(m_hIcon,0,sizeof(m_hIcon));
	m_bDraw = LTTRUE;

	m_fAlpha = 1.0f;
	m_bLargeIcons = LTFALSE;
	m_bNumbers = LTTRUE;

}

CHUDWeapons::~CHUDWeapons()
{
}


LTBOOL CHUDWeapons::Init()
{

	for( int i = 0; i < 10; ++i )
	{
		g_pDrawPrim->SetRGBA( &m_Poly[i], argbWhite );
		g_pDrawPrim->SetUVWH( &m_Poly[i], 0.0f, 0.0f, 1.0f, 1.0f );
	}

	UpdateLayout();

	return LTTRUE;
}


void CHUDWeapons::Render()
{
	float fAlpha = GetConsoleFloat("BindingIconAlpha",0.7f);
	LTBOOL bLarge = (GetConsoleInt("BindingIconSize",0) > 0);
	LTBOOL bNum   = (GetConsoleInt("BindingNumbers",1) > 0);

	if (fAlpha != m_fAlpha || bLarge != m_bLargeIcons || bNum != m_bNumbers)
	{
		m_fAlpha = fAlpha;
		m_bLargeIcons = bLarge;
		m_bNumbers = bNum;
		Update();
	}
	if (!m_bDraw) return;

	SetRenderState();

	char szTmp[8] = "";
	for( int i = 0; i < 10; ++i )
	{
		g_pDrawPrim->SetTexture( m_hIcon[i] );
		g_pDrawPrim->DrawPrim( &m_Poly[i], 1 );

		sprintf(szTmp,"%d",(i+1%10));
		m_pText->SetText(szTmp);
		m_pText->SetPosition(m_Poly[i].verts[0].x + 2.0f,m_Poly[i].verts[0].y + 2.0f);
		m_pText->Render();


	}
}


void CHUDWeapons::Update()
{
	ASSERT( 0 != g_pProfileMgr );
	ASSERT( 0 != g_pInterfaceResMgr );
	ASSERT( 0 != g_pWeaponMgr );
	ASSERT( 0 != g_pPlayerStats );
	ASSERT( 0 != g_pDrawPrim );

	UpdateStyle();
	if (!m_bDraw) return;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	float sz = (float)m_nIconSize;
	float ix = 640.0f * g_pInterfaceResMgr->GetXRatio() - sz;
	float ox = 640.0f * g_pInterfaceResMgr->GetXRatio() + sz;
	float y = 240.0f * g_pInterfaceResMgr->GetYRatio() - (5.0f * sz);
	for( int i = 0; i < 10; ++i )
	{
		// Get the weapon id
		uint8 nWeaponId =  g_pWeaponMgr->GetWeaponId(pProfile->m_nWpnActions[i]);
		if ( WMGR_INVALID_ID != nWeaponId  )
		{
			// Get the weapon structure
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
			if ( LTNULL != pWeapon )
			{
				if (g_pPlayerStats->HaveWeapon(nWeaponId))
					g_pDrawPrim->SetXYWH( &m_Poly[i], ix, y, sz, sz);
				else
					g_pDrawPrim->SetXYWH( &m_Poly[i], ox, y, sz, sz);
				
				std::string icon = pWeapon->GetNormalIcon();
				m_hIcon[i] = g_pInterfaceResMgr->GetTexture(icon.c_str());
			}
		}
		
		// update the y position
		y += sz;
	}


}

void CHUDWeapons::UpdateLayout()
{
	char *pTag = "HUDWeapons";

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
	m_pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_nFontSize = m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");
	m_nLargeFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"LargeFontSize");

	m_pText = g_pFontManager->CreateFormattedPolyString(m_pFont," ",0.0f,0.0f);
	if (!m_pText)
		return;

	m_nIconSize = m_nBaseIconSize = (uint8)g_pLayoutMgr->GetInt(pTag,"IconSize");
	m_nLargeIconSize = (uint8)g_pLayoutMgr->GetInt(pTag,"LargeIconSize");

	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_nTextColor = SET_ARGB(0xFF,nR,nG,nB);

	UpdateStyle();


}

void CHUDWeapons::UpdateStyle()
{
	if (m_fAlpha < 0.1f)
	{
		m_bDraw = LTFALSE;
		return;
	}

	if (m_bLargeIcons)
	{
		m_nIconSize = (uint8)(g_pInterfaceResMgr->GetXRatio() * (float)m_nLargeIconSize);
		m_nFontSize = (uint8)(g_pInterfaceResMgr->GetXRatio() * (float)m_nLargeFontSize);
	}
	else
	{
		m_nIconSize = (uint8)(g_pInterfaceResMgr->GetXRatio() * (float)m_nBaseIconSize);
		m_nFontSize = (uint8)(g_pInterfaceResMgr->GetXRatio() * (float)m_nBaseFontSize);
	}


	uint8 nA = (uint8)(255.0f * m_fAlpha);

	uint32 nColor = SET_ARGB(nA,0xFF,0xFF,0xFF);

	uint32 a,nR,nG,nB;
	GET_ARGB(m_nTextColor,a,nR,nG,nB);
	m_nTextColor = SET_ARGB(nA,nR,nG,nB);

	for( int i = 0; i < 10; ++i )
	{
		g_pDrawPrim->SetRGBA( &m_Poly[i], nColor );
	}

	m_pText->SetColor(m_nTextColor);
	m_pText->SetCharScreenHeight(m_nFontSize);

}
