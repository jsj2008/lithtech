// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.cpp
//
// PURPOSE : Item that any player can walk across and potentially pick up -
//			 Implementation
//
// CREATED : 10/27/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PickupItem.h"
#include "MsgIds.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "CVarTrack.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "CharacterHitBox.h"

extern CGameServerShell* g_pGameServerShell;

CVarTrack g_RespawnScaleTrack;

#define DEFAULT_PICKUP_SOUND	"Powerups\\Snd\\Pickup.wav"
#define DEFAULT_RESPAWN_SOUND	"Powerups\\Snd\\Respawn.wav"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		PickupItem
//
//	PURPOSE:	Any in-game object that the player can pick up
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(PickupItem)
	ADD_STRINGPROP_FLAG(Filename, "", PF_DIMS | PF_FILENAME | PF_LOCALDIMS)
	ADD_STRINGPROP_FLAG(Skin, "", PF_FILENAME)
	ADD_VECTORPROP_VAL(Scale, 1.0f, 1.0f, 1.0f)
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SHADOW_FLAG(1, 0)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PickupTriggerTarget, "", PF_OBJECTLINK)
	ADD_STRINGPROP(PickupTriggerMessage, "")
	ADD_BOOLPROP(Rotate, 0)
	ADD_BOOLPROP(Bounce, 0)
	ADD_REALPROP(RespawnTime, 10.0f)
	ADD_STRINGPROP_FLAG(RespawnSound, DEFAULT_RESPAWN_SOUND, PF_FILENAME)
	ADD_STRINGPROP_FLAG(PickupSound, DEFAULT_PICKUP_SOUND, PF_FILENAME)
    ADD_LONGINTPROP(PlayerTeamFilter, 0)
    ADD_BOOLPROP(MoveToFloor, LTTRUE)

