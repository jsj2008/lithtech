//----------------------------------------------------------
//
// MODULE  : Destructable.h
//
// PURPOSE : Destructable class
//
// CREATED : 9/23/97
//
//----------------------------------------------------------

#include <stdio.h>
#include "Destructable.h"
#include "cpp_server_de.h"
#include "RiotMsgIds.h"
#include "Projectile.h"
#include "RiotObjectUtilities.h"
#include "BaseCharacter.h"
#include "PlayerObj.h"
#include "RiotServerShell.h"

#define TRIGGER_MSG_DESTROY	"DESTROY"
#define TRIGGER_MSG_HEAL	"HEAL"
#define TRIGGER_MSG_REPAIR	"REPAIR"
#define TRIGGER_MSG_DAMAGE	"DAMAGE"
#define TRIGGER_MSG_RESET	"RESET"

#define INFINITE_MASS			100000.0f
#define MAXIMUM_BLOCK_PRIORITY	255.0f
#define MINIMUM_FRICTION		5.0f		
#define MAXIMUM_FRICTION		15.0f		
#define MINIMUM_FORCE			0.0f

#define CRITICAL_HIT_CHANCE			0.05f	// Percentage of time critical hits happen
#define CRITICAL_HIT_RATIO			4.0f	// 4 times as much damage
#define CRITICAL_HIT_HEALTH_BONUS	.25f	// Percentage of health bonus

#define CANRAGDOLL(eType) (((eType) != DT_PUNCTURE) && ((eType) != DT_MELEE) && ((eType) != DT_BURST) && ((eType) != DT_KATO))

extern CRiotServerShell* g_pRiotServerShellDE;


