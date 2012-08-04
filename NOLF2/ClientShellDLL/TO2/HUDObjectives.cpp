// ----------------------------------------------------------------------- //
//
// MODULE  : HUDObjectives.cpp
//
// PURPOSE : HUDItem to indicate that you've received a new objective
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "TO2PlayerMgr.h"
#include "GameClientShell.h"

//******************************************************************************************
//**
//** HUD Carry Icon display
//**
//******************************************************************************************

CHUDObjectives::CHUDObjectives()
:	CHUDItem			( ),
	m_BasePos			( 0, 0 ),
	m_fBlinkSpeed		( 0.0f ),
	m_fBlinkTime		( 0.0f ),
	m_fBlinkDuration	( 0.0f ),
	m_fAlpha			( 0.0f ),
	m_bShow				( false ),
	m_bBlink			( false ),
	m_fFadeDir			( 1.0f ),
	m_hIcon				( LTNULL )
{
	m_UpdateFlags = kHUDObjectives;
}


LTBOOL CHUDObjectives::Init()
{
	m_hIcon = g_pInterfaceResMgr->GetTexture("interface\\hud\\objectives.dtx");

	g_pDrawPrim->SetRGBA(&m_Poly,argbWhite);
	SetupQuadUVs(m_Poly,m_hIcon,0.0f,0.0f,1.0f,1.0f);

	UpdateLayout();

	return LTTRUE;
}

void CHUDObjectives::Term()
{

}

void CHUDObjectives::Render()
{
	if (!m_bShow) return;

	if( m_bBlink )
	{
		m_fAlpha += (g_pGameClientShell->GetFrameTime() * m_fFadeDir * m_fBlinkSpeed);
		m_fBlinkTime -= g_pGameClientShell->GetFrameTime();

		if (m_fAlpha < 0.0f)
		{
			m_fAlpha = 0.0f;
			m_fFadeDir = 1.0f;
		}

		if (m_fAlpha > 1.0f)
		{
			m_fAlpha = 1.0f;
			m_fFadeDir = -1.0f;
			if (m_fBlinkTime < 0.0f)
				m_bBlink = false;
		}

	}

	uint8 a = (uint8)( 255.0f * m_fAlpha );

	uint32 color = SET_ARGB(a,0xFF,0xFF,0xFF);
	g_pDrawPrim->SetRGBA(&m_Poly,color);

	SetRenderState();

	g_pDrawPrim->SetTexture(m_hIcon);
	g_pDrawPrim->DrawPrim(&m_Poly);


}

void CHUDObjectives::Update()
{
	if (!m_bShow) return;

	float fx = (float)(m_BasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float fy = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetXRatio();
	float fw = (float)(m_BaseSize.x) * g_pInterfaceResMgr->GetXRatio();
	float fh = (float)(m_BaseSize.y) * g_pInterfaceResMgr->GetYRatio();

	g_pDrawPrim->SetXYWH(&m_Poly,fx,fy,fw,fh);

}

void CHUDObjectives::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetObjectiveIconPos(nCurrentLayout);
	m_BaseSize		= g_pLayoutMgr->GetObjectiveIconSize(nCurrentLayout);

	m_fBlinkDuration = g_pLayoutMgr->GetObjectiveBlinkDuration(nCurrentLayout);
	if (m_fBlinkDuration <= 0.0f)
		m_fBlinkDuration = 3.0f;
	m_fBlinkSpeed = g_pLayoutMgr->GetObjectiveBlinkSpeed(nCurrentLayout);
	if (m_fBlinkSpeed <= 4.0f)
		m_fBlinkSpeed = 3.0f;

	if (!m_BaseSize.x)
	{
		m_BasePos = LTIntPt(536,360);
		m_BaseSize = LTIntPt(64,32);
	}

}


void CHUDObjectives::Show(bool bShow)
{
	if (m_bShow)
	{
		m_fAlpha = 0.0f;
		m_fFadeDir = 1.0f;
	}
	else
	{
		m_fAlpha = 1.0f;
		m_fFadeDir = -1.0f;
	}

	m_fBlinkTime = m_fBlinkDuration;
	m_bShow = bShow;
	m_bBlink = true;
	g_pHUDMgr->QueueUpdate(kHUDObjectives);
}

