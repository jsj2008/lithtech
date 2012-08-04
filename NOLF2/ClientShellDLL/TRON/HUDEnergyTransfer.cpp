// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEnergyTransfer.cpp
//
// PURPOSE : HUDItem to display an energy transfer in progress
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"
#include "TronPlayerMgr.h"

namespace
{
	const float DEG_TO_RAD = MATH_PI / 180.f;
}
//******************************************************************************************
//**
//** HUD Energy Transfer display
//**
//******************************************************************************************

CHUDEnergyTransfer::CHUDEnergyTransfer()
{
	// Make this something new
	m_UpdateFlags = kHUDEnergyTrans;
	m_fScale = 0.0f;
	m_bShow = false;
}


LTBOOL CHUDEnergyTransfer::Init()
{
	UpdateLayout();

//	g_pDrawPrim->SetRGBA(&m_Base,0xFF808080);
//	g_pDrawPrim->SetRGBA(&m_Progress, 0xFFC0FFC0);

	g_pDrawPrim->SetRGBA(&m_Connector, 0xC04040FF);
	LTIntPt pt1, pt2, pt3, pt4;

	BuildPoly(&m_Connector, 135.0f, 136.0f, 150.0f, 40.0f);

	float fTheta = 90.0f / 20.0f;

	for (int i =0 ; i < 20; i++)
	{
		BuildPoly(&m_EmptyArc[i], 135.0f + (fTheta * i), 135.0f + (fTheta * (i+1)), 150.0f, 147.0f);
		g_pDrawPrim->SetRGBA(&m_EmptyArc[i], 0xC04040FF);

		g_pDrawPrim->SetRGBA(&m_FullArc[i], 0xC0C0C0FF);
		m_FullArc[i].verts[0].z =
			m_FullArc[i].verts[1].z =
				m_FullArc[i].verts[2].z =
					m_FullArc[i].verts[3].z = SCREEN_NEAR_Z;
	}
	return LTTRUE;
}

LTIntPt CHUDEnergyTransfer::GetPoint(float fAngle, float fRadius)
{

	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	LTIntPt pt;
	
	pt.x = (int)((320.0f * xs) + fRadius * ltcosf(fAngle * DEG_TO_RAD));
	pt.y = (int)((240.0f * ys) - fRadius * ltsinf(fAngle * DEG_TO_RAD));
	return pt;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BuildPoly
//
//	PURPOSE:	Given a position, two angles and two radii, build a polygon.
//				The first radius is the inner one.
//
// ----------------------------------------------------------------------- //
void CHUDEnergyTransfer::BuildPoly(LT_POLYF4 * pPoly, float theta1, float theta2, float r1, float r2)
{
	if (!pPoly)
		return;

	LTIntPt p1, p2, p3, p4;

	p1 = GetPoint(theta1, r1);
	p2 = GetPoint(theta1, r2);
	p3 = GetPoint(theta2, r2);
	p4 = GetPoint(theta2, r1);

	pPoly->verts[0].x = (float)p1.x;
	pPoly->verts[0].y = (float)p1.y;
	pPoly->verts[0].z = SCREEN_NEAR_Z;

	pPoly->verts[1].x = (float)p2.x;
	pPoly->verts[1].y = (float)p2.y;
	pPoly->verts[1].z = SCREEN_NEAR_Z;

	pPoly->verts[2].x = (float)p3.x;
	pPoly->verts[2].y = (float)p3.y;
	pPoly->verts[2].z = SCREEN_NEAR_Z;

	pPoly->verts[3].x = (float)p4.x;
	pPoly->verts[3].y = (float)p4.y;
	pPoly->verts[3].z = SCREEN_NEAR_Z;
}

void CHUDEnergyTransfer::Term()
{
	// nothing to do here
}

void CHUDEnergyTransfer::Render()
{
	m_bShow = g_pTronPlayerMgr->IsTransferringEnergy();
	if (!m_bShow)
		return;

	SetRenderState();
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_NOBLEND);
//	g_pDrawPrim->DrawPrim(&m_Base);
//	g_pDrawPrim->DrawPrim(&m_Progress);
	g_pDrawPrim->DrawPrim(&m_Connector);
	g_pDrawPrim->DrawPrim(&m_FullArc[0], 20);
	g_pDrawPrim->DrawPrim(&m_EmptyArc[0], 20);
}

void CHUDEnergyTransfer::Update()
{
	if (g_pInterfaceResMgr->GetXRatio() != m_fScale)
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}
	
	float fPercentDone = g_pTronPlayerMgr->GetPercentEnergyTransferred();
	if (fPercentDone >= 1.0f)
	{
		fPercentDone = 1.0f;
//		g_pDrawPrim->SetRGBA(&m_Progress, 0xFFFFFFFF);
	}
/*	else
	{
		g_pDrawPrim->SetRGBA(&m_Progress, 0xBFC0FFC0);
	}

	float x2 = m_Base.verts[1].x;
	float x1 = m_Base.verts[0].x;
	float fWidth = (x2 - x1) * fPercentDone;
	float fNewX = x1 + fWidth;

	m_Progress.verts[1].x = m_Progress.verts[2].x = fNewX;
*/
	float fTheta = 90.0f * fPercentDone / 20.0f;
	for (int i = 0; i < 20; i++)
	{
		BuildPoly(&m_FullArc[i],135.0f + (i * fTheta),135.0f + fTheta + (i * fTheta),148.0f, 139.0f);
	}
}

void CHUDEnergyTransfer::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	BuildPoly(&m_Connector, 135.0f, 136.0f, 150.0f, 40.0f);

	float fTheta = 90.0f / 20.0f;

	for (int i =0 ; i < 20; i++)
	{
		BuildPoly(&m_EmptyArc[i], 135.0f + (fTheta * i), 135.0f + (fTheta * (i+1)), 150.0f, 147.0f);
		g_pDrawPrim->SetRGBA(&m_EmptyArc[i], 0xC04040FF);

		g_pDrawPrim->SetRGBA(&m_FullArc[i], 0xC0C0C0FF);
		m_FullArc[i].verts[0].z =
			m_FullArc[i].verts[1].z =
				m_FullArc[i].verts[2].z =
					m_FullArc[i].verts[3].z = SCREEN_NEAR_Z;
	}
/*
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	float xc = 320.0f * xs;
	float y = 100.0f * ys;
	float fWidth = 300.0f * xs;
	float fHeight = 16.0f * ys;

	g_pDrawPrim->SetXYWH( &m_Base, xc - (fWidth * 0.5f), y - (fHeight * 0.5f), fWidth, fHeight);
	m_Base.verts[0].z =
		m_Base.verts[1].z =
			m_Base.verts[2].z =
				m_Base.verts[3].z = SCREEN_NEAR_Z;

	m_Progress.verts[0].z =
		m_Progress.verts[1].z =
			m_Progress.verts[2].z =
				m_Progress.verts[3].z = SCREEN_NEAR_Z;
	g_pDrawPrim->SetXYWH( &m_Progress, xc - (fWidth * 0.5f), y - (fHeight * 0.5f), fWidth, fHeight);
*/
}

