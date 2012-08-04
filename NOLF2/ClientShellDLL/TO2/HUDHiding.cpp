// ----------------------------------------------------------------------- //
//
// MODULE  : HUDHiding.cpp
//
// PURPOSE : HUDItem to display hiding icon
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "TO2HUDMgr.h"
#include "TO2PlayerStats.h"
#include "TO2InterfaceMgr.h"
#include "GameClientShell.h"

extern VarTrack g_vtProgressBarScaleToSkills;

//******************************************************************************************
//**
//** HUD Hiding display
//**
//******************************************************************************************

CHUDHiding::CHUDHiding()
{
	m_UpdateFlags = kHUDHiding;
	m_bDraw = LTFALSE;
	m_fHideTimer = 0.0f;
	m_fHideDuration = 0.0f;
	m_fHideBarWaitTime = 0.0f;

	for( int i = 0; i < kNumPrims; ++i )
	{
		m_hIcon[i] = LTNULL;
	}
}


LTBOOL CHUDHiding::Init()
{
	//Hiding icon
	m_hIcon[kHidePrim] = g_pInterfaceResMgr->GetTexture("interface\\hud\\hide.dtx");
	g_pDrawPrim->SetRGBA(&m_Poly[kHidePrim],argbWhite);
	SetupQuadUVs(m_Poly[kHidePrim],m_hIcon[kHidePrim],0.0f,0.0f,1.0f,1.0f);

	//Hidden icon
	m_hIcon[kHiddenPrim] = g_pInterfaceResMgr->GetTexture( "interface\\hud\\hidden.dtx" );
	g_pDrawPrim->SetRGBA( &m_Poly[kHiddenPrim], argbWhite );
	SetupQuadUVs(m_Poly[kHiddenPrim], m_hIcon[kHiddenPrim], 0.0f, 0.0f, 1.0f, 1.0f );
	
	// Cant hide icon
	m_hIcon[kCantHidePrim] = g_pInterfaceResMgr->GetTexture( "interface\\hud\\hideno.dtx" );
	g_pDrawPrim->SetRGBA( &m_Poly[kCantHidePrim], argbWhite );
	SetupQuadUVs( m_Poly[kCantHidePrim], m_hIcon[kCantHidePrim], 0.0f, 0.0f, 1.0f, 1.0f );

	UpdateLayout();

	// Init our hiding bar...
	
	m_HideBar.Init();

	return LTTRUE;
}

void CHUDHiding::Term()
{

}

void CHUDHiding::Render()
{
	if (!m_bDraw) return;

	
	SetRenderState();
	
	if( g_pPlayerStats->CanHide() )
	{
		if( !g_pPlayerStats->IsHidden() )
		{
			if (m_fHideDuration <= 0.0f ) return;

			float fAlpha = Clamp((m_fHideTimer / m_fHideDuration), 0.0f, 1.0f);

			uint8 a = (uint8)( 255.0f * fAlpha );

			uint32 color = SET_ARGB(a,0xFF,0xFF,0xFF);
			g_pDrawPrim->SetRGBA( &m_Poly[kHiddenPrim], color );

			a = 255 - a;
			color = SET_ARGB(a,0xFF,0xFF,0xFF);
			g_pDrawPrim->SetRGBA(&m_Poly[kHidePrim],color);
		}
		else
		{
			uint32 color = SET_ARGB(0,0xFF,0xFF,0xFF);
			g_pDrawPrim->SetRGBA( &m_Poly[kHiddenPrim], color );
			
			color = SET_ARGB(255,0xFF,0xFF,0xFF);
			g_pDrawPrim->SetRGBA(&m_Poly[kHidePrim],color);
		}

		// draw our icons
		g_pDrawPrim->SetTexture(m_hIcon[kHidePrim]);
		g_pDrawPrim->DrawPrim(&m_Poly[kHidePrim]);

		// draw our icons
		g_pDrawPrim->SetTexture(m_hIcon[kHiddenPrim]);
		g_pDrawPrim->DrawPrim(&m_Poly[kHiddenPrim]);

		
		// Update our hiding bar...
		
		if( !g_pPlayerStats->IsHidden() )
		{
			if (!g_pGameClientShell->IsGamePaused())
			{
				m_fHideTimer -= g_pLTClient->GetFrameTime();
				if( m_fHideTimer <= 0.0f )
				{
					m_HideBar.SetMaxValue( 0 );
					m_HideBar.SetValue( 0 );
				}
				else
				{
					float fStealthSkillEffect = g_pPlayerStats->GetSkillModifier(SKL_STEALTH,StealthModifiers::eHideTime);
					
					uint8 nMaxProgress = 100;
					if( g_vtProgressBarScaleToSkills.GetFloat() > 0.0f )
					{
						nMaxProgress = (fStealthSkillEffect > 0.0f ? uint8(100 * fStealthSkillEffect) : 0);
					}

					uint8 nVal = uint8( (1 - (m_fHideTimer / m_fHideDuration)) * nMaxProgress );

					m_HideBar.SetMaxValue( nMaxProgress );
					m_HideBar.SetValue( nVal );
				}
			}

			if( m_fHideBarWaitTime > 0.0f )
			{
				if (!g_pGameClientShell->IsGamePaused())
					m_fHideBarWaitTime += g_pLTClient->GetFrameTime();
				if( m_fHideBarWaitTime >= 0.5f )
				{
					m_HideBar.Update();
					m_HideBar.Render();
				}
			}
		}
	}
	else
	{
		g_pDrawPrim->SetTexture(m_hIcon[kCantHidePrim]);
		g_pDrawPrim->DrawPrim(&m_Poly[kCantHidePrim]);
	}

}

void CHUDHiding::Update()
{
	m_bDraw = g_pPlayerStats->IsHiding() || g_pPlayerStats->IsHidden();
	if (!m_bDraw)
	{
		m_fHideDuration = m_fHideTimer = m_fHideBarWaitTime = 0.0f;

		return;
	}
	
	float x = (float)m_BasePos.x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)m_BasePos.y * g_pInterfaceResMgr->GetYRatio();

	float w = (float)m_BaseSize.x * g_pInterfaceResMgr->GetXRatio();
	float h = (float)m_BaseSize.y * g_pInterfaceResMgr->GetYRatio();

	g_pDrawPrim->SetXYWH(&m_Poly[kHidePrim],x,y,w,h);
	g_pDrawPrim->SetXYWH(&m_Poly[kHiddenPrim],x,y,w,h);
	g_pDrawPrim->SetXYWH(&m_Poly[kCantHidePrim],x,y,w,h);

	if (g_pPlayerStats->IsHidden())
	{
		m_fHideTimer = 0.0f; 
		m_fHideBarWaitTime = -1.0f;
	}
	else if (g_pPlayerStats->IsHiding())
	{
		m_fHideDuration = m_fHideTimer = g_pPlayerStats->GetHideDuration(); 
		m_fHideBarWaitTime = g_pLTClient->GetFrameTime();
	}
}

void CHUDHiding::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);

	m_BasePos		= g_pLayoutMgr->GetHideIconPos(nCurrentLayout);
	m_BaseSize		= g_pLayoutMgr->GetHideIconSize(nCurrentLayout);
}

