// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFollowHeavyArmor.cpp
//
// PURPOSE : AIActionFollowHeavyArmor class implementation
//
// CREATED : 4/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionFollowHeavyArmor.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFollowHeavyArmor, kAct_FollowHeavyArmor );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionFollowHeavyArmor::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionFollowHeavyArmor::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
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

	// Bail if follow source does not exist.
	// The source is the leader of orderly advance.

	HOBJECT hLeader = pFact->GetSourceObject();
	if( !IsAI( hLeader ) )
	{
		return false;
	}

	// Bail if AI is not a HeavyArmor.

	CAI* pLeader = (CAI*)g_pLTServer->HandleToObject( hLeader );
	const AIDB_AttributesRecord* pButes = pLeader->GetAIAttributes();
	if( !LTStrIEquals( pButes->strName.c_str(), "HeavyArmor" ) )
	{
		return false;
	}

	// Default validation.

	return super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning );
}

