// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDefeatedRecoil.cpp
//
// PURPOSE : 
//
// CREATED : 9/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDefeatedRecoil.h"
#include "AI.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDefeatedRecoil, kAct_DefeatedRecoil );

static bool  s_bInitializedConstants = false;
static float s_fDefeatedHealthThreshold = 0.0f;
static float s_fDefeatedRecoveryHitpointGain = 0.0f;

static void InitConstants()
{
	if ( !s_bInitializedConstants )
	{
		s_fDefeatedHealthThreshold = g_pAIDB->GetMiscFloat( "DefeatedHealthThreshold" );
		s_fDefeatedRecoveryHitpointGain = g_pAIDB->GetMiscFloat( "DefeatedRecoveryHitpointGain" );
		s_bInitializedConstants = true;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedRecoil::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDefeatedRecoil::CAIActionDefeatedRecoil()
{
	InitConstants();
}

CAIActionDefeatedRecoil::~CAIActionDefeatedRecoil()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedRecoil::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDefeatedRecoil::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI reacted to defeated state (this intentionally stomps the 
	// kWSE_Damaged value set by the base class).

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_Defeated );
}
 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedRecoil::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDefeatedRecoil::ActivateAction(CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Clear the facedir set in the base class, and turn off rotation to 
	// face all together.

	pAI->GetAIBlackBoard()->SetBBFaceType( kFaceType_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedRecoil::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDefeatedRecoil::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Fail if AI is attached to something.
	
	HOBJECT hAttached = pAI->GetAIBlackBoard()->GetBBAttachedTo();
	if( hAttached )
	{
		return false;
	}

	// Fail if the AIs health is above the threshold.

	float flHitPointPercent = LTMAX(0.0f, pAI->GetDestructible()->GetHitPoints() / pAI->GetDestructible()->GetMaxHitPoints() );
	if ( flHitPointPercent > s_fDefeatedHealthThreshold )
	{
		return false;
	}

	// Fail if the AI is currently playing a transition (this is to avoid
	// an animation pop as the AI goes to his knees

	if ( pAI->GetAnimationContext()->IsTransitioning() )
	{
		return false;
	}

	// Fail if the damage type was not melee.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pFact )
	{
		return false;
	}

	DamageType eDamageType = DT_UNSPECIFIED;
	pFact->GetDamage( &eDamageType, NULL, NULL );
	if ( DT_MELEE != eDamageType )
	{
		return false;;
	}

	// Normally we do the base class validation first.  By performing it
	//  second, we can do cheap tests first, as the base class does animation
	// tree searches.

	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDefeatedRecoil::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDefeatedRecoil::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Give the AI some amount if this action deactives due to satisfaction
	// while the AI is still alive.

	if ( !IsDeadAI( pAI->GetHOBJECT() ) )
	{
		float flNewHitPoints = LTMAX( 0.0f, pAI->GetDestructible()->GetHitPoints() ) + pAI->GetDestructible()->GetMaxHitPoints() * s_fDefeatedRecoveryHitpointGain;
		pAI->GetDestructible()->SetHitPoints( flNewHitPoints );
	}

	// Force clear the any scripted body state, just in case something either 
	// interrupted this action or in case we failed to clear it via model 
	// strings.

	pAI->GetAIBlackBoard()->SetBBScriptedBodyState( eBodyStateInvalid );
	pAI->GetAIBlackBoard()->SetBBUpdateClient( true );

	pAI->PlayerFinishingMoveCleanup();
}
