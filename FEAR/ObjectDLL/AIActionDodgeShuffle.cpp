// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeShuffle.cpp
//
// PURPOSE : AIActionDodgeShuffle class implementation
//
// CREATED : 3/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDodgeShuffle.h"
#include "AI.h"
#include "AIBrain.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIPathMgrNavMesh.h"
#include "AnimationContext.h"
#include "CharacterMgr.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeShuffle, kAct_DodgeShuffle );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDodgeShuffle::CAIActionDodgeShuffle()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeShuffle::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// Target is no longer aiming at me.

	m_wsWorldStateEffects.SetWSProp( kWSK_TargetIsAimingAtMe, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeShuffle::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Do not dodge if AI is attached to something.

	HOBJECT hAttached = pAI->GetAIBlackBoard()->GetBBAttachedTo();
	if( hAttached )
	{
		return false;
	}

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

	// Only dodge if the AI is not trying to dodge a blow

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_CounterMelee );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact && ( 0.0f != pFact->GetConfidence( CAIWMFact::kFactMask_DesireType ) ) )
	{
		return false;
	}

	// Only dodge if there is a clear path to the right or left.

	float fCheckDistance = GetDodgeDist( pAI ) + pAI->GetRadius();
	CAI* pIntersectedAI = NULL;
	if( !( IsClearForDodge(pAI, fCheckDistance, NULL, &pIntersectedAI) ||
		   IsClearForDodge(pAI, -fCheckDistance, pIntersectedAI, NULL)) )
	{
		return false;
	}

	// Do not dodge.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeShuffle::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Randomly choose to dodge left or right.

	float fRandom = GetRandom(0.0f, 1.0f);
	float fDir = ( fRandom < 0.5f ) ? 1.f : -1.f;

	// If there is not room in the randomly chosen direction, swap.

	float fCheckDistance = GetDodgeDist( pAI ) + pAI->GetRadius();
	if( !IsClearForDodge( pAI, fCheckDistance * fDir ) )
	{
		fDir *= -1.f;
	}

	// Face perpendicular to the dodge direction.

	pAI->GetAIBlackBoard()->SetBBFaceDir( pAI->GetTorsoForward() );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set dodge animation.

	CAnimationProps	animProps;
	SetDodgeAnim( pAI, fDir, animProps );

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
//	ROUTINE:	CAIActionDodgeShuffle::GetDodgeDist
//
//	PURPOSE:	Return the distance the dodge animation covers.
//
// ----------------------------------------------------------------------- //

float CAIActionDodgeShuffle::GetDodgeDist( CAI* pAI )
{
	return pAI->GetBrain()->GetDodgeVectorShuffleDist();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::GetDodgeAnim
//
//	PURPOSE:	Return the dodge animation prop.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeShuffle::SetDodgeAnim( CAI* pAI, float fDir, CAnimationProps& animProps )
{
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	if( pAI->GetAnimationContext()->GetCurrentProp( kAPG_Posture ) == kAP_POS_Crouch )
	{
		animProps.Set( kAPG_Posture, kAP_POS_Crouch );
	}

	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	animProps.Set( kAPG_Action, kAP_ACT_Shuffle );

	if( fDir < 0.f )
	{
		animProps.Set( kAPG_MovementDir, kAP_MDIR_Left );
	}
	else {
		animProps.Set( kAPG_MovementDir, kAP_MDIR_Right );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::IsClearForDodge
//
//	PURPOSE:	Return true if there is a clear path for dodging to 
//              the requested position.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeShuffle::IsClearForDodge( CAI* pAI, float fDodgeDist, CAI* pIgnoreAI/*=NULL*/, CAI** pIntersectedAI/*=NULL*/ )
{
	LTVector vPos = pAI->GetPosition();

	LTVector vOffset = pAI->GetTorsoRight() * fDodgeDist;

	// No straight path exists.

	if( !g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), vPos, vPos + vOffset, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		return false;
	}

	// Path is blocked by allies.

	if( g_pCharacterMgr->RayIntersectAI( vPos, vPos + vOffset, pAI, pIgnoreAI, pIntersectedAI, 2500.f ) )
	{
		return false;
	}

	// Path is clear.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDodgeShuffle::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDodgeShuffle::IsActionComplete( CAI* pAI )
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
//	ROUTINE:	CAIActionDodgeShuffle::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDodgeShuffle::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}
