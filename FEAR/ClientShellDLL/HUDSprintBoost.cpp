// ****************************************************************************************** //
//
// MODULE  : CHUDSprintBoost.cpp
//
// PURPOSE : Implementation of HUD speed boost display
//
// CREATED : 10/12/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#include "stdafx.h"

#ifndef __HUDSPRINTBOOST_H__
#include "HUDSprintBoost.h"
#endif//__HUDSPRINTBOOST_H__

#include "CMoveMgr.h"

// ****************************************************************************************** //

CHUDSprintBoost::CHUDSprintBoost()
{
	m_UpdateFlags = kHUDFrame;
}

// ****************************************************************************************** //

bool CHUDSprintBoost::Init()
{
	UpdateLayout();
	ScaleChanged();
	return true;
}

// ****************************************************************************************** //

void CHUDSprintBoost::Term()
{

}

// ****************************************************************************************** //

void CHUDSprintBoost::Render()
{
	SetRenderState();

	LTPoly_GT4 iPolyLeft = m_IconPoly;
	LTPoly_GT4 iPolyRight = m_IconPoly;

	float fPercent = g_pMoveMgr->GetSprintBoostPercent();
	float fWidth = ( iPolyLeft.verts[ 1 ].pos.x - iPolyLeft.verts[ 0 ].pos.x );

	iPolyLeft.verts[ 0 ].uv.y = 0.0f;
	iPolyLeft.verts[ 1 ].uv.y = 0.0f;
	iPolyLeft.verts[ 2 ].uv.y = 0.5f;
	iPolyLeft.verts[ 3 ].uv.y = 0.5f;

	iPolyRight.verts[ 0 ].uv.y = 0.5f;
	iPolyRight.verts[ 1 ].uv.y = 0.5f;
	iPolyRight.verts[ 2 ].uv.y = 1.0f;
	iPolyRight.verts[ 3 ].uv.y = 1.0f;

	iPolyLeft.verts[ 1 ].uv.x = ( 1.0f - fPercent );
	iPolyLeft.verts[ 2 ].uv.x = ( 1.0f - fPercent );
	iPolyLeft.verts[ 1 ].pos.x -= ( fPercent * fWidth );
	iPolyLeft.verts[ 2 ].pos.x -= ( fPercent * fWidth );

	iPolyRight.verts[ 0 ].uv.x = ( 1.0f - fPercent );
	iPolyRight.verts[ 3 ].uv.x = ( 1.0f - fPercent );
	iPolyRight.verts[ 0 ].pos.x += ( ( 1.0f - fPercent ) * fWidth );
	iPolyRight.verts[ 3 ].pos.x += ( ( 1.0f - fPercent ) * fWidth );

	g_pDrawPrim->SetTexture( m_hIconTexture );
	g_pDrawPrim->DrawPrim( &iPolyLeft, 1 );

	g_pDrawPrim->SetTexture( m_hIconTexture );
	g_pDrawPrim->DrawPrim( &iPolyRight, 1 );
}

// ****************************************************************************************** //

void CHUDSprintBoost::Update()
{
	DrawPrimSetRGBA( m_IconPoly, m_cIconColor );
}

// ****************************************************************************************** //

void CHUDSprintBoost::UpdateLayout()
{
	if( !m_hLayout )
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord( "HUDSprintBoost" );
	}

	CHUDItem::UpdateLayout();
}

