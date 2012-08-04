// ----------------------------------------------------------------------- //
//
// MODULE  : InventoryItem.h
//
// PURPOSE : Inventory item class declaration
//
// CREATED : 10/29/97
//
// ----------------------------------------------------------------------- //

#ifndef __INVENTORYITEM_H
#define __INVENTORYITEM_H

#include "PickupItem.h"

class InventoryItem : public PickupItem
{
	public:

		InventoryItem();
		~InventoryItem();

	protected:

		DDWORD			EngineMessageFn (DDWORD messageID, void *pData, DFLOAT fData);
		virtual void	ObjectTouch (HOBJECT hObject);

	protected:

		HSTRING		m_hstrName;
		DBYTE		m_itemType;
		DBYTE		m_itemSubType;
		DFLOAT		m_itemData;

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

};

#endif
