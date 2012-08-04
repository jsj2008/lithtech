// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTitleSafeAreaNTSC.h
//
// PURPOSE : Implementation of HUD's title-safe area on NTSC
//
// CREATED : 12/06/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDTitleSafeAreaNTSC.h"
#include "HUDMgr.h"
#include "InterfaceMgr.h"

VarTrack g_vtHUDTitleSafeAreaNTSCRender;

//******************************************************************************************
//**
//** HUD Health display
//**
//******************************************************************************************

CHUDTitleSafeAreaNTSC::CHUDTitleSafeAreaNTSC()
{
	m_UpdateFlags = kHUDFrame;
}

bool CHUDTitleSafeAreaNTSC::Init()
{
	g_vtHUDTitleSafeAreaNTSCRender.Init( g_pLTClient, "HUDTitleSafeAreaNTSCRender", NULL, 1.0f );
	UpdateLayout();
	ScaleChanged();
	return true;
}

void CHUDTitleSafeAreaNTSC::Term()
{
}

void CHUDTitleSafeAreaNTSC::Update()
{
	DrawPrimSetRGBA( m_IconPoly, m_cIconColor );
}

void CHUDTitleSafeAreaNTSC::Render()
{
	if( g_vtHUDTitleSafeAreaNTSCRender.GetFloat( ) > 0.0f )
		return;

	SetRenderState();

	g_pDrawPrim->SetTexture(m_hIconTexture);
	g_pDrawPrim->DrawPrim(&m_IconPoly,1);
}

void CHUDTitleSafeAreaNTSC::ScaleChanged()
{
	CHUDItem::ScaleChanged();

	int nRezX = g_pInterfaceResMgr->GetScreenWidth();
	int nRezY = g_pInterfaceResMgr->GetScreenHeight();

	float x,y,w,h,u,v,umax,vmax;

	// defaults
	x = y = 0.0f;
	w = float(nRezX);
	h = float(nRezY);
	u = v = 0.0f;
	umax = vmax = 1.0f;

	if( m_vIconSize.x > nRezX )
	{
		// texture is wider than the screen, adjust uvs
		float fDiff = float(m_vIconSize.x - nRezX) / float(m_vIconSize.x);
		u	 = 0.5f * fDiff;
		umax = 1.0f - fDiff;
	}
	else
	{
		// texture is narrower than the screen, adjust the xys
		x = 0.5f * (nRezX - m_vIconSize.x);
		w = float(m_vIconSize.x);
	}

	if( m_vIconSize.y > nRezY )
	{
		// texture is taller than the screen, adjust uvs
		float fDiff = float(m_vIconSize.y - nRezY) / float(m_vIconSize.y);
		v	 = 0.5f * fDiff;
		vmax = 1.0f - fDiff;
	}
	else
	{
		// texture is shorter than the screen, adjust xys
		y = 0.5f * (nRezY - m_vIconSize.y);
		h = float(m_vIconSize.y);
	}

	DrawPrimSetXYWH( m_IconPoly, x, y, w, h );
	DrawPrimSetUVWH( m_IconPoly, u, v, umax, vmax );
}

void CHUDTitleSafeAreaNTSC::UpdateLayout()
{
	//if we haven't initialized our layout info

	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDTitleSafeAreaNTSC");
	}

	m_hIconTexture.Load(g_pLayoutDB->GetString(m_hLayout,LDB_HUDIconTexture,0));

	CHUDItem::UpdateLayout();
}
