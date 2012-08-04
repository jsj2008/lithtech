// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeBackpedal.cpp
//
// PURPOSE : AIActionDodgeBackpedal class implementation
//
// CREATED : 3/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIActionDodgeBackpedal.h

#include "Stdafx.h"
#include "AIWorldState.h"
#include "AIActionAbstract.h"
#include "AIActionDodgeBackpedal.h"

// Includes required for AIActionDodgeBackpedal.cpp

#include "AI.h"
#include "AIBrain.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIPathMgrNavMesh.h"
#include "AnimationContext.h"
#include "CharacterMgr.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeBackpedal, kAct_DodgeBackpedal );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDodgeBackpedal::CAIActionDodgeBackpedal()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeBackpedal::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// Target is no longer aiming at me.
	// No longer at target.

	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsAimingAtMe, NULL, kWST_bool, false );
	m_wsWorldStateEffects.SetWSProp( kWSK_AtTargetPos, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeBackpedal::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Do not dodge if AI is at some node.

	SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
	if( pProp && pProp->hWSValue )
	{
		return false;
	}

	// Only dodge if we are armed.

	pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_WeaponArmed, pAI->m_hObject );
	if( pProp && !pProp->bWSValue )
	{
		return false;
	}

	// Only dodge if there is a clear path to back.

	float fCheckDistance = GetDodgeDist( pAI ) + pAI->GetRadius();
	if( !( IsClearForDodge( pAI, -fCheckDistance ) ) )
	{
		return false;
	}

	// Target is not in range (or really close).

	if (!AIWeaponUtils::IsInRange(pAI, kAIWeaponType_Melee, bIsPlanning) &&
		!AIWeaponUtils::IsInRange(pAI, kAIWeaponType_Ranged, bIsPlanning))
	{
		return false;
	}

	// Only dodge if the AI is not trying to dodge a blow

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_CounterMelee );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact && ( 0.0f != pFact->GetConfidence( CAIWMFact::kFactMask_DesireType ) ) )
	{
		return false;
	}

	// Do not dodge.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeBackpedal::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Face perpendicular to the dodge direction.

	pAI->GetAIBlackBoard()->SetBBFaceDir( pAI->GetTorsoForward() );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set dodge animation.

	CAnimationProps	animProps;
	SetDodgeAnim( pAI, animProps );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Depart from an AINode.
	
	pStateAnimate->DepartNode();

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::GetDodgeDist
//
//	PURPOSE:	Return the distance the dodge animation covers.
//
// ----------------------------------------------------------------------- //

float CAIActionDodgeBackpedal::GetDodgeDist( CAI* pAI )
{
	// TEMP HARD CODING -- BASE ON ANIMATION INFO
	return 32.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::GetDodgeAnim
//
//	PURPOSE:	Return the dodge animation prop.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeBackpedal::SetDodgeAnim( CAI* pAI, CAnimationProps& animProps )
{
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Action, kAP_ACT_Backpedal );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::IsClearForDodge
//
//	PURPOSE:	Return true if there is a clear path for dodging to 
//              the requested position.
//
// ----------------------------------------------------------------------- //
#include "DebugLineSystem.h"

bool CAIActionDodgeBackpedal::IsClearForDodge( CAI* pAI, float fDodgeDist )
{
	LTVector vPos = pAI->GetPosition();
	LTVector vOffset = pAI->GetTorsoForward();
	vOffset.y = 0;
	vOffset.Normalize();
	vOffset *= fDodgeDist;

	// No straight path exists.

	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), vPos, vPos + vOffset, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	// Path is blocked by allies.

	if( g_pCharacterMgr->RayIntersectAI( vPos, vPos + vOffset, pAI, NULL, NULL ) )
	{
		return false;
	}

	// Path is clear.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeBackpedal::IsActionComplete( CAI* pAI )
{
	// Dodging is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Dodging is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeBackpedal::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeBackpedal::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
