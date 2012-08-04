//----------------------------------------------------------
//
// MODULE  : Destructable.cpp
//
// PURPOSE : Destructable class
//
// CREATED : 9/23/97
//
//----------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include "BloodServerShell.h"
#include "Destructable.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "Projectile.h"
#include "PlayerObj.h"
#include "Trigger.h"
#include "Spawner.h"
#include "PhysicalAttributes.h"
#include "sfxmsgids.h"
#include "ai_mgr.h"

DLink CDestructable::m_DestructableHead;
DDWORD CDestructable::m_dwNumDestructables = 0;


#define TRIGGER_MSG_KILL		"KILL"

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
//	pOwner					= DNULL;

	m_bDead						= DFALSE;
	m_bApplyDamagePhysics		= DTRUE;
	m_fMass						= 1.0;
	m_fHitPoints				= 1;
	m_fDeathHitPoints			= 1;
	m_fMaxHitPoints				= 1;
	m_fMaxMegaHitPoints			= 1;
	m_fArmorPoints				= 0.0;
	m_fMaxArmorPoints			= 1.0;
	m_fMaxNecroArmorPoints		= 1.0;
	m_fResistance				= 1.0;

	m_nLastDamageType			= DAMAGE_TYPE_NORMAL;
	m_fLastDamagePercent		= 0.0f;
	m_fLastDamageAmount			= 0.0f;
	VEC_INIT(m_vLastDamageDirection);

	m_hstrDamageTriggerTarget	= DNULL;
	m_hstrDamageTriggerMessage	= DNULL;
	m_hstrDeathTriggerTarget	= DNULL;
	m_hstrDeathTriggerMessage	= DNULL;

	m_hstrSpawnObject			= DNULL;
	VEC_INIT(m_vSpawnObjectVel);
	
	m_bGodMode					= DFALSE;
	m_bNighInvulnerable			= DFALSE;
	m_hWhoKilledMeLast			= DNULL;
	m_bTriggerOnly				= DFALSE;

	m_fDeathDelay				= 0.0f;

	m_Link.m_pData				= DNULL;
	if( m_dwNumDestructables == 0 )
	{
		dl_TieOff( &m_DestructableHead );
	}

	m_hLastDamager = DNULL;
	m_nNodeHit = 0;
	m_nSideHit = 0;

	// Init these [gk]
	m_pInventoryMgr	= DNULL;
	m_pAnim_Sound = DNULL;

	m_bAddVelocity = DFALSE;
	VEC_INIT(m_vAddVelocity);

	m_bDestructable			= DTRUE;
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

	if (m_hstrSpawnObject)
	{
		pServerDE->FreeString(m_hstrSpawnObject);
	}

	if( m_Link.m_pData && m_dwNumDestructables > 0 )
	{
		dl_Remove( &m_Link );
		m_dwNumDestructables--;
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
	if (!g_pServerDE) return 0;

	switch(messageID)
	{
		case MID_PRECREATE:
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
			break;

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		case MID_UPDATE:
			{
				m_fLastDamagePercent = 0.0f;	// Reset cumulative damage;
				m_fLastDamageAmount = 0.0f;

				if (m_bDead && m_fDeathDelay > 0)
					m_fDeathDelay -= g_pServerDE->GetFrameTime();

				// Just in case..
				if (m_fHitPoints > m_fMaxMegaHitPoints)
					m_fHitPoints = m_fMaxMegaHitPoints;
				if (m_fArmorPoints > m_fMaxNecroArmorPoints)
					m_fArmorPoints = m_fMaxNecroArmorPoints;
			}
			break;

		case MID_LINKBROKEN:
			{
				HOBJECT hObj = (HOBJECT)pData;
				if (hObj == m_hLastDamager)
				{
					m_hLastDamager = DNULL;
				}
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
		case MID_DAMAGE:
		{
			HandleDamage(hSender, hRead);
			break;
		}

		case MID_REPAIR:
		{
			HandleRepair(hSender, hRead);
			break;
		}

		case MID_HEAL:
		{
			HandleHeal(hSender, hRead);
			break;
		}

		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
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
	GenericProp genProp;
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pStruct || !pServerDE) return DFALSE;

	char buf[MAX_CS_FILENAME_LEN];

	buf[0] = '\0';
	pServerDE->GetPropString("DamageTriggerTarget", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrDamageTriggerTarget = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("DamageTriggerMessage", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrDamageTriggerMessage = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("DeathTriggerTarget", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrDeathTriggerTarget = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("DeathTriggerMessage", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrDeathTriggerMessage = pServerDE->CreateString(buf);

	buf[0] = '\0';
	pServerDE->GetPropString("SpawnObject", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSpawnObject = pServerDE->CreateString(buf);

	pServerDE->GetPropVector("SpawnObjectVel", &m_vSpawnObjectVel);

	pServerDE->GetPropBool("TriggerDestroyOnly", &m_bTriggerOnly);


	// Create empty message strings if there is are trigger targets & no messages
	if (m_hstrDamageTriggerTarget && !m_hstrDamageTriggerMessage)
		m_hstrDamageTriggerMessage = pServerDE->CreateString("");

	if (m_hstrDeathTriggerTarget && !m_hstrDeathTriggerMessage)
		m_hstrDeathTriggerMessage = pServerDE->CreateString("");

	if (g_pServerDE->GetPropGeneric("CanDamage", &genProp) == DE_OK)
	{
		m_bDestructable = genProp.m_Bool;
	}


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Init
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DBOOL CDestructable::Init(HOBJECT hObject, CInventoryMgr* pInv, CAnim_Sound* pAnimSound)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!hObject || !pServerDE) return DFALSE;

	m_hObject = hObject;

	// insert it into the list
	if (!m_Link.m_pData)
	{
		dl_Insert( &m_DestructableHead, &m_Link );
		m_Link.m_pData = ( void * )this;
		m_dwNumDestructables++;
	}

	m_pInventoryMgr = pInv;
	m_pAnim_Sound = pAnimSound;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::CalculateResistance
//
//	PURPOSE:	Handle damage message
// 
// ----------------------------------------------------------------------- //

void CDestructable::CalculateResistance(DBYTE nResistValue)
{
	if (nResistValue == 1)
		m_fResistance = 1.20f;
	else if (nResistValue == 2)
		m_fResistance = 1.10f;
	else if (nResistValue == 3)
		m_fResistance = 1.0f;
	else if (nResistValue == 4)
		m_fResistance = 0.9f;
	else if (nResistValue == 5)
		m_fResistance = 0.8f;
	else if (nResistValue >= 6)
		m_fResistance = 0.7f;
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
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_fMass = fMass;

	pServerDE->SetForceIgnoreLimit(m_hObject, MIN_FORCE);

	// Set the friction based on the mass of the object...

	pServerDE->SetObjectMass(m_hObject, m_fMass);

	CLIPLOWHIGH(fMass, 0, 500.0f);
	DFLOAT fFricCoeff = MIN_FRICTION + (m_fMass * (MAX_FRICTION - MIN_FRICTION)) / 500.0f;
	pServerDE->SetFrictionCoefficient(m_hObject, fFricCoeff);
//	pServerDE->SetFrictionCoefficient(m_hObject, 10.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleDamage
//
//	PURPOSE:	Handle damage message
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleDamage(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DVector vDir,vPos;

	pServerDE->ReadFromMessageVector(hRead, &vDir);
	DFLOAT fDamage = pServerDE->ReadFromMessageFloat(hRead);
	DBYTE nDamageType	= pServerDE->ReadFromMessageByte(hRead);
	HOBJECT hWhoHit = pServerDE->ReadFromMessageObject(hRead);
	pServerDE->ReadFromMessageVector(hRead, &vPos);

//	char buf[50];
	DFLOAT fOldHitPoints = m_fHitPoints;
	fDamage = fDamage * m_fResistance;

	// Just return if we can't take damage
	if (m_bGodMode || (m_bTriggerOnly && nDamageType != DAMAGE_TYPE_DEATH) || fDamage < 0)
	{
		return;
	}

	if (!IsPlayerToPlayerDamageOk(hSender, hWhoHit))
	{
		return;
	}

	// If instant death, don't bother calculating stuff.. 
	if( m_bDestructable && nDamageType == DAMAGE_TYPE_DEATH)
	{
		m_fArmorPoints = 0;
		m_fHitPoints = 0;
		m_fDeathHitPoints = 0;
		m_fLastDamagePercent = 100.0f;
		m_fLastDamageAmount = (DFLOAT)m_fMaxHitPoints;

		nDamageType &= 0x0f;	// Mask off the damage type flags
		m_nLastDamageType = nDamageType;
	}
	else
	{
		// Special pre-damage base character modifiers
		if (IsAICharacter(m_hObject))
		{
			m_nNodeHit = CalculateHitLimb(vDir,vPos,fDamage);

			if(m_nNodeHit == -1 && !(nDamageType & DAMAGE_TYPE_NORMAL))
				m_nNodeHit = SetProperNode(pServerDE->IntRandom(0,NUM_ALL_NODES - 3));
			else if(m_nNodeHit >= 0)
				m_nNodeHit = SetProperNode(m_nNodeHit);
			else
				return;

			//Compute one side got hit for recoils
			DVector vU, vR, vF, vTmpDir;
			DRotation rRot;
			pServerDE->GetObjectRotation(m_hObject, &rRot);
			pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

			VEC_COPY(vTmpDir, vDir);

			VEC_MULSCALAR(vTmpDir,vTmpDir,-1.0f);
			DFLOAT fAmount = (DFLOAT) atan2(vTmpDir.x, vTmpDir.z);    
			DFLOAT fAmount2 = (DFLOAT) atan2(vF.x, vF.z);

			if(fAmount < 0.0f)
				fAmount = (MATH_PI*2) + fAmount;
			if(fAmount2 < 0.0f)
				fAmount2 = (MATH_PI*2) + fAmount2;

			DFLOAT fAngle = fAmount2 - fAmount;

			if(fAngle <= MATH_PI/2 || fAngle >= 3*MATH_PI/2)	//Hit the front
			{
				m_nSideHit = 0;
			}
			else //Hit the back
			{
				m_nSideHit = 6;	
			}

			if (m_hLastDamager)
				pServerDE->BreakInterObjectLink(m_hObject, m_hLastDamager);

			m_hLastDamager = hWhoHit;

			if(m_hLastDamager)
				pServerDE->CreateInterObjectLink(m_hObject, m_hLastDamager);

			if(m_nNodeHit == NODE_NECK)
			{
				AI_Mgr* pAI = (AI_Mgr*)pServerDE->HandleToObject(m_hObject);

				if(pAI->m_bCabal || pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hObject), pServerDE->GetClass("SoulDrudge")))
					m_fHitPoints = 0;
				
				fDamage *= 1.5f;
			}
			else if(m_nNodeHit == NODE_TORSO)
			{
				fDamage *= 1.0f;
			}
			else
			{
				fDamage *= 0.5f;
			}
		}

		if (IsBaseCharacter(m_hObject))
		{
			CBaseCharacter *pOwner = (CBaseCharacter*)pServerDE->HandleToObject(m_hObject);

//			if (pOwner->IsItemActive(SPELL_STONE))
//				return;

			// If Nigh-invulnerability powerup in effect..
			if (m_bNighInvulnerable)
				fDamage = fDamage * 0.05f;

			// Shield absorbs the damage as Focus ammo use
/*			if (nDamageType != DAMAGE_TYPE_DEATH && pOwner->IsItemActive(SPELL_SHIELD))
			{
				DFLOAT fFocusAmmo = pOwner->GetInventoryMgr()->GetAmmoCount(AMMO_FOCUS);
		
				fFocusAmmo -= fDamage;

				if (fFocusAmmo < 0.0f)
				{
					fDamage = -fFocusAmmo;
					fFocusAmmo = 0.0f;
				}
				else
				{
					fDamage = 0.0f;
				}

				pOwner->GetInventoryMgr()->SetAmmoCount(AMMO_FOCUS, fFocusAmmo);
			}
*/
			// Reflection reflects damage back to the sender, and absorbs 
			// twice the damage in focus ammo
/*			if (pOwner->IsItemActive(SPELL_REFLECTION) && !(nDamageType & DAMAGE_FLAG_AREAEFFECT))
			{
				DFLOAT fFocusAmmo = pOwner->GetInventoryMgr()->GetAmmoCount(AMMO_FOCUS);
		
				fFocusAmmo -= fDamage * 2.0f;

				if (fFocusAmmo < 0.0f)
				{
					fDamage = -fFocusAmmo / 2.0f;
					fFocusAmmo = 0.0f;
				}
				else
				{
					fDamage = 0.0f;
				}

				pOwner->GetInventoryMgr()->SetAmmoCount(AMMO_FOCUS, fFocusAmmo);
			}
*/
		}

		// If single player, don't let the player apply damage to himself.
		if (m_bApplyDamagePhysics && !(g_pBloodServerShell->GetGameType() == GAMETYPE_SINGLE && (m_hObject == hWhoHit) && IsPlayer(m_hObject)))
			ApplyDamagePhysics(fDamage, &vDir);
		
		// Can't damage if already dead...

		if( m_bDestructable && m_bDead )
			return;

		if( m_bDestructable && m_fArmorPoints > 0.0 && nDamageType != DAMAGE_TYPE_SUFFOCATE )
		{
			DFLOAT fAbsorb = 0.0f;

			if (m_fArmorPoints <= 25.0f)
				fAbsorb = fDamage * 0.3f;
			else if (m_fArmorPoints <= 50.0f)
				fAbsorb = fDamage * 0.5f;
			else if (m_fArmorPoints <= 100.0f)
				fAbsorb = fDamage * 0.7f;
			else if (m_fArmorPoints <= 150.0f)
				fAbsorb = fDamage * 0.8f;
			else 
				fAbsorb = fDamage * 0.9f;

			if (!m_bGodMode)
			{
				m_fArmorPoints -= fAbsorb;
				if (m_fArmorPoints < 0.0f) 
				{
					fAbsorb += m_fArmorPoints;
					m_fArmorPoints = 0.0f;
				}
        
				fDamage -= fAbsorb;
       		}
		}

		if (fDamage < 0.0f) fDamage = 0.0f;	// just to be sure :)

		// Save damage type so entity will know how to react
		nDamageType &= 0x0f;	// Mask off the damage type flags
		if (fDamage) m_nLastDamageType = nDamageType;

		if( m_bDestructable )
		{
			m_fHitPoints -= fDamage;
			m_fDeathHitPoints -= fDamage;
		}
	}
	// 01/13/98  How much damage was done (percentage of max hit points)
	m_fLastDamagePercent += (DFLOAT)fDamage/(DFLOAT)GetMaxHitPoints();
	m_fLastDamageAmount += fDamage;
	VEC_COPY(m_vLastDamageDirection, vDir);

	// Set pSender if sender is a player
	CPlayerObj* pSender = DNULL;

	if( m_bDestructable )
	{
		if(hWhoHit)
			if(pServerDE->IsKindOf(pServerDE->GetObjectClass(hWhoHit), pServerDE->GetClass("CPlayerObj")))
				pSender = (CPlayerObj*)pServerDE->HandleToObject(hWhoHit);

		// If it was hurt with a leech weapon, then add the damage done back to playerobj
		if(pSender && pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hObject), pServerDE->GetClass("CBaseCharacter")))
		{
			if(nDamageType == DAMAGE_TYPE_LEECH)
			{
				if (!pSender->IsDead() && !pSender->IsInSlowDeath())
				{
					pSender->GetDestructable()->Heal(fDamage / 10.0f);
				}
			}
		}
	}

//	m_fHitPoints = 1;
	if( m_bDestructable && m_fHitPoints <= 0 )
	{
		m_bDead = DTRUE;
		m_fHitPoints = 0;
		HandleDestruction();

		m_hWhoKilledMeLast = hWhoHit;

        // If its a PlayerObj then Increase the Kills 
/*		if(pSender && pServerDE->IsKindOf(pServerDE->GetObjectClass(m_hObject), pServerDE->GetClass("CBaseCharacter"))) 
		{
            // If it was killed with a melee weapon, then add the damage done back to playerobj
        	if(nDamageType == DAMAGE_TYPE_LEECH)
            {
				pSender->GetDestructable()->Heal(fDamage / 5.0f);
				pSender->AddMeleeKill();

				DFLOAT fIncrease = 0.0f;
				int nKills = pSender->GetMeleeKills();
        
				if (nKills > 0)
				{
                    // Increase Damage by 5% for every kill
                    fIncrease = (DFLOAT)nKills * .05f;
                    // Max Increase 200%
                    if (fIncrease > 2.0f)   fIncrease = 2.0f;
                }            
                
                // If the Increase is greater than 50% then add back some of the damage to player
                if (fIncrease > 0.5f)
                {
                    // Add back damage to playerobj
                    DFLOAT fAddHits = fDamage * (fIncrease/2.0f);
                    
    				pSender->GetDestructable()->Heal(fAddHits);
                    
                    // Need to Glow the player Sword...
                    // Set Glow on the Melee Weapon
                    
                }
            }
        }*/
	}


	// If this is supposed to send a damage trigger, send it now..

	if( m_bDestructable && m_hstrDamageTriggerTarget && m_hstrDamageTriggerMessage )
	{
		LPBASECLASS pD = pServerDE->HandleToObject(m_hObject);
		SendTriggerMsgToObjects(pD, m_hstrDamageTriggerTarget, m_hstrDamageTriggerMessage);
	}

	// If player did this damage and has soul stealing binding..
	if( m_bDestructable && fOldHitPoints != m_fHitPoints && m_hObject != hWhoHit)
	{
		// Make sure it's a player
		if(pSender) 
		{
			DFLOAT fHeal;
			if (pSender->HasSoulStealingBinding() && (fHeal = (fOldHitPoints - m_fHitPoints) / 10.0f))
			{
				pSender->GetDestructable()->Heal(fHeal);
			}
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleTrigger()
//
//	PURPOSE:	Handler for trigger messages - Handles the KILL trigger
//
// --------------------------------------------------------------------------- //

void CDestructable::HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	HSTRING hMsg = pServerDE->ReadFromMessageHString(hRead);

	char* pMsg = pServerDE->GetStringData(hMsg);
	if (!pMsg || !pMsg[0]) return;

	if (_mbsicmp((const unsigned char*)pMsg, (const unsigned char*)TRIGGER_MSG_KILL) == 0)
	{
		DVector vTmp;
		VEC_INIT(vTmp);
		BaseClass *ffObj = pServerDE->HandleToObject(hSender);
		DamageObject(hSender, ffObj, m_hObject, 1, vTmp, vTmp, DAMAGE_TYPE_DEATH);
	}
	pServerDE->FreeString(hMsg);
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CDestructable::SetProperNode
// DESCRIPTION	: Set the hit node to the parent node of the limb
// RETURN TYPE	: int 
// PARAMS		: int nNode
// ----------------------------------------------------------------------- //

int CDestructable::SetProperNode(int nNode)
{
	switch(nNode)
	{
		case 0:		return NODE_NECK;
		case 1:		return NODE_NECK;
		case 2:		return NODE_TORSO;
		case 3:		return NODE_TORSO;
		case 4:		return NODE_RARM;
		case 5:		return NODE_RARM;
		case 6:		return NODE_RARM;
		case 7:		return NODE_LARM;
		case 8:		return NODE_LARM;
		case 9:		return NODE_LARM;
		case 10:	return NODE_LLEG;
		case 11:	return NODE_LLEG;
		case 12:	return NODE_LLEG;
		case 13:	return NODE_LLEG;
		case 14:	return NODE_RLEG;
		case 15:	return NODE_RLEG;
		case 16:	return NODE_RLEG;
		case 17:	return NODE_RLEG;
		default:	return -1;
	}

	return -1;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CDestructable::CalculateHitLimb
// DESCRIPTION	: 
// RETURN TYPE	: int
// ----------------------------------------------------------------------- //

int CDestructable::CalculateHitLimb(DVector vDir, DVector vPos, DFLOAT fDamage)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject || !m_pInventoryMgr || !m_pAnim_Sound) return -1;

	int nNode = -1;
	DFLOAT fNodeDist = 0.0f, fDist = 999.0f, fTemp = 0.0f;
	DVector vShot, vNewShot, vTemp, vObjDims, vNodePos, vZ;
	DFLOAT fX, fY, ft;
	DBOOL bStatus = DFALSE;
	DRotation rRot;

	if(pServerDE->GetModelAnimUserDims(m_hObject, &vObjDims, pServerDE->GetModelAnimation(m_hObject)) == DE_INVALIDPARAMS)
		pServerDE->DebugOut("CalculateHitLimb() fucked up\r\n");

	vTemp.x = (float)fabs(vDir.x);
	vTemp.y = (float)fabs(vDir.y);
	vTemp.z = (float)fabs(vDir.z);

	if(vTemp.x > vTemp.y && vTemp.x > vTemp.z)
	{
		fTemp = vObjDims.x / vTemp.x;
	}
	else if(vTemp.y > vTemp.x  && vTemp.y > vTemp.z)
	{
		fTemp = vObjDims.y / vTemp.y;
	}
	else if(vTemp.z > vTemp.x  && vTemp.z > vTemp.y)
	{
		fTemp = vObjDims.z / vTemp.z;
	}

	VEC_MULSCALAR(vNewShot,vDir,fTemp);
	VEC_ADD(vShot,vPos,vNewShot);

	DVector vC;
	VEC_SUB(vC,vShot,vPos);

	fX = 1 / VEC_DOT(vC,vC);
	fY = fX * -(VEC_DOT(vC,vPos));
	
	for(int i = 0; i < NUM_STD_NODES; i++)
	{
		pServerDE->GetModelNodeHideStatus(m_hObject, szNodes[i], &bStatus);

		if(!bStatus)
		{
			DBOOL bRet = pServerDE->GetModelNodeTransform(m_hObject, szNodes[i], &vNodePos, &rRot);

			ft = VEC_DOT(vC,vNodePos) * fX + fY;

			if(ft >= 0.0f && ft <= 1.0f)
			{
				VEC_ADDSCALED(vZ,vPos,vC, ft);

				fNodeDist = VEC_DIST(vNodePos, vZ);

				if(fNodeDist < fDist && fNodeDist <= m_pAnim_Sound->m_fHitSpheres[i])
				{
					fDist = fNodeDist;
					nNode = i;
				}
			}
		}
	}
/*
	//Do we leave a pass through mark behind us?	
	if(nNode != -1)
	{
		CWeapon *pW = m_pInventoryMgr->GetCurrentWeapon();

		if(pW)
		{
			VEC_MULSCALAR(vTemp,vDir,-1.0f);
			// TODO: combine sparks with weaponFX GK 8/27
//			pW->AddSparks(vPos, vTemp, fDamage * 2.0f, m_hObject, SURFTYPE_FLESH);	
//			pW->AddBloodSpurt(vPos, vTemp, fDamage * 2.0f, m_hObject, SURFTYPE_FLESH);	

//			Took this out - more efficient to send one message. GK 8/27
//			vTemp.x *= -1.0f;
//			vTemp.z *= -1.0f;
//			pW->AddBloodSpurt(vPos, vTemp, fDamage * 2.0f, m_hObject, SURFTYPE_FLESH);	

			IntersectQuery	iq;
			IntersectInfo	ii;

			// Set the intersection query values
			iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			iq.m_FilterFn = DNULL;
			iq.m_pUserData = DNULL;

			VEC_COPY(iq.m_From, vPos);
			VEC_ADDSCALED(iq.m_To, vPos, vDir, 75.0f);

			// Apply a blood splat to the wall
			if(pServerDE->IntersectSegment(&iq, &ii) && (ii.m_hObject == pServerDE->GetWorldObject()))
			{
//				pW->AddImpact(WFX_BLOODSPLAT, ii.m_Point, vDir, ii.m_Plane.m_Normal, fDamage * 2.0f, 
//							  ii.m_hObject, SURFTYPE_FLESH);
//				pW->AddSparks(ii.m_Point, ii.m_Plane.m_Normal, fDamage * 2.0f, ii.m_hObject, SURFTYPE_FLESH);	
			}
		}
	}
*/
	return nNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::ApplyDamagePhysics
//
//	PURPOSE:	Applies physics when hit
//
// ----------------------------------------------------------------------- //

void CDestructable::ApplyDamagePhysics(DFLOAT fDamage, DVector *pvDir)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hObject || !pvDir) return;

	// Don't apply damage physics if the object is a trapped character (Andy 2/22/99)
	if(IsBaseCharacter(m_hObject))
	{
		CBaseCharacter *pObj = (CBaseCharacter*)pServerDE->HandleToObject(m_hObject);
		if(pObj->IsTrapped()) return;
	}

	if (VEC_MAGSQR(*pvDir) < 0.01) return;

	DVector vTemp, vVel;

	pServerDE->GetVelocity(m_hObject, &vVel);

	VEC_COPY(vTemp, *pvDir);
	VEC_NORM(vTemp);

	if (m_fMass <= 0) m_fMass = 1;
	
	DFLOAT fMultiplier = (fDamage * PA_DAMAGE_VEL_MUTLIPLIER) / m_fMass;

	VEC_MULSCALAR(vTemp, vTemp, fMultiplier);
	VEC_ADD(vVel, vTemp, vVel);

	// Accumulate damage velocity for player objects to send to the client..
	if (IsPlayer(m_hObject))
	{
		VEC_ADD(m_vAddVelocity, m_vAddVelocity, vTemp);
		m_bAddVelocity = DTRUE;
	}

	pServerDE->SetVelocity(m_hObject, &vVel);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleDestruction
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

void CDestructable::HandleDestruction()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (m_hstrDeathTriggerTarget)
	{
		LPBASECLASS pD = pServerDE->HandleToObject(m_hObject);
		SendTriggerMsgToObjects(pD, m_hstrDeathTriggerTarget, m_hstrDeathTriggerMessage);
	}

	// See if we need to spawn anything
	if (m_hstrSpawnObject)
	{
		DVector vPos;
		DRotation rRot;

		pServerDE->GetObjectPos(m_hObject, &vPos);
		pServerDE->GetObjectRotation(m_hObject, &rRot);

		SpawnObject(pServerDE->GetStringData(m_hstrSpawnObject), &vPos, &rRot, &m_vSpawnObjectVel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleHeal
//
//	PURPOSE:	Handle heal message.
// 
// ----------------------------------------------------------------------- //

void CDestructable::HandleHeal(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DFLOAT fAmount = pServerDE->ReadFromMessageFloat(hRead);

	Heal(fAmount);
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
	// Sanity checks...

	if (m_fHitPoints >= m_fMaxHitPoints)
		return DFALSE;


	// Check if this is a humiliated player in a multiplayer game...

	if (g_pBloodServerShell->IsMultiplayerGame())
	{
		if (IsPlayer(m_hObject))
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pServerDE->HandleToObject(m_hObject);
			if (pPlayer && pPlayer->IsInSlowDeath())
			{
				return(DFALSE);
			}
		}
	}


	// Heal...

	m_bDead = DFALSE;

	m_fHitPoints += fAmount;
	if(m_fHitPoints > m_fMaxHitPoints)
		m_fHitPoints = m_fMaxHitPoints;
	m_fDeathHitPoints = m_fHitPoints;


	// All done...

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::MegaHeal
//
//	PURPOSE:	Add some value to hit points
// 
// ----------------------------------------------------------------------- //

DBOOL CDestructable::MegaHeal(DFLOAT fAmount)
{
	if (m_fHitPoints >= m_fMaxMegaHitPoints)
		return DFALSE;

	m_bDead = DFALSE;

	m_fHitPoints += fAmount;
	if(m_fHitPoints > m_fMaxMegaHitPoints)
		m_fHitPoints = m_fMaxMegaHitPoints;
	m_fDeathHitPoints = m_fHitPoints;
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::HandleRepair()
//
//	PURPOSE:	Handle Repair message
//
// ----------------------------------------------------------------------- //

void CDestructable::HandleRepair(HOBJECT hSender, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	DFLOAT fAmount = pServerDE->ReadFromMessageFloat(hRead);

	AddWard(fAmount);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::AddWard()
//
//	PURPOSE:	Add some value to armor
//
// ----------------------------------------------------------------------- //
DBOOL CDestructable::AddWard(DFLOAT fAmount)
{
	if (m_fArmorPoints >= m_fMaxArmorPoints)
		return DFALSE;

	m_fArmorPoints += fAmount;

	if(m_fArmorPoints > m_fMaxArmorPoints) 
		m_fArmorPoints = m_fMaxArmorPoints;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::AddNecroWard()
//
//	PURPOSE:	Add some value to armor
//
// ----------------------------------------------------------------------- //
DBOOL CDestructable::AddNecroWard(DFLOAT fAmount)
{
	if (m_fArmorPoints >= m_fMaxNecroArmorPoints)
		return DFALSE;

	m_fArmorPoints += fAmount;

	if(m_fArmorPoints > m_fMaxNecroArmorPoints) 
		m_fArmorPoints = m_fMaxNecroArmorPoints;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Reset
//
//	PURPOSE:	Reset
//
// ----------------------------------------------------------------------- //

void CDestructable::Reset()
{
	AddWard(m_fMaxArmorPoints);
	Heal(m_fMaxHitPoints);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CDestructable::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;
	pServerDE->WriteToMessageDWord(hWrite, 0xcececece);

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hObject);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bApplyDamagePhysics);
	pServerDE->WriteToMessageFloat(hWrite, m_fMass);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxHitPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxMegaHitPoints);
	if( m_bDestructable )
		pServerDE->WriteToMessageFloat(hWrite, m_fHitPoints);
	else
		pServerDE->WriteToMessageFloat( hWrite, 1.0e6f );
	pServerDE->WriteToMessageFloat(hWrite, m_fDeathHitPoints);

	pServerDE->WriteToMessageFloat(hWrite, m_fMaxArmorPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fMaxNecroArmorPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fArmorPoints);
	pServerDE->WriteToMessageFloat(hWrite, m_fResistance);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bDead);
	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bNighInvulnerable);

	pServerDE->WriteToMessageByte(hWrite, m_nLastDamageType);
	pServerDE->WriteToMessageByte(hWrite, m_nLastDamageLocation);

	pServerDE->WriteToMessageVector(hWrite, &m_vLastDamageDirection);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastDamageAmount);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastDamagePercent);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDamageTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDamageTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDeathTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDeathTriggerMessage);

	pServerDE->WriteToMessageHString(hWrite, m_hstrSpawnObject);
	pServerDE->WriteToMessageVector(hWrite, &m_vSpawnObjectVel);

	pServerDE->WriteToMessageByte(hWrite, m_bGodMode);
	pServerDE->WriteToMessageByte(hWrite, m_bTriggerOnly);

	pServerDE->WriteToMessageFloat(hWrite, m_fDeathDelay);

	pServerDE->WriteToMessageDWord(hWrite, 0xcececece);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CDestructable::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	DDWORD dwTest						= pServerDE->ReadFromMessageDWord(hRead);

	// Gross hack because PlayerObj hObject deoesn't seem to be loaded.
	// Don't reload m_hObject if it is already set to something.
	HOBJECT hObjectTmp;
	if (!m_hObject)
		pServerDE->ReadFromLoadSaveMessageObject(hRead, &m_hObject);
	else
		pServerDE->ReadFromLoadSaveMessageObject(hRead, &hObjectTmp);
	m_bApplyDamagePhysics		= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	m_fMass						= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxHitPoints				= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxMegaHitPoints			= pServerDE->ReadFromMessageFloat(hRead);
	m_fHitPoints				= pServerDE->ReadFromMessageFloat(hRead);
	m_fDeathHitPoints			= pServerDE->ReadFromMessageFloat(hRead);

	m_fMaxArmorPoints			= pServerDE->ReadFromMessageFloat(hRead);
	m_fMaxNecroArmorPoints		= pServerDE->ReadFromMessageFloat(hRead);
	m_fArmorPoints				= pServerDE->ReadFromMessageFloat(hRead);
	m_fResistance				= pServerDE->ReadFromMessageFloat(hRead);
	m_bDead						= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	m_bNighInvulnerable			= (DBOOL)pServerDE->ReadFromMessageByte(hRead);

	m_nLastDamageType			= pServerDE->ReadFromMessageByte(hRead);
	m_nLastDamageLocation		= pServerDE->ReadFromMessageByte(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastDamageDirection);
	m_fLastDamageAmount			= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastDamagePercent		= pServerDE->ReadFromMessageFloat(hRead);
	m_hstrDamageTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDamageTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDeathTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDeathTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);

	m_hstrSpawnObject			= pServerDE->ReadFromMessageHString(hRead);
	pServerDE->ReadFromMessageVector(hRead, &m_vSpawnObjectVel);

	m_bGodMode					= pServerDE->ReadFromMessageByte(hRead);
	m_bTriggerOnly				= pServerDE->ReadFromMessageByte(hRead);

	m_fDeathDelay				= pServerDE->ReadFromMessageFloat(hRead);

	m_bDestructable = ( m_fHitPoints >= 1.0e6f ) ? DFALSE : DTRUE;

	dwTest						= pServerDE->ReadFromMessageDWord(hRead);

	if (!m_Link.m_pData)
	{
		dl_Insert( &m_DestructableHead, &m_Link );
		m_Link.m_pData = ( void * )this;
		m_dwNumDestructables++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::IsPlayerToPlayerDamageOk
//
//	PURPOSE:	Determines if it's ok for a player to damage this player
//
// ----------------------------------------------------------------------- //

DBOOL CDestructable::IsPlayerToPlayerDamageOk(HOBJECT hDamager, HOBJECT hOwner)
{
	// Sanity checks...

	if (m_hObject == hDamager) return(DTRUE);
	if (m_hObject == hOwner) return(DTRUE);


	// Check if we are playing a multiplayer game that has the right options set...

	if (!g_pBloodServerShell->IsMultiplayerTeamBasedGame()) return(DTRUE);
	if (g_pBloodServerShell->GetNetGameInfo()->m_bFriendlyFire) return(DTRUE);


	// Make sure we're a player...

	if (!IsPlayer(m_hObject)) return(DTRUE);


	// Get the attacker player for this damager or projectile...

	HOBJECT hHitter = DNULL;

	if (IsPlayer(hDamager))
	{
		hHitter = hDamager;
	}
	else if (IsPlayer(hOwner))
	{
		hHitter = hOwner;
	}

	if (!hHitter) return(DTRUE);


	// Get the player pointers...

	CPlayerObj* pVictim = (CPlayerObj*)g_pServerDE->HandleToObject(m_hObject);
	if (!pVictim) return(DTRUE);

	CPlayerObj* pDamager = (CPlayerObj*)g_pServerDE->HandleToObject(hHitter);
	if (!pDamager) return(DTRUE);


	// Check if these players are on the same team...

	if (pVictim->GetTeamID() == pDamager->GetTeamID()) return(DFALSE);


	// If we get here, it's ok to do the damage...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDestructable::SetStartingCharacterValues
//
//	PURPOSE:	Sets the starting values for a player character
//
// ----------------------------------------------------------------------- //

void CDestructable::SetStartingCharacterValues()
{
	// Sanity checks...

	if (!m_hObject) return;
	if (!IsPlayer(m_hObject)) return;


	// Set the health...

	m_fHitPoints = 100;


	// Set the armor...

	m_fArmorPoints = 0;
}






