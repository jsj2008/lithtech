// ----------------------------------------------------------------------- //
//
// MODULE  : HUDGrenade.cpp
//
// PURPOSE : HUDItem to display player grenade
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDGrenade.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "ClientWeaponMgr.h"
#include "PlayerMgr.h"

VarTrack g_vtHUDGrenadeRender;

//******************************************************************************************
//**
//** HUD Grenade display
//**
//******************************************************************************************

CHUDGrenade::CHUDGrenade()
{
	m_UpdateFlags = kHUDGrenade;
	m_bDraw = false;
	m_bInfinite = false;
	m_nCount = ( uint32 )-1;
}

bool CHUDGrenade::Init()
{
	g_vtHUDGrenadeRender.Init( g_pLTClient, "HUDGrenadeRender", NULL, 1.0f );

	UpdateLayout();
	ScaleChanged();

	SetSourceString( LoadString("HUD_Number_Chars"));


	m_Text.SetDropShadow(2);
	EnableFade( true );

	return true;
}

void CHUDGrenade::Term()
{
}

void CHUDGrenade::Render()
{
	if( g_vtHUDGrenadeRender.GetFloat( ) < 1.0f )
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
		SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	}

	if (m_hBarTexture)
	{
		g_pDrawPrim->SetTexture(m_hBarTexture);
		g_pDrawPrim->DrawPrim(&m_BarPoly,1);
	}

}

void CHUDGrenade::Update()
{

	HWEAPON hWeapon = g_pPlayerStats->GetCurrentGrenadeRecord();
	HAMMO hAmmo = NULL;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	if (hWeapon)
		hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName);

	m_bDraw = (hWeapon && hAmmo);

	if (!m_bDraw) return;

	m_bInfinite = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo );
	uint32 nCount = g_pPlayerStats->GetAmmoCount(hAmmo);
	if (nCount != m_nCount)
	{
		//flash if we pickup a weapon
		if (nCount > m_nCount)
		{
			Flash("Pickup");
		}


		m_nCount = nCount;
//		EnableFade( (m_nCount > 0) );

		if (!m_bInfinite)
		{
			wchar_t wstr[32];
			swprintf(wstr,L"%d", m_nCount );

			m_Text.SetText(wstr);
		}
		UpdateBar();


	}
	ResetFade();

	m_hIconTexture.Load( g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sIcon ) );

}

void CHUDGrenade::ScaleChanged()
{
	CHUDItem::ScaleChanged();
	UpdateBar();
}


void CHUDGrenade::UpdateBar()
{
	HWEAPON hWeapon = g_pPlayerStats->GetCurrentGrenadeRecord();
	HAMMO hAmmo = NULL;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	if (hWeapon)
		hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName);

	if (!hAmmo) return;

	float fPercent = 1.0f;
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo);
	uint32 maxAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nMaxAmount );

	if (!g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ) && maxAmmo > 0)
	{
		fPercent = float(m_nCount) / float(maxAmmo);
	}

	fPercent *= m_fBarWidth;
	SetupQuadUVs(m_BarPoly, m_hBarTexture, 0.0f, 0.0f, fPercent, 1.0f);

	float x = (float)(m_vBasePos.x * g_pInterfaceResMgr->GetXRatio()) + float(m_vIconOffset.x);
	float y = (float)(m_vBasePos.y * g_pInterfaceResMgr->GetYRatio()) + float(m_vIconOffset.y);

	float fPartialWidth = fPercent * float(m_vIconSize.x);
	DrawPrimSetXYWH(m_BarPoly,x,y, fPartialWidth ,float(m_vIconSize.y));
}




void CHUDGrenade::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDGrenade");
	}

	m_hBarTexture.Load(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,0));
	m_fBarWidth = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);

	CHUDItem::UpdateLayout();

}

void CHUDGrenade::UpdateFade()
{
	CHUDItem::UpdateFade();
	if (FadeLevelChanged())
	{
		float	fFade = GetFadeLevel();
		if (fFade < 1.0f)
		{
			DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
		}
		else
		{
			DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
		}
	}
}

void CHUDGrenade::UpdateFlicker()
{
	CHUDItem::UpdateFlicker();
	if (m_fFlicker > 0.0f)
	{
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,m_fFlicker));
	}
	else
	{
		DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
	}
}

void CHUDGrenade::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	CHUDItem::UpdateFlash();
	//still flashing, is the flash on or off?
	if (m_FlashTimer.IsStarted() && m_FlashTimer.GetTimeLeft() < (m_FlashTimer.GetDuration() / 2.0f))
	{
		DrawPrimSetRGBA(m_BarPoly,m_cFlashColor);
	}
	else
	{
		float fFade = GetFadeLevel();
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
	}
}


void CHUDGrenade::EndFlicker()
{
	CHUDItem::EndFlicker();
	float	fFade = GetFadeLevel();
	if (fFade < 1.0f)
	{
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
	}
	else
	{
		DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
	}
}

