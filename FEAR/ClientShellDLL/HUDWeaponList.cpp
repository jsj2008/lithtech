// ----------------------------------------------------------------------- //
//
// MODULE  : HUDWeaponList.cpp
//
// PURPOSE : HUD Element displaying a list of weapon items
//
// CREATED : 12/17/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDWeaponList.h"
#include "ClientWeaponMgr.h"
#include "CommandIDs.h"

// ----------------------------------------------------------------------- //
// Contructor
// ----------------------------------------------------------------------- //
CHUDWeaponList::CHUDWeaponList()
{
	m_UpdateFlags = kHUDWeapon;
	m_hSelectedWeapon = NULL;
}


// ----------------------------------------------------------------------- //
// Initialization
// ----------------------------------------------------------------------- //
bool CHUDWeaponList::Init()
{
	UpdateLayout();

	CHUDWeapon *pWpn = NULL;
	int nCommand = COMMAND_ID_WEAPON_BASE;
	for (uint8 nSlot = 0; nSlot < g_pWeaponDB->GetWeaponCapacity() && nCommand <= COMMAND_ID_WEAPON_MAX; ++nSlot, ++nCommand)
	{
		pWpn = debug_new(CHUDWeapon);
		pWpn->Init();
		pWpn->SetWeaponRecord(NULL);
		m_Items.push_back(pWpn);
	}

	ScaleChanged();
	UpdatePos();
	return true;

}

void CHUDWeaponList::Reset()
{
	uint8 nIndex = 0;

	int nCommand = COMMAND_ID_WEAPON_BASE;
	for (uint8 nSlot = 0; nSlot < g_pWeaponDB->GetWeaponCapacity() && nCommand <= COMMAND_ID_WEAPON_MAX; ++nSlot, ++nCommand)
	{
		HWEAPON hWeapon = g_pPlayerStats->GetWeaponInSlot( nSlot );

		CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(m_Items[nIndex]);
		pWpn->SetWeaponRecord(hWeapon);
		nIndex++;
	}



}


// ----------------------------------------------------------------------- //
// Clean up before removal - delete allocated objects
// ----------------------------------------------------------------------- //
void CHUDWeaponList::Term()
{
	//default cleanup
	CHUDInventoryList::Term();
}


// ----------------------------------------------------------------------- //
// Find the index of an item matching the given weapon
// ----------------------------------------------------------------------- //
uint8 CHUDWeaponList::FindWeapon(HWEAPON hWeapon)
{
	if (!hWeapon) 
		return WDB_INVALID_WEAPON_INDEX;

	for (uint8 i = 0; i < m_Items.size(); ++i)
	{
		CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(m_Items[i]);
		if (pWpn->GetWeaponRecord() == hWeapon)
		{
			return i;
		}
	}

	return WDB_INVALID_WEAPON_INDEX;
}


// ----------------------------------------------------------------------- //
// Add a weapon item
// ----------------------------------------------------------------------- //
void CHUDWeaponList::SetWeapon(HWEAPON hWeapon, uint8 nSlot )
{
	if (!hWeapon) return;
	if (nSlot >= g_pWeaponDB->GetWeaponCapacity())
		return;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	if (!g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bTakesInventorySlot)) 
		return;

	CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(m_Items[nSlot]);
	pWpn->SetWeaponRecord(hWeapon);
			
	UpdatePos();
}

// ----------------------------------------------------------------------- //
// Remove a weapon item
// ----------------------------------------------------------------------- //
void CHUDWeaponList::RemoveWeapon(HWEAPON hWeapon)
{
	if (!hWeapon) return;

	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end())
	{
		CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(*iter);
		HWEAPON hTest = pWpn->GetWeaponRecord();
		if (hTest == hWeapon)
		{
			pWpn->SetWeaponRecord(NULL);
			return;
		}
		++iter;
	}
}

// ----------------------------------------------------------------------- //
// Remove a weapon item in a particular slot
// ----------------------------------------------------------------------- //
void CHUDWeaponList::RemoveWeaponFromSlot( uint8 nSlot )
{
	if (nSlot >= m_Items.size())
		return;

	CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(m_Items[nSlot]);
	if (pWpn)
	{
		pWpn->SetWeaponRecord(NULL);
	}

}


// ----------------------------------------------------------------------- //
// Update the hotkey display for each weapon
// ----------------------------------------------------------------------- //
void CHUDWeaponList::UpdateTriggerNames()
{
	uint8 nIndex = 0;
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	int nCommand = COMMAND_ID_WEAPON_BASE;
	for (uint8 nSlot = 0; nSlot < g_pWeaponDB->GetWeaponCapacity() && nCommand <= COMMAND_ID_WEAPON_MAX; ++nSlot, ++nCommand)
	{
		CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(m_Items[nIndex]);
		pWpn->UpdateTriggerName(pProfile->GetTriggerNameFromCommandID(nCommand));
		nIndex++;
	}
}

