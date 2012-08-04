// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionBlindFireFromCover.cpp
//
// PURPOSE : AIActionBlindFireFromCover abstract class implementation
//
// CREATED : 02/03/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionBlindFireFromCover.h"
#include "AI.h"
#include "AINodeTypes.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AnimationContext.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionBlindFireFromCover, kAct_BlindFireFromCover );

#define BLIND_FIRE_CUTOFF_TIME	3.f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBlindFireFromCover::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionBlindFireFromCover::CAIActionBlindFireFromCover()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBlindFireFromCover::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionBlindFireFromCover::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Cut off blind firing after some number of seconds.

	if( pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() < g_pLTServer->GetTime() - BLIND_FIRE_CUTOFF_TIME )
	{
		return false;
	}

	// Bail if we have not been damaged since arriving at this node.

	double fNodeArrivalTime = pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime();

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !( pFact && DidDamage(pAI, pFact) && pFact->GetUpdateTime() > fNodeArrivalTime ) )
	{
		return false;
	}

	// Don't start blind firing again until we have seen the target again.

	if( pFact && pFact->GetUpdateTime() < pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() - 2.f )
	{
		return false;
	}

	// Bail if AI has valid cover to go to.

	HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, kNode_Cover, kNodeStatus_All, NULL, hTarget ) )
	{
		return false;
	}

	// Bail if AI has valid ambush to go to.

	if( pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, kNode_Ambush, kNodeStatus_All, NULL, hTarget ) )
	{
		return false;
	}

	// Blind Fire!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBlindFireFromCover::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionBlindFireFromCover::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Blind Fire!

	pAI->GetAIBlackBoard()->SetBBBlindFire( true );

	// Node track where the target was last seen, instead of
	// the actual position.  This prevents the AI from firing at a
	// wall.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerTrackLastVisible( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBlindFireFromCover::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionBlindFireFromCover::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	// Turn off blind firing.

	pAI->GetAIBlackBoard()->SetBBBlindFire( false );

	// Track normally.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerTrackLastVisible( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBlindFireFromCover::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionBlindFireFromCover::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	super::SetAttackAnimProps( pAI, pProps );

	pProps->Set( kAPG_Activity, kAP_ATVT_Blind );
}

