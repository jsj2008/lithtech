// ----------------------------------------------------------------------- //
//
// MODULE  : AmmoBox.cpp
//
// PURPOSE : AmmoBox object implementation
//
// CREATED : 10/28/99
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AmmoBox.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"

#define UPDATE_DELTA	0.1f
#define UNUSED_STRING	"<none>"

#define ADD_AMMO_PROP(num) \
		ADD_STRINGPROP_FLAG(AmmoType##num##, UNUSED_STRING, PF_STATICLIST)\
		ADD_LONGINTPROP(AmmoCount##num##, 0)


BEGIN_CLASS(AmmoBox)
	ADD_STRINGPROP_FLAG(Filename, "Powerups\\Models\\AmmoBox.abc", PF_DIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Powerups\\Skins\\AmmoBox.dtx", PF_FILENAME)
	ADD_BOOLPROP_FLAG(Rotate, 0, PF_HIDDEN)
	ADD_REALPROP(RespawnTime, 30.0f)
	ADD_STRINGPROP_FLAG(PickupSound, "Powerups\\Snd\\Pickup.wav", PF_HIDDEN)
	ADD_AMMO_PROP(1)
	ADD_AMMO_PROP(2)
	ADD_AMMO_PROP(3)
	ADD_AMMO_PROP(4)
	ADD_AMMO_PROP(5)
	ADD_AMMO_PROP(6)
	ADD_AMMO_PROP(7)
	ADD_AMMO_PROP(8)
	ADD_AMMO_PROP(9)
	ADD_AMMO_PROP(10)
END_CLASS_DEFAULT_FLAGS_PLUGIN(AmmoBox, PickupItem, NULL, NULL, 0, CAmmoBoxPlugin)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::AmmoBox
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

AmmoBox::AmmoBox() : PickupItem()
{
	for (int i = 0; i < AB_MAX_TYPES; i++)
	{
		m_nAmmoId[i] = m_nOriginalAmmoId[i] = WMGR_INVALID_ID;
		m_nAmmoCount[i]	= m_nOriginalAmmoCount[i] = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 AmmoBox::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
//	ROUTINE:	AmmoBox::ReadProp
//
//	PURPOSE:	Read object properties
//
// ----------------------------------------------------------------------- //

void AmmoBox::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
		char key[40];

		sprintf(key, "AmmoType%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			if (_stricmp(genProp.m_String, UNUSED_STRING) != 0)
			{
				AMMO* pAmmo = g_pWeaponMgr->GetAmmo(genProp.m_String);
				if (pAmmo)
				{
					m_nAmmoId[i] = m_nOriginalAmmoId[i] = pAmmo->nId;
				}
			}
		}

		sprintf(key, "AmmoCount%d", i+1);
        if (g_pLTServer->GetPropGeneric(key, &genProp) == LT_OK)
		{
			m_nAmmoCount[i] = m_nOriginalAmmoCount[i] = genProp.m_Long;
		}
	}


	// Set up the appropriate pick up sound...

	FREE_HSTRING(m_hstrSoundFile);
    m_hstrSoundFile = g_pLTServer->CreateString("Powerups\\Snd\\pu_ammo.wav");
 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::ObjectTouch
//
//	PURPOSE:	Add weapon PickupItem to object
//
// ----------------------------------------------------------------------- //

void AmmoBox::ObjectTouch(HOBJECT hObject)
{
	if (!hObject) return;

	// If we hit non-player objects, just ignore them...

	if (IsPlayer(hObject))
	{
        CCharacter* pCharObj = (CCharacter*)g_pLTServer->HandleToObject(hObject);

		if (pCharObj && !pCharObj->IsDead())
		{
			int nValidIds = 0;
			for (int i=0; i < AB_MAX_TYPES; i++)
			{
				if (m_nAmmoId[i] != WMGR_INVALID_ID)
				{
					nValidIds++;
				}
			}
			if (nValidIds)
			{
                HMESSAGEWRITE hMessage = g_pLTServer->StartMessageToObject(this, hObject, MID_AMMOBOX);
                g_pLTServer->WriteToMessageByte(hMessage, nValidIds);
				for (int i=0; i < AB_MAX_TYPES; i++)
				{
					if (m_nAmmoId[i] != WMGR_INVALID_ID)
					{
                        g_pLTServer->WriteToMessageByte(hMessage, m_nAmmoId[i]);
                        g_pLTServer->WriteToMessageFloat(hMessage, (LTFLOAT)m_nAmmoCount[i]);
					}
				}
                g_pLTServer->EndMessage(hMessage);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void AmmoBox::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
        g_pLTServer->WriteToMessageByte(hWrite, m_nAmmoId[i]);
        g_pLTServer->WriteToMessageFloat(hWrite, (LTFLOAT)m_nAmmoCount[i]);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void AmmoBox::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	for (int i=0; i < AB_MAX_TYPES; i++)
	{
        m_nAmmoId[i] = g_pLTServer->ReadFromMessageByte(hRead);
        m_nAmmoCount[i] = (int)g_pLTServer->ReadFromMessageFloat(hRead);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 AmmoBox::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_AMMOBOX:
		{
			//figure out what's left
			Leftovers(hRead);
		}
		break;

		default: break;
	}

	return PickupItem::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::Leftovers
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

void AmmoBox::Leftovers(HMESSAGEREAD hRead)
{
    uint8 numAmmoTypes = g_pLTServer->ReadFromMessageByte(hRead);

    int i;
    for (i = 0; i < numAmmoTypes; i++)
	{
        m_nAmmoId[i]    = g_pLTServer->ReadFromMessageByte(hRead);
        m_nAmmoCount[i] = (int) g_pLTServer->ReadFromMessageFloat(hRead);
	}

	for (i=numAmmoTypes; i < AB_MAX_TYPES; i++)
	{
		m_nAmmoId[i] = WMGR_INVALID_ID;
		m_nAmmoCount[i] = 0;
	}

	PlayPickedupSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AmmoBox::PickedUp()
//
//	PURPOSE:	Called when an object tells this item that the object
//				picked it up.
//
// ----------------------------------------------------------------------- //

void AmmoBox::PickedUp(HMESSAGEREAD hRead)
{
	PickupItem::PickedUp(hRead);

	// Reset the contents of the ammo box if this is a multiplayer game
	// this way when the object respawns it has full ammo...

	if (IsMultiplayerGame())
	{
		for (int i = 0; i < AB_MAX_TYPES; i++)
		{
			m_nAmmoId[i]	= m_nOriginalAmmoId[i];
			m_nAmmoCount[i]	= m_nOriginalAmmoCount[i];
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAmmoBoxPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CAmmoBoxPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( CWeaponMgrPlugin::PreHook_EditStringList(szRezPath, szPropName,
		aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK )
	{
		return LT_OK;
	}


	// See if we can handle the property...

	char key[40];
	for (int index=1; index <= AB_MAX_TYPES; index++)
	{
		sprintf(key, "AmmoType%d", index);

        if (_stricmp(key, szPropName) == 0)
		{
            uint32 dwAmmoTypes = sm_ButeMgr.GetNumAmmoTypes();

			_ASSERT(cMaxStrings >= dwAmmoTypes);
			*pcStrings = 1;

			// Make sure the first string is the unused slot...

			strcpy(aszStrings[0], UNUSED_STRING);

            AMMO* pAmmo = LTNULL;

            for (uint32 i=1; i <= dwAmmoTypes; i++)
			{
				pAmmo = sm_ButeMgr.GetAmmo(i-1);

				if (pAmmo && pAmmo->szName[0] && strlen(pAmmo->szName) < cMaxStringLength)
				{
					strcpy(aszStrings[i], pAmmo->szName);
					(*pcStrings)++;
				}
				else
				{
					return LT_UNSUPPORTED;
				}
			}

			return LT_OK;
		}
	}
	return LT_UNSUPPORTED;
}