// This is a global because it affects all characters.
CVarTrack g_DamageScale;
CVarTrack g_HealScale;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::CDestructable
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDestructable::CDestructable() : Aggregate()
{
	m_hObject					= DNULL;

	m_bDead						= DFALSE;
	m_bPlayer					= DFALSE;
	m_fNextRegen				= 0.0f;
	m_bApplyDamagePhysics		= DTRUE;
	m_fMass						= 1.0f;
	m_fHitPoints				= 1.0f;
	m_fMaxHitPoints				= 1.0f;
	m_fArmorPoints				= 0.0f;
	m_fMaxArmorPoints			= 1.0f;
	m_eDeathType				= DT_UNSPECIFIED;
	m_fDeathDamage				= 0.0f;
	m_dwCantDamageTypes			= 0;

	VEC_INIT(m_vDeathDir);
	VEC_INIT(m_vLastDamageDir);

	m_hstrDamageTriggerTarget	= DNULL;
	m_hstrDamageTriggerMessage	= DNULL;
	m_hstrDamagerMessage		= DNULL;
	m_hstrDeathTriggerTarget	= DNULL;
	m_hstrDeathTriggerMessage	= DNULL;
	m_hstrKillerMessage			= DNULL;
	m_nDamageTriggerNumSends	= 1;

	m_bCanHeal					= DTRUE;
	m_bCanRepair				= DTRUE;
	m_bCanDamage				= DTRUE;
	m_bNeverDestroy				= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::~CDestructable
//
//	PURPOSE:	Destructable
//
// ----------------------------------------------------------------------- //

CDestructable::~CDestructable()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hstrDamageTriggerTarget)
	{
		pServerDE->FreeString(m_hstrDamageTriggerTarget);
	}

	if (m_hstrDamageTriggerMessage)
	{
		pServerDE->FreeString(m_hstrDamageTriggerMessage);
	}

	if (m_hstrDeathTriggerTarget)
	{
		pServerDE->FreeString(m_hstrDeathTriggerTarget);
	}

	if (m_hstrDeathTriggerMessage)
	{
		pServerDE->FreeString(m_hstrDeathTriggerMessage);
	}

	if (m_hstrDamagerMessage)
	{
		pServerDE->FreeString(m_hstrDamagerMessage);
	}

	if (m_hstrKillerMessage)
	{
		pServerDE->FreeString(m_hstrKillerMessage);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //
		
DDWORD CDestructable::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_TOUCHNOTIFY:
		{
			HandleTouch(pObject, (HOBJECT)pData, fData); 
			break;
		}

		case MID_CRUSH:
		{
			HandleCrush(pObject, (HOBJECT)pData); 
			break;
		}

		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
			break;
		}

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pObject);
			}
			break;
		}

		case MID_UPDATE:
		{
			if (m_bPlayer) Update();
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DBYTE)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DBYTE)fData);
		}
		break;
	}

	return Aggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD CDestructable::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(pObject, hSender, hRead);
			break;
		}
		
		case MID_DAMAGE:
		{
			HandleDamage(pObject, hSender, hRead);
			break;
		}

		case MID_REPAIR:
		{
			if( m_bCanRepair )
				HandleRepair(pObject, hSender, hRead);
			break;
		}

		case MID_HEAL:
		{
			if( m_bCanHeal )
				HandleHeal(pObject, hSender, hRead);
			break;
		}

		case MID_ULTRAHEAL:
		{
			if( m_bCanHeal )
				HandleUltraHeal(pObject, hSender, hRead);
			break;
		}

		default : break;
	}

	return Aggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL CDestructable::ReadProp(BaseClass *pObject, ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pStruct || !pServerDE) return DFALSE;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("DamageTriggerTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDamageTriggerTarget = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("DamageTriggerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDamageTriggerMessage = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("DamageTriggerNumSends", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_nDamageTriggerNumSends = genProp.m_Long;
		}
	}

	if (pServerDE->GetPropGeneric("DamagerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDamagerMessage = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("DeathTriggerTarget", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDeathTriggerTarget = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("DeathTriggerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrDeathTriggerMessage = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("KillerMessage", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrKillerMessage = pServerDE->CreateString(genProp.m_String);
		}
	}

	if (pServerDE->GetPropGeneric("CanHeal", &genProp) == DE_OK)
	{
		m_bCanHeal = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("CanRepair", &genProp) == DE_OK)
	{
		m_bCanRepair = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("CanDamage", &genProp) == DE_OK)
	{
		m_bCanDamage = genProp.m_Bool;
	}

	if (pServerDE->GetPropGeneric("NeverDestroy", &genProp) == DE_OK)
	{
		m_bNeverDestroy = genProp.m_Bool;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Init
//
//	PURPOSE:	Initialize object - no longer used (InitialUpdate() does this)
//
// ----------------------------------------------------------------------- //

DBOOL CDestructable::Init(HOBJECT hObject)
{
	if (!m_hObject) m_hObject = hObject;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::InitialUpdate
//
//	PURPOSE:	Handle object initial update
//
// ----------------------------------------------------------------------- //

void CDestructable::InitialUpdate(LPBASECLASS pObject)
{
	if (!pObject || !pObject->m_hObject) return;
	if (!m_hObject) m_hObject = pObject->m_hObject;


	SetBlockingPriority();

	// determine if we are a player object or not

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	// Init this (if it hasn't been initted already).
	if(!g_DamageScale.IsInitted())
		g_DamageScale.Init(pServerDE, "DamageScale", DNULL, 1.0f);
	
	if(!g_HealScale.IsInitted())
		g_HealScale.Init(pServerDE, "HealScale", DNULL, 1.0f);

	if (m_fMass > 1.0f)
	{
		pServerDE->SetObjectMass(m_hObject, m_fMass);
	}

	if (IsPlayer (m_hObject))
	{
		m_bPlayer = DTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Update
//
//	PURPOSE:	Handle object updates
//
// ----------------------------------------------------------------------- //

void CDestructable::Update()
{
	if (!m_bPlayer) return;

	// does this player have the regeneration upgrade or enhancement(s)?
	
	HandleRegen();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::SetMass
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructable::SetMass(DFLOAT fMass)
{
	m_fMass = fMass;

	SetBlockingPriority();

	// Set the friction based on the mass of the object...

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	pServerDE->SetObjectMass(m_hObject, m_fMass);

	DFLOAT fFricCoeff = MINIMUM_FRICTION + (m_fMass * MAXIMUM_FRICTION / INFINITE_MASS);
	pServerDE->SetFrictionCoefficient(m_hObject, fFricCoeff);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::SetBlockingPriority
//
//	PURPOSE:	Set the blocking priority for this object
//
// ----------------------------------------------------------------------- //

void CDestructable::SetBlockingPriority()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DBYTE nPriority = (DBYTE)(m_fMass * MAXIMUM_BLOCK_PRIORITY / INFINITE_MASS);
	if (nPriority <= 0) nPriority = 1;
	
	pServerDE->SetBlockingPriority(m_hObject, nPriority);
	pServerDE->SetForceIgnoreLimit(m_hObject, m_fMass /*MINIMUM_FORCE*/);

	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_TOUCH_NOTIFY;
	pServerDE->SetObjectFlags(m_hObject, dwFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::GetMaxHitPoints
//
//	PURPOSE:	Returns the maximum hit points for this object
// 
// ----------------------------------------------------------------------- //

DFLOAT CDestructable::GetMaxHitPoints() const
{
	DFLOAT fAdjustedMax = m_fMaxHitPoints;
	if (m_bPlayer)
	{
		CServerDE* pServerDE = BaseClass::GetServerDE();
		if (pServerDE)
		{
			CPlayerObj* pPlayer = (CPlayerObj*) pServerDE->HandleToObject (m_hObject);
			if (pPlayer)
			{
				Inventory* pInventory = pPlayer->GetInventory();

				// see if the player has the health upgrade

				if (pInventory->HaveItem (IT_UPGRADE, IST_UPGRADE_HEALTH))
				{
					fAdjustedMax += m_fMaxHitPoints * 0.50f;
				}

				// see if the player has any health enhancements

				DDWORD nEnhancements = pInventory->GetNumItems (IT_ENHANCEMENT, IST_ENHANCEMENT_HEALTH);
				if (nEnhancements)
				{
					if (nEnhancements > 10) nEnhancements = 10;	// max out at 50%

					fAdjustedMax += (m_fMaxHitPoints * 0.05f) * nEnhancements;
				}
			}
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::GetMaxArmorPoints
//
//	PURPOSE:	Returns the maximum armor points for this object
// 
// ----------------------------------------------------------------------- //

DFLOAT CDestructable::GetMaxArmorPoints() const
{
	DFLOAT fAdjustedMax = m_fMaxArmorPoints;

	if (m_bPlayer)
	{
		CServerDE* pServerDE = BaseClass::GetServerDE();
		if (pServerDE)
		{
			CPlayerObj* pPlayer = (CPlayerObj*) pServerDE->HandleToObject (m_hObject);
			if (pPlayer)
			{
				Inventory* pInventory = pPlayer->GetInventory();

				// see if the player has the armor upgrade...
				
				if (pInventory->HaveItem (IT_UPGRADE, IST_UPGRADE_ARMOR))
				{
					fAdjustedMax += m_fMaxArmorPoints * 0.50f;
				}

				// see if the player has any armor enhancements

				DDWORD nEnhancements = pInventory->HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_ARMOR);
				if (nEnhancements)
				{
					if (nEnhancements > 10) nEnhancements = 10; // max out at 50%

					fAdjustedMax += (m_fMaxArmorPoints * 0.05f) * nEnhancements;
				}
			}
		}
	}

	return fAdjustedMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleDamage
//
//	PURPOSE:	Handle damage message
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleDamage(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;


	DVector vDir;
	pServerDE->ReadFromMessageVector(hRead, &vDir);
	DFLOAT	   fDamage  = pServerDE->ReadFromMessageFloat(hRead) * g_DamageScale.GetFloat(1.0f);
	DamageType eType	= (DamageType)pServerDE->ReadFromMessageByte(hRead);
    HOBJECT	   hHeHitMe = pServerDE->ReadFromMessageObject(hRead);
				
	pServerDE->ResetRead(hRead);	

	// See if this type of damage applies to us...

	if (m_dwCantDamageTypes & eType) return;


	// Check for critical hit...
	
	if (eType == DT_ENERGY || eType == DT_BURST ||
		eType == DT_EXPLODE || eType == DT_PUNCTURE)
	{
		HandleCriticalHit(hHeHitMe, fDamage);
	}

	// Process damage through any powerups...

	fDamage = ProcessPowerups (pObject, fDamage, eType, hHeHitMe, &vDir);

	// See if we should actually process damage...

	if (m_bDead || fDamage == 0.0f) return;


	// Rag-doll time...

	if (!IsPlayer(m_hObject) && m_bApplyDamagePhysics && (m_fMass < INFINITE_MASS) && 
		(VEC_MAGSQR(vDir) > 0.001f) && CANRAGDOLL(eType))
	{
		DVector vImpactVel, vTemp, vVel;

		pServerDE->GetVelocity(m_hObject, &vVel);

		VEC_COPY(vTemp, vDir);
		VEC_NORM(vTemp);

		if (m_fMass <= 0) m_fMass = 100.0f;
			
		DFLOAT fMultiplier = fDamage * (INFINITE_MASS/(m_fMass*5.0f));

		// If not on ground, scale down velocity...
		
		CollisionInfo standingInfo;
		pServerDE->GetStandingOn(m_hObject, &standingInfo);
			
		if (!standingInfo.m_hObject) 
		{
			fMultiplier /= 10.0f;
		}

		VEC_MULSCALAR(vTemp, vTemp, fMultiplier);
		VEC_ADD(vImpactVel, vTemp, vVel);

		pServerDE->SetVelocity(m_hObject, &vImpactVel);
	}


	// see if we're choking

	DFLOAT fAbsorb = 0.0f;
	if (m_fArmorPoints > 0.0 && eType != DT_CHOKE)
	{
		// Stole this armor calculation from Blood...
		// Absorb between 1/4 and 7/8 of damage
		DFLOAT fMaxAbsorb = fDamage*0.875f;
		DFLOAT fMinAbsorb = fDamage*0.25f;

		fAbsorb = fMinAbsorb + (m_fArmorPoints * ((fMaxAbsorb - fMinAbsorb) / GetMaxArmorPoints())); 
		if (fAbsorb > m_fArmorPoints) fAbsorb = m_fArmorPoints;

		fDamage	-= fAbsorb;

		if (fDamage < 0.0) 
		{
			fDamage = 0.0f;
		}
	}

	
	m_fLastDamage = fDamage;
	VEC_COPY(m_vLastDamageDir, vDir);

	
	// Actually apply the damage

	if (!m_bCanDamage || DebugDamageOn())
	{
		return;
	}
	else
	{
		m_fArmorPoints -= fAbsorb;
		if (m_fArmorPoints < 0.0) 
		{
			m_fArmorPoints = 0.0;
		}

		m_fHitPoints -= fDamage;
	}


	if (m_fHitPoints <= 0.5 && !m_bNeverDestroy) 
	{
		m_fHitPoints = 0.0f;

		m_fDeathDamage = fDamage;
		m_eDeathType   = eType;
		VEC_COPY(m_vDeathDir, vDir);

		HandleDestruction(hHeHitMe);

		DBOOL bIsPlayer = IsPlayer(m_hObject);

		if (bIsPlayer)
		{
			// Stop player movement if dead...

			DVector vZero;
			VEC_INIT(vZero);
			pServerDE->SetVelocity(m_hObject, &vZero);
			pServerDE->SetAcceleration(m_hObject, &vZero);
		}


		// If this was a player-player frag, send a message to all clients

		if (bIsPlayer)
		{
			CPlayerObj* pVictim = (CPlayerObj*) pServerDE->HandleToObject (m_hObject);
			CPlayerObj* pKiller = (IsPlayer (hHeHitMe)) ? ((CPlayerObj*) pServerDE->HandleToObject (hHeHitMe)) : (pVictim);
			
			if (pVictim && pKiller)
			{
				if (pVictim == pKiller)
				{
					pKiller->DecFragCount();
				}
				else
				{
					pKiller->IncFragCount();
				}
				
				if (g_pRiotServerShellDE)
				{
					g_pRiotServerShellDE->SetUpdateShogoServ();
				}

				HCLIENT hClientVictim = pVictim->GetClient();
				HCLIENT hClientKiller = pKiller->GetClient();
				if (hClientVictim && hClientKiller)
				{
					DDWORD nVictimID = pServerDE->GetClientID (hClientVictim);
					DDWORD nKillerID = pServerDE->GetClientID (hClientKiller);

					HMESSAGEWRITE hWrite = pServerDE->StartMessage (DNULL, MID_PLAYER_FRAGGED);
					pServerDE->WriteToMessageFloat (hWrite, (float)nVictimID);
					pServerDE->WriteToMessageFloat (hWrite, (float)nKillerID);
					pServerDE->EndMessage (hWrite);
				}
			}
		}
	}
	
	
	// If this is supposed to send a damage trigger, send it now...

	if (m_hstrDamageTriggerTarget && m_hstrDamageTriggerMessage && m_nDamageTriggerNumSends != 0)
	{
		SendTriggerMsgToObjects(pObject, m_hstrDamageTriggerTarget, m_hstrDamageTriggerMessage);
		m_nDamageTriggerNumSends--;
	}

	// Send message to object that damaged us...

	if (hHeHitMe && m_hstrDamagerMessage)
	{
		SendTriggerMsgToObject(pObject, hHeHitMe, m_hstrDamagerMessage);
	}
	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::ProcessPowerups
//
//	PURPOSE:	Adjust damage according to which powerups we or the 
//				damage originator have
// 
// ----------------------------------------------------------------------- //

DFLOAT CDestructable::ProcessPowerups (LPBASECLASS pObject, DFLOAT fDamage, DamageType eDamageType, HOBJECT hHeHitMe, DVector* pvDir)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return 0.0f;
	
	// hold on to original damage 

	DFLOAT fOriginalDamage = fDamage;

	// see if whoever hit us is derived from basecharacter

	CBaseCharacter* pEnemy = DNULL;
	if (IsBaseCharacter (hHeHitMe))
	{
		pEnemy = (CBaseCharacter*) pServerDE->HandleToObject (hHeHitMe);
	}

	// see if we are derived from basecharacter
	CBaseCharacter* pUs = DNULL;
	if (IsBaseCharacter (m_hObject))
	{
		pUs = (CBaseCharacter*) pServerDE->HandleToObject (m_hObject);
	}

	// if whoever hit us is another player and they have the damage upgrade or enhancement(s),
	// increase damage done...

	if (pEnemy)
	{
		Inventory* pInventory = pEnemy->GetInventory();
		
		// see if they have the damage upgrade

		if (pInventory->HaveItem (IT_UPGRADE, IST_UPGRADE_DAMAGE))
		{
			fDamage += fOriginalDamage * 1.25f;
		}

		// see if they have any damage enhancements

		DDWORD nEnhancements = pInventory->GetNumItems (IT_ENHANCEMENT, IST_ENHANCEMENT_DAMAGE);
		if (nEnhancements)
		{
			if (nEnhancements > 10) nEnhancements = 10;	// max out at 50%

			fDamage += (fOriginalDamage * 0.05f) * nEnhancements;
		}

		// see if they have the UltraDamage powerup

		if (pEnemy->HaveTimedPowerup (PIT_ULTRA_DAMAGE))
		{
			fDamage += (fOriginalDamage * 2.0f);	// increase by 300%
		}
		
		// see if this was a melee attack and they have any melee damage enhancements

		nEnhancements = pInventory->GetNumItems (IT_ENHANCEMENT, IST_ENHANCEMENT_MELEEDAMAGE);
		if (nEnhancements && eDamageType == DT_MELEE)
		{
			if (nEnhancements > 5) nEnhancements = 5;	// max out at 50%

			fDamage += (fOriginalDamage * 0.10f) * nEnhancements;
		}
	}

	// if we have the reflect ultra powerup, reflect the damage back on whatever it was that's hurting us

	if (pUs)
	{
		if (pUs->HaveTimedPowerup (PIT_ULTRA_REFLECT))
		{
			// reflect the damage and return...

			VEC_NEGATE (*pvDir, *pvDir);

			HMESSAGEWRITE hWrite = pServerDE->StartMessageToObject (pObject, hHeHitMe, MID_DAMAGE);
			pServerDE->WriteToMessageVector (hWrite, pvDir);
			pServerDE->WriteToMessageFloat (hWrite, fDamage);
			pServerDE->WriteToMessageByte (hWrite, eDamageType);
			pServerDE->WriteToMessageObject (hWrite, m_hObject);
			pServerDE->EndMessage (hWrite);

			return 0.0f;
		}
	}
	
	// if we are a player object and we have the resist damage upgrade or enhancement(s),
	// decrease damage done...

	if (pUs)
	{
		Inventory* pInventory = pUs->GetInventory();

		// see if they have the protection upgrade
	
		if (pInventory->HaveItem (IT_UPGRADE, IST_UPGRADE_PROTECTION))
		{
			fDamage -= fOriginalDamage * 0.25f;
		}

		// see if they have any general protection enhancements

		DDWORD nEnhancements = pInventory->GetNumItems (IT_ENHANCEMENT, IST_ENHANCEMENT_PROTECTION);
		if (nEnhancements)
		{
			if (nEnhancements > 10) nEnhancements = 10;	// max out at 50%

			fDamage -= (fOriginalDamage * 0.05f) * nEnhancements;
		}

		// now check for each specific protection type

		nEnhancements = pInventory->GetNumItems (IT_ENHANCEMENT, IST_ENHANCEMENT_ENERGYPROTECTION);
		if (nEnhancements && (eDamageType == DT_ENERGY || eDamageType == DT_ELECTROCUTE))
		{
			if (nEnhancements > 10) nEnhancements = 10;	// max out at 50%

			fDamage -= (fOriginalDamage * 0.05f) * nEnhancements;
		}

		nEnhancements = pInventory->GetNumItems (IT_ENHANCEMENT, IST_ENHANCEMENT_PROJECTILEPROTECTION);
		if (nEnhancements && (eDamageType == DT_PUNCTURE || eDamageType == DT_BURST))
		{
			if (nEnhancements > 10) nEnhancements = 10;	// max out at 50%

			fDamage -= (fOriginalDamage * 0.05f) * nEnhancements;
		}

		nEnhancements = pInventory->GetNumItems (IT_ENHANCEMENT, IST_ENHANCEMENT_EXPLOSIVEPROTECTION);
		if (nEnhancements && eDamageType == DT_EXPLODE)
		{
			if (nEnhancements > 10) nEnhancements = 10;	// max out at 50%

			fDamage -= (fOriginalDamage * 0.05f) * nEnhancements;
		}
	}

	return fDamage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleRegen
//
//	PURPOSE:	Handle regeneration if they have that powerup
//
// ----------------------------------------------------------------------- //

void CDestructable::HandleRegen()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;
	
	// does this player have the regeneration upgrade or enhancement(s)?

	DDWORD nRegenEnhancements = 0;
	CPlayerObj* pPlayer = (CPlayerObj*) pServerDE->HandleToObject (m_hObject);
	if (!pPlayer) return;
	Inventory* pInventory = pPlayer->GetInventory();
	if (pInventory->HaveItem (IT_UPGRADE, IST_UPGRADE_REGEN))
	{
		// if it's time to regen, send ourselves a heal message

		float nTime = pServerDE->GetTime();
		if (nTime > m_fNextRegen)
		{
			HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pPlayer, m_hObject, MID_HEAL);
			pServerDE->WriteToMessageFloat(hMessage, 2.0f);
			pServerDE->EndMessage(hMessage);

			m_fNextRegen = nTime + 1.0f;
		}
	}
	
	nRegenEnhancements = pInventory->GetNumItems (IT_ENHANCEMENT, IST_ENHANCEMENT_REGEN);
	if (nRegenEnhancements)
	{
		// if it's time to regen, send ourselves a heal message
		
		float nTime = pServerDE->GetTime();
		if (nTime > m_fNextRegen)
		{
			HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pPlayer, m_hObject, MID_HEAL);
			pServerDE->WriteToMessageFloat(hMessage, (float) nRegenEnhancements);
			pServerDE->EndMessage(hMessage);

			m_fNextRegen = nTime + 10.0f;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleDestruction
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

void CDestructable::HandleDestruction( HOBJECT hDamager )
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	m_bDead		 = DTRUE;
	m_fHitPoints = 0.0;

	LPBASECLASS pD = pServerDE->HandleToObject(m_hObject);

	if (m_hstrDeathTriggerTarget && m_hstrDeathTriggerMessage)
	{
		SendTriggerMsgToObjects(pD, m_hstrDeathTriggerTarget, m_hstrDeathTriggerMessage);
	}

	// Send message to object that killed us...
	if( hDamager && m_hstrKillerMessage )
	{
		SendTriggerMsgToObject( pD, hDamager, m_hstrKillerMessage );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleHeal
//
//	PURPOSE:	Handle heal message.
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleHeal(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || m_bDead) return;

	DFLOAT fAmount = pServerDE->ReadFromMessageFloat(hRead) * g_HealScale.GetFloat(1.0f);

	// see if we can heal
	
	if (Heal(fAmount))
	{
		HMESSAGEWRITE hWrite = pServerDE->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
		pServerDE->WriteToMessageFloat (hWrite, -1.0f);
		pServerDE->EndMessage (hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleUltraHeal
//
//	PURPOSE:	Handle ultra heal message.
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleUltraHeal(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || m_bDead) return;

	DFLOAT fAmount = pServerDE->ReadFromMessageFloat(hRead);

	// we should be a player to get this message, but double check and see whether or not we're on foot

	DBOOL bSuccessfulHeal = DFALSE;

	if (m_bPlayer)
	{
		CPlayerObj* pPlayer = (CPlayerObj*) pServerDE->HandleToObject (m_hObject);
		if (pPlayer)
		{
			if (pPlayer->IsMecha())
			{
				if (m_fHitPoints < 2000.0f)
				{
					DoActualHealing (1000.0f);
					if (m_fHitPoints > 2000.0f) m_fHitPoints = 2000.0f;
					bSuccessfulHeal = DTRUE;
				}
			}
			else
			{
				if (m_fHitPoints < 200.0f)
				{
					DoActualHealing (100.0f);
					if (m_fHitPoints > 200.0f) m_fHitPoints = 200.0f;
					bSuccessfulHeal = DTRUE;
				}
			}
		}
	}
	
	if (bSuccessfulHeal)
	{
		HMESSAGEWRITE hWrite = pServerDE->StartMessageToObject(pObject, hSender, MID_PICKEDUP);
		pServerDE->WriteToMessageFloat (hWrite, -1.0f);
		pServerDE->EndMessage (hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Heal
//
//	PURPOSE:	Add some value to hit points
// 
// ----------------------------------------------------------------------- //

DBOOL CDestructable::Heal(DFLOAT fAmount)
{
	DFLOAT fMax = GetMaxHitPoints();
	if (m_fHitPoints >= fMax) return DFALSE;

	// only heal what we need to (cap hit points at maximum)

	if (m_fHitPoints + fAmount > fMax)
	{
		fAmount = fMax - m_fHitPoints;
	}

	// now actually heal the object

	DoActualHealing (fAmount);	

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::DoActualHealing()
//
//	PURPOSE:	Simply adds a value to the hit points variable
//
// ----------------------------------------------------------------------- //

void CDestructable::DoActualHealing (DFLOAT fAmount)
{
	// NOTE: This function should only be called directly from the ultra heal 
	//		 powerup code, as it does not do any bounds checking on the hit points.
	//       All other healing should be done through the HandleHeal() function above.
	
	if(!m_bCanHeal) return;

	m_bDead = DFALSE;

	m_fHitPoints += fAmount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleRepair()
//
//	PURPOSE:	Handle Repair message
//
// ----------------------------------------------------------------------- //

void CDestructable::HandleRepair(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DFLOAT fAmount = pServerDE->ReadFromMessageFloat(hRead);

	if(Repair(fAmount))
	{
		HMESSAGEWRITE hWrite = pServerDE->StartMessageToObject (pObject, hSender, MID_PICKEDUP);
		pServerDE->WriteToMessageFloat (hWrite, -1.0f);
		pServerDE->EndMessage (hWrite);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Repair()
//
//	PURPOSE:	Add some value to armor
//
// ----------------------------------------------------------------------- //
DBOOL CDestructable::Repair(DFLOAT fAmount)
{
	if( !m_bCanRepair ) return DFALSE;

	DFLOAT fMax = GetMaxArmorPoints();
	if( m_fArmorPoints >= fMax ) return DFALSE;

	m_fArmorPoints += fAmount;

	if(m_fArmorPoints > fMax) 
		m_fArmorPoints = fMax;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Reset
//
//	PURPOSE:	Reset
//
// ----------------------------------------------------------------------- //

void CDestructable::Reset(DFLOAT fHitPts, DFLOAT fArmorPts)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DFLOAT fMaxHitPoints = GetMaxHitPoints();
	DFLOAT fMaxArmorPoints = GetMaxArmorPoints();

	m_fHitPoints	= fHitPts <= fMaxHitPoints ? fHitPts : fMaxHitPoints;
	m_fArmorPoints  = fArmorPts <= fMaxArmorPoints ? fArmorPts : fMaxArmorPoints;
	m_eDeathType	= DT_UNSPECIFIED;
	m_fDeathDamage	= 0.0f;
	m_bDead			= DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || m_bDead) return;

	HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);
	if (!hMsg) return;

	// See if we should destroy ourself...

	HSTRING hstr = pServerDE->CreateString(TRIGGER_MSG_DESTROY);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		DVector vDir;
		VEC_INIT(vDir);

		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pObject, pObject->m_hObject, MID_DAMAGE);
		pServerDE->WriteToMessageVector(hMessage, &vDir);
		pServerDE->WriteToMessageFloat(hMessage, GetMaxHitPoints()*5);
		pServerDE->WriteToMessageByte(hMessage, DT_UNSPECIFIED);
		pServerDE->WriteToMessageObject(hMessage, hSender);
		pServerDE->EndMessage(hMessage);
	}
	pServerDE->FreeString(hstr);

#ifdef IMPLEMENTED_THESE
	// See if we should damage ourself...

	hstr = pServerDE->CreateString(TRIGGER_MSG_DAMAGE);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		DVector vDir;
		INIT_VEC(vDir);

		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pObject, pObject->m_hObject, MID_DAMAGE);
		pServerDE->WriteToMessageVector(hMessage, &vDir);
		pServerDE->WriteToMessageFloat(hMessage, fDamage);
		pServerDE->WriteToMessageByte(hMessage, DT_UNSPECIFIED);
		pServerDE->WriteToMessageObject(hMessage, hSender);
		pServerDE->EndMessage(hMessage);
	}
	pServerDE->FreeString(hstr);


	// See if we should heal ourself...

	hstr = pServerDE->CreateString(TRIGGER_MSG_HEAL);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pObject, pObject->m_hObject, MID_HEAL);
		pServerDE->WriteToMessageFloat(hMessage, fAmount);
		pServerDE->EndMessage(hMessage);
	}
	pServerDE->FreeString(hstr);


	// See if we should repair ourself...

	hstr = pServerDE->CreateString(TRIGGER_MSG_REPAIR);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pObject, pObject->m_hObject, MID_REPAIR);
		pServerDE->WriteToMessageFloat(hMessage, fAmount);
		pServerDE->EndMessage(hMessage);
	}
	pServerDE->FreeString(hstr);
#endif // IMPLEMENTED_THESE

	// See if we should reset ourself...

	hstr = pServerDE->CreateString(TRIGGER_MSG_RESET);
	if (pServerDE->CompareStringsUpper(hMsg, hstr))
	{
		Reset(GetMaxHitPoints(), GetMaxArmorPoints());
	}
	pServerDE->FreeString(hstr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleCrush
//
//	PURPOSE:	Handle crush message
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleCrush(LPBASECLASS pObject, HOBJECT hSender)
{
	if (!pObject || !hSender || m_bDead) return;

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DFLOAT fMyMass, fHisMass;
	fMyMass  = pServerDE->GetObjectMass(m_hObject);
	fHisMass = pServerDE->GetObjectMass(hSender);

	// If we're within 15% of each other, don't do anything rash...

	if (fHisMass <= fMyMass*1.15) return;

	DFLOAT fDamage = 20.0f;

	DVector vDir;
	VEC_INIT(vDir);

	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pObject, pObject->m_hObject, MID_DAMAGE);
	pServerDE->WriteToMessageVector(hMessage, &vDir);
	pServerDE->WriteToMessageFloat(hMessage, fDamage);
	pServerDE->WriteToMessageByte(hMessage, DT_IMPACT);
	pServerDE->WriteToMessageObject(hMessage, hSender);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleTouch
//
//	PURPOSE:	Handle object contact
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleTouch(LPBASECLASS pObject, HOBJECT hSender, DFLOAT fForce)
{
	if (!pObject || !hSender || m_bDead || fForce < 0.0f) return;

	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	HOBJECT hWorld = pServerDE->GetWorldObject();

	DFLOAT fMyMass, fHisMass;
	fMyMass  = pServerDE->GetObjectMass(pObject->m_hObject);
	fHisMass = pServerDE->GetObjectMass(hSender);

	if (hSender == hWorld)
	{
		fHisMass = INFINITE_MASS;
	}

	if (fHisMass <= 0.1f || fMyMass <= 0.1f) return;

	// If we're within 15% of each other, don't do anything rash...

	if (fHisMass <= fMyMass*1.15 || fMyMass > fHisMass) return;


	// Not enough force to do anything...

	DFLOAT fVal = (hSender == hWorld) ? fMyMass * 15.0f : fMyMass * 10.0f;
	if (fForce < fVal) return;


	// Okay, one last check.  Make sure that he is moving, if I run into him
	// that shouldn't cause anything to happen.  It is only when he runs into
	// me that I can take damage...(well, unless it is the world...

	if (hSender != hWorld)
	{
		DVector vVel;
		pServerDE->GetVelocity(hSender, &vVel);
		if (VEC_MAG(vVel) < 1.0f) return;
	}


	// Calculate damage...

	DFLOAT fDamage = (fForce / fMyMass);
	fDamage *= (hSender == hWorld) ? 5.0f : 10.0f;

	//pServerDE->BPrint("%.2f, %.2f, %.2f, %.2f", 
	//				  fHisMass, fMyMass, fForce, fDamage);

	DVector vDir;
	VEC_INIT(vDir);

	if (hSender == hWorld)
	{
		pServerDE->GetVelocity(pObject->m_hObject, &vDir);
		VEC_NEGATE(vDir, vDir);
	}
	else
	{
		DVector vMyPos, vHisPos;
		pServerDE->GetObjectPos(pObject->m_hObject, &vMyPos);
		pServerDE->GetObjectPos(hSender, &vHisPos);

		VEC_SUB(vDir, vMyPos, vHisPos);
	}

	VEC_NORM(vDir);
	
	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pObject, pObject->m_hObject, MID_DAMAGE);
	pServerDE->WriteToMessageVector(hMessage, &vDir);
	pServerDE->WriteToMessageFloat(hMessage, fDamage);
	pServerDE->WriteToMessageByte(hMessage, DT_IMPACT);
	pServerDE->WriteToMessageObject(hMessage, hSender);
	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleCriticalHit
//
//	PURPOSE:	Handle crush message
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleCriticalHit(HOBJECT hHeHitMe, DFLOAT & fDamage)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hHeHitMe || m_bDead || !m_bCanDamage) return;

	if (GetRandom(0.0f, 1.0f) <= CRITICAL_HIT_CHANCE)
	{
		// Make sure this is a character type...

		if (IsBaseCharacter (m_hObject))
		{
			fDamage *= CRITICAL_HIT_RATIO;

			// Give the person who hit me a bonus...
	
			if (IsBaseCharacter (hHeHitMe))
			{
				CBaseCharacter* pChar = (CBaseCharacter*)pServerDE->HandleToObject(hHeHitMe);
				LPBASECLASS pObject = pServerDE->HandleToObject(m_hObject);

				if (pChar && pObject)
				{
					CDestructable* pDamage = pChar->GetDestructable();
					if (!pDamage) return;

					DFLOAT fAmount = CRITICAL_HIT_HEALTH_BONUS * pDamage->GetMaxHitPoints();

					HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject(pObject, hHeHitMe, MID_HEAL);
					pServerDE->WriteToMessageFloat(hMessage, fAmount);
					pServerDE->EndMessage(hMessage);
				}
			}

			// Create critical hit object...
	
			DFLOAT fHitter = -1.0f, fHittee = -1.0f;

			if (IsPlayer(m_hObject))
			{
				CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(m_hObject);
				if (pPlayer)
				{
					fHittee = (DFLOAT)pServerDE->GetClientID(pPlayer->GetClient());
				}
			}
					
			if (IsPlayer(hHeHitMe))
			{
				CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(hHeHitMe);
				if (pPlayer)
				{
					fHitter = (DFLOAT)pServerDE->GetClientID(pPlayer->GetClient());
				}
			}

			DVector vPos;
			pServerDE->GetObjectPos(m_hObject, &vPos);
					
			HMESSAGEWRITE hMessage = pServerDE->StartInstantSpecialEffectMessage(&vPos);
			pServerDE->WriteToMessageByte(hMessage, SFX_CRITICALHIT_ID);
			pServerDE->WriteToMessageCompPosition(hMessage, &vPos);
			pServerDE->WriteToMessageFloat(hMessage, fHitter);
			pServerDE->WriteToMessageFloat(hMessage, fHittee);
			pServerDE->EndMessage(hMessage);
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::DebugDamageOn
//
//	PURPOSE:	See if the object can be damaged
//
// ----------------------------------------------------------------------- //

DBOOL CDestructable::DebugDamageOn()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DBOOL bRet = DFALSE;

	HCONVAR	hVar  = pServerDE->GetGameConVar("SetDamage");
	char* pVal = pServerDE->GetVarValueString(hVar);

	if (!pVal) return DFALSE;

	return ((DBOOL) !atoi(pVal));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructable::Save(HMESSAGEWRITE hWrite, DBYTE nType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hObject);

	pServerDE->WriteToMessageFloat(hWrite, m_fHitPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxHitPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fArmorPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxArmorPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fMass);
	pServerDE->WriteToMessageFloat(hWrite, m_fNextRegen);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastDamage);
	pServerDE->WriteToMessageFloat(hWrite, m_fDeathDamage);
	pServerDE->WriteToMessageByte(hWrite, m_bDead);
	pServerDE->WriteToMessageByte(hWrite, m_bApplyDamagePhysics);
	pServerDE->WriteToMessageByte(hWrite, m_bPlayer);
	pServerDE->WriteToMessageByte(hWrite, m_bCanHeal);
	pServerDE->WriteToMessageByte(hWrite, m_bCanRepair);
	pServerDE->WriteToMessageByte(hWrite, m_bCanDamage);
	pServerDE->WriteToMessageByte(hWrite, m_bNeverDestroy);
	pServerDE->WriteToMessageByte(hWrite, m_eDeathType);
	pServerDE->WriteToMessageDWord(hWrite, m_nDamageTriggerNumSends);
	pServerDE->WriteToMessageDWord(hWrite, m_dwCantDamageTypes);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDamageTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDamageTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDamagerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDeathTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDeathTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrKillerMessage);
	pServerDE->WriteToMessageVector(hWrite, &m_vDeathDir);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastDamageDir);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructable::Load(HMESSAGEREAD hRead, DBYTE nType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hObject);

	m_fHitPoints				= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxHitPoints				= pServerDE->ReadFromMessageFloat(hRead);
	m_fArmorPoints				= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxArmorPoints			= pServerDE->ReadFromMessageFloat(hRead);
	m_fMass						= pServerDE->ReadFromMessageFloat(hRead);
	m_fNextRegen				= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastDamage				= pServerDE->ReadFromMessageFloat(hRead);
	m_fDeathDamage				= pServerDE->ReadFromMessageFloat(hRead);
	m_bDead 					= pServerDE->ReadFromMessageByte(hRead);
	m_bApplyDamagePhysics		= pServerDE->ReadFromMessageByte(hRead);
	m_bPlayer 					= pServerDE->ReadFromMessageByte(hRead);
	m_bCanHeal 					= pServerDE->ReadFromMessageByte(hRead);
	m_bCanRepair 				= pServerDE->ReadFromMessageByte(hRead);
	m_bCanDamage 				= pServerDE->ReadFromMessageByte(hRead);
	m_bNeverDestroy 			= pServerDE->ReadFromMessageByte(hRead);
	m_eDeathType 				= (DamageType) pServerDE->ReadFromMessageByte(hRead);
	m_nDamageTriggerNumSends	= pServerDE->ReadFromMessageDWord(hRead);
	m_dwCantDamageTypes			= pServerDE->ReadFromMessageDWord(hRead);
	m_hstrDamageTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDamageTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDamagerMessage		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDeathTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDeathTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrKillerMessage			= pServerDE->ReadFromMessageHString(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vDeathDir);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastDamageDir);
}

