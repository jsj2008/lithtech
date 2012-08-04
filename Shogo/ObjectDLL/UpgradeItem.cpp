// ----------------------------------------------------------------------- //
//
// MODULE  : UpgradeItem.cpp
//
// PURPOSE : Baseclass implementation for upgrade items
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#include "UpgradeItem.h"
#include "InventoryTypes.h"
#include "RiotMsgIds.h"
#include "cpp_server_de.h"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		UpgradeItem
//
//	PURPOSE:	Baseclass for upgrade items
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(UpgradeItem)
	ADD_BOOLPROP_FLAG(Rotate, 1, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(SoundFile, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(RespawnTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ItemType, 1.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ItemSubType, 0.0f, PF_HIDDEN)
END_CLASS_DEFAULT(UpgradeItem, InventoryItem, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UpgradeItem::UpgradeItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

UpgradeItem::UpgradeItem() : InventoryItem()
{
	m_nModelName = 0;
	m_nModelSkin = 0;
	m_nSoundName = 0;
	m_nUpgradeSubType = 0;
	m_fCreationTime = 0.0f;
	m_hDroppedBy = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UpgradeItem::~UpgradeItem()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

UpgradeItem::~UpgradeItem()
{
	if( !g_pServerDE ) return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UpgradeItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD UpgradeItem::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	DDWORD dwRet;

	if( g_pServerDE )
	{
		switch(messageID)
		{
			case MID_PRECREATE:
			{
				dwRet = InventoryItem::EngineMessageFn(messageID, pData, fData);
				PostPropRead((ObjectCreateStruct*)pData);
				return dwRet;
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

			default : break;
		}
	}

	return InventoryItem::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UpgradeItem::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void UpgradeItem::PostPropRead(ObjectCreateStruct *pStruct)
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
		SAFE_STRCPY(pStruct->m_Filename, "dummy string");	// this will force the dummy model
	}
	g_pServerDE->FreeString (hString);

	hString = g_pServerDE->FormatString (m_nModelSkin);
	if (hString) SAFE_STRCPY(pStruct->m_SkinName, g_pServerDE->GetStringData (hString));
	g_pServerDE->FreeString (hString);

	m_hstrSoundFile = g_pServerDE->FormatString (m_nSoundName);

	m_itemType = (DBYTE) IT_UPGRADE;
	m_itemSubType = (DBYTE) m_nUpgradeSubType;
	m_fRespawnDelay = 0.0f;

	m_bRotate = DFALSE;

	// get the handle of the player that dropped us, if any

	m_hDroppedBy = (HOBJECT) pStruct->m_UserData;

	// get the current time

	m_fCreationTime = g_pServerDE->GetTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UpgradeItem::ObjectTouch
//
//	PURPOSE:	Handle touch notifications
//
// ----------------------------------------------------------------------- //

void UpgradeItem::ObjectTouch (HOBJECT hObject)
{
	// if we were recently created, and we are touched by the same player
	// that dropped us, don't let that player pick us right back up again
	// (at least yet)

	if (hObject == m_hDroppedBy && g_pServerDE->GetTime() < m_fCreationTime + 2.0f)
	{
		return;
	}

	InventoryItem::ObjectTouch (hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UpgradeItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void UpgradeItem::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hDroppedBy);
	pServerDE->WriteToMessageDWord(hWrite, m_nModelName);
	pServerDE->WriteToMessageDWord(hWrite, m_nModelSkin);
	pServerDE->WriteToMessageDWord(hWrite, m_nSoundName);
	pServerDE->WriteToMessageDWord(hWrite, m_nUpgradeSubType);
	pServerDE->WriteToMessageFloat(hWrite, m_fCreationTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	UpgradeItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void UpgradeItem::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hDroppedBy);
	m_nModelName		= pServerDE->ReadFromMessageDWord(hRead);
	m_nModelSkin		= pServerDE->ReadFromMessageDWord(hRead);
	m_nSoundName		= pServerDE->ReadFromMessageDWord(hRead);
	m_nUpgradeSubType	= pServerDE->ReadFromMessageDWord(hRead);
	m_fCreationTime		= pServerDE->ReadFromMessageFloat(hRead);
}
