// ----------------------------------------------------------------------- //
//
// MODULE  : HUDCTFBase.cpp
//
// PURPOSE : HUDItem to display team CTF flag base status.
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDCTFBase.h"
#include "HUDMgr.h"
#include "InterfaceMgr.h"
#include "GameModeMgr.h"
#include "CTFFlagBaseSFX.h"
#include "CTFFlagSFX.h"

//******************************************************************************************
//**
//** HUD CTFBase Display
//**
//******************************************************************************************

CHUDCTFBase::CHUDCTFBase()
{
	m_eLevel = kHUDRenderDead;
	m_bUpdated = false;
	m_UpdateFlags = kHUDScores;
	m_bDraw = false;
	m_bHasFlag = false;
}

bool CHUDCTFBase::Init()
{
	UpdateLayout();
	ScaleChanged();

	EnableFade( false );

	return true;
}

void CHUDCTFBase::Term()
{
	m_bUpdated = false;
	m_bDraw = false;
	m_bHasFlag = false;
}

void CHUDCTFBase::Render()
{
	if( !m_bDraw || GameModeMgr::Instance( ).GetCTFRulesRecord() == NULL )
		return;

	SetRenderState();

	//render icon here
	if (m_hIconTexture)
	{
		g_pDrawPrim->SetTexture(m_hIconTexture);
		SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f, 0.0f, 1.0f, 1.0f);

		if( m_bHasFlag )
		{
			uint32 nColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDIconColor,0);
			DrawPrimSetRGBA(m_IconPoly,nColor);
		}
		else
		{
			uint32 nColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);
			DrawPrimSetRGBA(m_IconPoly,nColor);
		}

		g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	}

}

void CHUDCTFBase::Update()
{
	m_bDraw = false;

	if( GameModeMgr::Instance( ).GetCTFRulesRecord() == NULL )
		return;

	// Determine our teamid.
	if( IsFriendly( ))
	{
		m_nTeamId = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalTeam( );
	}
	else
	{
		m_nTeamId = !g_pInterfaceMgr->GetClientInfoMgr()->GetLocalTeam( );
	}

	CTeam* pTeam = CTeamMgr::Instance().GetTeam( m_nTeamId );
	if( !pTeam )
		return;

	if( !m_bUpdated )
	{
		m_bUpdated = true;
		UpdateLayout( );
	}

	// Iterate through the flagbases to find our team's flagbase.  Then check if it has the flag our not.
	for( CTFFlagBaseSFX::CTFFlagBaseSFXList::const_iterator iter = CTFFlagBaseSFX::GetCTFFlagBaseSFXList().begin( ); 
		iter != CTFFlagBaseSFX::GetCTFFlagBaseSFXList().end( ); iter++ )
	{
		CTFFlagBaseSFX* pCTFFlagBaseSFX = *iter;
		if( pCTFFlagBaseSFX->GetCS().m_nTeamId != m_nTeamId )
			continue;

		CTFFlagSFX* pCTFFlagSFX = pCTFFlagBaseSFX->GetFlag();
		if( !pCTFFlagSFX )
			continue;

		m_bHasFlag = pCTFFlagSFX->IsFlagInBase();
		break;
	}

	m_bDraw = true;
}

void CHUDCTFBase::ScaleChanged()
{
	CHUDItem::ScaleChanged();
}


void CHUDCTFBase::UpdateLayout()
{
	if( !m_bUpdated )
		return;

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord( GetLayoutRecordName( ));
	}

	CHUDItem::UpdateLayout();
}

void CHUDCTFBase::UpdateFade()
{
	CHUDItem::UpdateFade();
}

void CHUDCTFBase::UpdateFlicker()
{
	CHUDItem::UpdateFlicker();
}

void CHUDCTFBase::UpdateFlash()
{
	//not flashing, bail out
	if (!m_FlashTimer.GetEngineTimer().IsValid() || !m_FlashTimer.IsStarted())
		return;

	CHUDItem::UpdateFlash();
}


void CHUDCTFBase::EndFlicker()
{
	CHUDItem::EndFlicker();
}

void CHUDCTFBase::OnExitWorld()
{
	m_bUpdated = false;
	m_bHasFlag = false;
	m_hIconTexture.Free();

	CHUDItem::OnExitWorld();
}