END_CLASS_DEFAULT_FLAGS(PickupItem, GameBase, NULL, NULL, CF_HIDDEN)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PickupItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PickupItem::PickupItem() : GameBase(OT_MODEL)
{
	m_fRespawnDelay = 10.0f;
    m_bRotate       = LTFALSE;
    m_bBounce       = LTFALSE;
	m_dwFlags		= 0;
    m_bMoveToFloor  = LTFALSE;  // Dynamically created powerups shouldn't do this by default

	m_dwUserFlags	= USRFLG_GLOW;  // Pick up items glow

    m_hstrPickupTriggerTarget   = LTNULL;
    m_hstrPickupTriggerMessage  = LTNULL;
    m_hstrSoundFile             = LTNULL;
    m_hstrRespawnSoundFile      = LTNULL;

    m_hPlayerObj = LTNULL;
    m_nPlayerTeamFilter = 0;
	m_bFirstUpdate = LTFALSE;

	m_vScale.Init(1.0f, 1.0f, 1.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::~PickupItem()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
PickupItem::~PickupItem()
{
	FREE_HSTRING(m_hstrPickupTriggerTarget);
	FREE_HSTRING(m_hstrPickupTriggerMessage);
	FREE_HSTRING(m_hstrSoundFile);
	FREE_HSTRING(m_hstrRespawnSoundFile);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PickupItem::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (m_bFirstUpdate)
			{
				m_bFirstUpdate = LTFALSE;

				if (m_bMoveToFloor)
				{
					MoveObjectToFloor(m_hObject);
				}

				g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
			}
			else
			{
				Update();
			}
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HOBJECT hObj = (HOBJECT)pData;

			// If this is a character hit box, use its object...

			CCharacterHitBox* pHitBox = LTNULL;
			if (IsCharacterHitBox(hObj))
			{
				pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
				if (pHitBox)
				{
					hObj = pHitBox->GetModelObject();
				}
			}


			// If the object is dead, it can't pick up stuff...

			if (IsPlayer(hObj))
			{
                CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObj);

				if (!pPlayer || pPlayer->IsDead()) break;
				if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && m_nPlayerTeamFilter)
				{
					if (pPlayer->GetTeamID() != m_nPlayerTeamFilter)
						break;
				}


				if (pPlayer)
				{
					SetPlayerObj(hObj);
				}
			}
			else
			{
                SetPlayerObj(LTNULL);
				break;
			}

			ObjectTouch(hObj);
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
			else if (fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				Setup((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();
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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 PickupItem::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_PICKEDUP:
		{
			PickedUp(hRead);
		}
		break;

		default: break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL PickupItem::ReadProp(ObjectCreateStruct *pInfo)
{
	GenericProp genProp;

    if (!pInfo) return LTFALSE;

    if (g_pLTServer->GetPropGeneric("PickupTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPickupTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PickupTriggerMessage", &genProp) == LT_OK)
	{
		if(genProp.m_String[0])
		{
            m_hstrPickupTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PickupSound", &genProp)  == LT_OK)
	{
		if(genProp.m_String[0])
		{
            m_hstrSoundFile = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("RespawnSound", &genProp) == LT_OK)
	{
		if(genProp.m_String[0])
		{
            m_hstrRespawnSoundFile = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("Rotate", &genProp) == LT_OK)
	{
		m_bRotate = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Bounce", &genProp) == LT_OK)
	{
		m_bBounce = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("MoveToFloor", &genProp) == LT_OK)
	{
		 m_bMoveToFloor = genProp.m_Bool;
	}

	if (g_pLTServer->GetPropGeneric("RespawnTime", &genProp) == LT_OK)
	{
		m_fRespawnDelay = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("PlayerTeamFilter", &genProp) == LT_OK)
	{
		m_nPlayerTeamFilter = (uint8) genProp.m_Long;
	}

    if (g_pLTServer->GetPropGeneric("Scale", &genProp) == LT_OK)
	{
		 m_vScale = genProp.m_Vec;
	}

   return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Setup
//
//	PURPOSE:	Set property values
//
// ----------------------------------------------------------------------- //

LTBOOL PickupItem::Setup(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	// Only spawn once...

	m_fRespawnDelay = 0.0f;

	// Show yourself!

	pInfo->m_Flags |= FLAG_VISIBLE | FLAG_TOUCH_NOTIFY;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void PickupItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY;

	m_dwFlags |= pStruct->m_Flags;

	// Make sure we have a pickedup sound...

	if (!m_hstrSoundFile)
	{
		m_hstrSoundFile = g_pLTServer->CreateString(DEFAULT_PICKUP_SOUND);
	}

	if (!m_hstrRespawnSoundFile)
	{
		m_hstrRespawnSoundFile = g_pLTServer->CreateString(DEFAULT_RESPAWN_SOUND);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL PickupItem::InitialUpdate()
{
	// Do one update to handle MoveToFloor...
	
	m_bFirstUpdate = LTTRUE;
	g_pLTServer->SetNextUpdate(m_hObject, 0.001f);

	if (!g_RespawnScaleTrack.IsInitted())
	{
        g_RespawnScaleTrack.Init(GetServerDE(), "RespawnScale", LTNULL, 1.0f);
	}


	// Set up our user flags...

    uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwUserFlags |= USRFLG_NIGHT_INFRARED;

	// If item rotates or bounces (or is multiplayer and thus respawns), set
	// a special fx message to do these things on the client...

	if (m_bRotate || m_bBounce || g_pGameServerShell->GetGameType() != SINGLE)
	{
        HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
        g_pLTServer->WriteToMessageByte(hMessage, SFX_PICKUPITEM_ID);
        g_pLTServer->WriteToMessageByte(hMessage, m_bRotate);
        g_pLTServer->WriteToMessageByte(hMessage, m_bBounce);
        g_pLTServer->EndMessage(hMessage);
	}

    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags | m_dwUserFlags);

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | m_dwFlags);


	// Set the dims based on the current animation...

    LTVector vDims;
    g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));

	// Set object dims based on scale value...

    LTVector vNewDims;
	vNewDims.x = m_vScale.x * vDims.x;
	vNewDims.y = m_vScale.y * vDims.y;
	vNewDims.z = m_vScale.z * vDims.z;

    g_pLTServer->ScaleObject(m_hObject, &m_vScale);
    g_pLTServer->SetObjectDims(m_hObject, &vNewDims);

	uint32 dwAni = g_pLTServer->GetAnimIndex(m_hObject, "World");
	if (dwAni != INVALID_ANI)
	{
		g_pLTServer->SetModelAnimation(m_hObject, dwAni);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL PickupItem::Update()
{
    g_pLTServer->SetNextUpdate(m_hObject, 0.0f);

	// If we aren't visible it must be time to respawn...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if (!(dwFlags & FLAG_VISIBLE))
	{
        g_pLTServer->SetObjectFlags(m_hObject, m_dwFlags | FLAG_VISIBLE);

		// Let the world know what happened...

		if (m_hstrRespawnSoundFile)
		{
            LTVector vPos;
            g_pLTServer->GetObjectPos(m_hObject, &vPos);
            g_pServerSoundMgr->PlaySoundFromPos(vPos, g_pLTServer->GetStringData(m_hstrRespawnSoundFile),
						     600.0f, SOUNDPRIORITY_MISC_HIGH);
        }
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PickedUp()
//
//	PURPOSE:	Called when an object tells this item that the object
//				picked it up.
//
// ----------------------------------------------------------------------- //

void PickupItem::PickedUp(HMESSAGEREAD hRead)
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
	if (fRespawn == -1.0f) fRespawn = m_fRespawnDelay;

	if (fRespawn <= 0.0f || g_pGameServerShell->GetGameType() == SINGLE)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
	else
	{
        g_pLTServer->SetNextUpdate(m_hObject, fRespawn / g_RespawnScaleTrack.GetFloat(1.0f));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PlayPickedupSound()
//
//	PURPOSE:	Play the picked up sound
//
// ----------------------------------------------------------------------- //

void PickupItem::PlayPickedupSound()
{
	if (m_hstrSoundFile)
	{
		if (m_hPlayerObj)
		{
			// Play it in the player's head...

            g_pServerSoundMgr->PlaySoundFromObject(m_hPlayerObj, g_pLTServer->GetStringData(m_hstrSoundFile), 600.0f, SOUNDPRIORITY_MISC_HIGH, PLAYSOUND_CLIENTLOCAL);
		}
		else  // Play from pos, more efficient...
		{
            LTVector vPos;
            g_pLTServer->GetObjectPos(m_hObject, &vPos);
            g_pServerSoundMgr->PlaySoundFromPos(vPos, g_pLTServer->GetStringData(m_hstrSoundFile), 600.0f, SOUNDPRIORITY_MISC_HIGH);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetPlayerObj
//
//	PURPOSE:	Set our player obj data member
//
// ----------------------------------------------------------------------- //

void PickupItem::SetPlayerObj(HOBJECT hObj)
{
	if (m_hPlayerObj)
	{
        g_pLTServer->BreakInterObjectLink(m_hObject, m_hPlayerObj);
	}

	m_hPlayerObj = hObj;

	if (m_hPlayerObj)
	{
        g_pLTServer->CreateInterObjectLink(m_hObject, m_hPlayerObj);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PickupItem::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hPlayerObj);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fRespawnDelay);
    g_pLTServer->WriteToMessageByte(hWrite, m_bRotate);
    g_pLTServer->WriteToMessageByte(hWrite, m_bBounce);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwUserFlags);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwFlags);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPickupTriggerTarget);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPickupTriggerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrSoundFile);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrRespawnSoundFile);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vScale);

	// Update 1.003?  Should be saved for all versions, but that gets
	// complicated with loading saved games from pre 1.003...
#ifdef _DEMO
    g_pLTServer->WriteToMessageByte(hWrite, m_bMoveToFloor);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFirstUpdate);
#endif

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PickupItem::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hPlayerObj);

    m_fRespawnDelay             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bRotate                   = g_pLTServer->ReadFromMessageByte(hRead);
    m_bBounce                   = g_pLTServer->ReadFromMessageByte(hRead);
    m_dwUserFlags               = g_pLTServer->ReadFromMessageDWord(hRead);
    m_dwFlags                   = g_pLTServer->ReadFromMessageDWord(hRead);
    m_hstrPickupTriggerTarget   = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrPickupTriggerMessage  = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrSoundFile             = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrRespawnSoundFile      = g_pLTServer->ReadFromMessageHString(hRead);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vScale);

	// Update 1.003?  Should be saved for all versions, but that gets
	// complicated with loading saved games from pre 1.003...
#ifdef _DEMO
    m_bMoveToFloor              = g_pLTServer->ReadFromMessageByte(hRead);
    m_bFirstUpdate              = g_pLTServer->ReadFromMessageByte(hRead);
#endif

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::CacheFiles
//
//	PURPOSE:	Cache whatever resources this object uses
//
// ----------------------------------------------------------------------- //

void PickupItem::CacheFiles()
{
	if (m_hstrSoundFile)
	{
        char* pFile = g_pLTServer->GetStringData(m_hstrSoundFile);
		if (pFile && pFile[0])
		{
            g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrRespawnSoundFile)
	{
        char* pFile = g_pLTServer->GetStringData(m_hstrRespawnSoundFile);
		if (pFile && pFile[0])
		{
            g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}
}