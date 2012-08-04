// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeapon.cpp
//
// PURPOSE : HUD element to display a weapon item
//
// CREATED : 12/17/03
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDWeapon.h"
#include "CommandIDs.h"

VarTrack g_vtHUDWeaponRender;


// ----------------------------------------------------------------------- //
// Constructor
// ----------------------------------------------------------------------- //
CHUDWeapon::CHUDWeapon() :
	m_bSelected(false),
	m_hWeapon(NULL)
{
}

// ----------------------------------------------------------------------- //
// initialize item
// ----------------------------------------------------------------------- //
bool CHUDWeapon::Init()
{
	g_vtHUDWeaponRender.Init( g_pLTClient, "HUDWeaponRender", NULL, 1.0f );

	UpdateLayout();

	SetSourceString( LoadString("HUD_Number_Chars"));

	m_bSelected = false;
	m_nCount = -1;
	EnableFade( true );

	return true;
}


// ----------------------------------------------------------------------- //
// Associate this item with a particular weapon record
// ----------------------------------------------------------------------- //
void CHUDWeapon::SetWeaponRecord(HWEAPON hWeapon)
{
	//flash if we pickup a weapon
	if (hWeapon && !m_hWeapon)
	{
		Flash("Pickup");
	}

	m_hWeapon = hWeapon;
	m_bDraw = true;
	if (m_hWeapon)
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
		m_hIconTexture.Load( g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sIcon ) );
	}
	else
	{
		m_hIconTexture.Load( g_pLayoutDB->GetString(m_hLayout,"IconTexture") );
		Select(false);
	}

	SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f);
	m_nCount = -1;
	Update();
}

// ----------------------------------------------------------------------- //
// Update values
// ----------------------------------------------------------------------- //
void CHUDWeapon::Update()
{
	if (!m_hWeapon)
	{
		m_Text.SetText(L"");
		m_cIconColor = m_cDisabledColor;
		UpdateBar();

		return;
	}
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
	HAMMO hDefaultAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );

	uint32 nCount = g_pPlayerStats->GetAmmoCount(hDefaultAmmo);
	

	//if the number of items has changed
	if (nCount != m_nCount)
	{
		if (nCount > m_nCount)
		{
			ResetFade();
		}			

		m_nCount = nCount;

		//update our text
		wchar_t wstr[32];
		swprintf(wstr,L"%d", m_nCount );
		m_Text.SetText(wstr);

		if (nCount > 0)
		{
			m_cIconColor = m_cNormalColor;
		}
		else
		{
			m_cIconColor = m_cDisabledColor;
		}

		UpdateBar();

	}

}

void CHUDWeapon::UpdateFade()
{
	CHUDItem::UpdateFade();
	if (FadeLevelChanged())
	{
		float	fFade = GetFadeLevel();
		if (fFade < 1.0f)
		{
			m_Hotkey.SetColor(FadeARGB(m_cHotkeyColor,fFade));
			DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
		}
		else
		{
			m_Hotkey.SetColor(m_cHotkeyColor);
			DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
		}
	}
}

void CHUDWeapon::UpdateFlicker()
{
	CHUDItem::UpdateFlicker();
	if (m_fFlicker > 0.0f)
	{
		m_Hotkey.SetColor(FadeARGB(m_cHotkeyColor,m_fFlicker));
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,m_fFlicker));
	}
	else
	{
		m_Hotkey.SetColor(m_cHotkeyColor);
		DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
	}
}

void CHUDWeapon::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	CHUDItem::UpdateFlash();
	//still flashing, is the flash on or off?
	if (m_FlashTimer.IsStarted() && m_FlashTimer.GetTimeLeft() < (m_FlashTimer.GetDuration() / 2.0f))
	{
		m_Hotkey.SetColor(m_cFlashColor);
		DrawPrimSetRGBA(m_BarPoly,m_cFlashColor);
	}
	else
	{
		float fFade = GetFadeLevel();
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));

		m_Hotkey.SetColor(FadeARGB(m_cHotkeyColor,fFade));
	}
}


