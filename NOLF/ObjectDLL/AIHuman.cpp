// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include <stdio.h>
#include "AIHuman.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "Destructible.h"
#include "Weapons.h"
#include "ObjectMsgs.h"
#include "VolumeBrushTypes.h"
#include "HHWeaponModel.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "GameServerShell.h"
#include "AIState.h"
#include "AIButeMgr.h"
#include "SurfaceFunctions.h"
#include "Attachments.h"
#include "AISense.h"
#include "AIPathMgr.h"
#include "CharacterHitBox.h"
#include "AITarget.h"
#include "TeleportPoint.h"

using namespace AnimationsHuman;

// Cvars

static CVarTrack g_CheapMovementTrack;

// Define our properties (what is available in DEdit)...
BEGIN_CLASS(CAIHuman)

	// New properties

	ADD_STRINGPROP_FLAG(Brain, "Default", PF_STATICLIST)

	ADD_HUMANATTACHMENTS_AGGREGATE()
	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP3)

		ADD_STRINGPROP_FLAG(CreepSpeed,		"", PF_GROUP3)
		ADD_STRINGPROP_FLAG(WalkSpeed,		"", PF_GROUP3)
		ADD_STRINGPROP_FLAG(SwimSpeed,		"", PF_GROUP3)
		ADD_STRINGPROP_FLAG(RunSpeed,		"", PF_GROUP3)
		ADD_STRINGPROP_FLAG(RollSpeed,		"", PF_GROUP3)

	// Reaction overrides

#ifndef NUKE_REACTIONS
	PROP_DEFINEGROUP(IndividualReactions, PF_GROUP2)

		ADD_STRINGPROP_FLAG(ISE1st,					c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISE,					"Attack",						PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISEFalse1st,			c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISEFalse,				"Investigate and search",		PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISEFlashlight1st,		c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISEFlashlight,			"Attack",						PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISEFlashlightFalse1st,	c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISEFlashlightFalse,		"Investigate and search",		PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISEFootprint1st,		c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISEFootprint,			"Investigate and search",		PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISADeath1st,			c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISADeath,				"Investigate and search",		PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEFootstep1st,			c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEFootstep,			"Look at",						PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEFootstepFalse1st,	c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEFootstepFalse,		"Call out",						PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEWeaponFire1st,		c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEWeaponFire,			"Attack",						PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEWeaponImpact1st,		c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEWeaponImpact,		"Attack from cover",			PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEDisturbance1st,		c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEDisturbance,			"Investigate and return",		PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHAPain1st,				c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHAPain,				"Attack",						PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHADeath1st,			c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHADeath,				"Investigate and return",		PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHAWeaponFire1st,		c_szNoReaction,					PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHAWeaponFire,			"Attack",						PF_GROUP2|PF_STATICLIST)

	PROP_DEFINEGROUP(GroupReactions, PF_GROUP5)

		ADD_STRINGPROP_FLAG(GSE1st,					c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSE,					"Attack",						PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSEFalse1st,			c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSEFalse,				c_szNoReaction,					PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSEFlashlight1st,		c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSEFlashlight,			"AttacK",						PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSEFlashlightFalse1st,	c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSEFlashlightFalse,		c_szNoReaction,					PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSEFootprint1st,		c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSEFootprint,			c_szNoReaction,					PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSADeath1st,			c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSADeath,				"Become alert",					PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEFootstep1st,			c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEFootstep,			c_szNoReaction,					PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEFootstepFalse1st,	c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEFootstepFalse,		c_szNoReaction,					PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEWeaponFire1st,		c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEWeaponFire,			"Attack",						PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHAWeaponFire1st,		c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHAWeaponFire,			"Attack",						PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEWeaponImpact1st,		c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEWeaponImpact,		"Attack from cover",			PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEDisturbance1st,		c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEDisturbance,			c_szNoReaction,					PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHAPain1st,				c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHAPain,				"Attack",						PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHADeath1st,			c_szNoReaction,					PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHADeath,				"Attack",						PF_GROUP5|PF_STATICLIST)
#endif

	PROP_DEFINEGROUP(Commands, PF_GROUP4)

		ADD_STRINGPROP_FLAG(Lighter,		"", PF_GROUP4)

END_CLASS_DEFAULT_FLAGS_PLUGIN(CAIHuman, CAI, NULL, NULL, CF_HIDDEN, CAIHumanPlugin)

// Statemap

struct STATEMAP
{
	const char*	szState;
	CAIHumanState::AIHumanStateType	eState;
};

static STATEMAP s_aStateMaps[] =
{
	"IDLE",					CAIHumanState::eStateIdle,
	"AWARE",				CAIHumanState::eStateAware,
	"LOOKAT",				CAIHumanState::eStateLookAt,
	"TAIL",					CAIHumanState::eStateTail,
	"FOLLOWFOOTPRINT",		CAIHumanState::eStateFollowFootprint,
	"INVESTIGATE",			CAIHumanState::eStateInvestigate,
	"CHECKBODY",			CAIHumanState::eStateCheckBody,
	"SEARCH",				CAIHumanState::eStateSearch,
	"CHARGE",				CAIHumanState::eStateCharge,
	"CHASE",				CAIHumanState::eStateChase,
	"PANIC",				CAIHumanState::eStatePanic,
	"DISTRESS",				CAIHumanState::eStateDistress,
	"DROWSY",				CAIHumanState::eStateDrowsy,
	"UNCONSCIOUS",			CAIHumanState::eStateUnconscious,
	"STUNNED",				CAIHumanState::eStateStunned,
	"ATTACK",				CAIHumanState::eStateAttack,
	"DRAW",					CAIHumanState::eStateDraw,
	"ATTACKFROMCOVER",		CAIHumanState::eStateAttackFromCover,
	"ATTACKFROMVANTAGE",	CAIHumanState::eStateAttackFromVantage,
	"ATTACKFROMVIEW",		CAIHumanState::eStateAttackFromView,
	"ATTACKONSIGHT",		CAIHumanState::eStateAttackOnSight,
	"ASSASSINATE",			CAIHumanState::eStateAssassinate,
	"COVER",				CAIHumanState::eStateCover,
	"PATROL",				CAIHumanState::eStatePatrol,
	"GOTO",					CAIHumanState::eStateGoto,
	"FLEE",					CAIHumanState::eStateFlee,
//	"COME",					CAIHumanState::eStateCome,
	"FOLLOW",				CAIHumanState::eStateFollow,
	"GETBACKUP",			CAIHumanState::eStateGetBackup,
	"USEOBJECT",			CAIHumanState::eStateUseObject,
	"PICKUPOBJECT",			CAIHumanState::eStatePickupObject,
	"TALK",					CAIHumanState::eStateTalk,
	"ANIMATE",				CAIHumanState::eStateAnimate,
	"PARADIVE",				CAIHumanState::eStateParaDive,
	"PARASHOOT",			CAIHumanState::eStateParaShoot,
	"PARADIE",				CAIHumanState::eStateParaDie,
	"PARAESCAPE",			CAIHumanState::eStateParaEscape,
	"ATTACKPROP",			CAIHumanState::eStateAttackProp,
	"HELIATTACK",			CAIHumanState::eStateHeliAttack,
	"SCOT-BOX",				CAIHumanState::eStateScotBox,
	"INGE-SING",			CAIHumanState::eStateIngeSing,
};

