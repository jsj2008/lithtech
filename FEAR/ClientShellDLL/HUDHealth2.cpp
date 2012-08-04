// ****************************************************************************************** //
//
// MODULE  : HUDHealth2.cpp
//
// PURPOSE : Implementation of the Health meter
//
// CREATED : 07/16/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#include "stdafx.h"
#include "HUDHealth2.h"
#include "PlayerStats.h"

extern VarTrack			g_vtXUIUseOldHUD;


// ****************************************************************************************** //
// magic data to position the end cap of the health bar

static const float g_fBarUVStart		= 0.05f;					// where the blood starts/ends
static const float g_fBarUVEnd			= 0.75f;
static const float g_fBarCapSpacing		= -10.f;					// positions cap where bar ends

// ****************************************************************************************** //

CHUDHealth2::CHUDHealth2()
{
	m_hBGTexture = NULL;
	m_hIconTexture = NULL;
	m_hBarTexture = NULL;
	m_UpdateFlags = kHUDHealth;
}

// ****************************************************************************************** //

bool CHUDHealth2::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;
}

// ****************************************************************************************** //

void CHUDHealth2::Term()
{

}

// ****************************************************************************************** //

void CHUDHealth2::Render()
{
	if (!g_vtXUIUseOldHUD.GetFloat())
		return;

	SetRenderState();

	g_pDrawPrim->SetTexture( m_hBGTexture );	g_pDrawPrim->DrawPrim( &m_BGPoly );
	g_pDrawPrim->SetTexture( m_hBarTexture );	g_pDrawPrim->DrawPrim( &m_BarPoly );
	g_pDrawPrim->SetTexture( m_hCapTexture );	g_pDrawPrim->DrawPrim( &m_CapPoly );
}

// ****************************************************************************************** //

void CHUDHealth2::Update()
{
	float fHealth = ( ( float )g_pPlayerStats->GetHealth() / ( float )g_pPlayerStats->GetMaxHealth() );

	// scale only the 'meter' part of the texture by the players health
	float fBarUVWidth = (g_fBarUVStart + (g_fBarUVEnd - g_fBarUVStart) * fHealth);
	float fBarScreenWidth = fBarUVWidth * m_vBarSize.x;

	LTVector2 vNewPos;

	vNewPos.x = float(m_vScreenAdjustedBasePos.x + m_vBGBasePos.x);
	vNewPos.y = float(m_vScreenAdjustedBasePos.y + m_vBGBasePos.y);
	DrawPrimSetXYWH(m_BGPoly, vNewPos.x, vNewPos.y, float(m_vBGSize.x), float(m_vBGSize.y));

	vNewPos.x = float(m_vScreenAdjustedBasePos.x + m_vBarBasePos.x);
	vNewPos.y = float(m_vScreenAdjustedBasePos.y + m_vBarBasePos.y);
	DrawPrimSetXYWH(m_BarPoly, vNewPos.x, vNewPos.y, fBarScreenWidth, float(m_vBarSize.y));

	vNewPos.x = float(m_vScreenAdjustedBasePos.x + m_vCapBasePos.x + g_fBarCapSpacing) + fBarScreenWidth;
	vNewPos.y = float(m_vScreenAdjustedBasePos.y + m_vCapBasePos.y);
	DrawPrimSetXYWH(m_CapPoly, vNewPos.x, vNewPos.y, float(m_vCapSize.x), float(m_vCapSize.y));

	SetupQuadUVs( m_BarPoly, m_hBarTexture, 0.0f, 0.0f, fBarUVWidth, 1.0f );
}

// ****************************************************************************************** //

void CHUDHealth2::ScaleChanged()
{
	CHUDItem::ScaleChanged();
	// use the background image size for alignment
	m_vScreenAdjustedBasePos = AlignBasePosition( m_vBGSize );
}

// ****************************************************************************************** //

void CHUDHealth2::UpdateLayout()
{
	if (!g_vtXUIUseOldHUD.GetFloat())
		return;

	if( !m_hLayout )
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDHealth2");
	}

	CHUDItem::UpdateLayout();

	InitAdditionalTextureData( m_hLayout, 0, m_hBGTexture,  m_vBGBasePos,  m_vBGSize,  m_BGPoly  );
	InitAdditionalTextureData( m_hLayout, 1, m_hBarTexture, m_vBarBasePos, m_vBarSize, m_BarPoly );
	InitAdditionalTextureData( m_hLayout, 2, m_hCapTexture, m_vCapBasePos, m_vCapSize, m_CapPoly );
}
