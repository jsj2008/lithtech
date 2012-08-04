// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAnimation.h
//
// PURPOSE : Implementation of an animating HUD component
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDAnimation.h"
#include "HUDMgr.h"
#include "InterfaceMgr.h"

CHUDAnimationData::CHUDAnimationData()
{
	pszMaterialName = NULL;
	bCache = true;
};


CHUDAnimation::CHUDAnimation()
{
	m_bInitted = false;
	m_bVisible = false;
	m_hMaterial = NULL;
	m_nColor = SET_ARGB( 0xFF, 0xFF, 0xFF, 0xFF);
}


bool CHUDAnimation::Init()
{
	// Copy in the init values.
	if (!m_AnimationData.rnBaseRect.GetHeight() || !m_AnimationData.rnBaseRect.GetHeight())
	{
		return false;
	}

	// Load the material.
	m_hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance( m_AnimationData.pszMaterialName );
	if( !m_hMaterial )
		return false;

	// Setup the poly.
	DrawPrimSetRGBA(m_Poly,0xFF, 0xFF, 0xFF, 0xFF);
	DrawPrimSetUVWH(m_Poly, 0.0f, 0.0f, 1.0f, 1.0f );

	m_bInitted = true;
	return true;
}

void CHUDAnimation::Show(bool bShow)
{
	if (!m_bInitted) return;

	m_bVisible = bShow;

	if (bShow)
	{
		SetScale( g_pInterfaceResMgr->GetScreenScale() );
	}
	else if (!m_AnimationData.bCache)
	{
		Unload();
	}
}

void CHUDAnimation::Unload()
{
	m_bInitted = false;
	m_bVisible = false;

	// Release our material.
	if( m_hMaterial )
	{
		g_pLTClient->GetRenderer()->ReleaseMaterialInstance( m_hMaterial );
		m_hMaterial = NULL;
	}
}

void CHUDAnimation::Render()
{
	if (!m_bInitted) return;
	if (!m_bVisible) return;

	g_pDrawPrim->DrawPrimMaterial( &m_Poly, 1, m_hMaterial );
}


void CHUDAnimation::SetScale(const LTVector2& vfScale)
{
	if (m_vfScale == vfScale)
	{
		return;
	}

	m_vfScale = vfScale;
	float x = (float)m_AnimationData.rnBaseRect.Left() * vfScale.x;
	float y = (float)m_AnimationData.rnBaseRect.Top() * vfScale.y;
	float w = (float)m_AnimationData.rnBaseRect.GetWidth() * vfScale.x;
	float h = (float)m_AnimationData.rnBaseRect.GetHeight() * vfScale.y;
	DrawPrimSetXYWH(m_Poly,x,y,w,h);
}

void CHUDAnimation::SetColor( uint32 nColor )
{
	m_nColor = nColor;
	DrawPrimSetRGBA( m_Poly, nColor );
}