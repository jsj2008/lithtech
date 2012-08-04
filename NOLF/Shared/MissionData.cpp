// MissionData.cpp: implementation of the CMissionData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MissionData.h"
#include "WeaponMgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMissionData::CMissionData()
{
	m_nMission = -1;
	m_nLevel = -1;

	m_Weapons.SetSize(0);
	m_Ammo.SetSize(0);

}

CMissionData::~CMissionData()
{
	Clear();
}


void CMissionData::Clear()
{
	m_nMission = -1;
	m_nLevel = -1;
	ClearWeaponsAndGadgets();
	ClearAllAmmo();
	ClearMods();
	ClearGear();
}

void CMissionData::NewMission(int mission)
{
	m_nMission = mission;
	ClearWeaponsAndGadgets();
	ClearAllAmmo();
	ClearMods();
	ClearGear();
}

void CMissionData::ClearWeaponsAndGadgets()
{
	while (m_Weapons.GetSize() > 0)
	{
		debug_delete(m_Weapons[0]);
		m_Weapons.Remove(0);
	}
}

void CMissionData::ClearWeapons()
{
    uint32 index = 0;
	while (index < m_Weapons.GetSize())
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_Weapons[index]->m_nID);
		if (pWeapon)
		{
			if (pWeapon->IsAGadget())
			{
				index++;
			}
			else
			{
				debug_delete(m_Weapons[index]);
				m_Weapons.Remove(index);
			}
		}
	}
}

void CMissionData::ClearGadgets()
{
    uint32 index = 0;
	while (index < m_Weapons.GetSize())
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_Weapons[index]->m_nID);
		if (pWeapon)
		{
			if (!pWeapon->IsAGadget())
			{
				index++;
			}
			else
			{
				debug_delete(m_Weapons[index]);
				m_Weapons.Remove(index);
			}
		}
	}
}

int CMissionData::GetNumWeapons()
{

	int nCount = 0;
	for (uint32 index = 0;index < m_Weapons.GetSize();index++)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_Weapons[index]->m_nID);
		if (pWeapon)
		{
			if (!pWeapon->IsAGadget())
			{
				nCount++;
			}
		}
	}
	return nCount;
}

int CMissionData::GetNumGadgets()
{
	int nCount = 0;
	for (uint32 index = 0;index < m_Weapons.GetSize();index++)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_Weapons[index]->m_nID);
		if (pWeapon)
		{
			if (pWeapon->IsAGadget())
			{
				nCount++;
			}
		}
	}
	return nCount;
}


LTBOOL CMissionData::AddWeapon(int weaponID)
{
	CWeaponData* pData = GetWeaponData(weaponID);
    if (pData == LTNULL)
	{
		pData = debug_new(CWeaponData);
		m_Weapons.Add(pData);
		pData->m_nID = weaponID;
        return LTTRUE;
	}

    return LTFALSE;
}

LTBOOL CMissionData::RemoveWeapon(int weaponID)
{
	DWORD ndx = GetWeaponIndex(weaponID);
	if (ndx >= m_Weapons.GetSize())
        return LTFALSE;
	debug_delete(m_Weapons[ndx]);
	m_Weapons.Remove(ndx);
    return LTTRUE;
}

int CMissionData::GetWeapons(CWeaponData **weapons, int nArraySize)
{
    int i;
    for (i = 0; i < nArraySize && i < (int)m_Weapons.GetSize(); i++)
	{
		*weapons = m_Weapons.GetAt(i);
		weapons++;
	}
	return i;
}

CWeaponData* CMissionData::GetWeaponData(int nID)
{
	DWORD ndx = GetWeaponIndex(nID);
	if (ndx >= m_Weapons.GetSize())
        return LTNULL;
	return m_Weapons[ndx];
}



void CMissionData::ClearAllAmmo()
{
	while (m_Ammo.GetSize() > 0)
	{
		debug_delete(m_Ammo[0]);
		m_Ammo.Remove(0);
	}
}


void CMissionData::ClearAmmo()
{
    uint32 index = 0;
	while (index < m_Ammo.GetSize())
	{
		AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_Ammo[index]->m_nID);
		if (pAmmo)
		{
			if (pAmmo->eType == GADGET)
			{
				index++;
			}
			else
			{
				debug_delete(m_Ammo[index]);
				m_Ammo.Remove(index);
			}
		}
	}
}

