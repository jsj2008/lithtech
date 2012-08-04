// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAmmoStack.h
//
// PURPOSE : HUDItem to notify the player when the tool button can be used.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDAmmoStack.h"
#include "HUDMgr.h"
#include "ClientWeaponMgr.h"

// ****************************************************************************************** //
// Dependencies

extern bool g_bInfiniteAmmo;

// ****************************************************************************************** //

// defaults, overridden by values in the database if they exist.
static const float g_fDefaultPreFadeInDuration	= 1.0f;
static const float g_fDefaultFadeInDuration		= 1.0f;
static const float g_fDefaultHoldDuration		= 2.0f;
static const float g_fDefaultFadeOutDuration	= 1.0f;

// spacing between stacking ammo icons as a percentage of the icon height
static const float g_fAmmoStackOffsetPct = 0.8f;

//******************************************************************************************

CHUDAmmoStack::CHUDAmmoStack()
: m_nAmmoCount			( 0 )
, m_eState				( kASS_None )
, m_fStateTimeLeft		( 0.0f )
{
	// parent class properties
	m_UpdateFlags	= kHUDFrame;

	// ambiguous TextureReference ctors interfere with these being in the initialization list
	m_hBGTexture	= NULL;
	m_hIconTexture	= NULL;
	m_hBarTexture	= NULL;
}

//******************************************************************************************

bool CHUDAmmoStack::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;
}

//******************************************************************************************

void CHUDAmmoStack::Term()
{
}

//******************************************************************************************

void CHUDAmmoStack::Render()
{
	// no ammo icon to render
	if( !m_hCurAmmo && !IsFadingOut() )
		return;

	SetRenderState();

	g_pDrawPrim->SetTexture( m_hBGTexture );
	g_pDrawPrim->DrawPrim( &m_BGPoly );

	g_pDrawPrim->SetTexture( m_hIconTexture );
	g_pDrawPrim->DrawPrim( &m_IconPoly );

	float fAmmoStartY = m_BarPoly.verts[ 0 ].pos.y;
	float fAmmoHeight = float(m_vBarSize.y);
	float fAmmoSlideAmountY = ( fAmmoHeight * g_fAmmoStackOffsetPct );

	for (uint8 i=0; i<m_nAmmoCount; i++)
	{
		m_BarPoly.verts[ 0 ].pos.y = m_BarPoly.verts[ 1 ].pos.y = ( fAmmoStartY - ( fAmmoSlideAmountY * ( float ) i ) );
		m_BarPoly.verts[ 2 ].pos.y = m_BarPoly.verts[ 3 ].pos.y = ( m_BarPoly.verts[ 0 ].pos.y + fAmmoHeight );

		if (m_BarPoly.verts[ 2 ].pos.y <= 0.0f)
			break;

		g_pDrawPrim->SetTexture( m_hBarTexture );
		g_pDrawPrim->DrawPrim( &m_BarPoly );
	}

	m_BarPoly.verts[ 0 ].pos.y = m_BarPoly.verts[ 1 ].pos.y = fAmmoStartY;
	m_BarPoly.verts[ 2 ].pos.y = m_BarPoly.verts[ 3 ].pos.y = ( fAmmoStartY + fAmmoHeight );
}

//******************************************************************************************

void CHUDAmmoStack::Update()
{
	// nothing to do
	if( m_eState == kASS_None )
		return;

	// get game info
	if( m_hCurAmmo )
		m_nAmmoCount = g_pPlayerStats->GetAmmoCount( m_hCurAmmo );
	m_fStateTimeLeft -= g_pLTClient->GetFrameTime();


	switch( m_eState )
	{
	case kASS_PreFadeIn:
	{
		if( StateCompleted() )
			FadeIn();
		break;
	}
	case kASS_FadeIn:
	{
		float t = 1.0f - (m_fStateTimeLeft / m_fFadeInDuration);
		float fAlpha = Clamp( LTLERP( 0.0f, 255.0f, t ), 0.0f, 255.0f );
		SetTextureAlphas( uint8(fAlpha) );

		if( StateCompleted() )
			Hold();

		break;
	}
	case kASS_Hold:
	{
		if( StateCompleted() )
			FadeOut();
		break;
	}
	case kASS_FadeOut:
	{
		float t = m_fStateTimeLeft / m_fFadeOutDuration;
		float fAlpha = Clamp( LTLERP( 0.0f, 255.0f, t ), 0.0f, 255.0f );
		SetTextureAlphas( uint8(fAlpha) );

		if( StateCompleted() )
			m_eState = kASS_None;
		break;
	}
	case kASS_None:
		LTASSERT( 0, "Ammo stack update early out error" );
		break;
	default:
		LTASSERT( 0, "Unknown ammo stack state" );
		break;
	}
}

//******************************************************************************************

void CHUDAmmoStack::SetAmmoType(HAMMO hAmmo)
{
	if( hAmmo )
	{
		// switching to different ammo type
		if( hAmmo != m_hCurAmmo )
		{
			HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo);
			const char* pszBar = g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sIcon );
			const char* pszIcon = g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sHUDTexture, 0 );

			m_hBarTexture.Load( pszBar );
			m_hIconTexture.Load( pszIcon );

			SetupQuadUVs( m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f );
			SetupQuadUVs( m_BarPoly,  m_hBarTexture,  0.0f, 0.0f, 1.0f, 1.0f );

			// handle infinite ammo
			HWEAPON hWeapon = g_pPlayerStats->GetCurrentWeaponRecord();
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
			bool bInfiniteAmmo = ( g_bInfiniteAmmo || g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ));
			if( bInfiniteAmmo )
			{
				HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo);
				m_nAmmoCount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nMaxAmount );
			}
		}

		// start it up
		PreFadeIn();
	}
	else
	{
		// the player no longer has a weapon with ammo
		FadeOut();
	}

	m_hCurAmmo = hAmmo;
}