void CHUDWeaponList::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDWeapon");
	}

	CHUDInventoryList::UpdateLayout();

	m_vItemOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,3);
	m_vSelectedItemOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,0);

	ScaleChanged();
}

// ----------------------------------------------------------------------- //
// Update our items
// ----------------------------------------------------------------------- //
void CHUDWeaponList::Update()
{
	HWEAPON hCurrentWeapon = g_pClientWeaponMgr->GetRequestedWeaponRecord();
	if (!hCurrentWeapon)
	{
		hCurrentWeapon = g_pPlayerStats->GetCurrentWeaponRecord();
	}

	if (m_hSelectedWeapon != hCurrentWeapon)
	{
		for (uint8 i = 0; i < m_Items.size(); ++i)
		{
			CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(m_Items[i]);

			if (pWpn->GetWeaponRecord() == hCurrentWeapon)
			{
				pWpn->Select(true);
			}
			if (pWpn->GetWeaponRecord() == m_hSelectedWeapon)
			{
				pWpn->Select(false);
			}

			if( pWpn->GetWeaponRecord( ))
			{
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(pWpn->GetWeaponRecord(), !USE_AI_DATA);
				if (!g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsGrenade))
				{
					pWpn->ResetFade();
				}		
			}
		}

		m_hSelectedWeapon = hCurrentWeapon;
		UpdatePos();

	}

	CHUDInventoryList::Update();
}

// ----------------------------------------------------------------------- //
// position each item on screen
// ----------------------------------------------------------------------- //
void CHUDWeaponList::UpdatePos()
{
	LTVector2n vCurPos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(vCurPos);

	// the position of our first item is the same as our base position
	// each subsequent item is placed relative to the last using m_vItemOffset
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->SetBasePos(vCurPos);
		CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(*iter);
		if (pWpn->IsSelected())
			vCurPos += m_vSelectedItemOffset;
		else
			vCurPos += m_vItemOffset;
		++iter;
	}
}


// ----------------------------------------------------------------------- //
// Contructor
// ----------------------------------------------------------------------- //
CHUDGrenadeList::CHUDGrenadeList()
{
	m_UpdateFlags = kHUDWeapon;
}


// ----------------------------------------------------------------------- //
// Initialization
// ----------------------------------------------------------------------- //
bool CHUDGrenadeList::Init()
{
	UpdateLayout();

	CHUDWeapon *pWpn = NULL;
	uint8 nIndex = 0;
	for (int nCommand = COMMAND_ID_GRENADE_BASE; nIndex < g_pWeaponDB->GetNumPlayerGrenades() && nCommand <= COMMAND_ID_GRENADE_MAX; ++nCommand, ++nIndex)
	{
		pWpn = debug_new(CHUDWeapon);
		pWpn->Init();
		pWpn->SetWeaponRecord(g_pWeaponDB->GetPlayerGrenade(nIndex));
		m_Items.push_back(pWpn);
	}

	ScaleChanged();
	UpdatePos();
	return true;

}


// ----------------------------------------------------------------------- //
// Clean up before removal - delete allocated objects
// ----------------------------------------------------------------------- //
void CHUDGrenadeList::Term()
{
	//default cleanup
	CHUDInventoryList::Term();
}



// ----------------------------------------------------------------------- //
// Update the hotkey display for each weapon
// ----------------------------------------------------------------------- //
void CHUDGrenadeList::UpdateTriggerNames()
{
	uint8 nIndex = 0;
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	uint8 nGrenade = 0;
	for (int nCommand = COMMAND_ID_GRENADE_BASE; nGrenade < g_pWeaponDB->GetNumPlayerGrenades() && nCommand <= COMMAND_ID_GRENADE_MAX; ++nCommand, ++nGrenade)
	{
		CHUDWeapon *pWpn = dynamic_cast<CHUDWeapon*>(m_Items[nIndex]);
		pWpn->UpdateTriggerName(pProfile->GetTriggerNameFromCommandID(nCommand));
		nIndex++;
	}
}

void CHUDGrenadeList::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDGrenadeList");
	}

	CHUDInventoryList::UpdateLayout();

	m_vItemOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,3);

	ScaleChanged();
}

// ----------------------------------------------------------------------- //
// Update our items
// ----------------------------------------------------------------------- //
void CHUDGrenadeList::Update()
{
	CHUDInventoryList::Update();
}

// ----------------------------------------------------------------------- //
// position each item on screen
// ----------------------------------------------------------------------- //
void CHUDGrenadeList::UpdatePos()
{
	LTVector2n vCurPos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(vCurPos);

	// the position of our first item is the same as our base position
	// each subsequent item is placed relative to the last using m_vItemOffset
	InventoryArray::iterator iter = m_Items.begin();
	while (iter != m_Items.end() )
	{
		(*iter)->SetBasePos(vCurPos);
		vCurPos += m_vItemOffset;
		++iter;
	}
}