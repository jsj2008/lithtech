// ****************************************************************************************** //
//
// MODULE  : HUDSonicBreath.cpp
//
// PURPOSE : Implementation of HUD Sonic Breath display
//
// CREATED : 07/16/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#include "stdafx.h"

#ifndef __HUDSONICBREATH_H__
#include "HUDSonicBreath.h"
#endif//__HUDSONICBREATH_H__

// ****************************************************************************************** //

CHUDSonicBreath::CHUDSonicBreath()
{
	m_UpdateFlags = kHUDFrame;
}

// ****************************************************************************************** //

bool CHUDSonicBreath::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;
}

// ****************************************************************************************** //

void CHUDSonicBreath::Term()
{

}

// ****************************************************************************************** //

void CHUDSonicBreath::Render()
{
	SetRenderState();

	LTPoly_GT4 iPolyLeft = m_IconPoly;
	LTPoly_GT4 iPolyRight = m_IconPoly;
	SonicData iSonicData = g_pPlayerMgr->GetSonicData();

	float fBreath = iSonicData.GetBreathRemaining();
	float fWidth = ( iPolyLeft.verts[ 1 ].pos.x - iPolyLeft.verts[ 0 ].pos.x );

	iPolyLeft.verts[ 0 ].uv.y = 0.0f;
	iPolyLeft.verts[ 1 ].uv.y = 0.0f;
	iPolyLeft.verts[ 2 ].uv.y = 0.5f;
	iPolyLeft.verts[ 3 ].uv.y = 0.5f;

	iPolyRight.verts[ 0 ].uv.y = 0.5f;
	iPolyRight.verts[ 1 ].uv.y = 0.5f;
	iPolyRight.verts[ 2 ].uv.y = 1.0f;
	iPolyRight.verts[ 3 ].uv.y = 1.0f;

	iPolyLeft.verts[ 1 ].uv.x = ( 1.0f - fBreath );
	iPolyLeft.verts[ 2 ].uv.x = ( 1.0f - fBreath );
	iPolyLeft.verts[ 1 ].pos.x -= ( fBreath * fWidth );
	iPolyLeft.verts[ 2 ].pos.x -= ( fBreath * fWidth );

	iPolyRight.verts[ 0 ].uv.x = ( 1.0f - fBreath );
	iPolyRight.verts[ 3 ].uv.x = ( 1.0f - fBreath );
	iPolyRight.verts[ 0 ].pos.x += ( ( 1.0f - fBreath ) * fWidth );
	iPolyRight.verts[ 3 ].pos.x += ( ( 1.0f - fBreath ) * fWidth );

	g_pDrawPrim->SetTexture( m_hIconTexture );
	g_pDrawPrim->DrawPrim( &iPolyLeft, 1 );

	g_pDrawPrim->SetTexture( m_hIconTexture );
	g_pDrawPrim->DrawPrim( &iPolyRight, 1 );
}

// ****************************************************************************************** //

void CHUDSonicBreath::Update()
{
	DrawPrimSetRGBA( m_IconPoly, m_cIconColor );
}

// ****************************************************************************************** //

void CHUDSonicBreath::UpdateLayout()
{
	if( !m_hLayout )
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord( "HUDSonicBreath" );
	}

	CHUDItem::UpdateLayout();
}

