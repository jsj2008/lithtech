// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAmmo.cpp
//
// PURPOSE : HUDItem to display player ammo
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDAmmo.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "ClientWeaponMgr.h"
#include "PlayerMgr.h"

VarTrack	g_vtHUDAmmoRender;

//******************************************************************************************
//**
//** HUD Ammo display
//**
//******************************************************************************************

CHUDAmmo::CHUDAmmo()
{
	m_UpdateFlags = kHUDAmmo;
	m_bDraw = false;
	m_bInfinite = false;
	m_hLastAmmo = NULL;
}

bool CHUDAmmo::Init()
{
	g_vtHUDAmmoRender.Init( g_pLTClient, "HUDAmmoRender", NULL, 1.0f );

	UpdateLayout();

	m_Text.SetDropShadow(2);

	ScaleChanged();

	SetSourceString(LoadString("HUD_Ammo_Chars"));

	return true;
}

void CHUDAmmo::Term()
{
}

void CHUDAmmo::Render()
{
	if( g_vtHUDAmmoRender.GetFloat( ) < 1.0f )
		return;

	bool bWeaponsEnabled = g_pClientWeaponMgr->WeaponsEnabled();
	CClientWeapon* pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (!pClientWeapon || !bWeaponsEnabled) return;

	if (!m_bDraw || pClientWeapon->GetState() == W_DESELECT) return;

	SetRenderState();

	if (!m_bInfinite)
	{
		m_Text.Render();
	}

	//render icon here
	if (m_hIconTexture)
	{
		g_pDrawPrim->SetTexture(m_hIconTexture);
		g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	}
}

void CHUDAmmo::Update()
{

	HWEAPON hWeapon = g_pPlayerStats->GetCurrentWeaponRecord();
	HAMMO hAmmo = g_pPlayerStats->GetCurrentAmmoRecord();

	m_bDraw = (hWeapon && hAmmo);

	if (hAmmo)
	{
		DamageType dtAmmoInstDamageType = g_pWeaponDB->GetAmmoInstDamageType( hAmmo);
		if (dtAmmoInstDamageType == DT_MELEE)
			m_bDraw = false;
	}

	if (!m_bDraw) return;

	CClientWeapon* pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon( );
	int nAmmoInClip = pClientWeapon ? pClientWeapon->GetAmmoInClips() : 0;
	int nAmmo = g_pPlayerStats->GetCurrentAmmoCount() - nAmmoInClip;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	m_bInfinite = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo );

	if (!m_bInfinite)
	{
		wchar_t wstr[32];
		FormatString("HUD_Ammo_Format",wstr,LTARRAYSIZE(wstr), nAmmoInClip, nAmmo < 0 ? 0 : nAmmo);

		if ((nAmmo > 0) || (nAmmoInClip > 0))
		{
			EnableFade(true);
		}
		else
		{
			ResetFade();
			EnableFade( false );
		}
		

		m_Text.SetText(wstr);
	}

	if (m_hLastAmmo != hAmmo)
	{
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo);
		m_hIconTexture.Load( g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sIcon ) );
		SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f);
		m_hLastAmmo = hAmmo;
	}

	ResetFade();

}


void CHUDAmmo::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDAmmo");
	}
	
	CHUDItem::UpdateLayout();

}
