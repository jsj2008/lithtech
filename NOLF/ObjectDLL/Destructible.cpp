// ----------------------------------------------------------------------- //
//
// MODULE  : Destructible.h
//
// PURPOSE : Destructible class
//
// CREATED : 9/23/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "Destructible.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "Projectile.h"
#include "ServerUtilities.h"
#include "Character.h"
#include "PlayerObj.h"
#include "GameServerShell.h"
#include "ObjectMsgs.h"
#include "iltphysics.h"
#include "Camera.h"

#define TRIGGER_MSG_DESTROY	"DESTROY"
#define TRIGGER_MSG_RESET	"RESET"

#define MAXIMUM_BLOCK_PRIORITY	255.0f
#define MINIMUM_FRICTION		5.0f
#define MAXIMUM_FRICTION		15.0f
#define MINIMUM_FORCE			0.0f

extern CGameServerShell* g_pGameServerShell;
extern CVarTrack g_DamageScale;
extern CVarTrack g_HealScale;
extern CVarTrack g_vtNetFriendlyFire;
extern CVarTrack g_NetUseSpawnLimit;
extern CVarTrack g_NetSpawnLimit;
extern CVarTrack g_NetArmorHealthPercent;
const LTFLOAT DamageStruct::kInfiniteDamage = 100000.0f; // Infinite damage


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::InitFromMessage
//
//	PURPOSE:	Initialize from a MID_DAMAGE message
//
// ----------------------------------------------------------------------- //

LTBOOL DamageStruct::InitFromMessage(HMESSAGEREAD hRead)
{
    if (!hRead) return LTFALSE;

    g_pLTServer->ReadFromMessageVector(hRead, &vDir);
    fDuration  = g_pLTServer->ReadFromMessageFloat(hRead);
    fDamage    = g_pLTServer->ReadFromMessageFloat(hRead) * g_DamageScale.GetFloat(1.0f);
    eType      = (DamageType)g_pLTServer->ReadFromMessageByte(hRead);
    hDamager   = g_pLTServer->ReadFromMessageObject(hRead);
    hContainer = g_pLTServer->ReadFromMessageObject(hRead);

    g_pLTServer->ResetRead(hRead);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::DoDamage
//
//	PURPOSE:	Send the MID_DAMAGE message to do the damage
//
// ----------------------------------------------------------------------- //

LTBOOL DamageStruct::DoDamage(LPBASECLASS pDamager, HOBJECT hVictim, HOBJECT hContainer)
{
    if (!pDamager || !hVictim) return LTFALSE;

    HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pDamager, hVictim, MID_DAMAGE);
    g_pLTServer->WriteToMessageVector(hWrite, &vDir);
    g_pLTServer->WriteToMessageFloat(hWrite, fDuration);
    g_pLTServer->WriteToMessageFloat(hWrite, fDamage);
    g_pLTServer->WriteToMessageByte(hWrite, eType);
    g_pLTServer->WriteToMessageObject(hWrite, hDamager);
    g_pLTServer->WriteToMessageObject(hWrite, hContainer);
    g_pLTServer->EndMessage(hWrite);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::CDestructible
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDestructible::CDestructible() : IAggregate()
{
    m_hObject                   = LTNULL;
    m_hLastDamager              = LTNULL;

    m_bDead                     = LTFALSE;
    m_bIsPlayer                 = LTFALSE;
    m_bApplyDamagePhysics       = LTFALSE;
	m_fMass						= 1.0f;
	m_fHitPoints				= 1.0f;
	m_fMaxHitPoints				= 1.0f;
	m_fArmorPoints				= 0.0f;
	m_fMaxArmorPoints			= 1.0f;
	m_fDeathDamage				= 0.0f;
	m_fLastDamage				= 0.0f;
	m_fLastArmorAbsorb			= 0.0f;
	m_dwCantDamageTypes			= DamageTypeToFlag(DT_GADGET_DECAYPOWDER);
	m_eLastDamageType			= DT_UNSPECIFIED;
	m_eDeathType				= DT_UNSPECIFIED;

	VEC_INIT(m_vDeathDir);
	VEC_INIT(m_vLastDamageDir);

    m_hstrDamageTriggerTarget       = LTNULL;
    m_hstrDamageTriggerMessage      = LTNULL;
    m_hstrDamagerMessage            = LTNULL;
    m_hstrDeathTriggerTarget        = LTNULL;
    m_hstrDeathTriggerMessage       = LTNULL;
    m_hstrPlayerDeathTriggerTarget  = LTNULL;
    m_hstrPlayerDeathTriggerMessage = LTNULL;
    m_hstrKillerMessage             = LTNULL;
	m_nDamageTriggerCounter			= 0;
	m_nDamageTriggerNumSends		= 1;

    m_bCanHeal                  = LTTRUE;
    m_bCanRepair                = LTTRUE;
    m_bCanDamage                = LTTRUE;
    m_bNeverDestroy             = LTFALSE;

	memset(m_bGearOwned, 0, sizeof(m_bGearOwned));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::~CDestructible
//
//	PURPOSE:	Destructible
//
// ----------------------------------------------------------------------- //

CDestructible::~CDestructible()
{
	if (m_hstrDamageTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrDamageTriggerTarget);
	}

	if (m_hstrDamageTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrDamageTriggerMessage);
	}

	if (m_hstrDeathTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrDeathTriggerTarget);
	}

	if (m_hstrDeathTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrDeathTriggerMessage);
	}

	if (m_hstrPlayerDeathTriggerTarget)
	{
        g_pLTServer->FreeString(m_hstrPlayerDeathTriggerTarget);
	}

	if (m_hstrPlayerDeathTriggerMessage)
	{
        g_pLTServer->FreeString(m_hstrPlayerDeathTriggerMessage);
	}

	if (m_hstrDamagerMessage)
	{
        g_pLTServer->FreeString(m_hstrDamagerMessage);
	}

	if (m_hstrKillerMessage)
	{
        g_pLTServer->FreeString(m_hstrKillerMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CDestructible::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			HandleTouch(pObject, (HOBJECT)pData, fData);
		}
		break;

		case MID_CRUSH:
		{
			HandleCrush(pObject, (HOBJECT)pData);
		}
		break;

		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pObject);
			}
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_LINKBROKEN :
		{
			HandleLinkBroken((HOBJECT)pData);
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((HMESSAGEWRITE)pData, (uint8)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((HMESSAGEREAD)pData, (uint8)fData);
		}
		break;
	}

    return IAggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CDestructible::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(pObject, hSender, hRead);
		}
		break;

		case MID_DAMAGE:
		{
			HandleDamage(pObject, hSender, hRead);
		}
		break;

		case MID_REPAIR:
		{
			if (m_bCanRepair)
			{
				HandleRepair(pObject, hSender, hRead);
			}
		}
		break;

		case MID_HEAL:
		{
			if (m_bCanHeal)
			{
				HandleHeal(pObject, hSender, hRead);
			}
		}
		break;

		case MID_ADDGEAR:
		{
			HandleAddGear(pObject, hSender, hRead);
		}
		break;

		default : break;
	}

    return IAggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleLinkBroken
