// ----------------------------------------------------------------------- //
//
// MODULE  : TO2PlayerStats.cpp
//
// PURPOSE : Implementation of TO2PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "TO2PlayerStats.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "TO2HUDMgr.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "MissionMgr.h"
#include "KeyMgr.h"
#include "clientres.h"

void ArmorFn(int argc, char **argv)
{
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateArmor(atoi(argv[0]));
}
void MaxArmorFn(int argc, char **argv)
{
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateMaxArmor(atoi(argv[0]));
}
void HealthFn(int argc, char **argv)
{
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateHealth(atoi(argv[0]));
}
void MaxHealthFn(int argc, char **argv)
{
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateMaxHealth(atoi(argv[0]));
}
void AirFn(int argc, char **argv)
{
	if (!g_pPlayerStats || argc < 1) return;
	g_pPlayerStats->UpdateAir((float)atof(argv[0])/100.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::CTO2PlayerStats()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTO2PlayerStats::CTO2PlayerStats()
{
    LTRect nullrect(0,0,0,0);
    LTIntPt nullPt(0,0);

    m_pnAmmo            = LTNULL;
    m_pbHaveAmmo        = LTNULL;
    m_pbHaveWeapon      = LTNULL;
    m_pbCanUseWeapon    = LTNULL;
    m_pbCanUseAmmo      = LTNULL;
    m_pbHaveMod         = LTNULL;
    m_pbCanUseMod       = LTNULL;
    m_pbHaveGear        = LTNULL;
    m_pbCanUseGear      = LTNULL;
	m_nHealth			= 0;
	m_nDamage			= 0;
	m_nArmor			= 0;
	m_nMaxHealth		= 0;
	m_nMaxArmor			= 0;
	m_nCurrentWeapon	= WMGR_INVALID_ID;
	m_nCurrentAmmo		= WMGR_INVALID_ID;

	m_bHiding			= LTFALSE;
	m_bHidden			= LTFALSE;

	m_fAirPercent = 1.0f;

	m_bObjAdded = LTFALSE;
	m_bObjRemoved = LTFALSE;
	m_bObjCompleted = LTFALSE;

	m_nTotalSkillPoints = 0;
	m_nAvailSkillPoints = 0;
	
	m_dwProgress		= 0;
	m_dwMaxProgress		= 0;

	memset(m_nSkills,0,sizeof(m_nSkills));


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::~CTO2PlayerStats()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTO2PlayerStats::~CTO2PlayerStats()
{
	Term();
	g_pPlayerStats = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::Init()
{
    if (!g_pLTClient) return LTFALSE;

	g_pLTClient->RegisterConsoleProgram("Armor", ArmorFn);
	g_pLTClient->RegisterConsoleProgram("MaxArmor", MaxArmorFn);
	g_pLTClient->RegisterConsoleProgram("Health", HealthFn);
	g_pLTClient->RegisterConsoleProgram("MaxHealth", MaxHealthFn);
	g_pLTClient->RegisterConsoleProgram("Air", AirFn);

	if (m_pnAmmo)
	{
		debug_deletea(m_pnAmmo);
        m_pnAmmo = LTNULL;
	}

	if (m_pbHaveAmmo)
	{
		debug_deletea(m_pbHaveAmmo);
        m_pbHaveAmmo = LTNULL;
	}

	if (m_pbCanUseAmmo)
	{
		debug_deletea(m_pbCanUseAmmo);
        m_pbCanUseAmmo = LTNULL;
	}

	int nNumAmmoIds = g_pWeaponMgr->GetNumAmmoIds();
	if (nNumAmmoIds > 0)
	{
        m_pnAmmo = debug_newa(uint32, nNumAmmoIds);
        m_pbCanUseAmmo = debug_newa(LTBOOL, nNumAmmoIds);
        m_pbHaveAmmo = debug_newa(LTBOOL, nNumAmmoIds);
        memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmoIds);
        memset(m_pbHaveAmmo, 0, sizeof(LTBOOL) * nNumAmmoIds);
        memset(m_pbCanUseAmmo, 0, sizeof(LTBOOL) * nNumAmmoIds);
	}

	if (m_pbHaveWeapon)
	{
		debug_deletea(m_pbHaveWeapon);
        m_pbHaveWeapon = LTNULL;
	}

	if (m_pbCanUseWeapon)
	{
		debug_deletea(m_pbCanUseWeapon);
        m_pbCanUseWeapon = LTNULL;
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
        m_pbHaveWeapon = debug_newa(LTBOOL, nNumWeapons);
        memset(m_pbHaveWeapon, 0, sizeof(LTBOOL) * nNumWeapons);

        m_pbCanUseWeapon = debug_newa(LTBOOL, nNumWeapons);
        memset(m_pbCanUseWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
	}

	if (m_pbHaveMod)
	{
		debug_deletea(m_pbHaveMod);
        m_pbHaveMod = LTNULL;
	}

	if (m_pbCanUseMod)
	{
		debug_deletea(m_pbCanUseMod);
        m_pbCanUseMod = LTNULL;
	}

	if (m_pbHaveGear)
	{
		debug_deletea(m_pbHaveGear);
        m_pbHaveGear = LTNULL;
	}

	if (m_pbCanUseGear)
	{
		debug_deletea(m_pbCanUseGear);
        m_pbCanUseGear = LTNULL;
	}

	int nNumMods = g_pWeaponMgr->GetNumModIds();
	if (nNumMods > 0)
	{
        m_pbHaveMod = debug_newa(LTBOOL, nNumMods);
        memset(m_pbHaveMod, 0, sizeof(LTBOOL) * nNumMods);

        m_pbCanUseMod = debug_newa(LTBOOL, nNumMods);
        memset(m_pbCanUseMod, 0, sizeof(LTBOOL) * nNumMods);

	}

	int nNumGear = g_pWeaponMgr->GetNumGearIds();
	if (nNumGear > 0)
	{
        m_pbHaveGear = debug_newa(LTBOOL, nNumGear);
        memset(m_pbHaveGear, 0, sizeof(LTBOOL) * nNumGear);

        m_pbCanUseGear = debug_newa(LTBOOL, nNumGear);
        memset(m_pbCanUseGear, 0, sizeof(LTBOOL) * nNumGear);

	}

	ResetSkills();
	ResetObjectives();
	ResetMissionStats();

	g_pPlayerStats = this;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Term()
//
//	PURPOSE:	Terminate the player stats
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::Term()
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
        m_pnAmmo = LTNULL;
	}

	if (m_pbCanUseAmmo)
	{
		debug_deletea(m_pbCanUseAmmo);
        m_pbCanUseAmmo = LTNULL;
	}

	if (m_pbHaveAmmo)
	{
		debug_deletea(m_pbHaveAmmo);
        m_pbHaveAmmo = LTNULL;
	}

	if (m_pbHaveWeapon)
	{
		debug_deletea(m_pbHaveWeapon);
        m_pbHaveWeapon = LTNULL;
	}

	if (m_pbCanUseWeapon)
	{
		debug_deletea(m_pbCanUseWeapon);
        m_pbCanUseWeapon = LTNULL;
	}

	if (m_pbHaveMod)
	{
		debug_deletea(m_pbHaveMod);
        m_pbHaveMod = LTNULL;
	}

	if (m_pbCanUseMod)
	{
		debug_deletea(m_pbCanUseMod);
        m_pbCanUseMod = LTNULL;
	}

	if (m_pbHaveGear)
	{
		debug_deletea(m_pbHaveGear);
        m_pbHaveGear = LTNULL;
	}

	if (m_pbCanUseGear)
	{
		debug_deletea(m_pbCanUseGear);
        m_pbCanUseGear = LTNULL;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Update()
//
//	PURPOSE:	Handle regular updates
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::Update()
{
	if (m_bObjAdded)
	{
		g_pClientSoundMgr->PlaySoundLocal("interface\\snd\\objadd.wav", SOUNDPRIORITY_MISC_MEDIUM);
		g_pObjectives->Show(true);
	}
	else if (m_bObjCompleted)
	{
		g_pClientSoundMgr->PlaySoundLocal("interface\\snd\\objcomplete.wav", SOUNDPRIORITY_MISC_MEDIUM);
		g_pObjectives->Show(true);
	}
	else if (m_bObjRemoved)
	{
		g_pClientSoundMgr->PlaySoundLocal("interface\\snd\\objremove.wav", SOUNDPRIORITY_MISC_MEDIUM);
		g_pObjectives->Show(true);
	}

	m_bObjAdded = LTFALSE;
	m_bObjRemoved = LTFALSE;
	m_bObjCompleted = LTFALSE;


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::OnEnterWorld()
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::OnEnterWorld(LTBOOL bRestoringGame)
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
//	ROUTINE:	CTO2PlayerStats::OnExitWorld()
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::OnExitWorld()
{
    if (!g_pLTClient) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Clear()
//
//	PURPOSE:	Handle clearing the stats
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::Clear()
{
	UpdateHealth(0);
	m_nDamage = 0;
	UpdateArmor(0);

	// Get rid of any weapons, ammo, gear or mods..
	
	ResetInventory();
	ResetSkills();
	ResetObjectives();
	ResetMissionStats();
	m_IntelList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::ResetStats()
//
//	PURPOSE:	Reset the stats
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::ResetStats()
{
	m_MissionStats.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdatePlayerWeapon()
//
//	PURPOSE:	Update the weapon related stats
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdatePlayerWeapon(uint8 nWeaponId, uint8 nAmmoId, LTBOOL bForce)
{
	if (!g_pGameClientShell || !g_pWeaponMgr->IsValidWeaponId(nWeaponId)) return;

	if (!bForce)
	{
		if (m_nCurrentWeapon == nWeaponId && m_nCurrentAmmo == nAmmoId) return;
	}

	m_nCurrentWeapon = nWeaponId;

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_nCurrentWeapon);
	if (!pWeapon) return;

	if (nAmmoId != m_nCurrentAmmo)
	{
		m_nCurrentAmmo = nAmmoId;
        g_pHUDMgr->QueueUpdate(kHUDAmmo);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdateHealth()
//
//	PURPOSE:	Update the health stat
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateHealth(uint32 nHealth)
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
//	ROUTINE:	CTO2PlayerStats::UpdateArmor()
//
//	PURPOSE:	Update the armor stat
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateArmor(uint32 nArmor)
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
//	ROUTINE:	CTO2PlayerStats::UpdateMaxHealth()
//
//	PURPOSE:	Update the health stat maximum
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateMaxHealth(uint32 nHealth)
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
//	ROUTINE:	CTO2PlayerStats::UpdateMaxArmor()
//
//	PURPOSE:	Update the armor stat maximum
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateMaxArmor(uint32 nArmor)
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
//	ROUTINE:	CTO2PlayerStats::UpdateAmmo()
//
//	PURPOSE:	Update the ammo stat
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateAmmo(uint8 nWeaponId, uint8 nAmmoId,
                              uint32 nAmmo, LTBOOL bPickedup,
                              LTBOOL bDisplayMsg)
{
	if (!g_pLTClient || !g_pWeaponMgr) return;

	if (bPickedup && g_pWeaponMgr->IsValidWeaponId(nWeaponId))
	{
		if (m_pbHaveWeapon)
		{
			// [KLS 7/12/02] Always display the message if we pick up the weapon
			// even if we already have it...
			// if (!m_pbHaveWeapon[nWeaponId])
			{
				if (bDisplayMsg)
				{
					g_pGameClientShell->HandleWeaponPickup(nWeaponId);
				}
			}

			m_pbHaveWeapon[nWeaponId] = LTTRUE;
			
			if (m_pbCanUseWeapon)
			{
				m_pbCanUseWeapon[nWeaponId] = LTTRUE;
			}
		}	
	}

	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);

	if (bPickedup && pWeapon && m_pbHaveWeapon[nWeaponId])
	{
		for (int m = 0; m < pWeapon->nNumModIds; m++)
		{
			int nModId = pWeapon->aModIds[m];
			MOD const *pMod = g_pWeaponMgr->GetMod(nModId);

			if (pMod && pMod->bIntegrated)
			{
				if (m_pbHaveMod)
				{
					m_pbHaveMod[nModId] = LTTRUE;
					if (m_pbCanUseMod)
						m_pbCanUseMod[nModId] = LTTRUE;
				}
			}
		}
	}

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoId);

    LTBOOL bInfiniteAmmo	= pWeapon ? pWeapon->bInfiniteAmmo : LTFALSE;
	LTBOOL bDefaultAmmo		= pWeapon ? pWeapon->nDefaultAmmoId == nAmmoId : LTFALSE;
	int nEquipWeapon = WMGR_INVALID_ID;

	if (pAmmo )//&& (!bInfiniteAmmo || !bDefaultAmmo) )
	{
		if (m_pnAmmo)
		{
			if (bPickedup) /*** BL 12/08/2000 Added to equip weapon when you are out of ammo and pick up ammo/weapon ******/
			{
				// Check to see if we are out of ammo (minus gadgets, infinite ammo weapons like fisticuffs, barette, etc)

				LTBOOL bOutOfAmmo = LTTRUE;
				int nWeaponBest = WMGR_INVALID_ID;

				for ( int32 iWeapon = g_pWeaponMgr->GetNumWeapons()-1 ; iWeapon >= 0 ; iWeapon-- )
				{
					// There's an assumption that the ammo id
					// is the same as the index, so I've fixed
					// original code to do what it meant to do.
					int32 nWeapon = iWeapon;
					ASSERT ( nWeapon == g_pWeaponMgr->GetWeapon(nWeapon)->nId );

					// Here's the original incorrect line.  (Notice
					// that GetWeaponId wants a CommandID.)
					//int32 nWeapon = g_pWeaponMgr->GetWeaponId(iWeapon);

					if ( nWeapon != WMGR_INVALID_ID && m_pbHaveWeapon[nWeapon] )
					{
						WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWeapon);

						if ( !pWeapon->bInfiniteAmmo )
						{
							for ( int32 iAmmo = 0 ; iAmmo < pWeapon->nNumAmmoIds ; iAmmo++ )
							{
								if (pWeapon->aAmmoIds[iAmmo] == WMGR_INVALID_ID) 
									continue;
								if ( (nWeaponBest == WMGR_INVALID_ID) && (pWeapon->aAmmoIds[iAmmo] == nAmmoId) )
								{
									nWeaponBest = nWeapon;
								}

								AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->aAmmoIds[iAmmo]);

								if ( pAmmo->eType == GADGET )
								{
								}
								else if ( m_pnAmmo[pWeapon->aAmmoIds[iAmmo]] > 0 )
								{
									//g_pLTClient->CPrint("%s has %d rounds of %s", pWeapon->szName, m_pnAmmo[pWeapon->aAmmoTypes[iAmmo]], pAmmo->szName);
									bOutOfAmmo = LTFALSE;
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

					if ( nWeaponId == WMGR_INVALID_ID )
					{
						// Picked up ammo -- equip the best gun for that can use the ammo

						nEquipWeapon = nWeaponBest;
					}
					else
					{
						// Picked up a gun -- equip that gun

						nEquipWeapon = nWeaponId;
					}
				}
				else
				{
					//g_pLTClient->CPrint("Not out of ammo!");
				}
			}  /*** BL 12/08/2000 End changes ******/

			uint32 maxAmmo = pAmmo->GetMaxAmount(LTNULL);
			if (m_pnAmmo[nAmmoId] > maxAmmo)
			{
				m_pnAmmo[nAmmoId] = maxAmmo;
			}

			if (nAmmo > maxAmmo)
			{
				nAmmo = maxAmmo;
			}

			int taken = nAmmo - m_pnAmmo[nAmmoId];

			m_pnAmmo[nAmmoId] = nAmmo;

			// [KLS 7/14/02] Display a message even if we didn't pick up anything...
			if (bPickedup && bDisplayMsg)
			{
				// [KLS 7/12/02] If this ammo is associated with a weapon
				// only display a message if the weapon doesn't use infinite ammo
				if (!pWeapon || !(pWeapon->bInfiniteAmmo))
				{
					bool bTookAmmo = bool(taken != 0);
					g_pGameClientShell->HandleAmmoPickup(nAmmoId, taken, bTookAmmo, nWeaponId);
				}
			}

			if (taken != 0)
			{
				if (m_pbHaveAmmo)
				{
					m_pbHaveAmmo[nAmmoId] = LTTRUE;
					if (m_pbCanUseAmmo)
						m_pbCanUseAmmo[nAmmoId] = LTTRUE;
				}
			}
		}
	}

	if ( nEquipWeapon != WMGR_INVALID_ID )  /*** BL 12/08/2000 Added to change equip weapon when you are out of ammo and pick up a weapon ******/
	{
		g_pPlayerMgr->ChangeWeapon( nEquipWeapon, nAmmoId );
	}  /*** BL 12/08/2000 End changes ******/

	if (m_nCurrentAmmo == nAmmoId)
	{
		g_pHUDMgr->QueueUpdate(kHUDAmmo);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdateGear()
//
//	PURPOSE:	Update the gear stats
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateGear(uint8 nGearId)
{
    if (!g_pLTClient || !g_pWeaponMgr) return;

	if (g_pWeaponMgr->IsValidGearId(nGearId))
	{
        LTBOOL bHadAirSupply = HaveAirSupply();
		if (m_pbHaveGear)
		{
            m_pbHaveGear[nGearId] = LTTRUE;
			if (m_pbCanUseGear)
			{
				GEAR const *pGear = g_pWeaponMgr->GetGear(nGearId);
				if (pGear && pGear->bExclusive)
					m_pbCanUseGear[nGearId] = LTTRUE;
			}

		}

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdateMod()
//
//	PURPOSE:	Update the mod stats
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateMod(uint8 nModId)
{
	if (g_pWeaponMgr->IsValidModId(nModId))
	{
		if (m_pbHaveMod)
		{
			if (!m_pbHaveMod[nModId])
			{
				m_pbHaveMod[nModId] = LTTRUE;
				m_pbCanUseMod[nModId] = LTTRUE;

				// Make sure the player-view weapon is updated if
				// this mod is used with the weapon...

				MOD const *pMod = g_pWeaponMgr->GetMod(nModId);
				if (!pMod) return;

				IClientWeaponBase* pClientWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetCurrentClientWeapon();
				
				if (pClientWeapon && pClientWeapon->GetWeaponId() == pMod->GetWeaponId())
				{
					pClientWeapon->CreateMods();
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdateAir()
//
//	PURPOSE:	Update the air stat
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateAir(LTFLOAT fPercent)
{
    if (!g_pLTClient) return;

	if (fPercent > 1.0f)
		fPercent = 1.0f;
	if (fPercent < 0.0f)
		fPercent = 0.0f;

	if (m_fAirPercent == fPercent) return;

	m_fAirPercent = fPercent;
	g_pHUDMgr->QueueUpdate(kHUDAir);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdateObjectives()
//
//	PURPOSE:	Update the objectives
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateObjectives(uint8 nThing, uint8 nType, uint32 dwId)
{
	switch (nType)
	{
		case ITEM_ADD_ID:
		{
			uint8 nIndex;
			if (m_CompletedObjectives.Have(dwId, nIndex))
			{
				g_pLTClient->CPrint("objective %d already completed", dwId);
			}
			else
			{
				if (nThing == IC_OBJECTIVE_ID && m_Objectives.Add(dwId))
				{
					g_pLTClient->CPrint("objective %d added", dwId);
					m_bObjAdded = LTTRUE;
				}
				else if (nThing == IC_OPTION_ID && m_OptionalObjectives.Add(dwId))
				{
					g_pLTClient->CPrint("optional objective %d added", dwId);
					m_bObjAdded = LTTRUE;
				}
				else if (nThing == IC_PARAMETER_ID && m_Parameters.Add(dwId))
				{
					g_pLTClient->CPrint("mission parameter %d added", dwId);
					m_bObjAdded = LTTRUE;
				}
			}
		}
		break;

		case ITEM_REMOVE_ID:
		{
			m_bObjRemoved = m_Objectives.Remove(dwId);
			m_bObjRemoved |= m_OptionalObjectives.Remove(dwId);
			m_bObjRemoved |= m_Parameters.Remove(dwId);
			m_bObjRemoved |= m_CompletedObjectives.Remove(dwId);

			if (m_bObjRemoved)
				g_pLTClient->CPrint("objective %d removed", dwId);


		}
		break;

		case ITEM_COMPLETE_ID:
		{
			m_Objectives.Remove(dwId);
			m_OptionalObjectives.Remove(dwId);
			m_bObjCompleted = m_CompletedObjectives.Add(dwId);
			g_pLTClient->CPrint("objective %d completed", dwId);

		}
		break;

		case ITEM_CLEAR_ID:
		{
			if (m_Objectives.m_IDArray.size() || m_OptionalObjectives.m_IDArray.size() ||
				m_CompletedObjectives.m_IDArray.size() || m_Parameters.m_IDArray.size())
                m_bObjRemoved = LTTRUE;

			m_Objectives.Clear();
			m_OptionalObjectives.Clear();
			m_CompletedObjectives.Clear();
			m_Parameters.Clear();

			g_pLTClient->CPrint("objectives cleared", dwId);

		}
		break;

		default :
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdateKeys()
//
//	PURPOSE:	Update the key item list
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateKeys(uint8 nType, uint16 nId)
{
	switch (nType)
	{
		case ITEM_ADD_ID:
		{
			if (g_pKeyMgr->IsValidKey(nId))
			{
				m_Keys.Add(nId);
				KEY* pKey = g_pKeyMgr->GetKey(nId);
				g_pPickupMsgs->AddMessage(LoadTempString(pKey->nNameId),pKey->szImage);
			}
		}
		break;

		case ITEM_REMOVE_ID:
		{
			if (g_pKeyMgr->IsValidKey(nId))
			{
				m_Keys.Remove(nId);
			}
		}
		break;

		case ITEM_CLEAR_ID:
		{
			m_Keys.Clear();
		}
		break;

		default :
		break;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdateIntel()
//
//	PURPOSE:	Update the intel item list
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateIntel(uint32 nTextId, uint8 nPopupId, LTBOOL bIsIntel)
{
	uint16 nIndex = m_IntelList.GetIndex(nTextId);
	if (!m_IntelList.IsValid(nIndex))
	{
		INTEL_ITEM* pItem = debug_new(INTEL_ITEM);
		pItem->nTextId = nTextId;
		pItem->nPopupId = nPopupId;
		pItem->bIsIntel = bIsIntel;
		pItem->nMissionNum = (uint8)g_pMissionMgr->GetCurrentMission();
		m_IntelList.Add(pItem);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::UpdateHiding()
//
//	PURPOSE:	Update the player hiding state
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::UpdateHiding(LTBOOL bIsHiding, LTBOOL bIsHidden, LTBOOL bCanthide, float fHideDuration)
{
	if( bIsHidden && !m_bHidden )
	{
		g_pClientSoundMgr->PlaySoundLocal("interface\\snd\\playerhidden.wav", SOUNDPRIORITY_MISC_MEDIUM);
	}

	m_bHiding = bIsHiding;
	m_bHidden = bIsHidden;
	m_bCantHide = bCanthide;
	m_fHideDuration = fHideDuration;
	g_pHUDMgr->QueueUpdate(kHUDHiding);
}


void CTO2PlayerStats::UpdateSkills(ILTMessage_Read *pMsg)
{
	m_nTotalSkillPoints = pMsg->Readuint32();
	m_nAvailSkillPoints = pMsg->Readuint32();

	for (uint8 i = 0; i < kNumSkills; i++)
	{
		m_nSkills[i] = pMsg->Readuint8();
	}

}

void CTO2PlayerStats::UpdateMissionStats(ILTMessage_Read *pMsg)
{
	m_MissionStats.ReadData(pMsg);

}


void CTO2PlayerStats::GainSkills(uint8 nRewardId, uint8 nBonusId, uint16 nAmount)
{
	if (IsMultiplayerGame()) return;


	//deal with bonus points here
	if (nBonusId && nAmount)
	{
		m_nTotalSkillPoints += nAmount;
		m_nAvailSkillPoints += nAmount;

		// Inform the user that they gained a reward...

		g_pRewardMsgs->AddMessage(FormatTempString(IDS_REWARD_MSG_2,nAmount));
		g_pRewardMsgs->AddMessage(FormatTempString(IDS_TOTAL_REWARD_PTS, m_nAvailSkillPoints));

		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\reward.wav");
		return;
	}

	int nCurrentMission = g_pMissionMgr->GetCurrentMission();

	if (nCurrentMission < 0 || nCurrentMission >= g_pMissionButeMgr->GetNumMissions()) 
	{
		if (!nAmount)
			g_pLTClient->CPrint("CTO2PlayerStats::GainSkills Invalid current mission number (%d)", nCurrentMission);
		else
		{
			m_nTotalSkillPoints += nAmount;
			m_nAvailSkillPoints += nAmount;
			g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\reward.wav");
		}
		return;
	}

	MISSION *pMission = g_pMissionButeMgr->GetMission(nCurrentMission);
	if (!pMission)
	{
		g_pLTClient->CPrint("CTO2PlayerStats::GainSkills Invalid mission (%d)", nCurrentMission);
		return;
	}

	if (nRewardId >= pMission->nNumRewards)
	{
		if (!nAmount)
			g_pLTClient->CPrint("CTO2PlayerStats::GainSkills Invalid reward id (%d)", nRewardId);
		else
		{
			m_nTotalSkillPoints += nAmount;
			m_nAvailSkillPoints += nAmount;
			g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\reward.wav");
		}
		return;
	}

	uint32 nPoints = pMission->aRewards[nRewardId].nVal;
	m_nTotalSkillPoints += nPoints;
	m_nAvailSkillPoints += nPoints;

	// Inform the user that they gained a reward...

	g_pRewardMsgs->AddMessage(FormatTempString(pMission->aRewards[nRewardId].nDescriptionId,nPoints));
	g_pRewardMsgs->AddMessage(FormatTempString(IDS_TOTAL_REWARD_PTS, m_nAvailSkillPoints));
	g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\reward.wav");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::OnObjectivesDataMessage()
//
//	PURPOSE:	Override current objectives with new list from server
//
// ----------------------------------------------------------------------- //

void CTO2PlayerStats::OnObjectivesDataMessage(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	if (m_Objectives.m_IDArray.size() || m_CompletedObjectives.m_IDArray.size())
        m_bObjRemoved = LTTRUE;
	m_Objectives.Clear();
	m_CompletedObjectives.Clear();

	//read number of objectives
    uint8 nNumObj = pMsg->Readuint8();

	//read list of objectives
    for (uint8 i = 0; i < nNumObj; i++)
	{
        uint32 dwId = pMsg->Readuint32();
		m_Objectives.Add(dwId);
	}

	//read number of optional objectives
    uint8 nNumOptObj = pMsg->Readuint8();

	//read list of optional objectives
	for (uint8 i = 0; i < nNumOptObj; i++)
	{
        uint32 dwId = pMsg->Readuint32();
		m_OptionalObjectives.Add(dwId);
	}

	//read number of completed objectives
    uint8 nNumCompObj = pMsg->Readuint8();

	//read list of completed objectives
	for (uint8 i = 0; i < nNumCompObj; i++)
	{
        uint32 dwId = pMsg->Readuint32();
		m_CompletedObjectives.Add(dwId);
	}

	//read number of parameters
    uint8 nNumParam = pMsg->Readuint8();

	//read list of parameters
    for (uint8 i = 0; i < nNumParam; i++)
	{
        uint32 dwId = pMsg->Readuint32();
		m_Parameters.Add(dwId);
	}

	m_bObjAdded = (nNumObj + nNumOptObj + nNumCompObj + nNumParam) > 0;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Save
//
//	PURPOSE:	Save the player stats info
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::Save(ILTMessage_Write *pMsg)
{
    if (!pMsg || !g_pWeaponMgr) return;

    m_Objectives.Save(pMsg);
    m_OptionalObjectives.Save(pMsg);
    m_CompletedObjectives.Save(pMsg);
    m_Parameters.Save(pMsg);
    m_Keys.Save(pMsg);
    m_IntelList.Save(pMsg);

    pMsg->Writeuint32(m_nHealth);
    pMsg->Writeuint32(m_nDamage);
    pMsg->Writeuint32(m_nArmor);
    pMsg->Writeuint32(m_nMaxHealth);
    pMsg->Writeuint32(m_nMaxArmor);

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();
    uint8 i;
    for (i = 0; i < nNumAmmoTypes; i++)
	{
        pMsg->Writebool(m_pbCanUseAmmo[i] != LTFALSE);
        pMsg->Writebool(m_pbHaveAmmo[i] != LTFALSE);
        pMsg->Writeuint32(m_pnAmmo[i]);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	for (i = 0; i < nNumWeapons; i++)
	{
        pMsg->Writebool(m_pbCanUseWeapon[i] != LTFALSE);
        pMsg->Writebool(m_pbHaveWeapon[i] != LTFALSE);
	}

	int nNumMods = g_pWeaponMgr->GetNumModIds();
	for (i = 0; i < nNumMods; i++)
	{
        pMsg->Writebool(m_pbCanUseMod[i] != LTFALSE);
        pMsg->Writebool(m_pbHaveMod[i] != LTFALSE);
	}

	int nNumGear = g_pWeaponMgr->GetNumGearIds();
	for (i = 0; i < nNumGear; i++)
	{
        pMsg->Writebool(m_pbCanUseGear[i] != LTFALSE);
        pMsg->Writebool(m_pbHaveGear[i] != LTFALSE);
	}

    pMsg->Writeuint8(m_nCurrentWeapon);
    pMsg->Writeuint8(m_nCurrentAmmo);
    pMsg->Writefloat(m_fAirPercent);

    pMsg->Writebool(m_bHiding != LTFALSE);
    pMsg->Writebool(m_bHidden != LTFALSE);

	pMsg->Writeuint32(m_nTotalSkillPoints);
	pMsg->Writeuint32(m_nAvailSkillPoints);
	for (uint32 nCurSkill = 0; nCurSkill < kNumSkills; ++nCurSkill)
		pMsg->Writeuint8(m_nSkills[nCurSkill]);

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Load
//
//	PURPOSE:	Load the player stats info
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::Load(ILTMessage_Read *pMsg)
{
    if (!pMsg || !g_pWeaponMgr) return;

    m_Objectives.Load(pMsg);
    m_OptionalObjectives.Load(pMsg);
    m_CompletedObjectives.Load(pMsg);
    m_Parameters.Load(pMsg);
    m_Keys.Load(pMsg);
    m_IntelList.Load(pMsg);

    m_nHealth           = pMsg->Readuint32();
    m_nDamage           = pMsg->Readuint32();
    m_nArmor            = pMsg->Readuint32();
    m_nMaxHealth        = pMsg->Readuint32();
    m_nMaxArmor         = pMsg->Readuint32();


    uint8 i;
	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();
    for (i = 0; i < nNumAmmoTypes; i++)
	{
        m_pbCanUseAmmo[i]   = pMsg->Readbool() ? LTTRUE : LTFALSE;
        m_pbHaveAmmo[i] = pMsg->Readbool() ? LTTRUE : LTFALSE;
        m_pnAmmo[i]         = pMsg->Readuint32();
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	for ( i = 0; i < nNumWeapons; i++)
	{
        m_pbCanUseWeapon[i] = pMsg->Readbool() ? LTTRUE : LTFALSE;
        m_pbHaveWeapon[i] = pMsg->Readbool() ? LTTRUE : LTFALSE;
	}

	int nNumMods = g_pWeaponMgr->GetNumModIds();
	for ( i = 0; i < nNumMods; i++)
	{
        m_pbCanUseMod[i] = pMsg->Readbool() ? LTTRUE : LTFALSE;
        m_pbHaveMod[i] = pMsg->Readbool() ? LTTRUE : LTFALSE;
	}

	int nNumGear = g_pWeaponMgr->GetNumGearIds();
	for ( i = 0; i < nNumGear; i++)
	{
        m_pbCanUseGear[i] = pMsg->Readbool() ? LTTRUE : LTFALSE;
        m_pbHaveGear[i] = pMsg->Readbool() ? LTTRUE : LTFALSE;
	}

    m_nCurrentWeapon    = pMsg->Readuint8();
    m_nCurrentAmmo      = pMsg->Readuint8();
    m_fAirPercent       = pMsg->Readfloat();

    m_bHiding			= pMsg->Readbool() ? LTTRUE : LTFALSE;
    m_bHidden			= pMsg->Readbool() ? LTTRUE : LTFALSE;

	m_nTotalSkillPoints = pMsg->Readuint32();
	m_nAvailSkillPoints = pMsg->Readuint32();
	for (uint32 nCurSkill = 0; nCurSkill < kNumSkills; ++nCurSkill)
		m_nSkills[nCurSkill] = pMsg->Readuint8();

    UpdatePlayerWeapon(m_nCurrentWeapon, m_nCurrentAmmo, LTTRUE);
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::GetMod
//
//	PURPOSE:	Get the id of thefirst mod for the current weapon
//				of the given type
//
// --------------------------------------------------------------------------- //

uint8 CTO2PlayerStats::GetMod(ModType eType, const WEAPON* pW/*=LTNULL*/)
{
	uint8 nModId	= WMGR_INVALID_ID;
	int nPriority	= -1;

	// [KLS - 02/13/02] Updated to support checking for mods on weapons other than
	// the current weapon...
	WEAPON const *pWpn = (pW ? pW : g_pWeaponMgr->GetWeapon(m_nCurrentWeapon));

	if (pWpn)
	{
		for (int i=0; i < pWpn->nNumModIds; i++)
		{
			if (HaveMod(pWpn->aModIds[i]))
			{
				MOD const *pMod = g_pWeaponMgr->GetMod(pWpn->aModIds[i]);

				if (pMod && pMod->eType == eType)
				{
					// Get the mod with the hightest priority...

					if (pMod->nPriority > nPriority)
					{
						nPriority = pMod->nPriority;
						nModId = pWpn->aModIds[i];
					}
				}
			}
		}
	}


	return nModId;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::GetAmmoCount
//
//	PURPOSE:	Get the ammo count for the passed in ammo id
//
// --------------------------------------------------------------------------- //

uint32 CTO2PlayerStats::GetAmmoCount(uint8 nAmmoId) const
{
	 if (!m_pnAmmo || !g_pWeaponMgr->IsValidAmmoId(nAmmoId))  return 0;

	 return m_pnAmmo[nAmmoId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::HaveWeapon
//
//	PURPOSE:	Do we have the weapon associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::HaveWeapon(uint8 nWeaponId) const
{
     if (!m_pbHaveWeapon || !g_pWeaponMgr->IsValidWeaponId(nWeaponId)) return LTFALSE;

	 return m_pbHaveWeapon[nWeaponId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::CanUseWeapon
//
//	PURPOSE:	Can we use the weapon associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::CanUseWeapon(uint8 nWeaponId) const
{
     if (!m_pbCanUseWeapon || !g_pWeaponMgr->IsValidWeaponId(nWeaponId) || !g_pWeaponMgr->IsPlayerWeapon(nWeaponId)) return LTFALSE;

	 return m_pbCanUseWeapon[nWeaponId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::HaveMod
//
//	PURPOSE:	Do we have the mod associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::HaveMod(uint8 nModId) const
{
     if (!m_pbHaveMod || !g_pWeaponMgr->IsValidModId(nModId)) return LTFALSE;

	 return m_pbHaveMod[nModId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::CanUseMod
//
//	PURPOSE:	Can we use the mod associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::CanUseMod(uint8 nModId) const
{
     if (!m_pbCanUseMod || !g_pWeaponMgr->IsValidModId(nModId)) return LTFALSE;

	 return m_pbCanUseMod[nModId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::HaveGear
//
//	PURPOSE:	Do we have the Gear associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::HaveGear(uint8 nGearId) const
{
     if (!m_pbHaveGear || !g_pWeaponMgr->IsValidGearId(nGearId)) return LTFALSE;

	 return m_pbHaveGear[nGearId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::CanUseGear
//
//	PURPOSE:	Can we use the Gear associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::CanUseGear(uint8 nGearId) const
{
     if (!m_pbCanUseGear || !g_pWeaponMgr->IsValidGearId(nGearId)) return LTFALSE;

	 return m_pbCanUseGear[nGearId];
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::CanUseAmmo
//
//	PURPOSE:	Can we use the ammo associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::CanUseAmmo(uint8 nAmmoId) const
{
     if (!m_pbCanUseAmmo || !g_pWeaponMgr->IsValidAmmoId(nAmmoId)) return LTFALSE;

	 return m_pbCanUseAmmo[nAmmoId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::AddCanUseWeapon
//
//	PURPOSE:	Add a weapon to our can use list
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::AddCanUseWeapon(uint8 nWeaponId)
{
	 if (!m_pbCanUseWeapon || !g_pWeaponMgr->IsValidWeaponId(nWeaponId) || !g_pWeaponMgr->IsPlayerWeapon(nWeaponId)) return;

     m_pbCanUseWeapon[nWeaponId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::AddCanUseMod
//
//	PURPOSE:	Add a mod to our can use list
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::AddCanUseMod(uint8 nModId)
{
	 if (!m_pbCanUseMod || !g_pWeaponMgr->IsValidModId(nModId)) return;

     m_pbCanUseMod[nModId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::AddCanUseGear
//
//	PURPOSE:	Add a Gear to our can use list
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::AddCanUseGear(uint8 nGearId)
{
	 if (!m_pbCanUseGear || !g_pWeaponMgr->IsValidGearId(nGearId)) return;

     m_pbCanUseGear[nGearId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::AddCanUseAmmo
//
//	PURPOSE:	Add the ammo to our can use list
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::AddCanUseAmmo(uint8 nAmmoId)
{
	 if (!m_pbCanUseAmmo || !g_pWeaponMgr->IsValidAmmoId(nAmmoId)) return;

     m_pbCanUseAmmo[nAmmoId] = LTTRUE;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Setup
//
//	PURPOSE:	Setup the stats
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::Setup( )
{
	if (!m_pnAmmo || !m_pbHaveWeapon || !m_pbCanUseAmmo || !m_pbCanUseWeapon) return;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::ResetInventory
//
//	PURPOSE:	Reset all inventory items
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::ResetInventory()
{
	// Clear our data...
	
	g_pPlayerMgr->GetClientWeaponMgr()->ResetWeapons();

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();
	if (nNumAmmoTypes > 0)
	{
        memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmoTypes);
        memset(m_pbHaveAmmo, 0, sizeof(LTBOOL) * nNumAmmoTypes);
        memset(m_pbCanUseAmmo, 0, sizeof(LTBOOL) * nNumAmmoTypes);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
        memset(m_pbHaveWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
        memset(m_pbCanUseWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
	}

	int nNumMods = g_pWeaponMgr->GetNumModIds();
	if (nNumMods > 0)
	{
		memset(m_pbHaveMod, 0, sizeof(LTBOOL) * nNumMods);
		memset(m_pbCanUseMod, 0, sizeof(LTBOOL) * nNumMods);
	}

	int nNumGear = g_pWeaponMgr->GetNumGearIds();
	if (nNumGear > 0)
	{
		memset(m_pbHaveGear, 0, sizeof(LTBOOL) * nNumGear);
		memset(m_pbCanUseGear, 0, sizeof(LTBOOL) * nNumGear);
	}

	m_Keys.Clear();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::DropInventory
//
//	PURPOSE:	Removes all currently carried weapons and ammo, optionally
//				removes gear and mods
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::DropInventory(LTBOOL bRemoveGear)
{
	// Clear our data...

	g_pPlayerMgr->GetClientWeaponMgr()->ResetWeapons();

	int nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();
	if (nNumAmmoTypes > 0)
	{
        memset(m_pnAmmo, 0, sizeof(uint32) * nNumAmmoTypes);
        memset(m_pbHaveAmmo, 0, sizeof(LTBOOL) * nNumAmmoTypes);
	}

	int nNumWeapons = g_pWeaponMgr->GetNumWeapons();
	if (nNumWeapons > 0)
	{
        memset(m_pbHaveWeapon, 0, sizeof(LTBOOL) * nNumWeapons);
	}

	if (bRemoveGear)
	{
		int nNumMods = g_pWeaponMgr->GetNumModIds();
		if (nNumMods > 0)
		{
			memset(m_pbHaveMod, 0, sizeof(LTBOOL) * nNumMods);
		}

		int nNumGear = g_pWeaponMgr->GetNumGearIds();
		if (nNumGear > 0)
		{
			memset(m_pbHaveGear, 0, sizeof(LTBOOL) * nNumGear);
		}

		m_Keys.Clear();

	}

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Skills
//
//	PURPOSE:	Reset all skill data
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::ResetSkills()
{
	// Clear our data...
	m_nTotalSkillPoints = 0;
	m_nAvailSkillPoints = 0;
	memset(m_nSkills, 0, sizeof(m_nSkills));


}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::Objectives
//
//	PURPOSE:	Reset all Objective data
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::ResetObjectives()
{
	// Clear our data...
	m_Objectives.Clear();
	m_OptionalObjectives.Clear();
	m_CompletedObjectives.Clear();
	m_Parameters.Clear();

	if (g_pHUDMgr)
	{
		g_pObjectives->Show(false);
		g_pHUDMgr->QueueUpdate(kHUDObjectives);
	}


}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::RestMissionStats
//
//	PURPOSE:	Reset all mission data
//
// --------------------------------------------------------------------------- //

void CTO2PlayerStats::ResetMissionStats()
{
	// Clear our data...
	m_MissionStats.Init();
}

void CTO2PlayerStats::ClearMissionInfo()
{
	ResetMissionStats();
	ResetObjectives();
	m_nDamage = 0;
	m_Keys.Clear();
	m_IntelList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTO2PlayerStats::HaveAirSupply
//
//	PURPOSE:	Returns true if current gear provides air supply
//
// ----------------------------------------------------------------------- //

LTBOOL CTO2PlayerStats::HaveAirSupply()
{
    LTBOOL bAir = LTFALSE;
    GEAR const *pGear = LTNULL;

	int numGear = g_pWeaponMgr->GetNumGearIds();
	for (int nGearId=0; nGearId < numGear && !bAir; nGearId++)
	{
		if (m_pbHaveGear[nGearId])
		{
			pGear = g_pWeaponMgr->GetGear(nGearId);
			if (pGear)
			{
				bAir = ( (pGear->eProtectionType == DT_CHOKE) && (pGear->fProtection >= 1.0f) );
			}
		}
	}
	return bAir;
}


uint32 CTO2PlayerStats::GetCurrentAmmoCount()
{
	if (!m_pnAmmo || m_nCurrentAmmo < 0 ||	m_nCurrentAmmo >= g_pWeaponMgr->GetNumAmmoIds())
		return 0;
	return m_pnAmmo[m_nCurrentAmmo];
}



//Skills and experience functions
const RANK* CTO2PlayerStats::GetRank()
{
	return g_pSkillsButeMgr->GetRank(m_nTotalSkillPoints);
}

uint32  CTO2PlayerStats::GetCostToUpgrade(eSkill skill)
{
	uint8 nTgtLevel = m_nSkills[skill] + 1;
	if (nTgtLevel < kNumSkills)
		return g_pSkillsButeMgr->GetCostToUpgrade(skill,(eSkillLevel)nTgtLevel);
	else
		return -1;
}


void CTO2PlayerStats::SetObjectivesSeen()
{
	g_pObjectives->Show(false);
}

float CTO2PlayerStats::GetSkillModifier(eSkill skl, uint8 nMod)
{
	return g_pSkillsButeMgr->GetModifier(skl, (eSkillLevel)m_nSkills[skl], nMod);
}

