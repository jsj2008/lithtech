// ----------------------------------------------------------------------- //
//
// MODULE  : HUDSpectator.cpp
//
// PURPOSE : HUDItem to spectator information
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "HUDSpectator.h"
#include "CharacterFX.h"
#include "CommandIDs.h"
#include "MissionMgr.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDSpectator::CHUDSpectator
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CHUDSpectator::CHUDSpectator()
:	CHUDItem		( )
{
	m_eLevel = kHUDRenderText;
	m_UpdateFlags = kHUDSpectator;
	m_pos.Init();
	m_eLastSpectatorMode = eSpectatorMode_None;
	m_pLastCharFx = NULL;
	m_bLastCanRespawn = false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDSpectator::CHUDSpectator
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CHUDSpectator::~CHUDSpectator()
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDSpectator::Init
//
//  PURPOSE:	Setup the timer...
//
// ----------------------------------------------------------------------- //

bool CHUDSpectator::Init()
{
	m_eLastSpectatorMode = eSpectatorMode_None;
	m_pLastCharFx = NULL;
	m_bLastCanRespawn = false;

	UpdateLayout();

	Update();

	m_Text.SetAlignment(kCenter);

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDSpectator::Render
//
//  PURPOSE:	Draw the meter...
//
// ----------------------------------------------------------------------- //

void CHUDSpectator::Render()
{
	if( !g_pPlayerMgr->IsSpectating() || 
		( g_pMissionMgr->GetServerGameState() != EServerGameState_Playing && 
		g_pMissionMgr->GetServerGameState() != EServerGameState_PlayingSuddenDeath ))
		return;

	SetRenderState();
	m_Text.Render();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDSpectator::Update
//
//  PURPOSE:	Set the values for the bar to draw...
//
// ----------------------------------------------------------------------- //

void CHUDSpectator::Update()
{
	if( !g_pPlayerMgr )
		return;

	bool bUpdate = false;

	if( m_eLastSpectatorMode != g_pPlayerMgr->GetSpectatorMode( ))
	{
		bUpdate = true;
		m_eLastSpectatorMode = g_pPlayerMgr->GetSpectatorMode( );
	}

	bool bCanRespawn =  g_pPlayerMgr->CanRespawn( );
	if( m_bLastCanRespawn != bCanRespawn )
	{
		bUpdate = true;
		m_bLastCanRespawn = bCanRespawn;
	}

	wchar_t wszModeText[256] = L"";
	switch( g_pPlayerMgr->GetSpectatorMode())
	{
	case eSpectatorMode_Tracking:
		{
			wchar_t wszName[256] = L"";
			HOBJECT hTarget = g_pPlayerMgr->GetSpectatorTrackTarget( );

			// Check if the target changed.
			if( hTarget != m_hLastTarget )
			{
				m_hLastTarget = hTarget;
				m_pLastCharFx = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( hTarget );
				bUpdate = true;
			}

			if( bUpdate && m_pLastCharFx && m_pLastCharFx->m_cs.nClientID != INVALID_CLIENT )
			{
				LTStrCpy( wszName, g_pInterfaceMgr->GetClientInfoMgr()->GetPlayerName( m_pLastCharFx->m_cs.nClientID ), 
					LTARRAYSIZE( wszName ));
			}

			if( bUpdate )
				FormatString( "IDS_SPECTATORMODE_TRACKING", wszModeText, LTARRAYSIZE(wszModeText), wszName );
		}
		break;
	case eSpectatorMode_Clip:
		if( bUpdate )
			LTStrCpy( wszModeText, LoadString( "IDS_SPECTATORMODE_CLIPPING" ), LTARRAYSIZE( wszModeText ));
		break;
	case eSpectatorMode_Fixed:
		if( bUpdate )
			LTStrCpy( wszModeText, LoadString( "IDS_SPECTATORMODE_FIXED" ), LTARRAYSIZE( wszModeText ));
		break;
	case eSpectatorMode_Fly:
		if( bUpdate )
			LTStrCpy( wszModeText, LoadString( "IDS_SPECTATORMODE_FLYING" ), LTARRAYSIZE( wszModeText ));
		break;
	case eSpectatorMode_Follow:
		{
			wchar_t wszName[256] = L"";
			HOBJECT hTarget = g_pPlayerMgr->GetSpectatorFollowTarget( );

			// Check if the target changed.
			if( hTarget != m_hLastTarget )
			{
				m_hLastTarget = hTarget;
				m_pLastCharFx = g_pGameClientShell->GetSFXMgr()->GetCharacterFX( hTarget );
				bUpdate = true;
			}

			if( bUpdate && m_pLastCharFx && m_pLastCharFx->m_cs.nClientID != INVALID_CLIENT )
			{
				LTStrCpy( wszName, g_pInterfaceMgr->GetClientInfoMgr()->GetPlayerName( m_pLastCharFx->m_cs.nClientID ), 
					LTARRAYSIZE( wszName ));
			}

			if( bUpdate )
				FormatString( "IDS_SPECTATORMODE_FOLLOWING", wszModeText, LTARRAYSIZE(wszModeText), wszName );
		}
		break;
	}

	if( bUpdate )
	{
		wchar_t wszTitleText[256] = L"";
		if( wszModeText[0] )
			FormatString( "IDS_SPECTATING", wszTitleText, LTARRAYSIZE(wszTitleText), wszModeText );

		// Show that we can go to the next spectator mode if possible.
		wchar_t wszNextMode[256] = L"";
		SpectatorMode eNewSpectatorMode = g_pPlayerMgr->GetNextSpectatorMode( g_pPlayerMgr->GetSpectatorMode( ));
		if( eNewSpectatorMode != g_pPlayerMgr->GetSpectatorMode( ))
		{
			FormatString( "IDS_SPECTATOR_NEXTMODE", wszNextMode, LTARRAYSIZE(wszNextMode), 
				g_pProfileMgr->GetCurrentProfile()->GetTriggerNameFromCommandID(COMMAND_ID_ACTIVATE));
		}

		// Show how to go to the next player.
		wchar_t wszNextPlayer[256] = L"";
		if( g_pPlayerMgr->GetSpectatorMode( ) == eSpectatorMode_Follow )
		{
			FormatString( "IDS_SPECTATOR_NEXTPLAYER", wszNextPlayer, LTARRAYSIZE(wszNextPlayer), 
				g_pProfileMgr->GetCurrentProfile()->GetTriggerNameFromCommandID(COMMAND_ID_JUMP));
		}

		// Show how to respawn if necessary.
		wchar_t wszRespawn[256] = L"";
		if( bCanRespawn )
		{
			FormatString( "IDS_SPECTATOR_RESPAWN", wszRespawn, LTARRAYSIZE(wszRespawn), 
				g_pProfileMgr->GetCurrentProfile()->GetTriggerNameFromCommandID(COMMAND_ID_FIRING));
		}

		// Put it all together.
		wchar_t wszHudText[256] = L"";
		LTStrCpy( wszHudText, wszTitleText, LTARRAYSIZE( wszHudText ));
		if( wszNextMode[0] )
		{
			if( wszHudText[0] )
				LTStrCat( wszHudText, L"\n", LTARRAYSIZE( wszHudText ));
			LTStrCat( wszHudText, wszNextMode, LTARRAYSIZE( wszHudText ));
		}
		if( wszNextPlayer[0] )
		{
			if( wszHudText[0] )
				LTStrCat( wszHudText, L"\n", LTARRAYSIZE( wszHudText ));
			LTStrCat( wszHudText, wszNextPlayer, LTARRAYSIZE( wszHudText ));
		}
		if( wszRespawn[0] )
		{
			if( wszHudText[0] )
				LTStrCat( wszHudText, L"\n", LTARRAYSIZE( wszHudText ));
			LTStrCat( wszHudText, wszRespawn, LTARRAYSIZE( wszHudText ));
		}

		m_Text.SetText(wszHudText);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CHUDSpectator::ScaleChanged()
//
//  PURPOSE:	Update to fit new screen resolution
//
// ----------------------------------------------------------------------- //

void CHUDSpectator::ScaleChanged()
{
	CHUDItem::ScaleChanged();
}


void CHUDSpectator::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDSpectator");
	}

	CHUDItem::UpdateLayout();
}
