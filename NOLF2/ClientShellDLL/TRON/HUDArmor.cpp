// ----------------------------------------------------------------------- //
//
// MODULE  : HUDArmor.cpp
//
// PURPOSE : HUDItem to display Jet's current armor
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"
#include "TRONPlayerStats.h"
#include "TronInterfaceMgr.h"

namespace
{
	char * szPieceTex[] =
	{
		"Interface\\hud\\protection_submask.dtx",
		"Interface\\hud\\protection_truncate_plate.dtx",
		"Interface\\hud\\protection_peripheral_seal.dtx",
		"Interface\\hud\\protection_support_safeguard.dtx",
		"Interface\\hud\\protection_base_damping.dtx",
		"Interface\\hud\\protection_viral_shield.dtx",
	};
}

//******************************************************************************************
//**
//** HUD Armor display
//**
//******************************************************************************************

CHUDArmor::CHUDArmor()
{
	// Make this something new
	m_UpdateFlags = kHUDArmor;

	m_hBaseTex = LTNULL;
	for (int i = 0; i < 6; i++)
	{
		m_hArmorPieceTex[i] = LTNULL;
	}
	m_fScale = 0.0f;
}


LTBOOL CHUDArmor::Init()
{
	UpdateLayout();

	// Load textures here
	g_pDrawPrim->SetRGBA(&m_Base,argbWhite);
	g_pDrawPrim->SetUVWH(&m_Base, 0.0f, 0.0f, 1.0f, 1.0f );
	m_hBaseTex = g_pInterfaceResMgr->GetTexture("interface\\hud\\protection_base.dtx");

	for (int i = 0; i < 6; i++)
	{
		g_pDrawPrim->SetRGBA(&m_ArmorPiece[i],argbWhite);
		g_pDrawPrim->SetUVWH( &m_ArmorPiece[i], 0.0f, 0.0f, 1.0f, 1.0f );

		m_hArmorPieceTex[i] = g_pInterfaceResMgr->GetTexture(szPieceTex[i]);
	}
	return LTTRUE;
}

void CHUDArmor::Term()
{
	// nothing to do here
}

void CHUDArmor::Render()
{
	SetRenderState();

	// Render the base
	g_pDrawPrim->SetTexture(m_hBaseTex);
	g_pDrawPrim->DrawPrim(&m_Base);

	// Render the correct pieces
	for (int i = 0; i < 6; i++)
	{
		// Do we have it?
		if (m_bHavePiece[i])
		{
			// Render the piece
			g_pDrawPrim->SetTexture(m_hArmorPieceTex[i]);
			g_pDrawPrim->DrawPrim(&m_ArmorPiece[i]);
		}
	}
}

void CHUDArmor::Update()
{
	for(int i = 0; i < 6; i++)
	{
		m_bHavePiece[i] = g_pSubroutineMgr->HaveArmorPiece(i);
	}

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		UpdateLayout();
		m_fScale = g_pInterfaceResMgr->GetXRatio();
	}
}

void CHUDArmor::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	// Position all of the pieces relative to the xy of the base
	float xs = g_pInterfaceResMgr->GetXRatio();
	float ys = g_pInterfaceResMgr->GetYRatio();

	float x = 0.0f;// * xs;
	float y = (480.0f * ys) - 64.0f;

	float w = 64.0f;// * xs;
	float h = 64.0f;// * ys;

	g_pDrawPrim->SetXYWH( &m_Base, x, y, w, h);

	for (int i = 0; i < 6; i++)
	{
		g_pDrawPrim->SetXYWH( &m_ArmorPiece[i], x, y, w, h);
	}
}

