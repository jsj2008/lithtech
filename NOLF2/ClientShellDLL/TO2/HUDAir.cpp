// ----------------------------------------------------------------------- //
//
// MODULE  : HUDAir.cpp
//
// PURPOSE : HUDItem to display player air meter
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"

//******************************************************************************************
//**
//** HUD Air display
//**
//******************************************************************************************

CHUDAir::CHUDAir()
{
	m_UpdateFlags = kHUDAir;
	m_hIcon = LTNULL;
}


LTBOOL CHUDAir::Init()
{
	//Air icon
	m_hIcon = g_pInterfaceResMgr->GetTexture("interface\\hud\\icon_Air.dtx");
	g_pDrawPrim->SetRGBA(&m_Poly,argbWhite);
	SetupQuadUVs(m_Poly,m_hIcon,0.0f,0.0f,1.0f,1.0f);

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pStr = g_pFontManager->CreatePolyString(pFont,"",0.0f, 0.0f);

	UpdateLayout();

	m_Bar.Init(g_pInterfaceResMgr->GetTexture("interface\\hud\\airbar.dtx"));

	m_pStr->SetColor(m_Color);

	return LTTRUE;
}

void CHUDAir::Term()
{
	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}

}

void CHUDAir::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	if (m_bUseBar)
	{
		m_Bar.Render();
	}

	if (m_bUseText)
	{
		m_pStr->Render();
	}

	if (m_bUseIcon)
	{
		// draw our icons
		g_pDrawPrim->SetTexture(m_hIcon);
		g_pDrawPrim->DrawPrim(&m_Poly,1);
	}

}

void CHUDAir::Update()
{
	LTFLOAT fPercent = g_pPlayerStats->GetAirPercent() * 100.0f;

	m_bDraw = (fPercent < 100.0f);
	if (!m_bDraw) return;

	if (m_bUseBar)
	{
		float x = (float)(m_BasePos.x + m_BarOffset.x) * g_pInterfaceResMgr->GetXRatio();
		float y = (float)(m_BasePos.y + m_BarOffset.y) * g_pInterfaceResMgr->GetYRatio();

		float w = fPercent * m_fBarScale * g_pInterfaceResMgr->GetXRatio();
		float maxW = 100.0f * m_fBarScale * g_pInterfaceResMgr->GetXRatio();
		float h = (float)m_nBarHeight * g_pInterfaceResMgr->GetYRatio();
		
		m_Bar.Update(x,y,w,maxW,h);

	}

	if (m_bUseText)
	{
		float x = (float)(m_BasePos.x + m_TextOffset.x) * g_pInterfaceResMgr->GetXRatio();
		float y = (float)(m_BasePos.y + m_TextOffset.y) * g_pInterfaceResMgr->GetYRatio();

		uint8 h = (uint8)((float)m_nTextHeight * g_pInterfaceResMgr->GetYRatio());

		m_pStr->SetPosition(x,y);
		m_pStr->SetCharScreenHeight(h);
		char szTmp[16] = "";
		sprintf(szTmp,"%d",(int)fPercent);
		m_pStr->SetText(szTmp);


	}

	if (m_bUseIcon)
	{
		float x = (float)(m_BasePos.x + m_IconOffset.x) * g_pInterfaceResMgr->GetXRatio();
		float y = (float)(m_BasePos.y + m_IconOffset.y) * g_pInterfaceResMgr->GetYRatio();

		float w = (float)m_nIconSize * g_pInterfaceResMgr->GetYRatio();

		g_pDrawPrim->SetXYWH(&m_Poly,x,y,w,w);


	}

}

void CHUDAir::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetAirBasePos(nCurrentLayout);

	m_bUseBar		= g_pLayoutMgr->GetUseAirBar(nCurrentLayout);
	m_BarOffset	= g_pLayoutMgr->GetAirBarOffset(nCurrentLayout);

	m_bUseText	= g_pLayoutMgr->GetUseAirText(nCurrentLayout);
	m_TextOffset	= g_pLayoutMgr->GetAirTextOffset(nCurrentLayout);

	m_bUseIcon	= g_pLayoutMgr->GetUseAirIcon(nCurrentLayout);
	m_IconOffset	= g_pLayoutMgr->GetAirIconOffset(nCurrentLayout);
	m_nIconSize	= g_pLayoutMgr->GetAirIconSize(nCurrentLayout);

	m_nBarHeight		= g_pLayoutMgr->GetBarHeight(nCurrentLayout);
	m_nTextHeight		= g_pLayoutMgr->GetTextHeight(nCurrentLayout);
	m_fBarScale			= g_pLayoutMgr->GetBarScale(nCurrentLayout);

	m_Color		= g_pLayoutMgr->GetAirColor(nCurrentLayout);

}

