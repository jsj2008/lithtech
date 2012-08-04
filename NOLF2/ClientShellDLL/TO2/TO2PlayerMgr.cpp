// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerMgr.cpp
//
// PURPOSE : Implementation of class to handle managment of missions and worlds.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "TO2PlayerMgr.h"
#include "TO2HUDMgr.h"
#include "MsgIDs.h"
#include "GameClientShell.h"
#include "PlayerCamera.h"
#include "TargetMgr.h"
#include "WeaponMgr.h"





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerMgr::CTO2PlayerMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTO2PlayerMgr::CTO2PlayerMgr() : CPlayerMgr()
{
	m_nFlashlightID = WMGR_INVALID_ID;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerMgr::~CTO2PlayerMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTO2PlayerMgr::~CTO2PlayerMgr()
{
}


LTBOOL CTO2PlayerMgr::Init()
{
	const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon("Keychain");
	m_nFlashlightID = pWeapon->nId;
	return CPlayerMgr::Init();
}


LTBOOL CTO2PlayerMgr::OnCommandOn(int command)
{
	// Make sure we're in the world...

	if (!IsPlayerInWorld()) return LTFALSE;

	// Take appropriate action

	switch (command)
	{
		case COMMAND_ID_FLASHLIGHT :
		{
			if (g_pPlayerStats->GetCurrentWeapon() == m_nFlashlightID)
				LastWeapon();
			else
				ChangeWeapon(m_nFlashlightID);
		} break;
	};


	return CPlayerMgr::OnCommandOn(command);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::OnEnterWorld()
//
//	PURPOSE:	Handle entering world
//
// ----------------------------------------------------------------------- //

void CTO2PlayerMgr::OnEnterWorld()
{
	CPlayerMgr::OnEnterWorld( );

	// Check if they want radar on.
	if( g_pGameClientShell->ShouldUseRadar( ) )
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

		if( IsMultiplayerGame( ) )
		{
			if (g_pGameClientShell->GetGameType() != eGameTypeDeathmatch)
			{
				g_pCompass->SetDraw( pProfile->m_bMPRadar );
				g_pRadar->SetDraw( pProfile->m_bMPRadar );
			}
		}
		else
		{
			g_pCompass->SetDraw( pProfile->m_bSPRadar );
			g_pRadar->SetDraw( pProfile->m_bSPRadar );
		}
	}
	
}
