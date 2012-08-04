// ----------------------------------------------------------------------- //
//
// MODULE  : HUDToolSelect.h
//
// PURPOSE : HUDItem to notify the player when the tool button can be used.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDToolSelect.h"
#include "HUDMgr.h"
#include "ClientWeaponMgr.h"

//******************************************************************************************

static const float g_fFadeTime = 0.33f;

//******************************************************************************************

CHUDToolSelect::CHUDToolSelect()
{
	m_UpdateFlags = kHUDFrame;
	m_hIconTexture = NULL;
	m_hObjectTexture = NULL;
	m_hButtonTexture = NULL;
	m_bDisplayIcon = false;
	m_fFadeTimer = 1.0f;
}

//******************************************************************************************

bool CHUDToolSelect::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;
}

//******************************************************************************************

void CHUDToolSelect::Term()
{
}

//******************************************************************************************

void CHUDToolSelect::Render()
{
	if (m_bDisplayIcon)
	{
		SetRenderState();

		g_pDrawPrim->SetTexture( m_hIconTexture );
		g_pDrawPrim->DrawPrim( &m_IconPoly );

		g_pDrawPrim->SetTexture( m_hObjectTexture );
		g_pDrawPrim->DrawPrim( &m_ObjectPoly );

		g_pDrawPrim->SetTexture( m_hButtonTexture );
		g_pDrawPrim->DrawPrim( &m_ButtonPoly );
	}
}

//******************************************************************************************

void CHUDToolSelect::Update()
{
	CClientWeapon* pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	bool bHoldingTool = (pClientWeapon && IS_ACTIVATE_FORENSIC(pClientWeapon->GetActivationType()));

	m_bDisplayIcon = false;

	if (g_pPlayerMgr->GetForensicObject())
	{
		if (!bHoldingTool)
		{
			m_bDisplayIcon = true;
			DrawPrimSetAlpha( m_IconPoly,   255 );
			DrawPrimSetAlpha( m_ObjectPoly, 255 );
			DrawPrimSetAlpha( m_ButtonPoly, 255 );
		}
		m_fFadeTimer = 1.0f;
	}
	else
	{
		if (bHoldingTool)
		{
			m_bDisplayIcon = true;

			m_fFadeTimer += g_pLTClient->GetFrameTime();
			while (m_fFadeTimer > g_fFadeTime)	// wrap to -g_fFadeTime
			{
				m_fFadeTimer -= (2.0f * g_fFadeTime);
			}

			float fFadePct = LTAbs(m_fFadeTimer) / g_fFadeTime;
			uint8 nAlpha = ( uint8 )( 255.0f * fFadePct );
			DrawPrimSetAlpha( m_IconPoly,   nAlpha );
			DrawPrimSetAlpha( m_ObjectPoly, nAlpha );
			DrawPrimSetAlpha( m_ButtonPoly, nAlpha );
		}
	}
}

// ****************************************************************************************** //

void CHUDToolSelect::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	// background image size used to align to corner of the screen
	LTVector2n vNewPos = AlignBasePosition( m_vIconSize );

	DrawPrimSetXYWH( m_IconPoly,   float(vNewPos.x + m_vIconOffset.x),    float(vNewPos.y + m_vIconOffset.y),    float(m_vIconSize.x),   float(m_vIconSize.y)   );
	DrawPrimSetXYWH( m_ObjectPoly, float(vNewPos.x + m_vObjectBasePos.x), float(vNewPos.y + m_vObjectBasePos.y), float(m_vObjectSize.x), float(m_vObjectSize.y) );
	DrawPrimSetXYWH( m_ButtonPoly, float(vNewPos.x + m_vButtonBasePos.x), float(vNewPos.y + m_vButtonBasePos.y), float(m_vButtonSize.x), float(m_vButtonSize.y) );
}

// ****************************************************************************************** //

void CHUDToolSelect::UpdateLayout()
{
	if( !m_hLayout )
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDToolSelect");
	}

	CHUDItem::UpdateLayout();

	InitAdditionalTextureData( m_hLayout, 0, m_hIconTexture,   m_vIconOffset,    m_vIconSize,   m_IconPoly   );
	InitAdditionalTextureData( m_hLayout, 1, m_hButtonTexture, m_vButtonBasePos, m_vButtonSize, m_ButtonPoly );
	InitAdditionalTextureData( m_hLayout, 2, m_hObjectTexture, m_vObjectBasePos, m_vObjectSize, m_ObjectPoly );
}
