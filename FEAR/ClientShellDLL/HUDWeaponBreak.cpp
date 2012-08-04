// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeaponBreak.h
//
// PURPOSE : HUDItem to notify the player when his weapon is about to break.
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDWeaponBreak.h"
#include "HUDMgr.h"
#include "ClientWeaponMgr.h"
#include "ClientWeapon.h"

//******************************************************************************************

static const float g_fFadeDuration = 0.33f;

//******************************************************************************************

CHUDWeaponBreak::CHUDWeaponBreak()
: m_bDisplayIcon( false )
, m_bFadingOut( true )
, m_fFadeTimer( 0.0f )
{
	m_UpdateFlags = kHUDFrame;
}

//******************************************************************************************

bool CHUDWeaponBreak::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;
}

//******************************************************************************************

void CHUDWeaponBreak::Term()
{
}

//******************************************************************************************

void CHUDWeaponBreak::Render()
{
	if (m_bDisplayIcon)
	{
		SetRenderState();

		g_pDrawPrim->SetTexture( m_hIconTexture );
		g_pDrawPrim->DrawPrim( &m_IconPoly );

		g_pDrawPrim->SetTexture( m_hObjectTexture );
		g_pDrawPrim->DrawPrim( &m_ObjectPoly );
	}
}

//******************************************************************************************

void CHUDWeaponBreak::Update()
{
	CClientWeapon* pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (!pClientWeapon)
		return;

    bool bWarning = pClientWeapon->IsWeaponAboutToBreak();

	if (bWarning && !m_bDisplayIcon)
	{
		// fade in
		m_bDisplayIcon = true;
		m_bFadingOut = false;
		m_fFadeTimer = g_fFadeDuration;
	}

	if (!bWarning && !m_bFadingOut)
	{
		// fade out
		m_bDisplayIcon = true;
		m_bFadingOut = true;
		m_fFadeTimer = g_fFadeDuration;
	}

	// Update Effect
	if (m_fFadeTimer > 0.0f)
	{
		float fScale = 1.0f - ( m_fFadeTimer / g_fFadeDuration );
		float fFrameTime = g_pLTClient->GetFrameTime();

		// Update the current effect time
		m_fFadeTimer -= fFrameTime;

		if( m_fFadeTimer <= 0.0f && m_bFadingOut )
		{
			// fade out complete
			m_bDisplayIcon = false;
			return;
		}

		// Set the alpha fade for each graphic
		uint8 nAlpha;

		if( m_bFadingOut )
			nAlpha = ( uint8 )( 255 * ( 1.0 - fScale ) );
		else
			nAlpha = ( uint8 )( 255 * fScale );

		DrawPrimSetAlpha( m_IconPoly,	nAlpha ); 
		DrawPrimSetAlpha( m_ObjectPoly, nAlpha );
	}
}

// ****************************************************************************************** //

void CHUDWeaponBreak::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	// background image size used to align to corner of the screen
	LTVector2n vNewPos = AlignBasePosition( m_vIconSize );

	DrawPrimSetXYWH( m_IconPoly,   float(vNewPos.x + m_vIconOffset.x),    float(vNewPos.y + m_vIconOffset.y),    float(m_vIconSize.x),   float(m_vIconSize.y)   );
	DrawPrimSetXYWH( m_ObjectPoly, float(vNewPos.x + m_vObjectBasePos.x), float(vNewPos.y + m_vObjectBasePos.y), float(m_vObjectSize.x), float(m_vObjectSize.y) );
}

// ****************************************************************************************** //

void CHUDWeaponBreak::UpdateLayout()
{
	if( !m_hLayout )
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDWeaponBreak");
	}

	CHUDItem::UpdateLayout();

	InitAdditionalTextureData( m_hLayout, 0, m_hIconTexture,   m_vIconOffset,    m_vIconSize,   m_IconPoly   );
	InitAdditionalTextureData( m_hLayout, 1, m_hObjectTexture, m_vObjectBasePos, m_vObjectSize, m_ObjectPoly );
}
