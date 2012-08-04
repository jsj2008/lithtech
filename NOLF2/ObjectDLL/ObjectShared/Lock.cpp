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
#include "ParsedMsg.h"
#include "WeaponMgr.h"
#include "CommandMgr.h"
#include "ClientWeaponSFX.h"
#include "PlayerObj.h"
#include "ServerSoundMgr.h"

LINKFROM_MODULE( Lock );

// Statics

static char s_szGadget[]	= "GADGET";

#pragma force_active on
BEGIN_CLASS(Lock)

	// Override base-class properties...
	ADD_REALPROP_FLAG(HitPoints, 100.0f, PF_GROUP(1) | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxHitPoints, 100.0f, PF_GROUP(1) | PF_HIDDEN)
	ADD_REALPROP_FLAG(Armor, 0.0f, PF_GROUP(1) | PF_HIDDEN)
	ADD_REALPROP_FLAG(MaxArmor, 0.0f, PF_GROUP(1) | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanHeal, LTFALSE, PF_GROUP(1) | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanRepair, LTFALSE, PF_GROUP(1) | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(CanDamage, LTTRUE, PF_GROUP(1) | PF_HIDDEN)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP(1) | PF_HIDDEN)

	ADD_STRINGPROP_FLAG(Filename, "Props\\Models\\Lock.ltb", PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
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

END_CLASS_DEFAULT_FLAGS(Lock, Prop, NULL, NULL, CF_HIDDEN) // [RP] We no longer use this object, use GadgetTarget instead.
#pragma force_active off

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
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
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

uint32 Lock::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    if (!g_pLTServer) return 0;

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (m_bUnlocked) break;

            uint32 dwRet = 0;

			DamageStruct damage;
			damage.InitFromMessage(pMsg);

            LTBOOL bProcessDamage = m_bShootable;

			if (!bProcessDamage)
			{
				if (m_bWeldable)
				{
					bProcessDamage = (damage.eType == DT_GADGET_WELDER);
				}
				else
				{
					bProcessDamage = (damage.eType == DT_GADGET_LOCK_PICK);
				}
			}

			if (bProcessDamage)
			{
				dwRet = Prop::ObjectMessageFn(hSender, pMsg);

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

	return Prop::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool Lock::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Gadget(s_szGadget);

	if (cMsg.GetArg(0) == s_cTok_Gadget)
	{
		HandleGadgetMsg(hSender, cMsg);
	}
	else
		return Prop::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::HandleGadgetMsg
//
//	PURPOSE:	Handle gadget messages
//
// ----------------------------------------------------------------------- //

void Lock::HandleGadgetMsg(HOBJECT hSender, const CParsedMsg &cMsg)
{
	if (cMsg.GetArgCount() < 2) return;

	AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(atol(cMsg.GetArg(1)));
	if (!pAmmo) return;

	LTBOOL bProcess = LTFALSE;

	if (m_bWeldable && pAmmo->eInstDamageType == DT_GADGET_WELDER)
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
 
        const char* pSound = g_pLTServer->GetStringData(m_hstrPickSnd);
		if (pSound)
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSndRadius,
				SOUNDPRIORITY_MISC_MEDIUM);
		}
	}

	
	// Do fx for welding the lock...

	if (m_bWeldable && pAmmo->eInstDamageType == DT_GADGET_WELDER)
	{
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon("Lighter");
		if (!pWeapon) return;

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface("Metal");
		if (!pSurf) return;

		LTVector vHisPos;
		g_pLTServer->GetObjectPos(hSender, &vHisPos);
		LTVector vDir = vHisPos - vPos;
		vDir.Normalize();

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
    SetNextUpdate(UPDATE_NEVER);

	// Setup up our destructible object...

	m_damage.SetHitPoints(m_fUnlockHitPts);

    DamageFlags nCanDamageFlags = 0;

	// Set it up so nothing can damage us...

	m_damage.SetCantDamageFlags(~nCanDamageFlags);

	// See if we are affected by the welder or the lockpick...

	if (m_bWeldable)
	{
		m_dwUsrFlgs	|= USRFLG_GADGET_WELDER;

		nCanDamageFlags = 
			(
				DamageTypeToFlag(DT_GADGET_WELDER) |
				DamageTypeToFlag(DT_EXPLODE) |
				DamageTypeToFlag(DT_BULLET) |
				DamageTypeToFlag(DT_BURN)
			);
	}
	else // Lockpick
	{
		m_dwUsrFlgs	|= USRFLG_GADGET_LOCK_PICK;

		nCanDamageFlags |= 
			(
				DamageTypeToFlag(DT_GADGET_LOCK_PICK) |
				DamageTypeToFlag(DT_EXPLODE) |
				DamageTypeToFlag(DT_BULLET) |
				DamageTypeToFlag(DT_BURN)
			);
	}

	// Okay, now clear the types we want to damage us...

	m_damage.ClearCantDamageFlags(nCanDamageFlags);


	// Make sure we can be activated even if we are non-solid...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT);

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

	if (m_bWeldable)
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_GADGET_WELDER);
	}
	else
	{
        g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_GADGET_LOCK_PICK);
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

        const char* pSound = g_pLTServer->GetStringData(m_hstrUnlockSnd);
		if (pSound)
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSound, m_fSndRadius,
				SOUNDPRIORITY_MISC_MEDIUM);
		}
	}


	// Send the unlock command...

	if (m_hstrUnlockCmd)
	{
        const char* pCmd = g_pLTServer->GetStringData(m_hstrUnlockCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
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

void Lock::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_HSTRING(m_hstrUnlockCmd);
    SAVE_HSTRING(m_hstrUnlockSnd);
    SAVE_HSTRING(m_hstrPickSnd);
    SAVE_FLOAT(m_fSndRadius);
    SAVE_FLOAT(m_fUnlockHitPts);
    SAVE_FLOAT(m_fMinUnlockHitPts);
    SAVE_FLOAT(m_fMaxUnlockHitPts);
    SAVE_BOOL(m_bShootable);
    SAVE_BOOL(m_bUnlocked);
    SAVE_BOOL(m_bWeldable);
    SAVE_BOOL(m_bLightable);
    SAVE_BOOL(m_bRemoveWhenDone);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Lock::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Lock::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    LOAD_HSTRING(m_hstrUnlockCmd);
    LOAD_HSTRING(m_hstrUnlockSnd);
    LOAD_HSTRING(m_hstrPickSnd);
    LOAD_FLOAT(m_fSndRadius);
    LOAD_FLOAT(m_fUnlockHitPts);
    LOAD_FLOAT(m_fMinUnlockHitPts);
    LOAD_FLOAT(m_fMaxUnlockHitPts);
    LOAD_BOOL(m_bShootable);
    LOAD_BOOL(m_bUnlocked);
    LOAD_BOOL(m_bWeldable);
    LOAD_BOOL(m_bLightable);
	LOAD_BOOL(m_bRemoveWhenDone);
}