static int s_cStateMaps = sizeof(s_aStateMaps)/sizeof(STATEMAP);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CAIHuman()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

#pragma warning (push)
#pragma warning (disable: 4355)
CAIHuman::CAIHuman() : CAI(), m_Nudge(this)
#pragma warning (pop)
{
	m_bForceGround = LTFALSE;

	if (!g_CheapMovementTrack.IsInitted())
	{
        g_CheapMovementTrack.Init(g_pLTServer, "AICheapMovement", LTNULL, 0.0f);
	}

//	if ( g_CheapMovementTrack.GetFloat(0.0f) == 1.0f )
//	{
		m_bCheapMovement = LTTRUE;
//	}
//	else
//	{
//		m_bCheapMovement = LTFALSE;
//	}

	m_fCreepVel		= 0.0f;
	m_fWalkVel		= 0.0f;
	m_fSwimVel		= 0.0f;
	m_fRunVel		= 0.0f;

	m_bBlink			= LTTRUE;
	m_bShortRecoil		= LTTRUE;

	m_bCanShortRecoil	= LTTRUE;

	m_pHumanState = LTNULL;

	VEC_INIT(m_vMovePos);
	m_bMove = LTFALSE;
	m_fSpeed = 250.0f;

	m_hstrCmdLighter = LTNULL;

	m_pAnimationContext = LTNULL;

	m_pBrain = LTNULL;
	m_hstrBrain = LTNULL;

	m_bSyncPosition = LTFALSE;

	m_hstrHolster = LTNULL;

	m_bPosDirty = LTTRUE;

	m_hHeadNode = INVALID_MODEL_NODE;

	SetState(CAIHumanState::eStateIdle);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::~CAIHuman()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAIHuman::~CAIHuman()
{
	FREE_HSTRING(m_hstrCmdLighter);
	FREE_HSTRING(m_hstrBrain);
	FREE_HSTRING(m_hstrHolster);

	if ( m_pHumanState )
	{
		FACTORY_DELETE(m_pHumanState);
		m_pHumanState = LTNULL;
		m_pState = LTNULL;
	}

	if ( m_pAnimationContext )
	{
		g_pAnimationMgrHuman->DestroyAnimationContext(m_pAnimationContext);
		m_pAnimationContext = LTNULL;
	}

	if ( m_pBrain )
	{
		FACTORY_DELETE(m_pBrain);
		m_pBrain = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
    if (!g_pLTServer || !pData) return LTFALSE;

	// If we have an attribute template, fill in the info

	if ( m_hstrAttributeTemplate )
	{
        char *szAttributeTemplate = g_pLTServer->GetStringData(m_hstrAttributeTemplate);
		int nTemplateID = g_pAIButeMgr->GetTemplateIDByName(szAttributeTemplate);

		if ( nTemplateID < 0 )
		{
            g_pLTServer->CPrint("Bad AI Attribute Template referenced! : %s", szAttributeTemplate);
		}
		else
		{
			m_fCreepVel		 = g_pAIButeMgr->GetTemplate(nTemplateID)->fCreepSpeed;
			m_fWalkVel		 = g_pAIButeMgr->GetTemplate(nTemplateID)->fWalkSpeed;
			m_fSwimVel		 = g_pAIButeMgr->GetTemplate(nTemplateID)->fSwimSpeed;
			m_fRunVel		 = g_pAIButeMgr->GetTemplate(nTemplateID)->fRunSpeed;
			m_fRollVel		 = g_pAIButeMgr->GetTemplate(nTemplateID)->fRollSpeed;
		}
	}
	else
	{
        g_pLTServer->CPrint("No attribute template specified for AI!");
	}

    if ( g_pLTServer->GetPropGeneric("CreepSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fCreepVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("WalkSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fWalkVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("SwimSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fSwimVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("RunSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fRunVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("RollSpeed", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fRollVel = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric( "Lighter", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCmdLighter = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "Brain", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrBrain = g_pLTServer->CreateString( genProp.m_String );

	return CAI::ReadProp(pData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::PreCreateSpecialFX()
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //

void CAIHuman::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CAI::PreCreateSpecialFX(cs);

	cs.nTrackers = 2;
	cs.nDimsTracker = 0;
	cs.SetSleeping(GetAnimationContext() ? GetAnimationContext()->IsPropSet(aniAsleep) : LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetDeathAni()
//
//	PURPOSE:	Get the death animation
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIHuman::GetDeathAni(LTBOOL bFront)
{
	HMODELANIM hAni = INVALID_MODEL_ANIM;

	if ( m_pState )
	{
		if ( INVALID_MODEL_ANIM != (hAni = m_pState->GetDeathAni(bFront)) )
		{
			return hAni;
		}
	}

	if ( GetAnimationContext()->IsPropSet(aniRun) )
	{
		static const char* aszDeathRuns[] = { "DRun", "DRun2" };
        static const int cDeathRuns = sizeof(aszDeathRuns)/sizeof(const char*);

        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)aszDeathRuns[GetRandom(0, cDeathRuns-1)])) )
		{
			return hAni;
		}
	}

	if ( GetAnimationContext()->IsPropSet(aniCrouch) )
	{
        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)GetCrouchDeathAni())) )
		{
			return hAni;
		}
	}

	if ( GetAnimationContext()->IsPropSet(aniProne) )
	{
        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)GetProneDeathAni())) )
		{
			return hAni;
		}
	}

	return CAI::GetDeathAni(bFront);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetCrouchDeathAni()
