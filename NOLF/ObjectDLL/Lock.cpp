// ----------------------------------------------------------------------- //
//
// MODULE  : Lock.cpp
//
// PURPOSE : Implementation of the Lock object
//
// CREATED : 01/11/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Lock.h"
#include "SoundMgr.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "WeaponMgr.h"
#include "CommandMgr.h"
#include "ClientWeaponSFX.h"
#include "PlayerObj.h"

// Statics

static char s_szGadget[]	= "GADGET";

BEGIN_CLASS(Lock)

	// Override base-class properties...
	ADD_REALPROP_FLAG(HitPoints, 100.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxHitPoints, 100.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP1 | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanHeal, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanRepair, LTFALSE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanDamage, LTTRUE, PF_GROUP1 | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1 | PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\Lock.abc", PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, "Props\\Skins\\Lock.dtx", PF_FILENAME)

	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SOLID_FLAG(1, 0)

    ADD_BOOLPROP_FLAG(MoveToFloor, LTFALSE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Alpha, 1.0f, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Additive, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Multiply, LTFALSE, PF_HIDDEN)

	ADD_STRINGPROP(UnlockCommand, "")
	ADD_STRINGPROP_FLAG(UnlockSound, "", PF_FILENAME)
	ADD_STRINGPROP_FLAG(PickSound, "Snd\\Doors\\lockpick.wav", PF_FILENAME)
	ADD_REALPROP_FLAG(SoundRadius, 500.0f, PF_RADIUS)
	ADD_REALPROP_FLAG(UnlockHitPts, 100.0f, 0)
	ADD_REALPROP_FLAG(MinUnlockHitPts, 5.0f, 0)
	ADD_REALPROP_FLAG(MaxUnlockHitPts, 10.0f, 0)
    ADD_BOOLPROP_FLAG(Shootable, LTFALSE, 0)
    ADD_BOOLPROP_FLAG(Weldable, LTFALSE, 0)
    ADD_BOOLPROP_FLAG(Lightable, LTFALSE, 0)
    ADD_BOOLPROP_FLAG(RemoveWhenUnlocked, LTFALSE, 0)

	// Set our default debris type...
	ADD_STRINGPROP_FLAG(DebrisType, "Lock", PF_STATICLIST)

END_CLASS_DEFAULT(Lock, Prop, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::Lock()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Lock::Lock() : Prop()
{
    m_hstrUnlockCmd     = LTNULL;
    m_hstrUnlockSnd     = LTNULL;
    m_hstrPickSnd       = LTNULL;
	m_fSndRadius		= 500.0f;

	m_fUnlockHitPts		= 100.0f;
	m_fMinUnlockHitPts	= 5.0f;
	m_fMaxUnlockHitPts	= 10.0f;

    m_bShootable        = LTFALSE;
    m_bWeldable         = LTFALSE;
    m_bLightable        = LTFALSE;
    m_bRemoveWhenDone   = LTFALSE;
    m_bUnlocked         = LTFALSE;

    m_damage.m_bRemoveOnDeath = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::~Lock()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Lock::~Lock()
{
	FREE_HSTRING(m_hstrUnlockCmd);
	FREE_HSTRING(m_hstrUnlockSnd);
	FREE_HSTRING(m_hstrPickSnd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Lock::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			Cache();
		}
		break;

		case MID_INITIALUPDATE :
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Lock::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
            ILTCommon* pCommon = g_pLTServer->Common();
			if (!pCommon) return 0;

			const char *szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			// ConParse does not destroy szMsg, so this is safe
			ConParse parse;
			parse.Init((char*)szMsg);

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (_stricmp(parse.m_Args[0], s_szGadget) == 0)
					{
						HandleGadgetMsg(hSender, parse);
					}
				}
			}
		}
		break;

		case MID_DAMAGE:
		{
			if (m_bUnlocked) break;

            uint32 dwRet = 0;

			DamageStruct damage;
			damage.InitFromMessage(hRead);

            LTBOOL bProcessDamage = m_bShootable;

			if (!bProcessDamage)
			{
				if (m_bWeldable)
				{
					bProcessDamage = (damage.eType == DT_GADGET_WELDER);
				}
				else if (m_bLightable)
				{
					bProcessDamage = (damage.eType == DT_GADGET_LIGHTER);
				}
				else
				{
					bProcessDamage = (damage.eType == DT_GADGET_LOCK_PICK);
				}
			}

			if (bProcessDamage)
			{
				dwRet = Prop::ObjectMessageFn(hSender, messageID, hRead);

				if (m_damage.IsDead())
				{
					SetupUnlockState();
				}
			}

			return dwRet;
		}
		break;

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::HandleGadgetMsg
//
//	PURPOSE:	Handle gadget messages
//
// ----------------------------------------------------------------------- //

void Lock::HandleGadgetMsg(HOBJECT hSender, ConParse & parse)
{
	if (parse.m_nArgs < 2 || !parse.m_Args[1]) return;

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(atol(parse.m_Args[1]));
	if (!pAmmo) return;

	LTBOOL bProcess = LTFALSE;

	if (m_bWeldable && pAmmo->eInstDamageType == DT_GADGET_WELDER)
	{
		bProcess = LTTRUE;
	}
	else if (m_bLightable && pAmmo->eInstDamageType == DT_GADGET_LIGHTER)
	{
		bProcess = LTTRUE;
	}
	else if (!m_bWeldable && pAmmo->eInstDamageType == DT_GADGET_LOCK_PICK)
	{
		bProcess = LTTRUE;
	}

	if (!bProcess) return;

	// Pick the lock by doing lock-pick damage to it...

	DamageStruct damage;

	damage.eType	= pAmmo->eInstDamageType;
	damage.fDamage  = GetRandom(m_fMinUnlockHitPts, m_fMaxUnlockHitPts);
	damage.vDir.Init(0, 1, 0);

	damage.hDamager = m_hObject;
	damage.DoDamage(this, m_hObject);


	// Play the lock pick sound...

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (m_hstrPickSnd)
	{
 
        char* pSound = g_pLTServer->GetStringData(m_hstrPickSnd);
		if (pSound)
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSndRadius,
				SOUNDPRIORITY_MISC_MEDIUM);
		}
	}

	
	// Do fx for welding the lock...

	if (m_bWeldable && pAmmo->eInstDamageType == DT_GADGET_WELDER)
	{
		WEAPON* pWeapon = g_pWeaponMgr->GetWeapon("Lighter");
		if (!pWeapon) return;

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface("Metal");
		if (!pSurf) return;

		LTVector vHisPos;
		g_pLTServer->GetObjectPos(hSender, &vHisPos);
		LTVector vDir = vHisPos - vPos;
		vDir.Norm();

		CLIENTWEAPONFX fxStruct;
		fxStruct.hFiredFrom		= hSender;
		fxStruct.vSurfaceNormal	= vDir;
		fxStruct.vFirePos		= vHisPos;
		fxStruct.vPos			= vPos + vDir;
		fxStruct.hObj			= m_hObject;
		fxStruct.nWeaponId		= pWeapon->nId;
		fxStruct.nAmmoId		= pAmmo->nId;
		fxStruct.nSurfaceType	= pSurf->eType;

		// This should be a player object, get the client id...

		if (IsPlayer(hSender))
		{
			CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hSender);
			if (pPlayer)
			{
				fxStruct.nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
			}
		}

		CreateClientWeaponFX(fxStruct);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL Lock::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("UnlockCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrUnlockCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("UnlockSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrUnlockSnd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PickSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPickSnd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("SoundRadius", &genProp) == LT_OK)
	{
		m_fSndRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("UnlockHitPts", &genProp) == LT_OK)
	{
		m_fUnlockHitPts = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MinUnlockHitPts", &genProp) == LT_OK)
	{
		m_fMinUnlockHitPts = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("MaxUnlockHitPts", &genProp) == LT_OK)
	{
		m_fMaxUnlockHitPts = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Shootable", &genProp) == LT_OK)
	{
		m_bShootable = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Weldable", &genProp) == LT_OK)
	{
		m_bWeldable = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Lightable", &genProp) == LT_OK)
	{
		m_bLightable = genProp.m_Bool;
	}

	// Make sure that ALL lock-pickable locks can be shot...And all weld-only
	// locks Can't be shot!

	if (!m_bWeldable && !m_bLightable)
	{
		m_bShootable = LTTRUE;
	}
	else if (m_bWeldable && !m_bLightable)
	{
		m_bShootable = LTFALSE;
	}

    if (g_pLTServer->GetPropGeneric("RemoveWhenUnlocked", &genProp) == LT_OK)
	{
		m_bRemoveWhenDone = genProp.m_Bool;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL Lock::InitialUpdate()
{
    SetNextUpdate(0.0);

	// Setup up our destructible object...

	m_damage.SetHitPoints(m_fUnlockHitPts);

    uint32 dwCanDamageTypes = 0;

	// Set it up so nothing can damage us...

	m_damage.SetCantDamageTypes(~dwCanDamageTypes);

	// See if we are affected by the welder or the lockpick...

	if (m_bWeldable)
	{
		m_dwUsrFlgs	|= USRFLG_GADGET_WELDER;

		dwCanDamageTypes = (DamageTypeToFlag(DT_GADGET_WELDER) |
			DamageTypeToFlag(DT_EXPLODE) | DamageTypeToFlag(DT_BULLET) |
			DamageTypeToFlag(DT_BURN));
	}
	else if (m_bLightable)
	{
		m_dwUsrFlgs	|= USRFLG_GADGET_LIGHTER;

		dwCanDamageTypes |= (DamageTypeToFlag(DT_GADGET_LIGHTER) |
			DamageTypeToFlag(DT_EXPLODE) | DamageTypeToFlag(DT_BULLET) |
			DamageTypeToFlag(DT_BURN));
	}
	else // Lockpick
	{
		m_dwUsrFlgs	|= USRFLG_GADGET_LOCK_PICK;

		dwCanDamageTypes |= (DamageTypeToFlag(DT_GADGET_LOCK_PICK) |
			DamageTypeToFlag(DT_EXPLODE) | DamageTypeToFlag(DT_BULLET) |
			DamageTypeToFlag(DT_BURN));
	}

	// Okay, now clear the types we want to damage us...

	m_damage.ClearCantDamageTypes(dwCanDamageTypes);


	// Make sure we can be activated even if we are non-solid...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_RAYHIT);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::SetupUnlockState
//
//	PURPOSE:	Setup the un locked state
//
// ----------------------------------------------------------------------- //

void Lock::SetupUnlockState()
{
    m_bUnlocked = LTTRUE;

    uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	if (m_bWeldable)
	{
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_GADGET_WELDER);
	}
	else if (m_bLightable)
	{
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_GADGET_LIGHTER);
	}
	else
	{
        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags & ~USRFLG_GADGET_LOCK_PICK);
	}


	// Set the Lock's animation...

    HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hObject, "Open");
	if (hAni)
	{
        g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
        g_pLTServer->SetModelAnimation(m_hObject, hAni);
	}


	// Play the unlock sound...

	if (m_hstrUnlockSnd)
	{
        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

        char* pSound = g_pLTServer->GetStringData(m_hstrUnlockSnd);
		if (pSound)
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSndRadius,
				SOUNDPRIORITY_MISC_MEDIUM);
		}
	}


	// Send the unlock command...

	if (m_hstrUnlockCmd)
	{
        char* pCmd = g_pLTServer->GetStringData(m_hstrUnlockCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}
	}


	// Remove if necessary...

	if (m_bRemoveWhenDone)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Lock::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    g_pLTServer->WriteToMessageHString(hWrite, m_hstrUnlockCmd);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrUnlockSnd);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPickSnd);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fSndRadius);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fUnlockHitPts);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMinUnlockHitPts);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxUnlockHitPts);
    g_pLTServer->WriteToMessageByte(hWrite, m_bShootable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bUnlocked);
    g_pLTServer->WriteToMessageByte(hWrite, m_bWeldable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bLightable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bRemoveWhenDone);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Lock::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

    m_hstrUnlockCmd     = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrUnlockSnd     = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrPickSnd       = g_pLTServer->ReadFromMessageHString(hRead);
    m_fSndRadius        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fUnlockHitPts     = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMinUnlockHitPts  = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMaxUnlockHitPts  = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bShootable        = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bUnlocked         = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bWeldable         = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bLightable        = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_bRemoveWhenDone   = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::Cache
//
//	PURPOSE:	Cache the sounds....
//
// ----------------------------------------------------------------------- //

void Lock::Cache()
{
	if (m_hstrUnlockSnd)
	{
        g_pLTServer->CacheFile(FT_SOUND, g_pLTServer->GetStringData(m_hstrUnlockSnd));
	}

	if (m_hstrPickSnd)
	{
        g_pLTServer->CacheFile(FT_SOUND, g_pLTServer->GetStringData(m_hstrPickSnd));
	}
}