void CMissionData::ClearSupplies()
{
    uint32 index = 0;
	while (index < m_Ammo.GetSize())
	{
		AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_Ammo[index]->m_nID);
		if (pAmmo)
		{
			if (pAmmo->eType != GADGET)
			{
				index++;
			}
			else
			{
				debug_delete(m_Ammo[index]);
				m_Ammo.Remove(index);
			}
		}
	}
}



LTBOOL CMissionData::AddAmmo(int ammoID, int count)
{
	if (count == 0)
        return LTFALSE;
	CAmmoData* pData = GetAmmoData(ammoID);
    if (pData == LTNULL)
	{
		pData = debug_new(CAmmoData);
		m_Ammo.Add(pData);
		pData->m_nID = ammoID;
	}

	if (!g_pWeaponMgr->IsValidAmmoType(ammoID))
        return LTFALSE;


	pData->m_nCount += count;

    return LTTRUE;
}

LTBOOL CMissionData::RemoveAmmo(int ammoID)
{
	DWORD ndx = GetAmmoIndex(ammoID);
	if (ndx >= m_Ammo.GetSize())
        return LTFALSE;
	debug_delete(m_Ammo[ndx]);
	m_Ammo.Remove(ndx);
    return LTTRUE;
}

int CMissionData::GetAmmo(CAmmoData **ammo, int nArraySize)
{
    int i;
    for (i = 0; i < nArraySize && i < (int)m_Ammo.GetSize(); i++)
	{
		*ammo = m_Ammo.GetAt(i);
		ammo++;
	}
	return i;
}

CAmmoData*	CMissionData::GetAmmoData(int nID)
{
	DWORD ndx = GetAmmoIndex(nID);
	if (ndx >= m_Ammo.GetSize())
        return LTNULL;
	return m_Ammo[ndx];
}




void CMissionData::ClearMods()
{
	while (m_Mods.GetSize() > 0)
	{
		debug_delete(m_Mods[0]);
		m_Mods.Remove(0);
	}
}

LTBOOL CMissionData::AddMod(int modID)
{
	CModData* pData = GetModData(modID);
    if (pData == LTNULL)
	{
		pData = debug_new(CModData);
		m_Mods.Add(pData);
		pData->m_nID = modID;
        return LTTRUE;
	}

    return LTFALSE;
}

LTBOOL CMissionData::RemoveMod(int modID)
{
	DWORD ndx = GetModIndex(modID);
	if (ndx >= m_Mods.GetSize())
        return LTFALSE;
	debug_delete(m_Mods[ndx]);
	m_Mods.Remove(ndx);
    return LTTRUE;
}

int CMissionData::GetMods(CModData **mod, int nArraySize)
{
    int i;
    for (i = 0; i < nArraySize && i < (int)m_Mods.GetSize(); i++)
	{
		*mod = m_Mods.GetAt(i);
		mod++;
	}
	return i;
}

CModData*	CMissionData::GetModData(int nID)
{
	DWORD ndx = GetModIndex(nID);
	if (ndx >= m_Mods.GetSize())
        return LTNULL;
	return m_Mods[ndx];
}

void CMissionData::ClearGear()
{
	while (m_Gear.GetSize() > 0)
	{
		debug_delete(m_Gear[0]);
		m_Gear.Remove(0);
	}
}

LTBOOL CMissionData::AddGear(int gearID)
{
	CGearData* pData = GetGearData(gearID);
    if (pData == LTNULL)
	{
		pData = debug_new(CGearData);
		m_Gear.Add(pData);
		pData->m_nID = gearID;
        return LTTRUE;
	}

    return LTFALSE;
}

LTBOOL CMissionData::RemoveGear(int gearID)
{
	DWORD ndx = GetGearIndex(gearID);
	if (ndx >= m_Gear.GetSize())
        return LTFALSE;
	debug_delete(m_Gear[ndx]);
	m_Gear.Remove(ndx);
    return LTTRUE;
}

int CMissionData::GetGear(CGearData **gear, int nArraySize)
{
    int i;
    for (i = 0; i < nArraySize && i < (int)m_Gear.GetSize(); i++)
	{
		*gear = m_Gear.GetAt(i);
		gear++;
	}
	return i;
}

CGearData*	CMissionData::GetGearData(int nID)
{
	DWORD ndx = GetGearIndex(nID);
	if (ndx >= m_Gear.GetSize())
        return LTNULL;
	return m_Gear[ndx];
}



