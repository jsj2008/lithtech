// ----------------------------------------------------------------------- //
//
// MODULE  : BaseAI.cpp
//
// PURPOSE : Generic base AI object - Implementation
//
// CREATED : 9/29/97
//
// ----------------------------------------------------------------------- //

#include "BaseAI.h"
#include "cpp_server_de.h"
#include "RiotObjectUtilities.h"
#include "Destructable.h"
#include "Weapons.h"
#include "generic_msg_de.h"
#include "ClientCastLineSFX.h"
#include "AIPathList.h"
#include "VolumeBrushTypes.h"
#include "PVWeaponModel.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "AISounds.h"
#include "PlayerObj.h"
#include "RiotServerShell.h"

#include <stdio.h>


// Externs...

extern char g_tokenSpace[];
extern char *g_pTokens[];
extern char *g_pCommandPos;

extern CRiotServerShell* g_pRiotServerShellDE;
extern DBOOL g_bRobert;

// Defines....

#define AI_DEFAULT_MAX_CHEESE_COUNT		3.0f
#define AI_MAXIMUM_BLOCK_PRIORITY		254

#define AI_DEFAULT_SOUND_RADIUS			2000.0f
#define AI_DEACTIVATION_TIME			10.0f
#define AI_SCRIPTMOVEMENT_ERROR			20.0f
#define AI_FOLLOW_DISTANCE				100.0f
#define AI_MAX_FOLLOW_DISTANCE			600.0f
#define AI_SAFE_DISTANCE				100.0f
#define AI_RETREAT_DISTANCE				500.0f

#define AI_UPDATE_DELTA					0.01f
#define AI_MAX_DISTANCE					100000.0f
#define AI_DEFAULT_SIGHT_DISTANCE		2000.0f
#define AI_DEFAULT_HEAR_DISTANCE		100.0f
#define AI_DEFAULT_FIRE_DELAY			1.5f
#define	AI_MIN_SOUND_DELAY				5.0f
#define	AI_MAX_SOUND_DELAY				20.0f

#define AI_MIN_IDLE_TIME				0.0f			
#define AI_MIN_AGGRESSIVE_TIME			40.0f			
#define AI_MIN_DEFENSIVE_TIME			40.0f
#define AI_MIN_RETREATING_TIME			15.0f	
#define AI_MIN_PANICKED_TIME			20.0f	
#define AI_MIN_PSYCHO_TIME				20.0f	

#define AI_ATTACK_CHANCE_FRIEND			-1.0f
#define AI_ATTACK_CHANCE_TOLERATE		50.0f

#define TRIGGER_SCRIPT					"SCRIPT"
#define TRIGGER_STYPE_INTERRUPTABLE		"INTERRUPTABLE"

#define SCRIPT_MOVEMENT_WALK			"WALK"
#define SCRIPT_MOVEMENT_RUN				"RUN"
#define SCRIPT_LOOP_ANIMATIONS			"LOOP"

#define KEY_FIRE_WEAPON					"FIRE_KEY"

#define ANIM_PANICKED					"SP12"

// Define our properties (what is available in DEdit)...

