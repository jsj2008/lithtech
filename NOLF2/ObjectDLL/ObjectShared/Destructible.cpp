// ----------------------------------------------------------------------- //
//
// MODULE  : Destructible.cpp
//
// PURPOSE : Destructible class - Implementation
//
// CREATED : 9/23/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "AIUtils.h"
#include "PlayerButes.h"
#include "AI.h"
#include "ServerMissionMgr.h"
#include "DoomsDayDevice.h"

#define TRIGGER_MSG_DESTROY	"DESTROY"
#define TRIGGER_MSG_RESET	"RESET"

#define MAXIMUM_BLOCK_PRIORITY	255.0f
#define MINIMUM_FRICTION		5.0f
#define MAXIMUM_FRICTION		15.0f
#define MINIMUM_FORCE			0.0f

CVarTrack g_vtAIDamageAdjustEasy;
CVarTrack g_vtAIDamageAdjustNormal;
CVarTrack g_vtAIDamageAdjustHard;
CVarTrack g_vtAIDamageAdjustVeryHard;
CVarTrack g_vtAIDamageIncreasePerPlayer;
CVarTrack g_vtPlayerDamageDecreasePerPlayer;

extern CGameServerShell* g_pGameServerShell;
extern CVarTrack g_DamageScale;
extern CVarTrack g_HealScale;
const LTFLOAT DamageStruct::kInfiniteDamage = 100000.0f; // Infinite damage


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageStruct::InitFromMessage
//
//	PURPOSE:	Initialize from a MID_DAMAGE message
//
// ----------------------------------------------------------------------- //

LTBOOL DamageStruct::InitFromMessage(ILTMessage_Read *pMsg)
{
    if (!pMsg) return LTFALSE;
	
	uint32 dwPos = pMsg->Tell();

	vDir = pMsg->ReadLTVector();
    fDuration  = pMsg->Readfloat();
    fDamage    = pMsg->Readfloat() * g_DamageScale.GetFloat(1.0f);
    eType      = (DamageType)pMsg->Readuint8();
    hDamager   = pMsg->ReadObject();
    hContainer = pMsg->ReadObject();
    nAmmoId    = pMsg->Readuint8();

	pMsg->SeekTo( dwPos );

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

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_DAMAGE);
	cMsg.WriteLTVector(vDir);
	cMsg.Writefloat(fDuration);
	cMsg.Writefloat(fDamage);
	cMsg.Writeuint8(eType);
	cMsg.WriteObject(hDamager);
	cMsg.WriteObject(hContainer);
	cMsg.Writeuint8(nAmmoId);
	g_pLTServer->SendToObject(cMsg.Read(), pDamager->m_hObject, hVictim, MESSAGE_GUARANTEED);

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
	m_fEnergy					= 1.0f;
	m_fMaxEnergy				= 1.0f;
	m_fMaxArmorPoints			= 1.0f;
	m_fDeathDamage				= 0.0f;
	m_fLastDamage				= 0.0f;
	m_fLastArmorAbsorb			= 0.0f;
	m_nCantDamageFlags			= DamageTypeToFlag(DT_GADGET_DECAYPOWDER);
	m_eLastDamageType			= DT_UNSPECIFIED;
	m_eDeathType				= DT_UNSPECIFIED;

	VEC_INIT(m_vDeathDir);
	VEC_INIT(m_vLastDamageDir);

	m_fDamagePercentCommand			= 0.0f;
	m_nDamageTriggerCounter			= 0;
	m_nDamageTriggerNumSends		= 1;

    m_bCanHeal                  = LTTRUE;
    m_bCanRepair                = LTTRUE;
    m_bCanDamage                = LTTRUE;
    m_bNeverDestroy             = LTFALSE;

	// damage filter function
    m_pDamageFilterFunction = 0;
    m_pDamageFilterObject = 0;

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

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint8)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint8)fData);
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

