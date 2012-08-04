/****************************************************************************
;
;	MODULE:			InventoryButeMgr.cpp
;
;	PURPOSE:		Inventory Bute manager
;
;	HISTORY:		3/20/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/


#include "stdafx.h"
#include "InventoryButeMgr.h"

// Globals/statics

CInventoryButeMgr* g_pInventoryButeMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttName[100];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::CInventoryButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CInventoryButeMgr::CInventoryButeMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::~CInventoryButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //
CInventoryButeMgr::~CInventoryButeMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //
LTBOOL CInventoryButeMgr::Init(const char* szAttributeFile)
{
    if (g_pInventoryButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile))
	{
		char szError[512];
		sprintf( szError, "CInventoryButeMgr::Init: Failed to parse %s", szAttributeFile );
		return LTFALSE;
	}

	// Set up global pointer
	g_pInventoryButeMgr = this;

	// Read Goal Sets.
	uint32 iItem = 0;
	sprintf(s_aTagName, "%s%d", "Item", iItem);
	
	while(m_buteMgr.Exist(s_aTagName))
	{
		ReadItem();
		iItem++;
		sprintf(s_aTagName, "%s%d", "Item", iItem);
	}

	m_buteMgr.Term();
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //
void CInventoryButeMgr::Term()
{
	m_lstItems.clear();
    g_pInventoryButeMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::ReadItem
//
//	PURPOSE:	Reads an item
//
// ----------------------------------------------------------------------- //
void CInventoryButeMgr::ReadItem()
{
	// Create new goal set and add it to list.
	GEN_INVENTORY_ITEM_DEF def;

	sprintf(def.szName,m_buteMgr.GetString(s_aTagName,"Name",""));
	sprintf(def.szCmd,m_buteMgr.GetString(s_aTagName,"Cmd",""));
	
	m_lstItems.push_back(def);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::GetItemIndex
//
//	PURPOSE:	Gets index of an item by name.
//
// ----------------------------------------------------------------------- //
int CInventoryButeMgr::GetItemIndex(const char* szItemName)
{
	ASSERT(szItemName[0] != LTNULL);

	// Iterate and look for a match
	uint32 iItem = 0;
	GEN_INVENTORY_ITEM_DEF_LIST::iterator it;
	for(it = m_lstItems.begin(); it != m_lstItems.end(); ++it)
	{
		if(stricmp(it->szName, szItemName) == 0)
		{
			// Found it!
			return iItem;
		}

		iItem++;
	}

	return -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::GetItemName
//
//	PURPOSE:	Gets an item name by index
//
// ----------------------------------------------------------------------- //
char* CInventoryButeMgr::GetItemName(const uint32 iItem)
{
	if(iItem < m_lstItems.size())
	{
		return(m_lstItems[iItem].szName);
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::GetItemCommand
//
//	PURPOSE:	Gets an item command by index
//
// ----------------------------------------------------------------------- //
char* CInventoryButeMgr::GetItemCommand(const uint32 iItem)
{
	if(iItem < m_lstItems.size())
	{
		return(m_lstItems[iItem].szCmd);
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryButeMgr::GetItem
//
//	PURPOSE:	Gets an item by name.
//
// ----------------------------------------------------------------------- //
/*bool CInventoryButeMgr::GetItem(const char* szItem, GEN_INVENTORY_ITEM_DEF& def)
{
	if(!szItem || !szItem[0])
		return false;

	// Iterate and look for a match
	GEN_INVENTORY_ITEM_DEF_LIST::iterator it;
	for(it = m_lstItems.begin(); it != m_lstItems.end(); ++it)
	{
		if(stricmp(it->szName, szItem) == 0)
		{
			// Found it!
			strcpy(def.szName,it->szName);
			return true;
		}
	}

	return false;
}*/