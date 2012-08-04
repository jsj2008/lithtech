// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionShovePathBlocker.cpp
//
// PURPOSE : 
//
// CREATED : 11/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionShovePathBlocker.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionShovePathBlocker, kAct_ShovePathBlocker );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShovePathBlocker::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionShovePathBlocker::CAIActionShovePathBlocker()
{
}

CAIActionShovePathBlocker::~CAIActionShovePathBlocker()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShovePathBlocker::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionShovePathBlocker::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// Reacted to the blockedpath event

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_BlockedPath );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShovePathBlocker::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid..
//
// ----------------------------------------------------------------------- //

bool CAIActionShovePathBlocker::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Fail if the AI does not have a blocked path fact.

	CAIWMFact queryBlockedPathFact;
	queryBlockedPathFact.SetFactType( kFact_Knowledge );
	queryBlockedPathFact.SetKnowledgeType( kKnowledge_BlockedPath );
	CAIWMFact* pBlockedPathFact = pAI->GetAIWorkingMemory()->FindWMFact( queryBlockedPathFact );
	if ( !pBlockedPathFact )
	{
		return false;
	}

	// Fail if the blocker is not an AI.

	if ( !IsAI( pBlockedPathFact->GetTargetObject() ) )
	{
		return false;
	}

	// Fail if the other AI is not solid to AIs.

	CAI* pPathBlockingAI = CAI::DynamicCast( pBlockedPathFact->GetTargetObject() );
	if ( !pPathBlockingAI->IsSolidToAI( ) )
	{
		return false;
	}

	// Action is valid

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShovePathBlocker::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionShovePathBlocker::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Set the behavior.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Set the anim object so that the AI can notify the other when the 
	// push occurs.

	HOBJECT hBlockingCharacter = NULL;

	CAIWMFact queryBlockedPathFact;
	queryBlockedPathFact.SetFactType( kFact_Knowledge );
	queryBlockedPathFact.SetKnowledgeType( kKnowledge_BlockedPath );
	CAIWMFact* pBlockedPathFact = pAI->GetAIWorkingMemory()->FindWMFact( queryBlockedPathFact );
	if ( pBlockedPathFact )
	{
		hBlockingCharacter = pBlockedPathFact->GetTargetObject();
	}

	pAI->SetAnimObject( hBlockingCharacter );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );

	// Face the guy we are kicking.  This should increase our chances of 
	// actually looking like we are kicking him.

	pAI->GetAIBlackBoard()->SetBBFaceObject( hBlockingCharacter );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShovePathBlocker::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionShovePathBlocker::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	if ( IsActionComplete( pAI ) )
	{
		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_BlockedPath );
		pAI->GetAIWorkingMemory()->ClearWMFacts( queryFact );

		pAI->GetAIBlackBoard()->SetBBMovementCollisionFlags( 
			(~kAIMovementFlag_BlockedPath) & pAI->GetAIBlackBoard()->GetBBMovementCollisionFlags() );
	}

	// Clear the animobject to prevent accidental interactions.

	pAI->SetAnimObject( NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionShovePathBlocker::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionShovePathBlocker::IsActionComplete( CAI* pAI )
{
	// Shove is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Shove is not complete.

	return false;
}
