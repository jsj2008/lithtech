// ----------------------------------------------------------------------- //
//
// MODULE  : HUDInstinct.h
//
// PURPOSE : HUDItem to display the current instinct level.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDInstinct.h"
#include "HUDMgr.h"
#include "ForensicObjectFX.h"
#include "CMoveMgr.h"

//******************************************************************************************

static const int g_nInstinctPosX = 0;
static const int g_nInstinctPosY = 0;
static const int g_nInstinctWidth = 128;
static const int g_nInstinctHeight = 128;

//******************************************************************************************

struct InstinctLevelProps
{
	const char* sTexture;
	float	fRotation;
	float	fPulse;
};

static const InstinctLevelProps s_aInstinctLevelProps[ INSTINCT_LEVELS ] =
{
	{ "global\\ui\\instinct03.dds", ( MATH_PI / 3.0f ), -1.0f },
	{ "global\\ui\\instinct04.dds", 0.0f, 2.0f },
};

static InstinctLevelProps s_aInstinctLevelValues[ INSTINCT_LEVELS ];

//******************************************************************************************

CHUDInstinct::CHUDInstinct()
{
	m_UpdateFlags = kHUDFrame;

	for( int i = 0; i < INSTINCT_LEVELS; ++i )
	{
		m_hInstinctLevels[ i ] = NULL;
	}

	m_fCurrInstinctLevel = 0.0f;
	m_fDestInstinctLevel = 0.0f;

	UpdateLayout();
}

//******************************************************************************************

bool CHUDInstinct::Init()
{
	float fXRatio = g_pInterfaceResMgr->GetXRatio();
	float fYRatio = g_pInterfaceResMgr->GetYRatio();

	for( int i = 0; i < INSTINCT_LEVELS; ++i )
	{
		m_hInstinctLevels[ i ].Load( s_aInstinctLevelProps[ i ].sTexture );

		DrawPrimSetXYWH( m_Rects[ i ], ( g_nInstinctPosX * fXRatio ), ( g_nInstinctPosY * fYRatio ), ( g_nInstinctWidth * fXRatio ), ( g_nInstinctHeight * fYRatio ) );
		DrawPrimSetRGBA( m_Rects[ i ], argbWhite );
		SetupQuadUVs( m_Rects[ i ], m_hInstinctLevels[ i ], 0.0f, 0.0f, 1.0f, 1.0f );
	}

	return true;
}

//******************************************************************************************

void CHUDInstinct::Term()
{
}

//******************************************************************************************

void CHUDInstinct::Render()
{
	// used for debugging only
	m_Text.Render();

/* pulsing-eye render

	if( m_fCurrInstinctLevel > 0.0f )
	{
		SetRenderState();

		GameDifficulty eDiff = g_pGameClientShell->GetDifficulty();

		if (eDiff == GD_EASY)
		{
			float fStep = ( 1.0f / ( INSTINCT_LEVELS - 1 ) );
			float fCurr = 0.0f;
			float fNext = fStep;

			for( int i = 0; i < ( INSTINCT_LEVELS - 1 ); ++i, fCurr += fStep, fNext += fStep )
			{
				if( m_fCurrInstinctLevel <= fCurr )
				{
					continue;
				}
				else if( m_fCurrInstinctLevel >= fNext )
				{
					float fPulse = 1.0f;

					if( s_aInstinctLevelValues[ i ].fPulse != -1.0f )
					{
						fPulse = ( s_aInstinctLevelValues[ i ].fPulse > 1.0f ) ? ( 1.0f - ( s_aInstinctLevelValues[ i ].fPulse - 1.0f ) ) : ( s_aInstinctLevelValues[ i ].fPulse );
					}

					DrawPrimSetAlpha( m_Rects[ i ], ( uint8 )( fPulse * 255.0f ) );
				}
				else
				{
					float fAlpha = ( ( m_fCurrInstinctLevel - fCurr ) / fStep );
					float fPulse = 1.0f;

					if( s_aInstinctLevelValues[ i ].fPulse != -1.0f )
					{
						fPulse = ( s_aInstinctLevelValues[ i ].fPulse > 1.0f ) ? ( 1.0f - ( s_aInstinctLevelValues[ i ].fPulse - 1.0f ) ) : ( s_aInstinctLevelValues[ i ].fPulse );
					}

					DrawPrimSetAlpha( m_Rects[ i ], ( uint8 )( fPulse * 255.0f * fAlpha ) );
				}

				g_pDrawPrim->SetTexture( m_hInstinctLevels[ i ] );
				g_pDrawPrim->DrawPrim( &m_Rects[ i ], 1 );
			}


			// Draw the final level if we're at full instinct
			if( m_fCurrInstinctLevel >= 1.0f )
			{
				float fPulse = 1.0f;
				int nIndex = INSTINCT_LEVELS - 1;

				if( s_aInstinctLevelValues[ nIndex ].fPulse != -1.0f )
				{
					fPulse = ( s_aInstinctLevelValues[ nIndex ].fPulse > 1.0f ) ? ( 1.0f - ( s_aInstinctLevelValues[ nIndex ].fPulse - 1.0f ) ) : ( s_aInstinctLevelValues[ nIndex ].fPulse );
				}

				DrawPrimSetAlpha( m_Rects[ i ], ( uint8 )( fPulse * 255.0f ) );

				g_pDrawPrim->SetTexture( m_hInstinctLevels[ nIndex ] );
				g_pDrawPrim->DrawPrim( &m_Rects[ nIndex ], 1 );
			}
		}
		else	// only activate within the core radius when not on easy.
		{
			if( m_fDestInstinctLevel >= 1.0f )
			{
				float fAlpha = (m_fCurrInstinctLevel / m_fDestInstinctLevel);

				for( int i = 0; i < INSTINCT_LEVELS; ++i )
				{
					float fPulse = 1.0f;

					if( s_aInstinctLevelValues[ i ].fPulse != -1.0f )
					{
						fPulse = ( s_aInstinctLevelValues[ i ].fPulse > 1.0f ) ? ( 1.0f - ( s_aInstinctLevelValues[ i ].fPulse - 1.0f ) ) : ( s_aInstinctLevelValues[ i ].fPulse );
					}

					DrawPrimSetAlpha( m_Rects[ i ], ( uint8 )( fPulse * 255.0f * fAlpha ) );

					g_pDrawPrim->SetTexture( m_hInstinctLevels[ i ] );
					g_pDrawPrim->DrawPrim( &m_Rects[ i ], 1 );
				}
			}
		}
	}
*/
}

