// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDamageDir.cpp
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
#include "HUDDamageDir.h"
#include "PlayerMgr.h"


VarTrack			g_vtDamageMinAlpha;
VarTrack			g_vtDamageMaxAlpha;
VarTrack			g_vtDamageShowAll;

//******************************************************************************************
//**
//** HUD Compass display
//**
//******************************************************************************************

CHUDDamageDir::CHUDDamageDir()
{
	m_UpdateFlags = kHUDFrame;
	m_hArrow = LTNULL;
	m_bDraw = LTFALSE;
	m_fScale = 0.0f;
}


LTBOOL CHUDDamageDir::Init()
{
	m_hArrow = g_pInterfaceResMgr->GetTexture("interface\\hud\\damagearrow.dtx");

	for (int i =0; i < kNumDamageSectors; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Poly[i],argbWhite);
		SetupQuadUVs(m_Poly[i], m_hArrow, 0.0f,0.0f,1.0f,1.0f);
	}

	g_vtDamageMinAlpha.Init(g_pLTClient, "DamageMinAlpha", NULL, 0.25f);
	g_vtDamageMaxAlpha.Init(g_pLTClient, "DamageMaxAlpha", NULL, 0.9f);
	g_vtDamageShowAll.Init(g_pLTClient, "DamageShowAll", NULL, 0.0f);

	UpdateLayout();

	return LTTRUE;
}

void CHUDDamageDir::Term()
{

}

void CHUDDamageDir::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	g_pDrawPrim->SetTexture(m_hArrow);
	g_pDrawPrim->DrawPrim(m_Poly,kNumDamageSectors);

}

void CHUDDamageDir::Update()
{
	if (m_nSize <= 0)
	{
		m_bDraw = LTFALSE;
		return;
	}

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		UpdateScale();

	float fAlphaRange = g_vtDamageMaxAlpha.GetFloat() - g_vtDamageMinAlpha.GetFloat();

	float fDamTotal = 0.0f;
	for (uint8 i = 0; i < kNumDamageSectors; i++)
	{
		float fDam = g_pPlayerMgr->GetDamageFromSector(i);
		fDam = LTCLAMP( fDam, 0.0f, 1.0f );

		if (g_vtDamageShowAll.GetFloat() > 0.0f && fDam < 0.1f)
			fDam = 0.1f;

		fDamTotal += fDam;

		uint8 nAlpha = 0;

			
		
		if (fDam > 0.0f)
		{
			float fA = g_vtDamageMinAlpha.GetFloat() + fDam * fAlphaRange;
			nAlpha = (uint8) (fA * 255.0f);
			
		}

		uint32 argbCol = SET_ARGB(nAlpha,255,255,255);
		g_pDrawPrim->SetRGBA(&m_Poly[i],argbCol);
	}

	m_bDraw = (fDamTotal > 0.0f);

}

void CHUDDamageDir::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_nSize = g_pLayoutMgr->GetDamageSize(nCurrentLayout);

	if (m_nSize == 0)
		m_nSize = 48;

	UpdateScale();

}


void CHUDDamageDir::UpdateScale()
{
	m_fScale = g_pInterfaceResMgr->GetXRatio();

	float r = (float)(m_nSize) / (2.0f * (float)sin(MATH_PI/4.0f));

	float fx = 320.0f;
	float fy = 240.0f;
	float fw = (float)(m_nSize/2) * m_fScale;

	for (int n = 0; n < kNumDamageSectors; n++)
	{
		float fRot = ((float)n * MATH_PI * 2) / kNumDamageSectors;

		float x[4];
		float y[4];
		
		float cx = fx + fw * (float)cos(fRot);
		float cy = fy - fw * (float)sin(fRot);


		fRot += (1.25f * MATH_PI);


		for (uint8 i = 0; i < 4; i++)
		{
			x[i] = (cx + (float)cos(fRot) * r) * m_fScale;
			y[i] = (cy - (float)sin(fRot) * r) * m_fScale;
			fRot += (MATH_PI / 2);
		}

		g_pDrawPrim->SetXY4(&m_Poly[n],x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]);
	}


}