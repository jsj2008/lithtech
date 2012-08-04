// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoFlamePotPosition.cpp
//
// PURPOSE : 
//
// CREATED : 4/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoFlamePotPosition.h"
#include "AIStateGoto.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoFlamePotPosition, kAct_GotoFlamePotPosition );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoFlamePotPosition::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoFlamePotPosition::CAIActionGotoFlamePotPosition()
{
}

CAIActionGotoFlamePotPosition::~CAIActionGotoFlamePotPosition()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoFlamePotPosition::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoFlamePotPosition::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Reacted to an enemy 

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_EnemyInFlamePot );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoFlamePotPosition::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoFlamePotPosition::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if the AI does not have a FlamePotPosition fact.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_FlamePotPosition );
	queryFact.SetSourceObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		return false;
	}

	// Determine if a path exists to the position.

	return g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), pFact->GetPos() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoFlamePotPosition::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoFlamePotPosition::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_FlamePotPosition );
	queryFact.SetSourceObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		return;
	}

	// Set the Goto state.

	pAI->SetState( kState_Goto );

	// Set the destination node.

	CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();
	pGoto->SetDest( pFact->GetPos() );

	// Set up the trigger state.

	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoFlamePotPosition::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoFlamePotPosition::IsActionComplete( CAI* pAI )
{
	// Allow transitions to finish.

	if( pAI->GetAnimationContext()->IsTransitioning() )
	{
		return false;
	}

	// Goto is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Goto is not complete.

	return false;
}
