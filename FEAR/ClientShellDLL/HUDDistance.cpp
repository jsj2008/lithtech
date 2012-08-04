// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDistance.cpp
//
// PURPOSE : HUDItem to display player distance to an area
//
// CREATED : 5/08/02
//
// (c) 2002-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//
	
	#include "stdafx.h"
	#include "HUDDistance.h"

VarTrack g_vtHUDDistanceRender;


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::CHUDDistance
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDDistance::CHUDDistance()
:	CHUDItem			( ),
	m_fBlinkSpeed		( 0.0f ),
	m_fAlpha			( 0.0f ),
	m_bDraw				( false ),
	m_bBlink			( false ),
	m_fFadeIncrement	( 0.0f ),
	m_nFadeDir			( 1 ),
	m_bFadeOut			( false ),
	m_fMaxAlpha			( 0.0f ),
	m_bFirstUpdate		( true )
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

bool CHUDDistance::Init()
{
	g_vtHUDDistanceRender.Init( g_pLTClient, "HUDDistanceRender", NULL, 1.0f );

	UpdateLayout();
	ScaleChanged();

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CHUDDistance::Term()
//              
//	PURPOSE:	Term is called when a world is exited.  As such, clear out
//				state at this time.
//              
//----------------------------------------------------------------------------
void CHUDDistance::Term()
{
	m_fAlpha			= 0.0f;
	m_bDraw				= false;
	m_bBlink			= false;
	m_fFadeIncrement	= 0.0f;
	m_nFadeDir			= 1;
	m_bFadeOut			= false;
	m_bFirstUpdate		= true;
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
	if( g_vtHUDDistanceRender.GetFloat( ) < 1.0f )
		return;

	if (!m_bDraw || m_bFirstUpdate) return;

	if( m_bFadeOut )
	{
		m_fAlpha += RealTimeTimer::Instance().GetTimerElapsedS( ) * m_fFadeIncrement;

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
		m_fAlpha += RealTimeTimer::Instance().GetTimerElapsedS( ) * m_fFadeIncrement;

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

	DrawPrimSetRGBA(m_IconPoly, 0xFF, 0xFF, 0xFF, a);

	SetRenderState();

	// draw our icons
	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(&m_IconPoly,1);
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
	if( m_bFirstUpdate )
	{
		m_bFirstUpdate = false;
		g_pHUDMgr->QueueUpdate( kHUDDistance );
		return;
	}

	m_fDistPercent = g_pPlayerMgr->GetDistanceIndicatorPercent();
	m_hIconTexture = g_pPlayerMgr->GetDistanceIndicatorIcon();
	m_bDraw = (m_fDistPercent > 0.0f && m_hIconTexture);

	if( !m_bDraw )
	{
		if( m_fAlpha > 0.0f )
		{
			m_bDraw			= true;
			m_nFadeDir		= -1;
			m_fDistPercent	= 0.5f;
			m_bFadeOut		= true;

			m_fFadeIncrement = m_nFadeDir / m_fFadeTime;
		}
		else
		{
			m_fAlpha	= 0.0f;
			m_nFadeDir	= 1;
			return;
		}	
	}

	// Setup the poly and get the texture...
	
	DrawPrimSetRGBA( m_IconPoly, argbWhite );
	SetupQuadUVs( m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f );

	m_bBlink = true;
	if( m_fDistPercent >= 1.0f )
	{
		m_bBlink = false;
		m_fAlpha = m_fMaxAlpha;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::ScaleChanged
//
//  PURPOSE:	Update position based on changed resolution
//
// ----------------------------------------------------------------------- //

void CHUDDistance::ScaleChanged()
{
	CHUDItem::ScaleChanged();
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
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDDistance");
	}

	CHUDItem::UpdateLayout();


	m_fBlinkSpeed	= g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);

	float nA = ( float )GET_A(m_cIconColor);
	m_fMaxAlpha = nA / 255.0f;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDistance::Clear
//
//  PURPOSE:	Clears to not init state.
//
// ----------------------------------------------------------------------- //
void CHUDDistance::Clear()
{
	m_fAlpha	= 0.0f;
	m_nFadeDir	= 1;
	m_bDraw = false;
}
