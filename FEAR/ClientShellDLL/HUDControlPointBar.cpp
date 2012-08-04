// ----------------------------------------------------------------------- //
//
// MODULE  : HUDControlPointBar.cpp
//
// PURPOSE : HUDItem to display hiding icon
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDControlPointBar.h"
#include "ControlPointSFX.h"
#include "CMoveMgr.h"
#include "ltintersect.h"

enum EHUDControlPointBar_AddTex
{
	kHUDControlPointBar_AddText_FriendlyBar = 0,
	kHUDControlPointBar_AddText_EnemyBar = 1,
	kHUDControlPointBar_AddText_FriendlyOwned = 2,
	kHUDControlPointBar_AddText_EnemyOwned = 3,
	kHUDControlPointBar_AddText_NeutralOwned = 4,
};

enum EHUDControlPointBar_AddRect
{
	kHUDControlPointBar_AddRect_FriendlyPos = 0,
	kHUDControlPointBar_AddRect_EnemyPos = 1,
	kHUDControlPointBar_AddRect_ControlPoint = 2,
};

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDControlPointBar::CHUDControlPointMeter::CHUDControlPointMeter
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDControlPointBar::CHUDControlPointMeter::CHUDControlPointMeter()
{
	m_bFriendlyLayout = true;
	m_fPercent = 0.0f;
	m_pHUDControlPointBar = NULL;
}

bool CHUDControlPointBar::CHUDControlPointMeter::Init( CHUDControlPointBar const& parentBar, bool bFriendlyLayout )
{
	m_pHUDControlPointBar = &parentBar;

	m_bFriendlyLayout = bFriendlyLayout;

	// Get our position.
	m_rectPosition = g_pLayoutDB->GetRect( parentBar.GetLayoutRecord(),LDB_HUDAddRect, bFriendlyLayout ? 
		kHUDControlPointBar_AddRect_FriendlyPos : kHUDControlPointBar_AddRect_EnemyPos );

	// Load up the texture to use.
	TextureReference hFrame(g_pLayoutDB->GetString(parentBar.GetLayoutRecord(), LDB_HUDAddTex, bFriendlyLayout ? 
		kHUDControlPointBar_AddText_FriendlyBar : kHUDControlPointBar_AddText_EnemyBar ));
	m_MeterBar.Init( hFrame );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDControlPointBar::CHUDControlPointMeter::Render
//
//  PURPOSE:	Draw the meter...
//
// ----------------------------------------------------------------------- //

void CHUDControlPointBar::CHUDControlPointMeter::Render()
{
	float x = (float)(m_rectPosition.Left()) + m_pHUDControlPointBar->GetBasePos().x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)(m_rectPosition.Top()) + m_pHUDControlPointBar->GetBasePos().y * g_pInterfaceResMgr->GetYRatio();

	float maxW = float(m_rectPosition.GetWidth());
	float w = m_fPercent * maxW;
	float h = float(m_rectPosition.GetHeight());

	m_MeterBar.Update( x, y, w, maxW, h );
	m_MeterBar.Render();
}


CHUDControlPointBar::CHUDControlPoint::CHUDControlPoint()
{
	m_eTeamId = INVALID_TEAM;
	m_nIndex = 0;
	m_pHUDControlPointBar = NULL;
}

bool CHUDControlPointBar::CHUDControlPoint::Init( CHUDControlPointBar const& parentBar )
{
	m_pHUDControlPointBar = &parentBar;

	m_Text.SetDropShadow(2);

	ScaleChanged();

	SetSourceString(LoadString("HUD_Ammo_Chars"));

	m_hLayout = parentBar.GetLayoutRecord();

	SetUseBasePosFromLayout( false );

	if( !Init( ))
		return false;

	UpdateLayout( );

	return true;
}


void CHUDControlPointBar::CHUDControlPoint::Render()
{
	//render icon here
	if (m_hIconTexture)
	{
		g_pDrawPrim->SetTexture(m_hIconTexture);
		g_pDrawPrim->DrawPrim(&m_IconPoly,1);
	}

	m_Text.Render();
}

void CHUDControlPointBar::CHUDControlPoint::UpdateLayout()
{
	if( !m_pHUDControlPointBar )
		return;

	CHUDItem::UpdateLayout();

	// Get our position.
	LTRect2n rectPosition = g_pLayoutDB->GetRect( m_pHUDControlPointBar->GetLayoutRecord(),LDB_HUDAddRect, kHUDControlPointBar_AddRect_ControlPoint );

	m_vBasePos = m_pHUDControlPointBar->GetBasePos();
	m_vIconOffset = rectPosition.GetTopLeft();
	m_vIconSize.Init( rectPosition.GetWidth(), rectPosition.GetHeight());

	for (uint32 i = 0; i < 3; ++i)
	{
		m_cTeamColor[i] = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,i);
		m_hTeamIcon[i].Load( g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,i+kHUDControlPointBar_AddText_FriendlyOwned) );
	}
}

void CHUDControlPointBar::CHUDControlPoint::SetIndex(uint16 nIndex)
{
	if (nIndex >= MAX_CONTROLPOINT_OBJECTS)
	{
		m_Text.SetText(L"");
		m_nIndex = 0;
	}
	else
	{
		wchar_t wsText[8] = L"";
		LTSNPrintF(wsText,LTARRAYSIZE(wsText),L"%d",nIndex);
		m_Text.SetText(wsText);
		m_nIndex = nIndex;
	}
}

