// ----------------------------------------------------------------------- //
//
// MODULE  : TronHUDMgr.cpp
//
// PURPOSE : Implementation of CTronHUDMgr class
//
// CREATED : 11/5/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TronHUDMgr.h"

#include "TRONPlayerStats.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"

extern CGameClientShell* g_pGameClientShell;

inline void CHUDItem::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
}

CHUDCrosshair*		g_pCrosshair = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::CTronHUDMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronHUDMgr::CTronHUDMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::~CTronHUDMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronHUDMgr::~CTronHUDMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronHUDMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CTronHUDMgr::Init()
{
	//crosshair should be first so that it is rendered first (potential overlap)
	m_itemArray.push_back(&m_Crosshair);

	m_itemArray.push_back(&m_Weapons);
	m_itemArray.push_back(&m_Version);

	// Tron-specific items
	m_itemArray.push_back(&m_Permissions);
	m_itemArray.push_back(&m_HealthEnergy);

	m_itemArray.push_back(&m_Armor);
	m_itemArray.push_back(&m_EnergyTrans);
	m_itemArray.push_back(&m_Progress);

	m_itemArray.push_back(&m_Weapon);
	m_itemArray.push_back(&m_WpnChooser);
	m_itemArray.push_back(&m_AmmoChooser);

	g_pCrosshair = &m_Crosshair;

    return CHUDMgr::Init();
}

