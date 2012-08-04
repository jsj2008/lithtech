// ----------------------------------------------------------------------- //
//
// MODULE  : AimMgr.cpp
//
// PURPOSE : implementation of class to manage play aim mode
//
// CREATED : 05/18/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AimMgr.h"
#include "PlayerCamera.h"
#include "PlayerBodyMgr.h"
#include "CMoveMgr.h"
#include "CommandIds.h"
#include "ClientWeaponMgr.h"
#include "LadderMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMgr::AimMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AimMgr::AimMgr( ) :
	m_bAiming	(false),
	m_bCanAim   (true)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AimMgr::~AimMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

AimMgr::~AimMgr( )
{
}


void AimMgr::Init()
{
}

void AimMgr::Update()
{

	if (g_pPlayerMgr->IsUnderwater() || LadderMgr::Instance().IsClimbing() || CPlayerBodyMgr::Instance().IsPlayingSpecial() ) 
	{
		EndAim();
	}
	// Determine the posture...
	if( m_bAiming )
	{
		CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_WeaponPosition, kAP_WPOS_Up);
	}

}

bool AimMgr::OnCommandOn(int command)
{
	switch (command)
	{
	case COMMAND_ID_JUMP :
	case COMMAND_ID_ALT_FIRING :
	case COMMAND_ID_RELOAD :
		{
			if (m_bAiming)
			{
				EndAim();
			}
		}
		break;

	case COMMAND_ID_ZOOM_IN :
		{
			if (m_bAiming)
				EndAim();
			else
				BeginAim();
		}
		break;
	}

	return false;
}

bool AimMgr::OnCommandOff(int command)
{
	return false;
}

void AimMgr::BeginAim()
{
	const bool bIsZoomed =  g_pPlayerMgr->GetPlayerCamera()->IsZoomed()
						 && !g_pPlayerMgr->GetPlayerCamera()->IsExittingZoom();

	if (bIsZoomed)
		m_bAiming = true;
	else
	{
		// Let the player camera handle this...
		m_bAiming = CanAim();
		if (m_bAiming)
			g_pPlayerMgr->GetPlayerCamera()->HandleZoomRequest(true);
	}

	
}

void AimMgr::EndAim()
{
	// Can't kill the aim while on a turret...
	if( g_pPlayerMgr->IsOperatingTurret( ))
		return;

	if (m_bAiming)
	{
		// Let the player camera handle this...
		g_pPlayerMgr->GetPlayerCamera()->HandleZoomRequest(false);
	}

	m_bAiming = false;
}

bool AimMgr::CanAim() const
{
	if( !m_bCanAim )
		return false;

	// Can't zoom without a weapon...
	if( !g_pClientWeaponMgr->WeaponsEnabled() )
		return false;

	// Can't aim if we are playing a special animation.
	if( CPlayerBodyMgr::Instance().IsPlayingSpecial() )
		return false;

	// Or a scope...
	HMOD hScope = g_pPlayerStats->GetScope();
	if( !hScope )
		return false;

	// Make sure the new zoom mag is valid.
	float fNewZoomMag = g_pWeaponDB->GetFloat( hScope, WDB_MOD_fMagPower );
	if( fNewZoomMag < 0.0f )
		return false;

	return true;

}

void AimMgr::SetCanAim(bool bCanAim)
{
	// If we are not allowed to aim, be sure
	// our current aiming is disabled.
	if( !bCanAim && m_bAiming )
	{
		EndAim();
	}

	m_bCanAim = bCanAim;
}