//
//	PURPOSE:	Handle broken links
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleLinkBroken(HOBJECT hObj)
{
	if (hObj == m_hLastDamager)
	{
        m_hLastDamager = LTNULL;
	}

	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].hDamager == hObj)
		{
			m_ProgressiveDamage[i].Clear();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::ReadProp(BaseClass *pObject, ObjectCreateStruct *pStruct)
{
    if (!pStruct) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("Mass", &genProp) == LT_OK)
	{
		SetMass(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("HitPoints", &genProp) == LT_OK)
	{
		SetHitPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxHitPoints", &genProp) == LT_OK)
	{
		SetMaxHitPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("Armor", &genProp) == LT_OK)
	{
		SetArmorPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxArmor", &genProp) == LT_OK)
	{
		SetMaxArmorPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDamageTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDamageTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerCounter", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_nDamageTriggerCounter = genProp.m_Long;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamageTriggerNumSends", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_nDamageTriggerNumSends = genProp.m_Long;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamagerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDamagerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DeathTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDeathTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DeathTriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDeathTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerDeathTriggerTarget", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPlayerDeathTriggerTarget = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerDeathTriggerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPlayerDeathTriggerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("KillerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrKillerMessage = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("CanHeal", &genProp) == LT_OK)
	{
		m_bCanHeal = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CanRepair", &genProp) == LT_OK)
	{
		m_bCanRepair = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CanDamage", &genProp) == LT_OK)
	{
		m_bCanDamage = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("NeverDestroy", &genProp) == LT_OK)
	{
		m_bNeverDestroy = genProp.m_Bool;
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Init
//
//	PURPOSE:	Initialize object - no longer used (InitialUpdate() does this)
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::Init(HOBJECT hObject)
{
	if (!m_hObject) m_hObject = hObject;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::InitialUpdate
//
//	PURPOSE:	Handle object initial update
//
// ----------------------------------------------------------------------- //

void CDestructible::InitialUpdate(LPBASECLASS pObject)
{
	if (!pObject || !pObject->m_hObject) return;
	if (!m_hObject) m_hObject = pObject->m_hObject;

	SetBlockingPriority();

	if (m_fMass > 1.0f)
	{
        g_pLTServer->SetObjectMass(m_hObject, m_fMass);
	}

	// Determine if we are a player object or not

	if (IsPlayer(m_hObject))
	{
        m_bIsPlayer = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Update
//
//	PURPOSE:	Handle object updates
//
// ----------------------------------------------------------------------- //

void CDestructible::Update()
{
	// Update progressive damage...Don't allow progressive damage to happen 
	// in cinematics (fixes bugs with the player breaking cinematics)...

	if (Camera::IsActive())
	{
		ClearProgressiveDamage();
	}
	else
	{
		UpdateProgressiveDamage();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ClearProgressiveDamage
//
//	PURPOSE:	Remove all progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamage()
{
	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].fDuration = 0.0f;
	}
	UpdateProgressiveDamage();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::UpdateProgressiveDamage
//
//	PURPOSE:	Update any progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::UpdateProgressiveDamage()
{
	DamageStruct damage;

    LTFLOAT fFrameTime = g_pLTServer->GetFrameTime();

    LTBOOL bBleeding = LTFALSE, bPoisoned = LTFALSE, bStunned = LTFALSE, bSleeping = LTFALSE;
    LTBOOL bBurned = LTFALSE, bElectrocuted = LTFALSE;

	// Loop over all the progressive damage done to us...

	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].fDuration > 0.0f)
		{
			m_ProgressiveDamage[i].fDuration -= fFrameTime;

			// Process the damage for this frame...

			damage = m_ProgressiveDamage[i];
			damage.fDamage = damage.fDamage * fFrameTime;
			DoDamage(damage);

			if (damage.eType == DT_BLEEDING)
			{
                bBleeding = LTTRUE;
			}
			else if (damage.eType == DT_POISON)
			{
                bPoisoned = LTTRUE;
			}
			else if (damage.eType == DT_STUN)
			{
                bStunned = LTTRUE;
			}
			else if (damage.eType == DT_SLEEPING)
			{
                bSleeping = LTTRUE;
			}
			else if (damage.eType == DT_BURN)
			{
                bBurned = LTTRUE;
			}
			else if (damage.eType == DT_ELECTROCUTE)
			{
                bElectrocuted = LTTRUE;
			}
		}
	}


	// See if we have protection from bleeding damage...

	if (bBleeding)
	{
        LTFLOAT fProtection = 0.0f;

        GEAR* pGear = LTNULL;

		for (int nGearId=0; nGearId < MAX_GEAR_ITEMS; nGearId++)
		{
			if (m_bGearOwned[nGearId])
			{
				pGear = g_pWeaponMgr->GetGear(nGearId);

				if (pGear && pGear->eProtectionType == DT_BLEEDING)
				{
					fProtection += pGear->fProtection;

					if (fProtection >= 1.0f)
					{
                        bBleeding = LTFALSE;
						break;
					}
				}
			}
		}
	}


	// Set our user flags if we're bleeding/poisoned/stunned/sleeping...

	if (IsCharacter(m_hObject))
	{
        uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		if (bBleeding)
		{
			dwUserFlags |= USRFLG_CHAR_BLEEDING;
		}
		else
		{
			dwUserFlags &= ~USRFLG_CHAR_BLEEDING;
		}

		if (bPoisoned)
		{
			dwUserFlags |= USRFLG_CHAR_POISONED;
		}
		else
		{
			dwUserFlags &= ~USRFLG_CHAR_POISONED;
		}

		if (bStunned)
		{
			dwUserFlags |= USRFLG_CHAR_STUNNED;
		}
		else
		{
			dwUserFlags &= ~USRFLG_CHAR_STUNNED;
		}

		if (bSleeping)
		{
			dwUserFlags |= USRFLG_CHAR_SLEEPING;
		}
		else
		{
			dwUserFlags &= ~USRFLG_CHAR_SLEEPING;
		}

		if (bBurned)
		{
			dwUserFlags |= USRFLG_CHAR_BURN;
		}
		else
		{
			dwUserFlags &= ~USRFLG_CHAR_BURN;
		}

		if (bElectrocuted)
		{
			dwUserFlags |= USRFLG_CHAR_ELECTROCUTE;
		}
		else
		{
			dwUserFlags &= ~USRFLG_CHAR_ELECTROCUTE;
		}

        g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::AddProgressiveDamage
//
//	PURPOSE:	Add progressive damage to the object
//
// ----------------------------------------------------------------------- //

void CDestructible::AddProgressiveDamage(DamageStruct & damage)
{
    LTFLOAT fLeastDuration = 100000.0f;
	int nIndex = -1;

	HOBJECT hDamager	= damage.hDamager;
    LPBASECLASS pObject = g_pLTServer->HandleToObject(m_hObject);
	if (IsPlayer(hDamager))
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
		if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && IsPlayer(m_hObject) && g_vtNetFriendlyFire.GetFloat() < 1.0f)
		{
			CPlayerObj* pPlayer2 = (CPlayerObj*) pObject;
			if (pPlayer != pPlayer2 && pPlayer->GetTeamID() == pPlayer2->GetTeamID())
			{
				return;
			}
		}
	}

	// If the new damage is from a container, see if this container
	// is already damaging us...

    int i;
	if (damage.hContainer)
	{
        for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
		{
			if (m_ProgressiveDamage[i].hContainer == damage.hContainer)
			{
				m_ProgressiveDamage[i] = damage;

				if (damage.hDamager)
				{
					g_pLTServer->CreateInterObjectLink(m_hObject, damage.hDamager);
				}

				return;
			}
		}
	}



    for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].fDuration <= 0.0f)
		{
			m_ProgressiveDamage[i] = damage;

			if (damage.hDamager)
			{
				g_pLTServer->CreateInterObjectLink(m_hObject, damage.hDamager);
			}

			return;
		}
		else
		{
			if (m_ProgressiveDamage[i].fDuration < fLeastDuration)
			{
				fLeastDuration = m_ProgressiveDamage[i].fDuration;
				nIndex = i;
			}
		}
	}

	// If we got to here there were no empty slots...Replace the slot
	// with the least amount of time if the new progressive damage item
	// would do more damage...

	if (nIndex != -1)
	{
        LTFLOAT fDamageLeft = m_ProgressiveDamage[nIndex].fDuration * m_ProgressiveDamage[nIndex].fDamage;
        LTFLOAT fNewDamage = damage.fDuration * damage.fDamage;

		// This assumes that all damage is equal.  In the future we may
		// want to check for powerups or other items that could make this
		// assumption false...

		if (fNewDamage > fDamageLeft)
		{
			m_ProgressiveDamage[nIndex] = damage;

			if (damage.hDamager)
			{
				g_pLTServer->CreateInterObjectLink(m_hObject, damage.hDamager);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetMass
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructible::SetMass(LTFLOAT fMass)
{
	m_fMass = fMass;

	SetBlockingPriority();

	// Set the friction based on the mass of the object...

	if (!m_hObject) return;

    g_pLTServer->SetObjectMass(m_hObject, m_fMass);

    LTFLOAT fFricCoeff = MINIMUM_FRICTION + (m_fMass * MAXIMUM_FRICTION / INFINITE_MASS);
    g_pLTServer->SetFrictionCoefficient(m_hObject, fFricCoeff);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetBlockingPriority
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructible::SetBlockingPriority()
{
	if (!m_hObject) return;

    uint8 nPriority = (uint8)(m_fMass * MAXIMUM_BLOCK_PRIORITY / INFINITE_MASS);
	if (nPriority <= 0) nPriority = 1;

    g_pLTServer->SetBlockingPriority(m_hObject, nPriority);
    g_pLTServer->SetForceIgnoreLimit(m_hObject, MINIMUM_FORCE);

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_TOUCH_NOTIFY;
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxHitPoints
//
//	PURPOSE:	Returns the maximum hit points for this object
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetMaxHitPoints() const
{
    LTFLOAT fAdjustedMax = m_fMaxHitPoints;

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetMaxArmorPoints
//
//	PURPOSE:	Returns the maximum armor points for this object
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetMaxArmorPoints() const
{
    LTFLOAT fAdjustedMax = m_fMaxArmorPoints;

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			// See if we should increase/decrease the max...
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::GetStealthModifier
//
//	PURPOSE:	Returns footstep sound volume multiplier for the current gear
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetStealthModifier()
{
    LTFLOAT fMult = 1.0f;
    GEAR* pGear = LTNULL;

	for (int nGearId=0; nGearId < MAX_GEAR_ITEMS; nGearId++)
	{
		if (m_bGearOwned[nGearId])
		{
			pGear = g_pWeaponMgr->GetGear(nGearId);
			if (pGear)
			{
				fMult *= (1.0f - pGear->fStealth);
			}
		}
	}

	if (m_bIsPlayer && g_pGameServerShell->GetGameType() == SINGLE)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
        fMult *= pPlayer->GetPlayerSummaryMgr()->m_PlayerRank.fStealthMultiplier;
	};
	return fMult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HasAirSupply
//
//	PURPOSE:	Returns true if current gear provides air supply
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::HasAirSupply()
{
    LTBOOL bAir = LTFALSE;
    GEAR* pGear = LTNULL;

	for (int nGearId=0; nGearId < MAX_GEAR_ITEMS && !bAir; nGearId++)
	{
		if (m_bGearOwned[nGearId])
		{
			pGear = g_pWeaponMgr->GetGear(nGearId);
			if (pGear)
			{
				bAir = ( (pGear->eProtectionType == DT_CHOKE) && (pGear->fProtection >= 1.0f) );
			}
		}
	}
	return bAir;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDamage
//
//	PURPOSE:	Handle damage message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDamage(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	DamageStruct damage;
	damage.InitFromMessage(hRead);

	// Check for progressive damage...

	if (damage.fDuration > 0.0f)
	{
		AddProgressiveDamage(damage);
	}
	else
	{
		DoDamage(damage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DoDamage
//
//	PURPOSE:	Do the damage
//
// ----------------------------------------------------------------------- //

void CDestructible::DoDamage(DamageStruct & damage)
{
    LTVector vDir        = damage.vDir;
    LTFLOAT  fDamage     = damage.fDamage;
	DamageType eType	= damage.eType;
	HOBJECT hDamager	= damage.hDamager;

    LPBASECLASS pObject = g_pLTServer->HandleToObject(m_hObject);

	// See if we should actually process damage...

	if (m_bDead) return;


	// See if this type of damage applies to us...

	if (m_dwCantDamageTypes)
	{
		if (m_dwCantDamageTypes & DamageTypeToFlag(eType)) return;
	}

	// Process damage through any gear items...

	fDamage = ProcessGear(fDamage, eType, hDamager, &vDir);

	if (IsPlayer(hDamager))
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
		// in single player game
		if (g_pGameServerShell->GetGameType() == SINGLE)
		{
			fDamage *= pPlayer->GetPlayerSummaryMgr()->m_PlayerRank.fDamageMultiplier;
        }
		else if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && IsPlayer(m_hObject) && g_vtNetFriendlyFire.GetFloat() < 1.0f)
		{
			CPlayerObj* pPlayer2 = (CPlayerObj*) pObject;
			if (pPlayer != pPlayer2 && pPlayer->GetTeamID() == pPlayer2->GetTeamID())
			{
				fDamage = 0.0f;
			}
		}
	}


	// If we're a player, make our armor twice as absorbant...

	LTFLOAT fArmorAdjustFactor = ((m_bIsPlayer && g_pGameServerShell->GetGameType() == SINGLE) ? 2.0f : 1.0f);
    LTFLOAT fAbsorb = 0.0f;

	// Do rag-doll, and adjust damage if it was greater than 0...(if the damage
	// was zero, we still want to process it)...

	if (fDamage > 0.0f)
	{
		// Rag-doll time...

		if (CanRagDoll(vDir, eType))
		{
            LTVector vImpactVel, vTemp, vVel;

            g_pLTServer->GetVelocity(m_hObject, &vVel);

			VEC_COPY(vTemp, vDir);
			VEC_NORM(vTemp);

			if (m_fMass <= 0.0f)
			{
				m_fMass = 100.0f;
			}

            LTFLOAT fMultiplier = fDamage * (INFINITE_MASS/(m_fMass*5.0f));

			// If not on ground, scale down velocity...

			CollisionInfo standingInfo;
            g_pLTServer->GetStandingOn(m_hObject, &standingInfo);

			if (!standingInfo.m_hObject)
			{
				fMultiplier /= 10.0f;
			}

			VEC_MULSCALAR(vTemp, vTemp, fMultiplier);
			VEC_ADD(vImpactVel, vTemp, vVel);

            g_pLTServer->SetVelocity(m_hObject, &vImpactVel);
		}


		// Basically armor just absorbs as much of the damage
		// as possible...

		if (m_fArmorPoints > 0.0 && ArmorCanAbsorb(eType))
		{
			LTFLOAT fAdjustedArmor = (m_fArmorPoints * fArmorAdjustFactor);
			fAbsorb = fDamage > fAdjustedArmor ? fAdjustedArmor : fDamage;
			fDamage	-= fAbsorb;

			if (fDamage < 0.0)
			{
				fDamage = 0.0f;
			}
		}
	}


	m_fLastArmorAbsorb	= fAbsorb;
	m_fLastDamage		= fDamage;
	m_eLastDamageType	= eType;
	m_vLastDamageDir	= vDir;
	SetLastDamager(hDamager);


	// Actually apply the damage

	if (!m_bCanDamage || DebugDamageOn())
	{
		return;
	}
	else
	{
		m_fArmorPoints -= (fAbsorb/fArmorAdjustFactor);

		if (m_fArmorPoints < 0.0f)
		{
			m_fArmorPoints = 0.0f;
		}

		m_fHitPoints -= fDamage;

		if ( m_nDamageTriggerCounter > 0 )
		{
			m_nDamageTriggerCounter--;
		}
	}


	// If this is supposed to send a damage trigger, send it now...

	if (m_nDamageTriggerCounter <= 0)
	{
		if (m_hstrDamageTriggerTarget && m_hstrDamageTriggerMessage && m_nDamageTriggerNumSends != 0)
		{
			m_nDamageTriggerNumSends--;
			SendTriggerMsgToObjects(pObject, m_hstrDamageTriggerTarget, m_hstrDamageTriggerMessage);
		}
	}

	// Send message to object that damaged us...

	if (hDamager && m_hstrDamagerMessage)
	{
		SendTriggerMsgToObject(pObject, hDamager, m_hstrDamagerMessage);
	}


	// See if we're dead...

	if (m_fHitPoints <= 1.0f && !m_bNeverDestroy)
	{
		m_fHitPoints	= 0.0f;
		m_fDeathDamage	= fDamage;
		m_eDeathType	= eType;
		m_vDeathDir		= vDir;

		HandleDestruction(hDamager);

		// Update the number of kills done by this player if I'm
		// a character...

		if (IsPlayer(hDamager) && IsCharacter(m_hObject))
		{
            CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
            CCharacter* pTarget = (CCharacter*) g_pLTServer->HandleToObject(m_hObject);
			if (pPlayer)
			{
				CharacterSide cs = GetSide(pPlayer->GetCharacterClass(),pTarget->GetCharacterClass());
				switch (cs)
				{
				case CS_ENEMY:
					pPlayer->GetPlayerSummaryMgr()->IncNumEnemyKills();
					break;
				case CS_FRIEND:
					pPlayer->GetPlayerSummaryMgr()->IncNumFriendKills();
					break;
				}
            }
		}
		if (!IsMultiplayerGame() && IsCharacter(hDamager) && IsCharacter(m_hObject))
		{
            CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
            CCharacter* pTarget = (CCharacter*) g_pLTServer->HandleToObject(m_hObject);
			if (pPlayer)
			{
				CharacterSide cs = GetSide(pPlayer->GetCharacterClass(),pTarget->GetCharacterClass());
				if (cs == CS_NEUTRAL)
				{
					pPlayer->GetPlayerSummaryMgr()->IncNumNeutralKills();
				}
            }
		}


		if (m_bIsPlayer)
		{
			ProcessPlayerDeath(hDamager);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::SetLastDamager
//
//	PURPOSE:	Save our last damager
//
// ----------------------------------------------------------------------- //

void CDestructible::SetLastDamager(HOBJECT hDamager)
{
	// Save info on the guy who hit us...

	if (m_hLastDamager != hDamager)
	{
		if (m_hLastDamager)
		{
            g_pLTServer->BreakInterObjectLink(m_hObject, m_hLastDamager);
		}

		m_hLastDamager = hDamager;

		if (m_hLastDamager)
		{
            g_pLTServer->CreateInterObjectLink(m_hObject, m_hLastDamager);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ProcessPlayerDeath
//
//	PURPOSE:	Process the death of a player
//
// ----------------------------------------------------------------------- //

void CDestructible::ProcessPlayerDeath(HOBJECT hDamager)
{
	// Stop player movement if dead...

    LTVector vZero;
	VEC_INIT(vZero);
    g_pLTServer->SetVelocity(m_hObject, &vZero);
    g_pLTServer->SetAcceleration(m_hObject, &vZero);

	// If this was a player-player frag, send a message to all clients

    CPlayerObj* pVictim = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
    CPlayerObj* pKiller = (IsPlayer(hDamager)) ? ((CPlayerObj*) g_pLTServer->HandleToObject(hDamager)) : (pVictim);

	if (pVictim && pKiller)
	{
		if (pVictim == pKiller)
		{
			pKiller->DecFragCount();
		}
		else
		{
			// If we killed a team mate, deduct a frag (this should only
			// happen if friendly fire is enabled)...
			if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && 
				(pVictim->GetTeamID() == pKiller->GetTeamID()))
			{
				pKiller->DecFragCount();
			}
			else
			{
				pKiller->IncFragCount();
			}
		}

		if (g_pGameServerShell)
		{
			g_pGameServerShell->SetUpdateGameServ();
		}

		uint8 nLivesLeft = 255;
		if (g_pGameServerShell->GetGameType() == COOPERATIVE_ASSAULT && g_NetUseSpawnLimit.GetFloat() > 0.0f)
		{
			nLivesLeft = (uint8)g_NetSpawnLimit.GetFloat() - (uint8)pVictim->GetRespawnCount();
		}


		HCLIENT hClientVictim = pVictim->GetClient();
		HCLIENT hClientKiller = pKiller->GetClient();
		if (hClientVictim && hClientKiller)
		{
            uint32 nVictimID = g_pLTServer->GetClientID (hClientVictim);
            uint32 nKillerID = g_pLTServer->GetClientID (hClientKiller);

            HMESSAGEWRITE hWrite = g_pLTServer->StartMessage(LTNULL, MID_PLAYER_FRAGGED);
            g_pLTServer->WriteToMessageFloat(hWrite, (float)nVictimID);
            g_pLTServer->WriteToMessageFloat(hWrite, (float)nKillerID);
            g_pLTServer->WriteToMessageByte(hWrite, nLivesLeft);
            g_pLTServer->EndMessage(hWrite);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ProcessGear
//
//	PURPOSE:	Adjust damage according to what gear items we have
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::ProcessGear(LTFLOAT fDamage, DamageType eDamageType, HOBJECT hDamager, LTVector* pvDir)
{
	// Hold on to original damage

    LTFLOAT fOriginalDamage = fDamage;

	for (int nGearId=0; nGearId < MAX_GEAR_ITEMS; nGearId++)
	{
		if (m_bGearOwned[nGearId])
		{
			GEAR* pGear = g_pWeaponMgr->GetGear(nGearId);
			if (pGear && eDamageType == pGear->eProtectionType)
			{
				fDamage *= (1.0f - pGear->fProtection);
			}
		}
	}


	return fDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDestruction
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDestruction(HOBJECT hDamager)
{
	if (m_bDead) return;

    m_bDead      = LTTRUE;
	m_fHitPoints = 0.0;

    LPBASECLASS pD = g_pLTServer->HandleToObject(m_hObject);

	if (m_hstrDeathTriggerTarget && m_hstrDeathTriggerMessage)
	{
		SendTriggerMsgToObjects(pD, m_hstrDeathTriggerTarget, m_hstrDeathTriggerMessage);
	}

	if (m_hstrPlayerDeathTriggerTarget && m_hstrPlayerDeathTriggerMessage && IsPlayer(hDamager))
	{
		SendTriggerMsgToObjects(pD, m_hstrPlayerDeathTriggerTarget, m_hstrPlayerDeathTriggerMessage);
	}

	// Send message to object that killed us...

	if(hDamager && m_hstrKillerMessage)
	{
		SendTriggerMsgToObject(pD, hDamager, m_hstrKillerMessage);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleHeal
//
//	PURPOSE:	Handle heal message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleHeal(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	if (m_bDead) return;

    LTFLOAT fAmount = g_pLTServer->ReadFromMessageFloat(hRead) * g_HealScale.GetFloat(1.0f);

	// See if we can heal

	if (Heal(fAmount))
	{
        HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
        g_pLTServer->WriteToMessageFloat(hWrite, -1.0f);
        g_pLTServer->EndMessage(hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleAddGear
//
//	PURPOSE:	Handle add gear message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleAddGear(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	if (m_bDead) return;

    uint8 nGearId = g_pLTServer->ReadFromMessageByte(hRead);

 	if (AddGear(nGearId))
	{
        HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
        g_pLTServer->WriteToMessageFloat(hWrite, -1.0f);
        g_pLTServer->EndMessage(hWrite);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::AddGear
//
//	PURPOSE:	Add the specified gear to the owned list
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::AddGear(uint8 nGearId)
{
	// This may change after the new item is added...

    LTFLOAT fOldStealth = GetStealthModifier();

    LTBOOL bGearAdded = LTFALSE;
	if (m_bIsPlayer && nGearId != WMGR_INVALID_ID)
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			if (nGearId >= 0 && nGearId < MAX_GEAR_ITEMS)
			{
				if (!m_bGearOwned[nGearId])
				{
					GEAR* pGear = g_pWeaponMgr->GetGear(nGearId);

					if (pGear && pGear->bExclusive)
					{
                        m_bGearOwned[nGearId] = LTTRUE;
					}

					if (pGear->fArmor)
					{
						bGearAdded = Repair(pGear->fArmor);

						// Give the player health if necessary...
						if (IsMultiplayerGame() && g_NetArmorHealthPercent.GetFloat() > 0.0f)
						{
							LTFLOAT fHeal = g_NetArmorHealthPercent.GetFloat() * pGear->fArmor;
							Heal(fHeal);
						}
					}
					else
					{
						bGearAdded = LTTRUE;
					}
				}
			}
			else
			{
                g_pLTServer->CPrint("Gear id out of range! : %s", nGearId);
			}
		}
	}


	// If our stealth modifier has changed, notify the clients...

	if (fOldStealth != GetStealthModifier())
	{
		if (IsCharacter(m_hObject))
		{
            CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hObject);
			if (pChar)
			{
				pChar->SendStealthToClients();
			}
		}
	}

	return bGearAdded;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Heal
//
//	PURPOSE:	Add some value to hit points
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::Heal(LTFLOAT fAmount)
{
    LTFLOAT fMax = GetMaxHitPoints();
    if (m_fHitPoints >= fMax) return LTFALSE;

	// only heal what we need to (cap hit points at maximum)

	if (m_fHitPoints + fAmount > fMax)
	{
		fAmount = fMax - m_fHitPoints;
	}

	// now actually heal the object

	DoActualHealing (fAmount);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DoActualHealing()
//
//	PURPOSE:	Simply adds a value to the hit points variable
//
// ----------------------------------------------------------------------- //

void CDestructible::DoActualHealing(LTFLOAT fAmount)
{
	// NOTE: This function should only be called directly from the ultra heal
	//		 powerup code, as it does not do any bounds checking on the hit points.
	//       All other healing should be done through the HandleHeal() function above.

	if(!m_bCanHeal) return;

    m_bDead = LTFALSE;

	m_fHitPoints += fAmount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleRepair()
//
//	PURPOSE:	Handle Repair message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleRepair(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
    LTFLOAT fAmount = g_pLTServer->ReadFromMessageFloat(hRead);

	if (Repair(fAmount))
	{
        HMESSAGEWRITE hWrite = g_pLTServer->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
        g_pLTServer->WriteToMessageFloat(hWrite, -1.0f);
        g_pLTServer->EndMessage(hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Repair()
//
//	PURPOSE:	Add some value to armor
//
// ----------------------------------------------------------------------- //
LTBOOL CDestructible::Repair(LTFLOAT fAmount)
{
    if (!m_bCanRepair) return LTFALSE;

    LTFLOAT fMax = GetMaxArmorPoints();
    if (m_fArmorPoints >= fMax) return LTFALSE;

	m_fArmorPoints += fAmount;

	if (m_fArmorPoints > fMax)
	{
		m_fArmorPoints = fMax;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Reset
//
//	PURPOSE:	Reset
//
// ----------------------------------------------------------------------- //

void CDestructible::Reset(LTFLOAT fHitPts, LTFLOAT fArmorPts)
{
    LTFLOAT fMaxHitPoints = GetMaxHitPoints();
    LTFLOAT fMaxArmorPoints = GetMaxArmorPoints();

	m_fHitPoints	= fHitPts <= fMaxHitPoints ? fHitPts : fMaxHitPoints;
	m_fArmorPoints  = fArmorPts <= fMaxArmorPoints ? fArmorPts : fMaxArmorPoints;
	m_eDeathType	= DT_UNSPECIFIED;
	m_fDeathDamage	= 0.0f;
    m_bDead         = LTFALSE;

	m_eLastDamageType = DT_UNSPECIFIED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	if (m_bDead) return;

	const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

	// See if we should destroy ourself...

    if (_stricmp(TRIGGER_MSG_DESTROY, szMsg) == 0)
	{
		DamageStruct damage;

		damage.fDamage	= damage.kInfiniteDamage;
		damage.hDamager = hSender;

		damage.DoDamage(pObject, pObject->m_hObject);
	}

	// See if we should reset ourself...

    if (_stricmp(TRIGGER_MSG_RESET, szMsg) == 0)
	{
		Reset(GetMaxHitPoints(), GetMaxArmorPoints());
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleCrush
//
//	PURPOSE:	Handle crush message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleCrush(LPBASECLASS pObject, HOBJECT hSender)
{
	if (!pObject || !hSender || m_bDead) return;

	// Don't do crush damage...

	return;


    LTFLOAT fMyMass, fHisMass;
    fMyMass  = g_pLTServer->GetObjectMass(m_hObject);
    fHisMass = g_pLTServer->GetObjectMass(hSender);

	// If we're within 15% of each other, don't do anything rash...

	if (fHisMass <= fMyMass*1.15) return;

    uint32 dwUsrFlg = g_pLTServer->GetObjectUserFlags(hSender);
	if (dwUsrFlg & USRFLG_CANT_CRUSH) return;

    LTFLOAT fDamage = 20.0f;

	DamageStruct damage;

	damage.eType	= DT_CRUSH;
	damage.fDamage	= fDamage;
	damage.hDamager = hSender;

	damage.DoDamage(pObject, pObject->m_hObject);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleTouch
//
//	PURPOSE:	Handle object contact
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleTouch(LPBASECLASS pObject, HOBJECT hSender, LTFLOAT fForce)
{
	if (!pObject || !hSender || m_bDead || fForce < 0.0f) return;

	// Don't do touch damage...

	return;


    LTFLOAT fMyMass, fHisMass;
    fMyMass  = g_pLTServer->GetObjectMass(pObject->m_hObject);
    fHisMass = g_pLTServer->GetObjectMass(hSender);

    LTBOOL bIsWorld = IsMainWorld(hSender);

	if (bIsWorld)
	{
		fHisMass = INFINITE_MASS;

		// Don't allow the world to damage the player...

		if (IsPlayer(pObject->m_hObject)) return;
	}

    uint32 dwUsrFlg = g_pLTServer->GetObjectUserFlags(hSender);
	if (dwUsrFlg & USRFLG_CANT_CRUSH) return;

	if (fHisMass <= 0.1f || fMyMass <= 0.1f) return;

	// If we're within 15% of each other, don't do anything rash...

	if (fHisMass <= fMyMass*1.15 || fMyMass > fHisMass) return;


	// Not enough force to do anything...

    LTFLOAT fVal = bIsWorld ? fMyMass * 15.0f : fMyMass * 10.0f;
	if (fForce < fVal) return;


	// Okay, one last check.  Make sure that he is moving, if I run into him
	// that shouldn't cause anything to happen.  It is only when he runs into
	// me that I can take damage...(well, unless it is the world...

	if (!bIsWorld)
	{
        LTVector vVel;
        g_pLTServer->GetVelocity(hSender, &vVel);
		if (VEC_MAG(vVel) < 1.0f) return;
	}


	// Calculate damage...

    LTFLOAT fDamage = (fForce / fMyMass);
	fDamage *= bIsWorld ? 5.0f : 10.0f;

    //g_pLTServer->BPrint("%.2f, %.2f, %.2f, %.2f",
	//				  fHisMass, fMyMass, fForce, fDamage);

    LTVector vDir;
	VEC_INIT(vDir);

	if (bIsWorld)
	{
        g_pLTServer->GetVelocity(pObject->m_hObject, &vDir);
		VEC_NEGATE(vDir, vDir);
	}
	else
	{
        LTVector vMyPos, vHisPos;
        g_pLTServer->GetObjectPos(pObject->m_hObject, &vMyPos);
        g_pLTServer->GetObjectPos(hSender, &vHisPos);

		VEC_SUB(vDir, vMyPos, vHisPos);
	}

	vDir.Norm();

	DamageStruct damage;

	damage.eType	= DT_CRUSH;
	damage.fDamage	= fDamage;
	damage.hDamager = hSender;
	damage.vDir		= vDir;

	damage.DoDamage(pObject, pObject->m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DebugDamageOn
//
//	PURPOSE:	See if the object can be damaged
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::DebugDamageOn()
{
    if (!m_hObject) return LTFALSE;

    LTBOOL bRet = LTFALSE;

    HCONVAR hVar  = g_pLTServer->GetGameConVar("SetDamage");
    char* pVal = g_pLTServer->GetVarValueString(hVar);

    if (!pVal) return LTFALSE;

    return ((LTBOOL) !atoi(pVal));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Save(HMESSAGEWRITE hWrite, uint8 nType)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hObject);
    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hLastDamager);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fHitPoints);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxHitPoints);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fArmorPoints);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMaxArmorPoints);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMass);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fLastDamage);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fLastArmorAbsorb);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fDeathDamage);
    g_pLTServer->WriteToMessageByte(hWrite, m_bDead);
    g_pLTServer->WriteToMessageByte(hWrite, m_bApplyDamagePhysics);
    g_pLTServer->WriteToMessageByte(hWrite, m_bIsPlayer);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanHeal);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanRepair);
    g_pLTServer->WriteToMessageByte(hWrite, m_bCanDamage);
    g_pLTServer->WriteToMessageByte(hWrite, m_bNeverDestroy);
    g_pLTServer->WriteToMessageByte(hWrite, m_eDeathType);
    g_pLTServer->WriteToMessageByte(hWrite, m_eLastDamageType);
    g_pLTServer->WriteToMessageDWord(hWrite, m_nDamageTriggerCounter);
    g_pLTServer->WriteToMessageDWord(hWrite, m_nDamageTriggerNumSends);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwCantDamageTypes);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDamageTriggerTarget);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDamageTriggerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDamagerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDeathTriggerTarget);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDeathTriggerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPlayerDeathTriggerTarget);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPlayerDeathTriggerMessage);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrKillerMessage);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vDeathDir);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vLastDamageDir);

    int i;
    for (i=0; i < MAX_GEAR_ITEMS; i++)
	{
        g_pLTServer->WriteToMessageByte(hWrite, m_bGearOwned[i]);
	}

	for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].Save(hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Load(HMESSAGEREAD hRead, uint8 nType)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hLastDamager);

    m_fHitPoints                = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMaxHitPoints             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fArmorPoints              = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMaxArmorPoints           = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMass                     = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fLastDamage               = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fLastArmorAbsorb          = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fDeathDamage              = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bDead                     = g_pLTServer->ReadFromMessageByte(hRead);
    m_bApplyDamagePhysics       = g_pLTServer->ReadFromMessageByte(hRead);
    m_bIsPlayer                 = g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanHeal                  = g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanRepair                = g_pLTServer->ReadFromMessageByte(hRead);
    m_bCanDamage                = g_pLTServer->ReadFromMessageByte(hRead);
    m_bNeverDestroy             = g_pLTServer->ReadFromMessageByte(hRead);
    m_eDeathType                = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_eLastDamageType           = (DamageType) g_pLTServer->ReadFromMessageByte(hRead);
    m_nDamageTriggerCounter		= g_pLTServer->ReadFromMessageDWord(hRead);
    m_nDamageTriggerNumSends    = g_pLTServer->ReadFromMessageDWord(hRead);
    m_dwCantDamageTypes         = g_pLTServer->ReadFromMessageDWord(hRead);
    m_hstrDamageTriggerTarget   = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDamageTriggerMessage  = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDamagerMessage        = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDeathTriggerTarget    = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDeathTriggerMessage   = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrPlayerDeathTriggerTarget  = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrPlayerDeathTriggerMessage = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrKillerMessage         = g_pLTServer->ReadFromMessageHString(hRead);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vDeathDir);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vLastDamageDir);

    int i;
    for (i=0; i < MAX_GEAR_ITEMS; i++)
	{
        m_bGearOwned[i] = g_pLTServer->ReadFromMessageByte(hRead);
	}

	for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].Load(hRead);
	}
}