uint32 CDestructible::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(pObject, hSender, pMsg);
		}
		break;

		case MID_DAMAGE:
		{
			HandleDamage(pObject, hSender, pMsg);
		}
		break;

		case MID_REPAIR:
		{
			if (m_bCanRepair)
			{
				HandleRepair(pObject, hSender, pMsg);
			}
		}
		break;

		case MID_HEAL:
		{
			if (m_bCanHeal)
			{
				HandleHeal(pObject, hSender, pMsg);
			}
		}
		break;

		case MID_ADDGEAR:
		{
			HandleAddGear(pObject, hSender, pMsg);
		}
		break;

		default : break;
	}

    return IAggregate::ObjectMessageFn(pObject, hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pStruct)
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

	if (g_pLTServer->GetPropGeneric("Energy", &genProp) == LT_OK)
	{
		SetEnergy(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxEnergy", &genProp) == LT_OK)
	{
		SetMaxEnergy(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("Armor", &genProp) == LT_OK)
	{
		SetArmorPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("MaxArmor", &genProp) == LT_OK)
	{
		SetMaxArmorPoints(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("DamagePercentForCommand", &genProp) == LT_OK)
	{
		SetDamagePercentCommand(genProp.m_Float);
	}
   
    if (g_pLTServer->GetPropGeneric("DamageCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sDamageCommand = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("PlayerDamageCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sPlayerDamageCommand = genProp.m_String;
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
            m_sDamagerMessage = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("DamagePercentCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sDamagePercentCommand = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("DeathCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
           	m_sDeathCommand = genProp.m_String;
		}
	}
    
	if (g_pLTServer->GetPropGeneric("PlayerDeathCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sPlayerDeathCommand = genProp.m_String;
		}
	}

    if (g_pLTServer->GetPropGeneric("KillerMessage", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_sKillerMessage = genProp.m_String;
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

	// Set the mass, which will also set the blocking priority and friction coefficient...

	SetMass( m_fMass );
	
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
//	PURPOSE:	Remove all, or a specific DamageType, progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamage( DamageType DT /*= DT_INVALID*/, bool bUpdateProgressiveDamage /*= true*/ )
{
	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if( (DT == DT_INVALID) || (DT == m_ProgressiveDamage[i].eType) )
		{
			m_ProgressiveDamage[i].fDuration = 0.0f;
		}
	}
	
	if ( bUpdateProgressiveDamage )
	{
		UpdateProgressiveDamage();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::ClearProgressiveDamage
//
//	PURPOSE:	Remove all, or a specific DamageType, progressive damage being done to the object.
//
// ----------------------------------------------------------------------- //

void CDestructible::ClearProgressiveDamage( DamageFlags nDamageFlag )
{
	for ( DamageType i = static_cast< DamageType >( DT_INVALID + 1 );
	      i < kNumDamageTypes;
	      i = static_cast< DamageType >( i + 1 ) )
	{
		if ( DamageTypeToFlag( i ) & nDamageFlag )
		{
			ClearProgressiveDamage( i, false );
		}
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

	DamageFlags nActiveDamageFlags = 0;
	bool bBleeding = false;

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

			// Add this to the active damageflags.
			nActiveDamageFlags |= DamageTypeToFlag( damage.eType );

			bBleeding |= ( damage.eType == DT_BLEEDING );
		}
	}


	// See if we have protection from bleeding damage...

	if (bBleeding)
	{
		LTFLOAT fProtection = 0.0f;

		GEAR const *pGear = LTNULL;

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
						nActiveDamageFlags &= ~DamageTypeToFlag( DT_BLEEDING );
						break;
					}
				}
			}
		}
	}


	// Set our damage flags if we're bleeding/poisoned/stunned/sleeping...
	// [RP] 10/30/02 - Don't set damage flags if we can't be damaged...

	if( m_bIsPlayer && m_bCanDamage )
	{
		// Get the character...

		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
		if( !pPlayer ) return;

        pPlayer->SetDamageFlags( nActiveDamageFlags );
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

	// No since adding progressive damage if we are already dead...

	if( m_bDead )
		return;

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
				return;
			}
		}
	}

	if (IsScoreTagType(damage.eType) && IsPlayer(hDamager) && m_bIsPlayer && !IsTakingProgressiveDamage(damage.eType))
	{
		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*> (g_pLTServer->HandleToObject(hDamager));
		CPlayerObj* pTarget = dynamic_cast<CPlayerObj*> (g_pLTServer->HandleToObject(m_hObject));

		if (hDamager == m_hObject)
		{
			pPlayer->GetPlayerScore()->RemoveTag();
		}
		else
		{
			if( IsTeamGameType() )
			{
				if( pPlayer->GetTeamID() == pTarget->GetTeamID() )
				{
					// The player is either stupid or a jack-ass so penalize him and the team...
				
					pPlayer->GetPlayerScore()->RemoveTag();
				}
				else
				{
					// The player tagged another player of a different team so this is valid...

					pPlayer->GetPlayerScore()->AddTag();
				}	
			}
			else
			{
				pPlayer->GetPlayerScore()->AddTag();
			}
		}

		HCLIENT hClientTarget = pTarget->GetClient();
		HCLIENT hClientPlayer = pPlayer->GetClient();
		if (hClientTarget)
		{
			uint32 nTargetID = g_pLTServer->GetClientID (hClientTarget);
			uint32 nPlayerID = g_pLTServer->GetClientID (hClientPlayer);

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_SCORED);
			cMsg.Writebool(false);
			cMsg.Writeuint32(nTargetID);
			cMsg.Writeuint32(nPlayerID);
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
		}

		if( g_pGameServerShell )
		{
			g_pGameServerShell->SetUpdateGameServ();
		}
	}




    for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if (m_ProgressiveDamage[i].fDuration <= 0.0f)
		{
			m_ProgressiveDamage[i] = damage;
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
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::IsTakingProgressiveDamage
//
//	PURPOSE:	returns TRUE if currently affected by specified type of progressive damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CDestructible::IsTakingProgressiveDamage( DamageType DT )
{
	for (int i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		if( ( DT == m_ProgressiveDamage[i].eType ) && 
			( m_ProgressiveDamage[i].fDuration > 0.0f ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
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
	// Make sure we don't try and go above infinite mass as that will mess with the blocking priority...

	m_fMass = Clamp( fMass, 0.0f, INFINITE_MASS - 1.0f );

	SetBlockingPriority();

	// Set the friction based on the mass of the object...

	if (!m_hObject) return;

    g_pPhysicsLT->SetMass(m_hObject, m_fMass);

    LTFLOAT fFricCoeff = MINIMUM_FRICTION + (m_fMass * MAXIMUM_FRICTION / INFINITE_MASS);
	g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fFricCoeff);
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
    g_pPhysicsLT->SetForceIgnoreLimit(m_hObject, MINIMUM_FORCE);

// BEP 7/9/02 Why is this setting touch notify??  Removed.
//	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY);
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
//	ROUTINE:	CDestructible::GetMaxEnergy
//
//	PURPOSE:	Returns the maximum energy points for this object
//
// ----------------------------------------------------------------------- //

LTFLOAT CDestructible::GetMaxEnergy() const
{
    LTFLOAT fAdjustedMax = m_fMaxEnergy;

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
    GEAR const *pGear = LTNULL;

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

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			fMult *= pPlayer->GetPlayerSkills()->GetSkillModifier(SKL_STEALTH,StealthModifiers::eRadius);
		}
	}

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
    GEAR const *pGear = LTNULL;

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
//	ROUTINE:	CDestructible::RegisterFilterFunction
//
//	PURPOSE:	Register the filter function
//
// ----------------------------------------------------------------------- //

LTBOOL  CDestructible::RegisterFilterFunction( DamageFilterFunction pFn, GameBase *pObject )
{
	if ( ( 0 == pFn ) || ( 0 == pObject ) )
	{
		// if one parameter is non-null, the both must be nonnull
		ASSERT( 0 != pFn );
		ASSERT( 0 != pObject );
	}

	m_pDamageFilterFunction = pFn;
	m_pDamageFilterObject = pObject;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleDamage
//
//	PURPOSE:	Handle damage message
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleDamage(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	DamageStruct damage;
	damage.InitFromMessage(pMsg);

	// Send things through the filer function, if any...
	if( m_pDamageFilterFunction )
	{
		bool bResult = m_pDamageFilterFunction( m_pDamageFilterObject, &damage );
		if ( !bResult )
		{
			// the filter return false, which means to disregard
			// processing this damage
			return;
		}
	}

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
    LTVector vDir		= damage.vDir;
    LTFLOAT fDamage		= damage.fDamage;
	DamageType eType	= damage.eType;
	HOBJECT hDamager	= damage.hDamager;

    LPBASECLASS pObject = g_pLTServer->HandleToObject(m_hObject);

	// See if we should actually process damage...

	if (m_bDead) 
		return;


	// See if this type of damage applies to us...

	if (m_nCantDamageFlags)
	{
		if (m_nCantDamageFlags & DamageTypeToFlag(eType)) 
			return;
	}
	

	// If the damage is only for worldmodels make sure we are a world model...

	if ( eType == DT_WORLDONLY && !IsWorldModel( m_hObject ) )
		return;
	

	// Adjust the damage based on difficulty, who I am and who I damaged...

	fDamage = AdjustDamage(fDamage, hDamager);
	

	// Process damage through any gear items...

	fDamage = ProcessGear(fDamage, eType, hDamager, &vDir);


    LTFLOAT fAbsorb = 0.0f;

	// Do rag-doll, and adjust damage if it was greater than 0...(if the damage
	// was zero, we still want to process it)...

	if (fDamage > 0.0f)
	{
		// Rag-doll time...

		if (CanRagDoll(vDir, eType))
		{
            LTVector vImpactVel, vTemp, vVel;

            g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

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

            g_pPhysicsLT->SetVelocity(m_hObject, &vImpactVel);
		}


		// Basically armor just absorbs as much of the damage
		// as possible...

		if (m_fArmorPoints > 0.0 && ArmorCanAbsorb(eType))
		{
			LTFLOAT fAdjustedArmor = m_fArmorPoints;
			fAbsorb = (fDamage > fAdjustedArmor ? fAdjustedArmor : fDamage);
			fDamage	-= fAbsorb;
		}
	}

	// [KLS 7/15/02] Negative damage will actually give us hit points, so make sure
	// we don't do that ;)
	if (fDamage < 0.0)
	{
		fDamage = 0.0f;
	}

	// [KLS 7/15/02] If we're a player, now that we have processed gear and armor, 
	// adjust the actual damage done based on our skill level...

	if (m_bIsPlayer)
	{
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hObject);
		if (pPlayer)
		{
			fDamage *= pPlayer->GetPlayerSkills()->GetSkillModifier(SKL_STAMINA,StaminaModifiers::eDamage);
		}
	}


	// Save these for later...

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
		m_fArmorPoints -= fAbsorb;

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
		if (m_nDamageTriggerNumSends != 0)
		{
			m_nDamageTriggerNumSends--;

			if(m_sDamageCommand.length() )
			{
				const char *pCmd = m_sDamageCommand.c_str();

				if( g_pCmdMgr->IsValidCmd( pCmd ) )
				{
					g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
				}
			}

			if( m_sPlayerDamageCommand.size( ) != 0 && IsPlayer( hDamager ))
			{
				const char *pCmd = m_sPlayerDamageCommand.c_str( );

				if( g_pCmdMgr->IsValidCmd( pCmd ) )
				{
					g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
				}
			}
		}
	}

	// If we ought to send a percentage based damage command, do it now.

	if ( m_sDamagePercentCommand.length() && (m_fHitPoints < m_fMaxHitPoints * m_fDamagePercentCommand) )
	{
		const char *pCmd = m_sDamagePercentCommand.c_str();

		if( g_pCmdMgr->IsValidCmd( pCmd ) )
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
	}
	
	// Send message to object that damaged us...

	if (hDamager && m_sDamagerMessage.length())
	{
		SendTriggerMsgToObject(pObject, hDamager, LTFALSE, m_sDamagerMessage.c_str());
	}
	
	// Send instant damage flags for characters...

	if( IsCharacter( m_hObject ) && !(damage.fDuration > 0.0f) )
	{
		CCharacter *pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( m_hObject ));
		if( pChar )
		{
			pChar->SetInstantDamageFlags( DamageTypeToFlag( eType ));
		}
	}


	// See if we're dead...

	if (m_fHitPoints <= 1.0f && !m_bNeverDestroy && ( fDamage > 0.f ) )
	{
//		m_fHitPoints	= 0.0f;
		m_fDeathDamage	= fDamage;
		m_eDeathType	= eType;
		m_vDeathDir		= vDir;

		HandleDestruction(hDamager);

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
	m_hLastDamager = hDamager;
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
	if (!m_bIsPlayer) return;

	// Stop player movement if dead...

    LTVector vZero;
	vZero.Init();
    g_pPhysicsLT->SetVelocity(m_hObject, &vZero);

    CPlayerObj* pVictim = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_hObject));
	if (!pVictim) return;

	pVictim->GetMissionStats()->dwNumTimesKilled++;

	bool bKillerIdentified = false;

	// If another player killed us, give them the score.
    CPlayerObj* pPlayerKiller = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hDamager ));
	if( pPlayerKiller )
	{
		bKillerIdentified = true;

		// Check if we killed ourselves.
		if (pVictim == pPlayerKiller)
		{
			//don't count it against them if they were switching teams
			if (!pVictim->RequestedTeamChange())
				pVictim->GetPlayerScore()->RemoveFrag();
		}
		else
		{
			if( IsTeamGameType() )
			{
				if( pPlayerKiller->GetTeamID() == pVictim->GetTeamID() )
				{
					// The player is either stupid or a jack-ass so penalize him and the team...
				
					pPlayerKiller->GetPlayerScore()->RemoveFrag();
				}
				else
				{
					// The player killed another player of a different team so this is valid...

					pPlayerKiller->GetPlayerScore()->AddFrag();
				}	
			}
			else
			{
				pPlayerKiller->GetPlayerScore()->AddFrag();
			}
		}
	}

	if( !bKillerIdentified )
	{
		// If the killer was identified, then then the scoring is handled elsewhere.
		DoomsDayDevice* pDoomsDayDevice = dynamic_cast< DoomsDayDevice* >( g_pLTServer->HandleToObject( hDamager ));
		if( pDoomsDayDevice )
		{
			bKillerIdentified = true;
		}
	}

	if( !bKillerIdentified )
	{
		// If the killer was not an AI, then assume we killed ourselves.
		CAI* pAIKiller = dynamic_cast< CAI* >( g_pLTServer->HandleToObject( hDamager ));
		if( pAIKiller )
		{
			bKillerIdentified = true;
		}
		else
		{
			// No killer, so they must have killed themselves, but don't count it against them
			//	if they were switching teams
			if (!pVictim->RequestedTeamChange())
				pVictim->GetPlayerScore()->RemoveFrag();
		}
	}


	if (g_pGameServerShell)
	{
		g_pGameServerShell->SetUpdateGameServ();
	}

	if (!pVictim->RequestedTeamChange())
	{
		HCLIENT hClientVictim = pVictim->GetClient();
		HCLIENT hClientKiller = ( pPlayerKiller ) ? pPlayerKiller->GetClient() : NULL;
		if (hClientVictim )
		{
			uint32 nVictimID = g_pLTServer->GetClientID (hClientVictim);
			uint32 nKillerID = ( hClientKiller ) ? g_pLTServer->GetClientID (hClientKiller) : -1;

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_SCORED);
			cMsg.Writebool( true );
			cMsg.Writeuint32(nVictimID);
			cMsg.Writeuint32(nKillerID);
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
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
	for (int nGearId=0; nGearId < MAX_GEAR_ITEMS; nGearId++)
	{
		if (m_bGearOwned[nGearId])
		{
			GEAR const *pGear = g_pWeaponMgr->GetGear(nGearId);
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

void CDestructible::HandleDestruction(HOBJECT hDamager, DamageType eDamageType)
{
	m_eLastDamageType = eDamageType;
	HandleDestruction( hDamager );
}

void CDestructible::HandleDestruction(HOBJECT hDamager)
{
	if (m_bDead) return;

    m_bDead      = LTTRUE;
//	m_fHitPoints = 0.0;

    LPBASECLASS pD = g_pLTServer->HandleToObject(m_hObject);

	if (m_sDeathCommand.length())
	{
		const char *pCmd = m_sDeathCommand.c_str();

		if( g_pCmdMgr->IsValidCmd( pCmd ) )
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
	}

	if (m_sPlayerDeathCommand.length() && IsPlayer(hDamager))
	{
		const char *pCmd = m_sPlayerDeathCommand.c_str();

		if( g_pCmdMgr->IsValidCmd( pCmd ) )
		{
			g_pCmdMgr->Process( pCmd, m_hObject, m_hObject );
		}
	}

	// Send message to object that killed us...

	if(hDamager && m_sKillerMessage.length())
	{
		SendTriggerMsgToObject(pD, hDamager, LTFALSE, m_sKillerMessage.c_str());
	}

	if (IsPlayer(hDamager) && IsCharacter(m_hObject) && !m_bIsPlayer)
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(m_hObject);
        CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
		if (pPlayer && pChar)
		{
			//TODO: verify alignment
			pPlayer->GetMissionStats()->dwNumEnemyKills++;
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleHeal
//
//	PURPOSE:	Handle heal message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleHeal(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (m_bDead) return;

    LTFLOAT fAmount = pMsg->Readfloat() * g_HealScale.GetFloat(1.0f);

	// See if we can heal

 	LTBOOL bHealed = Heal(fAmount);

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writeuint8((uint8)bHealed);
	g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::HandleAddGear
//
//	PURPOSE:	Handle add gear message.
//
// ----------------------------------------------------------------------- //

void CDestructible::HandleAddGear(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (m_bDead) return;

    uint8 nGearId = pMsg->Readuint8();

 	LTBOOL bAdded = AddGear(nGearId);

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writeuint8((uint8)bAdded);
	g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
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
	if( !m_bIsPlayer )
		return LTFALSE;
	
	if( nGearId == WMGR_INVALID_ID)
		return LTFALSE;

	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
	if( !pPlayer )
		return LTFALSE;

	if (nGearId >= 0 && nGearId < MAX_GEAR_ITEMS)
	{
		if (!m_bGearOwned[nGearId])
		{
			GEAR const *pGear = g_pWeaponMgr->GetGear(nGearId);
			if( pGear && !pGear->bServerRestricted )
			{
				if ( pGear->bExclusive)
				{
					m_bGearOwned[nGearId] = LTTRUE;
				}

				if (pGear->fArmor)
				{
					LTFLOAT fRepair = pGear->fArmor;
					fRepair *= pPlayer->GetPlayerSkills()->GetSkillModifier(SKL_ARMOR,ArmorModifiers::eArmorPickup);
					bGearAdded = Repair(fRepair);
				}
				else if (pGear->fHealth)
				{
					LTFLOAT fHealth = pGear->fHealth;
					fHealth *= pPlayer->GetPlayerSkills()->GetSkillModifier(SKL_STAMINA,StaminaModifiers::eHealthPickup);
					bGearAdded = Heal(fHealth);
				}
				else
				{
					bGearAdded = LTTRUE;
				}
			}
		}
	}
	else
	{
		g_pLTServer->CPrint("Gear id out of range! : %d", nGearId);
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

void CDestructible::HandleRepair(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
    LTFLOAT fAmount = pMsg->Readfloat();

 	LTBOOL bRepaired = Repair(fAmount);

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writeuint8((uint8)bRepaired);
	g_pLTServer->SendToObject(cMsg.Read(), pObject->m_hObject, hSender, MESSAGE_GUARANTEED);
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

void CDestructible::Reset(LTFLOAT fHitPts, LTFLOAT fArmorPts, LTFLOAT fEnergy)
{
    LTFLOAT fMaxHitPoints = GetMaxHitPoints();
    LTFLOAT fMaxArmorPoints = GetMaxArmorPoints();
	LTFLOAT fMaxEnergy = GetMaxEnergy();

	m_fHitPoints	= fHitPts <= fMaxHitPoints ? fHitPts : fMaxHitPoints;
	m_fArmorPoints  = fArmorPts <= fMaxArmorPoints ? fArmorPts : fMaxArmorPoints;
	m_fEnergy		= fEnergy <= fMaxEnergy ? fEnergy : fMaxEnergy;
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

void CDestructible::HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (m_bDead) return;

	const char* szMsg = (const char*)pMsg->Readuint32();
	if( !szMsg )
	{
		ASSERT( !"CDestructible::HandleTrigger:  Empty trigger message received." );
		return;
	}

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
		Reset(GetMaxHitPoints(), GetMaxArmorPoints(), GetMaxEnergy());
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
    g_pPhysicsLT->GetMass(m_hObject, &fMyMass);
    g_pPhysicsLT->GetMass(hSender, &fHisMass);

	// If we're within 15% of each other, don't do anything rash...

	if (fHisMass <= fMyMass*1.15) return;

    uint32 dwUsrFlg;
	g_pCommonLT->GetObjectFlags(hSender, OFT_User, dwUsrFlg);
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
    g_pPhysicsLT->GetMass(pObject->m_hObject, &fMyMass);
    g_pPhysicsLT->GetMass(hSender, &fHisMass);

    LTBOOL bIsWorld = IsMainWorld(hSender);

	if (bIsWorld)
	{
		fHisMass = INFINITE_MASS;

		// Don't allow the world to damage the player...

		if (IsPlayer(pObject->m_hObject)) return;
	}

    uint32 dwUsrFlg;
	g_pCommonLT->GetObjectFlags(hSender, OFT_User, dwUsrFlg);
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
		g_pPhysicsLT->GetVelocity(hSender, &vVel);
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
        g_pPhysicsLT->GetVelocity(pObject->m_hObject, &vDir);
		VEC_NEGATE(vDir, vDir);
	}
	else
	{
        LTVector vMyPos, vHisPos;
		g_pLTServer->GetObjectPos(pObject->m_hObject, &vMyPos);
        g_pLTServer->GetObjectPos(hSender, &vHisPos);

		VEC_SUB(vDir, vMyPos, vHisPos);
	}

	vDir.Normalize();

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
    const char* pVal = g_pLTServer->GetVarValueString(hVar);

    if (!pVal) return LTFALSE;

    return ((LTBOOL) !atoi(pVal));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::DebugDamageOn
//
//	PURPOSE:	See if the object can be damaged
//
// ----------------------------------------------------------------------- //

float CDestructible::AdjustDamage( float fDamage, HOBJECT hDamager )
{
	// Only do following adjustments for characters damaging characters...

	if( !IsCharacter( m_hObject ) || !IsCharacter( hDamager ) )
	{
		return fDamage;
	}

	// AI damaging a player...

	if( IsAI( hDamager ) && IsPlayer( m_hObject ))
	{

		float fDifficultyFactor = 1.0f;
    		
		// If we're an AI damage is based on the current difficutly setting...

		switch (g_pGameServerShell->GetDifficulty())
		{
			case GD_EASY:
				fDifficultyFactor = g_vtAIDamageAdjustEasy.GetFloat();
			break;

			case GD_NORMAL:
				fDifficultyFactor = g_vtAIDamageAdjustNormal.GetFloat();
			break;

			case GD_HARD:
				fDifficultyFactor = g_vtAIDamageAdjustHard.GetFloat();
			break;

			case GD_VERYHARD:
				fDifficultyFactor = g_vtAIDamageAdjustVeryHard.GetFloat();
			break;

			default :
			break;
		}

		// Adjust for the number of players...
		
		float fPlayerMod = 0.0f;
		uint32 nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );

		if( nPlayersInGame > 1 )
		{
			// Increase the difficulty by an amount per player, factoring out the first player...
			
			float fPlayerInc = g_vtAIDamageIncreasePerPlayer.GetFloat();
			fPlayerMod = fPlayerInc * (nPlayersInGame-1);
		}

		return fDamage * (fDifficultyFactor + fPlayerMod);
	}

	// Player damaging an AI

	if( IsPlayer( hDamager ) && IsAI( m_hObject ) )
	{
		// Leave the damage the way it was if in SP game...

		if( IsMultiplayerGame() )
		{
			uint32 nPlayersInGame = CPlayerObj::GetNumberPlayersWithClients( );
			if( nPlayersInGame > 1 )
			{
				float fDifficultyFactor = 1.0f;
				float fPlayerInc = g_vtPlayerDamageDecreasePerPlayer.GetFloat();
				float fPlayerMod = fPlayerInc * (nPlayersInGame-1);

				return fDamage * (fDifficultyFactor - fPlayerMod);
			}
		}
	}

	return fDamage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Save(ILTMessage_Write *pMsg, uint8 nType)
{
	if (!pMsg) return;

    SAVE_HOBJECT(m_hObject);
    SAVE_HOBJECT(m_hLastDamager);

	SAVE_FLOAT(m_fHitPoints);
    SAVE_FLOAT(m_fMaxHitPoints);
	SAVE_FLOAT(m_fEnergy);
    SAVE_FLOAT(m_fMaxEnergy);
    SAVE_FLOAT(m_fArmorPoints);
    SAVE_FLOAT(m_fMaxArmorPoints);
    SAVE_FLOAT(m_fMass);
    SAVE_FLOAT(m_fLastDamage);
    SAVE_FLOAT(m_fLastArmorAbsorb);
    SAVE_FLOAT(m_fDeathDamage);
    SAVE_BOOL(m_bDead);
    SAVE_BOOL(m_bApplyDamagePhysics);
    SAVE_BOOL(m_bIsPlayer);
    SAVE_BOOL(m_bCanHeal);
    SAVE_BOOL(m_bCanRepair);
    SAVE_BOOL(m_bCanDamage);
    SAVE_BOOL(m_bNeverDestroy);
    SAVE_BYTE(m_eDeathType);
    SAVE_BYTE(m_eLastDamageType);
    SAVE_DWORD(m_nDamageTriggerCounter);
    SAVE_DWORD(m_nDamageTriggerNumSends);
    SAVE_QWORD(m_nCantDamageFlags);
	
	SAVE_CHARSTRING( m_sDamageCommand.c_str() );
    SAVE_CHARSTRING( m_sDamagerMessage.c_str() );
    SAVE_CHARSTRING( m_sDeathCommand.c_str() );
    SAVE_CHARSTRING( m_sPlayerDeathCommand.c_str() );
    SAVE_CHARSTRING( m_sKillerMessage.c_str() );
    SAVE_CHARSTRING( m_sDamagePercentCommand.c_str() );

	SAVE_CHARSTRING( m_sPlayerDamageCommand.c_str( ));
 
	SAVE_VECTOR(m_vDeathDir);
    SAVE_VECTOR(m_vLastDamageDir);
	SAVE_FLOAT( m_fDamagePercentCommand );

    int i;
    for (i=0; i < MAX_GEAR_ITEMS; i++)
	{
        SAVE_BOOL(m_bGearOwned[i]);
	}

	for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].Save(pMsg);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructible::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructible::Load(ILTMessage_Read *pMsg, uint8 nType)
{
	if (!pMsg) return;

    LOAD_HOBJECT(m_hObject);
    LOAD_HOBJECT(m_hLastDamager);

    LOAD_FLOAT(m_fHitPoints);
    LOAD_FLOAT(m_fMaxHitPoints);
	LOAD_FLOAT(m_fEnergy);
    LOAD_FLOAT(m_fMaxEnergy);
    LOAD_FLOAT(m_fArmorPoints);
    LOAD_FLOAT(m_fMaxArmorPoints);
    LOAD_FLOAT(m_fMass);
    LOAD_FLOAT(m_fLastDamage);
    LOAD_FLOAT(m_fLastArmorAbsorb);
    LOAD_FLOAT(m_fDeathDamage);
    LOAD_BOOL(m_bDead);
    LOAD_BOOL(m_bApplyDamagePhysics);
    LOAD_BOOL(m_bIsPlayer);
    LOAD_BOOL(m_bCanHeal);
    LOAD_BOOL(m_bCanRepair);
    LOAD_BOOL(m_bCanDamage);
    LOAD_BOOL(m_bNeverDestroy);
    LOAD_BYTE_CAST(m_eDeathType, DamageType);
    LOAD_BYTE_CAST(m_eLastDamageType, DamageType);
    LOAD_DWORD(m_nDamageTriggerCounter);
    LOAD_DWORD(m_nDamageTriggerNumSends);
    LOAD_QWORD(m_nCantDamageFlags);

	char szString[1024] = {0};
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDamageCommand = szString;
    LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDamagerMessage = szString;
    LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDeathCommand = szString;
    LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sPlayerDeathCommand = szString;
    LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sKillerMessage = szString;
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDamagePercentCommand = szString;
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sPlayerDamageCommand = szString;
	
	LOAD_VECTOR(m_vDeathDir);
    LOAD_VECTOR(m_vLastDamageDir);
 	LOAD_FLOAT( m_fDamagePercentCommand );
	
    int i;
    for (i=0; i < MAX_GEAR_ITEMS; i++)
	{
        LOAD_BOOL(m_bGearOwned[i]);
	}

	for (i=0; i < MAX_PROGRESSIVE_DAMAGE; i++)
	{
		m_ProgressiveDamage[i].Load(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDestructiblePlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CDestructiblePlugin::PreHook_PropChanged( const char *szObjName,
													   const char *szPropName, 
													   const int  nPropType, 
													   const GenericProp &gpPropValue,
													   ILTPreInterface *pInterface,
													   const char *szModifiers)
{
	// Check if the props are our commands and then just send it to the CommandMgr..

	if( !_stricmp( "DamageCommand", szPropName ) ||
		!_stricmp( "DeathCommand", szPropName ) ||
		!_stricmp( "PlayerDamageCommand", szPropName ) ||
		!_stricmp( "PlayerDeathCommand", szPropName ) )
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}
