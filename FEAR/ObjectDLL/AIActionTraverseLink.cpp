// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTraverseLink.cpp
//
// PURPOSE : AIActionTraverseLink class implementation
//
// CREATED : 7/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AIActionTraverseLink.h

#include "Stdafx.h"
#include "AIActionTraverseLink.h"
#include "AIWorldState.h"
#include "AI.h"
#include "AIDB.h"
#include "AIState.h"
#include "AIStateUseSmartObject.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "NodeTrackerContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTraverseLink, kAct_TraverseLink );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionTraverseLink::CAIActionTraverseLink()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionTraverseLink::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.

	// Set effects.
	// AI has traversed a NavMeshLink.

	m_wsWorldStateEffects.SetWSProp( kWSK_TraversedLink, NULL, kWST_Variable, kWSK_TraversedLink );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionTraverseLink::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Link does not exist.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( !pLink )
	{
		return false;
	}

	// Link does exist.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionTraverseLink::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Link does not exist.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( !pLink )
	{
		return;
	}

	// Set UseSmartObject state.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );

	// Set the smart object for the link.

	pStateUseSmartObject->SetSmartObject( pLink->GetSmartObject() );

	// Allow the link to make adjustments.

	pLink->ActivateTraversal( pAI, pStateUseSmartObject );

	// Auto-reload while traversing.

	pAI->GetAIBlackBoard()->SetBBAutoReload( true );

	// Torso tracking.

	if( pStateUseSmartObject->GetProp( kAPG_Action ) == kAP_ACT_Fire )
	{
		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	}
	else {
		pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
	}
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionTraverseLink::IsActionComplete( CAI* pAI )
{
	// Link does not exist.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( !pLink )
	{
		return true;
	}

	// Traversal is complete.

	if( pLink->IsTraversalComplete( pAI ) )
	{
		return true;
	}

	// Traversal is not complete.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionTraverseLink::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );

	// Apply any effects the link has.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( pLink )
	{
		pLink->ApplyTraversalEffect( pAI );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::ValidateAction
//
//	PURPOSE:	Returns true if the link is valid for openning, returns 
//				false if it is not.
//
// ----------------------------------------------------------------------- //

bool CAIActionTraverseLink::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Link does not exist.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( !pLink )
	{
		return false;
	}

	// Traversal is complete.

	if( !pLink->IsLinkValid( pAI, GetActionRecord()->eActionType, TRAVERSAL_IN_PROGRESS ) )
	{
		return false;
	}

	return true;
}
