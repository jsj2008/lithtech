// ----------------------------------------------------------------------- //
//
// MODULE  : GearItems.cpp
//
// PURPOSE : Gear items - Implementation
//
// CREATED : 10/22/99
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GearItems.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(GearItem)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_FILENAME | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupSound, "", PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Dims, 10.0f, 25.0f, 10.0f, PF_HIDDEN | PF_DIMS)

	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(GearType, "", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(GearItem, PickupItem, NULL, NULL, 0, CGearPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::GearItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

GearItem::GearItem() : PickupItem()
{
	m_nGearId = -1;
    m_bBounce = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 GearItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return PickupItem::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void GearItem::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("GearType", &genProp) == LT_OK)
	{
		GEAR* pGear = g_pWeaponMgr->GetGear(genProp.m_String);
		if (pGear)
		{
			m_nGearId = pGear->nId;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::PostPropRead
//
//	PURPOSE:	Handle post property read engine messages
//
// ----------------------------------------------------------------------- //

void GearItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	GEAR* pGear = g_pWeaponMgr->GetGear(m_nGearId);
	if (!pGear) return;

	if (pStruct)
	{
		SAFE_STRCPY(pStruct->m_Filename, pGear->szModel);
		SAFE_STRCPY(pStruct->m_SkinName, pGear->szSkin);

		// Set up the appropriate sounds...

		if (pGear->szPickUpSound[0])
		{
			FREE_HSTRING(m_hstrSoundFile);
			m_hstrSoundFile = g_pLTServer->CreateString(pGear->szPickUpSound);
		}

		if (pGear->szRespawnSound[0])
		{
			FREE_HSTRING(m_hstrRespawnSoundFile);
			m_hstrRespawnSoundFile = g_pLTServer->CreateString(pGear->szRespawnSound);
		}

        m_bRotate = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::ObjectTouch
//
//	PURPOSE:	Add weapon PickupItem to object
//
// ----------------------------------------------------------------------- //

void GearItem::ObjectTouch(HOBJECT hObject)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);

		if (pPlayer && !pPlayer->IsDead())
		{
            HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, hObject, MID_ADDGEAR);
            g_pLTServer->WriteToMessageByte(hMessage, m_nGearId);
            g_pLTServer->EndMessage(hMessage);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void GearItem::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite, m_nGearId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void GearItem::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_nGearId = g_pLTServer->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GearItem::PickedUp
//
//	PURPOSE:	Picked up
//
// ----------------------------------------------------------------------- //

void GearItem::PickedUp(HMESSAGEREAD hRead)
{
	// If we were touched by a player, our m_hPlayerObj data member will be
	// set.  Send a message to that player's client letting it know that an
	// item has been picked up...

	if (m_hPlayerObj)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hPlayerObj);
		if (pPlayer && !pPlayer->IsDead())
		{
			HCLIENT hClient = pPlayer->GetClient();
			if (hClient)
			{
                HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(hClient, MID_GEAR_PICKEDUP);
                g_pLTServer->WriteToMessageByte(hWrite, m_nGearId);
                g_pLTServer->EndMessage(hWrite);
			}
		}
	}

	PickupItem::PickedUp(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGearPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
LTRESULT CGearPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( CWeaponMgrPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK )
	{
		return LT_OK;
	}

	// See if we can handle the property...

	if (_strcmpi("GearType", szPropName) == 0)
	{
        uint32 dwGearTypes = sm_ButeMgr.GetNumGearTypes();

		_ASSERT(cMaxStrings >= dwGearTypes);

        GEAR* pGear = LTNULL;

        for (uint32 i=0; i < dwGearTypes; i++)
		{
			pGear = sm_ButeMgr.GetGear(i);

			if (pGear && pGear->szName[0] && strlen(pGear->szName) < cMaxStringLength)
			{
				strcpy(aszStrings[i], pGear->szName);
				(*pcStrings)++;
			}
			else
			{
				return LT_UNSUPPORTED;
			}
		}

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
