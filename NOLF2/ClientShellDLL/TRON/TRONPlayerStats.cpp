// ----------------------------------------------------------------------- //
//
// MODULE  : TronPlayerStats.cpp
//
// PURPOSE : Implementation of PlayerStats class
//
// CREATED : 10/9/97
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "TronPlayerStats.h"
#include "GameClientShell.h"
#include "MsgIDs.h"
#include "TronHUDMgr.h"
#include "ClientWeaponBase.h"
#include "ClientWeaponMgr.h"
#include "MissionMgr.h"
#include "KeyMgr.h"

CTronPlayerStats* g_pTronPlayerStats = LTNULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::CTronPlayerStats()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CTronPlayerStats::CTronPlayerStats()
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
	m_nEnergy			= 100;
	m_nMaxEnergy		= 100;
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

	m_fSubroutineArmor	= 0.0f;
	m_fVirusArmor		= 0.0f;
	m_fArmorScalar		= 1.0f;

	memset(m_nSkills,0,sizeof(m_nSkills));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::~CTronPlayerStats()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CTronPlayerStats::~CTronPlayerStats()
{
	Term();
	g_pPlayerStats = LTNULL;
	g_pTronPlayerStats = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::Init()
{
    if (!g_pLTClient) return LTFALSE;

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
	ResetMissionStats();

	m_RatingMgr.Init("attributes\\additives.txt");

	g_pPlayerStats = this;
	g_pTronPlayerStats = this;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::Term()
//
//	PURPOSE:	Terminate the player stats
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::Term()
{
    if (!g_pLTClient) return;

	m_nHealth		= 0;
	m_nDamage		= 0;
	m_nArmor		= 0;
	m_nMaxHealth	= 0;
	m_nMaxArmor		= 0;
	m_nEnergy		= 0;
	m_nMaxEnergy	= 0;

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

	m_RatingMgr.Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::OnEnterWorld()
//
//	PURPOSE:	Handle entering the world
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::OnEnterWorld(LTBOOL bRestoringGame)
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
//	ROUTINE:	CTronPlayerStats::OnExitWorld()
//
//	PURPOSE:	Handle exiting the world
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::OnExitWorld()
{
    if (!g_pLTClient) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::Clear()
//
//	PURPOSE:	Handle clearing the stats
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::Clear()
{
	UpdateHealth(0);
	m_nDamage = 0;
	UpdateArmor(0);
	UpdateMaxEnergy(100);
	UpdateEnergy(100);

	// Get rid of any weapons, ammo, gear or mods..
	
	ResetInventory();
	ResetSkills();
	ResetMissionStats();


	ClearPermissions();
	SetJetVersion(100); // version 1.0.0
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::ResetStats()
//
//	PURPOSE:	Reset the stats
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::ResetStats()
{
	m_MissionStats.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdatePlayerWeapon()
//
//	PURPOSE:	Update the weapon related stats
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdatePlayerWeapon(uint8 nWeaponId, uint8 nAmmoId, LTBOOL bForce)
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
//        g_pHUDMgr->QueueUpdate(kHUDAmmo);
	}
	g_pHUDMgr->QueueUpdate(kHUDWeapons);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateHealth()
//
//	PURPOSE:	Update the health stat
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateHealth(uint32 nHealth)
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
//	ROUTINE:	CTronPlayerStats::UpdateArmor()
//
//	PURPOSE:	Update the armor stat
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateArmor(uint32 nArmor)
{
	if (nArmor > m_nMaxArmor)
		nArmor = m_nMaxArmor;
	if (m_nArmor == nArmor) return;

	// update the member variable
	m_nArmor = nArmor;
//    g_pHUDMgr->QueueUpdate(kHUDArmor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateMaxHealth()
//
//	PURPOSE:	Update the health stat maximum
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateMaxHealth(uint32 nHealth)
{
	if (m_nMaxHealth == nHealth) return;

	// update the member variable
	m_nMaxHealth = nHealth;

    g_pHUDMgr->QueueUpdate(kHUDHealth);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateEnergy()
//
//	PURPOSE:	Update the energy stat
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateEnergy(uint32 nEnergy)
{
	if (nEnergy > m_nMaxEnergy)
		nEnergy = m_nMaxEnergy;

	// update the member variable
	m_nEnergy = nEnergy;
    g_pHUDMgr->QueueUpdate(kHUDEnergy);
}

uint32 CTronPlayerStats::GetEnergy()
{
	return m_nEnergy;
}	// Energy levels

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateMaxEnergy()
//
//	PURPOSE:	Update the energy stat maximum
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateMaxEnergy(uint32 nEnergy)
{
	// update the member variable
	m_nMaxEnergy = nEnergy;

	if (m_nEnergy > m_nMaxEnergy)
		m_nEnergy = m_nMaxEnergy;

    g_pHUDMgr->QueueUpdate(kHUDEnergy);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateMaxArmor()
//
//	PURPOSE:	Update the armor stat maximum
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateMaxArmor(uint32 nArmor)
{
	if (m_nMaxArmor == nArmor) return;

	// update the member variable
	m_nMaxArmor = nArmor;

	//if we have more than our max... reduce
	if (m_nArmor > m_nMaxArmor)
		m_nArmor = m_nMaxArmor;
//    g_pHUDMgr->QueueUpdate(kHUDArmor);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateAmmo()
//
//	PURPOSE:	Update the ammo stat
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateAmmo(uint8 nWeaponId, uint8 nAmmoId,
                                  uint32 nAmmo, LTBOOL bPickedup,
                                  LTBOOL bDisplayMsg)
{
	if (!g_pLTClient || !g_pWeaponMgr) return;

	if (bPickedup && g_pWeaponMgr->IsValidWeaponId(nWeaponId))
	{
		if (m_pbHaveWeapon)
		{
			if (!m_pbHaveWeapon[nWeaponId])
			{
				g_pHUDMgr->QueueUpdate(kHUDWeapons);
				if (bDisplayMsg)
					g_pGameClientShell->HandleWeaponPickup(nWeaponId);
			}

			m_pbHaveWeapon[nWeaponId] = LTTRUE;
			if (m_pbCanUseWeapon)
				m_pbCanUseWeapon[nWeaponId] = LTTRUE;

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

	LTBOOL bInfiniteAmmo = pWeapon ? pWeapon->bInfiniteAmmo : LTFALSE;
	LTBOOL bDefaultAmmo		= pWeapon ? pWeapon->nDefaultAmmoId == nAmmoId : LTFALSE;
	int nEquipWeapon = WMGR_INVALID_ID;

	if (pAmmo && (!bInfiniteAmmo || !bDefaultAmmo) )
	{
		if (m_pnAmmo)
		{
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

			if (taken != 0)
			{
				if (bPickedup && bDisplayMsg)
				{
					g_pGameClientShell->HandleAmmoPickup(nAmmoId,taken);
				}

				if (m_pbHaveAmmo)
				{
					m_pbHaveAmmo[nAmmoId] = LTTRUE;
					if (m_pbCanUseAmmo)
						m_pbCanUseAmmo[nAmmoId] = LTTRUE;
				}
			}
		}
	}

	if (m_nCurrentAmmo == nAmmoId)
	{
		g_pHUDMgr->QueueUpdate(kHUDAmmo);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateGear()
//
//	PURPOSE:	Update the gear stats
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateGear(uint8 nGearId)
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
//	ROUTINE:	CTronPlayerStats::UpdateMod()
//
//	PURPOSE:	Update the mod stats
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateMod(uint8 nModId)
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

				IClientWeaponBase* pClientWeapon = g_pPlayerMgr->
				        GetClientWeaponMgr()->GetCurrentClientWeapon();
				
				if (pClientWeapon && 
				    (pClientWeapon->GetWeaponId() == pMod->GetWeaponId()))
				{
					pClientWeapon->CreateMods();
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateAir()
//
//	PURPOSE:	Update the air stat
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateAir(LTFLOAT fPercent)
{
    if (!g_pLTClient) return;

	if (fPercent > 1.0f)
		fPercent = 1.0f;
	if (fPercent < 0.0f)
		fPercent = 0.0f;

	if (m_fAirPercent == fPercent) return;

	m_fAirPercent = fPercent;
//	g_pHUDMgr->QueueUpdate(kHUDAir);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateObjectives()
//
//	PURPOSE:	Update the objectives
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateObjectives(uint8 nThing, uint8 nType, uint32 dwId)
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
	g_pHUDMgr->QueueUpdate(kHUDObjectives);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::UpdateKeys()
//
//	PURPOSE:	Update the key item list
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateKeys(uint8 nType, uint16 nId)
{
	switch (nType)
	{
		case ITEM_ADD_ID:
		{
			if (g_pKeyMgr->IsValidKey(nId))
			{
				m_Keys.Add(nId);
				KEY* pKey = g_pKeyMgr->GetKey(nId);
				char szName[128];
				LoadString(pKey->nNameId,szName,sizeof(szName));
				g_pPickupMsgs->AddMessage(FormatTempString(IDS_KEY_PICKUP,szName),NULL);
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
//	ROUTINE:	CTronPlayerStats::UpdateIntel()
//
//	PURPOSE:	Update the intel item list
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateIntel(uint32 nTextId, uint8 nPopupId, LTBOOL bIsIntel)
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
//	ROUTINE:	CTronPlayerStats::UpdateHiding()
//
//	PURPOSE:	Update the player hiding state
//
// ----------------------------------------------------------------------- //

void CTronPlayerStats::UpdateHiding(LTBOOL bIsHiding,LTBOOL bIsHidden)
{
/*
	if (bIsHiding && !m_bHiding)
		g_pLTClient->CPrint("Starting to hide");
	if (bIsHidden && !m_bHidden)
		g_pLTClient->CPrint("Hidden");
	if (!bIsHiding && !bIsHidden)
		g_pLTClient->CPrint("Not hidden");
*/		

	m_bHiding = bIsHiding;
	m_bHidden = bIsHidden;
//	g_pHUDMgr->QueueUpdate(kHUDHiding);
}


void CTronPlayerStats::UpdateSkills(ILTMessage_Read *pMsg)
{
	m_nTotalSkillPoints = pMsg->Readuint32();
	m_nAvailSkillPoints = pMsg->Readuint32();

	for (uint8 i = 0; i < kNumSkills; i++)
	{
		m_nSkills[i] = pMsg->Readuint8();
	}

}

void CTronPlayerStats::UpdateMissionStats(ILTMessage_Read *pMsg)
{
	m_MissionStats.ReadData(pMsg);
}

void CTronPlayerStats::GainSkills(uint8 nRewardId)
{
	int nCurrentMission = g_pMissionMgr->GetCurrentMission();

	if (nCurrentMission < 0 || nCurrentMission >= g_pMissionButeMgr->GetNumMissions()) 
	{
		g_pLTClient->CPrint("CTronPlayerStats::GainSkills Invalid current mission number (%d)", nCurrentMission);
		return;
	}

	MISSION *pMission = g_pMissionButeMgr->GetMission(nCurrentMission);
	if (!pMission)
	{
		g_pLTClient->CPrint("CTronPlayerStats::GainSkills Invalid mission (%d)", nCurrentMission);
		return;
	}

	if (nRewardId >= pMission->nNumRewards)
	{
		g_pLTClient->CPrint("CTronPlayerStats::GainSkills Invalid reward id (%d)", nRewardId);
		return;
	}

	uint32 nPoints = pMission->aRewards[nRewardId].nVal;
	m_nTotalSkillPoints += nPoints;
	m_nAvailSkillPoints += nPoints;

	// Inform the user that they gained a reward...

	g_pRewardMsgs->AddMessage(FormatTempString(pMission->aRewards[nRewardId].nDescriptionId,nPoints));
	g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\reward.wav");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::SetMultiplayerObjectives()
//
//	PURPOSE:	Override current objectives with new list from server
//
// ----------------------------------------------------------------------- //
void CTronPlayerStats::SetMultiplayerObjectives(ILTMessage_Read *pMsg)
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
	for (i = 0; i < nNumOptObj; i++)
	{
        uint32 dwId = pMsg->Readuint32();
		m_OptionalObjectives.Add(dwId);
	}

	//read number of completed objectives
    uint8 nNumCompObj = pMsg->Readuint8();

	//read list of completed objectives
	for (i = 0; i < nNumCompObj; i++)
	{
        uint32 dwId = pMsg->Readuint32();
		m_CompletedObjectives.Add(dwId);
	}

	//read number of parameters
    uint8 nNumParam = pMsg->Readuint8();

	//read list of parameters
    for (i = 0; i < nNumParam; i++)
	{
        uint32 dwId = pMsg->Readuint32();
		m_Parameters.Add(dwId);
	}

	m_bObjAdded = (nNumObj + nNumOptObj + nNumCompObj + nNumParam) > 0;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::Save
//
//	PURPOSE:	Save the player stats info
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::Save(ILTMessage_Write *pMsg)
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
	pMsg->Writeuint32(m_nEnergy);
	pMsg->Writeuint32(m_nMaxEnergy);

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


}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::Load
//
//	PURPOSE:	Load the player stats info
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::Load(ILTMessage_Read *pMsg)
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
	m_nEnergy			= pMsg->Readuint32();
	m_nMaxEnergy		= pMsg->Readuint32();


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

    UpdatePlayerWeapon(m_nCurrentWeapon, m_nCurrentAmmo, LTTRUE);
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::GetMod
//
//	PURPOSE:	Get the id of thefirst mod for the current weapon
//				of the given type
//
// --------------------------------------------------------------------------- //

uint8 CTronPlayerStats::GetMod(ModType eType, const WEAPON* pW/*=LTNULL*/)
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
//	ROUTINE:	CTronPlayerStats::GetAmmoCount
//
//	PURPOSE:	Get the ammo count for the passed in ammo id
//
// --------------------------------------------------------------------------- //

uint32 CTronPlayerStats::GetAmmoCount(uint8 nAmmoId) const
{
	 if (!m_pnAmmo || !g_pWeaponMgr->IsValidAmmoId(nAmmoId))  return 0;

	 return m_pnAmmo[nAmmoId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::HaveWeapon
//
//	PURPOSE:	Do we have the weapon associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::HaveWeapon(uint8 nWeaponId) const
{
     if (!m_pbHaveWeapon || !g_pWeaponMgr->IsValidWeaponId(nWeaponId)) return LTFALSE;

	 return m_pbHaveWeapon[nWeaponId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::CanUseWeapon
//
//	PURPOSE:	Can we use the weapon associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::CanUseWeapon(uint8 nWeaponId) const
{
     if (!m_pbCanUseWeapon || !g_pWeaponMgr->IsValidWeaponId(nWeaponId) || !g_pWeaponMgr->IsPlayerWeapon(nWeaponId)) return LTFALSE;

	 return m_pbCanUseWeapon[nWeaponId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::HaveMod
//
//	PURPOSE:	Do we have the mod associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::HaveMod(uint8 nModId) const
{
     if (!m_pbHaveMod || !g_pWeaponMgr->IsValidModId(nModId)) return LTFALSE;

	 return m_pbHaveMod[nModId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::CanUseMod
//
//	PURPOSE:	Can we use the mod associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::CanUseMod(uint8 nModId) const
{
     if (!m_pbCanUseMod || !g_pWeaponMgr->IsValidModId(nModId)) return LTFALSE;

	 return m_pbCanUseMod[nModId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::HaveGear
//
//	PURPOSE:	Do we have the Gear associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::HaveGear(uint8 nGearId) const
{
     if (!m_pbHaveGear || !g_pWeaponMgr->IsValidGearId(nGearId)) return LTFALSE;

	 return m_pbHaveGear[nGearId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::CanUseGear
//
//	PURPOSE:	Can we use the Gear associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::CanUseGear(uint8 nGearId) const
{
     if (!m_pbCanUseGear || !g_pWeaponMgr->IsValidGearId(nGearId)) return LTFALSE;

	 return m_pbCanUseGear[nGearId];
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::CanUseAmmo
//
//	PURPOSE:	Can we use the ammo associated with the passed in id
//
// --------------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::CanUseAmmo(uint8 nAmmoId) const
{
     if (!m_pbCanUseAmmo || !g_pWeaponMgr->IsValidAmmoId(nAmmoId)) return LTFALSE;

	 return m_pbCanUseAmmo[nAmmoId];
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::AddCanUseWeapon
//
//	PURPOSE:	Add a weapon to our can use list
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::AddCanUseWeapon(uint8 nWeaponId)
{
	 if (!m_pbCanUseWeapon || !g_pWeaponMgr->IsValidWeaponId(nWeaponId) || !g_pWeaponMgr->IsPlayerWeapon(nWeaponId)) return;

     m_pbCanUseWeapon[nWeaponId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::AddCanUseMod
//
//	PURPOSE:	Add a mod to our can use list
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::AddCanUseMod(uint8 nModId)
{
	 if (!m_pbCanUseMod || !g_pWeaponMgr->IsValidModId(nModId)) return;

     m_pbCanUseMod[nModId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::AddCanUseGear
//
//	PURPOSE:	Add a Gear to our can use list
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::AddCanUseGear(uint8 nGearId)
{
	 if (!m_pbCanUseGear || !g_pWeaponMgr->IsValidGearId(nGearId)) return;

     m_pbCanUseGear[nGearId] = LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::AddCanUseAmmo
//
//	PURPOSE:	Add the ammo to our can use list
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::AddCanUseAmmo(uint8 nAmmoId)
{
	 if (!m_pbCanUseAmmo || !g_pWeaponMgr->IsValidAmmoId(nAmmoId)) return;

     m_pbCanUseAmmo[nAmmoId] = LTTRUE;
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::Setup
//
//	PURPOSE:	Setup the stats based on the CMissionData info
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::Setup( )
{
	if (!m_pnAmmo || !m_pbHaveWeapon || !m_pbCanUseAmmo || !m_pbCanUseWeapon) return;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::ResetInventory
//
//	PURPOSE:	Reset all inventory items
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::ResetInventory()
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
//	ROUTINE:	CTronPlayerStats::DropInventory
//
//	PURPOSE:	Removes all currently carried weapons and ammo, optionally
//				removes gear and mods
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::DropInventory(LTBOOL bRemoveGear)
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
//	ROUTINE:	CTronPlayerStats::Skills
//
//	PURPOSE:	Reset all skill data
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::ResetSkills()
{
	// Clear our data...
	m_nTotalSkillPoints = 0;
	m_nAvailSkillPoints = 0;
	memset(m_nSkills, 0, sizeof(m_nSkills));


}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::RestMissionStats
//
//	PURPOSE:	Reset all mission data
//
// --------------------------------------------------------------------------- //

void CTronPlayerStats::ResetMissionStats()
{
	// Clear our data...
	m_MissionStats.Init();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CTronPlayerStats::HaveAirSupply
//
//	PURPOSE:	Returns true if current gear provides air supply
//
// ----------------------------------------------------------------------- //

LTBOOL CTronPlayerStats::HaveAirSupply()
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


uint32 CTronPlayerStats::GetCurrentAmmoCount()
{
	if (!m_pnAmmo || m_nCurrentAmmo < 0 ||	m_nCurrentAmmo >= g_pWeaponMgr->GetNumAmmoIds())
		return 0;
	return m_pnAmmo[m_nCurrentAmmo];
}



//Skills and experience functions
const RANK* CTronPlayerStats::GetRank()
{
	return g_pSkillsButeMgr->GetRank(m_nTotalSkillPoints);
}

uint32  CTronPlayerStats::GetCostToUpgrade(eSkill skill)
{
	uint8 nTgtLevel = m_nSkills[skill] + 1;
	if (nTgtLevel < kNumSkills)
		return g_pSkillsButeMgr->GetCostToUpgrade(skill,(eSkillLevel)nTgtLevel);
	else
		return -1;
}
// ABM new stuff

// -------------------------------------------------------------------
// Permission Sets
// -------------------------------------------------------------------

void CTronPlayerStats::SetPermissions(uint8 nPermissionSet)
{
	m_nPermissions = nPermissionSet;
	g_pHUDMgr->QueueUpdate(kHUDPermissions);
}

void CTronPlayerStats::SetRequiredPermissions(uint8 nPermissionSet)
{
	m_nRequiredPermissions = nPermissionSet;
	g_pHUDMgr->QueueUpdate(kHUDPermissions);
}

void CTronPlayerStats::ClearPermissions()
{
	m_nPermissions = 0;
	g_pHUDMgr->QueueUpdate(kHUDPermissions);
}

void CTronPlayerStats::ClearRequiredPermissions()
{
	m_nRequiredPermissions = 0;
	g_pHUDMgr->QueueUpdate(kHUDPermissions);
}

bool CTronPlayerStats::HasRightPermissions()
{
	if (m_nRequiredPermissions - (m_nRequiredPermissions & m_nPermissions) != 0)
		return false;

	return true;
}

bool CTronPlayerStats::HasPermission(uint8 nPermission)
{
	// out of range? Let's say we have it, then.
	if (nPermission < 0 || nPermission > 7)
		return true;

	if (m_nPermissions & (1 << nPermission))
		return true;

	return false;
}

bool CTronPlayerStats::HasPermissionSet(uint8 nPermissionSet)
{
	if (nPermissionSet - (nPermissionSet& m_nPermissions) != 0)
		return false;

	return true;
}

void CTronPlayerStats::GetPermissions(uint8 &iPermissions, uint8 &iRequiredPermissions)
{
	iPermissions = m_nPermissions;
	iRequiredPermissions = m_nRequiredPermissions;
}

// -------------------------------------------------------------------
// Jet's version
// -------------------------------------------------------------------
void CTronPlayerStats::SetJetVersion(uint16 iVersion)
{
	int iPreviousVersion = m_nVersion;
	if (iVersion != 100)
	{
		if ((iVersion / 100) > (iPreviousVersion / 100))
		{
			// FIXME hook up the upgrade indicator
		}
	}
	m_nVersion = iVersion;
	g_pHUDMgr->QueueUpdate(kHUDVersion);
}

void CTronPlayerStats::GetJetVersion(uint16 &iVersion)
{
	iVersion = m_nVersion;
}

void CTronPlayerStats::IncrementJetVersion(uint8 iVersionPoints)
{
	uint16 iOldVersion = m_nVersion;
	m_nVersion += iVersionPoints;
	g_pHUDMgr->QueueUpdate(kHUDVersion);

	// Have we gone up a major version number?
	if ((m_nVersion / 100) > (iOldVersion / 100))
	{
		// FIXME hook up the upgrade indicator
	}
}

void CTronPlayerStats::UpdateSubroutineArmor(float fArmor, float fVirus, float fScalar)
{
	// Clipping
	if (fArmor < 0.0f) fArmor = 0.0f;
	if (fArmor > 1.0f) fArmor = 1.0f;
	if (fVirus < 0.0f) fVirus = 0.0f;
	if (fVirus > 1.0f) fVirus = 1.0f;
	if (fScalar < 0.0f) fScalar = 0.0f;
	if (fScalar > 1.0f) fScalar = 1.0f;

	m_fSubroutineArmor = fArmor;
	m_fVirusArmor = fVirus;
	m_fArmorScalar = fScalar;

	g_pHUDMgr->QueueUpdate(kHUDArmor);
}


// -------------------------------------------------------------------
// CTronPlayerStats::Compile()
//
// Called by the Subroutine/Ratings screen when the player hits the
// Compile() button.  Is responsible for preparing a message to send
// back to the PlayerObj containing all the information that the
// PlayerObj wants about ratings and armor
// -------------------------------------------------------------------

void CTronPlayerStats::Compile()
{
	// Set up a message
	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_COMPILE);

	// the percentage of armor
	cMsg.Writefloat(m_fSubroutineArmor);

	// percentage protection from infection
	cMsg.Writefloat(m_fVirusArmor);

	// A float (0-1) on how much to scale the damage to energy points
	cMsg.Writefloat(m_fArmorScalar);

	// Include a byte for each performance rating
	for (int i = 0; i < NUM_RATINGS; i++)
	{
		uint8 iRating;
		iRating = (uint8)GetRating((PerformanceRating)i);
		cMsg.Writeuint8(iRating);
	}

	// End of line.
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);  
}
