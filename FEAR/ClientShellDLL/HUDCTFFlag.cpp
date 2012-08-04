// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCTFFlag.h
//
// PURPOSE : HUDItem to display player has flag.
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDCTFFlag.h"
#include "HUDMgr.h"
#include "InterfaceMgr.h"
#include "GameModeMgr.h"

//******************************************************************************************
//**
//** HUD CTFFlag Display
//**
//******************************************************************************************

CHUDCTFFlag::CHUDCTFFlag()
{
	m_UpdateFlags = kHUDNone;
	m_bHasFlag = false;
	m_eLevel = kHUDRenderDead;
	m_bUpdated = false;
}

bool CHUDCTFFlag::Init()
{
	UpdateLayout();
	ScaleChanged();

	m_Text.SetDropShadow(2);
	EnableFade( false );

	return true;
}

void CHUDCTFFlag::Term()
{
	m_bUpdated = false;
	m_bHasFlag = false;
}

void CHUDCTFFlag::Render()
{
	if( GameModeMgr::Instance( ).GetCTFRulesRecord() == NULL )
		return;

	if( !m_bHasFlag )
		return;

	SetRenderState();

	//render icon here
	if (m_hIconTexture)
	{
		g_pDrawPrim->SetTexture(m_hIconTexture);
		SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f);
		g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	}
}

void CHUDCTFFlag::Update()
{
	if( GameModeMgr::Instance( ).GetCTFRulesRecord() == NULL )
		return;

	if( !m_bHasFlag )
		return;

	if( !m_bUpdated )
	{
		m_bUpdated = true;
		UpdateLayout( );
	}

	ResetFade();
}

void CHUDCTFFlag::ScaleChanged()
{
	CHUDItem::ScaleChanged();
}


void CHUDCTFFlag::UpdateLayout()
{
	if( !m_bUpdated )
		return;

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDCTFFlag");
	}

	CHUDItem::UpdateLayout();
}

void CHUDCTFFlag::UpdateFade()
{
	CHUDItem::UpdateFade();
}

void CHUDCTFFlag::UpdateFlicker()
{
	CHUDItem::UpdateFlicker();
}

void CHUDCTFFlag::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	CHUDItem::UpdateFlash();
}


void CHUDCTFFlag::EndFlicker()
{
	CHUDItem::EndFlicker();
}

void CHUDCTFFlag::OnExitWorld()
{
	m_bUpdated = false;
	m_bHasFlag = false;
	m_hIconTexture.Free();

	CHUDItem::OnExitWorld();
}