DWORD CMissionData::GetAmmoIndex(int nID)
{
	DWORD ndx = 0;
	while (ndx < m_Ammo.GetSize() && m_Ammo[ndx]->m_nID != nID)
		ndx++;
	return ndx;
}

DWORD CMissionData::GetModIndex(int nID)
{
	DWORD ndx = 0;
	while (ndx < m_Mods.GetSize() && m_Mods[ndx]->m_nID != nID)
		ndx++;
	return ndx;
}

DWORD CMissionData::GetGearIndex(int nID)
{
	DWORD ndx = 0;
	while (ndx < m_Gear.GetSize() && m_Gear[ndx]->m_nID != nID)
		ndx++;
	return ndx;
}

DWORD CMissionData::GetWeaponIndex(int nID)
{
	DWORD ndx = 0;
	while (ndx < m_Weapons.GetSize() && m_Weapons[ndx]->m_nID != nID)
		ndx++;
	return ndx;
}

void CMissionData::WriteToMessage(ILTCSBase *pInterface, HMESSAGEWRITE hMessage)
{
	if (!hMessage) return;

    pInterface->WriteToMessageFloat(hMessage, (LTFLOAT)m_nMission);
    pInterface->WriteToMessageFloat(hMessage, (LTFLOAT)m_nLevel);

	int nSize = m_Weapons.GetSize();

    pInterface->WriteToMessageDWord(hMessage, (uint32)nSize);

    int i;
    for (i = 0; i < nSize; i++)
	{
		CWeaponData *pWeapon = m_Weapons.GetAt(i);
		if (pWeapon)
		{
            pWeapon->WriteToMessage(pInterface, hMessage);
		}
	}

	nSize = m_Ammo.GetSize();
    pInterface->WriteToMessageDWord(hMessage, (uint32)nSize);

	for (i = 0; i < nSize; i++)
	{
		CAmmoData *pAmmo = m_Ammo.GetAt(i);
		if (pAmmo)
		{
            pAmmo->WriteToMessage(pInterface, hMessage);
		}
	}

	nSize = m_Mods.GetSize();
    pInterface->WriteToMessageDWord(hMessage, (uint32)nSize);

	for (i = 0; i < nSize; i++)
	{
		CModData *pMods = m_Mods.GetAt(i);
		if (pMods)
		{
            pMods->WriteToMessage(pInterface, hMessage);
		}
	}

	nSize = m_Gear.GetSize();
    pInterface->WriteToMessageDWord(hMessage, (uint32)nSize);

	for (i = 0; i < nSize; i++)
	{
		CGearData *pGear = m_Gear.GetAt(i);
		if (pGear)
		{
            pGear->WriteToMessage(pInterface, hMessage);
		}
	}


}

void CMissionData::ReadFromMessage(ILTCSBase *pInterface, HMESSAGEREAD hMessage)
{
	if (!hMessage) return;

	// Clear our data...

	Clear();

	// Read in the new data...

    m_nMission = (int) pInterface->ReadFromMessageFloat(hMessage);
    m_nLevel   = (int) pInterface->ReadFromMessageFloat(hMessage);

    int nSize = (int) pInterface->ReadFromMessageDWord(hMessage);

    int i;
	CWeaponData Weapon;
    for (i = 0; i < nSize; i++)
	{
        Weapon.ReadFromMessage(pInterface, hMessage);
		AddWeapon(Weapon.m_nID);
	}

    nSize = (int) pInterface->ReadFromMessageDWord(hMessage);

	CAmmoData Ammo;
	for (i = 0; i < nSize; i++)
	{
        Ammo.ReadFromMessage(pInterface, hMessage);
		AddAmmo(Ammo.m_nID, Ammo.m_nCount);
	}

    nSize = (int) pInterface->ReadFromMessageDWord(hMessage);

	CModData Mod;
	for (i = 0; i < nSize; i++)
	{
        Mod.ReadFromMessage(pInterface, hMessage);
		AddMod(Mod.m_nID);
	}

    nSize = (int) pInterface->ReadFromMessageDWord(hMessage);

	CGearData Gear;
	for (i = 0; i < nSize; i++)
	{
        Gear.ReadFromMessage(pInterface, hMessage);
		AddGear(Gear.m_nID);
	}
}