void CHUDControlPointBar::CHUDControlPoint::SetTeam(ETeamId eTeamID)
{
	ETeamId eLocalID = ( ETeamId )g_pInterfaceMgr->GetClientInfoMgr()->GetLocalTeam( );

	if (eTeamID == INVALID_TEAM)
	{
		m_hIconTexture = m_hTeamIcon[1];
		m_cIconColor = m_cTeamColor[1];
	}
	else
	{
		if (eTeamID == eLocalID)
		{
			m_hIconTexture = m_hTeamIcon[0];
			m_cIconColor = m_cTeamColor[0];
		}
		else
		{
			m_hIconTexture = m_hTeamIcon[2];
			m_cIconColor = m_cTeamColor[2];
		}
	}

	if (m_hIconTexture)
	{
		SetupQuadUVs(m_IconPoly, m_hIconTexture, 0.0f,0.0f,1.0f,1.0f);
	}

	m_Text.SetColor(m_cTextColor);
	DrawPrimSetRGBA(m_IconPoly,m_cIconColor);

	if (eTeamID != m_eTeamId)
	{
		m_eTeamId = eTeamID;
		Flash("CPStatus");
	}
}


//******************************************************************************************
//**
//** CHUDControlPointBar display
//**
//******************************************************************************************

CHUDControlPointBar::CHUDControlPointBar()
{
	m_UpdateFlags = kHUDFrame;
	m_eLevel = kHUDRenderText;
	m_bDraw = false;
}


bool CHUDControlPointBar::Init()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDControlPointBar");
	}

	if( !CHUDItem::Init())
		return false;

	UpdateLayout();

	// Init our bar.
	m_FriendlyMeter.Init( *this, true );
	m_EnemyMeter.Init( *this, false );

	// Init the CP icon.
	m_ControlPoint.Init( *this );

	ScaleChanged();

	return true;
}

void CHUDControlPointBar::Term()
{
}

void CHUDControlPointBar::Render()
{
	if( !m_bDraw )
		return;

	SetRenderState();
	m_FriendlyMeter.Render();
	m_EnemyMeter.Render();
	m_ControlPoint.Render();
}

void CHUDControlPointBar::Update()
{
	m_bDraw = false;
	// Don't need to display if not mp.
	if( !IsMultiplayerGameClient())
		return;
	// Don't need to display if no cp's.
	ControlPointSFX::ControlPointSFXList const& list = ControlPointSFX::GetControlPointSFXList();
	if( list.size( ) == 0 )
		return;

	// Get the player info to test for intersection with the CP's.
	LTVector vPlayerPos;
	LTVector vPlayerDims;
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPlayerPos);
	g_pPhysicsLT->GetObjectDims(g_pMoveMgr->GetObject(), &vPlayerDims);
	LTRect3f rectPlayer( vPlayerPos - vPlayerDims, vPlayerPos + vPlayerDims );

	// Iterate through the list and draw the first one we find we are within.
	LTVector vCPPos;
	for( ControlPointSFX::ControlPointSFXList::const_iterator iter = list.begin(); iter != list.end( ); iter++ )
	{
		ControlPointSFX const* pCPSFX = *iter;

		// Calculate the CP's AABB.
		g_pLTClient->GetObjectPos(pCPSFX->GetServerObj(), &vCPPos);
		LTVector const& vCPDims = pCPSFX->GetCS().m_vZoneDims;
		LTRect3f rectCP( vCPPos - vCPDims, vCPPos + vCPDims );

		// Check if AABB from player and CP intersect.
		if( LTIntersect::AABB_AABB( rectPlayer, rectCP ))
		{
			// Use this CP's control level as the level of our meter.
			float fControlLevel = pCPSFX->GetControlLevel();

			bool bLocalTeam0 = g_pInterfaceMgr->GetClientInfoMgr( )->GetLocalTeam( ) == kTeamId0;

			// Convert this to an integer for the meter.  We're rounding up with values
			// greater than .5 here rather than rounding toward the owner, since
			// this bar only shows 0 being neutral and 100% being owned.
			float fAbsControlLevel = fabs( fControlLevel );

			// Handle control on the friendly side.
			if(( fControlLevel < 0.0f && bLocalTeam0 ) || ( fControlLevel >= 0.0f && !bLocalTeam0 ))
			{
				m_FriendlyMeter.SetPercent( fAbsControlLevel );
				// For enemybar, Bar goes down to 0% when control goes to 100%.
				m_EnemyMeter.SetPercent( 1.0f );
			}
			// Handle control on the enemy side.
			else
			{
				m_FriendlyMeter.SetPercent( 0.0f );
				// For enemybar, Bar goes down to 0% when control goes to 100%.
				m_EnemyMeter.SetPercent( 1.0f - fAbsControlLevel );
			}

			m_ControlPoint.SetTeam( pCPSFX->GetTeamId());
			m_ControlPoint.SetIndex( pCPSFX->GetCS().m_nControlPointId );

			// Flag we should be drawn.
			m_bDraw = true;
		}
	}
}

void CHUDControlPointBar::ScaleChanged()
{
	CHUDItem::ScaleChanged();
	m_ControlPoint.ScaleChanged();
}

void CHUDControlPointBar::UpdateLayout()
{
	CHUDItem::UpdateLayout();
	m_ControlPoint.UpdateLayout();
}
