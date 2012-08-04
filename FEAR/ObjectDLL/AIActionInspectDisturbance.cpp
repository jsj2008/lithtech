// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionInspectDisturbance.cpp
//
// PURPOSE : AIActionInspectDisturbance class implementation
//
// CREATED : 3/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionInspectDisturbance.h"
#include "AI.h"
#include "AIState.h"
#include "AIStateAnimate.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIPathMgrNavMesh.h"
#include "AIStimulusMgr.h"
#include "AIUtils.h"
#include "AIWorkingMemoryCentral.h"
#include "AnimationContext.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionInspectDisturbance, kAct_InspectDisturbance );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInspectDisturbance::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionInspectDisturbance::CAIActionInspectDisturbance()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInspectDisturbance::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionInspectDisturbance::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// At the disturbance location.
	// AI is armed.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_WeaponArmed, NULL, kWST_bool, true );
	m_wsWorldStatePreconditions.SetWSProp( kWSK_AtTargetPos, NULL, kWST_bool, true );

	// Set effects.
	// Disturbance has been investigated.

	m_wsWorldStateEffects.SetWSProp( kWSK_DisturbanceExists, NULL, kWST_bool, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInspectDisturbance::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionInspectDisturbance::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// This action is only valid if we can find a path to the disturbance.

	if( g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPosition() ) )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInspectDisturbance::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionInspectDisturbance::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set animate state.
	
	pAI->SetState( kState_Animate );

	// Set investigate animation.

	CAnimationProps	animProps;
	animProps.Set( kAPG_Posture, kAP_POS_Stand );
	animProps.Set( kAPG_Weapon, pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	animProps.Set( kAPG_WeaponPosition, kAP_WPOS_Lower );
	animProps.Set( kAPG_Action, kAP_ACT_Alert );

	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( animProps, !LOOP );

	// Let other AI know we have claimed this NavMesh poly for investigation.

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
	if (pFact)
	{
		pFact->SetKnowledgeType( kKnowledge_InvestigatingNavMeshPoly, 1.f );
		pFact->SetSourceObject( pAI->m_hObject );
		pFact->SetIndex( pAI->GetLastNavMeshPoly() );
	}

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
//	ROUTINE:	CAIActionInspectDisturbance::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionInspectDisturbance::DeactivateAction( CAI* pAI )
{
	// Remove knowledge of investigating.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_InvestigatingNavMeshPoly );
	factQuery.SetSourceObject( pAI->m_hObject );
	g_pAIWorkingMemoryCentral->ClearWMFact( factQuery );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionInspectDisturbance::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionInspectDisturbance::IsActionComplete( CAI* pAI )
{
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
//	ROUTINE:	CAIActionInspectDisturbance::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionInspectDisturbance::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

