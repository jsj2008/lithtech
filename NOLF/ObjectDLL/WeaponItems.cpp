// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponItems.cpp
//
// PURPOSE : Weapon items - Implementation
//
// CREATED : 10/7/97
//
// REVISED : 10/22/99 - jrg
//
// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "WeaponItems.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "Character.h"
#include "ObjectMsgs.h"

#define UPDATE_DELTA	0.1f

BEGIN_CLASS(WeaponItem)
	// Hidden property overrides...
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_FILENAME | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME | PF_HIDDEN)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupSound, "", PF_HIDDEN)

	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(WeaponType, "", PF_STATICLIST)
	ADD_LONGINTPROP_FLAG(WeaponTypeId, WMGR_INVALID_ID, PF_HIDDEN) // Only used by SpawnObject
	ADD_LONGINTPROP(AmmoAmount, -1)
	ADD_BOOLPROP_FLAG(IsLevelPowerup, 1, PF_HIDDEN)
END_CLASS_DEFAULT_FLAGS_PLUGIN(WeaponItem, PickupItem, NULL, NULL, 0, CWeaponItemPlugin)


extern CVarTrack g_WeaponsStay;
extern CVarTrack g_RespawnScaleTrack;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::WeaponItem
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

WeaponItem::WeaponItem() : PickupItem()
{
	m_nAmmo		= -1;
	m_nWeaponId = WMGR_INVALID_ID;
	m_nAmmoId	= WMGR_INVALID_ID;

    m_bBounce = LTFALSE;

	m_dwFlags |= FLAG_NOSLIDING;

	m_bIsLevelPowerup = LTTRUE;  // Assume weapon was placed in the level
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 WeaponItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);
            g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
			return dwRet;
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			if (fData != PRECREATE_SAVEGAME)
			{
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = PickupItem::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}

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
//	ROUTINE:	WeaponItem::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void WeaponItem::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

 	g_pWeaponMgr->ReadWeaponProp("WeaponType", m_nWeaponId, m_nAmmoId);

	// If the WeaponType is invalid, see if the WeaponTypeId property
	// was set correctly...

	if (m_nWeaponId == WMGR_INVALID_ID)
	{
		if (g_pLTServer->GetPropGeneric("WeaponTypeId", &genProp) == LT_OK)
		{
			m_nWeaponId = (uint8) genProp.m_Long;

			// Set ammo type to default (could be changed by AmmoType
			// property...
			WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
			if (pWeapon)
			{
				m_nAmmoId = pWeapon->nDefaultAmmoType;
			}
		}
	}

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (pWeapon && pWeapon->bInfiniteAmmo)
	{
		m_nAmmo = 1000;
	}
	else
	{
		if (g_pLTServer->GetPropGeneric("AmmoAmount", &genProp) == LT_OK)
		{
			m_nAmmo = genProp.m_Long;
		}
	}

	// This allows ammo type to be specified without appending it
	// to the WeaponType attribute (e.g., a spawned weapon)...

    if (g_pLTServer->GetPropGeneric("AmmoType", &genProp) == LT_OK)
	{
		AMMO* pAmmo = g_pWeaponMgr->GetAmmo(genProp.m_String);
		if (pAmmo)
		{
			m_nAmmoId = pAmmo->nId;
		}
	}

    if (g_pLTServer->GetPropGeneric("IsLevelPowerup", &genProp) == LT_OK)
	{
		m_bIsLevelPowerup = genProp.m_Bool;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::PostPropRead
//
//	PURPOSE:	Handle post property read engine messages
//
// ----------------------------------------------------------------------- //

void WeaponItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (!pWeapon) return;

	if (pStruct)
	{
		SAFE_STRCPY(pStruct->m_Filename, pWeapon->szHHModel);
		SAFE_STRCPY(pStruct->m_SkinName, pWeapon->szHHSkin);

		// Set up the appropriate pick up sound...

		FREE_HSTRING(m_hstrSoundFile);
        m_hstrSoundFile = g_pLTServer->CreateString("Powerups\\Snd\\pu_weapon.wav");

        m_bBounce = LTFALSE;
        m_bRotate = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::InitialUpdate
//
//	PURPOSE:	Handle inital update engine messages
//
// ----------------------------------------------------------------------- //

void WeaponItem::InitialUpdate()
{
	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	if (!pWeapon) return;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoId);
	if (!pAmmo) return;

    LTVector vDims, vScale, vNewDims;
    g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));

	vScale = pWeapon->vHHScale;
    g_pLTServer->ScaleObject(m_hObject, &vScale);

	// Set object dims based on scale value...

	vNewDims.x = vScale.x * vDims.x;
	vNewDims.y = vScale.y * vDims.y;
	vNewDims.z = vScale.z * vDims.z;

    g_pLTServer->SetObjectDims(m_hObject, &vNewDims);
    g_pLTServer->SetNextUpdate(m_hObject, UPDATE_DELTA);

	// Set our default ammo if necessary...

	if (m_nAmmo < 0)
	{
		m_nAmmo = pAmmo->nSpawnedAmount;
	}

	// Cache the files...

	pWeapon->Cache(g_pWeaponMgr);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::ObjectTouch
