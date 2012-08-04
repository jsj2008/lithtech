// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLookAtDisturbance.cpp
//
// PURPOSE : AIActionLookAtDisturbance class implementation
//
// CREATED : 3/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionLookAtDisturbance.h"
#include "AI.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AIPathMgrNavMesh.h"
#include "AITarget.h"
#include "AINodeMgr.h"
#include "AIUtils.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLookAtDisturbance, kAct_LookAtDisturbance );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbance::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionLookAtDisturbance::CAIActionLookAtDisturbance()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbance::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionLookAtDisturbance::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// AI is armed.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );

	// Set effects.
	// Disturbance has been investigated.

	m_wsWorldStateEffects.SetWSProp( kWSK_DisturbanceExists, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbance::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionLookAtDisturbance::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// This action is only valid if the AI can potentially see the position the disturbance is at.
	// TODO: This is expensive to test.  May way to look for a less expensive way to handle this.
	// TODO: This uses the AIs current position and current direction.  A crouching AI may not 
	//		be able to see what a standing AI can see.  Consider using some other source position.

	if ( bIsPlanning )
	{

		HOBJECT hTarget = NULL;
		LTVector vTargetPos;
		if( pAI->HasTarget( kTarget_Disturbance ) )
		{
			hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
			vTargetPos = pAI->GetAIBlackBoard()->GetBBTargetPosition();
		}

		// Don't look at a disturbance if our target is in view.

		else if( pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
		{
			return false;
		}

		// This action may be used to look at danger, e.g. a grenade.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Danger );
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			vTargetPos = pFact->GetPos();
			hTarget = pFact->GetSourceObject();
		}

		if( g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vTargetPos, pAI->GetLastNavMeshPoly(), 0.f ) )
		{
			return true;
		}

		float fSeeEnemyDistanceSqr = pAI->GetAIBlackBoard()->GetBBSeeDistance();
		fSeeEnemyDistanceSqr *= fSeeEnemyDistanceSqr;

		bool bDisturbanceVisible = false;
		if( pAI->CanSeeThrough() )
		{
			bDisturbanceVisible = pAI->IsObjectPositionVisible( CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, pAI->GetEyePosition(), hTarget, vTargetPos, fSeeEnemyDistanceSqr, false, true );
			if( !bDisturbanceVisible )
			{
				bDisturbanceVisible = pAI->IsObjectPositionVisible( CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, pAI->GetEyePosition(), NULL, vTargetPos, fSeeEnemyDistanceSqr, false, true );
			}
		}
		else
		{
			bDisturbanceVisible = pAI->IsObjectPositionVisible( CAI::DefaultFilterFn, NULL, pAI->GetEyePosition(), hTarget, vTargetPos, fSeeEnemyDistanceSqr, false, false );
			if( !bDisturbanceVisible )
			{
				bDisturbanceVisible = pAI->IsObjectPositionVisible( CAI::DefaultFilterFn, NULL, pAI->GetEyePosition(), NULL, vTargetPos, fSeeEnemyDistanceSqr, false, false );
			}
		}

		// Can't see the disturbance position from where the AI is at currently.

		if (!bDisturbanceVisible)
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbance::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionLookAtDisturbance::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set look animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Lower );

	LTVector vTargetPos = pAI->GetAIBlackBoard()->GetBBTargetPosition();

	// This action may be used to look at danger, e.g. a grenade.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	CAIWMFact* pDangerFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pDangerFact )
	{
		vTargetPos = pDangerFact->GetPos();
	}


	LTVector vDir = vTargetPos - pAI->GetPosition();
	vDir.Normalize();

	if( vDir.Dot( pAI->GetForwardVector() ) < 0.f )
	{
		animProps.Set( kAPG_Action, kAP_ACT_Alert );
		pAI->GetAIBlackBoard()->SetBBFacePos( vTargetPos );
	}
	else if( vDir.Dot( pAI->GetForwardVector() ) > c_fFOV60 )
	{
		animProps.Set( kAPG_Action, kAP_ACT_Alert );
	}
	else if( vDir.Dot( pAI->GetRightVector() ) > 0.0f )
	{
		animProps.Set( kAPG_Action, kAP_ACT_LookRight );
	}
	else {
		animProps.Set( kAPG_Action, kAP_ACT_LookLeft );
	}

	// Loop infinitely if searching is not an option.
	// Only loop infinitely if investigating.
	// Do not loop if we are reacting to danger.
	// Do not loop if the game settings do not permit it.

	bool bLoop = false;
	if( !pDangerFact 
		&& !pAI->CanSearch() 
		&& g_pAIDB->GetAIConstantsRecord()->bLoopInvestigateIfCantSearch )
	{
		animProps.Set( kAPG_Action, kAP_ACT_Alert );
		bLoop = true;
	}

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, bLoop );

	// Search for the source of the disturbance.

	if( pAI->IsMajorlyAlarmed() && pAI->CanSearch() )
	{
		// Find an existing memory for the desire to search, or create one.

		CAIWMFact factQuery;
		factQuery.SetFactType(kFact_Desire);
		factQuery.SetDesireType(kDesire_Search);
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( !pFact )
		{
			pFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
		}

		// Setup the current desire.

		if( pFact )
		{
			pFact->SetDesireType( kDesire_Search, 1.f );

			// The source of the disturbance is the search origin.
			// If searching an ally pain sound, search from the damager's position.

			LTVector vPos;
			HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
			if( ( pAI->GetAIBlackBoard()->GetBBTargetStimulusType() == kStim_PainSound ) &&
				IsAI( hTarget ) )
			{
				CAI* pAI = (CAI*)g_pLTServer->HandleToObject( hTarget );
				hTarget = pAI->GetDestructible()->GetLastDamager();
			}

			g_pLTServer->GetObjectPos( hTarget, &vPos );
			pFact->SetPos( vPos, 1.f );
		}
	}

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbance::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionLookAtDisturbance::IsActionComplete( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return true;
	}

	// Investigation is complete if we can see a character target.

	if( pAI->HasTarget( kTarget_Character ) &&
		pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
	{
		return true;
	}

	// Investigating is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Investigating is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbance::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionLookAtDisturbance::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

