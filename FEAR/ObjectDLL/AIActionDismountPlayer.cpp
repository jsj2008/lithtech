// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountPlayer.cpp
//
// PURPOSE : 
//
// CREATED : 8/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionDismountPlayer.h"
#include "AIStateAnimate.h"
#include "AI.h"
#include "AIDB.h"
#include "PlayerObj.h"

LINKFROM_MODULE(AIActionDismountPlayer);


DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountPlayer, kAct_DismountPlayer );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountPlayer::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionDismountPlayer::CAIActionDismountPlayer()
{
}

CAIActionDismountPlayer::~CAIActionDismountPlayer()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountPlayer::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountPlayer::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// AI has dismounted the player.

	m_wsWorldStateEffects.SetWSProp( kWSK_MountedObject, NULL, kWST_HOBJECT, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountPlayer::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionDismountPlayer::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if the object being used is not a Player

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountPlayer::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountPlayer::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Fail if there is not smartobject record for this action.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	AIASSERT( pSmartObjectRecord, pAI->GetHOBJECT(), "CAIActionDismountPlayer::ActivateAction : No SmartObjectRecord specified.");
	if ( !pSmartObjectRecord )
	{
		return;
	}

	// Set the state to animate, and configure the state.

	pAI->SetState( kState_Animate );
	CAIStateAnimate* pStateAnimate = (CAIStateAnimate*)( pAI->GetState() );
	pStateAnimate->SetAnimation( pSmartObjectRecord->Props, !LOOP );

	// Notify the player that they are released.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact )
	{
		// Release the player.

		CPlayerObj* pPlayer = CPlayerObj::DynamicCast( pFact->GetTargetObject() );
		if ( pPlayer )
		{
			pPlayer->BerserkerAbort( pAI->GetHOBJECT() );
		}

		// Remove the fact.

		pAI->GetAIWorkingMemory()->ClearWMFact( pFact );

		AITRACE( AIShowBerserker, ( pAI->GetHOBJECT(), "(Dismounted) Kick count: %d", 0 ) );
	}

	// Remove all of the kicked facts, as kicks should not persist across 
	// berserker sessions.

	CAIWMFact queryKickedFact;
	queryKickedFact.SetFactType( kFact_Knowledge );
	queryKickedFact.SetKnowledgeType( kKnowledge_BerserkerKicked );
	pAI->GetAIWorkingMemory()->ClearWMFacts( queryKickedFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountPlayer::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionDismountPlayer::IsActionComplete( CAI* pAI )
{
	// Dismount is complete when the animation finishes.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Dismount is not complete.

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionDismountPlayer::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionDismountPlayer::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

