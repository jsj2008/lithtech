// ----------------------------------------------------------------------- //
//
// MODULE  : HUDDamage.cpp
//
// PURPOSE : HUDItem to display player air meter
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "PlayerStats.h"
#include "DamageFxMgr.h"
#include "InterfaceMgr.h"

//******************************************************************************************
//**
//** HUD Air display
//**
//******************************************************************************************

CHUDDamage::CHUDDamage()
{
	m_UpdateFlags = kHUDDamage;
	m_hIcon = LTNULL;
	m_Poly	= LTNULL;
	m_bDraw = LTTRUE;
	m_nIconHt = 0;
}

CHUDDamage::~CHUDDamage()
{
	debug_deletea( m_hIcon );
	debug_deletea( m_Poly );
}


LTBOOL CHUDDamage::Init()
{
	if( !g_pDamageFXMgr )
		return LTFALSE;

	int nNumDmgFX = g_pDamageFXMgr->GetNumDamageFX();
	
	// Alloc the texture and poly arrays

	m_hIcon = debug_newa( HTEXTURE, nNumDmgFX );
	m_Poly	= debug_newa( LTPoly_GT4, nNumDmgFX );

	for( int i = 0; i < nNumDmgFX; ++i )
	{
		m_hIcon[i] = g_pInterfaceResMgr->GetTexture( g_pDamageFXMgr->GetDamageFX( i )->m_szIcon );

		g_pDrawPrim->SetRGBA( &m_Poly[i], argbWhite );
		SetupQuadUVs( m_Poly[i], m_hIcon[i], 0.0f, 0.0f, 1.0f, 1.0f );
	}

	UpdateLayout();
	return LTTRUE;
}


void CHUDDamage::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstActiveFX();
	while( pDamageFX )
	{
		g_pDrawPrim->SetTexture( m_hIcon[pDamageFX->m_nID] );
		g_pDrawPrim->DrawPrim( &m_Poly[pDamageFX->m_nID], 1 );
	
		pDamageFX = g_pDamageFXMgr->GetNextActiveFX();
	}
}


void CHUDDamage::Update()
{
	if (!m_bDraw) return;

	float x = (float)m_BasePos.x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)m_BasePos.y * g_pInterfaceResMgr->GetYRatio();
	float sz = m_nIconHt * g_pInterfaceResMgr->GetYRatio();

	uint32 nNumDamageFX = g_pDamageFXMgr->GetNumDamageFX();
	bool *aDamageSet = (bool*)alloca(nNumDamageFX);
	memset(aDamageSet, 0, sizeof(aDamageSet[0]) * nNumDamageFX); 

	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstActiveFX();
	while( pDamageFX )
	{
		if (!aDamageSet[pDamageFX->m_nID])
		{
			g_pDrawPrim->SetXYWH( &m_Poly[pDamageFX->m_nID], x, y, sz, sz );
			y += sz;
			aDamageSet[pDamageFX->m_nID] = true;
		}
	
		pDamageFX = g_pDamageFXMgr->GetNextActiveFX();
	}
}

void CHUDDamage::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetDamageBasePos(nCurrentLayout);
	m_nIconHt		= g_pLayoutMgr->GetDamageIconSize(nCurrentLayout);
}

