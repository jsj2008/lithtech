// ----------------------------------------------------------------------- //
//
// MODULE  : TO2HUDMgr.cpp
//
// PURPOSE : Implementation of CTO2HUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "SurfaceFunctions.h"
#include "CharacterFX.h"

extern CGameClientShell* g_pGameClientShell;

CHUDCrosshair*		g_pCrosshair = LTNULL;
CHUDCompass*		g_pCompass = LTNULL;
CHUDObjectives*		g_pObjectives = LTNULL;
CHUDRadio*			g_pRadio = LTNULL;

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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2HUDMgr::CTO2HUDMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTO2HUDMgr::CTO2HUDMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2HUDMgr::~CTO2HUDMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTO2HUDMgr::~CTO2HUDMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2HUDMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CTO2HUDMgr::Init()
{
	//crosshair should be first so that it is rendered first (potential overlap)
	m_itemArray.push_back(&m_Crosshair);

	m_itemArray.push_back(&m_Ammo);
	m_itemArray.push_back(&m_Air);
	m_itemArray.push_back(&m_Compass);
	m_itemArray.push_back(&m_Hiding);
	m_itemArray.push_back(&m_Carrying);
	m_itemArray.push_back(&m_Health);
	m_itemArray.push_back(&m_DamageDir);
	m_itemArray.push_back(&m_Distance);
	m_itemArray.push_back(&m_Objectives);
	m_itemArray.push_back(&m_Radio);
	m_itemArray.push_back(&m_ProgressBar);
	m_itemArray.push_back(&m_Respawn);
	m_itemArray.push_back(&m_Doomsday);


	g_pCrosshair = &m_Crosshair;
	g_pCompass = &m_Compass;
	g_pObjectives = &m_Objectives;
	g_pRadio = &m_Radio;

	m_itemArray.push_back(&m_WpnChooser);
	m_itemArray.push_back(&m_AmmoChooser);

    return CHUDMgr::Init();
}
