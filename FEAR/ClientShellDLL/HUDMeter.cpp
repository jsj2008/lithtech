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
	m_dwValue		( 0 ),
	m_dwMaxValue	( 0 ),
	m_bDraw			( false ),
	m_bCentered		( false )
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

bool CHUDMeter::Init()
{
	// Get the data for layout.txt to setup the meter...

	UpdateLayout();

	// Set the meter bar texture...

	if( m_szMeterTex[0] )
	{
		TextureReference hFrame(m_szMeterTex);
		m_MeterBar.Init( hFrame );
	}
	else
	{
		return false;
	}

	ScaleChanged();

	return true;
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

	float x = (float)(m_vBasePos.x + m_vIconOffset.x) * g_pInterfaceResMgr->GetXRatio();
	float y = (float)(m_vBasePos.y + m_vIconOffset.y) * g_pInterfaceResMgr->GetYRatio();

	float fPercent = float(m_dwValue)/float(m_dwMaxValue);

	float maxW = float(m_vIconSize.x);
	float w = fPercent * maxW;
	float h = float(m_vIconSize.y);

	if (m_bCentered)
	{
		x -= (maxW / 2.0f);
		y -= (h / 2.0f);
	}


	m_MeterBar.Update( x, y, w, maxW, h );
}