void CHUDWeapon::EndFlicker()
{
	CHUDItem::EndFlicker();
	float	fFade = GetFadeLevel();
	if (fFade < 1.0f)
	{
		m_Hotkey.SetColor(FadeARGB(m_cHotkeyColor,fFade));
		DrawPrimSetRGBA(m_BarPoly,FadeARGB(m_cIconColor,fFade));
	}
	else
	{
		m_Hotkey.SetColor(m_cHotkeyColor);
		DrawPrimSetRGBA(m_BarPoly,m_cIconColor);
	}
}



void CHUDWeapon::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDWeapon");
	}

	CHUDItem::UpdateLayout();


	m_cHotkeyColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);
	m_vHotkeyOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,1);

	m_vSmallSize = m_vIconSize;
	m_vLargeSize = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,2);

	m_vSmallOffset = m_vTextOffset;
	m_vLargeOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,4);

	m_cNormalColor = m_cIconColor;
	m_cDisabledColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,1);

	CFontInfo tmpFont = m_sTextFont;
	tmpFont.m_nStyle = CFontInfo::kStyle_Bold;

	m_Hotkey.SetFont(tmpFont);
	m_Hotkey.SetColor(m_cHotkeyColor);
	m_Hotkey.SetDropShadow(1);
	m_Hotkey.SetText(L" ");

	m_hBarTexture.Load(g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,0));
	m_fBarWidth = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);


}


// ----------------------------------------------------------------------- //
// Position the item on screen
// ----------------------------------------------------------------------- //
void CHUDWeapon::SetBasePos(const LTVector2n& pos)
{
	CHUDInventory::SetBasePos(pos);

	LTVector2 vPos;
	vPos.x = (float)(m_vBasePos.x + m_vHotkeyOffset.x);
	vPos.y = (float)(m_vBasePos.y + m_vHotkeyOffset.y);
	m_Hotkey.SetPos(vPos);

	UpdateBar();

}

// ----------------------------------------------------------------------- //
// Draw the item to the screen
// ----------------------------------------------------------------------- //
void CHUDWeapon::Render()
{
	if( g_vtHUDWeaponRender.GetFloat( ) < 1.0f )
		return;

	float fFade = GetFadeLevel();
	if (!m_bDraw || fFade < MATH_EPSILON) return;

	CHUDInventory::Render();

	if (m_hBarTexture && m_hWeapon)
	{
		g_pDrawPrim->SetTexture(m_hBarTexture);
		g_pDrawPrim->DrawPrim(&m_BarPoly,1);
	}
	m_Hotkey.Render();

}


// ----------------------------------------------------------------------- //
// Update the hotkey display
// ----------------------------------------------------------------------- //
void CHUDWeapon::UpdateTriggerName(const wchar_t* szTrigger)
{
	m_Hotkey.SetText(szTrigger);
}


void CHUDWeapon::Select(bool bSelected)
{
	if (bSelected)
	{
		m_vIconSize = m_vLargeSize;
		m_vTextOffset = m_vLargeOffset;
	}
	else
	{
		m_vIconSize = m_vSmallSize;
		m_vTextOffset = m_vSmallOffset;
	}
	m_bSelected = bSelected;

	float x = (float)(m_vBasePos.x + m_vIconOffset.x);
	float y = (float)(m_vBasePos.y + m_vIconOffset.y);
	DrawPrimSetXYWH(m_IconPoly,x,y,float(m_vIconSize.x),float(m_vIconSize.y));

	LTVector2 vPos;
	vPos.x = float( m_vBasePos.x + m_vTextOffset.x);
	vPos.y = float( m_vBasePos.y + m_vTextOffset.y);
	m_Text.SetPos(vPos);


	UpdateBar();
	ResetFade();
}

void CHUDWeapon::ScaleChanged()
{
	//position handled by CHUDWeaponList
	UpdateBar();
}


void CHUDWeapon::UpdateBar()
{
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
	if (!hWpnData) return;
		
	HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName);
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

	float x = (float)(m_vBasePos.x + m_vIconOffset.x);
	float y = (float)(m_vBasePos.y + m_vIconOffset.y);

	float fPartialWidth = fPercent * float(m_vIconSize.x);
	DrawPrimSetXYWH(m_BarPoly,x,y, fPartialWidth ,float(m_vIconSize.y));
}



