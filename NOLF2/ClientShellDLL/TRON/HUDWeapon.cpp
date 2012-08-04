// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeapon.cpp
//
// PURPOSE : HUDItem to display Jet's current Weapon
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "HUDWeapon.h"
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"


//******************************************************************************************
//**
//** HUD Weapon display
//**
//******************************************************************************************

CHUDWeapon::CHUDWeapon()
{
	// Make this something new
	m_UpdateFlags = kHUDWeapons;

	m_hHUDTex = LTNULL;
	m_hWeaponTex = LTNULL;

	m_fScale = 0.0f;

	m_nWeaponID = 0;
}

LTBOOL CHUDWeapon::Init()
{
	UpdateLayout();

	m_rcBackground.Init(1, 364, 47, 406);
	m_rcAmmo.Init(101,374, 104, 413);

	// Load textures here
	m_hHUDTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\hud.dtx");

	g_pDrawPrim->SetRGBA(&m_BasePrim, argbWhite);
	SetUVWH(&m_BasePrim, m_rcBackground );

	g_pDrawPrim->SetRGBA(&m_AmmoPrim, argbWhite);
	SetUVWH(&m_AmmoPrim, m_rcAmmo);

	g_pDrawPrim->SetRGBA(&m_WeaponPrim, argbWhite);
	g_pDrawPrim->SetUVWH(&m_WeaponPrim, 0.0f, 0.0f, 1.0f, 1.0f);

	return LTTRUE;
}

void CHUDWeapon::Term()
{
	// nothing to do here
}

void CHUDWeapon::Render()
{
	SetRenderState();

	// Render the base
	g_pDrawPrim->SetTexture(m_hHUDTex);
	g_pDrawPrim->DrawPrim(&m_BasePrim);

	if (m_hWeaponTex)
	{
		g_pDrawPrim->SetTexture(m_hWeaponTex);
		g_pDrawPrim->DrawPrim(&m_WeaponPrim);
	}
}

void CHUDWeapon::Update()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}

	// Query for the correct weapon
	uint8 nWeaponID = g_pTronPlayerStats->GetCurWeapon();
	// different weapon?
	if (m_nWeaponID != nWeaponID)
	{
		m_nWeaponID = nWeaponID;
		m_hWeaponTex = LTNULL;
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponID);
		if (pWeapon)
		{
			std::string icon = pWeapon->GetNormalIcon();
			{
				m_hWeaponTex = g_pInterfaceResMgr->GetTexture( icon.c_str() );
			}
		}
	}
	// query for the correct amount of ammo and adjust the ammo prim
}

void CHUDWeapon::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	// Position the weapon indicator in the top-right corner

	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	float x = (640.0f * xs) - 40.0f;
	float y = 0.0f;

	float w = 40.0f;
	float h = 46.0f;

	g_pDrawPrim->SetXYWH( &m_BasePrim, x, y, w, h);
	g_pDrawPrim->SetXYWH( &m_AmmoPrim, x, y, 4, h);
//( 7, 2, 42, 37 )
	g_pDrawPrim->SetXYWH( &m_WeaponPrim, x+6.0f, y+2.0f, 32.0f, 32.0f);
}

void CHUDWeapon::SetUVWH(LT_POLYFT4 * pPrim, LTRect rect)
{
	rect.right -= rect.left;
	rect.bottom -= rect.top;

	g_pDrawPrim->SetUVWH(pPrim, rect.left / 512.0f, rect.top / 512.0f, rect.right / 512.0f, rect.bottom / 512.0f);
}
