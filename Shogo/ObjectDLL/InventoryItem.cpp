// ----------------------------------------------------------------------- //
//
// MODULE  : InventoryItem.cpp
//
// PURPOSE : Inventory item implementation
//
// CREATED : 10/29/97
//
// ----------------------------------------------------------------------- //

#include "InventoryItem.h"
#include "InventoryTypes.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"

BEGIN_CLASS(InventoryItem)
	ADD_REALPROP(ItemType, 1.0f)
	ADD_REALPROP(ItemSubType, 0.0f)
	ADD_VECTORPROP_VAL_FLAG( Dims, 20.0f, 25.0f, 10.0f, PF_HIDDEN | PF_DIMS )
END_CLASS_DEFAULT(InventoryItem, PickupItem, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InventoryItem::InventoryItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

InventoryItem::InventoryItem() : PickupItem()
{
	m_hstrName = NULL;
	m_itemType = 0;
	m_itemSubType = 0;
	m_itemData = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InventoryItem::~InventoryItem()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

InventoryItem::~InventoryItem()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrName)
	{
		pServerDE->FreeString (m_hstrName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InventoryItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD InventoryItem::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	GenericProp genProp;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if( fData == 1.0f || fData == 2.0f )
			{
				if( pServerDE->GetPropGeneric( "ItemType", &genProp ) == DE_OK )
					m_itemType = (DBYTE)genProp.m_Long;

				if( pServerDE->GetPropGeneric( "ItemSubType", &genProp ) == DE_OK )
					m_itemSubType = (DBYTE)genProp.m_Long;

				if( pServerDE->GetPropGeneric( "Name", &genProp ) == DE_OK )
					if( genProp.m_String[0] )
						m_hstrName = pServerDE->CreateString( genProp.m_String );

				if( !m_hstrName )
					m_hstrName = pServerDE->CreateString( "Noname" );
			}
			
			break;
		}

		case MID_INITIALUPDATE:
		{
			if ((int)fData != INITIALUPDATE_SAVEGAME)
			{
				DVector vDims;
				VEC_SET(vDims, 20.0f, 25.0f, 10.0f);
				pServerDE->SetObjectDims(m_hObject, &vDims);
			}
			break;
		}
		
		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default: break;
	}

	return PickupItem::EngineMessageFn (messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InventoryItem::EngineMessageFn
//
//	PURPOSE:	Send specific message to object that touched us
//
// ----------------------------------------------------------------------- //

void InventoryItem::ObjectTouch (HOBJECT hObject)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	// send an inventory item touch message to the object that touched us

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(this, hObject, MID_INVENTORYITEMTOUCH);
	pServerDE->WriteToMessageByte( hMessage, m_itemType );
	pServerDE->WriteToMessageByte( hMessage, m_itemSubType );
	pServerDE->WriteToMessageHString( hMessage, m_hstrName );
	pServerDE->WriteToMessageFloat( hMessage, m_itemData );
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InventoryItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void InventoryItem::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageHString(hWrite, m_hstrName);
	pServerDE->WriteToMessageByte(hWrite, m_itemType);
	pServerDE->WriteToMessageByte(hWrite, m_itemSubType);
	pServerDE->WriteToMessageFloat(hWrite, m_itemData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InventoryItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void InventoryItem::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_hstrName		= pServerDE->ReadFromMessageHString(hRead);
	m_itemType		= pServerDE->ReadFromMessageByte(hRead);
	m_itemSubType	= pServerDE->ReadFromMessageByte(hRead);
	m_itemData		= pServerDE->ReadFromMessageFloat(hRead);
}