//******************************************************************************************

bool CHUDAmmoStack::IsPreFadingIn() { return( m_eState == kASS_PreFadeIn ); }
bool CHUDAmmoStack::IsFadingIn()	{ return( m_eState == kASS_FadeIn	 ); }
bool CHUDAmmoStack::IsHolding()		{ return( m_eState == kASS_Hold		 ); }
bool CHUDAmmoStack::IsFadingOut()	{ return( m_eState == kASS_FadeOut	 ); }
bool CHUDAmmoStack::IsShutdown()	{ return( m_eState == kASS_None		 ); }

//******************************************************************************************

bool CHUDAmmoStack::StateCompleted() { return( m_fStateTimeLeft < 0.0f ); }

//******************************************************************************************

void CHUDAmmoStack::PreFadeIn()
{
	if( IsFadingOut() )
	{
		// already fading out, fade smoothly back in
		FadeIn();
	}
	else if( IsShutdown() )	// don't interfere if its already being rendered
	{
		m_fStateTimeLeft = m_fPreFadeInDuration;
		m_eState = kASS_PreFadeIn;
		SetTextureAlphas( 0 );
	}
}

//******************************************************************************************

void CHUDAmmoStack::FadeIn()
{
	// if hud item is fading out, just fade the same amount back in
	// ie: don't change m_fStateTimeLeft

	if( !IsHolding() && !IsFadingOut() )
		m_fStateTimeLeft = m_fFadeInDuration;

	m_eState = kASS_FadeIn;
}

//******************************************************************************************

void CHUDAmmoStack::Hold()
{
	if( m_eState == kASS_FadeIn )		// should always be coming from a fade-in
	{
		m_fStateTimeLeft = m_fHoldDuration;
		m_eState = kASS_Hold;
		SetTextureAlphas( 255 );
	}
}

//******************************************************************************************

void CHUDAmmoStack::FadeOut()
{
	// if hud item is fading in, just fade the same amount back out
	// ie: don't change m_fStateTimeLeft

	if( IsHolding() && !IsFadingIn() )
		m_fStateTimeLeft = m_fFadeOutDuration;

	m_eState = kASS_FadeOut;
}

//******************************************************************************************

void CHUDAmmoStack::SetTextureAlphas( uint8 nNewAlpha )
{
	DrawPrimSetAlpha( m_BGPoly,   nNewAlpha );
	DrawPrimSetAlpha( m_IconPoly, nNewAlpha );
	DrawPrimSetAlpha( m_BarPoly,  nNewAlpha );
}

//******************************************************************************************

void CHUDAmmoStack::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	// use the background image size for alignment
	LTVector2n vNewBasePos = AlignBasePosition( m_vBGSize );

	DrawPrimSetXYWH( m_BGPoly,   float(vNewBasePos.x + m_vBGBasePos.x),   float(vNewBasePos.y + m_vBGBasePos.y),   float(m_vBGSize.x),   float(m_vBGSize.y)   );
	DrawPrimSetXYWH( m_IconPoly, float(vNewBasePos.x + m_vIconBasePos.x), float(vNewBasePos.y + m_vIconBasePos.y), float(m_vIconSize.x), float(m_vIconSize.y) );
	DrawPrimSetXYWH( m_BarPoly,  float(vNewBasePos.x + m_vBarBasePos.x),  float(vNewBasePos.y + m_vBarBasePos.y),  float(m_vBarSize.x),  float(m_vBarSize.y)  );
}

// ****************************************************************************************** //

void CHUDAmmoStack::UpdateLayout()
{
	if( !m_hLayout )
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDAmmoStack");
	}

	CHUDItem::UpdateLayout();

	InitAdditionalTextureData( m_hLayout, 0, m_hBGTexture,   m_vBGBasePos,   m_vBGSize,   m_BGPoly   );
	InitAdditionalTextureData( m_hLayout, 1, m_hIconTexture, m_vIconBasePos, m_vIconSize, m_IconPoly );
	InitAdditionalTextureData( m_hLayout, 2, m_hBarTexture,  m_vBarBasePos,  m_vBarSize,  m_BarPoly  );

	m_fPreFadeInDuration	= g_pLayoutDB->GetFloat( m_hLayout, LDB_HUDAddFloat, 0, g_fDefaultPreFadeInDuration );
	m_fFadeInDuration		= g_pLayoutDB->GetFloat( m_hLayout, LDB_HUDAddFloat, 1, g_fDefaultFadeInDuration );
	m_fHoldDuration			= g_pLayoutDB->GetFloat( m_hLayout, LDB_HUDAddFloat, 2, g_fDefaultHoldDuration );
	m_fFadeOutDuration		= g_pLayoutDB->GetFloat( m_hLayout, LDB_HUDAddFloat, 3, g_fDefaultFadeOutDuration );
}
