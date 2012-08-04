// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMeter.cpp
//
// PURPOSE : HUDItem to display a meter
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "HUDMeter.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDMeter::CHUDMeter
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDMeter::CHUDMeter()
:	CHUDItem		( ),
	m_MeterBasePos	( 0, 0 ),
	m_MeterOffset	( 0, 0 ),
	m_nMeterHeight	( 0 ),
	m_fMeterScale	( 0.0f ),
	m_dwValue		( 0 ),
	m_dwMaxValue	( 0 ),
	m_bDraw			( false )
{
	m_szMeterTex[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDMeter::CHUDMeter
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CHUDMeter::~CHUDMeter()
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDMeter::Init
//
//  PURPOSE:	Setup the meter...
//
// ----------------------------------------------------------------------- //

LTBOOL CHUDMeter::Init()
{
	// Get the data for layout.txt to setup the meter...

	UpdateLayout();

	// Set the meter bar texture...

	if( m_szMeterTex[0] )
	{
		m_MeterBar.Init( g_pInterfaceResMgr->GetTexture( m_szMeterTex ));
	}
	else
	{
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDMeter::Render
//
//  PURPOSE:	Draw the meter...
//
// ----------------------------------------------------------------------- //

void CHUDMeter::Render()
{
	if( !m_bDraw )
		return;

	SetRenderState();

	m_MeterBar.Render();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDMeter::Update
//
//  PURPOSE:	Set the values for the bar to draw...
//
// ----------------------------------------------------------------------- //

void CHUDMeter::Update()
{
	if( m_dwValue == 0 || m_dwMaxValue == 0 )
	{
		m_bDraw = false;
		return;
	}

	m_bDraw = true;

	float x = (float)(m_MeterBasePos.x + m_MeterOffset.x) * g_pInterfaceResMgr->GetXRatio();
	float y = (float)(m_MeterBasePos.y + m_MeterOffset.y) * g_pInterfaceResMgr->GetYRatio();

	float w = (float)m_dwValue * m_fMeterScale * g_pInterfaceResMgr->GetXRatio();
	float h = (float)m_nMeterHeight * g_pInterfaceResMgr->GetYRatio();
	float maxW = (float)m_dwMaxValue * m_fMeterScale * g_pInterfaceResMgr->GetXRatio();

	m_MeterBar.Update( x, y, w, maxW, h );
}
