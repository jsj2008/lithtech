// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHidingBar.cpp
//
// PURPOSE : HUDHidingBar to display a meter when hiding....
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
	#include "HUDHidingBar.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDHidingBar::CHUDHidingBar
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDHidingBar::CHUDHidingBar()
:	CHUDMeter	()
{
	m_UpdateFlags = kHUDNone;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDHidingBar::Update
//
//  PURPOSE:	Update the values to display the bar...
//
// ----------------------------------------------------------------------- //

void CHUDHidingBar::Update()
{
	// Center the hiding bar in the middle of the screen...
	m_MeterBasePos.x	= 320 - (uint16)(m_fMeterScale * (float)m_dwMaxValue / 2.0f);

	CHUDMeter::Update();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDHidingBar::UpdateLayout
//
//  PURPOSE:	Get the values for displaying the bar...
//
// ----------------------------------------------------------------------- //

void CHUDHidingBar::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_MeterBasePos.y	= g_pLayoutMgr->GetHidingBarBasePosY( nCurrentLayout );
	m_MeterOffset		= g_pLayoutMgr->GetHidingBarOffset( nCurrentLayout );
	m_nMeterHeight		= g_pLayoutMgr->GetHidingBarHeight( nCurrentLayout );
	m_fMeterScale		= g_pLayoutMgr->GetHidingBarScale( nCurrentLayout );
	
	g_pLayoutMgr->GetHidingBarTexture( nCurrentLayout, m_szMeterTex, ARRAY_LEN( m_szMeterTex ));

}
