// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDamageDir.cpp
//
// PURPOSE : HUDItem to display directional damage info
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "HUDDamageDir.h"
#include "PlayerMgr.h"
#include "LayoutDB.h"


VarTrack			g_vtDamageShowAll;

//******************************************************************************************
//**
//** HUD Damage dir display
//**
//******************************************************************************************

CHUDDamageDir::CHUDDamageDir()
{
	m_UpdateFlags = kHUDFrame;
	m_bDraw = false;
}


bool CHUDDamageDir::Init()
{

	UpdateLayout();

	return true;
}

void CHUDDamageDir::Term()
{

}

void CHUDDamageDir::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(m_Poly,kNumDamageSectors);

}

void CHUDDamageDir::Update()
{
	if (m_nOuterRadius <= 0)
	{
		m_bDraw = false;
		return;
	}

	float fAlphaRange = m_fMaxAlpha - m_fMinAlpha;

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
			float fA = m_fMinAlpha + fDam * fAlphaRange;
			nAlpha = (uint8) (fA * 255.0f);
			
		}

		DrawPrimSetRGBA(m_Poly[i], 0xFF, 0xFF, 0xFF, nAlpha);
	}

	m_bDraw = (fDamTotal > 0.0f);

}

void CHUDDamageDir::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDDamageDir");
	}

	CHUDItem::UpdateLayout();

	m_nInnerRadius = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);
	m_nOuterRadius = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,1);

	for (int i =0; i < kNumDamageSectors; i++)
	{
		DrawPrimSetRGBA(m_Poly[i],argbWhite);
		SetupQuadUVs(m_Poly[i], m_hIconTexture, 0.0f,0.0f,1.0f,1.0f);
	}

	m_fMinAlpha = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,0);
	m_fMaxAlpha = g_pLayoutDB->GetFloat(m_hLayout,LDB_HUDAddFloat,1);

	g_vtDamageShowAll.Init(g_pLTClient, "DamageShowAll", NULL, 0.0f);

	ScaleChanged();

}


void CHUDDamageDir::ScaleChanged()
{

	float r = (float)(m_nOuterRadius) * g_pInterfaceResMgr->GetXRatio() / (2.0f * (float)sin(MATH_PI/4.0f));

	float fx = float(g_pInterfaceResMgr->GetScreenWidth() / 2);
	float fy = float(g_pInterfaceResMgr->GetScreenHeight() / 2);
	float fw = (float(m_nInnerRadius) + float(m_nOuterRadius)/ 2.0f) * g_pInterfaceResMgr->GetXRatio();

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
			x[i] = (cx + (float)cos(fRot) * r);
			y[i] = (cy - (float)sin(fRot) * r);
			fRot += (MATH_PI / 2);
		}

		DrawPrimSetXY(m_Poly[n], 0, x[0], y[0]);
		DrawPrimSetXY(m_Poly[n], 1, x[1], y[1]);
		DrawPrimSetXY(m_Poly[n], 2, x[2], y[2]);
		DrawPrimSetXY(m_Poly[n], 3, x[3], y[3]);
	}
}