// ------------------------------------------------------------------------------------------ //
//
// MODULE  : HUDFocus.cpp
//
// PURPOSE : Implementation of HUD Focus display
//
// CREATED : 06/15/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ------------------------------------------------------------------------------------------ //

#include "stdafx.h"

#ifndef __HUDFOCUS_H__
#include "HUDFocus.h"
#endif//__HUDFOCUS_H__

#ifndef __HUDMGR_H__
#include "HUDMgr.h"
#endif//__HUDMGR_H__

#ifndef __OBJECTDETECTOR_H__
#include "objectdetector.h"
#endif//__OBJECTDETECTOR_H__

#ifndef __PLAYERCAMERA_H__
#include "playercamera.h"
#endif//__PLAYERCAMERA_H__

#ifndef __BINDMGR_H__
#include "bindmgr.h"
#endif//__BINDMGR_H__

#ifndef __COMMANDIDS_H__
#include "commandids.h"
#endif//__COMMANDIDS_H__

// ****************************************************************************************** //
// External values / interfaces

extern ObjectDetector		g_iFocusObjectDetector;

// ****************************************************************************************** //

CHUDFocus::CHUDFocus()
{
	m_UpdateFlags = kHUDFrame;
}

// ****************************************************************************************** //

bool CHUDFocus::Init()
{
	UpdateLayout();
	ScaleChanged();

	m_IconPoly.verts[ 0 ].rgba.Init( 255, 255, 255, 255 );
	m_IconPoly.verts[ 0 ].uv.Init( 0.0f, 0.0f );
	m_IconPoly.verts[ 1 ].rgba.Init( 255, 255, 255, 255 );
	m_IconPoly.verts[ 1 ].uv.Init( 1.0f, 0.0f );
	m_IconPoly.verts[ 2 ].rgba.Init( 255, 255, 255, 255 );
	m_IconPoly.verts[ 2 ].uv.Init( 1.0f, 1.0f );
	m_IconPoly.verts[ 3 ].rgba.Init( 255, 255, 255, 255 );
	m_IconPoly.verts[ 3 ].uv.Init( 0.0f, 1.0f );

	return true;
}

// ****************************************************************************************** //

void CHUDFocus::Term()
{

}

// ****************************************************************************************** //

void CHUDFocus::Render()
{
	if( !CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_FOCUS ) || !g_iFocusObjectDetector.GetObject() )
	{
		return;
	}

	SetRenderState();

	g_pDrawPrim->SetTexture( m_hIconTexture );
	g_pDrawPrim->DrawPrim( &m_IconPoly, 1 );
}

// ****************************************************************************************** //

void CHUDFocus::Update()
{
	if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_FOCUS ) )
	{
		HOBJECT hFocusObject = g_iFocusObjectDetector.GetObject();

		if( hFocusObject )
		{
			LTVector vScreenPos;
			LTVector2 vMinScale( 16.0f, 16.0f );
			LTVector2 vMaxScale( 48.0f, 48.0f );

			// Figure out the scale we want to use...
			float fAccuracy = g_pPlayerMgr->GetFocusAccuracy();
			LTVector2 vScale = ( vMaxScale + ( ( vMinScale - vMaxScale ) * fAccuracy ) );

			// Get the current center and scale
			LTVector vFocusObjectPos = g_iFocusObjectDetector.GetObjectSpatialPosition();
			HOBJECT hCamera = g_pPlayerMgr->GetPlayerCamera()->GetCamera();

			if( LT_OK == g_pLTClient->GetRenderer()->WorldPosToScreenPos( hCamera, vFocusObjectPos, vScreenPos ) )
			{
				vScreenPos.x *= ( float )g_pInterfaceResMgr->GetScreenWidth();
				vScreenPos.y *= ( float )g_pInterfaceResMgr->GetScreenHeight();

				// Fill in the current render location...
				m_IconPoly.verts[ 0 ].pos = ( vScreenPos + LTVector( -vScale.x, -vScale.y, 0.0f ) );
				m_IconPoly.verts[ 1 ].pos = ( vScreenPos + LTVector( vScale.x, -vScale.y, 0.0f ) );
				m_IconPoly.verts[ 2 ].pos = ( vScreenPos + LTVector( vScale.x, vScale.y, 0.0f ) );
				m_IconPoly.verts[ 3 ].pos = ( vScreenPos + LTVector( -vScale.x, vScale.y, 0.0f ) );
			}
			else
			{
				// Zero it all out, so it won't render...
				m_IconPoly.verts[ 0 ].pos.Init( 0.0f, 0.0f, 0.0f );
				m_IconPoly.verts[ 1 ].pos.Init( 0.0f, 0.0f, 0.0f );
				m_IconPoly.verts[ 2 ].pos.Init( 0.0f, 0.0f, 0.0f );
				m_IconPoly.verts[ 3 ].pos.Init( 0.0f, 0.0f, 0.0f );
			}
		}
	}
}

// ****************************************************************************************** //

void CHUDFocus::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord( "HUDFocus" );
	}

	CHUDItem::UpdateLayout();
}