//
//	PURPOSE:	Get a crouching death animation
//
// ----------------------------------------------------------------------- //

const char* CAIHuman::GetCrouchDeathAni()
{
	static const char* aszDeathCrouches[] = { "DCrouch2", "DCrouch3", "DCrouch4", "DCrouch5" };
    static const int cDeathCrouches = sizeof(aszDeathCrouches)/sizeof(const char*);
	return aszDeathCrouches[GetRandom(0, cDeathCrouches-1)];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetProneDeathAni()
//
//	PURPOSE:	Get a Proneing death animation
//
// ----------------------------------------------------------------------- //

const char* CAIHuman::GetProneDeathAni()
{
	static const char* aszDeathPronees[] = { "DProne", };
    static const int cDeathPronees = sizeof(aszDeathPronees)/sizeof(const char*);
	return aszDeathPronees[GetRandom(0, cDeathPronees-1)];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetBodyState()
//
//	PURPOSE:	Gets the state of our body
//
// ----------------------------------------------------------------------- //

BodyState CAIHuman::GetBodyState()
{
	BodyState eBodyState = CAI::GetBodyState();

	if ( GetAnimationContext()->IsPropSet(aniUnconscious) ||
 		 (m_pHumanState && m_pHumanState->GetType() == CAIHumanState::eStateUnconscious) )
	{
		return eBodyStateNormal;
	}
	else if ( GetAnimationContext()->IsPropSet(aniSit) )
	{
		return GetPriorityBodyState(eBodyState, eBodyStateChair);
	}
	else if ( m_pState )
	{
		return GetPriorityBodyState(eBodyState, m_pState->GetBodyState());
	}
	else
	{
		return eBodyState;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::PlayDeathSound()
//
//	PURPOSE:	Play the death sound
//
// ----------------------------------------------------------------------- //

void CAIHuman::PlayDeathSound()
{
	char* szDeathSound = LTNULL;

	// If we're unconscious make no noise

	if ( GetAnimationContext()->IsPropSet(aniUnconscious) ||
 		 (m_pHumanState && m_pHumanState->GetType() == CAIHumanState::eStateUnconscious) )
	{
		return;
	}

	// Check for "silent kill" -- hit in head and not alert

	if ( WasSilentKill() ||
		 GetAnimationContext()->IsPropSet(aniAsleep) )
	{
		szDeathSound = GetDeathSilentSound();
	}
	else
	{
		szDeathSound = GetDeathSound();
	}

	CCharacter::PlaySound(szDeathSound, m_fSoundRadius, LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::WasSilentKill()
//
//	PURPOSE:	Determines if we have been killed silently or not
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::WasSilentKill()
{
	if ( m_pSenseMgr->IsAlert() )
	{
		return LTFALSE;
	}
	else if ( eModelNodeInvalid == m_eModelNodeLastHit )
	{
		return LTFALSE;
	}
	else
	{
		return NODEFLAG_CRITICALHIT & g_pModelButeMgr->GetSkeletonNodeFlags(m_eModelSkeleton, m_eModelNodeLastHit) ? LTTRUE : LTFALSE;
	}
}

// ----------------------------------------------------------------------- //

void CAIHuman::HandleTeleport(const LTVector& vPos)
{
	CAI::HandleTeleport(vPos);
	m_bPosDirty = LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHuman::HandleTeleport(TeleportPoint* pTeleportPoint)
{
	CAI::HandleTeleport(pTeleportPoint);

	if ( pTeleportPoint->IsMoveToFloor() )
	{
		m_bPosDirty = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::HandleGadget()
//
//	PURPOSE:	Handle having a gadget used on us
//
// ----------------------------------------------------------------------- //

void CAIHuman::HandleGadget(uint8 nAmmoID)
{
	CCharacter::HandleGadget(nAmmoID);

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(nAmmoID);
	if (!pAmmo) return;

	if ( pAmmo->eInstDamageType == DT_GADGET_LIGHTER )
	{
		if ( m_hstrCmdLighter )
		{
			SendMixedTriggerMsgToObject(this, m_hObject, m_hstrCmdLighter);
			FREE_HSTRING(m_hstrCmdLighter);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::HandleCommand()
//
//	PURPOSE:	Handle a command
//
// --------------------------------------------------------------------------- //

LTBOOL CAIHuman::HandleCommand(char **pTokens, int nArgs)
{
	// Let base class have a whack at it...

	if (CAI::HandleCommand(pTokens, nArgs)) return LTTRUE;

	// State-changing message

	for ( int iState = 0 ; iState < s_cStateMaps ; iState++ )
	{
		// In case this was an optional command, skip past the ?
		uint32 bOptional = LTFALSE;
		if ( '?' == *pTokens[0] )
		{
			bOptional = LTTRUE;
		}

		if ( !_stricmp(&pTokens[0][bOptional], s_aStateMaps[iState].szState) )
		{
			if ( bOptional && !m_pHumanState->CanChangeToState(s_aStateMaps[iState].eState) )
			{
				return LTTRUE;
			}

			// Found a matching state, let's see if we can change.

			if ( m_pState && m_pState->DelayChangeState() )
			{
				// We can't change states right now...

				char szBuffer[4096];
				szBuffer[0] = 0;

				// Re-concatenate the tokens into a single message...

				for ( int iArg = 0 ; iArg < nArgs ; iArg++ )
				{
					strcat(szBuffer, pTokens[iArg]);
					strcat(szBuffer, " ");
				}

				// And stuff it into the NEXT= parameter for the current state

				m_pState->HandleNameValuePair("NEXT", szBuffer);
			}
			else if ( m_pState && m_pState->RejectChangeState() )
			{
			}
			else
			{
				// We can change states, go ahead and do so...

				SetState(s_aStateMaps[iState].eState);
				HandleCommandParameters(pTokens, nArgs);
			}

			return LTTRUE;
		}
	}

	// Non-state-changing messages

	if ( !_stricmp(pTokens[0], "TRACKPLAYER") )
	{
		LTBOOL bOn = LTTRUE;
		LTFLOAT fTime = 0.0f;
        HSTRING hstr = g_pLTServer->CreateString("Head_node");

		if(nArgs > 1)
		{
			if ( !_stricmp(pTokens[1], "ON") )
				bOn = LTTRUE;
			else if ( !_stricmp(pTokens[1], "OFF") )
				bOn = LTFALSE;
			else
				fTime = (LTFLOAT)atof(pTokens[1]);
		}

        HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
        g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
        g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
        g_pLTServer->WriteToMessageByte(hMessage, CFX_NODECONTROL_HEAD_FOLLOW_OBJ);
        g_pLTServer->WriteToMessageObject(hMessage, g_pCharacterMgr->FindPlayer()->m_hObject);
        g_pLTServer->WriteToMessageHString(hMessage, hstr);
        g_pLTServer->WriteToMessageFloat(hMessage, 130.0f);
        g_pLTServer->WriteToMessageFloat(hMessage, fTime);
        g_pLTServer->WriteToMessageByte(hMessage, bOn);
        g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

        g_pLTServer->FreeString(hstr);

		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "FORCEGROUND") )
	{
		m_bForceGround = IsTrueChar(*pTokens[1]);
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "CIGARETTE") )
	{
		if ( IsTrueChar(*pTokens[1]) )
		{
			DestroyCigarette();
			CreateCigarette(LTFALSE);
		}
		else
		{
			DestroyCigarette();
		}
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "SMOKE") )
	{
		if ( IsTrueChar(*pTokens[1]) )
		{
			DestroyCigarette();
			CreateCigarette(LTTRUE);
		}
		else
		{
			DestroyCigarette();
			CreateCigarette(LTFALSE);
		}
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "HOLSTER") )
	{
		FREE_HSTRING(m_hstrHolster);
		m_hstrHolster = g_pLTServer->CreateString(pTokens[1]);
	}
	else if ( !_stricmp(pTokens[0], "CINEFIRE") )
	{
		CWeapon* pWeapon = GetWeapon(0);
		if ( pWeapon )
		{
			HMODELSOCKET hRightHand;
			if ( LT_OK == g_pModelLT->GetSocket(m_hObject, "RightHand", hRightHand) )
			{
				LTransform tf;
				LTRotation rRot;
				LTVector vNull, vForward;

				if ( LT_OK == g_pModelLT->GetSocketTransform(m_hObject, hRightHand, tf, LTTRUE) )
				{
					g_pTransLT->GetRot(tf, rRot);
					g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

					// Now fire the weapon

					WFireInfo fireInfo;
					fireInfo.hFiredFrom = m_hObject;
					fireInfo.vPath		= vForward;
					fireInfo.vFirePos	= m_vPos;
					fireInfo.vFlashPos	= m_vPos;
					fireInfo.fPerturbR	= 0.0f;
					fireInfo.fPerturbU	= 0.0f;

					pWeapon->ReloadClip(LTFALSE);
					pWeapon->GetParent()->AddAmmo(pWeapon->GetAmmoId(), 999999);
					pWeapon->UpdateWeapon(fireInfo, LTTRUE);
				}
			}
		}

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetKneePosition()
//
//	PURPOSE:	Compute our 'knee level'
//
// ----------------------------------------------------------------------- //

LTVector CAIHuman::GetKneePosition() const
{
    LTVector vPos, vDims;
	VEC_COPY(vPos, m_vPos);

    if (!g_pLTServer || !m_hObject) return vPos;

    g_pLTServer->GetObjectDims(m_hObject, &vDims);

	vPos.y -= vDims.y / 2.0f;
	vPos.y += 2.0f;

	return vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::IsObjectVisible*()
//
//	PURPOSE:	Is the test object visible
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::IsObjectVisibleFromKnee(ObjectFilterFn ofn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV /* = LTTRUE */)
{
	return IsObjectVisible(ofn, NULL, GetKneePosition(), hObj, fVisibleDistanceSqr, bFOV);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::IsPositionVisible*()
//
//	PURPOSE:	Is the test position visible to us (regardless of direction)
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::IsPositionVisibleFromKnee(ObjectFilterFn ofn, const LTVector& vSourcePosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV /* = LTTRUE */)
{
	return IsPositionVisible(ofn, NULL, GetKneePosition(), vSourcePosition, fVisibleDistanceSqr, bFOV);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::Move()
//
//	PURPOSE:	Sets our new position for this frame
//
// ----------------------------------------------------------------------- //

void CAIHuman::Move(const LTVector& vPos)
{
	m_vMovePos = vPos;
	m_bMove = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::OpenDoor()
//
//	PURPOSE:	Attempts to open a door
//
// ----------------------------------------------------------------------- //

void CAIHuman::OpenDoor(HOBJECT hDoor)
{
	SendTriggerMsgToObject(this, hDoor, LTFALSE, "ACTIVATE");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CloseDoor()
//
//	PURPOSE:	Attempts to close a door
//
// ----------------------------------------------------------------------- //

void CAIHuman::CloseDoor(HOBJECT hDoor)
{
	SendTriggerMsgToObject(this, hDoor, LTFALSE, "ACTIVATE");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::HandleDamage()
//
//	PURPOSE:	Notification that we are hit by something
//
// ----------------------------------------------------------------------- //

void CAIHuman::HandleDamage(const DamageStruct& damage)
{
	if ( DT_MELEE == damage.eType && m_damage.IsCantDamageType(damage.eType) )
	{
		if ( !IsAlert() && (damage.fDamage > 100.0f) )
		{
			m_fLastPainVolume = 0.1f;
			m_damage.SetHitPoints(0.0f);
			m_damage.HandleDestruction(damage.hDamager);
			return;
		}
		else
		{
			return;
		}
	}

	if ( m_damage.IsCantDamageType(damage.eType) || !m_damage.GetCanDamage() ) return;

	if ( (damage.fDamage > 0.0f) &&
		 ((GetAnimationContext() && GetAnimationContext()->IsPropSet(aniUnconscious) ||
		  (m_pHumanState && m_pHumanState->GetType() == CAIHumanState::eStateUnconscious))) )
	{
		m_damage.SetHitPoints(0.0f);
		m_damage.HandleDestruction(damage.hDamager);
		return;
	}
	else if ( DT_BURN == damage.eType )
	{
		static LTBOOL bParity = LTTRUE;
		LTFLOAT fDelay = bParity ? GetRandom(1.0f, 1.1f) : GetRandom(1.2f, 1.3f);
		bParity ^= 1;

		m_damage.SetHitPoints(m_damage.GetHitPoints()/fDelay);
//		m_damage.HandleDestruction(damage.hDamager);
	}
	else if ( DT_SLEEPING == damage.eType )
	{
		static LTBOOL bParity = LTTRUE;
		LTFLOAT fDelay = bParity ? GetRandom(0.0f, 0.25f) : GetRandom(0.75f, 1.0f);
		bParity ^= 1;

		ChangeState("Delay %f (msg %s (TARGET (%s)));Delay %f (msg %s UNCONSCIOUS)", 
			fDelay, GetName(), damage.hDamager ? g_pLTServer->GetObjectName(damage.hDamager) : "noone",
			fDelay, GetName());
		return;
	}
	else if ( DT_STUN == damage.eType )
	{
		static LTBOOL bParity = LTTRUE;
		LTFLOAT fDelay = bParity ? GetRandom(0.0f, 0.25f) : GetRandom(0.75f, 1.0f);
		bParity ^= 1;

		ChangeState("Delay %f (msg %s (TARGET (%s)));Delay %f (msg %s STUNNED)", 
			fDelay, GetName(), damage.hDamager ? g_pLTServer->GetObjectName(damage.hDamager) : "noone",
			fDelay, GetName());
		return;
	}

	CAI::HandleDamage(damage);

	if ( !m_damage.IsDead() )
	{
		if ( m_bCanShortRecoil )
		{
			HandleShortRecoil();
		}
	}
	else
	{
        if ( WasSilentKill() )
		{
			m_fLastPainVolume = 0.1f;
		}

		// Move to wherever our torso is

		SyncPosition();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::PreUpdate
//
//	PURPOSE:	Does our Preupdate
//
// ----------------------------------------------------------------------- //

void CAIHuman::PreUpdate()
{
	CAI::PreUpdate();

	if ( m_bFirstUpdate )
	{
		if ( !m_bMoveToFloor )
		{
			m_bPosDirty = LTFALSE;
		}

		if ( !m_pAnimationContext )
		{
			m_pAnimationContext = g_pAnimationMgrHuman->CreateAnimationContext(m_hObject);
		}

		if ( !m_pBrain )
		{
			SetBrain(m_hstrBrain);
		}
	}

	if ( m_bSyncPosition )
	{
		LTVector vNewPosition(m_vTorsoPos.x, m_vPos.y, m_vTorsoPos.z);
		g_pLTServer->TeleportObject(m_hObject, &vNewPosition);

		m_bPosDirty = LTTRUE;
		m_bSyncPosition = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UpdateNodes
//
//  PURPOSE:	Update the position of important nodes (eye, torso, etc)
//
// ----------------------------------------------------------------------- //

void CAIHuman::UpdateNodes()
{
	CAI::UpdateNodes();

	if ( INVALID_MODEL_NODE == m_hHeadNode )
	{
		g_pModelLT->GetNode(m_hObject, "Head_node", m_hHeadNode);
	}

	LTransform transform;
	LTRotation rEyeRot;
	LTVector vNull;

	g_pModelLT->GetNodeTransform(m_hObject, m_hHeadNode, transform, LTTRUE);

	g_pTransLT->GetPos(transform, m_vEyePos);
	g_pTransLT->GetRot(transform, rEyeRot);

	g_pMathLT->GetRotationVectors(rEyeRot, vNull, m_vEyeForward, vNull);

	m_vTorsoPos = m_vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UpdateTarget
//
//	PURPOSE:	Performs the movement of our object
//
// ----------------------------------------------------------------------- //

void CAIHuman::UpdateTarget()
{
	CAI::UpdateTarget();

	if ( HasTarget() )
	{
		AimAt(GetTarget()->GetObject());
	}
}

void CAIHuman::UpdateMovement()
{
	m_Nudge.Update(m_bMove);
	LTVector vNudge = m_Nudge.GetNudge();

	if ( !m_bMove || m_bForceGround || m_bPosDirty)
	{
		if ( !m_Nudge.IsNudging() && !m_bForceGround && !m_bPosDirty )
		{
			return;
		}
		else if ( !m_bMove )
		{
			m_vMovePos = m_vPos;
		}
	}

	if ( m_Nudge.IsNudging() )
	{
		// TODO: we might be able to get the direction out of nudge a bit differently
		// in order to avoid the renormalization... clean up after E3

		// Make sure we stay in this volume or a neighboring volume

		if ( HasLastVolume() && GetLastVolume()->Inside2dLoose(m_vMovePos+vNudge, GetRadius()) )
		{
			// We're okay...
		}
		else
		{
			vNudge = LTVector(0,0,0);
		}
	}
	else
	{
		vNudge = LTVector(0,0,0);
	}

	if ( m_bCheapMovement )
	{
		if ( !IsScuba() )
		{
			// Find the floor underneath us

			IntersectQuery IQuery;
			IntersectInfo IInfo;

			IQuery.m_From = LTVector(m_vMovePos.x, m_vMovePos.y + m_vDims.y, m_vMovePos.z);
			IQuery.m_To = LTVector(m_vMovePos.x, m_vMovePos.y - m_vDims.y*3.0f, m_vMovePos.z);
			IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			IQuery.m_FilterFn = GroundFilterFn;

			g_cIntersectSegmentCalls++;
            if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && (IsMainWorld(IInfo.m_hObject) || (OT_WORLDMODEL == g_pLTServer->GetObjectType(IInfo.m_hObject))))
			{
				m_vMovePos.y = IInfo.m_Point.y + m_vDims.y;
			}
		}

		// Move us

        g_pLTServer->TeleportObject(m_hObject, &LTVector(m_vMovePos+vNudge));
	}
	else
	{
        g_pLTServer->MoveObject(m_hObject, &LTVector(m_vMovePos+vNudge));
	}

	// Clear out movement info

	m_bPosDirty = LTFALSE;
	m_bMove = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UpdateCharacterFx
//
//	PURPOSE:	Handles any pending Animator changes
//
// ----------------------------------------------------------------------- //

void CAIHuman::UpdateCharacterFx()
{
	if ( m_cs.IsSleeping() && !GetAnimationContext()->IsPropSet(aniAsleep) )
	{
		DestroyZzz();
//		SetSleeping(LTFALSE);
//		CreateSpecialFX();
	}
	else if ( !m_cs.IsSleeping() && GetAnimationContext()->IsPropSet(aniAsleep) )
	{
		CreateZzz();
//		SetSleeping(LTTRUE);
//		CreateSpecialFX();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::UpdateAnimator
//
//	PURPOSE:	Handles any pending Animator changes
//
// ----------------------------------------------------------------------- //

void CAIHuman::UpdateAnimation()
{
	CAI::UpdateAnimation();
	if ( !m_damage.IsDead() )
	{
		GetAnimationContext()->SetProp(aniWeapon3);

		CWeapon* pWeapon = GetCurrentWeapon();
		if ( pWeapon )
		{
			WEAPON* pWeaponData = g_pWeaponMgr->GetWeapon(GetCurrentWeapon()->GetId());
			if (pWeaponData)
			{
				switch ( pWeaponData->nAniType )
				{
					case 0:
						GetAnimationContext()->SetProp(aniWeapon1);
						break;
					case 1:
					case 2:
					case 3:
					case 4:
						GetAnimationContext()->SetProp(aniWeapon2);
						break;
					case 5:
						GetAnimationContext()->SetProp(aniWeapon3);
						break;
				}
			}
		}
	}

	if ( IsScuba() )
	{
		GetAnimationContext()->SetProp(aniSwim);
	}
/*
	if ( !stricmp(GetName(), "Scotsman") )
	{
		char szBuffer[256];
		szBuffer[0] = 0;
		GetAnimationContext()->GetPropsString(szBuffer, 256);
		g_pLTServer->CPrint("%s props = %s", GetName(), szBuffer);
	}
*/
	GetAnimationContext()->Update();

	// Update the hitbox offset

	LTVector vOffset = m_vTorsoPos - m_vPos;
	vOffset.y = 0.0f;

	CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(m_hHitBox);
	pHitBox->SetOffset(vOffset);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAIHuman::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    if (!g_pLTServer || !hWrite) return;

	CAI::Save(hWrite, dwSaveFlags);

	SAVE_HSTRING(m_hstrBrain);
	if ( m_pBrain )
	{
		SAVE_BOOL(LTTRUE);
		m_pBrain->Save(hWrite);
	}
	else
	{
		SAVE_BOOL(LTFALSE);
	}

	SAVE_BOOL((LTBOOL)(!!m_pAnimationContext));

	if ( m_pAnimationContext )
	{
		m_pAnimationContext->Save(hWrite);
	}

	m_Nudge.Save(hWrite);

	// Save state...

	uint32 dwState = (uint32)-1;
	if (m_pHumanState)
	{
		dwState = (uint32) m_pHumanState->GetType();
	}

	SAVE_DWORD(dwState);

	if (m_pHumanState)
	{
		m_pHumanState->Save(hWrite);
	}

	SAVE_BOOL(m_bSyncPosition);
	SAVE_VECTOR(m_vMovePos);
	SAVE_BOOL(m_bMove);
	SAVE_BOOL(m_bPosDirty);
	SAVE_FLOAT(m_fSpeed);
	SAVE_FLOAT(m_fCreepVel);
	SAVE_BOOL(m_bCanShortRecoil);
	SAVE_HSTRING(m_hstrCmdLighter);
	SAVE_BOOL(m_bForceGround);
	SAVE_HSTRING(m_hstrHolster);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAIHuman::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    if (!g_pLTServer || !hRead) return;

	CAI::Load(hRead, dwLoadFlags);

	LOAD_HSTRING(m_hstrBrain);
	LTBOOL bBrain = LTFALSE;
	LOAD_BOOL(bBrain);

	if ( bBrain )
	{
		SetBrain(m_hstrBrain);
		if ( m_pBrain )
		{
			m_pBrain->Load(hRead);
		}
	}

	LTBOOL bAnimationContext = LTFALSE;
	LOAD_BOOL(bAnimationContext);

	if ( m_pAnimationContext )
	{
		g_pAnimationMgrHuman->DestroyAnimationContext(m_pAnimationContext);
		m_pAnimationContext = LTNULL;
	}

	if (bAnimationContext)
	{
		m_pAnimationContext = g_pAnimationMgrHuman->CreateAnimationContext(m_hObject);

		if ( m_pAnimationContext )
		{
			m_pAnimationContext->Load(hRead);
		}
	}

	m_Nudge.Load(hRead);

	// Load state...

	uint32 dwState;
	LOAD_DWORD(dwState);

	if (dwState != (DWORD)-1)
	{
		SetState((CAIHumanState::AIHumanStateType)dwState, LTFALSE);

		if (m_pHumanState)
		{
			m_pHumanState->Load(hRead);
		}
	}

	LOAD_BOOL(m_bSyncPosition);
	LOAD_VECTOR(m_vMovePos);
	LOAD_BOOL(m_bMove);
	LOAD_BOOL(m_bPosDirty);
	LOAD_FLOAT(m_fSpeed);
	LOAD_FLOAT(m_fCreepVel);
	LOAD_BOOL(m_bCanShortRecoil);
	LOAD_HSTRING(m_hstrCmdLighter);
	LOAD_BOOL(m_bForceGround);
	LOAD_HSTRING(m_hstrHolster);

	ComputeSquares();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::SetBrain
//
//	PURPOSE:	Changes our current Brain
//
// ----------------------------------------------------------------------- //

void CAIHuman::SetBrain(HSTRING hstrBrain)
{
	// Delete the old Brain

	if ( m_pBrain )
	{
		FACTORY_DELETE(m_pBrain);
		m_pBrain = LTNULL;
	}

	m_pBrain = FACTORY_NEW(CAIBrain);

	if ( !hstrBrain || (-1 == g_pAIButeMgr->GetBrainIDByName(g_pLTServer->GetStringData(hstrBrain))) )
	{
		g_pLTServer->CPrint("AI ''%s'' does not have a valid brain specified! Using ''Default''.", GetName());
		m_pBrain->Init(this, "Default");
	}
	else
	{
		m_pBrain->Init(this, g_pLTServer->GetStringData(hstrBrain));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::SetState
//
//	PURPOSE:	Changes our current state
//
// ----------------------------------------------------------------------- //

void CAIHuman::SetState(CAIHumanState::AIHumanStateType eState, LTBOOL bUnlockAnimation /* = LTTRUE */)
{
	if ( m_pState && m_pState->DelayChangeState() )
	{
		_ASSERT(!"Got direct SetState() in state that can't change states!!!");
	}

	// Delete the old state

	if ( m_pHumanState )
	{
		FACTORY_DELETE(m_pHumanState);
		m_pHumanState = LTNULL;
		m_pState = LTNULL;
	}

	switch ( eState )
	{
		case CAIHumanState::eStateIdle:					m_pHumanState = FACTORY_NEW(CAIHumanStateIdle);					break;
		case CAIHumanState::eStateAware:				m_pHumanState = FACTORY_NEW(CAIHumanStateAware);				break;
		case CAIHumanState::eStateLookAt:				m_pHumanState = FACTORY_NEW(CAIHumanStateLookAt);				break;
		case CAIHumanState::eStateAssassinate:			m_pHumanState = FACTORY_NEW(CAIHumanStateAssassinate);			break;
		case CAIHumanState::eStateCover:				m_pHumanState = FACTORY_NEW(CAIHumanStateCover);				break;
		case CAIHumanState::eStateDraw:					m_pHumanState = FACTORY_NEW(CAIHumanStateDraw);					break;
		case CAIHumanState::eStateAttack:				m_pHumanState = FACTORY_NEW(CAIHumanStateAttack);				break;
		case CAIHumanState::eStateAttackFromCover:		m_pHumanState = FACTORY_NEW(CAIHumanStateAttackFromCover);		break;
		case CAIHumanState::eStateAttackFromVantage:	m_pHumanState = FACTORY_NEW(CAIHumanStateAttackFromVantage);	break;
		case CAIHumanState::eStateAttackFromView:		m_pHumanState = FACTORY_NEW(CAIHumanStateAttackFromView);		break;
		case CAIHumanState::eStateAttackOnSight:		m_pHumanState = FACTORY_NEW(CAIHumanStateAttackOnSight);		break;
		case CAIHumanState::eStateAttackProp:			m_pHumanState = FACTORY_NEW(CAIHumanStateAttackProp);			break;
		case CAIHumanState::eStatePanic:				m_pHumanState = FACTORY_NEW(CAIHumanStatePanic);				break;
		case CAIHumanState::eStateDistress:				m_pHumanState = FACTORY_NEW(CAIHumanStateDistress);				break;
		case CAIHumanState::eStatePatrol:				m_pHumanState = FACTORY_NEW(CAIHumanStatePatrol);				break;
		case CAIHumanState::eStateGoto:					m_pHumanState = FACTORY_NEW(CAIHumanStateGoto);					break;
		case CAIHumanState::eStateFlee:					m_pHumanState = FACTORY_NEW(CAIHumanStateFlee);					break;
		case CAIHumanState::eStateSearch:				m_pHumanState = FACTORY_NEW(CAIHumanStateSearch);				break;
		case CAIHumanState::eStateTail:					m_pHumanState = FACTORY_NEW(CAIHumanStateTail);					break;
		case CAIHumanState::eStateFollowFootprint:		m_pHumanState = FACTORY_NEW(CAIHumanStateFollowFootprint);		break;
		case CAIHumanState::eStateInvestigate:			m_pHumanState = FACTORY_NEW(CAIHumanStateInvestigate);			break;
		case CAIHumanState::eStateCheckBody:			m_pHumanState = FACTORY_NEW(CAIHumanStateCheckBody);			break;
		case CAIHumanState::eStateChase:				m_pHumanState = FACTORY_NEW(CAIHumanStateChase);				break;
		case CAIHumanState::eStateUseObject:			m_pHumanState = FACTORY_NEW(CAIHumanStateUseObject);			break;
		case CAIHumanState::eStatePickupObject:			m_pHumanState = FACTORY_NEW(CAIHumanStatePickupObject);			break;
		case CAIHumanState::eStateTalk:					m_pHumanState = FACTORY_NEW(CAIHumanStateTalk);					break;
		case CAIHumanState::eStateGetBackup:			m_pHumanState = FACTORY_NEW(CAIHumanStateGetBackup);			break;
		case CAIHumanState::eStateDrowsy:				m_pHumanState = FACTORY_NEW(CAIHumanStateDrowsy);				break;
		case CAIHumanState::eStateUnconscious:			m_pHumanState = FACTORY_NEW(CAIHumanStateUnconscious);			break;
		case CAIHumanState::eStateStunned:				m_pHumanState = FACTORY_NEW(CAIHumanStateStunned);				break;
		case CAIHumanState::eStateAnimate:				m_pHumanState = FACTORY_NEW(CAIHumanStateAnimate);				break;
		case CAIHumanState::eStateCharge:				m_pHumanState = FACTORY_NEW(CAIHumanStateCharge);				break;
//		case CAIHumanState::eStateCome:					m_pHumanState = FACTORY_NEW(CAIHumanStateCome);					break;
		case CAIHumanState::eStateFollow:				m_pHumanState = FACTORY_NEW(CAIHumanStateFollow);				break;
		case CAIHumanState::eStateParaDive:				m_pHumanState = FACTORY_NEW(CAIHumanStateParaDive);				break;
		case CAIHumanState::eStateParaShoot:			m_pHumanState = FACTORY_NEW(CAIHumanStateParaShoot);			break;
		case CAIHumanState::eStateParaDie:				m_pHumanState = FACTORY_NEW(CAIHumanStateParaDie);				break;
		case CAIHumanState::eStateParaEscape:			m_pHumanState = FACTORY_NEW(CAIHumanStateParaEscape);			break;
		case CAIHumanState::eStateHeliAttack:			m_pHumanState = FACTORY_NEW(CAIHumanStateHeliAttack);			break;
		case CAIHumanState::eStateScotBox:				m_pHumanState = FACTORY_NEW(CAIHumanStateScotBox);				break;
		case CAIHumanState::eStateIngeSing:				m_pHumanState = FACTORY_NEW(CAIHumanStateIngeSing);				break;
	}

	m_pState = m_pHumanState;

	if ( !m_pHumanState )
	{
		_ASSERT(LTFALSE);
		return;
	}

	// Set our nudge priority

	m_Nudge.SetPriority(m_pHumanState->GetNudgePriority());

	// Unlock the animation context so we don't get stuck playing some dippy animation

	if ( bUnlockAnimation && GetAnimationContext() )
	{
		GetAnimationContext()->Unlock();
	}

	// Init the state

	if ( !m_pHumanState->Init(this) )
	{
		_ASSERT(LTFALSE);
		FACTORY_DELETE(m_pHumanState);
		m_pHumanState = LTNULL;
		m_pState = LTNULL;
	}

	// Update our user flags

	UpdateUserFlagCanActivate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CAIHuman::CreateAttachments()
{
	if ( !m_pAttachments )
	{
		m_pAttachments = static_cast<CHumanAttachments*>(CAttachments::Create(ATTACHMENTS_TYPE_HUMAN));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetSpeed
//
//	PURPOSE:	Gets our current movement speed
//
// ----------------------------------------------------------------------- //

LTFLOAT CAIHuman::GetSpeed()
{
	// We basically want to slow down if we're not pointing the way we're moving

	LTFLOAT fModifier = (1.0f + m_vTargetForward.Dot(m_vForward))/2.0f;

	return fModifier*m_fSpeed;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::Aim
//
//	PURPOSE:	Aim in a direction
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::Aim(const LTVector& vDir)
{
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::AimAt
//
//	PURPOSE:	Aim at a position
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::AimAt(const LTVector& vPosition)
{
	LTVector vPos1(vPosition.x, 0.0f, vPosition.z);
	LTVector vPos2(m_vPos.x, 0.0f, m_vPos.z);

	LTFLOAT fDist = vPos1.Dist(vPos2);

	LTFLOAT fPitch = vPosition.y - m_vPos.y;
	fPitch = (LTFLOAT)atan(fPitch/(fDist+.001f));

	fPitch = (fPitch + MATH_PI/2.0f)/MATH_PI;
	_ASSERT(fPitch >= 0.0f && fPitch <= 1.0f );

//	g_pLTServer->CPrint("pitch = %f", fPitch);
	GetAnimationContext()->SetPitch(fPitch);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::AimAt
//
//	PURPOSE:	Aim at an object
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHuman::AimAt(HOBJECT hObject)
{
    LTVector vPos;
    g_pLTServer->GetObjectPos(hObject, &vPos);
	AimAt(vPos);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::GetWeaponPosition()
//
//	PURPOSE:	Is the position of our "Weapon" (could be anything)
//
// ----------------------------------------------------------------------- //

LTVector CAIHuman::GetWeaponPosition(CWeapon* pWeapon)
{
	LTVector vPos = m_vTorsoPos;
	vPos.y = m_vPos.y + m_vDims.y;

	return vPos;
}
/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHuman::SetSleeping()
//
//	PURPOSE:	Handle sleep mode
//
// ----------------------------------------------------------------------- //

void CAIHuman::SetSleeping(LTBOOL bSleeping)
{
    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, CFX_SLEEP_MSG);
	g_pLTServer->WriteToMessageByte(hMessage, bSleeping );
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}
*/
// Plugin statics
LTBOOL s_bPluginInitted = LTFALSE;
CAIButeMgr s_AIButeMgr;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CAIHumanPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if ( !s_bPluginInitted )
	{
		char szFile[256];
		sprintf(szFile, "%s\\Attributes\\AIButes.txt", szRezPath);
		s_AIButeMgr.SetInRezFile(LTFALSE);
        s_AIButeMgr.Init(g_pLTServer, szFile);
		s_bPluginInitted = LTTRUE;
	}

	// See if the base class wishes to handle it

	if ( LT_OK == CAIPlugin::PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

	// See if the attachments plugin will handle the property

	if ( LT_OK == GetAttachmentsPlugin()->PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

	// See if the model style plugin will handle the property

	if ( !_strcmpi("Brain", szPropName) )
	{
		// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
		uint32 cBrains = s_AIButeMgr.GetNumBrains();
		_ASSERT(cMaxStrings >= cBrains);
		for ( uint32 iBrain = 0 ; iBrain < cBrains ; iBrain++ )
		{
			strcpy(aszStrings[(*pcStrings)++], s_AIButeMgr.GetBrain(iBrain)->szName);
		}

		return LT_OK;
	}

	// No one wants it

	return LT_UNSUPPORTED;
}
