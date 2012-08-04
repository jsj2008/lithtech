// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFollow.cpp
//
// PURPOSE : AIActionFollow class implementation
//
// CREATED : 4/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFollow.h"
#include "AI.h"
#include "AIDB.h"
#include "AIStateGoto.h"
#include "AIStateAnimate.h"
#include "AIStateUseSmartObject.h"
#include "AINavMesh.h"
#include "AIQuadTree.h"
#include "AIPathMgrNavMesh.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFollow, kAct_Follow );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollow::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionFollow::CAIActionFollow()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollow::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionFollow::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// At the target's pos.

	m_wsWorldStateEffects.SetWSProp( kWSK_AtTargetPos, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollow::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionFollow::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Only check path if planning.

	if( !bIsPlanning )
	{
		return true;
	}

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Bail if we are not following someone.

	if( ( pAI->GetAIBlackBoard()->GetBBTargetType() != kTarget_Leader ) &&
		( pAI->GetAIBlackBoard()->GetBBTargetType() != kTarget_Interest ) )
	{
		return false;
	}

	// Bail if we have no Follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) ) )
	{
		return false;
	}

	// Bail if follow target does not exist.

	HOBJECT hFollow = pFact->GetTargetObject();
	if( !hFollow )
	{
		return false;
	}

	// Go to the follow object's position.

	LTVector vDest;
	g_pLTServer->GetObjectPos( hFollow, &vDest );

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Return true if a path exists.

	return g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), vDest );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollow::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionFollow::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Bail if we have no Follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) ) )
	{
		return;
	}

	// Bail if follow target does not exist.

	HOBJECT hFollow = pFact->GetTargetObject();
	if( !hFollow )
	{
		return;
	}

	// Go to the follow object's position.

	LTVector vDest;
	g_pLTServer->GetObjectPos( hFollow, &vDest );


	//
	// Do not follow if we're already within some distance of the leader.
	//

	float fDistSqr = pAI->GetPosition().DistSqr( vDest );
	if( fDistSqr < pSmartObjectRecord->fMaxDist * pSmartObjectRecord->fMaxDist )
	{
		// Set SmartObject state.

		pAI->SetState( kState_UseSmartObject );

		// Set smartobject.

		CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
		pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );
	}


	//
	// Follow.
	//

	else {
		// Set the Goto state.

		pAI->SetState( kState_Goto );
		CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();

		// Set the destination.

		pGoto->SetDestObject( pFact->GetTargetObject(), LTVector( 0.f, 0.f, 0.f ) );

		// TODO: Determine a better number than 100.  If the target moves
		// 100 units, recalculate the path.  Shouldn't be so arbitrary.

		pGoto->SetDynamicRepathDistance(100.0f);

		// Run if we are too far away.

		float fTooFarSqr = pSmartObjectRecord->fMaxDist * 2.f;
		fTooFarSqr *= fTooFarSqr;
		if( fDistSqr > fTooFarSqr )
		{
			pGoto->SetMovementProp( kAP_MOV_Run );
		}

		// Use special investigative animations.

///		pGoto->SetActivityProp( pSmartObjectRecord->Props.Get( kAPG_Activity ) );

		// Last AI in the lineup faces backwards.

		int nIndex = pFact->GetIndex();
		if( nIndex == -1 )
		{
			pGoto->SetDirectionProp( kAP_MDIR_Backward );
		}
	}

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollow::ValidateAction
//
//	PURPOSE:	Returns true if AI should keep following.
//
// ----------------------------------------------------------------------- //

bool CAIActionFollow::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Bail if we have no Follow task.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Task );
	factQuery.SetTaskType( kTask_Follow );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_TaskType ) == 1.f ) ) )
	{
		return false;
	}

	// Bail if follow target does not exist.

	HOBJECT hFollow = pFact->GetTargetObject();
	if( !hFollow )
	{
		return false;
	}

	// Bail if no state.

	if( !pAI->GetState() )
	{
		return false;
	}

	// Do not follow if we're already within some distance of the leader.

	LTVector vDest;
	g_pLTServer->GetObjectPos( hFollow, &vDest );
	float fDistSqr = pAI->GetPosition().DistSqr( vDest );

	if( pAI->GetState()->GetStateClassType() == kState_Goto )
	{
		// Too close to the leader.

		if( fDistSqr < pSmartObjectRecord->fMinDist * pSmartObjectRecord->fMinDist )
		{
			return false;
		}

		// Far enough to run.

		if( pAI->GetAnimationContext()->GetCurrentProp( kAPG_Movement ) == kAP_MOV_Walk )
		{
			float fTooFarSqr = pSmartObjectRecord->fMaxDist * 2.f;
			fTooFarSqr *= fTooFarSqr;
			if( fDistSqr > fTooFarSqr )
			{
				return false;
			}
		}
	}
	
	// Start following again if leader gets too far away.

	else {
		if( fDistSqr > pSmartObjectRecord->fMaxDist * pSmartObjectRecord->fMaxDist )
		{
			return false;
		}
	}

	// Keep following.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollow::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionFollow::IsActionComplete( CAI* pAI )
{
	// Follow is complete if goto state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateClassType() == kState_Goto ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Follow is not complete.

	return false;
}

