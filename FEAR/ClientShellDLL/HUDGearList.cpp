// ----------------------------------------------------------------------- //
//
// MODULE  : HUDGearList.cpp
//
// PURPOSE : HUD element displaying a list of gear items
//
// CREATED : 12/16/03
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "HUDGearList.h"

CHUDGearList* CHUDGearList::g_pGearList = NULL;

// ----------------------------------------------------------------------- //
// Contructor
// ----------------------------------------------------------------------- //
CHUDGearList::CHUDGearList()
{
	m_UpdateFlags = kHUDGear;
	LTASSERT(!g_pGearList,"Multiple instances of CHUDGearList created");
	g_pGearList = this;
}


// ----------------------------------------------------------------------- //
// Retrieve "singleton"
// ----------------------------------------------------------------------- //
CHUDGearList& CHUDGearList::Instance()
{
	return (*g_pGearList);
}


// ----------------------------------------------------------------------- //
// Initialize list
// ----------------------------------------------------------------------- //
bool CHUDGearList::Init()
{

	uint8 nNumGear = g_pWeaponDB->GetNumDisplayGear();

	for (uint8 i = 0; i < nNumGear; ++i)
	{
		HGEAR hGear = g_pWeaponDB->GetDisplayGear(i);

		if (hGear && g_pWeaponDB->GetInt32(hGear,WDB_GEAR_nMaxAmount) > 0)
		{
			CHUDGear *pGear = debug_new(CHUDGear);
			if (pGear->Init())
			{
				pGear->SetGearRecord(hGear);
				m_Items.push_back(pGear);
			}
			else
			{
				debug_delete(pGear);
			}
		}
	}

	UpdateLayout();
	UpdatePos();

	return true;

}

void CHUDGearList::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDGear");
	}

	CHUDInventoryList::UpdateLayout();

	m_vItemOffset = g_pLayoutDB->GetPosition(m_hLayout,LDB_HUDAddPoint,0);
}