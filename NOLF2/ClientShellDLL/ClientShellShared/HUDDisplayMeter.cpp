// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDisplayMeter.cpp
//
// PURPOSE : HUDDisplayMeter to display a meter from a server object or from the client game code....
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "HUDMgr.h"
	#include "LayoutMgr.h"
	#include "HUDDisplayMeter.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDisplayMeter::CHUDDisplayMeter
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDDisplayMeter::CHUDDisplayMeter()
:	CHUDMeter	()
{
	m_UpdateFlags = kHUDDisplayMeter;

	m_dwMaxValue = 100;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDDisplayMeter::UpdateLayout
//
//  PURPOSE:	Get the values for displaying the bar...
//
// ----------------------------------------------------------------------- //

void CHUDDisplayMeter::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_MeterBasePos.y	= g_pLayoutMgr->GetDisplayMeterBasePosY( nCurrentLayout );
	m_MeterOffset		= g_pLayoutMgr->GetDisplayMeterOffset( nCurrentLayout );
	m_nMeterHeight		= g_pLayoutMgr->GetDisplayMeterHeight( nCurrentLayout );
	m_fMeterScale		= g_pLayoutMgr->GetDisplayMeterScale( nCurrentLayout );
	
	g_pLayoutMgr->GetDisplayMeterTexture( nCurrentLayout, m_szMeterTex, ARRAY_LEN( m_szMeterTex ));

	// Center the display bar in the middle of the screen...
	m_MeterBasePos.x	= 320 - (uint16)(m_fMeterScale * (float)m_dwMaxValue / 2.0f);

}