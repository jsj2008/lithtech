// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoTarget.cpp
//
// PURPOSE : AIActionGotoTarget class implementation
//
// CREATED : 3/13/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoTarget.h"
#include "AI.h"
#include "AITarget.h"
#include "AIState.h"
#include "AIStateGoto.h"
#include "AISoundMgr.h"
#include "AIUtils.h"
#include "AIBlackBoard.h"
#include "AIPathMgrNavMesh.h"
#include "NodeTrackerContext.h"
#include "AINavMesh.h"
#include "AIQuadTree.h"
#include "AIWorkingMemoryCentral.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoTarget, kAct_GotoTarget );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoTarget::CAIActionGotoTarget()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoTarget::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// At the target's pos.

	m_wsWorldStateEffects.SetWSProp( kWSK_AtTargetPos, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget::SetPlanWSPreconditions
//
//	PURPOSE:	Set this action's preconditions in plan's goal world state.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoTarget::SetPlanWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::SetPlanWSPreconditions( pAI, wsWorldStateGoal );

	// Pause to survey area before walking to invstigate something.
	// Do not pause to survey the area if responding to gunfire.

	if( pAI->HasTarget( kTarget_Disturbance ) &&
		( pAI->GetAIBlackBoard()->GetBBTargetStimulusType() != kStim_WeaponFireSound ) )
	{
		wsWorldStateGoal.SetWSProp( kWSK_SurveyedArea, pAI->m_hObject, kWST_bool, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoTarget::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Only check path if planning.

	if( !bIsPlanning )
	{
		return true;
	}

	// Determine if a path exists to the target.

	if (!pAI->HasTarget( kTarget_All & ~kTarget_Leader ) )
	{
		return false;
	}

	// Determine if target is reachable.

	if( pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPoly() == kNMPoly_Invalid )
	{
		return false;
	}

	LTVector vDest = pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPosition();

	// Determine if the target is outside the value range (if one is set)

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if ( pSmartObjectRecord )
	{
		float flDistSqr = vDest.DistSqr( pAI->GetPosition() );
		if ( flDistSqr < pSmartObjectRecord->fMinDist*pSmartObjectRecord->fMinDist
			|| flDistSqr > pSmartObjectRecord->fMaxDist*pSmartObjectRecord->fMaxDist )
		{
			return false;
		}
	}

	// Already there.

	LTVector vPos = pAI->GetPosition();
	if( ( vPos.x == vDest.x ) &&
		( vPos.z == vDest.z ) )
	{
		// Now do the expensive check to insure dest is in the same poly the
		// AI to take into account the fact that this is a 3D mesh, and that
		// the AI could be directly a floor below the location he wants to 
		// path to.

		// If targeting a character, use the character's last known
		// NavMesh poly as a hint for finding the target's current
		// NavMesh poly.

		ENUM_NMPolyID ePolyHint = kNMPoly_Invalid;
		if( pAI->HasTarget( kTarget_Character ) )
		{
			HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
			CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
			if( pChar )
			{
				ePolyHint = pChar->GetLastNavMeshPoly();
			}
		}

		// Do not go anywhere if the AI is already at the dest.

		ENUM_NMPolyID eLastPoly = pAI->GetLastNavMeshPoly();
		ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly( vDest, pAI->GetCharTypeMask(), ePolyHint, pAI );
		if( ePoly == eLastPoly )
		{
			return false;
		}
	}

	// Return true if a path exists.

	return g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), vDest );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget::ValidateWSPreconditions
//
//	PURPOSE:	Return true if this action's preconditions are met in 
//              plan's current world state.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoTarget::ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK )
{
	if (!super::ValidateWSPreconditions( pAI, wsWorldStateCur, wsWorldStateGoal, pFailedWSK ) )
	{
		return false;
	}

	// If a WeaponArmed prop is specified, insure that it is already satisfied.
	// This is a workaround to the fact that the current precedence 
	// specification functionality does not work correct.
	//
	// The exception of the WeaponItems allows an AI to run to a targeted 
	// weapon item (when an AI is in the process of resolving WeaponArmed)
	//
	// If this logic is problematic, it can definitely be changed as needed.

	if ( !pAI->HasTarget(kTarget_WeaponItem) && wsWorldStateGoal.HasWSProp( kWSK_WeaponArmed, pAI->GetHOBJECT() ) )
	{
		SAIWORLDSTATE_PROP* pCurProp = wsWorldStateCur.GetWSProp( kWSK_WeaponArmed, pAI->GetHOBJECT() );
		SAIWORLDSTATE_PROP* pGoalProp = wsWorldStateGoal.GetWSProp( kWSK_WeaponArmed, pAI->GetHOBJECT() );
		if ( pCurProp && pGoalProp && pCurProp->bWSValue != pGoalProp->bWSValue)
		{
			return false;
		}
	}

	// The survey area precondition is added dynamically.
	// This is a workaround for failed precedence implementation.

	if( pAI->HasTarget( kTarget_Disturbance ) &&
		( pAI->GetAIBlackBoard()->GetBBTargetStimulusType() != kStim_WeaponFireSound ) )
	{
		SAIWORLDSTATE_PROP* pCurProp = wsWorldStateCur.GetWSProp( kWSK_SurveyedArea, pAI->GetHOBJECT() );
		if( pCurProp && !pCurProp->bWSValue )
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoTarget::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	if( !pAI->HasTarget( kTarget_All ) )
	{
		return;
	}

	// Set the Goto state.

	pAI->SetState( kState_Goto );
	CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();

	// Targeting a character.

	if( pAI->HasTarget( kTarget_Character ) )
	{
		// Play chase sound.

		if( pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
		{
			g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_Chase, kAISndCat_Combat, pAI->GetAIBlackBoard()->GetBBTargetObject(), 0.f );
		}

		// Set the destination.

		pGoto->SetDestObject( pAI->GetAIBlackBoard()->GetBBTargetObject(), LTVector( 0.f, 0.f, 0.f ) );

		// TODO: Determine a better number than 100.  If the target moves
		// 100 units, recalculate the path.  Shouldn't be so arbitrary.
		pGoto->SetDynamicRepathDistance(100.0f);

		// Torso tracking.

		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
		pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	}

	// Targeting something else.

	else {
		// Set the destination.

		pGoto->SetDest( pAI->GetAIBlackBoard()->GetBBTargetReachableNavMeshPosition() );

		// Torso tracking.

		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
		pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoTarget::IsActionComplete( CAI* pAI )
{
	// Goto is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// If heading towards a disturbance, consider yourself close enough
	// if another AI is already investigating the disturbance's NavMesh poly.

	if( pAI->HasTarget( kTarget_Disturbance ) &&
		( pAI->GetLastNavMeshPoly() == pAI->GetAIBlackBoard()->GetBBTargetTrueNavMeshPoly() ) )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Knowledge );
		factQuery.SetKnowledgeType( kKnowledge_InvestigatingNavMeshPoly );
		factQuery.SetIndex( pAI->GetAIBlackBoard()->GetBBTargetTrueNavMeshPoly() );
		CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
		if( pFact )
		{
			// Clear record of dead AI.

			if( IsDeadAI( pFact->GetSourceObject() ) )
			{
				g_pAIWorkingMemoryCentral->ClearWMFact( factQuery );
			}

			// We're done if we're not standing on anyone.

			else if( !g_pCharacterMgr->FindCharactersWithinRadius( NULL, pAI->GetPosition(), pAI->GetRadius(), pAI->m_hObject ) )
			{
				return true;
			}
		}
	}

	// Goto is not complete.

	return false;
}

