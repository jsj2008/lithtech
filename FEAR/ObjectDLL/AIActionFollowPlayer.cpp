// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFollowPlayer.cpp
//
// PURPOSE : AIActionFollowPlayer class implementation
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFollowPlayer.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFollowPlayer, kAct_FollowPlayer );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollowPlayer::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionFollowPlayer::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
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

	// Bail if leader is not a player.

	HOBJECT hLeader = pFact->GetSourceObject();
	if( !IsPlayer( hLeader ) )
	{
		return false;
	}

	// Default validation.

	return super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning );
}