BEGIN_CLASS(BaseAI)
	ADD_BOOLPROP(Large, DFALSE)
	ADD_STRINGPROP(SpotTriggerTarget, "")
	ADD_STRINGPROP(SpotTriggerMessage, "")
	ADD_LONGINTPROP(SpotTriggerNumSends, 1)
	ADD_STRINGPROP(LostTargetTriggerTarget, "")
	ADD_STRINGPROP(LostTargetTriggerMessage, "")
	ADD_LONGINTPROP(LostTargetTriggerNumSends, 1)
	ADD_STRINGPROP(BumpedTriggerTarget, "")
	ADD_STRINGPROP(BumpedTriggerMessage, "")
	ADD_LONGINTPROP(BumpedTriggerNumSends, 1)

	PROP_DEFINEGROUP(AvailableSounds, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SetIdle, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SetDefensive, 1, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SetAggressive, 1, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SetRetreating, 1, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SetPanicked, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(SetPsycho, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Spot, 1, PF_GROUP1)
		ADD_BOOLPROP_FLAG(LostTarget, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Death, 1, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Idle, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Defensive, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Aggressive, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Retreating, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Guarding, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Panicked, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Psycho, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(FollowLost, 0, PF_GROUP1)
		ADD_BOOLPROP_FLAG(Bumped, 1, PF_GROUP1)

	PROP_DEFINEGROUP(Sounds, PF_GROUP2)
		ADD_STRINGPROP_FLAG(SetIdleSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(SetDefensiveSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(SetAggressiveSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(SetRetreatingSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(SetPanickedSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(SetPsychoSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(SpotSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(LostTargetSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(DeathSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(IdleSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(DefensiveSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(AggressiveSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(RetreatingSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(GuardingSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PanickedSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(PsychoSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(FollowLostSound, "", PF_GROUP2)
		ADD_STRINGPROP_FLAG(BumpedSound, "", PF_GROUP2)

	ADD_LONGINTPROP(State, BaseAI::AGGRESSIVE)

	PROP_DEFINEGROUP(AvailableStates, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateIdle, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateDefensive, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateAggressive, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateRetreating, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StateGuarding, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StatePanicked, 1, PF_GROUP3)
		ADD_BOOLPROP_FLAG(StatePsycho, 1, PF_GROUP3)

	ADD_STRINGPROP(ObjectOfDesire, "")

	ADD_LONGINTPROP(Bravado, BaseAI::BRAVADO2)
	ADD_LONGINTPROP(Experience, BaseAI::EXPERIENCE2)
	ADD_LONGINTPROP(Marksmanship, BaseAI::MARKSMANSHIP4)
	ADD_LONGINTPROP(Evasive, BaseAI::EVASIVE2)
	ADD_LONGINTPROP(Condition, BaseAI::HEALTHY)
	ADD_LONGINTPROP(WeaponId, GUN_PULSERIFLE_ID)
	ADD_REALPROP_FLAG(VisibleRange, AI_DEFAULT_SIGHT_DISTANCE, PF_RADIUS)
	ADD_REALPROP_FLAG(HearingRange, AI_DEFAULT_HEAR_DISTANCE, PF_RADIUS)
	ADD_REALPROP_FLAG(SoundRadius, AI_DEFAULT_SOUND_RADIUS, PF_RADIUS)

END_CLASS_DEFAULT(BaseAI, CBaseCharacter, NULL, NULL)


static int s_nNumCallsToIntersectSegment = 0;

// This is a filter function used with cast ray...

static DBOOL TransparentObjectFilterFn(HOBJECT hObj, void *pUserData);


// Following are tables used to calculate state transitions...

static int s_nStateTransitionTable[7][2] =
{// low, high
	{-1, -1},		// IDLE - not used
	{21, 55},		// DEFENSIVE
	{56, 95},		// AGGRESIVE
	{6,  20},		// RETREATING
	{-1, -1},		// GAURDING - not used
	{0, 5},			// PANICKED 
	{96, 100}		// PSYCHO
};

static int s_nStateConditionMedian = 50;
static int s_nStateConditionAdjust[3][2] =
{  // less than median, greater than median
	{0, 0},		// HEALTHY
	{10, -10},	// COCKY
	{-10, 10}	// WOUNDED
};


static int s_nStateBravadoMedian = 50;
static int s_nStateBravadoAdjust[5][2] =
{  // less than media, greater than media
	{-10, -40},	
	{0, 0},
	{20, 0},	
	{30, 5},
	{40, 10}
};

static int s_nStateExperienceMedian = 50;
static int s_nStateExperienceAdjust[5][2] =
{  // less than media, greater than media
	{-20, 20},	
	{0, 0},
	{10, -5},	
	{20, -10},
	{30, -20}
};

static int s_nMarksmanshipPerturbe[6][2] =
{  
	{0,  32},	// Marksmanship1 = EXPERT
	{32, 64},
	{64, 96},	
	{96, 128},
	{96, 160},	
	{128, 160}	// Marksmanship6 = Jason Hall
};

static DFLOAT s_fEvasiveDelay[6][2] =
{  
	{0.0f, 0.5f},		// UnReal
	{0.25f, 1.0f},		// Very Evasive
	{1.0f, 2.0f},		// Evasive
	{2.0f, 3.0f},		// Not really evasive
	{3.0f, 5.0f},		// Lump O dirt
	{100.0f, 200.0f}	// Jason Hall (i.e., can't evade)
};




//*************************************************************************//
//*************************************************************************//
//
// E N G I N E   R E L A T E D   F U N C T I O N A L I T Y
// 
// (construction, destruction, initialization, engine callbacks)
//*************************************************************************//
//*************************************************************************//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::BaseAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

BaseAI::BaseAI() : CBaseCharacter()
{
	VEC_INIT(m_vRight);
	VEC_INIT(m_vUp);
	VEC_INIT(m_vForward);
	VEC_INIT(m_vLastPos);

	m_AIPathList.Init(DFALSE);

	m_dwAction					= 0;
	m_dwLastAction				= 0;
	m_hTarget					= DNULL;
	m_hLastDamager				= DNULL;

	m_eState					= AGGRESSIVE;
	m_eCondition				= HEALTHY;
	m_eBravado					= BRAVADO2;
	m_eExperience				= EXPERIENCE2;
	m_eMarksmanship				= MARKSMANSHIP4;
	m_eEvasive					= EVASIVE2;

	SetStateFlag(AI_SFLG_ALLSTATES);

	m_nWeaponId					= GUN_PULSERIFLE_ID;

	m_fVisibleRange				= AI_DEFAULT_SIGHT_DISTANCE;
	m_fHearingRange				= AI_DEFAULT_HEAR_DISTANCE;
	m_fSoundRadius				= AI_DEFAULT_SOUND_RADIUS;
	m_fNextSoundTime			= AI_MIN_SOUND_DELAY;

	m_fCurTime					= 0.0f;
	m_fRecomputeTime			= 0.0f;
	m_fFireStartTime			= 0.0f;
	m_fFireStopTime				= 0.0f;
	m_fNextBumpedTime			= 0.0f;

	m_bRecompute				= DFALSE;
	m_eLastTurnDir				= NONE;
	m_bSpottedPlayer			= DFALSE;
	m_bLostPlayer				= DFALSE;
	m_bStuckOnSomething			= DFALSE;
	m_bSearchingForPlayer		= DFALSE;

	m_hstrSpotTriggerTarget		= DNULL;
	m_hstrSpotTriggerMessage	= DNULL;
	m_nSpotTriggerNumSends		= 1;

	m_hstrLostTargetTriggerTarget	= DNULL;
	m_hstrLostTargetTriggerMessage	= DNULL;
	m_nLostTargetTriggerNumSends	= 1;

	m_hstrBumpedTriggerTarget	= DNULL;
	m_hstrBumpedTriggerMessage	= DNULL;
	m_nBumpedTriggerNumSends	= 1;

	m_hstrSetIdleSound			= DNULL;
	m_hstrSetAggressiveSound	= DNULL;
	m_hstrSetDefensiveSound		= DNULL;
	m_hstrSetPanickedSound		= DNULL;
	m_hstrSetPsychoSound		= DNULL;
	m_hstrSetRetreatingSound	= DNULL;

	m_hstrSpotSound				= DNULL;
	m_hstrLostTargetSound		= DNULL;
	m_hstrDeathSound			= DNULL;
	m_hstrIdleSound				= DNULL;
	m_hstrAggressiveSound		= DNULL;
	m_hstrDefensiveSound		= DNULL;
	m_hstrGuardingSound			= DNULL;
	m_hstrPanickedSound			= DNULL;
	m_hstrPsychoSound			= DNULL;
	m_hstrRetreatingSound		= DNULL;
	m_hstrFollowLostSound		= DNULL;
	m_hstrBumpedSound			= DNULL;

	m_dwAvailableSounds			= 0;
	SetSoundFlag(AI_SNDFLG_SPOT);
	SetSoundFlag(AI_SNDFLG_DEATH);
	SetSoundFlag(AI_SNDFLG_IDLE);
	SetSoundFlag(AI_SNDFLG_BUMPED);

	m_nBasePriority				= SOUNDPRIORITYBASE_AI;

	// Set up base class values used for movement...

	m_fWalkVel		= 175.0f;
	m_fRunVel		= 300.0f;
	m_bOkAdjustVel	= DTRUE;

	m_bUpdateScriptCmd			= DFALSE;
	m_eScriptMovement			= SM_WALK;
	m_fScriptWaitEnd			= 0.0f;
	m_eScriptState				= DONE;
	m_dwScriptFlags				= 0;
	m_eOldState					= IDLE;
	m_nScriptCmdIndex			= 0;
	m_bLoopScriptedAni			= DFALSE;

	m_fPredTravelDist			= 0.0f;
	m_fLastDistTraveled			= 0.0f;

	m_fFollowStartTime			= 0.0f;
	m_fFollowTime				= -1.0f;  // Follow forever
	m_hLeader					= DNULL;
	m_bLostLeader				= DFALSE;
	VEC_INIT(m_vMoveToPos);
	VEC_INIT(m_vTargetPos);
	VEC_INIT(m_vTargetLastPos);

	m_fNextTargetTime			= 0.0f;
	m_fTargetCalcDelta			= 0.001f;

	// Evasive action stuff...

	m_fStartEvasiveTime			= 0.0f;
	m_fStopEvasiveTime			= 0.0f;
	m_dwEvasiveAction			= 0;
	m_dwLastEvasiveAction		= 0;

	m_fFireRestAdjust			= 1.0f;
	m_nNumRecoilFireTrys		= 0;
	m_nMaxCheeseCount			= (DBYTE)AI_DEFAULT_MAX_CHEESE_COUNT;

	
	m_dwLastLoadFlags			= 0;


	// Debug stuff

	m_nDebugRecomputeValue		= -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetAnimationIndexes()
//
//	PURPOSE:	Initialize model animation indexes
//
// ----------------------------------------------------------------------- //
	
void BaseAI::SetAnimationIndexes()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	CBaseCharacter::SetAnimationIndexes();

	// Set up our walk strafe anis...

	m_hWalkStrafeLRifleAni			= m_hRunStrafeLRifleAni;
	m_hWalkStrafeRRifleAni			= m_hRunStrafeRRifleAni;
	m_hWalkStrafeLRifleAttackAni	= m_hRunStrafeLRifleAttackAni;
	m_hWalkStrafeRRifleAttackAni	= m_hRunStrafeRRifleAttackAni;

	// Set up our crouch (i.e., panicked - crouched in fear) anis...

	m_hCrouchRifleAni				= pServerDE->GetAnimIndex(m_hObject, ANIM_PANICKED);	
	m_hCrouchRifleAttackAni			= m_hCrouchRifleAni;
	m_hCrouchWalkRifleAni			= m_hCrouchRifleAni;
	m_hCrouchWalkRifleAttackAni		= m_hCrouchRifleAni;
	m_hCrouchStrafeLRifleAni		= m_hCrouchRifleAni;
	m_hCrouchStrafeRRifleAni		= m_hCrouchRifleAni;
	m_hCrouchStrafeLRifleAttackAni	= m_hCrouchRifleAni;
	m_hCrouchStrafeRRifleAttackAni	= m_hCrouchRifleAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::~BaseAI()
//
//	PURPOSE:	Deallocate data
//
// ----------------------------------------------------------------------- //

BaseAI::~BaseAI()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrSpotTriggerTarget)
	{
		pServerDE->FreeString(m_hstrSpotTriggerTarget);
	}

	if (m_hstrSpotTriggerMessage)
	{
		pServerDE->FreeString(m_hstrSpotTriggerMessage);
	}

	if (m_hstrLostTargetTriggerTarget)
	{
		pServerDE->FreeString(m_hstrLostTargetTriggerTarget);
	}

	if (m_hstrLostTargetTriggerMessage)
	{
		pServerDE->FreeString(m_hstrLostTargetTriggerMessage);
	}

	if (m_hstrBumpedTriggerTarget)
	{
		pServerDE->FreeString(m_hstrBumpedTriggerTarget);
	}

	if (m_hstrBumpedTriggerMessage)
	{
		pServerDE->FreeString(m_hstrBumpedTriggerMessage);
	}

	if (m_hstrSetIdleSound)
	{
		pServerDE->FreeString(m_hstrSetIdleSound);
	}

	if (m_hstrSetDefensiveSound)
	{
		pServerDE->FreeString(m_hstrSetDefensiveSound);
	}

	if (m_hstrSetAggressiveSound)
	{
		pServerDE->FreeString(m_hstrSetAggressiveSound);
	}

	if (m_hstrSetRetreatingSound)
	{
		pServerDE->FreeString(m_hstrSetRetreatingSound);
	}

	if (m_hstrSetPanickedSound)
	{
		pServerDE->FreeString(m_hstrSetPanickedSound);
	}

	if (m_hstrSetPsychoSound)
	{
		pServerDE->FreeString(m_hstrSetPsychoSound);
	}

	if (m_hstrSpotSound)
	{
		pServerDE->FreeString(m_hstrSpotSound);
	}

	if (m_hstrLostTargetSound)
	{
		pServerDE->FreeString(m_hstrLostTargetSound);
	}

	if (m_hstrDeathSound)
	{
		pServerDE->FreeString(m_hstrDeathSound);
	}

	if (m_hstrIdleSound)
	{
		pServerDE->FreeString(m_hstrIdleSound);
	}

	if (m_hstrDefensiveSound)
	{
		pServerDE->FreeString(m_hstrDefensiveSound);
	}

	if (m_hstrAggressiveSound)
	{
		pServerDE->FreeString(m_hstrAggressiveSound);
	}

	if (m_hstrRetreatingSound)
	{
		pServerDE->FreeString(m_hstrRetreatingSound);
	}

	if (m_hstrGuardingSound)
	{
		pServerDE->FreeString(m_hstrGuardingSound);
	}

	if (m_hstrPanickedSound)
	{
		pServerDE->FreeString(m_hstrPanickedSound);
	}

	if (m_hstrPsychoSound)
	{
		pServerDE->FreeString(m_hstrPsychoSound);
	}

	if (m_hstrFollowLostSound)
	{
		pServerDE->FreeString(m_hstrFollowLostSound);
	}

	if (m_hstrBumpedSound)
	{
		pServerDE->FreeString(m_hstrBumpedSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD BaseAI::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			if (!Update())
			{
				CServerDE* pServerDE = GetServerDE();
				if (!pServerDE) return 0;

				pServerDE->RemoveObject(m_hObject);		
			}
			break;
		}

		case MID_TOUCHNOTIFY:
		{
			m_fLastTouchForce = fData;
			HandleTouch((HOBJECT)pData);
			break;
		}

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hTarget)
				{
					m_hTarget = DNULL;
				}
				if (hLink == m_hLastDamager)
				{
					m_hLastDamager = DNULL;
				}
				if (hLink == m_hLeader)
				{
					m_hLeader = DNULL;
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			CServerDE* pServerDE = GetServerDE();
			if (!pServerDE) return 0;

			pServerDE->SetNextUpdate(m_hObject, GetRandom(0.0f, 2.0f) + AI_UPDATE_DELTA);
			pServerDE->SetDeactivationTime(m_hObject, AI_DEACTIVATION_TIME);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();	
			}
			CacheFiles();
			break;
		}

		case MID_PRECREATE:
		{
			DDWORD dwRet = CBaseCharacter::EngineMessageFn(messageID, pData, fData);
			
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}
			return dwRet;
			break;
		}

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}


	return CBaseCharacter::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

DDWORD BaseAI::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			DVector vDir;
			DFLOAT fDamage;
			DamageType eType;
			HOBJECT hHeHitMe = DNULL;

			pServerDE->ReadFromMessageVector(hRead, &vDir);
			fDamage  = pServerDE->ReadFromMessageFloat(hRead);
			eType	 = (DamageType)pServerDE->ReadFromMessageByte(hRead);
			hHeHitMe = pServerDE->ReadFromMessageObject(hRead);

			pServerDE->ResetRead(hRead);

			HitByObject(hHeHitMe, fDamage);
		}
		break;

		default : break;
	}

	return CBaseCharacter::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pData) return DFALSE;

	if ( pServerDE->GetPropGeneric( "SpotTriggerTarget", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSpotTriggerTarget = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric( "SpotTriggerNumSends", &genProp ) == DE_OK )
			m_nSpotTriggerNumSends = genProp.m_Long;

	if ( pServerDE->GetPropGeneric("SpotTriggerMessage", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSpotTriggerMessage = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("LostTargetTriggerTarget", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrLostTargetTriggerTarget = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("LostTargetTriggerMessage", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrLostTargetTriggerMessage = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric( "LostTargetTriggerNumSends", &genProp ) == DE_OK )
			m_nLostTargetTriggerNumSends = genProp.m_Long;

	if ( pServerDE->GetPropGeneric("BumpedTriggerTarget", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrBumpedTriggerTarget = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("BumpedTriggerMessage", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrBumpedTriggerMessage = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric( "BumpedTriggerNumSends", &genProp ) == DE_OK )
			m_nBumpedTriggerNumSends = genProp.m_Long;

	if ( pServerDE->GetPropGeneric("SetIdleSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSetIdleSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("SetDefensiveSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSetDefensiveSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("SetAggressiveSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSetAggressiveSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("SetRetreatingSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSetRetreatingSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("SetPanickedSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSetPanickedSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("SetPsychoSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSetPsychoSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("SpotSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrSpotSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("LostTargetSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrLostTargetSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("DeathSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrDeathSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("IdleSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrIdleSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("DefensiveSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrDefensiveSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("AggressiveSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrAggressiveSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("RetreatingSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrRetreatingSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("GuardingSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrGuardingSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("PanickedSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrPanickedSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("PsychoSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrPsychoSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("FollowLostSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrFollowLostSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("BumpedSound", &genProp ) == DE_OK )
		if ( genProp.m_String[0] )
			m_hstrBumpedSound = pServerDE->CreateString( genProp.m_String );

	if ( pServerDE->GetPropGeneric("SetIdle", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_SETIDLE;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_SETIDLE;
	}

	if ( pServerDE->GetPropGeneric("SetDefensive", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_SETDEFENSIVE;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_SETDEFENSIVE;
	}

	if ( pServerDE->GetPropGeneric("SetAggressive", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_SETAGGRESSIVE;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_SETAGGRESSIVE;
	}

	if ( pServerDE->GetPropGeneric("SetRetreating", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_SETRETREATING;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_SETRETREATING;
	}

	if ( pServerDE->GetPropGeneric("SetPanicked", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_SETPANICKED;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_SETPANICKED;
	}

	if ( pServerDE->GetPropGeneric("SetPsycho", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_SETPSYCHO;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_SETPSYCHO;
	}

	if ( pServerDE->GetPropGeneric("Spot", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_SPOT;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_SPOT;
	}

	if ( pServerDE->GetPropGeneric("LostTarget", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_LOSTTARGET;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_LOSTTARGET;
	}

	if ( pServerDE->GetPropGeneric("Death", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_DEATH;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_DEATH;
	}

	if ( pServerDE->GetPropGeneric("Idle", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_IDLE;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_IDLE;
	}

	if ( pServerDE->GetPropGeneric("Defensive", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_DEFENSIVE;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_DEFENSIVE;
	}

	if ( pServerDE->GetPropGeneric("Aggressive", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_AGGRESSIVE;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_AGGRESSIVE;
	}

	if ( pServerDE->GetPropGeneric("Retreating", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_RETREATING;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_RETREATING;
	}

	if ( pServerDE->GetPropGeneric("Guarding", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_GUARDING;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_GUARDING;
	}

	if ( pServerDE->GetPropGeneric("Panicked", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_PANICKED;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_PANICKED;
	}

	if ( pServerDE->GetPropGeneric("Psycho", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_PSYCHO;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_PSYCHO;
	}

	if ( pServerDE->GetPropGeneric("FollowLost", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_FOLLOWLOST;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_FOLLOWLOST;
	}

	if ( pServerDE->GetPropGeneric("Bumped", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableSounds |= AI_SNDFLG_BUMPED;
		else 
			m_dwAvailableSounds &= ~AI_SNDFLG_BUMPED;
	}


	if ( pServerDE->GetPropGeneric("StateIdle", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableStates |= AI_SFLG_IDLE;
		else 
			m_dwAvailableStates &= ~AI_SFLG_IDLE;
	}

	if ( pServerDE->GetPropGeneric("StateDefensive", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableStates |= AI_SFLG_DEFENSIVE;
		else 
			m_dwAvailableStates &= ~AI_SFLG_DEFENSIVE;
	}

	if ( pServerDE->GetPropGeneric("StateAggressive", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableStates |= AI_SFLG_AGGRESSIVE;
		else 
			m_dwAvailableStates &= ~AI_SFLG_AGGRESSIVE;
	}

	if ( pServerDE->GetPropGeneric("StateRetreating", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableStates |= AI_SFLG_RETREATING;
		else 
			m_dwAvailableStates &= ~AI_SFLG_RETREATING;
	}

	if ( pServerDE->GetPropGeneric("StateGuarding", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableStates |= AI_SFLG_GUARDING;
		else 
			m_dwAvailableStates &= ~AI_SFLG_GUARDING;
	}

	if ( pServerDE->GetPropGeneric("StatePanicked", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableStates |= AI_SFLG_PANICKED;
		else 
			m_dwAvailableStates &= ~AI_SFLG_PANICKED;
	}

	if ( pServerDE->GetPropGeneric("StatePsycho", &genProp ) == DE_OK )
	{
		if (genProp.m_Bool) 
			m_dwAvailableStates |= AI_SFLG_PSYCHO;
		else 
			m_dwAvailableStates &= ~AI_SFLG_PSYCHO;
	}

		
	if ( pServerDE->GetPropGeneric("State", &genProp ) == DE_OK )
		m_eState = (AIState)genProp.m_Long;

	if ( pServerDE->GetPropGeneric("Bravado", &genProp ) == DE_OK )
		m_eBravado = (AIBravado)genProp.m_Long;

	if ( pServerDE->GetPropGeneric("Experience", &genProp ) == DE_OK )
		m_eExperience = (AIExperience)genProp.m_Long;

	if ( pServerDE->GetPropGeneric("Marksmanship", &genProp ) == DE_OK )
		m_eMarksmanship = (AIMarksmanship)genProp.m_Long;

	if ( pServerDE->GetPropGeneric("Evasive", &genProp ) == DE_OK )
		m_eEvasive = (AIEvasive)genProp.m_Long;

	if ( pServerDE->GetPropGeneric("WeaponId", &genProp ) == DE_OK )
		m_nWeaponId = (DBYTE)genProp.m_Long;

	if ( pServerDE->GetPropGeneric("VisibleRange", &genProp ) == DE_OK )
		m_fVisibleRange = genProp.m_Float;

	if ( pServerDE->GetPropGeneric("HearingRange", &genProp ) == DE_OK )
		m_fHearingRange = genProp.m_Float;

	if ( pServerDE->GetPropGeneric("SoundRadius", &genProp ) == DE_OK )
		m_fSoundRadius = genProp.m_Float;

	if ( pServerDE->GetPropGeneric("Large", &genProp ) == DE_OK )
		m_eModelSize = genProp.m_Bool ? MS_LARGE : m_eModelSize;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void BaseAI::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	char* pFilename = GetModel(m_nModelId, m_eModelSize);
	char* pSkin		= GetSkin(m_nModelId, m_cc, m_eModelSize);

	if (pFilename && pFilename[0])
	{
		SAFE_STRCPY(pStruct->m_Filename, pFilename);
	}

	if (pSkin && pSkin[0])
	{
		SAFE_STRCPY(pStruct->m_SkinName, pSkin);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::InitialUpdate()
//
//	PURPOSE:	Initialize the AI routines
//
// ----------------------------------------------------------------------- //

void BaseAI::InitialUpdate()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (m_eModelSize == MS_SMALL && m_bOkAdjustVel)
	{
		// m_dwFlags &= ~FLAG_SHADOW;

		m_dwFlags |= FLAG_CLIENTNONSOLID;	// So we can squish little guys

		m_fRunVel  /= 5.0f;
		m_fWalkVel /= 5.0f;
	}


	// Adjust data members based on the current difficulty level...

	AdjustMarksmanshipPerturb();
	AdjustEvasiveDelay();
	AdjustFireDelay();


	// Don't rag-doll AI anymore...

	m_damage.SetApplyDamagePhysics(DFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CacheFiles()
//
//	PURPOSE:	Cache resources used by this AI
//
// ----------------------------------------------------------------------- //

void BaseAI::CacheFiles()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if( !( pServerDE->GetServerFlags( ) & SS_CACHING ))
		return;

	char* pFile = DNULL;

	// Cache sounds...

	if (m_hstrSetIdleSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSetIdleSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSetAggressiveSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSetAggressiveSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSetDefensiveSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSetDefensiveSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSetPanickedSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSetPanickedSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSetPsychoSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSetPsychoSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSetRetreatingSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSetRetreatingSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrSpotSound)
	{
		pFile = pServerDE->GetStringData(m_hstrSpotSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrLostTargetSound)
	{
		pFile = pServerDE->GetStringData(m_hstrLostTargetSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrDeathSound)
	{
		pFile = pServerDE->GetStringData(m_hstrDeathSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrIdleSound)
	{
		pFile = pServerDE->GetStringData(m_hstrIdleSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrAggressiveSound)
	{
		pFile = pServerDE->GetStringData(m_hstrAggressiveSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrDefensiveSound)
	{
		pFile = pServerDE->GetStringData(m_hstrDefensiveSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrGuardingSound)
	{
		pFile = pServerDE->GetStringData(m_hstrGuardingSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrPanickedSound)
	{
		pFile = pServerDE->GetStringData(m_hstrPanickedSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrPsychoSound)
	{
		pFile = pServerDE->GetStringData(m_hstrPsychoSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrRetreatingSound)
	{
		pFile = pServerDE->GetStringData(m_hstrRetreatingSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrFollowLostSound)
	{
		pFile = pServerDE->GetStringData(m_hstrFollowLostSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrBumpedSound)
	{
		pFile = pServerDE->GetStringData(m_hstrBumpedSound);
		if (pFile)
		{
			pServerDE->CacheFile(FT_SOUND, pFile);
		}
	}


	
	// Cache all other AI sounds...

	CacheAISounds(this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::InitializeWeapons()
//
//	PURPOSE:	Initialize the weapons
//
// ----------------------------------------------------------------------- //

void BaseAI::InitializeWeapons()
{
	// AI's will only create the weapons they use...

	m_weapons.SetArsenal(CWeapons::AT_AS_NEEDED);

	m_weapons.ObtainWeapon(m_nWeaponId);
	m_weapons.ChangeWeapon(m_nWeaponId);
	m_weapons.AddAmmo(m_nWeaponId, GetWeaponMaxAmmo(m_nWeaponId));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::HandleWeaponChange()
//
//	PURPOSE:	Handle our weapon changing...
//
// ----------------------------------------------------------------------- //
	
void BaseAI::HandleWeaponChange()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return;
	
	CBaseCharacter::HandleWeaponChange();

	if (m_hHandHeldWeapon)
	{
		// Associated our hand held weapon with our weapon...

		CPVWeaponModel* pModel = (CPVWeaponModel*)pServerDE->HandleToObject(m_hHandHeldWeapon);
		if (pModel) 
		{
			pModel->SetParent(pWeapon);
			pWeapon->SetModelObject(m_hHandHeldWeapon);
			pWeapon->InitAnimations();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void BaseAI::HandleTouch(HOBJECT hObj)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_activation.IsActive()) return;

	if (m_fCurTime < m_fNextBumpedTime) return;

	// Only players bump into us

	if (IsPlayer(hObj) && m_eState != PANICKED)
	{
		CBaseCharacter* pB = (CBaseCharacter*)pServerDE->HandleToObject(hObj);
		if (!pB) return;

		if (!pB->IsDead()) 
		{
			HandleBumped();
		}
	}

	m_fNextBumpedTime = m_fCurTime + GetRandom(10.0f, 20.0f);
}


//*************************************************************************//
//*************************************************************************//
//
// A I  S T A T E   M A C H I N E   F U N C T I O N A L I T Y
// 
// (updates, state-changes, logic)
//*************************************************************************//
//*************************************************************************//

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::Update()
//
//	PURPOSE:	Update the AI
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	m_fCurTime = pServerDE->GetTime();
	pServerDE->GetObjectPos(m_hObject, &m_vPos);


	//pServerDE->BPrint("NumIntersectSegs: %d", s_nNumCallsToIntersectSegment);
	s_nNumCallsToIntersectSegment = 0;


	// Check to see if we just reloaded a saved game...

	if (m_dwLastLoadFlags == LOAD_RESTORE_GAME || m_dwLastLoadFlags == LOAD_NEW_LEVEL)
	{
		HandleGameRestore();
		m_dwLastLoadFlags = 0;
	}

	m_dwLastAction = m_dwAction;
	
	ClearActionFlags();

	// Update Senses...

	UpdateSenses();

	
	// Make sure we act dead, when dead...

	if (m_damage.IsDead() && m_eCondition != DEAD)
	{
		m_fRecomputeTime = 0.0f;
		m_bRecompute = DTRUE;
	}
	
	if (m_bRecompute && m_fCurTime > m_fRecomputeTime) 
	{
		RecomputeState();
	}


	// Based upon current state try to update our state.  What we do in 
	// a given state depends on quite a few factors, our condition and 
	// the environment can force us to change states.  

	switch (m_eState)
	{
		case IDLE		: UpdateIdle();			break;
		case DEFENSIVE	: UpdateDefensive();	break;
		case AGGRESSIVE	: UpdateAggressive();	break;
		case RETREATING	: UpdateRetreating();	break;
		case GUARDING	: UpdateGuarding();		break;
		case PANICKED	: UpdatePanicked();		break;
		case PSYCHO		: UpdatePsycho();		break;

		// The following are internal states (i.e., can't be set in the editor).

		case SCRIPT		: UpdateScript();		break;
		case DYING		: UpdateDying();		break;
		default			: UpdateAggressive();	break;
	}


	// Update evasive actions...

	if ((m_dwAction & AI_AFLG_EVASIVE) && (m_eEvasive != NON_EVASIVE))
	{
		UpdateEvasiveAction();
	}

	
	// Update the control (movement) flags...

	UpdateControlFlags();


	// Update our weapon...

	UpdateWeapon();


	// Update our movement...

	UpdateMovement();


	// Print out debug info...

	PrintDebugInfo();


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::RecomputeState()
//
//	PURPOSE:	Recompute our current state based on info
//
// ----------------------------------------------------------------------- //

void BaseAI::RecomputeState()
{
	m_bRecompute = DFALSE;

	RecomputeCondition();

	if (m_eCondition == DEAD || m_eState == PANICKED) return;

	// Use "The Hubbard Method" to determine what state we should go to...

	int nUnModifiedVal = GetRandom(1, 100);
	int nVal = nUnModifiedVal;

	// Adjust the value based on our current condition...

	int nIndex = nUnModifiedVal < s_nStateConditionMedian ? 0 : 1;
	nVal += s_nStateConditionAdjust[m_eCondition][nIndex];


	// Adjust the value based on our bravado...

	nIndex = nUnModifiedVal < s_nStateBravadoMedian ? 0 : 1;
	nVal += s_nStateBravadoAdjust[m_eBravado][nIndex];


	// Adjust the value based on our experience...

	nIndex = nUnModifiedVal < s_nStateExperienceMedian ? 0 : 1;
	nVal += s_nStateExperienceAdjust[m_eExperience][nIndex];

	// Normalize value...

	if (nVal < 0) nVal = 0;
	else if (nVal > 100) nVal = 100;


	// Now, determine what state we should go to...

	AIState eState = IDLE;
	for(int i=0; i < 7; i++)
	{
		if (s_nStateTransitionTable[i][0] <= nVal && nVal <= s_nStateTransitionTable[i][1])
		{
			eState = (AIState)i;
			break;
		}
	}

#ifdef _DEBUG
	m_nDebugRecomputeValue = nVal;
#endif

	if (eState == IDLE) return;

	// Find the closest valid state...

	AIState eNewState = FindClosestValidState(eState, IDLE);

	if (eNewState != m_eState) SetState(eNewState);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::FindClosestValidState()
//
//	PURPOSE:	Find the closest valid state to the state passed in
//
// ----------------------------------------------------------------------- //

BaseAI::AIState BaseAI::FindClosestValidState(AIState eNewState, 
											  AIState ePrevRecurseState)
{
	AIState eState = m_eState;

	switch (eNewState)
	{
		case PSYCHO :
		{
			// Start at the top of the range, and work down...

			if (IsValidState(PSYCHO))			eState = PSYCHO;
			else if (IsValidState(AGGRESSIVE))	eState = AGGRESSIVE;
			else if (IsValidState(DEFENSIVE))	eState = DEFENSIVE;
			else if (IsValidState(RETREATING))	eState = RETREATING;
			else if (IsValidState(PANICKED))	eState = PANICKED;
		}
		break;

		case AGGRESSIVE :
		{
			if (IsValidState(AGGRESSIVE))
			{
				eState = AGGRESSIVE;
			}
			else
			{
				// 80% of the time we'll go to Defensive, 
				// 20% we'll go Psycho...

				if (GetRandom(1, 5) == 1)
				{	
					eState = FindClosestValidState(PSYCHO, AGGRESSIVE);
				}
				else
				{
					// Make sure we don't get caught in recursive loops...

					if (ePrevRecurseState != DEFENSIVE)
					{	
						eState = FindClosestValidState(DEFENSIVE, AGGRESSIVE);
					}
					else
					{
						eState = FindClosestValidState(PSYCHO, AGGRESSIVE);
					}
				}
			}
		}
		break;

		case DEFENSIVE :
		{
			if (IsValidState(DEFENSIVE))
			{
				eState = DEFENSIVE;
			}
			else
			{
				// 80% of the time we'll go to Aggressive, 
				// 20% we'll go to Retreating...

				if (GetRandom(1, 5) == 1)
				{
					// Make sure we don't get caught in recursive loops...

					if (ePrevRecurseState != RETREATING)
					{	
						eState = FindClosestValidState(RETREATING, DEFENSIVE);
					}
					else
					{
						eState = FindClosestValidState(AGGRESSIVE, DEFENSIVE);
					}
				}
				else
				{
					// Make sure we don't get caught in recursive loops...

					if (ePrevRecurseState != AGGRESSIVE)
					{	
						eState = FindClosestValidState(AGGRESSIVE, DEFENSIVE);
					}
					else
					{
						eState = FindClosestValidState(RETREATING, DEFENSIVE);
					}
				}
			}
		}
		break;

		case RETREATING :
		{
			if (IsValidState(RETREATING))
			{
				eState = RETREATING;
			}
			else
			{
				// 80% of the time we'll go to Defensive, 
				// 20% we'll go Panicked...

				if (GetRandom(1, 5) == 1)
				{	
					eState = FindClosestValidState(PANICKED, RETREATING);
				}
				else
				{
					// Make sure we don't get caught in recursive loops...

					if (ePrevRecurseState != DEFENSIVE)
					{	
						eState = FindClosestValidState(DEFENSIVE, RETREATING);
					}
					else
					{
						eState = FindClosestValidState(PANICKED, RETREATING);
					}
				}
			}
		}
		break;

		case PANICKED :
		{
			// Start at the bottom of the range, and work up...

			if (IsValidState(PANICKED))			eState = PANICKED;
			else if (IsValidState(RETREATING))	eState = RETREATING;
			else if (IsValidState(DEFENSIVE))	eState = DEFENSIVE;
			else if (IsValidState(AGGRESSIVE))	eState = AGGRESSIVE;
			else if (IsValidState(PSYCHO))		eState = PSYCHO;
		}
		break;

		default : break;  // Couldn't find one
	}

	return eState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::IsValidState()
//
//	PURPOSE:	Make sure the passed in state is valid
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::IsValidState(AIState eState)
{
	DBOOL bRet = DFALSE;

	switch (eState)
	{
		case DEFENSIVE :
			if (m_dwAvailableStates & AI_SFLG_DEFENSIVE) bRet = DTRUE;
		break;
		case AGGRESSIVE :
			if (m_dwAvailableStates & AI_SFLG_AGGRESSIVE) bRet = DTRUE;
		break;
		case RETREATING :
			if (m_dwAvailableStates & AI_SFLG_RETREATING) bRet = DTRUE;
		break;
		case PANICKED :
			if (m_dwAvailableStates & AI_SFLG_PANICKED) bRet = DTRUE;
		break;
		case PSYCHO :
			if (m_dwAvailableStates & AI_SFLG_PSYCHO) bRet = DTRUE;
		break;

		default : break;;
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::RecomputeCondition()
//
//	PURPOSE:	Recompute our current condition
//
// ----------------------------------------------------------------------- //

void BaseAI::RecomputeCondition()
{
	// Condition will be based on health...

	DFLOAT fHealthPercentage = m_damage.GetHitPoints()/m_damage.GetMaxHitPoints();

	if (m_damage.IsDead()) 
	{
		m_eCondition = DEAD;
		SetDying();
	}
	else if (fHealthPercentage >= 0.80f)
	{
		m_eCondition = COCKY;
	}
	else if (fHealthPercentage >= 0.30f)
	{
		m_eCondition = HEALTHY;
	}
	else
	{
		m_eCondition = WOUNDED;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetState()
//
//	PURPOSE:	Set the AI to a specific state
//
// ----------------------------------------------------------------------- //

void BaseAI::SetState(AIState eState)
{
	if (m_damage.IsDead() || (m_eState == DYING)) return;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Reset the deactivation time if we've changed states...

	pServerDE->SetDeactivationTime(m_hObject, AI_DEACTIVATION_TIME);


	if (eState == PANICKED)
	{
		m_bCanPlayDialogSound = DFALSE;
	}
	else
	{
		m_bCanPlayDialogSound = DTRUE;
	}
	
	switch (eState) 
	{
		case IDLE: 
		{
			SetIdle();
		}
		break;

		case DEFENSIVE: 
		{
			SetDefensive();
		}
		break;

		case AGGRESSIVE: 
		{
			SetAggressive();
		}
		break;

		case RETREATING: 
		{
			SetRetreating();
		}
		break;

		case GUARDING: 
		{
			SetGuarding();
		}
		break;

		case PANICKED: 
		{
			SetPanicked();
		}
		break;

		case PSYCHO: 
		{
			SetPsycho();
		}
		break;

		default:
			SetAggressive();
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetState()
//
//	PURPOSE:	Set the state
//
// ----------------------------------------------------------------------- //

void BaseAI::SetState(char* pState)
{
	if (!pState || m_damage.IsDead() || (m_eState == DYING)) return;

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	
	// Reset the deactivation time if we've changed states...

	pServerDE->SetDeactivationTime(m_hObject, AI_DEACTIVATION_TIME);


	if (stricmp(pState, "IDLE") == 0)
	{
		SetIdle();
	}
	else if (stricmp(pState, "DEFENSIVE") == 0)
	{
		SetDefensive();
	}
	else if (stricmp(pState, "AGGRESSIVE") == 0)
	{
		SetAggressive();
	}
	else if (stricmp(pState, "RETREATING") == 0)
	{
		SetRetreating();
	}
	else if (stricmp(pState, "GUARDING") == 0)
	{
		SetGuarding();
	}
	else if (stricmp(pState, "PANICKED") == 0)
	{
		SetPanicked();
	}
	else if (stricmp(pState, "PSYCHO") == 0)
	{
		SetPsycho();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetIdle()
//
//	PURPOSE:	Set state to Idle
//
// ----------------------------------------------------------------------- //

void BaseAI::SetIdle()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_fRecomputeTime = m_fCurTime + AI_MIN_IDLE_TIME;

	if (m_dwAvailableSounds & AI_SNDFLG_SETIDLE)
	{
		char* pSound = DNULL;
		if (m_hstrSetIdleSound)
		{
			pSound = pServerDE->GetStringData(m_hstrSetIdleSound);
		}

		PlayDialogSound(pSound);
	}

	m_eState = IDLE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateIdle()
//
//	PURPOSE:	Do Idle thang
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateIdle()
{
	// We're just standing around...

	ClearActionFlags();
	SetActionFlag(AI_AFLG_STAND);

	// Possible switch to a different idle animation at some point...
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetDefensive()
//
//	PURPOSE:	Set state to Defensive
//
// ----------------------------------------------------------------------- //

void BaseAI::SetDefensive()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_fRecomputeTime = m_fCurTime + AI_MIN_DEFENSIVE_TIME;

	m_eState = DEFENSIVE;

	if (m_dwAvailableSounds & AI_SNDFLG_SETDEFENSIVE)
	{
		char* pSound = DNULL;
		if (m_hstrSetDefensiveSound)
		{
			pSound = pServerDE->GetStringData(m_hstrSetDefensiveSound);
		}
		else
		{
			pSound = GetSetDefensiveSound(this);
		}

		PlayDialogSound(pSound);
	}

	ClearActionFlags();
	SetActionFlag(AI_AFLG_STAND);
	SetActionFlag(AI_AFLG_AIMING);
}	


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateDefensive()
//
//	PURPOSE:	Implement the defending actions
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateDefensive()
{
	// If we don't have a target try to aquire one...

	if (!ChooseTarget())
	{
		return;
	}

	// A defending AI should stay still and shoot at enemies
	// within range.  (very similar to guarding, but doesn't
	// care about a target object)

	ShootTarget();

	SetActionFlag(AI_AFLG_STAND);
	SetActionFlag(AI_AFLG_AIMING);
	SetActionFlag(AI_AFLG_EVASIVE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetAggressive()
//
//	PURPOSE:	Set state to Aggressive
//
// ----------------------------------------------------------------------- //

void BaseAI::SetAggressive()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_fRecomputeTime = m_fCurTime + AI_MIN_AGGRESSIVE_TIME;

	if (m_dwAvailableSounds & AI_SNDFLG_SETAGGRESSIVE)
	{
		char* pSound = DNULL;
		if (m_hstrSetAggressiveSound)
		{
			pSound = pServerDE->GetStringData(m_hstrSetAggressiveSound);
		}
		else
		{
			pSound = GetSetAggressiveSound(this);
		}

		PlayDialogSound(pSound);
	}

	m_eState = AGGRESSIVE;
}	


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateAggressive()
//
//	PURPOSE:	Implement the attacking actions
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateAggressive()
{
	// If we don't have a target try to aquire one...

	if (!ChooseTarget()) return;
	
	// Try to shoot target.....

	ShootTarget();

	
	SetActionFlag(AI_AFLG_AIMING);
	SetActionFlag(AI_AFLG_EVASIVE);

	// Always try to get a better shot...

	ApproachTarget();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::ApproachTarget()
//
//	PURPOSE:	Move towards our target
//
// ----------------------------------------------------------------------- //

void BaseAI::ApproachTarget()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !m_hTarget) return;

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return;

	// Distance to target...

	DFLOAT fDist = DistanceToObject(m_hTarget);
	
	// See if we should approach target or not...

	ProjectileType type = GetWeaponType(pWeapon->GetId());

	if (type == PROJECTILE || type == CANNON)
	{
		// If we'll get caught in the blast, don't approach...

		SetActionFlag(AI_AFLG_STAND);
		return;
	}


	// If we can't see our target, see if we can find him...

	if (!IsObjectVisibleToAI(m_hTarget))
	{
		if (IsPlayer(m_hTarget))
		{
			if (m_bLostPlayer)
			{
				SetNewTarget(DNULL);
				m_bSpottedPlayer = DFALSE;
			}
			else
			{
				SearchForPlayer(m_hTarget);
			}
			return;
		}
	}
	
	if (fDist > AI_SAFE_DISTANCE + 50.0f)
	{
		if (m_eCondition != COCKY || type == MELEE)
		{
			RunForward();
		}
		else
		{
			WalkForward();
		}
	}
	else
	{
		SetActionFlag(AI_AFLG_STAND);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SearchForPlayer()
//
//	PURPOSE:	Try to find our target
//
// ----------------------------------------------------------------------- //

void BaseAI::SearchForPlayer(HOBJECT hPlayerObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !hPlayerObj || !IsPlayer(hPlayerObj)) return;

	CPlayerObj* pPlayer = (CPlayerObj*)pServerDE->HandleToObject(hPlayerObj);
	if (!pPlayer) return;

	CTList<DVector*>* m_pBiscuits = pPlayer->GetScentBiscuitList();
	if (!m_pBiscuits) return;


	// Don't do any evasive stuff while looking for the player...

	ClearActionFlag(AI_AFLG_EVASIVE);


	// Find the most recent player position that I can still see...

	DVector** pCur = DNULL;
	DVector*  pPos = DNULL;
	DVector*  pSavePos = DNULL;

	pCur = m_pBiscuits->GetItem(TLIT_FIRST);
	while (pCur)
	{
		pPos = *pCur;
		if (pPos)
		{
			if (IsPosVisibleToAI(pPos))
			{
				pSavePos = pPos;
			}
		}
		pCur = m_pBiscuits->GetItem(TLIT_NEXT);
	}


	if (pSavePos)
	{
		// Move toward last visible player pos...(this value can change
		// before we get there which is fine...)...

		m_bSearchingForPlayer = DTRUE;
		m_eScriptMovement	  = SM_RUN;
		UpdateScriptMovement(pSavePos);
	}
	else
	{
		HandleLostPlayer();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetRetreating()
//
//	PURPOSE:	Set state to Retreating
//
// ----------------------------------------------------------------------- //

void BaseAI::SetRetreating()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_fRecomputeTime = m_fCurTime + AI_MIN_RETREATING_TIME;

	if (m_dwAvailableSounds & AI_SNDFLG_SETRETREATING)
	{
		char* pSound = DNULL;
		if (m_hstrSetRetreatingSound)
		{
			pSound = pServerDE->GetStringData(m_hstrSetRetreatingSound);
		}
		else
		{
			pSound = GetSetRetreatingSound(this);
		}

		PlayDialogSound(pSound);
	}

	m_eState = RETREATING;
}	


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateRetreating()
//
//	PURPOSE:	Implement the retreating actions
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateRetreating()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !m_hTarget) return;

	FaceObject(m_hTarget);  

	if (TargetInRange())
	{
		ShootTarget();

		if (DistanceToObject(m_hTarget) < AI_RETREAT_DISTANCE)
		{
			if (m_eCondition == WOUNDED)
			{
				RunBackward();
			}
			else
			{
				WalkBackward();
			}
		}
	}
	else
	{
		UpdateIdle();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetGuarding()
//
//	PURPOSE:	Set state to Guarding
//
// ----------------------------------------------------------------------- //

void BaseAI::SetGuarding()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_fRecomputeTime = -1.0f;  // We never recompute guarding state

	m_eState = GUARDING;
}	


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateGuarding()
//
//	PURPOSE:	Implement the guarding actions
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateGuarding()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	UpdateIdle();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetPanicked()
//
//	PURPOSE:	Set state to Panicked
//
// ----------------------------------------------------------------------- //

void BaseAI::SetPanicked()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_fRecomputeTime = m_fCurTime + AI_MIN_PANICKED_TIME;

	if (m_dwAvailableSounds & AI_SNDFLG_SETPANICKED)
	{
		char* pSound = DNULL;
		if (m_hstrSetPanickedSound)
		{
			pSound = pServerDE->GetStringData(m_hstrSetPanickedSound);
		}
		else
		{
			pSound = GetSetPanickedSound(this);
		}

		PlayDialogSound(pSound);
	}

	m_eState = PANICKED;
}	


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdatePanicked()
//
//	PURPOSE:	Implement the Panicked actions
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdatePanicked()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	if (m_hTarget && TargetInRange())
	{
		FaceObject(m_hTarget);
	}
	
	UpdateIdle();
	SetActionFlag(AI_AFLG_DUCK);  // Cower in corner
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetPsycho()
//
//	PURPOSE:	Set state to Psycho
//
// ----------------------------------------------------------------------- //

void BaseAI::SetPsycho()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	m_fRecomputeTime = m_fCurTime + AI_MIN_PSYCHO_TIME;

	if (m_dwAvailableSounds & AI_SNDFLG_SETPSYCHO)
	{
		char* pSound = DNULL;
		if (m_hstrSetPsychoSound)
		{
			pSound = pServerDE->GetStringData(m_hstrSetPsychoSound);
		}
		else
		{
			pSound = GetSetPsychoSound(this);
		}

		PlayDialogSound(pSound);
	}

	m_eState = PSYCHO;
}	


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdatePsycho()
//
//	PURPOSE:	Implement the Psycho actions
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdatePsycho()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	UpdateRetreating();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetDying()
//
//	PURPOSE:	Set state to Dying
//
// ----------------------------------------------------------------------- //

void BaseAI::SetDying()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_damage.IsDead()) return;

	m_eState = DYING;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateDying()
//
//	PURPOSE:	Implement anything special we should do when we die
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateDying()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateControlFlags
//
//	PURPOSE:	Set the movement/firing flags
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateControlFlags()
{
	ClearControlFlags();


	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;


	// Determine what actions are currently taking place...

	if (m_dwAction & AI_AFLG_STAND)
	{
		ClearActionFlag(AI_AFLG_RUN_REVERSE);
		ClearActionFlag(AI_AFLG_WALK_REVERSE);
		ClearActionFlag(AI_AFLG_RUN);
		ClearActionFlag(AI_AFLG_WALK);
	}


	if (m_dwAction & AI_AFLG_RUN)
	{
		m_dwControlFlags |= BC_CFLG_RUN;
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}

	if (m_dwAction & AI_AFLG_RUN_REVERSE)
	{
		m_dwControlFlags |= BC_CFLG_RUN;
		m_dwControlFlags |= BC_CFLG_REVERSE;
	}

	if (m_dwAction & AI_AFLG_JUMP)
	{
		m_dwControlFlags |= BC_CFLG_JUMP;
	}

	if (m_dwAction & AI_AFLG_DUCK)
	{
		m_dwControlFlags |= BC_CFLG_DUCK;
	}

	if (m_dwAction & AI_AFLG_WALK)
	{
		m_dwControlFlags |= BC_CFLG_FORWARD;
	}

	if (m_dwAction & AI_AFLG_WALK_REVERSE)
	{
		m_dwControlFlags |= BC_CFLG_REVERSE;
	}

	if (m_dwAction & AI_AFLG_TURN_LEFT)
	{
		m_dwControlFlags |= BC_CFLG_LEFT;
	}

	if (m_dwAction & AI_AFLG_TURN_RIGHT)
	{
		m_dwControlFlags |= BC_CFLG_RIGHT;
	}

	if (m_dwAction & AI_AFLG_POSE)
	{
		m_dwControlFlags |= BC_CFLG_POSING;
	}

	if (m_dwAction & AI_AFLG_STRAFE_RIGHT)
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_RIGHT;
	}

	if (m_dwAction & AI_AFLG_STRAFE_LEFT)
	{
		m_dwControlFlags |= BC_CFLG_STRAFE_LEFT;
	}

	if (m_dwAction & AI_AFLG_AIMING)
	{
		m_dwControlFlags |= BC_CFLG_FIRING;
	}


	CBaseCharacter::UpdateControlFlags();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateMovement
//
//	PURPOSE:	Update AI movement
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateMovement()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;


	UpdateOnGround();


	// The engine might apply an acceleration to us, make sure 
	// it doesn't last...(keep gravity however...)

	DVector vAccel;
	pServerDE->GetAcceleration(m_hObject, &vAccel);
	vAccel.x = vAccel.z = 0.0f;
	if (vAccel.y > 0.0f) vAccel.y = 0.0f;

	pServerDE->SetAcceleration(m_hObject, &vAccel);


	// Retrieve object vectors for current frame..

	DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);


	// If we are playing a scripted animation, don't update the movement...

	if ((m_eState == SCRIPT) && (m_curScriptCmd.command == AI_SCMD_PLAYANIMATION))
	{
		return;
	}


	// If we're moving (and not scripted), see if we are stuck...

	if ((m_eState != SCRIPT) && (m_dwControlFlags & BC_CFLG_MOVING))
	{
		// If we're stuck on something, see if turing will help us continue...

		if (m_bStuckOnSomething)
		{
			m_bStuckOnSomething = AvoidObstacle();
			if (!m_bStuckOnSomething) 
			{
				UpdateControlFlags();
			}
		}


		// If we are okay to move, check for ledges...

		if (!m_bStuckOnSomething)
		{
			m_bStuckOnSomething = !CanMoveDir(m_vForward);
		}
		

		// Make us stand still...

		if (m_bStuckOnSomething)
		{
			SetActionFlag(AI_AFLG_STAND);
			UpdateControlFlags();
		}
	}


	// Do real work...

	NewUpdateMovement();
	

	// Update current animation...

	UpdateAnimation();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::NewUpdateMovement
//
//	PURPOSE:	Update AI movement using MoveObject
//
// ----------------------------------------------------------------------- //

void BaseAI::NewUpdateMovement()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_damage.IsDead() || !m_bAllowMovement) return;

	m_bStuckOnSomething = DFALSE;

	// Make sure we're trying to move...

	if ( !(m_dwControlFlags & BC_CFLG_MOVING) ) return;

	DFLOAT fTimeDelta = pServerDE->GetFrameTime();

	DVector vNewPos;
	VEC_COPY(vNewPos, m_vPos);

	
	// See if we are stuck on something.  Can only happen if we were
	// moving on the last frame...

	if (m_dwLastFrameCtlFlgs & BC_CFLG_MOVING)
	{
		m_fLastDistTraveled = VEC_DIST(m_vPos, m_vLastPos);

		if (m_fLastDistTraveled < m_fPredTravelDist * 0.95f)
		{
			m_bStuckOnSomething = DTRUE;
		}
	}


	DFLOAT fMoveVel = m_fWalkVel;
	DFLOAT fVal		= (m_dwControlFlags & BC_CFLG_REVERSE) ? -1.0f : 1.0f;

	// Check for running...

	if ((m_dwControlFlags & BC_CFLG_RUN) && m_bAllowRun)
	{
		fMoveVel = m_fRunVel;
	}

	fMoveVel *= (fTimeDelta * fVal);
	
	
	// Move us forward/backward...

	if ((m_dwControlFlags & BC_CFLG_FORWARD) || 
		(m_dwControlFlags & BC_CFLG_REVERSE))
	{
		DVector vF;
		VEC_COPY(vF, m_vForward);

		// Limit movement to x and z...

		vF.y = 0.0;
		VEC_MULSCALAR(vF, vF, fMoveVel);
		VEC_ADD(vNewPos, vNewPos, vF);
	}
	
	
	// See if we should strafe...

	DVector vDims, vR;
	pServerDE->GetObjectDims(m_hObject, &vDims);

	if (m_dwControlFlags & BC_CFLG_STRAFE_RIGHT)
	{
		// Make sure new position isn't off a cliff (or into a wall)...

		if (CanMoveDir(m_vRight))
		{
			VEC_MULSCALAR(vR, m_vRight, fMoveVel);
			VEC_ADD(vNewPos, vNewPos, vR);
		}
	}
	else if (m_dwControlFlags & BC_CFLG_STRAFE_LEFT)
	{
		DVector vLeft;
		VEC_COPY(vLeft, m_vRight);
		VEC_MULSCALAR(vLeft, vLeft, -1.0f); // point left

		// Make sure new position isn't off a cliff (or into a wall)...

		if (CanMoveDir(vLeft))
		{
			VEC_MULSCALAR(vR, m_vRight, fMoveVel);
			VEC_SUB(vNewPos, vNewPos, vR);
		}
	}


	// Save our last position...

	VEC_COPY(m_vLastPos, m_vPos);
	m_fPredTravelDist = VEC_DIST(m_vLastPos, vNewPos);
	
	pServerDE->MoveObject(m_hObject, &vNewPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateOnGround
//
//	PURPOSE:	Update AI on ground
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateOnGround()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// Lets see if we are in the ground or in the air.

	CollisionInfo Info;
	pServerDE->GetStandingOn(m_hObject, &Info);

	if (Info.m_hObject) 
	{
		if (Info.m_Plane.m_Normal.y < 0.76)
		{
			// Force us down...

			DVector vVel;
			pServerDE->GetVelocity(m_hObject, &vVel);

			vVel.y = - VEC_MAG(vVel);
			vVel.x = vVel.z = 0.0f;
			pServerDE->SetVelocity(m_hObject, &vVel);
		}
	} 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateSenses
//
//	PURPOSE:	Update AI senses (sight, hearing, smell)
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateSenses()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return;


	// Until we've spotted a player, keep looking for one...

	if (!m_bSpottedPlayer)
	{
		HOBJECT hObj = pCharMgr->FindVisiblePlayer(this);
		if (hObj)
		{
			if (CheckAlignment(HATE, hObj))
			{
				SetNewTarget(hObj);
				TargetObject(hObj);
			}
			
			SpotPlayer(hObj);
		}
	}


	// Always face target (or last damager)...

	if (m_eState != SCRIPT)
	{
		if (m_hTarget && !m_bSearchingForPlayer)
		{
			TargetObject(m_hTarget);
		}
		else if (m_hLastDamager)
		{
			TargetObject(m_hLastDamager);
		}
		else
		{
			// Listen for enemy firing...

			HOBJECT hObj = pCharMgr->ListenForEnemyFire(this);
			if (hObj)
			{
				SetNewTarget(hObj);
			}
		}

		m_bSearchingForPlayer = DFALSE;  // Will get set if searching for player
	}


	if (m_hTarget)
	{
		UpdateTargetPos();
	}


	// We only need to do a lot of updates if we've got a target, or we're
	// currently processing a script...

	if (m_hTarget || m_hLastDamager || m_eState == SCRIPT)
	{
		pServerDE->SetNextUpdate(m_hObject, AI_UPDATE_DELTA);
	}
	else
	{
		pServerDE->SetNextUpdate(m_hObject, /*AI_UPDATE_DELTA*/ 1.0f);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateWeapon()
//
//	PURPOSE:	Update weapon...
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateWeapon()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return;


	// See if we're firing...

	DBOOL bFire = IsFiring() && !!m_hTarget;
	//DBOOL bFire = (IsFiring() && !!m_hTarget) || pWeapon->ShotPending();


	DVector vFirePos, vTargetPos, vPos, vDir;
	VEC_INIT(vTargetPos);
	VEC_INIT(vPos);
	VEC_INIT(vDir);
	VEC_INIT(vFirePos);
	
	if (bFire)
	{
		vFirePos = GetFirePos(&m_vPos);

		bFire = GetTargetPos(vTargetPos);	
		TargetPos(vTargetPos);

		vDir = GetTargetDir(vFirePos, vTargetPos);
		VEC_NORM(vDir);


		// Make sure fire position is inside world (so we don't shoot through
		// walls)...

		CommonLT* pCommon = pServerDE->Common();
		if (!pCommon || pCommon->GetPointStatus(&vFirePos) != DE_INSIDE)
		{
			bFire = DFALSE;
		}


		// Adjust aiming direction based on our aiming skillz...

		int nPerturbe = GetRandom(s_nMarksmanshipPerturbe[m_eMarksmanship][0], 
								  s_nMarksmanshipPerturbe[m_eMarksmanship][1]);

			
		if (nPerturbe > 0) 
		{
			DVector vTemp;
			DFLOAT rPerturbe = ((DFLOAT)GetRandom(-nPerturbe, nPerturbe))/1000.0f;
			DFLOAT uPerturbe = ((DFLOAT)GetRandom(-nPerturbe, nPerturbe))/1000.0f;

			VEC_MULSCALAR(vTemp, m_vRight, rPerturbe);
			VEC_ADD(vDir, vDir, vTemp);

			VEC_MULSCALAR(vTemp, m_vUp, uPerturbe);
			VEC_ADD(vDir, vDir, vTemp);
		}
	}


	// Need to call this to make sure weapon states get updated...

	pWeapon->UpdateWeapon(m_hObject, vDir, vFirePos, bFire, DFALSE);
	
	
	// We never run out of ammo

	if (pWeapon->GetAmmoCount() <= 0)
	{
		pWeapon->AddAmmo(GetWeaponMaxAmmo(pWeapon->GetId()));
	}
}





//*************************************************************************//
//*************************************************************************//
//
// A I  S C R I P T I N G  F U N C T I O N A L I T Y
// 
// (everything to do with the SCRIPT state)
//*************************************************************************//
//*************************************************************************//


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetScript()
//
//	PURPOSE:	Set state to Script
//
// ----------------------------------------------------------------------- //

void BaseAI::SetScript()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	// Determine if we should push things out of the way or not...

	// Make sure the AI is active...

	pServerDE->SetDeactivationTime(m_hObject, 0.0f);

	m_fRecomputeTime	= 0.0f;
	m_bRecompute		= DFALSE;
	m_bUpdateScriptCmd	= DTRUE;
	m_nScriptCmdIndex	= 0;

	m_eOldState = m_eState;
	m_eState	= SCRIPT;

	// Turn rag-dolling off...

	//if (m_dwScriptFlags & AI_SCRFLG_NORAGDOLL)
	//{
	//	m_damage.SetApplyDamagePhysics(DFALSE);
	//}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateScript()
//
//	PURPOSE:	Implement the script actions
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateScript()
{
	if (m_bUpdateScriptCmd) 
	{
		UpdateScriptCommand();
	}

	switch(m_curScriptCmd.command)
	{
		case AI_SCMD_SETMOVEMENT:
		case AI_SCMD_SETANIMATIONSTATE:
		case AI_SCMD_PLAYSOUND:
		case AI_SCMD_SETSTATE:
		case AI_SCMD_TARGET:
		case AI_SCMD_CHANGEWEAPON:
			m_bUpdateScriptCmd = DTRUE;
		break;
		
		case AI_SCMD_FOLLOWPATH:
			UpdateFollowPathCmd();
		break;

		case AI_SCMD_WAIT:
			UpdateWaitCmd();
		break;
		
		case AI_SCMD_PLAYANIMATION:
			UpdatePlayAnimationCmd();
		break;
		
		case AI_SCMD_MOVETOOBJECT:
			UpdateMoveToObjectCmd();
		break;
		
		case AI_SCMD_FOLLOWOBJECT:
			UpdateFollowObjectCmd();
		break;
		
		case AI_SCMD_DONE:
		default: 
		{
			// Turn rag-dolling back on...

			//if (m_dwScriptFlags & AI_SCRFLG_NORAGDOLL)
			//{
			//	m_damage.SetApplyDamagePhysics(DTRUE);
			//}

			SetState(m_eOldState);
		}
		break;
	}

	
	// Do we take opportunity fire?

	if (m_dwScriptFlags & AI_SCRFLG_OPPORTFIRE)
	{
		if (ChooseOpportunityTarget()) 
		{
			if (ShootTarget() || (m_bSpottedPlayer && (m_dwScriptFlags & AI_SCRFLG_ATTACKTILLDEAD)))
			{
				SetActionFlag(AI_AFLG_AIMING);
				SetActionFlag(AI_AFLG_STAND);  // Don't move if shooting target
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::GetNextScriptCommand()
//
//	PURPOSE:	Update the current script command
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateScriptCommand()
{
	m_bUpdateScriptCmd = DFALSE;

	m_curScriptCmd.command = AI_SCMD_DONE;

	int nNumItems = m_scriptCmdList.GetNumItems();
	if (nNumItems > 0)
	{
		m_curScriptCmd = *(m_scriptCmdList[m_nScriptCmdIndex]);
		
		// Determine what the next script cmd index should be...

		if (m_dwScriptFlags & AI_SCRFLG_LOOP)
		{
			m_nScriptCmdIndex = (m_nScriptCmdIndex + 1 < nNumItems) ? m_nScriptCmdIndex + 1 : 0;
		}
		else  // Non-looping, remove current script command...
		{
			m_nScriptCmdIndex = 0;
			m_scriptCmdList.Remove(0);
		}


		switch(m_curScriptCmd.command)
		{
			case AI_SCMD_SETMOVEMENT:
				SetSetMovementCmd();
			break;

			case AI_SCMD_SETANIMATIONSTATE:
				SetSetAnimationStateCmd();
			break;

			case AI_SCMD_FOLLOWPATH:
				SetFollowPathCmd();
			break;
			
			case AI_SCMD_PLAYSOUND:
				SetPlaysoundCmd();
			break;
			
			case AI_SCMD_SETSTATE:
				SetSetStateCmd();
			break;
			
			case AI_SCMD_TARGET:
				SetTargetCmd();
			break;
			
			case AI_SCMD_WAIT:
				SetWaitCmd();
			break;
			
			case AI_SCMD_PLAYANIMATION:
				SetPlayAnimationCmd();
			break;
			
			case AI_SCMD_CHANGEWEAPON:
				SetChangeWeaponCmd();
			break;
			
			case AI_SCMD_MOVETOOBJECT:
				SetMoveToObjectCmd();
			break;
			
			case AI_SCMD_FOLLOWOBJECT:
				SetFollowObjectCmd();
			break;

			case AI_SCMD_SET_FOLLOWTIME:
				SetFollowTimeCmd();
			break;
			
			case AI_SCMD_SPAWN:
				SetSpawnCmd();
			break;
			
			case AI_SCMD_REMOVE:
				SetRemoveCmd();
			break;
			
			case AI_SCMD_DONE:
			default: 
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetSetMovementCmd()
//
//	PURPOSE:	Set the current AI scripting command to the SetMovementCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::SetSetMovementCmd()
{
	if (stricmp(m_curScriptCmd.args, SCRIPT_MOVEMENT_WALK) == 0)
	{
		m_eScriptMovement = SM_WALK;
	}
	else if (stricmp(m_curScriptCmd.args, SCRIPT_MOVEMENT_RUN) == 0)
	{
		m_eScriptMovement = SM_RUN;
	}	
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetSetAnimationStateCmd()
//
//	PURPOSE:	Set the current AI scripting animation state
//
// ----------------------------------------------------------------------- //

void BaseAI::SetSetAnimationStateCmd()
{
	if (stricmp(m_curScriptCmd.args, SCRIPT_LOOP_ANIMATIONS) == 0)
	{
		m_bLoopScriptedAni = DTRUE;
	}
	else
	{
		m_bLoopScriptedAni = DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetFollowPathCmd()
//
//	PURPOSE:	Set the current AI scripting command to the SetFollowPathCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::SetFollowPathCmd()
{
	m_AIPathList.RemoveAll();

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return;

	CAIPathMgr* pAIPathMgr = pCharMgr->GetAIPathMgr();
	if (!pAIPathMgr) return;
	
	pAIPathMgr->GetPath(m_curScriptCmd.args, &m_AIPathList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateFollowPathCmd()
//
//	PURPOSE:	Update the UpdateFollowPathCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateFollowPathCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	// Reset the flags...

	ClearActionFlags();
	SetActionFlag(AI_AFLG_STAND);


	if (m_AIPathList.IsEmpty())
	{
		m_bUpdateScriptCmd = DTRUE;
		return;
	}

	CAIKeyData* pCurKey = m_AIPathList[0];
	if (!pCurKey)
	{
		m_bUpdateScriptCmd = DTRUE;
		return;
	}


	// Determine where we are going...

	DVector vTargetPos;
	VEC_COPY(vTargetPos, pCurKey->m_vPos);


	// Update the movement to that position...

	if (UpdateScriptMovement(&vTargetPos))
	{
		m_AIPathList.Remove(0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetPlaysoundCmd()
//
//	PURPOSE:	Set the current AI scripting command to the SetPlaysoundCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::SetPlaysoundCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	PlayDialogSound(m_curScriptCmd.args, CST_EXCLAMATION);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetSetStateCmd()
//
//	PURPOSE:	Set the current AI scripting command to the SetSetStateCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::SetSetStateCmd()
{
	SetState(m_curScriptCmd.args);
	m_eOldState = m_eState;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetTargetCmd()
//
//	PURPOSE:	Set the current AI scripting command to the SetTargetCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::SetTargetCmd()
{
	SetTarget(m_curScriptCmd.args);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetWaitCmd()
//
//	PURPOSE:	Set the current AI scripting command to the SetWaitCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::SetWaitCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_fScriptWaitEnd = m_fCurTime + atoi(m_curScriptCmd.args);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateWaitCmd()
//
//	PURPOSE:	Update the UpdateWaitCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateWaitCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_fCurTime >= m_fScriptWaitEnd)
	{
		m_bUpdateScriptCmd = DTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetPlayAnimationCmd()
//
//	PURPOSE:	Set the current AI scripting command to the SetPlayAnimationCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::SetPlayAnimationCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DDWORD dwIndex = pServerDE->GetAnimIndex(m_hObject, m_curScriptCmd.args);
	pServerDE->SetModelLooping(m_hObject, m_bLoopScriptedAni);
	pServerDE->SetModelAnimation(m_hObject, dwIndex);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdatePlayAnimationCmd()
//
//	PURPOSE:	Update the UpdatePlayAnimationCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdatePlayAnimationCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DDWORD dwState = pServerDE->GetModelPlaybackState(m_hObject);

	if ((dwState & MS_PLAYDONE))
	{
		m_bUpdateScriptCmd = DTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetChangeWeaponCmd()
//
//	PURPOSE:	Change the AI's weapon
//
// ----------------------------------------------------------------------- //

void BaseAI::SetChangeWeaponCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	RiotWeaponId nWeaponId = (RiotWeaponId)atoi(m_curScriptCmd.args);
	if (!m_weapons.IsValidIndex(nWeaponId)) return;

	m_nWeaponId = nWeaponId;
	m_weapons.ObtainWeapon(m_nWeaponId);
	m_weapons.ChangeWeapon(m_nWeaponId);
	m_weapons.AddAmmo(m_nWeaponId, GetWeaponMaxAmmo(m_nWeaponId));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetMoveToObjectCmd()
//
//	PURPOSE:	Move to another object's position
//
// ----------------------------------------------------------------------- //

void BaseAI::SetMoveToObjectCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	ObjectList*	pList = pServerDE->FindNamedObjects(m_curScriptCmd.args);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	if (pLink && pLink->m_hObject)
	{
		pServerDE->GetObjectPos(pLink->m_hObject, &m_vMoveToPos);
		FacePos(m_vMoveToPos);
	}

	pServerDE->RelinquishList(pList);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateMoveToObjectCmd()
//
//	PURPOSE:	Update the UpdateMoveToObjectCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateMoveToObjectCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	// Reset the flags...

	ClearActionFlags();
	SetActionFlag(AI_AFLG_STAND);

	// Update the movement to the move-to position...

	if (UpdateScriptMovement(&m_vMoveToPos))
	{
		m_bUpdateScriptCmd = DTRUE;
	}

	return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetFollowTimeCmd()
//
//	PURPOSE:	Set how long an object follows another object
//
// ----------------------------------------------------------------------- //

void BaseAI::SetFollowTimeCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_fFollowTime = (DFLOAT)atof(m_curScriptCmd.args);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetSpawnCmd()
//
//	PURPOSE:	Spawn the specified sheeyot
//
// ----------------------------------------------------------------------- //

void BaseAI::SetSpawnCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DRotation rRot;
	ROT_INIT(rRot);

	SpawnItem(m_curScriptCmd.args, m_vPos, rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetRemoveCmd()
//
//	PURPOSE:	Remove the AI
//
// ----------------------------------------------------------------------- //

void BaseAI::SetRemoveCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	RemoveHandHeldWeapon();
	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetFollowObjectCmd()
//
//	PURPOSE:	Follow another object
//
// ----------------------------------------------------------------------- //

void BaseAI::SetFollowObjectCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	ObjectList*	pList = pServerDE->FindNamedObjects(m_curScriptCmd.args);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	if (pLink && pLink->m_hObject)
	{
		SetNewLeader(pLink->m_hObject);
	}

	pServerDE->RelinquishList(pList);

	if (m_hLeader)
	{
		m_fFollowStartTime = m_fCurTime;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateFollowObjectCmd()
//
//	PURPOSE:	Update the UpdateFollowObjectCmd
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateFollowObjectCmd()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	// Reset the flags...

	ClearActionFlags();
	SetActionFlag(AI_AFLG_STAND);


	// Will only stop following leader, if script is changed, or leader
	// dies...

	if (!m_hLeader || (m_fFollowTime > 0 && m_fCurTime > m_fFollowStartTime + m_fFollowTime))
	{
		m_bUpdateScriptCmd = DTRUE;
		return;
	}


	// Determine where we should move to...

	DVector vNewPos;
	pServerDE->GetObjectPos(m_hLeader, &vNewPos);

	DFLOAT fDistance = VEC_DIST(m_vPos, vNewPos);

	if (fDistance <= AI_FOLLOW_DISTANCE)
	{
		m_bLostLeader	 = DFALSE;
		return;  // We're close enough...
	}
	else if (fDistance > AI_MAX_FOLLOW_DISTANCE)
	{
		if (!m_bLostLeader && (m_dwAvailableSounds & AI_SNDFLG_FOLLOWLOST)) 
		{
			char* pSound = DNULL;
			if (m_hstrFollowLostSound)
			{
				pSound = pServerDE->GetStringData(m_hstrFollowLostSound);
			}
			else
			{
				pSound = GetFollowLostSound(this);
			}

			PlayDialogSound(pSound);
		}

		m_bLostLeader = DTRUE;

		if (IsPlayer(m_hLeader))
		{
			SearchForPlayer(m_hLeader);
		}
		return;  // Our leader left us :(
	}

	// Move towards our leader

	UpdateScriptMovement(&vNewPos);

	return;
}





//*************************************************************************//
//*************************************************************************//
//
// A I   U T I L I T Y   F U N C T I O N S
// 
// (leftovers, helper functions)
//*************************************************************************//
//*************************************************************************//



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateScriptMovement()
//
//	PURPOSE:	Update the ai movement due to a scripted command
//				(return TRUE if we are at the target position)
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::UpdateScriptMovement(DVector* pvTargetPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !pvTargetPos) return DTRUE;

	DVector vTargetPos;
	VEC_COPY(vTargetPos, *pvTargetPos);


	// If we shouldn't be moved by outside forces, zero out our acceleration
	// and velocity...(don't zero out -y accelerations/velocities (assume this
	// is just gravity)

	if (m_dwScriptFlags & AI_SCRFLG_NORAGDOLL)
	{
		DVector vVel;
		pServerDE->GetVelocity(m_hObject, &vVel);
		vVel.x = vVel.z = 0.0f;
		if (vVel.y > 0.0f) vVel.y = 0.0f;

		pServerDE->SetVelocity(m_hObject, &vVel);

		pServerDE->GetAcceleration(m_hObject, &vVel);
		vVel.x = vVel.z = 0.0f;
		if (vVel.y > 0.0f) vVel.y = 0.0f;

		pServerDE->SetAcceleration(m_hObject, &vVel);
	}


	// Depending on what our movement setting is, walk or run toward the
	// position...

	DFLOAT fSpeed = m_fWalkVel;

	switch (m_eScriptMovement)
	{
		case SM_RUN:
			RunForward();
			fSpeed = m_fRunVel;
		break;

		case SM_WALK:
		default :
			WalkForward();
			fSpeed = m_fWalkVel;
		break;
	}
	
	
	DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

	// Don't let the Y distance be a factor...

	vTargetPos.y = vPos.y;

	
	DFLOAT fDistLeft = VEC_DIST(vPos, vTargetPos);
	DFLOAT fDistPredict = pServerDE->GetFrameTime() * fSpeed;

	if (fDistLeft <= fDistPredict * 2.0f /*+ AI_SCRIPTMOVEMENT_ERROR*/)
	{
		pServerDE->MoveObject(m_hObject, &vTargetPos);
		return DTRUE;  // At target pos...
	}


	// See if we're stuck on something...

	//if (m_bStuckOnSomething)
	//{
		// Okay, we're stuck...Pretend like we're okay with it (i.e., 
		// just play our standing ani ;)

	//	ClearActionFlags();
	//	SetActionFlag(AI_AFLG_STAND);
	//}


	// Face where we are going...

	FacePos(vTargetPos);


	return DFALSE;  // Not to target pos yet
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::ChooseTarget()
//
//	PURPOSE:	Choose a new target if one available
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::ChooseTarget()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;


	// If our target is different from the last thing that damaged
	// us, we'll see if we should change targets...

	if (m_hLastDamager && m_hTarget != m_hLastDamager)
	{
		// Check the alignment on the last damager...

		HCLASS hClass = pServerDE->GetObjectClass(m_hLastDamager);
		HCLASS hBaseTest = pServerDE->GetClass("CBaseCharacter");

		if (pServerDE->IsKindOf(hClass, hBaseTest)) 
		{
			if (CheckAlignment(LIKE, m_hLastDamager) && 
				GetRandom(0.0f, 100.0f) < AI_ATTACK_CHANCE_FRIEND)
			{
				SetNewTarget(m_hLastDamager);
			}
			else if (CheckAlignment(TOLERATE, m_hLastDamager) && 
				     GetRandom(0.0f, 100.0f) < AI_ATTACK_CHANCE_TOLERATE)
			{
				SetNewTarget(m_hLastDamager);
			}
			else if (CheckAlignment(HATE, m_hLastDamager) &&
					 (!m_hTarget || !CheckAlignment(HATE, m_hTarget)))
			{
				SetNewTarget(m_hLastDamager);
			}
		} 
	}


	// If we have a current target or just picked one from above,
	// see if it's still valid...

	if (m_hTarget)
	{
		CBaseCharacter* pB = (CBaseCharacter*)pServerDE->HandleToObject(m_hTarget);
		if (!pB) return DFALSE;

		if (pB->IsDead()) 
		{
			// Stop coming after this target...

			SetNewTarget(DNULL);
			m_bSpottedPlayer = DFALSE;
		} 
		else if (IsPlayer(m_hTarget))
		{
			return DTRUE;  // Keep the player as our target...
		}
	}


	// Okay we don't have a target, see if we can find one...

	DBOOL bRet = DFALSE;

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return DFALSE;

	HOBJECT hTarget = pCharMgr->FindAITarget(this);
	if (hTarget)
	{
		SetNewTarget(hTarget);
		bRet = DTRUE;
	}


	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::ChooseOpportunityTarget()
//
//	PURPOSE:	Choose a new target if one is available while processing
//				a script
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::ChooseOpportunityTarget()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;


	// If our target is different from the last thing that damaged
	// us, we'll see if we should change targets...

	if (m_hLastDamager && m_hTarget != m_hLastDamager)
	{
		// Check the alignment on the last damager...

		HCLASS hClass = pServerDE->GetObjectClass(m_hLastDamager);
		HCLASS hBaseTest = pServerDE->GetClass("CBaseCharacter");

		if (pServerDE->IsKindOf(hClass, hBaseTest)) 
		{
			if (CheckAlignment(LIKE, m_hLastDamager) && 
				GetRandom(0.0f, 100.0f) < AI_ATTACK_CHANCE_FRIEND)
			{
				if (InLineOfSight(m_hLastDamager))
				{
					SetNewTarget(m_hLastDamager);
				}
			}
			else if (CheckAlignment(TOLERATE, m_hLastDamager) && 
				     GetRandom(0.0f, 100.0f) < AI_ATTACK_CHANCE_TOLERATE)
			{
				if (InLineOfSight(m_hLastDamager))
				{
					SetNewTarget(m_hLastDamager);
				}
			}
			else if (CheckAlignment(HATE, m_hLastDamager) &&
					 (!m_hTarget || !CheckAlignment(HATE, m_hTarget)))
			{
				if (InLineOfSight(m_hLastDamager))
				{
					SetNewTarget(m_hLastDamager);
				}
			}
		} 
	}


	// If we have a current target or just picked one from above,
	// see if it's still valid...

	if (m_hTarget)
	{
		CBaseCharacter* pB = (CBaseCharacter*)pServerDE->HandleToObject(m_hTarget);
		if (!pB) return DFALSE;

		if (pB->IsDead()) 
		{
			// Stop coming after this target...

			SetNewTarget(DNULL);
			m_bSpottedPlayer = DFALSE;
		} 
		else if ((m_dwScriptFlags & AI_SCRFLG_ATTACKTILLDEAD) || InLineOfSight(m_hTarget))
		{
			return DTRUE;  // Keep this target...
		}
	}


	// Okay we don't have a target, see if we can find one...

	DBOOL bRet = DFALSE;

	CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
	if (!pCharMgr) return DFALSE;

	HOBJECT hTarget = pCharMgr->FindAITarget(this, 	!(m_dwScriptFlags & AI_SCRFLG_ATTACKTILLDEAD));
	if (hTarget)
	{
		SetNewTarget(hTarget);
		bRet = DTRUE;
	}


	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::ShootTarget()
//
//	PURPOSE:	Shoot at a target if there is one eligable to be shot
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::ShootTarget()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject ) return DFALSE;

	if (!m_hTarget /*|| !TargetInRange()*/) return DFALSE;

	SetActionFlag(AI_AFLG_AIMING);

	// See if we should fire or wait...

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return DFALSE;

	if (m_fCurTime < m_fFireStopTime)  // We're firing...
	{
		SetActionFlag(AI_AFLG_FIRE);
		return DTRUE;
	}
	else if (m_fCurTime > m_fFireStartTime) // We just got done resting...
	{
		m_fFireStopTime = (m_fCurTime + GetRandom(pWeapon->GetMinFireDuration(), pWeapon->GetMaxFireDuration()));

		m_fFireStartTime = (m_fFireStopTime + GetRandom(pWeapon->GetMinFireRest() * m_fFireRestAdjust,
													    pWeapon->GetMinFireRest() * m_fFireRestAdjust));
		return DTRUE;
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::HitByObject()
//
//	PURPOSE:	Notification that we are hit by something
//
// ----------------------------------------------------------------------- //

void BaseAI::HitByObject(HOBJECT hObj, DFLOAT fDamage)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	DBOOL bOkToTargetObject = DFALSE;

	if (m_eState != SCRIPT || (m_dwScriptFlags & AI_SCRFLG_INT))
	{
		m_bRecompute	  = DTRUE;
		bOkToTargetObject = DTRUE;
		pServerDE->SetNextUpdate(m_hObject, AI_UPDATE_DELTA);
	}
	

	if (hObj && IsBaseCharacter(hObj))
	{
		if (!m_damage.IsDead() && bOkToTargetObject)
		{
			// Target whoever damaged us...

			TargetObject(hObj);
		}

		if (hObj != m_hObject)
		{
			SetNewDamager(hObj);

			// Get some help :)

			CBaseCharacter* pB = (CBaseCharacter*)g_pServerDE->HandleToObject(hObj);
			if (!pB) return;

			// Make sure it wasn't friendly fire...

			if (GetAlignement(GetCharacterClass(), pB->GetCharacterClass()) == HATE)
			{
				CCharacterMgr* pCharMgr = CCharacterMgr::GetMgr();
				if (!pCharMgr) return;

				pCharMgr->CallForBackup(this);
			}
		}
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateSounds
//
//	PURPOSE:	Update our sounds...
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateSounds()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	CBaseCharacter::UpdateSounds();

	// See if we should play a sound...

	if (m_fCurTime > m_fNextSoundTime)
	{
		m_fNextSoundTime = m_fCurTime + GetRandom(AI_MIN_SOUND_DELAY, AI_MAX_SOUND_DELAY);
	
		if (GetRandom(0,1) == 1) return;  // Only play sounds 50% of the time

		switch (m_eState)
		{
			case IDLE: 
			{
				if (m_dwAvailableSounds & AI_SNDFLG_IDLE)
				{
					char* pSound = DNULL;
					if (m_hstrIdleSound)
					{
						pSound = pServerDE->GetStringData(m_hstrIdleSound);
					}
					PlayDialogSound(pSound);
				}
			}
			break;

			case DEFENSIVE: 
			{
				if (m_dwAvailableSounds & AI_SNDFLG_DEFENSIVE)
				{
					char* pSound = DNULL;
					if (m_hstrDefensiveSound)
					{
						pSound = pServerDE->GetStringData(m_hstrDefensiveSound);
					}
					PlayDialogSound(pSound);	
				}
			}
			break;

			case AGGRESSIVE: 
			{
				if (m_dwAvailableSounds & AI_SNDFLG_AGGRESSIVE)
				{
					char* pSound = DNULL;
					if (m_hstrAggressiveSound)
					{
						pSound = pServerDE->GetStringData(m_hstrAggressiveSound);
					}
					PlayDialogSound(pSound);	
				}
			}
			break;

			case RETREATING: 
			{
				if (m_dwAvailableSounds & AI_SNDFLG_RETREATING)
				{
					char* pSound = DNULL;
					if (m_hstrRetreatingSound)
					{
						pSound = pServerDE->GetStringData(m_hstrRetreatingSound);
					}
					PlayDialogSound(pSound);	
				}
			}
			break;

			case GUARDING: 
			{
				if (m_dwAvailableSounds & AI_SNDFLG_GUARDING)
				{
					char* pSound = DNULL;
					if (m_hstrGuardingSound)
					{
						pSound = pServerDE->GetStringData(m_hstrGuardingSound);
					}
					PlayDialogSound(pSound);	
				}
			}
			break;

			case PANICKED: 
			{
				if (m_dwAvailableSounds & AI_SNDFLG_PANICKED)
				{
					char* pSound = DNULL;
					if (m_hstrPanickedSound)
					{
						pSound = pServerDE->GetStringData(m_hstrPanickedSound);
					}
					else
					{
						pSound = GetPanickedSound(this);
					}

					PlayDialogSound(pSound);	
				}
			}
			break;

			case PSYCHO: 
			{
				if (m_dwAvailableSounds & AI_SNDFLG_PSYCHO)
				{
					char* pSound = DNULL;
					if (m_hstrPsychoSound)
					{
						pSound = pServerDE->GetStringData(m_hstrPsychoSound);
					}
					PlayDialogSound(pSound);	
				}
			}
			break;

			case DYING:
			default: break;
		}	
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::PlayDeathSound()
//
//	PURPOSE:	Play the death sound
//
// ----------------------------------------------------------------------- //

void BaseAI::PlayDeathSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !(m_dwAvailableSounds & AI_SNDFLG_DEATH)) return;

	char* pSound = DNULL;

	if (g_bRobert)
	{
		char* pRobert[] = 
		{ 
			"Sounds\\Enemies\\Bob1.wav", 
			"Sounds\\Enemies\\Bob2.wav",
			"Sounds\\Enemies\\Bob3.wav" 
		};

		PlayDialogSound(pRobert[GetRandom(0,2)], CST_DEATH, DTRUE);
		return;
	}
	else if (m_hstrDeathSound)
	{
		pSound = pServerDE->GetStringData(m_hstrDeathSound);
		if (pSound) 
		{
			PlayDialogSound(pSound, CST_DEATH, DTRUE);
			return;
		}
	}

	CBaseCharacter::PlayDeathSound();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetDeathSound
//
//	PURPOSE:	Determine what death sound to play
//
// ----------------------------------------------------------------------- //

char* BaseAI::GetDeathSound()
{	
	return ::GetDeathSound(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseCharacter::GetDamageSound
//
//	PURPOSE:	Determine what damage sound to play
//
// ----------------------------------------------------------------------- //

char* BaseAI::GetDamageSound(DamageType)
{	
	return ::GetDamageSound(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::GetEyeLevel()
//
//	PURPOSE:	Compute our 'eye level'
//
// ----------------------------------------------------------------------- //

DVector BaseAI::GetEyeLevel()
{
	DVector vPos, vDims;
	VEC_COPY(vPos, m_vPos);

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return vPos;

	pServerDE->GetObjectDims(m_hObject, &vDims);

	vPos.y += vDims.y / 2.0f;
	vPos.y -= 2.0f;

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::GetKneeLevel()
//
//	PURPOSE:	Compute our 'knee level'
//
// ----------------------------------------------------------------------- //

DVector BaseAI::GetKneeLevel()
{
	DVector vPos, vDims;
	VEC_COPY(vPos, m_vPos);

	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return vPos;

	pServerDE->GetObjectDims(m_hObject, &vDims);

	vPos.y -= vDims.y / 2.0f;
	vPos.y += 2.0f;

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CheckForCollision()
//
//	PURPOSE:	See if there is anything fDist away in the vDir
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::CheckForCollision(DVector vDir, DFLOAT fDist)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DVector vPos = HandHeldWeaponFirePos(); 

	VEC_NORM(vDir);
	VEC_MULSCALAR(vDir, vDir, fDist);

	DVector vTo;
	VEC_ADD(vTo, vPos, vDir);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To, vTo);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = DNULL;

	DFLOAT fEyeDist  = AI_MAX_DISTANCE;
	DFLOAT fKneeDist = AI_MAX_DISTANCE;

	s_nNumCallsToIntersectSegment++;
	if (pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		return DTRUE;
	}

	vPos = GetKneeLevel();

	VEC_ADD(vTo, vPos, vDir);
	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To, vTo);

	s_nNumCallsToIntersectSegment++;
	if (pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		return DTRUE;
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CheckForCollisionForward()
//
//	PURPOSE:	Check for forward collisions
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::CheckForCollisionForward(DFLOAT fDist)
{
	return CheckForCollision(m_vForward, fDist);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CheckForCollisionBackward()
//
//	PURPOSE:	Check for backward collisions
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::CheckForCollisionBackward(DFLOAT fDist)
{
	DVector vDir;
	VEC_COPY(vDir, m_vForward);
	VEC_MULSCALAR(vDir, vDir, -1.0f); // point backward

	return CheckForCollision(vDir, fDist);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CheckForCollisionRight()
//
//	PURPOSE:	Check for collision to the right
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::CheckForCollisionRight(DFLOAT fDist)
{
	return CheckForCollision(m_vRight, fDist);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CheckForCollisionLeft()
//
//	PURPOSE:	Check for collision to the left
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::CheckForCollisionLeft(DFLOAT fDist)
{
	DVector vDir;
	VEC_COPY(vDir, m_vRight);
	VEC_MULSCALAR(vDir, vDir, -1.0f); // point left
	
	return CheckForCollision(vDir, fDist);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::IsObjectVisible()
//
//	PURPOSE:	Is the test object visible
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::IsObjectVisible(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !hObj) return DFALSE;

	DVector vTheirPos;
	DVector vPos = GetEyeLevel(); // HandHeldWeaponFirePos();

	pServerDE->GetObjectPos(hObj, &vTheirPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To, vTheirPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = TransparentObjectFilterFn;

	s_nNumCallsToIntersectSegment++;
	if (pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		if (IInfo.m_hObject == hObj)
		{
			return DTRUE;
		}
	}

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::IsPosVisible()
//
//	PURPOSE:	Is the test position visible
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::IsPosVisible(DVector* pvPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !pvPos) return DFALSE;

	DVector vPos = GetEyeLevel(); // HandHeldWeaponFirePos();

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To, *pvPos);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = TransparentObjectFilterFn;

	s_nNumCallsToIntersectSegment++;
	if (pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		return DFALSE;  // Damn, hit something
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::IsPosVisibleToAI()
//
//	PURPOSE:	Is the test position visible to us (regardless of direction)
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::IsPosVisibleToAI(DVector* pvPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !pvPos) return DFALSE;

	DVector vDir;
	VEC_SUB(vDir, *pvPos, m_vPos);
	
	if (VEC_MAG(vDir) < m_fVisibleRange)
	{
		return IsPosVisible(pvPos);
	}
		
	return DFALSE;
}	


DBOOL TransparentObjectFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj || !g_pServerDE) return DFALSE;
	
	HCLASS hGlass = g_pServerDE->GetClass("GlassProp");

	if (g_pServerDE->IsKindOf(g_pServerDE->GetObjectClass(hObj), hGlass))
	{
		return DFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::IsObjectVisibleToAI()
//
//	PURPOSE:	Is the test object visible to us (forward + FoV)?
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::IsObjectVisibleToAI(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !hObj || m_fVisibleRange <= 0.0f) return DFALSE;

	DVector vTheirPos;
	pServerDE->GetObjectPos(hObj, &vTheirPos);

	DVector vDir;
	VEC_SUB(vDir, vTheirPos, m_vPos);
	
	// Do the quick tests first...

	// Test to see if AI is facing test object...

	DFLOAT fDp = (vDir.x * m_vForward.x) + (vDir.y * m_vForward.y) + (vDir.z * m_vForward.z);
		
	if (fDp <= 0) return DFALSE;
	

	// AI is facing test object so see if we are close enough to see it...

	DFLOAT fDist = VEC_MAG(vDir);

		if (fDist > m_fVisibleRange) return DFALSE;


	// Okay see if there is any thing blocking our line of sight...

	if (!IsObjectVisible(hObj)) return DFALSE;


#ifndef LIGHT_AFFECTS_AI_VISION
	
	return DTRUE;

#else
	// Okay see if our target is hiding in a shadow...

	DVector vColor;
	if (pServerDE->GetPointShade(&vTheirPos, &vColor))
	{
		//pServerDE->BPrint("PointShade: %.2f, %.2f, %.2f", vColor.x, vColor.y, vColor.z);
		
		DFLOAT fAve = VEC_MAG(vColor) / 3.0f; // Average colors...
		if (fAve > 200.0) return DTRUE;		  // too light
				
		DFLOAT fMinRange = m_fVisibleRange * 0.125f;
		DFLOAT fRange	 = m_fVisibleRange - fMinRange;
		DFLOAT fNewDist  = (fMinRange + (fRange * fAve/200.0f));

		//pServerDE->BPrint("Max Sight: %.2f", fNewDist);

		if (fDist < fNewDist)
		{
			return DTRUE;
		}
	}	
#endif		

	return DFALSE;
}			


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::InLineOfSight()
//
//	PURPOSE:	Is the test object in our line of sight
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::InLineOfSight(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !hObj || m_fVisibleRange <= 0.0f) return DFALSE;

	DVector vTheirPos;
	pServerDE->GetObjectPos(hObj, &vTheirPos);

	DVector vDir;
	VEC_SUB(vDir, vTheirPos, m_vPos);
	
	// Test to see if AI is facing test object...

	DFLOAT fDp = (vDir.x * m_vForward.x) + (vDir.y * m_vForward.y) + (vDir.z * m_vForward.z);
		
	if (fDp <= 0) return DFALSE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CanAIHearObject()
//
//	PURPOSE:	Can we hear the test object
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::CanAIHearObject(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !hObj) return DFALSE;

	DVector vTheirPos;
	pServerDE->GetObjectPos(hObj, &vTheirPos);

	DVector vDir;
	VEC_SUB(vDir, vTheirPos, m_vPos);

	if (VEC_MAG(vDir) < m_fHearingRange) 
	{
		if (IsObjectVisible(hObj)) return DTRUE;
	}

	return DFALSE;
}	


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CanAIHearWeaponFire()
//
//	PURPOSE:	Can we hear the character firing (return who fired)
//
// ----------------------------------------------------------------------- //

HOBJECT BaseAI::CanAIHearWeaponFire(CBaseCharacter* pChar)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !pChar) return DNULL;

	// If we have a target, don't bother listening for other enemies
	// firing...

	if (m_hTarget) return DNULL;

	DVector vFiredPos, vImpactPos;
	DBYTE nWeaponId;
	DFLOAT fFireTime;
	DBOOL bSilenced;

	pChar->GetLastFireInfo(vFiredPos, vImpactPos, nWeaponId, fFireTime, bSilenced);

	// Make sure this is a recent firing of the weapon...

	if (fFireTime + 2.0 < m_fCurTime || nWeaponId == GUN_NONE) return DNULL;

	// If we're close enough to hear the impact, or the weapon itself,
	// turn towards the fire pos...

	DFLOAT fRadius = (DFLOAT)GetWeaponFireSoundRadius(nWeaponId, pChar->GetModelSize());
	if (bSilenced) fRadius *= 0.25f;
		
	DFLOAT fDist = VEC_DIST(vFiredPos, m_vPos);
	if (fDist < fRadius)
	{
		// If we can't see their position, then we can't hear it either...(can't
		// hear through walls)...

		if (IsPosVisibleToAI(&vFiredPos))
		{
			FacePos(vFiredPos);
			return pChar->m_hObject;
		}
	}
	 
	fRadius = (DFLOAT)GetWeaponImpactSoundRadius(nWeaponId, pChar->GetModelSize());
	fDist   = VEC_DIST(vImpactPos, m_vPos);

	if (fDist < fRadius)
	{
		// If we can't see their position, then we can't hear it either...(can't
		// hear through walls)...

		//if (IsPosVisibleToAI(&vImpactPos))
		//{
			FacePos(vFiredPos);
			return pChar->m_hObject;
		//}
	}


	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::FaceObject()
//
//	PURPOSE:	Turn to face a specific object
//
// ----------------------------------------------------------------------- //

void BaseAI::FaceObject(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hObj) return;

	DVector vTargetPos;
	pServerDE->GetObjectPos(hObj, &vTargetPos);
	FacePos(vTargetPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::FacePos()
//
//	PURPOSE:	Turn to face a specific pos
//
// ----------------------------------------------------------------------- //

void BaseAI::FacePos(DVector vTargetPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	vTargetPos.y = m_vPos.y; // Don't look up/down.

	DVector vDir;
	VEC_SUB(vDir, vTargetPos, m_vPos);
	VEC_NORM(vDir);

	DRotation rRot;
	pServerDE->AlignRotation(&rRot, &vDir, NULL);
	pServerDE->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::TargetObject()
//
//	PURPOSE:	Aim at a specific object
//
// ----------------------------------------------------------------------- //

void BaseAI::TargetObject(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hObj) return;

	DVector vTargetPos;
	pServerDE->GetObjectPos(hObj, &vTargetPos);
	TargetPos(vTargetPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::TargetPos()
//
//	PURPOSE:	Turn to target a specific pos
//
// ----------------------------------------------------------------------- //

void BaseAI::TargetPos(DVector vTargetPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

	// Default, same as FacePos()...

	FacePos(vTargetPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::TargetInRange()
//
//	PURPOSE:	Is the target in our firing range?
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::TargetInRange(AIRange eType)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !m_hTarget) return DFALSE;

	DFLOAT fDistance = DistanceToObject(m_hTarget);

	DBOOL bRet = DFALSE;

	switch (eType)
	{
		case SHORT:
		case MID:
		case LONG:
			bRet = (fDistance <= m_fVisibleRange);
		break;

		default : break;
	}
	
	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DistanceToObject()
//
//	PURPOSE:	Find the distance to the object
//
// ----------------------------------------------------------------------- //

DFLOAT BaseAI::DistanceToObject(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject || !hObj) return -1.0f;

	DVector vTheirPos;
	pServerDE->GetObjectPos(hObj, &vTheirPos);

	return VEC_DIST(vTheirPos, m_vPos);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheckAlignment
//
//	PURPOSE:	Test to see if my alignment to hObj matches that of ca
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::CheckAlignment(CharacterAlignment ca, HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hObj) return DFALSE;

	CBaseCharacter* pB = (CBaseCharacter*)pServerDE->HandleToObject(hObj);
	if (!pB) return DFALSE;

	return (GetAlignement(GetCharacterClass(), pB->GetCharacterClass()) == ca);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::CanMoveDir()
//
//	PURPOSE:	Determine if we can move the specified direction
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::CanMoveDir(DVector & vDir, DFLOAT fCollisionDist)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;
    
	DVector vPos, vTemp, vDims;
	VEC_COPY(vPos, m_vPos);
	VEC_INIT(vTemp);
    
	pServerDE->GetObjectDims(m_hObject, &vDims);

    DFLOAT fStepHeight = vDims.y * 2.0f;  // Can jump down our own body height
    
	DFLOAT fVal = vDims.x * 1.5f;

	// Make sure we're not against a wall...

	if (fCollisionDist < 0.0f)
	{
		fCollisionDist = fVal;
	}

	if (CheckForCollision(vDir, fCollisionDist)) return DFALSE;


	// Check for ledges...


	// Determine if we should check infront, or behind us...

	if (m_dwControlFlags & BC_CFLG_REVERSE)
	{
		fVal = -fVal;
	}


	VEC_MULSCALAR(vTemp, vDir, fVal);
	VEC_ADD(vPos, vPos, vTemp);
    
    // vPos is a point in front of/behind you.
    // vTo is a point in front of/behind you, but down
    
	DVector vTo;
	VEC_COPY(vTo, vPos);
	vTo.y = (vTo.y - vDims.y) - fStepHeight;

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To, vTo);
    
	IQuery.m_Flags	   = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn  = LiquidFilterFn;

	s_nNumCallsToIntersectSegment++;
	if (pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
    	return DTRUE;
	}
    
  	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::RunForward
//
//	PURPOSE:	Move the AI forward fast
//
// ----------------------------------------------------------------------- //

void BaseAI::RunForward()
{
	ClearActionFlag(AI_AFLG_STAND);
	SetActionFlag(AI_AFLG_RUN);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::RunBackward
//
//	PURPOSE:	Move the AI backward fast
//
// ----------------------------------------------------------------------- //

void BaseAI::RunBackward()
{
	ClearActionFlag(AI_AFLG_STAND);
	SetActionFlag(AI_AFLG_RUN_REVERSE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::WalkForward
//
//	PURPOSE:	Move the AI forward slow
//
// ----------------------------------------------------------------------- //

void BaseAI::WalkForward()
{
	ClearActionFlag(AI_AFLG_STAND);
	SetActionFlag(AI_AFLG_WALK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::WalkBackward
//
//	PURPOSE:	Move the AI backward slow
//
// ----------------------------------------------------------------------- //

void BaseAI::WalkBackward()
{
	ClearActionFlag(AI_AFLG_STAND);
	SetActionFlag(AI_AFLG_WALK_REVERSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::StrafeLeft
//
//	PURPOSE:	Move the AI to the left
//
// ----------------------------------------------------------------------- //

void BaseAI::StrafeLeft()
{
	ClearActionFlag(AI_AFLG_STAND);
	ClearActionFlag(AI_AFLG_STRAFE_RIGHT);
	SetActionFlag(AI_AFLG_STRAFE_LEFT);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::StrafeLeft
//
//	PURPOSE:	Move the AI to the right
//
// ----------------------------------------------------------------------- //

void BaseAI::StrafeRight()
{
	ClearActionFlag(AI_AFLG_STAND);
	ClearActionFlag(AI_AFLG_STRAFE_LEFT);
	SetActionFlag(AI_AFLG_STRAFE_RIGHT);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::TurnRorL
//
//	PURPOSE:	Randomly turn the AI right or left
//
// ----------------------------------------------------------------------- //

void BaseAI::TurnRorL()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;
    
	// Pick a direction to turn...

    if (m_eLastTurnDir == RIGHT)
	{
	    TurnRight();
	}
	else if (m_eLastTurnDir == LEFT)
	{
        TurnLeft();
	}
    else if ( (pServerDE->IntRandom(1, 2) == 1) )
	{
	    TurnRight();
	}
	else
	{
        TurnLeft();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::TurnRight
//
//	PURPOSE:	Turn the AI right
//
// ----------------------------------------------------------------------- //

void BaseAI::TurnRight()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

    DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);
    pServerDE->EulerRotateY(&rRot, DEG2RAD(90.0f));
	pServerDE->SetObjectRotation(m_hObject, &rRot);	
  	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);
      
    m_eLastTurnDir = RIGHT;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::TurnLeft
//
//	PURPOSE:	Turn the AI left
//
// ----------------------------------------------------------------------- //

void BaseAI::TurnLeft()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return;

    DRotation rRot;
	pServerDE->GetObjectRotation(m_hObject, &rRot);
    pServerDE->EulerRotateY(&rRot, DEG2RAD(-90.0f));
	pServerDE->SetObjectRotation(m_hObject, &rRot);	
   	pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);
       
    m_eLastTurnDir = LEFT;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::AvoidObstacle
//
//	PURPOSE:	See if strafing left or right will allow us to continue
//				moving forward
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::AvoidObstacle()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hObject) return DFALSE;

	DVector vPos, vDims, vFrom, vTo;
	VEC_COPY(vPos, m_vPos);

	pServerDE->GetObjectDims(m_hObject, &vDims);

	DFLOAT fForwardDistance = vDims.x + 20.0f;
	DFLOAT fRightDistance	= vDims.x + 10.0f;

	// See if we should strafe right...

	VEC_MULSCALAR(vFrom, m_vRight, fRightDistance);
	VEC_ADD(vFrom, vFrom, vPos);

	VEC_MULSCALAR(vTo, m_vForward, fForwardDistance);
	VEC_ADD(vTo, vTo, vFrom);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vFrom);
	VEC_COPY(IQuery.m_To, vTo);
	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = DNULL;

	s_nNumCallsToIntersectSegment++;
	if (!pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		StrafeRight();
		return DFALSE;
	}


	// No, how about left...

	VEC_MULSCALAR(vFrom, m_vRight, -fRightDistance);
	VEC_ADD(vFrom, vFrom, vPos);

	VEC_MULSCALAR(vTo, m_vForward, fForwardDistance);
	VEC_ADD(vTo, vTo, vFrom);

	VEC_COPY(IQuery.m_From, vFrom);
	VEC_COPY(IQuery.m_To, vTo);
	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = DNULL;

	s_nNumCallsToIntersectSegment++;
	if (!pServerDE->IntersectSegment(&IQuery, &IInfo))
	{
		StrafeLeft();
		return DFALSE;
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateEvasiveAction()
//
//	PURPOSE:	Basically just strafe right and left...
//
// ----------------------------------------------------------------------- //

void BaseAI::UpdateEvasiveAction()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hTarget) return;
	
	if (m_fCurTime < m_fStartEvasiveTime) return;

	if (m_fCurTime > m_fStopEvasiveTime)
	{
		m_dwLastEvasiveAction = m_dwEvasiveAction;
		
		m_fStartEvasiveTime = m_fCurTime + GetRandom(s_fEvasiveDelay[m_eEvasive][0], 
													 s_fEvasiveDelay[m_eEvasive][1]);
		
		m_fStopEvasiveTime  = m_fStartEvasiveTime + GetRandom(0.25f, 0.5f);
		
		m_dwEvasiveAction = (m_dwLastEvasiveAction & AI_AFLG_STRAFE_LEFT) ? 
							 AI_AFLG_STRAFE_RIGHT : AI_AFLG_STRAFE_LEFT;
		return;
	}

	if (m_dwEvasiveAction & AI_AFLG_STRAFE_LEFT)
	{
		StrafeLeft();
	}
	else if (m_dwEvasiveAction & AI_AFLG_STRAFE_RIGHT)
	{
		StrafeRight();
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::GetStateString()
//
//	PURPOSE:	Return the string value of the current state
//
// ----------------------------------------------------------------------- //

char* BaseAI::GetStateString()
{
	char* pVal = "UNKNOWN";

	switch (m_eState)
	{
		case IDLE:			pVal = "IDLE";			break;
		case GUARDING:		pVal = "GUARDING";		break;
		case DEFENSIVE:		pVal = "DEFENSIVE";		break;
		case AGGRESSIVE:	pVal = "AGGRESSIVE";	break;
		case RETREATING:	pVal = "RETREATING";	break;
		case SCRIPT:		pVal = "SCRIPT";		break;
		case DYING:			pVal = "DYING";			break;
		default : break;
	}

	return pVal;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::GetConditionString()
//
//	PURPOSE:	Return the string value of the current condition
//
// ----------------------------------------------------------------------- //

char* BaseAI::GetConditionString()
{
	char* pVal = "UNKNOWN";

	switch (m_eCondition)
	{
		case HEALTHY:	pVal = "HEALTHY";	break;
		case COCKY:		pVal = "COCKY";		break;
		case WOUNDED:	pVal = "WOUNDED";	break;
		case DEAD:		pVal = "DEAD";		break;
		default : break;
	}

	return pVal;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::PrintDebugInfo()
//
//	PURPOSE:	Print information about this AI to the console
//
// ----------------------------------------------------------------------- //

void BaseAI::PrintDebugInfo()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HCONVAR	hVar  = pServerDE->GetGameConVar("DebugAI");
	char* pArg = pServerDE->GetVarValueString(hVar);
	char* pName   = pServerDE->GetObjectName(m_hObject);

	if (!pArg) return;


	if (_stricmp(pArg, "SIZE") == 0)
	{
		pServerDE->BPrint("== AI SIZE (%s) =============================", pName);
		pServerDE->BPrint("BaseAI: %d", sizeof(BaseAI));
		pServerDE->BPrint("CBaseCharacter: %d", sizeof(CBaseCharacter));
		pServerDE->BPrint("CProjectile: %d", sizeof(CProjectile));
		pServerDE->BPrint("CWeapon: %d", sizeof(CWeapon));
	}
	else if (_stricmp(pArg, "DIFFICULTY") == 0)
	{
		CWeapon* pWeapon = m_weapons.GetCurWeapon();
		if (!pWeapon) return;

		pServerDE->BPrint("== AI (%s) DIFFICULTY (%d) =============================", pName, g_pRiotServerShellDE->GetDifficulty());
		pServerDE->BPrint("Weapon (%d) Damage: %.2f", pWeapon->GetId(), pWeapon->GetDamage());
		pServerDE->BPrint("Marksmanship: %d", m_eMarksmanship);
		pServerDE->BPrint("Evasive: %d", m_eEvasive);
		pServerDE->BPrint("Armor: %.2f", m_damage.GetArmorPoints());
		pServerDE->BPrint("Hit Pts: %.2f", m_damage.GetHitPoints());
	}
	else if ((_stricmp(pArg, "ALL") == 0) || (_stricmp(pArg, pName) == 0))
	{
		DVector vDims;
		pServerDE->GetObjectDims(m_hObject, &vDims);

		char* pTargetName  = m_hTarget ? pServerDE->GetObjectName(m_hTarget) : "";
		char* pDamagerName = m_hLastDamager ? pServerDE->GetObjectName(m_hLastDamager) : "";
		char* pLeaderName  = m_hLeader ? pServerDE->GetObjectName(m_hLeader) : "";
		
		// Print out basic info...

		pServerDE->BPrint("== AI INFO (%s) =======================", pName);
		pServerDE->BPrint("Pos: %.2f, %.2f, %.2f", m_vPos.x, m_vPos.y, m_vPos.z);
		pServerDE->BPrint("Dims: %.2f, %.2f, %.2f", vDims.x, vDims.y, vDims.z);
		pServerDE->BPrint("--State Info---------------------------------");
		pServerDE->BPrint("State: %s", GetStateString());
		pServerDE->BPrint("Condition: %s", GetConditionString());
		pServerDE->BPrint("Bravado: %d", m_eBravado);
		pServerDE->BPrint("Experience: %d", m_eExperience);
		pServerDE->BPrint("Marksmanship: %d", m_eMarksmanship);
		pServerDE->BPrint("Evasive: %d", m_eEvasive);
		pServerDE->BPrint("Recompute Value: %d", m_nDebugRecomputeValue);
		pServerDE->BPrint("Last Dist Traveled: %f", m_fLastDistTraveled);
		pServerDE->BPrint("Stuck: %s", (m_bStuckOnSomething ? "TRUE" : "FALSE"));
		pServerDE->BPrint("--Additional Info----------------------------");
		pServerDE->BPrint("Current Target: %s (%x)", pTargetName, m_hTarget);
		pServerDE->BPrint("Last Damager: %s (%x)", pDamagerName, m_hLastDamager);
		pServerDE->BPrint("Leader: %s (%x)", pLeaderName, m_hLeader);
		pServerDE->BPrint("Last Touch Force: %.2f", m_fLastTouchForce);

		if (m_eState == SCRIPT && m_curScriptCmd.command == AI_SCMD_FOLLOWPATH)
		{
			pServerDE->BPrint("--AI Script Info-----------------------------");
		}		
		
		DVector vVel;
		pServerDE->GetVelocity(m_hObject, &vVel);
		DFLOAT fVelMag = VEC_MAG(vVel);
		pServerDE->BPrint("Velocity Magnitude: %f", fVelMag);

		pServerDE->BPrint("== END AI INFO (%s) ===================", pName);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //
	
void BaseAI::HandleModelString(ArgList* pArgList)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, KEY_FIRE_WEAPON) == 0)
	{
		CWeapon* pWeapon = m_weapons.GetCurWeapon();
		if (!pWeapon) return;

		DVector vBogus;
		VEC_INIT(vBogus);

		// Make sure we can see our target...

		if (GetTargetPos(vBogus))
		{
			pWeapon->Fire();
		}

		pServerDE->SetNextUpdate(m_hObject, AI_UPDATE_DELTA);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::ProcessCommand()
//
//	PURPOSE:	Process a command
//
// --------------------------------------------------------------------------- //

DBOOL BaseAI::ProcessCommand(char** pTokens, int nArgs, char* pNextCommand)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pTokens || nArgs < 1) return DFALSE;


	// Let base class have a whack at it...

	if (CBaseCharacter::ProcessCommand(pTokens, nArgs, pNextCommand)) return DTRUE;


	// See if it is a script...

	if (stricmp(TRIGGER_SCRIPT, pTokens[0]) == 0)
	{
		// Get script type from message...

		m_dwScriptFlags &= ~AI_SCRFLG_INT;  // Not interruptable

		if (nArgs > 1)
		{
			char* pType = pTokens[1];
			if (pType)
			{
				if (stricmp(TRIGGER_STYPE_INTERRUPTABLE, pType) == 0)
				{
					m_dwScriptFlags |= AI_SCRFLG_INT;
				}
				else  // Assume the flags were specified as a number
				{
					m_dwScriptFlags = atoi(pType);
				}
			}
		}

		BuildScript(pNextCommand);

		SetScript();
	}

	return DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::BuildScript()
//
//	PURPOSE:	Process the script list
//
// --------------------------------------------------------------------------- //

void BaseAI::BuildScript(char* pScriptBody)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	DBOOL bMore;

	m_scriptCmdList.RemoveAll();

	char* pCommand = pScriptBody;

	int nArgs;
	bMore = DTRUE;
	while( bMore )
	{
		bMore = pServerDE->Parse(pCommand, &g_pCommandPos, g_tokenSpace, g_pTokens, &nArgs);
		if (nArgs < 1) return;

		AISCRIPTCMD* pCmd = new AISCRIPTCMD;
		if (!pCmd) return;

		pCmd->command = StringToAIScriptCmdType(g_pTokens[0]);

		if (nArgs > 1)
		{
			char* pArgs = g_pTokens[1];
			if (pArgs) strncpy(pCmd->args, pArgs, MAX_AI_ARGS_LENGTH);
		}

		m_scriptCmdList.Add(pCmd);

		pCommand = g_pCommandPos;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::PlayAnimation()
//
//	PURPOSE:	Play a specific animation
//
// --------------------------------------------------------------------------- //

void BaseAI::PlayAnimation(char* pAniName)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pAniName) return;

	DDWORD dwAniIndex = pServerDE->GetAnimIndex(m_hObject, pAniName);

	pServerDE->SetModelLooping(m_hObject, DFALSE);
	pServerDE->SetModelAnimation(m_hObject, dwAniIndex);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetTarget()
//
//	PURPOSE:	Set target
//
// --------------------------------------------------------------------------- //

void BaseAI::SetTarget(char* pTargetName)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pTargetName) return;

	ObjectList*	pList = pServerDE->FindNamedObjects(pTargetName);
	if (!pList) return;

	ObjectLink* pLink = pList->m_pFirstLink;
	if (pLink)
	{
		SetNewTarget(pLink->m_hObject);
		TargetObject(m_hTarget);
	}

	pServerDE->RelinquishList(pList);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetNewTarget()
//
//	PURPOSE:	Set a new target
//
// --------------------------------------------------------------------------- //

void BaseAI::SetNewTarget(HOBJECT hNewTarget)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE ) return;

	if (m_hTarget != hNewTarget)
	{
		if (m_hTarget && (m_hLastDamager != m_hTarget) && (m_hLeader != m_hTarget))
		{
			pServerDE->BreakInterObjectLink(m_hObject, m_hTarget);
		}

		m_hTarget = hNewTarget;
		m_fNextTargetTime = 0.0f;	// Get this target's position now!

		if (m_hTarget && (m_hLastDamager != m_hTarget) && (m_hLeader != m_hTarget))
		{
			pServerDE->CreateInterObjectLink(m_hObject, m_hTarget);
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetNewDamager()
//
//	PURPOSE:	Set a new damager
//
// --------------------------------------------------------------------------- //

void BaseAI::SetNewDamager(HOBJECT hNewDamager)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hLastDamager != hNewDamager)
	{
		if (m_hLastDamager && (m_hLastDamager != m_hTarget) && (m_hLastDamager != m_hLeader))
		{
			pServerDE->BreakInterObjectLink(m_hObject, m_hLastDamager);
		}

		m_hLastDamager = hNewDamager;

		if (m_hLastDamager && (m_hLastDamager != m_hTarget) && (m_hLastDamager != m_hLeader))
		{
			pServerDE->CreateInterObjectLink(m_hObject, m_hLastDamager);
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SetNewLeader()
//
//	PURPOSE:	Set a new leader (scripted only)
//
// --------------------------------------------------------------------------- //

void BaseAI::SetNewLeader(HOBJECT hNewLeader)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE ) return;

	if (m_hLeader != hNewLeader)
	{
		if (m_hLeader && (m_hLeader != m_hTarget) && (m_hLeader != m_hLastDamager))
		{
			pServerDE->BreakInterObjectLink(m_hObject, m_hLeader);
		}

		m_hLeader = hNewLeader;

		if (m_hLeader && (m_hLeader != m_hTarget) && (m_hLeader != m_hLastDamager))
		{
			pServerDE->CreateInterObjectLink(m_hObject, m_hLeader);
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::SpotPlayer()
//
//	PURPOSE:	Handle spotting the player
//
// --------------------------------------------------------------------------- //

void BaseAI::SpotPlayer(HOBJECT hObj)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hObj || m_bSpottedPlayer) return;

	if (IsPlayer(hObj)) 
	{
		// If we have a spot trigger, trigger it...

		if (m_hstrSpotTriggerTarget && m_hstrSpotTriggerMessage)
		{
			LPBASECLASS pAI = pServerDE->HandleToObject(m_hObject);
			if (pAI && m_nSpotTriggerNumSends != 0)
			{
				SendTriggerMsgToObjects(pAI, m_hstrSpotTriggerTarget, m_hstrSpotTriggerMessage);
				m_nSpotTriggerNumSends--;
			}
		}
	
		if (m_dwAvailableSounds & AI_SNDFLG_SPOT)
		{
			char* pSound = DNULL;
			if (m_hstrSpotSound)
			{
				pSound = pServerDE->GetStringData(m_hstrSpotSound);
			}
			else
			{
				pSound = GetSpotSound(this);
			}

			PlayDialogSound(pSound, CST_EXCLAMATION);
		}

		m_bSpottedPlayer = DTRUE;
		m_bLostPlayer	 = DFALSE;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::HandleLostPlayer()
//
//	PURPOSE:	Handle losing sight of the player
//
// --------------------------------------------------------------------------- //

void BaseAI::HandleLostPlayer()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || m_bLostPlayer) return;

	// If we have a lost target trigger, trigger it...

	if (m_hstrLostTargetTriggerTarget && m_hstrLostTargetTriggerMessage)
	{
		LPBASECLASS pAI = pServerDE->HandleToObject(m_hObject);
		if (pAI && m_nLostTargetTriggerNumSends != 0)
		{
			SendTriggerMsgToObjects(pAI, m_hstrLostTargetTriggerTarget, m_hstrLostTargetTriggerMessage);
			m_nLostTargetTriggerNumSends--;
		}
	}

//#define PLAY_LOST_TARGET_SOUNDS
#ifdef PLAY_LOST_TARGET_SOUNDS
	if (m_dwAvailableSounds & AI_SNDFLG_LOSTTARGET)
	{
		char* pSound = DNULL;
		if (m_hstrLostTargetSound)
		{
			pSound = pServerDE->GetStringData(m_hstrLostTargetSound);
		}
		else
		{
			pSound = GetLostTargetSound(this);
		}

		PlayDialogSound(pSound);
	}
#endif

	m_bLostPlayer = DTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::HandleBumped()
//
//	PURPOSE:	Handle being bumped
//
// --------------------------------------------------------------------------- //

void BaseAI::HandleBumped()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	// If we have a bumped trigger, trigger it...

	if (m_hstrBumpedTriggerTarget && m_hstrBumpedTriggerMessage)
	{
		LPBASECLASS pAI = pServerDE->HandleToObject(m_hObject);
		if (pAI && m_nBumpedTriggerNumSends != 0)
		{
			SendTriggerMsgToObjects(pAI, m_hstrBumpedTriggerTarget, m_hstrBumpedTriggerMessage);
			m_nBumpedTriggerNumSends--;
		}
	}
	
	if (m_dwAvailableSounds & AI_SNDFLG_BUMPED)
	{
		char* pSound = DNULL;
		if (m_hstrBumpedSound)
		{
			pSound = pServerDE->GetStringData(m_hstrBumpedSound);
		}
		else
		{
			pSound = GetBumpedSound(this);
		}

		PlayDialogSound(pSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::GetTargetDir()
//
//	PURPOSE:	Get direction to our target
//
// ----------------------------------------------------------------------- //
	
DVector BaseAI::GetTargetDir(DVector & vFirePos, DVector & vTargetPos)
{
	// Adjust target pos based on current weapon...
	
	DVector vDir;
	VEC_SUB(vDir, vTargetPos, vFirePos);

	CWeapon* pWeapon = m_weapons.GetCurWeapon();
	if (!pWeapon) return vDir;

	DVector vNewTargetPos;
	VEC_COPY(vNewTargetPos, vTargetPos);

	// Adjust for gravity...

	if (pWeapon->GetId() == GUN_ENERGYGRENADE_ID)
	{
		DVector vDist;
		VEC_SUB(vDist, vTargetPos, vFirePos);
		DFLOAT fDist = VEC_MAG(vDist);
		if (fDist > 2000.0f) fDist = 2000.0f;

		DFLOAT fAdjust = 0.0f;

		if (fDist > 200.0f)
		{
			fAdjust = fDist / 10.0f;
		}

		vNewTargetPos.y += fAdjust;
	}

	VEC_SUB(vDir, vNewTargetPos, vFirePos);
	return vDir;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::UpdateTargetPos()
//
//	PURPOSE:	Update where we are aiming...
//
// ----------------------------------------------------------------------- //
	
void BaseAI::UpdateTargetPos()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hTarget) return;

	// See if it is time to get an accurate target position...

	if (m_fCurTime > m_fNextTargetTime)
	{
		m_fNextTargetTime = m_fCurTime + m_fTargetCalcDelta;
		pServerDE->GetObjectPos(m_hTarget, &m_vTargetPos);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::GetTargetPos()
//
//	PURPOSE:	Where should we aim?
//
// ----------------------------------------------------------------------- //
	
DBOOL BaseAI::GetTargetPos(DVector & vTargetPos)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !m_hTarget) return DFALSE;

	// Get the position of the target...

	VEC_COPY(vTargetPos, m_vTargetPos);

	if (IsObjectVisible(m_hTarget))
	{
		// Use the Target's pos...(save the "current" position)...

		VEC_COPY(m_vTargetLastPos, m_vTargetPos);
	}
	else if (m_vTargetLastPos.x != 0.0f && 
			 m_vTargetLastPos.y != 0.0f &&
			 m_vTargetLastPos.z != 0.0f)
	{
		// Make sure the target is close to it's last position (i.e., 
		// he is just hiding around a corner or something...)
		//
		if (VEC_DIST(vTargetPos, m_vTargetLastPos) < 150.0f)
		{
			VEC_COPY(vTargetPos, m_vTargetLastPos);
		}
		else
		{
			return DFALSE;  // He's out of range...
		}
	}
	else
	{
		return DFALSE;  // Don't have a valid target pos
	}

	return DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void BaseAI::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;


	// Save Objects...

	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hTarget);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hLastDamager);
	pServerDE->WriteToLoadSaveMessageObject(hWrite, m_hLeader);


	// Save bytes...

	pServerDE->WriteToMessageByte(hWrite, m_eState);
	pServerDE->WriteToMessageByte(hWrite, m_eCondition);
	pServerDE->WriteToMessageByte(hWrite, m_eLastTurnDir);
	pServerDE->WriteToMessageByte(hWrite, m_eBravado);
	pServerDE->WriteToMessageByte(hWrite, m_eExperience);
	pServerDE->WriteToMessageByte(hWrite, m_eMarksmanship);
	pServerDE->WriteToMessageByte(hWrite, m_eEvasive);
	pServerDE->WriteToMessageByte(hWrite, m_eScriptMovement);
	pServerDE->WriteToMessageByte(hWrite, m_eScriptState);
	pServerDE->WriteToMessageByte(hWrite, m_eOldState);
	pServerDE->WriteToMessageByte(hWrite, m_bStuckOnSomething);
	pServerDE->WriteToMessageByte(hWrite, m_bRecompute);
	pServerDE->WriteToMessageByte(hWrite, m_bSpottedPlayer);
	pServerDE->WriteToMessageByte(hWrite, m_bLostPlayer);
	pServerDE->WriteToMessageByte(hWrite, m_bOkAdjustVel);
	pServerDE->WriteToMessageByte(hWrite, m_bUpdateScriptCmd);
	pServerDE->WriteToMessageByte(hWrite, m_bScriptInterruptable);
	pServerDE->WriteToMessageByte(hWrite, m_bLostLeader);
	pServerDE->WriteToMessageByte(hWrite, m_bLoopScriptedAni);
	pServerDE->WriteToMessageByte(hWrite, m_nWeaponId);
	pServerDE->WriteToMessageByte(hWrite, m_nNumRecoilFireTrys);
	pServerDE->WriteToMessageByte(hWrite, m_nMaxCheeseCount);

	// Save dwords...

	pServerDE->WriteToMessageDWord(hWrite, m_dwAction);
	pServerDE->WriteToMessageDWord(hWrite, m_dwLastAction);
	pServerDE->WriteToMessageDWord(hWrite, m_dwAvailableStates);
	pServerDE->WriteToMessageDWord(hWrite, m_nSpotTriggerNumSends);
	pServerDE->WriteToMessageDWord(hWrite, m_nLostTargetTriggerNumSends);
	pServerDE->WriteToMessageDWord(hWrite, m_nBumpedTriggerNumSends);
	pServerDE->WriteToMessageDWord(hWrite, m_dwAvailableSounds);
	pServerDE->WriteToMessageDWord(hWrite, m_dwEvasiveAction);
	pServerDE->WriteToMessageDWord(hWrite, m_dwLastEvasiveAction);
	pServerDE->WriteToMessageDWord(hWrite, m_dwScriptFlags);


	// Save floats...

	pServerDE->WriteToMessageFloat(hWrite, m_fFireRestAdjust);
	pServerDE->WriteToMessageFloat(hWrite, m_fVisibleRange);
	pServerDE->WriteToMessageFloat(hWrite, m_fHearingRange);
	pServerDE->WriteToMessageFloat(hWrite, m_fRecomputeTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fNextBumpedTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fNextSoundTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fFollowStartTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fFollowTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fFireStartTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fFireStopTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fScriptWaitEnd);
	pServerDE->WriteToMessageFloat(hWrite, m_fPredTravelDist);
	pServerDE->WriteToMessageFloat(hWrite, m_fLastDistTraveled);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartEvasiveTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fStopEvasiveTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fNextTargetTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fTargetCalcDelta);
	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_nScriptCmdIndex);
	

	// Save vectors...

	pServerDE->WriteToMessageVector(hWrite, &m_vRight);
	pServerDE->WriteToMessageVector(hWrite, &m_vUp);
	pServerDE->WriteToMessageVector(hWrite, &m_vForward);
	pServerDE->WriteToMessageVector(hWrite, &m_vLastPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vMoveToPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vTargetPos);
	pServerDE->WriteToMessageVector(hWrite, &m_vTargetLastPos);


	// Save strings...

	pServerDE->WriteToMessageHString(hWrite, m_hstrSpotTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpotTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrLostTargetTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrLostTargetTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrBumpedTriggerTarget);
	pServerDE->WriteToMessageHString(hWrite, m_hstrBumpedTriggerMessage);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSetIdleSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSetDefensiveSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSetAggressiveSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSetRetreatingSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSetPanickedSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSetPsychoSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrSpotSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrLostTargetSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDeathSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrIdleSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrDefensiveSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrAggressiveSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrRetreatingSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrGuardingSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPanickedSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrPsychoSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrFollowLostSound);
	pServerDE->WriteToMessageHString(hWrite, m_hstrBumpedSound);


	// Save complicated shit...

	m_curScriptCmd.Save(hWrite);
	m_scriptCmdList.Save(hWrite);
	m_AIPathList.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void BaseAI::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_dwLastLoadFlags = dwLoadFlags;

	// Load objects...

	HOBJECT hObj = DNULL;
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &hObj);
	SetNewTarget(hObj);

	hObj = DNULL;
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &hObj);
	SetNewDamager(hObj);

	hObj = DNULL;
	pServerDE->ReadFromLoadSaveMessageObject(hRead, &hObj);
	SetNewLeader(hObj);

	
	// Load bytes...
	
	m_eState				= (AIState)	pServerDE->ReadFromMessageByte(hRead);
	m_eCondition			= (AICondition) pServerDE->ReadFromMessageByte(hRead);
	m_eLastTurnDir			= (AIDirection) pServerDE->ReadFromMessageByte(hRead);
	m_eBravado				= (AIBravado) pServerDE->ReadFromMessageByte(hRead);		
	m_eExperience			= (AIExperience) pServerDE->ReadFromMessageByte(hRead);
	m_eMarksmanship			= (AIMarksmanship) pServerDE->ReadFromMessageByte(hRead);	 
	m_eEvasive				= (AIEvasive) pServerDE->ReadFromMessageByte(hRead);
	m_eScriptMovement		= (AIScriptMovement) pServerDE->ReadFromMessageByte(hRead);
	m_eScriptState			= (AIScriptState) pServerDE->ReadFromMessageByte(hRead);
	m_eOldState				= (AIState)	pServerDE->ReadFromMessageByte(hRead);	
	m_bStuckOnSomething		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bRecompute			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bSpottedPlayer		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bLostPlayer			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bOkAdjustVel			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bUpdateScriptCmd		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bScriptInterruptable	= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bLostLeader			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bLoopScriptedAni		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_nWeaponId				= pServerDE->ReadFromMessageByte(hRead);
	m_nNumRecoilFireTrys	= pServerDE->ReadFromMessageByte(hRead);
	m_nMaxCheeseCount		= pServerDE->ReadFromMessageByte(hRead);



	// Load Dwords...
					
	m_dwAction						= pServerDE->ReadFromMessageDWord(hRead);
	m_dwLastAction					= pServerDE->ReadFromMessageDWord(hRead);
	m_dwAvailableStates				= pServerDE->ReadFromMessageDWord(hRead);
	m_nSpotTriggerNumSends			= pServerDE->ReadFromMessageDWord(hRead);
	m_nLostTargetTriggerNumSends	= pServerDE->ReadFromMessageDWord(hRead);
	m_nBumpedTriggerNumSends		= pServerDE->ReadFromMessageDWord(hRead);
	m_dwAvailableSounds				= pServerDE->ReadFromMessageDWord(hRead);
	m_dwEvasiveAction				= pServerDE->ReadFromMessageDWord(hRead);
	m_dwLastEvasiveAction			= pServerDE->ReadFromMessageDWord(hRead);
	m_dwScriptFlags					= pServerDE->ReadFromMessageDWord(hRead);


	// Load Floats...

	m_fFireRestAdjust	= pServerDE->ReadFromMessageFloat(hRead);
	m_fVisibleRange		= pServerDE->ReadFromMessageFloat(hRead);
	m_fHearingRange		= pServerDE->ReadFromMessageFloat(hRead);
	m_fRecomputeTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fNextBumpedTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fNextSoundTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fFollowStartTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fFollowTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_fFireStartTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fFireStopTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_fScriptWaitEnd	= pServerDE->ReadFromMessageFloat(hRead);
	m_fPredTravelDist	= pServerDE->ReadFromMessageFloat(hRead);
	m_fLastDistTraveled	= pServerDE->ReadFromMessageFloat(hRead);
	m_fStartEvasiveTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fStopEvasiveTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fNextTargetTime	= pServerDE->ReadFromMessageFloat(hRead);
	m_fTargetCalcDelta	= pServerDE->ReadFromMessageFloat(hRead);
	m_nScriptCmdIndex	= (int)	pServerDE->ReadFromMessageFloat(hRead);


	// Load vectors...

	pServerDE->ReadFromMessageVector(hRead, &m_vRight);
	pServerDE->ReadFromMessageVector(hRead, &m_vUp);
	pServerDE->ReadFromMessageVector(hRead, &m_vForward);
	pServerDE->ReadFromMessageVector(hRead, &m_vLastPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vMoveToPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vTargetPos);
	pServerDE->ReadFromMessageVector(hRead, &m_vTargetLastPos);


	// Load strings...

	m_hstrSpotTriggerTarget			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpotTriggerMessage		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrLostTargetTriggerTarget	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrLostTargetTriggerMessage	= pServerDE->ReadFromMessageHString(hRead);
	m_hstrBumpedTriggerTarget		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrBumpedTriggerMessage		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSetIdleSound				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSetDefensiveSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSetAggressiveSound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSetRetreatingSound		= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSetPanickedSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSetPsychoSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrSpotSound					= pServerDE->ReadFromMessageHString(hRead);
	m_hstrLostTargetSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDeathSound				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrIdleSound					= pServerDE->ReadFromMessageHString(hRead);
	m_hstrDefensiveSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrAggressiveSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrRetreatingSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrGuardingSound				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrPanickedSound				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrPsychoSound				= pServerDE->ReadFromMessageHString(hRead);
	m_hstrFollowLostSound			= pServerDE->ReadFromMessageHString(hRead);
	m_hstrBumpedSound				= pServerDE->ReadFromMessageHString(hRead);

		
	// Load complicated shit...

	m_curScriptCmd.Load(hRead);
	m_scriptCmdList.Load(hRead);
	m_AIPathList.Load(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::HandleGameRestore
//
//	PURPOSE:	Setup the object
//
// ----------------------------------------------------------------------- //

void BaseAI::HandleGameRestore()
{
	HandleWeaponChange();
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::AdjustMarksmanshipPerturb
//
//	PURPOSE:	Adjust our marksmanship skill based on the difficulty level
//
// ----------------------------------------------------------------------- //

void BaseAI::AdjustMarksmanshipPerturb()
{
	if (!g_pRiotServerShellDE) return;

	int nVal = m_eMarksmanship;

	switch (g_pRiotServerShellDE->GetDifficulty())
	{
		case GD_EASY:
			nVal += 3;
		break;

		case GD_NORMAL:
			nVal += 1;
		break;

		case GD_VERYHARD:
			nVal -= 3;
		break;

		case GD_HARD:
		default :
			return;
		break;
	}

	m_eMarksmanship = (AIMarksmanship)nVal;

	if (m_eMarksmanship < MARKSMANSHIP1) m_eMarksmanship = MARKSMANSHIP1;
	else if (m_eMarksmanship > MARKSMANSHIP6) m_eMarksmanship = MARKSMANSHIP6;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::AdjustEvasiveDelay
//
//	PURPOSE:	Adjust the evasiveness of the ai based on the difficulty
//				level
//
// ----------------------------------------------------------------------- //

void BaseAI::AdjustEvasiveDelay()
{
	if (!g_pRiotServerShellDE) return;

	int nVal = m_eEvasive;

	switch (g_pRiotServerShellDE->GetDifficulty())
	{
		case GD_EASY:
			nVal += 4;
		break;

		case GD_NORMAL:
			nVal += 2;
		break;

		case GD_VERYHARD:
			nVal -= 2;
		break;

		case GD_HARD:
		default :
			return;
		break;
	}

	m_eEvasive = (AIEvasive)nVal;

	if (m_eEvasive < EVASIVE1) m_eEvasive = EVASIVE1;
	else if (m_eEvasive > NON_EVASIVE) m_eEvasive = NON_EVASIVE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::AdjustFireDelay
//
//	PURPOSE:	Adjust the time between firing
//
// ----------------------------------------------------------------------- //

void BaseAI::AdjustFireDelay()
{
	if (!g_pRiotServerShellDE) return;

	switch (g_pRiotServerShellDE->GetDifficulty())
	{
		case GD_EASY:
			m_nMaxCheeseCount = (DBYTE)(AI_DEFAULT_MAX_CHEESE_COUNT * 2.0f);
			m_fFireRestAdjust = 2.0f;
		break;

		case GD_NORMAL:
			m_nMaxCheeseCount = (DBYTE)(AI_DEFAULT_MAX_CHEESE_COUNT * 1.5f);
			m_fFireRestAdjust = 1.5f;
		break;

		case GD_VERYHARD:
			m_nMaxCheeseCount = 1;
			m_fFireRestAdjust = 0.5f;
		break;

		case GD_HARD:
		default :
			m_nMaxCheeseCount = (DBYTE)AI_DEFAULT_MAX_CHEESE_COUNT;
			m_fFireRestAdjust = 1.0f;
		break;
	}

	if (m_nMaxCheeseCount < 0) m_nMaxCheeseCount = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::AdjustDamageAggregate
//
//	PURPOSE:	Adjust hit points / armor based on the difficulty level
//
// ----------------------------------------------------------------------- //

void BaseAI::AdjustDamageAggregate()
{
	if (!g_pRiotServerShellDE) return;

	DFLOAT fFactor = 1.0f;
	switch (g_pRiotServerShellDE->GetDifficulty())
	{
		case GD_EASY:
			fFactor = 0.25f;
		break;

		case GD_NORMAL:
			fFactor = 0.5f;
		break;

		case GD_VERYHARD:
			fFactor = 1.5f;
		break;

		case GD_HARD:
		default : 
			return;	
		break;
	}

	// Adjsut hit points / armor accordingly...

	m_damage.SetMaxHitPoints(m_damage.GetMaxHitPoints() * fFactor);
	m_damage.SetHitPoints(m_damage.GetHitPoints() * fFactor);
	m_damage.SetMaxArmorPoints(m_damage.GetMaxArmorPoints() * fFactor);
	m_damage.SetArmorPoints(m_damage.GetArmorPoints() * fFactor);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BaseAI::IsFiring
//
//	PURPOSE:	See if we are trying to fire.
//
// ----------------------------------------------------------------------- //

DBOOL BaseAI::IsFiring()
{
	DBOOL bRet = DFALSE;

	if ((m_dwAction & AI_AFLG_FIRE) && !m_damage.IsDead())
	{
		if (m_bRecoiling)
		{
			if (++m_nNumRecoilFireTrys > m_nMaxCheeseCount)
			{
				bRet = DTRUE;
				m_nNumRecoilFireTrys = 0;
			}
			else
			{
				bRet = DFALSE;
			}
		}
		else
		{
			bRet = DTRUE;
		}
	}

	return bRet;
}
