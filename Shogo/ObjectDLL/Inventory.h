//----------------------------------------------------------
//
// MODULE  : Inventory.h
//
// PURPOSE : Inventory Aggregate
//
// CREATED : 10/29/97
//
//----------------------------------------------------------

#ifndef __INVENTORY_H
#define __INVENTORY_H

#include "cpp_aggregate_de.h"
#include "dynarray.h"

struct ITEM
{
	ITEM()	{ itemType = 0; itemSubType = 0; itemName = NULL; itemData = 0; }

	DBYTE	itemType;
	DBYTE	itemSubType;
	HSTRING	itemName;
	DFLOAT	itemData;

	void Save(HMESSAGEWRITE hWrite, DBYTE nType);
	void Load(HMESSAGEREAD hRead, DBYTE nType);

};

inline void ITEM::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{
	if (!g_pServerDE || !hWrite) return;

	g_pServerDE->WriteToMessageByte(hWrite, itemType);
	g_pServerDE->WriteToMessageByte(hWrite, itemSubType);
	g_pServerDE->WriteToMessageHString(hWrite, itemName);
	g_pServerDE->WriteToMessageFloat(hWrite, itemData);

}

inline void ITEM::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	if (!g_pServerDE || !hRead) return;

	itemType	= g_pServerDE->ReadFromMessageByte(hRead);
	itemSubType = g_pServerDE->ReadFromMessageByte(hRead);
	itemName	= g_pServerDE->ReadFromMessageHString(hRead);
	itemData	= g_pServerDE->ReadFromMessageFloat(hRead);
}


class Inventory : public Aggregate
{
	public:

		Inventory();
		~Inventory();

		DBOOL				HaveItem (DBYTE itemType, DBYTE itemSubType = 0);						// the HaveItem() functions could just call the GetNumItems() functions and cast the return to 
		DBOOL				HaveItem (HSTRING itemName, DBYTE itemType, DBYTE itemSubType = 0);		// a DBOOL, but these will be slightly faster depending on the number of items in the inventory
		DDWORD				GetNumItems (DBYTE itemType, DBYTE itemSubType = 0);
		DDWORD				GetNumItems (HSTRING itemName, DBYTE itemType, DBYTE itemSubType = 0);
		void				RemoveItem (DBYTE itemType, DBYTE itemSubType = 0, DDWORD nCount = 1);
		void				RemoveItem (HSTRING itemName, DBYTE itemType, DBYTE itemSubType = 0, DDWORD nCount = 1);
		DBOOL				CanHaveMultiple (DBYTE itemType, DBYTE itemSubType);

		DFLOAT				GetItemData (DBYTE itemType, DBYTE itemSubType = 0, DDWORD nItem = 0);
	
	protected:

		virtual DDWORD		EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData);
		virtual DDWORD		ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	protected:

		CDynArray<ITEM>		m_pItems;
		DDWORD				m_nItems;

		HOBJECT				m_hObject;		// object that we belong to

	private:

		void Save(HMESSAGEWRITE hWrite, DBYTE nType);
		void Load(HMESSAGEREAD hRead, DBYTE nType);
};

#endif
