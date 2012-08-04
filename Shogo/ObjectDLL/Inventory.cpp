//----------------------------------------------------------
//
// MODULE  : Inventory.cpp
//
// PURPOSE : Inventory Aggregate
//
// CREATED : 10/29/97
//
//----------------------------------------------------------

#include "Inventory.h"
#include "cpp_server_de.h"
#include "cpp_engineobjects_de.h"
#include "RiotMsgIds.h"
#include "InventoryTypes.h"
#include "RiotObjectUtilities.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::Inventory()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Inventory::Inventory() : Aggregate()
{
	m_pItems.SetMemCopyable (1);
	m_pItems.SetGrowBy (3);
	m_nItems = 0;
	m_hObject = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::~Inventory()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Inventory::~Inventory()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;
	
	for (DDWORD i = 0; i < m_nItems; i++)
	{
		if (m_pItems[i].itemName)
		{
			pServerDE->FreeString (m_pItems[i].itemName);
		}
	}

	m_pItems.Flush();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::HaveItem
//
//	PURPOSE:	returns DTRUE if the the specified item is in 
//				the inventory
//
// ----------------------------------------------------------------------- //

DBOOL Inventory::HaveItem (DBYTE itemType, DBYTE itemSubType)
{
	for (DDWORD i = 0; i < m_nItems; i++)
	{
		if (m_pItems[i].itemType == itemType && (m_pItems[i].itemSubType == itemSubType || itemSubType == 0))
		{
			return DTRUE;
		}
	}
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::HaveItem
//
//	PURPOSE:	returns DTRUE if the the specified item is in 
//				the inventory
//
// ----------------------------------------------------------------------- //

DBOOL Inventory::HaveItem (HSTRING itemName, DBYTE itemType, DBYTE itemSubType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	for (DDWORD i = 0; i < m_nItems; i++)
	{
		if (pServerDE->CompareStrings (itemName, m_pItems[i].itemName) && m_pItems[i].itemType == itemType && (m_pItems[i].itemSubType == itemSubType || itemSubType == 0))
		{
			return DTRUE;
		}
	}
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::GetNumItems
//
//	PURPOSE:	returns the number of specified items in the inventory
//
// ----------------------------------------------------------------------- //

DDWORD Inventory::GetNumItems (DBYTE itemType, DBYTE itemSubType)
{
	DDWORD nCount = 0;
	for (DDWORD i = 0; i < m_nItems; i++)
	{
		if (m_pItems[i].itemType == itemType && (m_pItems[i].itemSubType == itemSubType || itemSubType == 0))
		{
			nCount++;
		}
	}
	return nCount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::GetNumItems
//
//	PURPOSE:	returns the number of specified items in the inventory
//
// ----------------------------------------------------------------------- //

DDWORD Inventory::GetNumItems (HSTRING itemName, DBYTE itemType, DBYTE itemSubType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return 0;

	DDWORD nCount = 0;
	for (DDWORD i = 0; i < m_nItems; i++)
	{
		if (pServerDE->CompareStrings (itemName, m_pItems[i].itemName) && m_pItems[i].itemType == itemType && (m_pItems[i].itemSubType == itemSubType || itemSubType == 0))
		{
			nCount++;
		}
	}
	return nCount;
}
		
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::RemoveItem
//
//	PURPOSE:	Removes the specified item from the inventory
//
// ----------------------------------------------------------------------- //

void Inventory::RemoveItem (DBYTE itemType, DBYTE itemSubType, DDWORD nCount)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	for (DDWORD i = 0; i < m_nItems; i++)
	{
		if (m_pItems[i].itemType == itemType && (m_pItems[i].itemSubType == itemSubType || itemSubType == 0))
		{
			m_pItems[i].itemType = 0;
			m_pItems[i].itemSubType = 0;
			if (m_pItems[i].itemName)
			{
				pServerDE->FreeString (m_pItems[i].itemName);
				m_pItems[i].itemName = DNULL;
			}

			nCount--;
			if (nCount == 0) break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::RemoveItem
//
//	PURPOSE:	Removes the specified item from the inventory
//
// ----------------------------------------------------------------------- //

void Inventory::RemoveItem (HSTRING itemName, DBYTE itemType, DBYTE itemSubType, DDWORD nCount)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	for (DDWORD i = 0; i < m_nItems; i++)
	{
		if (pServerDE->CompareStrings (itemName, m_pItems[i].itemName) && m_pItems[i].itemType == itemType && (m_pItems[i].itemSubType == itemSubType || itemSubType == 0))
		{
			m_pItems[i].itemType = 0;
			m_pItems[i].itemSubType = 0;
			if (m_pItems[i].itemName)
			{
				pServerDE->FreeString (m_pItems[i].itemName);
				m_pItems[i].itemName = DNULL;
			}

			nCount--;
			if (nCount == 0) break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::CanHaveMultiple
//
//	PURPOSE:	Returns TRUE if we can have multiple items of this type
//
// ----------------------------------------------------------------------- //

DBOOL Inventory::CanHaveMultiple (DBYTE itemType, DBYTE itemSubType)
{
	switch (itemType)
	{
		case IT_KEY:			return DTRUE;
		case IT_UPGRADE:		return DFALSE;
		case IT_ENHANCEMENT:	return DTRUE;
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::GetItemData
//
//	PURPOSE:	Returns user data associated with given item
//
// ----------------------------------------------------------------------- //

DFLOAT Inventory::GetItemData (DBYTE itemType, DBYTE itemSubType, DDWORD nItem)
{
	DDWORD nCount = 0;
	for (DDWORD i = 0; i < m_nItems; i++)
	{
		if (m_pItems[i].itemType == itemType && (m_pItems[i].itemSubType == itemSubType || itemSubType == 0))
		{
			if (nCount == nItem)
			{
				return m_pItems[i].itemData;
			}
			nCount++;
		}
	}

	return 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //
		
DDWORD Inventory::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			if (!pObject || !pObject->m_hObject) break;
			if (!m_hObject) m_hObject = pObject->m_hObject;
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DBYTE)lData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DBYTE)lData);
		}
		break;
	}

	return Aggregate::EngineMessageFn(pObject, messageID, pData, lData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::ObjectMessageFn()
//
//	PURPOSE:	Handle inventory messages
//
// ----------------------------------------------------------------------- //

DDWORD Inventory::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return 0;
	
	switch (messageID)
	{
		case MID_INVENTORYITEMTOUCH:
		{
			// if we're not a player object, don't pick this up...

			if (!IsPlayer (m_hObject))
			{
				break;
			}

			DBYTE itemType = pServerDE->ReadFromMessageByte (hRead);
			DBYTE itemSubType = pServerDE->ReadFromMessageByte (hRead);
			HSTRING itemName = pServerDE->ReadFromMessageHString (hRead);
			DFLOAT itemData = pServerDE->ReadFromMessageFloat (hRead);

			// see if we can have multiple items of this type and if so, if we already have an item of this type

			DBOOL bHaveIt = DFALSE;
			if (!CanHaveMultiple (itemType, itemSubType))
			{
				for (DDWORD i = 0; i < m_nItems; i++)
				{
					if (m_pItems[i].itemType == itemType && pServerDE->CompareStrings (m_pItems[i].itemName, itemName))
					{
						pServerDE->FreeString (itemName);
						bHaveIt = DTRUE;
						break;
					}
				}
			}

			// if we don't already have this item, pick it up

			if (!bHaveIt)
			{
				m_pItems[m_nItems].itemType = itemType;
				m_pItems[m_nItems].itemSubType = itemSubType;
				m_pItems[m_nItems].itemName = itemName;
				m_pItems[m_nItems].itemData = itemData;
				m_nItems++;

				HMESSAGEWRITE hWrite = pServerDE->StartMessageToObject (pObject, hSender, MID_PICKEDUP);
				pServerDE->WriteToMessageFloat (hWrite, -1.0f);
				pServerDE->EndMessage (hWrite);
			}

			break;
		}

		case MID_INVENTORYITEMQUERY:
		{
			// get info on the item this object needs
			// if itemSubType == 0, this will match ALL subtypes.  Subtypes should never be numbered zero for this reason.

			DBYTE itemType = pServerDE->ReadFromMessageByte (hRead);
			DBYTE itemSubType = pServerDE->ReadFromMessageByte (hRead);
			HSTRING itemName = pServerDE->ReadFromMessageHString (hRead);

			// see if we have this item

			DBOOL bHaveItem = DFALSE;
			for (DDWORD i = 0; i < m_nItems; i++)
			{
				char* pStr1 = pServerDE->GetStringData (m_pItems[i].itemName);
				char* pStr2 = pServerDE->GetStringData (itemName);
				if (m_pItems[i].itemType == itemType && (itemSubType == 0 || m_pItems[i].itemSubType == itemSubType) && pServerDE->CompareStrings (m_pItems[i].itemName, itemName))
				{
					bHaveItem = DTRUE;
					break;
				}
			}

			// send a response, saying if we have the item or not

			HMESSAGEWRITE hWrite = pServerDE->StartMessageToObject (pObject, hSender, MID_INVENTORYQUERYRESPONSE);
			pServerDE->WriteToMessageByte (hWrite, itemType);
			pServerDE->WriteToMessageByte (hWrite, itemSubType);
			pServerDE->WriteToMessageHString (hWrite, itemName);
			pServerDE->WriteToMessageByte (hWrite, bHaveItem ? 1 : 0);
			pServerDE->EndMessage (hWrite);

			pServerDE->FreeString(itemName);

			break;
		}

		default : break;
	}

	return Aggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Inventory::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageDWord(hWrite, m_nItems);
			
	for (DDWORD i = 0; i < m_nItems; i++)
	{
		m_pItems[i].Save(hWrite, nType);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Inventory::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Inventory::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	m_nItems = pServerDE->ReadFromMessageDWord(hRead);
	
	for (DDWORD i = 0; i < m_nItems; i++)
	{
		m_pItems[i].Load(hRead, nType);
	}
}
