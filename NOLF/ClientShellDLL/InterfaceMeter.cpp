// ----------------------------------------------------------------------- //
//
// MODULE  : InterfaceMeter.cpp
//
// PURPOSE : Implementation of InterfaceMeter class
//
// CREATED : 10/18/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "InterfaceMeter.h"
#include "GameClientShell.h"
#include "InterfaceResMgr.h"

int g_nWidth;

CInterfaceMeter::CInterfaceMeter()
{
	m_nValue = 0;
	m_rcRect.Init(0,0,0,0);
	m_hEmptySurf = LTNULL;
	m_hFullSurf = LTNULL;
}

CInterfaceMeter::~CInterfaceMeter()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMeter::Init()
//
//	PURPOSE:	prepare meter for display
//
// ----------------------------------------------------------------------- //

void CInterfaceMeter::Init()
{
	char szFullBar[64] = "statbar\\art\\boss_full.pcx";
	char szEmptyBar[64] = "statbar\\art\\boss_empty.pcx";
	m_rcRect = g_pLayoutMgr->GetBossRect();
	g_nWidth = m_rcRect.right - m_rcRect.left;

	m_hEmptySurf = g_pLTClient->CreateSurfaceFromBitmap(szEmptyBar);
	m_hFullSurf = g_pLTClient->CreateSurfaceFromBitmap(szFullBar);
	
		
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMeter::Term()
//
//	PURPOSE:	clean up meter
//
// ----------------------------------------------------------------------- //

void CInterfaceMeter::Term()
{
	if (m_hEmptySurf)
	{
		g_pLTClient->DeleteSurface(m_hEmptySurf);
		m_hEmptySurf = LTNULL;
	}
	if (m_hFullSurf)
	{
		g_pLTClient->DeleteSurface(m_hFullSurf);
		m_hFullSurf = LTNULL;
	}
	
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInterfaceMeter::Draw()
//
//	PURPOSE:	Handle drawing the stats
//
// ----------------------------------------------------------------------- //

void CInterfaceMeter::Draw(HSURFACE hScreen)
{
	if (!hScreen) return;

	if (m_nValue > 100) m_nValue = 100;

	// Update/Draw the meter if there is anything left...

	if (m_nValue > 0)
	{
		LTRect rcTemp;

		int nTemp = (int)((LTFLOAT)g_nWidth * (LTFLOAT)m_nValue / 100.0f );
		rcTemp.left = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_rcRect.left);
		rcTemp.right = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)(m_rcRect.left + nTemp));
		rcTemp.top = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_rcRect.top);
		rcTemp.bottom = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_rcRect.bottom);

//		g_pLTClient->FillRect(hScreen,&rcTemp,SETRGB(0,255,0));
		g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hFullSurf, &rcTemp, LTNULL);

		if (m_nValue < 100)
		{
			rcTemp.left = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)(m_rcRect.left + nTemp));
			rcTemp.right = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_rcRect.right);
//			g_pLTClient->FillRect(hScreen,&rcTemp,SETRGB(255,0,0));
			g_pLTClient->ScaleSurfaceToSurface(hScreen, m_hEmptySurf, &rcTemp, LTNULL);
		}

		rcTemp.left = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_rcRect.left);
		rcTemp.right = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)(m_rcRect.right));
		rcTemp.top = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_rcRect.top);
		rcTemp.bottom = rcTemp.top + 1;
		g_pLTClient->FillRect(hScreen,&rcTemp,LTNULL);

		rcTemp.bottom = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_rcRect.bottom);
		rcTemp.top = rcTemp.bottom - 1;
		g_pLTClient->FillRect(hScreen,&rcTemp,LTNULL);

		rcTemp.left = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_rcRect.left);
		rcTemp.right = rcTemp.left + 1;
		rcTemp.top = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_rcRect.top);
		rcTemp.bottom = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_rcRect.bottom);
		g_pLTClient->FillRect(hScreen,&rcTemp,LTNULL);

		rcTemp.right = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)(m_rcRect.right));
		rcTemp.left = rcTemp.right - 1;
		g_pLTClient->FillRect(hScreen,&rcTemp,LTNULL);

		if (m_nValue < 100)
		{
			rcTemp.left = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)(m_rcRect.left + nTemp));
			rcTemp.right = rcTemp.left + 1;
			g_pLTClient->FillRect(hScreen,&rcTemp,LTNULL);
		}

	}

}


