// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerStats.cpp
//
// PURPOSE : Implementation of PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "PlayerStats.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "HUDMgr.h"
#include "ClientWeaponMgr.h"
#include "MissionMgr.h"
#include "HUDMessageQueue.h"
#include "HUDWeaponList.h"
#include "ObjectiveDB.h"

CPlayerStats* g_pPlayerStats = NULL;

void ArmorFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateArmor(atoi(argv[0]));
}
void MaxArmorFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateMaxArmor(atoi(argv[0]));
}
void HealthFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateHealth(atoi(argv[0]));
}
void MaxHealthFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateMaxHealth(atoi(argv[0]));
}
void AirFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateAir((float)atof(argv[0])/100.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::CPlayerStats()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerStats::CPlayerStats()
{
	LTRect2n nullrect(0,0,0,0);
	LTVector2n nullPt(0,0);

	m_pnAmmo			= NULL;
	m_pbHaveWeapon		= NULL;
	m_pnAmmoInClip		= NULL;
	m_nNumHaveWeapons	= 0;
	m_pbHaveMod			= NULL;
	m_nHealth			= 0;
	m_nDamage			= 0;
	m_nArmor			= 0;
	m_nMaxHealth		= 0;
	m_nMaxArmor			= 0;
	m_hCurrentWeapon	= NULL;
	m_hCurrentGrenade	= NULL;
	m_hLastGrenade		= NULL;
	m_hCurrentAmmo		= NULL;

	m_fAirPercent = 1.0f;

	m_dwProgress		= 0;
	m_dwMaxProgress		= 0;
	

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::~CPlayerStats()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPlayerStats::~CPlayerStats()
{
	Term();
	g_pPlayerStats = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

bool CPlayerStats::Init()
{
	g_pLTClient->RegisterConsoleProgram("Armor", ArmorFn);
	g_pLTClient->RegisterConsoleProgram("MaxArmor", MaxArmorFn);
	g_pLTClient->RegisterConsoleProgram("Health", HealthFn);
	g_pLTClient->RegisterConsoleProgram("MaxHealth", MaxHealthFn);
	g_pLTClient->RegisterConsoleProgram("Air", AirFn);

	if (m_pnAmmo)
	{
		debug_deletea(m_pnAmmo);
		m_pnAmmo = NULL;
	}

	uint8 nNumAmmo = g_pWeaponDB->GetNumAmmo();
	if( nNumAmmo > 0 )
	{
		m_pnAmmo = debug_newa(uint32, nNumAmmo);
		memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmo);
	}

	if (m_pbHaveWeapon)
	{
		debug_deletea(m_pbHaveWeapon);
		m_pbHaveWeapon = NULL;
		m_nNumHaveWeapons = 0;
	}
	if (m_pnAmmoInClip)
	{
		debug_deletea(m_pnAmmoInClip);
		m_pnAmmoInClip = NULL;
	}

	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	if (nNumWeapons > 0)
	{
		m_pbHaveWeapon = debug_newa(bool, nNumWeapons);
		memset(m_pbHaveWeapon, 0, sizeof(bool) * nNumWeapons);
		m_nNumHaveWeapons = 0;

		m_pnAmmoInClip = debug_newa(uint32, nNumWeapons);
		memset(m_pnAmmoInClip, 0, sizeof(uint32) * nNumWeapons);
	}

	if (m_pbHaveMod)
	{
		debug_deletea(m_pbHaveMod);
		m_pbHaveMod = NULL;
	}


	uint8 nNumMods = g_pWeaponDB->GetNumMods();
	if( nNumMods > 0 )
	{
		m_pbHaveMod = debug_newa(uint8, nNumMods);
		memset(m_pbHaveMod, 0, sizeof(uint8) * nNumMods);

	}

	//set up our gear array
	uint8 nNumGear = g_pWeaponDB->GetNumGear();
	m_vecGearCount.clear();
	m_vecGearCount.insert(m_vecGearCount.begin(),nNumGear,0);

	m_vecHadGear.clear();
	m_vecHadGear.insert(m_vecHadGear.begin(),nNumGear,0);

	m_hObjective = NULL;
	m_sMission = "";

	ResetMissionStats();

	SetWeaponCapacity(g_pWeaponDB->GetWeaponCapacity());

	g_pPlayerStats = this;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Term()
//
//	PURPOSE:	Terminate the player stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Term()
{
	if (!g_pLTClient) return;

	m_nHealth		= 0;
	m_nDamage		= 0;
	m_nArmor		= 0;
	m_nMaxHealth	= 0;
	m_nMaxArmor		= 0;

	if (m_pnAmmo)
	{
		debug_deletea(m_pnAmmo);
		m_pnAmmo = NULL;
	}

	if (m_pbHaveWeapon)
	{
		debug_deletea(m_pbHaveWeapon);
		m_pbHaveWeapon = NULL;
		m_nNumHaveWeapons = 0;
	}

	if (m_pnAmmoInClip)
	{
		debug_deletea(m_pnAmmoInClip);
		m_pnAmmoInClip = NULL;
	}

	if (m_pbHaveMod)
	{
		debug_deletea(m_pbHaveMod);
		m_pbHaveMod = NULL;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Update()
//
//	PURPOSE:	Handle regular updates
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Update()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::OnEnterWorld()
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CPlayerStats::OnEnterWorld(bool bRestoringGame)
{
	if (!g_pLTClient || !g_pGameClientShell) return;

	// find out what mode we are in and make sure that mode is set

	ResetStats();

	if (!bRestoringGame)
	{
		// clear the values

		Clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::OnExitWorld()
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CPlayerStats::OnExitWorld()
{
	if (!g_pLTClient) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Clear()
//
//	PURPOSE:	Handle clearing the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::Clear()
{
	UpdateHealth(0);
	m_nDamage = 0;
	UpdateArmor(0);

	// Get rid of any weapons, ammo, gear or mods..

	ResetInventory();
	ResetMissionStats();
	m_hObjective = NULL;
	m_sMission = "";

	g_pPlayerMgr->ResetFlashlight();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ResetStats()
//
//	PURPOSE:	Reset the stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::ResetStats()
{
	m_MissionStats.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdatePlayerWeapon()
//
//	PURPOSE:	Update the weapon related stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdatePlayerWeapon( HWEAPON hWeapon, HAMMO hAmmo, bool bForce /* = false  */ )
{
	if( !hWeapon )
		return;

	if (!bForce)
	{
		if( (m_hCurrentWeapon == hWeapon) && (m_hCurrentAmmo == hAmmo) )
			return;
	}

	m_hCurrentWeapon = hWeapon;

	if( hAmmo != m_hCurrentAmmo )
	{
		m_hCurrentAmmo = hAmmo;
		g_pHUDMgr->QueueUpdate(kHUDAmmo);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdatePlayerGrenade()
//
//	PURPOSE:	set the player's current grenade
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdatePlayerGrenade(HWEAPON hGrenade, bool bForceDeselect )
{
	//tell our grenade HUD display to update
	if (g_pHUDMgr)
	{
		g_pHUDMgr->QueueUpdate(kHUDGrenade);
	}

	m_hLastGrenade = m_hCurrentGrenade;
	m_hCurrentGrenade = hGrenade;

	if (!hGrenade)
	{
		return;
	}

	//see if there's weapon we're supposed to select at the same time...
	HWEAPONDATA hGData = g_pWeaponDB->GetWeaponData(hGrenade, !USE_AI_DATA);
	HWEAPON hLinkedWeapon = g_pWeaponDB->GetRecordLink( hGData, WDB_WEAPON_rLinkedWeapon );
	bool bSelect = g_pWeaponDB->GetBool( hGData, WDB_WEAPON_bSelectLinked );
	if( hLinkedWeapon && bSelect && HaveWeapon(hLinkedWeapon) )
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hLinkedWeapon, !USE_AI_DATA);
		bool bGrenade = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bIsGrenade );

		if (!bGrenade && g_pClientWeaponMgr->CanChangeToWeapon(hLinkedWeapon))
		{
			g_pClientWeaponMgr->ChangeWeapon(hLinkedWeapon);
		}
	}
	//check to see if the last weapon we had was linked
	else if (m_hLastGrenade && bForceDeselect)
	{
		HWEAPONDATA hLGData = g_pWeaponDB->GetWeaponData(m_hLastGrenade, !USE_AI_DATA);
		HWEAPON hLinkedWeapon = g_pWeaponDB->GetRecordLink( hLGData, WDB_WEAPON_rLinkedWeapon );
		bool bSelect = g_pWeaponDB->GetBool( hLGData, WDB_WEAPON_bSelectLinked );
		if( bSelect && hLinkedWeapon == m_hCurrentWeapon )
		{
			g_pClientWeaponMgr->LastWeapon();
		}

	}


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateHealth()
//
//	PURPOSE:	Update the health stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateHealth(uint32 nHealth)
{
	if (nHealth > m_nMaxHealth)
		nHealth = m_nMaxHealth;

	if (m_nHealth == nHealth) return;

	if (nHealth < m_nHealth)
	{
		m_nDamage += (m_nHealth - nHealth);
	}


	// update the member variable
	m_nHealth = nHealth;
	g_pHUDMgr->QueueUpdate(kHUDHealth);



}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateArmor()
//
//	PURPOSE:	Update the armor stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateArmor(uint32 nArmor)
{
	if (nArmor > m_nMaxArmor)
		nArmor = m_nMaxArmor;
	if (m_nArmor == nArmor) return;

	// update the member variable
	m_nArmor = nArmor;
	g_pHUDMgr->QueueUpdate(kHUDArmor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateMaxHealth()
//
//	PURPOSE:	Update the health stat maximum
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateMaxHealth(uint32 nHealth)
{
	if (m_nMaxHealth == nHealth) return;

	// update the member variable
	m_nMaxHealth = nHealth;

	//if we have more than our max... reduce
	if (m_nHealth > m_nMaxHealth)
		m_nHealth = m_nMaxHealth;

	g_pHUDMgr->QueueUpdate(kHUDHealth);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateMaxArmor()
//
//	PURPOSE:	Update the armor stat maximum
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateMaxArmor(uint32 nArmor)
{
	if (m_nMaxArmor == nArmor) return;

	// update the member variables
	m_nMaxArmor = nArmor;

	//if we have more than our max... reduce
	if (m_nArmor > m_nMaxArmor)
		m_nArmor = m_nMaxArmor;
	g_pHUDMgr->QueueUpdate(kHUDArmor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAmmo()
//
//	PURPOSE:	Update the ammo stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateAmmo(	HWEAPON hWeapon, HAMMO hAmmo,
								uint32 nAmmo, bool bPickedup,
								bool bDisplayMsg, uint8 nSlot, bool bInstantChange )
{
	LTASSERT(m_pbHaveWeapon,"CPlayerStats acccessed before initialization.");
	if( !g_pLTClient || !g_pWeaponDB || !m_pbHaveWeapon)
		return;


	uint8 nWeaponIndex	= g_pWeaponDB->GetPlayerWeaponIndex( hWeapon );
	uint32 nAmmoIndex	= g_pWeaponDB->GetRecordIndex( hAmmo );

	if( bPickedup && (nWeaponIndex != WDB_INVALID_WEAPON_INDEX))
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

		// Add the weapon if we do not already have it.
		if( !m_pbHaveWeapon[nWeaponIndex] )
		{
			m_nNumHaveWeapons++;
			m_pbHaveWeapon[nWeaponIndex] = true;
			m_pnAmmoInClip[nWeaponIndex] = 0;

			// Add the weapon's integrated mods.
			uint8 nNumMods = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rModName );
			for( uint8 nMod = 0; nMod < nNumMods; ++nMod )
			{
				HMOD hMod = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rModName );
				if( hMod && g_pWeaponDB->GetBool( hMod, WDB_MOD_bIntegrated ))
				{
					uint32 nModIndex = g_pWeaponDB->GetRecordIndex( hMod );
					if( nModIndex != INVALID_GAME_DATABASE_INDEX )
					{
						LTASSERT( m_pbHaveMod[nModIndex] < 255, "mod ref-count overrun.");
						if( m_pbHaveMod[nModIndex] < 255 )
							++m_pbHaveMod[nModIndex];
					}
				}
			}

		}

		// Update the HUD.
		if (nSlot != WDB_INVALID_WEAPON_INDEX && g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bTakesInventorySlot))
		{
			LTASSERT(nSlot < m_vecWeaponSlots.size(),"CPlayerStats::UpdateAmmo() : Adding a weapon to an invalid slot");
			m_vecWeaponSlots[nSlot] = hWeapon;
			if( g_pHUDWeaponList )
				g_pHUDWeaponList->SetWeapon(hWeapon,nSlot);
		}
		else if (g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsGrenade)) 
		{
			if( g_pHUDGrenadeList )
				g_pHUDGrenadeList->Update();
		}

	}

	if (!hWeapon && hAmmo)
	{
		HWEAPON hTestWeapon = g_pWeaponDB->GetWeaponFromAmmo(hAmmo, !USE_AI_DATA);
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hTestWeapon, !USE_AI_DATA);
		if (hTestWeapon && g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsAmmo))
			hWeapon = hTestWeapon;
	}

	g_pHUDMgr->QueueUpdate(kHUDWeapon);

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	bool bIsGrenade	= hWeapon ? g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bIsGrenade ) : false;

	//special handling of grenades
	if (bIsGrenade)
	{
		g_pHUDMgr->QueueUpdate(kHUDGrenade);

		//if we picked up a grenade...
		if (bPickedup)
		{
			//choose this as our current grenade type only if we don't have a current grenade...
			if (!m_hCurrentGrenade)
			{
				UpdatePlayerGrenade(hWeapon,true);
			}
		}
		else if (nAmmo == 0 && hWeapon == m_hCurrentGrenade)
		{
			//we've thrown the last grenade of this type
			
			//NextGrenade(); // we don't want to switch...

			//we do want to keep the HUD updated...
			g_pHUDMgr->QueueUpdate(kHUDWeapon);
		 
		}
	}

	HWEAPON hEquipWeapon = NULL;
	if( nAmmoIndex != INVALID_GAME_DATABASE_INDEX ) 
	{
		if (bPickedup && !bIsGrenade) /*** BL 12/08/2000 Added to equip weapon when you are out of ammo and pick up ammo/weapon ******/
		{
			// Check to see if we are out of ammo...

			bool bOutOfAmmo = true;
			HWEAPON hBestWeapon = NULL;

			uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
			for( uint8 nCurWeaponIndex = 0; nCurWeaponIndex < nNumWeapons; ++nCurWeaponIndex )
			{
				HWEAPON hCurWeapon = g_pWeaponDB->GetPlayerWeapon( nCurWeaponIndex );
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hCurWeapon, !USE_AI_DATA);
				if (!g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bTakesInventorySlot ))
					continue;

				// Make sure the index is valid and we atually have the weapon...

				if( nCurWeaponIndex != WDB_INVALID_WEAPON_INDEX && m_pbHaveWeapon[nCurWeaponIndex] )
				{
					if ( !g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ))
					{
						uint8 nNumAmmo = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );
						for( int8 nCurAmmo = 0 ; nCurAmmo < nNumAmmo ; ++nCurAmmo )
						{
							HAMMO hCurAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nCurAmmo );
							uint32 nCurAmmoIndex = g_pWeaponDB->GetRecordIndex( hCurAmmo );

							if( !hCurAmmo || (nCurAmmoIndex == INVALID_GAME_DATABASE_INDEX) ) 
								continue;

							if( (hBestWeapon == NULL) && (hCurAmmo == hAmmo) )
							{
								hBestWeapon = hCurWeapon;
							}
							
							if ( m_pnAmmo[nCurAmmoIndex] > 0 )
							{
								//g_pLTClient->CPrint("%s has %d rounds of %s", pWeapon->szName, m_pnAmmo[pWeapon->aAmmoTypes[iAmmo]], pAmmo->szName);
								bOutOfAmmo = false;
								break;
							}
							else
							{
								//g_pLTClient->CPrint("%s is out of %s", pWeapon->szName, pAmmo->szName);
							}
						}
					}
				}
			}

			if ( bOutOfAmmo )
			{
				//g_pLTClient->CPrint("Out of ammo!");

				if ( hWeapon == NULL )
				{
					// Picked up ammo -- equip the best gun for that can use the ammo

					hEquipWeapon = hBestWeapon;
				}
				else
				{
					// Picked up a gun -- equip that gun

					hEquipWeapon = hWeapon;
				}
			}
			else
			{
				//g_pLTClient->CPrint("Not out of ammo!");
			}
		}  /*** BL 12/08/2000 End changes ******/
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,false);
		uint32 maxAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nMaxAmount );
		if (m_pnAmmo[nAmmoIndex] > maxAmmo)
		{
			LTASSERT(nAmmoIndex < g_pWeaponDB->GetNumAmmo(),"ammo overrun");
			m_pnAmmo[nAmmoIndex] = maxAmmo;
		}

		if (nAmmo > maxAmmo)
		{
			nAmmo = maxAmmo;
		}

		LTASSERT(nAmmoIndex < g_pWeaponDB->GetNumAmmo(),"ammo overrun");
		m_pnAmmo[nAmmoIndex] = nAmmo;

	}

	//force weapon change if we don't have a current weapon and the weapon matches our default
	bool bForce = (!g_pPlayerMgr->GetClientWeaponMgr( )->GetCurrentClientWeapon() && 
					g_pPlayerMgr->GetClientWeaponMgr()->GetDefaultWeapon() == hEquipWeapon);
	if( hEquipWeapon != NULL && (bForce || GetConsoleBool("AutoWeaponSwitch",true)))
	{
		// if the current weapon is broken, don't play select/deselect animations
		g_pClientWeaponMgr->ChangeWeapon( hEquipWeapon, hAmmo, -1, !bInstantChange, !bInstantChange );
	} 

	if( m_hCurrentAmmo == hAmmo )
	{
		g_pHUDMgr->QueueUpdate(kHUDAmmo);
	}

	if( m_hCurrentGrenade == hWeapon)
	{
		g_pHUDMgr->QueueUpdate(kHUDGrenade);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAmmoInClip()
//
//	PURPOSE:	Update the ammo in a particular weapon's clip
//
// ----------------------------------------------------------------------- //
void CPlayerStats::UpdateAmmoInClip(HWEAPON hWeapon,uint32 nAmmo)
{
	LTASSERT(m_pbHaveWeapon,"CPlayerStats acccessed before initialization.");
	if( !g_pLTClient || !g_pWeaponDB || !m_pbHaveWeapon)
		return;
	uint8 nWeaponIndex	= g_pWeaponDB->GetPlayerWeaponIndex( hWeapon );

	if (nWeaponIndex != WDB_INVALID_WEAPON_INDEX && m_pbHaveWeapon[nWeaponIndex])
	{
		m_pnAmmoInClip[nWeaponIndex] = nAmmo;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::RemoveWeapon()
//
//	PURPOSE:	Remove the specified weapon from inventory
//
// ----------------------------------------------------------------------- //

void CPlayerStats::RemoveWeapon( HWEAPON hWeapon )
{
	LTASSERT(m_pbHaveWeapon,"CPlayerStats acccessed before initialization.");
	if( !g_pLTClient || !g_pWeaponDB || !m_pbHaveWeapon)
		return;

	//remove the weapon and and any mods
	uint8 nWeaponIndex	= g_pWeaponDB->GetPlayerWeaponIndex( hWeapon );
	if( (nWeaponIndex != WDB_INVALID_WEAPON_INDEX))
	{
		if( m_pbHaveWeapon[nWeaponIndex] )
		{
			m_nNumHaveWeapons--;
			m_pbHaveWeapon[nWeaponIndex] = false;
			m_pnAmmoInClip[nWeaponIndex] = 0;
		}

		uint8 nSlot = GetWeaponSlot(hWeapon);
		if (nSlot != WDB_INVALID_WEAPON_INDEX) 
		{
			m_vecWeaponSlots[nSlot] = NULL;
		}

		if( g_pHUDWeaponList )
			g_pHUDWeaponList->RemoveWeapon(hWeapon);
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		uint8 nNumMods = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rModName );

		// Remove all mod's associated with the weapon.
		for( uint8 nMod = 0; nMod < nNumMods; ++nMod )
		{
			HMOD hMod = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rModName );
			uint32 nModIndex = g_pWeaponDB->GetRecordIndex( hMod );
			if( nModIndex != INVALID_GAME_DATABASE_INDEX )
			{
				LTASSERT( m_pbHaveMod[nModIndex] > 0, "mod ref-count under-run.");
				if( m_pbHaveMod[nModIndex] > 0 )
					--m_pbHaveMod[nModIndex];
			}
		}

	}

#ifdef PROJECT_FEAR	//!!ARL: FEAR wants all ammo to disappear when you drop weapons, the DARK wants to keep the ammo around in case you pick the weapon back up.  (see also CArsenal::RemoveWeapon)
	//step through our ammos, removing any that can no longer be used
	// this is complicated because each weapon could use multiple ammo types,
	//		and each ammo could be used by multiple weapons
	uint8 nNumAmmoTypes = g_pWeaponDB->GetNumAmmo();
	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();

	for( int8 nAmmoType = 0 ; nAmmoType < nNumAmmoTypes ; ++nAmmoType )
	{
		//we have some of this ammo type
		if (m_pnAmmo[nAmmoType])
		{
			bool bUsed = false;
			//step through each weapon
			for( uint8 nCurWeaponIndex = 0; !bUsed && nCurWeaponIndex < nNumWeapons; ++nCurWeaponIndex )
			{
				//if we have the weapon
				if (m_pbHaveWeapon[nCurWeaponIndex])
				{
					HWEAPON hCurWeapon = g_pWeaponDB->GetPlayerWeapon( nCurWeaponIndex );
					HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hCurWeapon, !USE_AI_DATA);

					// check each ammo type used by this weapon
					uint8 nNumWeaponAmmo = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );
					for( int8 nCurAmmo = 0 ; !bUsed && nCurAmmo < nNumWeaponAmmo ; ++nCurAmmo )
					{
						//see if it matches the ammo type we're testing
						HAMMO hCurAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nCurAmmo );
						uint32 nCurAmmoIndex = g_pWeaponDB->GetRecordIndex( hCurAmmo );
						bUsed = (nCurAmmoIndex == nAmmoType);
					}

				}
			}

			//none of our current weapons uses this ammo, so clear it
			if (!bUsed)
			{
				LTASSERT(nAmmoType < g_pWeaponDB->GetNumAmmo(),"ammo overrun");
				m_pnAmmo[nAmmoType] = 0;
			}

		}
	}
