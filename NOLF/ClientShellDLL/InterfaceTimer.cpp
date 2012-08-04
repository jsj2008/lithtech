// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceTimer.cpp
//
// PURPOSE : Implementation of InterfaceTimer class
//
// CREATED : 10/18/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "InterfaceTimer.h"
#include "GameClientShell.h"
#include "iltclient.h"
#include "ClientRes.h"
#include "InterfaceResMgr.h"

static void BuildTimeString(char* aBuffer, int nTime);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceTimer::Draw()
//
//	PURPOSE:	Handle drawing the stats
//
// ----------------------------------------------------------------------- //

void CInterfaceTimer::Draw(HSURFACE hScreen)
{
	if (!hScreen) return;

	// Get the screen size

    uint32 nWidth = 0;
    uint32 nHeight = 0;
    g_pLTClient->GetSurfaceDims(hScreen, &nWidth, &nHeight);

	// Update/Draw the timer if there is anytime left...

	if (m_fTime > 0.0f)
	{
		int x = nWidth/2;
		int y = 40;

		// Draw the string to the surface...

		int nMinutes = int(m_fTime) / 60;
		int nSeconds = m_fTime > 60.0 ? int(m_fTime) % 60 : int(m_fTime);

		char aBuffer[50];
		char aMinutes[20];
		char aSeconds[20];
		BuildTimeString(aMinutes, nMinutes);
		BuildTimeString(aSeconds, nSeconds);

		sprintf(aBuffer, "%s:%s", aMinutes, aSeconds);
        HSTRING hStr = g_pLTClient->CreateString(aBuffer);

		CLTGUIFont* pFont = g_pInterfaceResMgr->GetTitleFont();
        LTIntPt size = pFont->GetTextExtents(hStr);
		pFont->Draw(hStr, hScreen, x - (size.x/2)+2, y - (size.y/2)+2, LTF_JUSTIFY_LEFT,kBlack);
		pFont->Draw(hStr, hScreen, x - (size.x/2), y - (size.y/2), LTF_JUSTIFY_LEFT,kWhite);

        g_pLTClient->FreeString(hStr);
	}

	if (!m_bPause)
		m_fTime -= g_pGameClientShell->GetFrameTime();
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