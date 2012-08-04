// ----------------------------------------------------------------------- //
//
// MODULE  : InventoryButeMgr.h
//
// PURPOSE : Read templates for Goals.
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _INVENTORYBUTE_MGR_H_
#define _INVENTORYBUTE_MGR_H_

#include "GameButeMgr.h"
#include "GeneralInventory.h"

// Defines
#define INVENTORY_FILE			"Attributes\\Inventory.txt"

// Forward declarations.
class  CInventoryButeMgr;

// Globals.
extern CInventoryButeMgr* g_pInventoryButeMgr;

// Classes
class CInventoryButeMgr : public CGameButeMgr
{
	public : // Public member variables

		CInventoryButeMgr();
		~CInventoryButeMgr();

        LTBOOL	Init(const char* szAttributeFile = INVENTORY_FILE);
		void	Term();

		// Templates
		uint32 GetNumItems() const { return m_lstItems.size(); }
		int GetItemIndex(const char* szItem);

		char* GetItemName(const uint32 iItem);
		char* GetItemCommand(const uint32 iItem);

	protected:

		void				ReadItem();

	private :

		GEN_INVENTORY_ITEM_DEF_LIST m_lstItems;
};

#endif // __InventoryBUTE_MGR_H__