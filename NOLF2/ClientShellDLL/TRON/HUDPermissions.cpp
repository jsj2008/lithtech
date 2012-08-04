// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPermissions.cpp
//
// PURPOSE : HUDItem to display Jet's current permission sets
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"

//******************************************************************************************
//**
//** HUD Permission set display
//**
//******************************************************************************************

CHUDPermissions::CHUDPermissions()
{
	// Make this something new
	m_UpdateFlags = kHUDPermissions;

	m_hBaseTex = LTNULL;
	for (int i = 0; i < 8; i++)
	{
		m_hHaveTex[i] = LTNULL;
		m_hNeedTex[i] = LTNULL;
	}
	m_fScale = 0.0f;
}


LTBOOL CHUDPermissions::Init()
{
	UpdateLayout();

	m_nPermissions = 0;
	m_nRequiredPermissions = 0;

	// Load textures here
	g_pDrawPrim->SetRGBA(m_Base,argbWhite);
	g_pDrawPrim->SetUVWH( m_Base, 0.0f, 0.0f, 1.0f, 1.0f );
	m_hBaseTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\pset_base.dtx");

	for (int i = 0; i < 8; i++)
	{
		g_pDrawPrim->SetRGBA(&m_Have[i],argbWhite);
		g_pDrawPrim->SetRGBA(&m_Need[i],argbWhite);
		g_pDrawPrim->SetUVWH( &m_Have[i], 0.0f, 0.0f, 1.0f, 1.0f );
		g_pDrawPrim->SetUVWH( &m_Need[i], 0.0f, 0.0f, 1.0f, 1.0f );

		char buf[256];

		sprintf(buf, "interface\\hud\\pset_blue_%d.dtx",i+1);
		m_hHaveTex[i] = g_pInterfaceResMgr->GetTexture(buf);

		sprintf(buf, "interface\\hud\\pset_red_%d.dtx",i+1);
		m_hNeedTex[i] = g_pInterfaceResMgr->GetTexture(buf);
	}
	return LTTRUE;
}

void CHUDPermissions::Term()
{
	// Free the 17 textures
	// nothing to do here
}

void CHUDPermissions::Render()
{
	SetRenderState();
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

/*
	// Hack to test each piece
	static int timer = 0;
	static int cp = 0;
	static int cp2 = 4;
	timer++;
	if (timer > 60)
	{
		timer = 0;
		cp++;
		if (cp >= 8) cp = 0;
		m_nPermissions = 1<<cp;
		cp2++;
		if (cp2 >= 8) cp2 = 0;
		m_nRequiredPermissions = 1<<cp2;
	}
*/
/*
	// Special breakaway case.  No permissions required or owned.
	if (!m_nPermissions && !m_nRequiredPermissions)
		return;
*/
	// Render the base
	g_pDrawPrim->SetTexture(m_hBaseTex);
	g_pDrawPrim->DrawPrim(m_Base);

	// Render the correct pieces
	for (int i = 0; i < 8; i++)
	{
		// Do we have it?
		if (m_nPermissions & (1 << i))
		{
			// Render the piece
			g_pDrawPrim->SetTexture(m_hHaveTex[i]);
			g_pDrawPrim->DrawPrim(&m_Have[i]);
		}
		 // Did we need it?
		else if (m_nRequiredPermissions & (1 << i))
		{
			// Render the missing piece
			g_pDrawPrim->SetTexture(m_hNeedTex[i]);
			g_pDrawPrim->DrawPrim(&m_Need[i]);
		}
	}
}

void CHUDPermissions::Update()
{
	// Grab the new stats from PlayerStats
	g_pTronPlayerStats->GetPermissions(m_nPermissions, m_nRequiredPermissions);

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}
}

void CHUDPermissions::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);
	// Get the coords for placing the P-set
	// Get the offsets for the 8 pieces

	// Position all of the pieces relative to the xy of the base
//	for (int i = 0; i < 8; i++)
//	{
//	}
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();
	float xw = 32.0f;
	float yw = 32.0f;
	float x, y;
	x = (640.0f * xs) - 68;
	y = (480.0f * ys) - 62;

	g_pDrawPrim->SetXYWH( m_Base, x, y, 64, 64 );
	g_pDrawPrim->SetXYWH( &m_Have[0], x+32	, y+6	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Have[1], x+32	, y+14	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Have[2], x+32	, y+27	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Have[3], x+32	, y+27	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Have[4], x+14	, y+27	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Have[5], x+6	, y+27	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Have[6], x+6	, y+14	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Have[7], x+14	, y+6	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Need[0], x+32	, y+6	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Need[1], x+32	, y+14	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Need[2], x+32	, y+27	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Need[3], x+32	, y+27	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Need[4], x+14	, y+27	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Need[5], x+6	, y+27	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Need[6], x+6	, y+14	, xw, yw);
	g_pDrawPrim->SetXYWH( &m_Need[7], x+14	, y+6	, xw, yw);
}