//******************************************************************************************

void CHUDInstinct::Update()
{
	float fFrameTime = g_pLTClient->GetFrameTime();


	// Initialize our destination instinct level to zero...
	m_fDestInstinctLevel = 0.0f;


	// Update Instinct Level based on the player's current position.
	CForensicObjectFX* pFX = g_pPlayerMgr->GetForensicObject();
	if( pFX )
	{
		LTVector vClientPos;
		HOBJECT hClientObj = g_pLTClient->GetClientObject();
		g_pLTClient->GetObjectPos( hClientObj, &vClientPos );

		float fDist = pFX->GetDistance(g_pPlayerMgr->GetMoveMgr()->GetServerObject());
		if( (fDist > 0.0f) && (fDist <= pFX->m_cs.m_fMaxDistance) )
		{
			fDist -= pFX->m_cs.m_fCoreRadius;
			float fMaxDist = (pFX->m_cs.m_fMaxDistance - pFX->m_cs.m_fCoreRadius);	// max distance is guaranteed to be larger than core radius (see ForensicObject.cpp)

			float fLevel = (fDist <= 0.0f) ? 1.0f : ( 1.0f - ( fDist / fMaxDist ) );
			if( fLevel > m_fDestInstinctLevel )
			{
				m_fDestInstinctLevel = fLevel;
			}
		}
	}


	// Interpolate the instinct level
	if( m_fCurrInstinctLevel > m_fDestInstinctLevel )
	{
		m_fCurrInstinctLevel -= fFrameTime;

		if( m_fCurrInstinctLevel < m_fDestInstinctLevel )
		{
			m_fCurrInstinctLevel = m_fDestInstinctLevel;
		}
	}
	else if( m_fCurrInstinctLevel < m_fDestInstinctLevel )
	{
		m_fCurrInstinctLevel += fFrameTime;

		if( m_fCurrInstinctLevel > m_fDestInstinctLevel )
		{
			m_fCurrInstinctLevel = m_fDestInstinctLevel;
		}
	}

	// used for debugging only
	wchar_t wszHudText[64];
	LTSNPrintF( wszHudText, LTARRAYSIZE(wszHudText), L"Instinct: %6.2f", m_fCurrInstinctLevel );
	m_Text.SetText( wszHudText );

	/* pulsing-eye update

	// Update the properties
	if( m_fCurrInstinctLevel <= 0.0f )
	{
		return;
	}

	LTVector vOffset( ( g_nInstinctWidth * 0.5f ), ( g_nInstinctHeight * 0.5f ), 0.0f );
	LTVector vCenter( ( g_nInstinctPosX + vOffset.x ), ( g_nInstinctPosY + vOffset.y ), 0.0f );

	for( int i = 0; i < INSTINCT_LEVELS; ++i )
	{
		// Handle rotation...
		s_aInstinctLevelValues[ i ].fRotation += ( fFrameTime * s_aInstinctLevelProps[ i ].fRotation );
		LTCLAMP( s_aInstinctLevelValues[ i ].fRotation, 0.0f, MATH_TWOPI );

		LTVector vR( cosf( s_aInstinctLevelValues[ i ].fRotation ) * vOffset.x, sinf( s_aInstinctLevelValues[ i ].fRotation ) * vOffset.y, 0.0f );
		LTVector vU( -vR.y, vR.x, 0.0f );

		m_Rects[ i ].verts[ 0 ].pos = ( vCenter - vR - vU );
		m_Rects[ i ].verts[ 1 ].pos = ( vCenter + vR - vU );
		m_Rects[ i ].verts[ 2 ].pos = ( vCenter + vR + vU );
		m_Rects[ i ].verts[ 3 ].pos = ( vCenter - vR + vU );


		// Handle pulse...
		if( s_aInstinctLevelProps[ i ].fPulse == -1.0f )
		{
			s_aInstinctLevelValues[ i ].fPulse = -1.0f;
		}
		else
		{
			s_aInstinctLevelValues[ i ].fPulse += ( fFrameTime * s_aInstinctLevelProps[ i ].fPulse );
			s_aInstinctLevelValues[ i ].fPulse = fmod( s_aInstinctLevelValues[ i ].fPulse, 2.0f );
		}
	}
	*/

}

void CHUDInstinct::UpdateLayout()
{
	//if we haven't initialized our layout info

	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDInstinct");
	}

	CHUDItem::UpdateLayout();
}
