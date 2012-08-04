// ----------------------------------------------------------------------- //
//
// MODULE  : EnhancementItem.cpp
//
// PURPOSE : Baseclass implementation for enhancement items
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#include "EnhancementItem.h"
#include "InventoryTypes.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		EnhancementItem
//
//	PURPOSE:	Baseclass for enhancement items
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(EnhancementItem)
	ADD_BOOLPROP_FLAG(Rotate, 1, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(SoundFile, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(RespawnTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ItemType, 1.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ItemSubType, 0.0f, PF_HIDDEN)
END_CLASS_DEFAULT(EnhancementItem, InventoryItem, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnhancementItem::EnhancementItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

EnhancementItem::EnhancementItem() : InventoryItem()
{
	m_nModelName = 0;
	m_nModelSkin = 0;
	m_nSoundName = 0;
	m_nEnhancementSubType = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnhancementItem::~EnhancementItem()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

EnhancementItem::~EnhancementItem()
{
	if( !g_pServerDE ) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnhancementItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD EnhancementItem::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = InventoryItem::EngineMessageFn(messageID, pData, fData);

			PostPropRead((ObjectCreateStruct*)pData);
			
			return dwRet;
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

		default : break;
	}

	return InventoryItem::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnhancementItem::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void EnhancementItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!g_pServerDE) return;

	// set the model name, skin name, and sound file name

	HSTRING hString = DNULL;
	
	hString = g_pServerDE->FormatString (m_nModelName);
	if (hString)
	{
		SAFE_STRCPY(pStruct->m_Filename, g_pServerDE->GetStringData (hString));
	}
	else
	{
		strcpy(pStruct->m_Filename, "dummy string");	// this will force the dummy model
	}
	g_pServerDE->FreeString (hString);

	hString = g_pServerDE->FormatString (m_nModelSkin);
	if (hString) SAFE_STRCPY(pStruct->m_SkinName, g_pServerDE->GetStringData (hString));
	g_pServerDE->FreeString (hString);

	m_hstrSoundFile = g_pServerDE->FormatString (m_nSoundName);

	m_itemType = (DBYTE) IT_ENHANCEMENT;
	m_itemSubType = (DBYTE) m_nEnhancementSubType;
	m_fRespawnDelay = 0.0f;
	
	m_bRotate = DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnhancementItem::ObjectTouch
//
//	PURPOSE:	Handle touch notifications
//
// ----------------------------------------------------------------------- //

void EnhancementItem::ObjectTouch (HOBJECT hObject)
{
	InventoryItem::ObjectTouch (hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnhancementItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void EnhancementItem::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageDWord(hWrite, m_nEnhancementSubType);
	pServerDE->WriteToMessageDWord(hWrite, m_nModelName);
	pServerDE->WriteToMessageDWord(hWrite, m_nModelSkin);
	pServerDE->WriteToMessageDWord(hWrite, m_nSoundName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EnhancementItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void EnhancementItem::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_nEnhancementSubType	= pServerDE->ReadFromMessageDWord(hRead);
	m_nModelName			= pServerDE->ReadFromMessageDWord(hRead);
	m_nModelSkin			= pServerDE->ReadFromMessageDWord(hRead);
	m_nSoundName			= pServerDE->ReadFromMessageDWord(hRead);
}