#endif

	if (m_hCurrentWeapon == hWeapon)
	{
		g_pClientWeaponMgr->ChangeWeapon(g_pWeaponDB->GetUnarmedRecord( ),NULL,-1,false);
	} 

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateGear()
//
//	PURPOSE:	Update the gear stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateGear( HGEAR hGear, GearMsgType eMsgType, uint8 nCount )
{
	if( !g_pLTClient || !g_pWeaponDB)
		return;

	if( hGear )
	{
		uint32 nIndex = g_pWeaponDB->GetRecordIndex( hGear );
		if( nIndex > m_vecGearCount.size() )
			return;

		//check to see if we can carry more of this type of item
		//	if the limit is 0, then there is no limit
		uint8 nMax = g_pWeaponDB->GetInt32(hGear,WDB_GEAR_nMaxAmount);
		if (!nMax) return;

		switch(eMsgType)
		{
		case kGearAdd:
			{
				m_vecGearCount[nIndex] += nCount;
				m_vecHadGear[nIndex] = 1;
				if (m_vecGearCount[nIndex] > nMax)
					m_vecGearCount[nIndex] = nMax;
			}
			break;
		case kGearUse:
			{
				if (m_vecGearCount[nIndex] < nCount)
				{
					m_vecGearCount[nIndex] = 0;
				}
				else
				{
					m_vecGearCount[nIndex] -= nCount;
				}
			}
			break;
		case kGearRemove:
			if (m_vecGearCount[nIndex] < nCount)
			{
				m_vecGearCount[nIndex] = 0;
			}
			else
			{
				m_vecGearCount[nIndex] -= nCount;
			}
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateMod()
//
//	PURPOSE:	Update the mod stats
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateMod( HMOD hMod )
{
	LTASSERT(m_pbHaveWeapon,"CPlayerStats acccessed before initialization.");
	if( !g_pLTClient || !g_pWeaponDB || !m_pbHaveWeapon)
		return;
	if( hMod )
	{
		uint32 nModIndex = g_pWeaponDB->GetRecordIndex( hMod );
		if( (nModIndex != INVALID_GAME_DATABASE_INDEX) && m_pbHaveMod[nModIndex] == 0)
		{
			++m_pbHaveMod[nModIndex];

			// Make sure the player-view weapon is updated if
			// this mod is used with the weapon...

			CClientWeapon* pClientWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetCurrentClientWeapon();
			
			if( pClientWeapon && (pClientWeapon->GetWeaponRecord() == hMod) )
			{
				pClientWeapon->CreateMods();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::UpdateAir()
//
//	PURPOSE:	Update the air stat
//
// ----------------------------------------------------------------------- //

void CPlayerStats::UpdateAir(float fPercent)
{
    if (!g_pLTClient) return;

	if (fPercent > 1.0f)
		fPercent = 1.0f;
	if (fPercent < 0.0f)
		fPercent = 0.0f;

	if (m_fAirPercent == fPercent) return;

	m_fAirPercent = fPercent;

}


void CPlayerStats::UpdateMissionStats(ILTMessage_Read *pMsg)
{
	m_MissionStats.ReadData(pMsg);

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Save
//
//	PURPOSE:	Save the player stats info
//
// --------------------------------------------------------------------------- //

void CPlayerStats::Save(ILTMessage_Write *pMsg)
{
	if( !pMsg || !g_pWeaponDB )
		return;


	pMsg->Writeuint32(m_nHealth);
	pMsg->Writeuint32(m_nDamage);
	pMsg->Writeuint32(m_nArmor);
	pMsg->Writeuint32(m_nMaxHealth);
	pMsg->Writeuint32(m_nMaxArmor);

	uint8 nNumAmmoTypes = g_pWeaponDB->GetNumAmmo();
	for( uint8 nAmmo = 0; nAmmo < nNumAmmoTypes; ++nAmmo )
	{
		pMsg->Writeuint32(m_pnAmmo[nAmmo]);
	}

	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	for( uint8 nWeapon = 0; nWeapon < nNumWeapons; ++nWeapon )
	{
		pMsg->Writebool(m_pbHaveWeapon[nWeapon]);
		pMsg->Writeuint32(m_pnAmmoInClip[nWeapon]);
	}

	uint8 nNumMods = g_pWeaponDB->GetNumMods();
	for( uint8 nMod = 0; nMod < nNumMods; ++nMod )
	{
		pMsg->Writeuint8(m_pbHaveMod[nMod]);
	}

	uint8 nNumGear = g_pWeaponDB->GetNumGear();
	for( uint8 nGear = 0; nGear < nNumGear; ++nGear )
	{
		pMsg->Writeuint8(m_vecGearCount[nGear]);
		pMsg->Writeuint8(m_vecHadGear[nGear]);
	}

	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hCurrentWeapon );
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hCurrentGrenade );
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hLastGrenade );
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hCurrentAmmo);
	pMsg->Writefloat(m_fAirPercent);

	pMsg->Writeuint8(m_nWeaponCapacity);

	//save weapons slots
	WeaponArray::iterator iter = m_vecWeaponSlots.begin();
	while (iter != m_vecWeaponSlots.end())
	{
		pMsg->WriteDatabaseRecord( g_pLTDatabase, (*iter) );
		++iter;
	};

	pMsg->WriteDatabaseRecord(g_pLTDatabase,m_hObjective);
	pMsg->WriteString(m_sMission.c_str());

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::Load
//
//	PURPOSE:	Load the player stats info
//
// --------------------------------------------------------------------------- //

void CPlayerStats::Load(ILTMessage_Read *pMsg)
{
	if( !pMsg || !g_pWeaponDB )
		return;

	g_pHUDMgr->Reset();

	m_nHealth		= pMsg->Readuint32();
	m_nDamage		= pMsg->Readuint32();
	m_nArmor		= pMsg->Readuint32();
	m_nMaxHealth	= pMsg->Readuint32();
	m_nMaxArmor		= pMsg->Readuint32();


	uint8 nNumAmmoTypes = g_pWeaponDB->GetNumAmmo();
	for( uint8 nAmmo = 0; nAmmo < nNumAmmoTypes; ++nAmmo )
	{
		m_pnAmmo[nAmmo]			= pMsg->Readuint32();
	}

	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	m_nNumHaveWeapons = 0;
	for( uint8 nWeapon = 0; nWeapon < nNumWeapons; ++nWeapon )
	{
		m_pbHaveWeapon[nWeapon]		= pMsg->Readbool();
		if( m_pbHaveWeapon[nWeapon] )
			m_nNumHaveWeapons++;
		if	(g_pVersionMgr->GetCurrentSaveVersion( ) > CVersionMgr::kSaveVersion__1_04)
		{
			m_pnAmmoInClip[nWeapon] = pMsg->Readuint32();
		}
		else
		{
			m_pnAmmoInClip[nWeapon] = 0;
		}
	}

	uint8 nNumMods = g_pWeaponDB->GetNumMods();
	for( uint8 nMod = 0; nMod < nNumMods; ++nMod )
	{
		m_pbHaveMod[nMod]	= pMsg->Readuint8();
	}

	uint8 nNumGear = g_pWeaponDB->GetNumGear();
	for( uint8 nGear = 0; nGear < nNumGear; ++nGear )
	{
		//HACK - a gear item was added at index 11 for v1.05
		// in order to not break the saved game we have to skip it
		if	(nGear == 11 && g_pVersionMgr->GetCurrentSaveVersion( ) <= CVersionMgr::kSaveVersion__1_04)
		{
			m_vecGearCount[nGear] = 0;
			m_vecHadGear[nGear] = 0;
		}
		else
		{
			m_vecGearCount[nGear]		= pMsg->Readuint8();
			m_vecHadGear[nGear]			= pMsg->Readuint8();
		}
	}

	if( g_pHUDWeaponList )
		g_pHUDWeaponList->Reset();

	HWEAPON hCurrentWeapon	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	HWEAPON hCurrentGrenade	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	HWEAPON hLastGrenade	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	m_hCurrentAmmo		= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
	m_fAirPercent		= pMsg->Readfloat();

	UpdatePlayerWeapon( hCurrentWeapon, m_hCurrentAmmo, true );
	UpdatePlayerGrenade( hCurrentGrenade,false );
	m_hLastGrenade = hLastGrenade;

	m_nWeaponCapacity = pMsg->Readuint8();
	m_vecWeaponSlots.clear();
	m_vecWeaponSlots.resize(m_nWeaponCapacity,(HWEAPON)NULL);
	for (uint8 i = 0; i < m_nWeaponCapacity; ++i)
	{
		m_vecWeaponSlots[i] = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );;
		if( g_pHUDWeaponList )
			g_pHUDWeaponList->SetWeapon(m_vecWeaponSlots[i],i);
	}

	HRECORD hRec = pMsg->ReadDatabaseRecord(g_pLTDatabase, DATABASE_CATEGORY( Objective ).GetCategory() );
	SetObjective(hRec);

	char szMission[256];
	pMsg->ReadString(szMission,LTARRAYSIZE(szMission));
	SetMission(szMission);

}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetMod
//
//	PURPOSE:	Get the id of thefirst mod for the current weapon
//				of the given type
//
// --------------------------------------------------------------------------- //

HMOD CPlayerStats::GetMod( ModType eType, HWEAPON hW /* = NULL */ )
{
	HMOD hMod	= NULL;
	int32 nPriority	= -1;

	// If no weapon was specified use the current weapon...

	HWEAPON hWpn = (hW ? hW : m_hCurrentWeapon);

	if( hWpn )
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWpn, !USE_AI_DATA);

		uint8 nNumMods = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rModName );
		for( uint8 nMod = 0; nMod < nNumMods; ++nMod )
		{
			HMOD hCurMod = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rModName, nMod );

			if( HaveMod( hCurMod ))
			{
				if( g_pWeaponDB->GetModType( hCurMod ) == eType )
				{
					// Get the mod with the hightest priority...
					
					int32 nCurModPriorit = g_pWeaponDB->GetInt32( hCurMod, WDB_MOD_nPriority );
					if( nCurModPriorit > nPriority )
					{
						nPriority = nCurModPriorit;
						hMod = hCurMod;
					}
				}
			}
		}
	}

	return hMod;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetAmmoCount
//
//	PURPOSE:	Get the ammo count for the passed in ammo id
//
// --------------------------------------------------------------------------- //

uint32 CPlayerStats::GetAmmoCount( HAMMO hAmmo ) const
{
	 if( !m_pnAmmo || !hAmmo )
		 return 0;

	 uint32 nIndex = g_pWeaponDB->GetRecordIndex( hAmmo );
	 if( nIndex == INVALID_GAME_DATABASE_INDEX )
		 return 0;

	 return m_pnAmmo[nIndex];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetAmmoInClip
//
//	PURPOSE:	Get the ammo in the clip of a particular weapon
//
// --------------------------------------------------------------------------- //
uint32 CPlayerStats::GetAmmoInClip( HWEAPON hWeapon ) const
{
	if( !m_pnAmmoInClip || !hWeapon )
		return 0;

	uint8 nIndex = g_pWeaponDB->GetPlayerWeaponIndex( hWeapon );
	if( nIndex == WDB_INVALID_WEAPON_INDEX )
		return 0;

	return m_pnAmmoInClip[nIndex];
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveWeapon
//
//	PURPOSE:	Do we have the weapon associated with the passed in id
//
// --------------------------------------------------------------------------- //

bool CPlayerStats::HaveWeapon( HWEAPON hWeapon ) const
{
     if( !m_pbHaveWeapon || !hWeapon )
		 return false;

	 uint8 nIndex = g_pWeaponDB->GetPlayerWeaponIndex( hWeapon );
	 if( nIndex == WDB_INVALID_WEAPON_INDEX )
		 return false;

	 return m_pbHaveWeapon[nIndex];
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveMod
//
//	PURPOSE:	Do we have the mod associated with the passed in id
//
// --------------------------------------------------------------------------- //

bool CPlayerStats::HaveMod( HMOD hMod ) const
{
	uint32 nModIndex = g_pWeaponDB->GetRecordIndex( hMod );
	
	if( !m_pbHaveMod || (nModIndex == INVALID_GAME_DATABASE_INDEX) )
		return false;

	return (m_pbHaveMod[nModIndex] != 0);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetGearCount
//
//	PURPOSE:	Do we have the Gear associated with the passed in id
//
// --------------------------------------------------------------------------- //

uint8 CPlayerStats::GetGearCount( HGEAR hGear ) const
{
	if( !hGear )
		return 0;

	uint32 nGearIndex = g_pWeaponDB->GetRecordIndex( hGear );
	if( nGearIndex == INVALID_GAME_DATABASE_INDEX )
		return 0;

	return m_vecGearCount[nGearIndex];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveHadGear
//
//	PURPOSE:	Heve we ever had the Gear associated with the passed in id
//
// --------------------------------------------------------------------------- //

bool CPlayerStats::HaveHadGear( HGEAR hGear ) const
{
	if( !hGear )
		return false;

	uint32 nGearIndex = g_pWeaponDB->GetRecordIndex( hGear );
	if( nGearIndex == INVALID_GAME_DATABASE_INDEX )
		return false;

	return (m_vecHadGear[nGearIndex] > 0);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::NextGrenade()
//
//	PURPOSE:	Cycle our selected grenade type
//
// --------------------------------------------------------------------------- //

void CPlayerStats::NextGrenade()
{
	LTASSERT(m_pbHaveWeapon,"CPlayerStats acccessed before initialization.");
	if( !g_pLTClient || !g_pWeaponDB || !m_pbHaveWeapon)
		return;

	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	//start with the first player weapon unless we already have selected a grenade, 
	//	in which case, start with the selected grenade
	uint8 nIndex = 0;
	if (m_hCurrentGrenade)
		nIndex = g_pWeaponDB->GetPlayerWeaponIndex( m_hCurrentGrenade );

	//cycle through the weapons...
	uint8 nCurIndex = (nIndex + 1) % nNumWeapons;
	while (nIndex != nCurIndex)
	{
		HWEAPON hWeapon = g_pWeaponDB->GetPlayerWeapon(nCurIndex);
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		bool bIsGrenade	= hWeapon ? g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bIsGrenade ) : false;

		//if this weapon is a grenade...
		if (bIsGrenade)
		{
			HAMMO hGrenadeAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName);
			uint8 nAmmoIndex = g_pWeaponDB->GetRecordIndex( hGrenadeAmmo );
			//and we have ammo for it...
			if (m_pnAmmo[nAmmoIndex] > 0)
			{
				//select it and stop looking...
				UpdatePlayerGrenade(hWeapon,true);
				break;
			};
		}
		
		nCurIndex = (nCurIndex + 1) % nNumWeapons;
	}

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::LastGrenade()
//
//	PURPOSE:	Re-select the last grenade type we used...
//
// --------------------------------------------------------------------------- //

void CPlayerStats::LastGrenade()
{
	LTASSERT(m_pbHaveWeapon,"CPlayerStats acccessed before initialization.");
	if( !g_pLTClient || !g_pWeaponDB || !m_pbHaveWeapon)
		return;

	if (m_hLastGrenade)
	{
		UpdatePlayerGrenade(m_hLastGrenade,false);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::ResetInventory
//
//	PURPOSE:	Reset all inventory items
//
// --------------------------------------------------------------------------- //

void CPlayerStats::ResetInventory()
{

	// Clear our data...
	m_hCurrentWeapon	= NULL;
	m_hCurrentGrenade	= NULL;
	m_hLastGrenade	= NULL;
	m_hCurrentAmmo		= NULL;
	if (g_pHUDMgr)
	{
		g_pHUDMgr->QueueUpdate(kHUDGrenade);
		g_pHUDMgr->QueueUpdate(kHUDAmmo);
		g_pHUDMgr->QueueUpdate(kHUDGear);
		g_pHUDMgr->QueueUpdate(kHUDWeapon);
	}


	
	if( g_pHUDWeaponList )
		g_pHUDWeaponList->Reset();
	g_pPlayerMgr->GetClientWeaponMgr()->ResetWeapons();

	uint8 nNumAmmoTypes = g_pWeaponDB->GetNumAmmo();
	if (nNumAmmoTypes > 0)
	{
        memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmoTypes);
	}

	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	if (nNumWeapons > 0)
	{
        memset(m_pbHaveWeapon, 0, sizeof(bool) * nNumWeapons);
		memset(m_pnAmmoInClip, 0, sizeof(uint32) * nNumWeapons);
		m_nNumHaveWeapons = 0;
	}

	uint8 nNumMods = g_pWeaponDB->GetNumMods();
	if (nNumMods > 0)
	{
		memset(m_pbHaveMod, 0, sizeof(uint8) * nNumMods);
	}


	for (uint8 nGear = 0; nGear < g_pWeaponDB->GetNumGear(); nGear++)
	{
		m_vecGearCount[nGear] = 0;
		m_vecHadGear[nGear] = 0;
	}

	m_vecWeaponSlots.clear();
	m_vecWeaponSlots.resize(m_nWeaponCapacity,(HWEAPON)NULL);


}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::DropInventory
//
//	PURPOSE:	Removes all currently carried weapons and ammo, optionally
//				removes gear and mods
//
// --------------------------------------------------------------------------- //

void CPlayerStats::DropInventory(bool bRemoveGear)
{
	// Clear our data...

	g_pPlayerMgr->GetClientWeaponMgr()->ResetWeapons();

	uint8 nNumAmmo = g_pWeaponDB->GetNumAmmo();
	if( nNumAmmo > 0 )
	{
        memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmo);
	}

	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	if( nNumWeapons > 0 )
	{
        memset(m_pbHaveWeapon, 0, sizeof(bool) * nNumWeapons);
		memset(m_pnAmmoInClip, 0, sizeof(uint32) * nNumWeapons);
		m_nNumHaveWeapons = 0;
	}

	m_vecWeaponSlots.clear();
	m_vecWeaponSlots.resize(m_nWeaponCapacity,(HWEAPON)NULL);

	if( g_pHUDWeaponList )
		g_pHUDWeaponList->Reset();


	if( bRemoveGear )
	{
		uint8 nNumMods = g_pWeaponDB->GetNumMods();
		if( nNumMods > 0 )
		{
			memset(m_pbHaveMod, 0, sizeof(uint8) * nNumMods);
		}

		for (uint8 nGear = 0; nGear < g_pWeaponDB->GetNumGear(); nGear++)
		{
			m_vecGearCount[nGear] = 0;
			m_vecHadGear[nGear] = 0;
		}

	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::RestMissionStats
//
//	PURPOSE:	Reset all mission data
//
// --------------------------------------------------------------------------- //

void CPlayerStats::ResetMissionStats()
{
	// Clear our data...
	m_MissionStats.Init();
}

void CPlayerStats::ClearMissionInfo()
{
	ResetMissionStats();
	m_nDamage = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::HaveAirSupply
//
//	PURPOSE:	Returns true if current gear provides air supply
//
// ----------------------------------------------------------------------- //

bool CPlayerStats::HaveAirSupply()
{
	return true;

}


uint32 CPlayerStats::GetCurrentAmmoCount()
{
	if( !m_pnAmmo || !m_hCurrentAmmo )
		return 0;

	uint32 nAmmoIndex = g_pWeaponDB->GetRecordIndex( m_hCurrentAmmo );
	if( nAmmoIndex == INVALID_GAME_DATABASE_INDEX )
		return 0;

	return m_pnAmmo[nAmmoIndex];
}

uint32 CPlayerStats::GetCurrentGrenadeCount()
{
	if( !m_pnAmmo || !m_hCurrentGrenade )
		return 0;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hCurrentGrenade, !USE_AI_DATA);

	HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName);
	return GetAmmoCount(hAmmo);

}




void CPlayerStats::SetWeaponCapacity(uint8 nCap) 
{
	m_nWeaponCapacity = nCap;
	m_vecWeaponSlots.resize(nCap);
}


bool CPlayerStats::UnlimitedCapacity() const
{
	return (m_nWeaponCapacity >= g_pWeaponDB->GetNumDefaultWeaponPriorities());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetWeaponSlot( )
//
//	PURPOSE:	Find the specified weapon...
//
// ----------------------------------------------------------------------- //

uint8 CPlayerStats::GetWeaponSlot(HWEAPON hWeapon )
{
	uint8 n = 0;
	while (n < m_nWeaponCapacity && hWeapon != m_vecWeaponSlots[n])
	{
		n++;
	}

	if (n >= m_nWeaponCapacity )
	{
		return WDB_INVALID_WEAPON_INDEX;
	}
	else
	{
		return n;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::GetWeaponInSlot( )
//
//	PURPOSE:	Get the weapon in the specified slot...
//
// ----------------------------------------------------------------------- //
HWEAPON CPlayerStats::GetWeaponInSlot( uint8 nSlot )
{
	if (nSlot < m_vecWeaponSlots.size())
		return m_vecWeaponSlots[nSlot];

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerStats::FindFirstEmptySlot( )
//
//	PURPOSE:	Find the first open slot
//
// ----------------------------------------------------------------------- //

uint8 CPlayerStats::FindFirstEmptySlot( ) const
{
	uint8 n = 0;
	while (n < m_nWeaponCapacity && (NULL != m_vecWeaponSlots[n]))
	{
		n++;
	}
	if (n >= m_nWeaponCapacity )
	{
		return WDB_INVALID_WEAPON_INDEX;
	}
	else
	{
		return n;
	}
}

void CPlayerStats::SetObjective(HRECORD hObjective)
{
	m_hObjective = hObjective;
}
void CPlayerStats::SetMission(const char* pszMission)
{
	if (LTStrEmpty( LoadString(pszMission) ))
	{
		m_sMission = "";
	}
	else
	{
		m_sMission = pszMission;
	}
	
}
