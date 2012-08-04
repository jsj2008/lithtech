// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceTimer.cpp
//
// PURPOSE : Implementation of InterfaceTimer class
//
// CREATED : 10/18/99
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "InterfaceTimer.h"
#include "GameClientShell.h"
#include "iltclient.h"
#include "InterfaceResMgr.h"

static void BuildTimeString(char* aBuffer, int nTime);

CInterfaceTimer::CInterfaceTimer() 
{
	m_fTime		= 0.0f;
	m_bPause	= LTFALSE;
	m_pTimeStr	= LTNULL;
	m_nBaseSize = 0;
	m_nTeamId	= INVALID_TEAM;
	m_nColor	= argbWhite;
}

CInterfaceTimer::~CInterfaceTimer() 
{ 
	if (m_pTimeStr) 
		g_pFontManager->DestroyPolyString(m_pTimeStr); 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceTimer::Draw()
//
//	PURPOSE:	Handle drawing the stats
//
// ----------------------------------------------------------------------- //

void CInterfaceTimer::Draw()
{

	// Get the time left.
	float fCountDownTime = m_fTime - g_pLTClient->GetGameTime( );

	// Update/Draw the timer if there is anytime left...
	if( fCountDownTime <= 0.0f )
		return;

	// Draw the string to the surface...

	int nMinutes = int(fCountDownTime) / 60;
	int nSeconds = fCountDownTime > 60.0 ? int(fCountDownTime) % 60 : int(fCountDownTime);

	char aBuffer[8];
	char aMinutes[8];
	char aSeconds[8];
	BuildTimeString(aMinutes, nMinutes);
	BuildTimeString(aSeconds, nSeconds);
	uint8 nFont = 0;
	LTVector vColor;

	sprintf(aBuffer, "%s:%s", aMinutes, aSeconds);
	
 	if (!m_pTimeStr)
 	{
		switch( m_nTeamId )
		{
			// Team1.
			case 0:
				nFont = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","Team1TimerFont");
				m_nBaseSize = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","Team1TimerSize");
				m_BasePos = g_pLayoutMgr->GetPoint("Miscellaneous","Team1TimerPos");
				vColor = g_pLayoutMgr->GetVector("Miscellaneous","Team1TimerColor");
				break;
			// Team2.
			case 1:
				nFont = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","Team2TimerFont");
				m_nBaseSize = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","Team2TimerSize");
				m_BasePos = g_pLayoutMgr->GetPoint("Miscellaneous","Team2TimerPos");
				vColor = g_pLayoutMgr->GetVector("Miscellaneous","Team2TimerColor");
				break;
			// Unspecified.
			default:
				nFont = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","TimerFont");
				m_nBaseSize = (uint8)g_pLayoutMgr->GetInt("Miscellaneous","TimerSize");
				m_BasePos = g_pLayoutMgr->GetPoint("Miscellaneous","TimerPos");
				vColor = g_pLayoutMgr->GetVector("Miscellaneous","TimerColor");
				break;
		}

		if (m_nBaseSize == 0) 
			m_nBaseSize = 24;

		if (m_BasePos.x == 0)
			m_BasePos = LTIntPt(320,40);

		CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
		m_pTimeStr = g_pFontManager->CreateFormattedPolyString(pFont, aBuffer);
		m_pTimeStr->SetAlignmentH(CUI_HALIGN_CENTER);

		m_nColor = SET_ARGB( 0xFF, ( uint8 )vColor.x, ( uint8 )vColor.y, ( uint8 )vColor.z );
 	}
 	else
 	{
		m_pTimeStr->SetText(aBuffer);
 	}

	uint8 nSize = (uint8)((LTFLOAT)m_nBaseSize * g_pInterfaceResMgr->GetXRatio());
	m_pTimeStr->SetCharScreenHeight(nSize);

	// Position the text
	float x = (float)m_BasePos.x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)m_BasePos.y * g_pInterfaceResMgr->GetYRatio();

	m_pTimeStr->SetPosition( x+2.0f, y+2.0f);
	m_pTimeStr->SetColor(argbBlack);
	m_pTimeStr->Render();
	
	m_pTimeStr->SetPosition( x, y );
	m_pTimeStr->SetColor(m_nColor);
	m_pTimeStr->Render();
}

static void BuildTimeString(char* aBuffer, int nTime)
{
	if (nTime > 9)
	{
		sprintf(aBuffer, "%d", nTime);
	}
	else
	{
		sprintf(aBuffer, "0%d", nTime);
	}
}