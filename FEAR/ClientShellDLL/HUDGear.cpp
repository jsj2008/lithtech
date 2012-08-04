// ----------------------------------------------------------------------- //
//
// MODULE  : HUDGear.cpp
//
// PURPOSE : HUD Element to display a gear item
//
// CREATED : 12/16/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDGear.h"

VarTrack g_vtHUDGearRender;

// ----------------------------------------------------------------------- //
// Constructor
// ----------------------------------------------------------------------- //
CHUDGear::CHUDGear() :
	m_hGear(NULL)
{
}

// ----------------------------------------------------------------------- //
// initialize item
// ----------------------------------------------------------------------- //
bool CHUDGear::Init()
{
	g_vtHUDGearRender.Init( g_pLTClient, "HUDGearRender", NULL, 1.0f );

	UpdateLayout();

	SetSourceString( LoadString("HUD_Number_Chars"));

	m_nCount = -1;
	m_Text.SetText(L"0");
	EnableFade( true );

	return true;
}


// ----------------------------------------------------------------------- //
// Associate this item with a particular gear record
// ----------------------------------------------------------------------- //
void CHUDGear::SetGearRecord(HGEAR hGear)
{
	m_hGear = hGear;

	if (m_hGear)
	{
		m_nCount = -1;
		m_hIconTexture.Load(g_pWeaponDB->GetString(hGear,WDB_GEAR_sIcon));
		m_nThreshold = ( uint32 )g_pWeaponDB->GetInt32(hGear,WDB_GEAR_nThreshold);
		SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f);
		Update();
	}
	else
	{
		m_bDraw = false;
		m_hIconTexture = NULL;
	}
}

// ----------------------------------------------------------------------- //
// Update values
// ----------------------------------------------------------------------- //
void CHUDGear::Update()
{
	if (!m_hGear) return;

	const CUserProfile* const pProfile = g_pProfileMgr->GetCurrentProfile();

	uint8 nCount = g_pPlayerStats->GetGearCount(m_hGear);
	m_bDraw = g_pPlayerStats->HaveHadGear(m_hGear) || (pProfile && pProfile->m_bPersistentHUD);

	//if the number of items has changed
	if (nCount != m_nCount)
	{
		if (nCount > m_nCount)
		{
			Flash("Pickup");
		}
		m_nCount = nCount;
		ResetFade();

		//enable the fade out if we have more than our threshold
		EnableFade(  m_nCount >= m_nThreshold || !g_pPlayerStats->HaveHadGear(m_hGear) );
		if (!m_bFadeEnabled)
		{
			m_Text.SetColor(m_cTextColor);
			DrawPrimSetRGBA(m_IconPoly,m_cIconColor);
		}


		//update our text
		wchar_t wstr[32];
		swprintf(wstr,L"%d", m_nCount );
		m_Text.SetText(wstr);

	}

}


void CHUDGear::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDGear");
	}

	CHUDItem::UpdateLayout();

}

void CHUDGear::Render( )
{
	if( g_vtHUDGearRender.GetFloat( ) < 1.0f )
		return;

	CHUDInventory::Render( );
}
