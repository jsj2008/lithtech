// ----------------------------------------------------------------------- //
//
// MODULE  : ModItem.cpp
//
// PURPOSE : Mod items - Implementation
//
// CREATED : 7/21/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ModItem.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"

BEGIN_CLASS(ModItem)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_FILENAME | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupSound, "", PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(Dims, 10.0f, 25.0f, 10.0f, PF_HIDDEN | PF_DIMS)
	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(ModType, "", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(ModItem, PickupItem, NULL, NULL, 0, CModPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::ModItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ModItem::ModItem() : PickupItem()
{
	m_nModId	= -1;
    m_bBounce	= LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ModItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
//	ROUTINE:	ModItem::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void ModItem::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("ModType", &genProp) == LT_OK)
	{
		MOD* pMod = g_pWeaponMgr->GetMod(genProp.m_String);
		if (pMod)
		{
			m_nModId = pMod->nId;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::PostPropRead
//
//	PURPOSE:	Handle post property read engine messages
//
// ----------------------------------------------------------------------- //

void ModItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	MOD* pMod = g_pWeaponMgr->GetMod(m_nModId);
	if (!pMod) return;

	if (pStruct)
	{
		SAFE_STRCPY(pStruct->m_Filename, pMod->szPowerupModel);
		SAFE_STRCPY(pStruct->m_SkinName, pMod->szPowerupSkin);

		// Set up the appropriate sounds...

		if (pMod->szPickUpSound[0])
		{
			FREE_HSTRING(m_hstrSoundFile);
			m_hstrSoundFile = g_pLTServer->CreateString(pMod->szPickUpSound);
		}

		if (pMod->szRespawnSound[0])
		{
			FREE_HSTRING(m_hstrSoundFile);
			m_hstrSoundFile = g_pLTServer->CreateString(pMod->szRespawnSound);
		}

		m_vScale.Init(pMod->fPowerupScale, pMod->fPowerupScale, pMod->fPowerupScale);

        m_bRotate = LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::ObjectTouch
//
//	PURPOSE:	Add mod to object
//
// ----------------------------------------------------------------------- //

void ModItem::ObjectTouch(HOBJECT hObject)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObject);

		if (pPlayer && !pPlayer->IsDead())
		{
            HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, hObject, MID_ADDMOD);
            g_pLTServer->WriteToMessageByte(hMessage, m_nModId);
            g_pLTServer->EndMessage(hMessage);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ModItem::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite, m_nModId);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ModItem::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_nModId = g_pLTServer->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ModItem::PickedUp
//
//	PURPOSE:	Picked up
//
// ----------------------------------------------------------------------- //

void ModItem::PickedUp(HMESSAGEREAD hRead)
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
                HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_PLAYER_INFOCHANGE);
                g_pLTServer->WriteToMessageByte(hMessage, IC_MOD_PICKUP_ID);
                g_pLTServer->WriteToMessageByte(hMessage, 0);
                g_pLTServer->WriteToMessageByte(hMessage, m_nModId);
                g_pLTServer->WriteToMessageFloat(hMessage, 0.0f);
                g_pLTServer->EndMessage(hMessage);
			}
		}
	}

	PickupItem::PickedUp(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CModPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
LTRESULT CModPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( CWeaponMgrPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK )
	{
		return LT_OK;
	}

	// See if we can handle the property...

	if (_strcmpi("ModType", szPropName) == 0)
	{
        uint32 dwModTypes = sm_ButeMgr.GetNumModTypes();

		_ASSERT(cMaxStrings >= dwModTypes);

        MOD* pMod = LTNULL;

        for (uint32 i=0; i < dwModTypes; i++)
		{
			pMod = sm_ButeMgr.GetMod(i);

			if (pMod && pMod->szName[0] && strlen(pMod->szName) < cMaxStringLength)
			{
				strcpy(aszStrings[i], pMod->szName);
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
