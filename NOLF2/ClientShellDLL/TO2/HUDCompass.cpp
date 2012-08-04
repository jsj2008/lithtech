// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCompass.cpp
//
// PURPOSE : HUDItem to display a compass
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "PlayerMgr.h"

//******************************************************************************************
//**
//** HUD Compass display
//**
//******************************************************************************************

CHUDCompass::CHUDCompass()
{
	m_UpdateFlags = kHUDFrame;
	m_hBack = LTNULL;
	m_hNeedle = LTNULL;
	m_bDraw = LTFALSE;
	m_eLevel = kHUDRenderDead;
	m_fLastRotation = 0.0f;
}


LTBOOL CHUDCompass::Init()
{
	

	for (int i =0; i < 2; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Poly[i],argbWhite);
	}

	m_hBack = g_pInterfaceResMgr->GetTexture("interface\\hud\\compass_back.dtx");
	m_hNeedle = g_pInterfaceResMgr->GetTexture("interface\\hud\\compass_needle.dtx");

	SetupQuadUVs(m_Poly[0], m_hBack, 0.0f, 0.0f, 1.0f, 1.0f);
	SetupQuadUVs(m_Poly[1], m_hNeedle, 0.0f, 0.0f, 1.0f, 1.0f);

	UpdateLayout();

	return LTTRUE;
}

void CHUDCompass::Term()
{

}

void CHUDCompass::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	g_pDrawPrim->SetTexture(m_hBack);
	g_pDrawPrim->DrawPrim(&m_Poly[0]);


	g_pDrawPrim->SetTexture(m_hNeedle);
	g_pDrawPrim->DrawPrim(&m_Poly[1]);

}

void CHUDCompass::Update()
{
	if (!m_bDraw) return;

	static float fRat = (2.0f * (float)sin(MATH_PI/4.0f));
	float fWorldNorth = GetConsoleFloat("WorldNorth",0.0f);


	float cx = (float)(m_BasePos.x + m_nSize/2);
	float cy = (float)(m_BasePos.y + m_nSize/2);
	float fw = (float)(m_nSize) * g_pInterfaceResMgr->GetXRatio();
	float r = (float)(m_nSize) / fRat;

	float fx = (float)(m_BasePos.x) * g_pInterfaceResMgr->GetXRatio();
	float fy = (float)(m_BasePos.y) * g_pInterfaceResMgr->GetXRatio();

	g_pDrawPrim->SetXYWH(&m_Poly[1],fx,fy,fw,fw);
	
	float fRot = (fWorldNorth - (MATH_PI * 0.75f)) - g_pPlayerMgr->GetYaw();

	if (g_pPlayerMgr->IsCameraAttachedToHead())
	{
		fRot = m_fLastRotation;
	}
	else
	{
		m_fLastRotation = fRot;
	}




	float x[4];
	float y[4];


	for (uint8 i = 0; i < 4; i++)
	{
		x[i] = (cx + (float)cos(fRot) * r) * g_pInterfaceResMgr->GetXRatio();
		y[i] = (cy + (float)sin(fRot) * r) * g_pInterfaceResMgr->GetXRatio();
		fRot += (MATH_PI / 2);
	}

	g_pDrawPrim->SetXY4(&m_Poly[0],x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]);



}

void CHUDCompass::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetCompassPos(nCurrentLayout);
	m_nSize			= g_pLayoutMgr->GetCompassSize(nCurrentLayout);

}



