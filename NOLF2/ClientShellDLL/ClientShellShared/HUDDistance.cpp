// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDistance.cpp
//
// PURPOSE : HUDItem to display player distance to an area
//
// CREATED : 5/08/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "stdafx.h"
	#include "HUDDistance.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::CHUDDistance
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDDistance::CHUDDistance()
:	CHUDItem			( ),
	m_BasePos			( 0, 0 ),
	m_fBlinkSpeed		( 0.0f ),
	m_fAlpha			( 0.0f ),
	m_fIconSize			( 0.0f ),
	m_bDraw				( false ),
	m_bBlink			( false ),
	m_fFadeIncrement	( 0.0f ),
	m_nFadeDir			( 1 ),
	m_bFadeOut			( false ),
	m_fFadeOutSpeed		( 0.0f ),
	m_fMaxAlpha			( 0.0f ),
	m_hIcon				( LTNULL )
{
	m_UpdateFlags = kHUDDistance;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::~CHUDDistance
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CHUDDistance::~CHUDDistance()
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::Init
//
//  PURPOSE:	Setup the icon and poly we are going to to draw...
//
// ----------------------------------------------------------------------- //

LTBOOL CHUDDistance::Init()
{
	UpdateLayout();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::Render
//
//  PURPOSE:	Draw the Icon...
//
// ----------------------------------------------------------------------- //

void CHUDDistance::Render()
{
	if (!m_bDraw) return;

	if( m_bFadeOut )
	{
		m_fAlpha += g_pGameClientShell->GetFrameTime() * m_fFadeIncrement;

		if( m_fAlpha <= 0.0f )
		{
			m_bFadeOut	= false;
			m_bDraw		= false;
			m_fAlpha	= 0.0f;
			m_nFadeDir	= 1;
		}
	}
	else if( m_bBlink )
	{
		m_fAlpha += g_pGameClientShell->GetFrameTime() * m_fFadeIncrement;

		if (m_fAlpha < 0.0f)
		{
			m_fAlpha = 0.0f;
			m_nFadeDir = 1;
		}

		if (m_fAlpha > m_fMaxAlpha)
		{
			m_fAlpha = m_fMaxAlpha;
			m_nFadeDir = -1;
		}

		m_fFadeIncrement = m_nFadeDir * m_fBlinkSpeed * m_fDistPercent;
	}

	uint8 a = (uint8)( 255.0f * m_fAlpha );

	uint32 color = SET_ARGB(a,0xFF,0xFF,0xFF);
	g_pDrawPrim->SetRGBA(&m_Poly,color);

	SetRenderState();

	// draw our icons
	g_pDrawPrim->SetTexture(m_hIcon);
	g_pDrawPrim->DrawPrim(&m_Poly,1);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::Update
//
//  PURPOSE:	Decide if we should draw the icon setup the render data...
//
// ----------------------------------------------------------------------- //

void CHUDDistance::Update()
{
	m_fDistPercent = g_pPlayerMgr->GetDistanceIndicatorPercent();
	m_hIcon = g_pPlayerMgr->GetDistanceIndicatorIcon();
	m_bDraw = (m_fDistPercent > 0.0f && m_hIcon);

	if( !m_bDraw )
	{
		if( m_fAlpha > 0.0f )
		{
			m_bDraw			= true;
			m_nFadeDir		= -1;
			m_fDistPercent	= 0.5f;
			m_bFadeOut		= true;

			m_fFadeIncrement = m_nFadeDir * m_fFadeOutSpeed;
		}
		else
		{
			m_fAlpha	= 0.0f;
			m_nFadeDir	= 1;
			return;
		}	
	}

	// Setup the poly and get the texture...
	
	g_pDrawPrim->SetRGBA( &m_Poly, argbWhite );
	SetupQuadUVs( m_Poly, m_hIcon, 0.0f, 0.0f, 1.0f, 1.0f );

	uint32 tw, th;
	g_pTexInterface->GetTextureDims( m_hIcon, tw, th );	
	m_fIconSize = (float)th;

	float x = (float)m_BasePos.x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)m_BasePos.y * g_pInterfaceResMgr->GetYRatio();

	float w = m_fIconSize * g_pInterfaceResMgr->GetYRatio();

	g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,w);

	m_bBlink = true;
	if( m_fDistPercent >= 1.0f )
	{
		m_bBlink = false;
		m_fAlpha = m_fMaxAlpha;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::UpdateLayout
//
//  PURPOSE:	Get data for this HUDItem from layouts.txt...
//
// ----------------------------------------------------------------------- //

void CHUDDistance::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt( "HUDLayout", 0 );

	m_BasePos		= g_pLayoutMgr->GetDistanceIconPos( nCurrentLayout );
	m_fBlinkSpeed	= g_pLayoutMgr->GetDistanceIconBlinkSpeed( nCurrentLayout );
	m_fMaxAlpha  	= g_pLayoutMgr->GetDistanceIconAlpha( nCurrentLayout );
	m_fFadeOutSpeed = g_pLayoutMgr->GetDistanceIconFadeOutSpeed( nCurrentLayout );
}