//
//	PURPOSE:	Add weapon powerup to object
//
// ----------------------------------------------------------------------- //

void WeaponItem::ObjectTouch(HOBJECT hObject)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CCharacter* pCharObj = (CCharacter*)g_pLTServer->HandleToObject(hObject);

		if (pCharObj && !pCharObj->IsDead())
		{
            HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, hObject, MID_ADDWEAPON);
            g_pLTServer->WriteToMessageByte(hMessage, m_nWeaponId);
            g_pLTServer->WriteToMessageByte(hMessage, m_nAmmoId);
            g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)m_nAmmo);
			g_pLTServer->WriteToMessageByte(hMessage, m_bIsLevelPowerup);
            g_pLTServer->EndMessage(hMessage);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WeaponItem::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageByte(hWrite, m_nWeaponId);
    g_pLTServer->WriteToMessageByte(hWrite, m_nAmmoId);
    g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT)m_nAmmo);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WeaponItem::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    m_nWeaponId = g_pLTServer->ReadFromMessageByte(hRead);
    m_nAmmoId   = g_pLTServer->ReadFromMessageByte(hRead);
    m_nAmmo     = (int) g_pLTServer->ReadFromMessageFloat(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WeaponItem::PickedUp()
//
//	PURPOSE:	Called when an object tells this weapon that the object
//				picked it up.
//
// ----------------------------------------------------------------------- //

void WeaponItem::PickedUp(HMESSAGEREAD hRead)
{
	// make the item invisible for the correct amount of time

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE & ~FLAG_TOUCH_NOTIFY);

	// Let the world know what happened...

	PlayPickedupSound();


	// Clear our player obj, we no longer need this link...

    SetPlayerObj(LTNULL);


	// if we're supposed to trigger something, trigger it here

	if (m_hstrPickupTriggerTarget && m_hstrPickupTriggerMessage)
	{
		SendTriggerMsgToObjects(this, m_hstrPickupTriggerTarget, m_hstrPickupTriggerMessage);
	}


	// get the override respawn time - if it's -1.0, use the default

    LTFLOAT fRespawn = g_pLTServer->ReadFromMessageFloat (hRead);
	if (fRespawn == -1.0f) 
	{
		fRespawn = m_fRespawnDelay;
	}

	fRespawn /= g_RespawnScaleTrack.GetFloat(1.0f);

	if (g_pGameServerShell->GetGameType() != SINGLE && g_WeaponsStay.GetFloat() > 0.0f && m_fRespawnDelay > 0.0f)
    {
		fRespawn = 0.1f;
	}

	if (fRespawn <= 0.0f || g_pGameServerShell->GetGameType() == SINGLE)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
	else
	{
        g_pLTServer->SetNextUpdate(m_hObject, fRespawn);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponItemPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
LTRESULT CWeaponItemPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	if (_strcmpi("WeaponType", szPropName) == 0)
	{
		m_WeaponMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		m_WeaponMgrPlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
