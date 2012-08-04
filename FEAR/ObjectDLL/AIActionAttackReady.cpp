// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackReady.cpp
//
// PURPOSE : AIActionAttackReady class implementation
//
// CREATED : 11/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackReady.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackReady, kAct_AttackReady );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackReady::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackReady::CAIActionAttackReady()
{
	m_bValidateVisibility = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackReady::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackReady::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Don't go ready if target is within melee range.

	if( AIWeaponUtils::HasWeaponType( pAI, kAIWeaponType_Melee, bIsPlanning ) &&
		AIWeaponUtils::IsInRange( pAI, kAIWeaponType_Melee, bIsPlanning ) )
	{
		return false;
	}

	// Do not wait ready if AI has never seen the target.

	if( pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() == 0.f )
	{
		return false;
	}

	// Do not wait ready if AI can see the target.

	if( pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	// Do not wait ready unless AI has seen his target somewhat recently.

	if( pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() < g_pLTServer->GetTime() - 3.f * 60.f )
	{
		return false;
	}

	// Only wait ready when in an area that has potential cover,
	// or when within some distance of the threat.

	bool bCanWaitReady = false;

	// Potential cover is available.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Node );
	factQuery.SetNodeType( kNode_Cover );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( pFact )
	{
		bCanWaitReady = true;
	}

	// Threat is close enough to wait ready.

	else
	{
		LTVector vTargetPos = pAI->GetAIBlackBoard()->GetBBTargetPosition();
		float fDistSqr = vTargetPos.DistSqr( pAI->GetPosition() );
		if( fDistSqr < g_pAIDB->GetAIConstantsRecord()->fHoldPositionDistanceSqr )
		{
			bCanWaitReady = true;
		}
	}

	return bCanWaitReady;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackReady::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackReady::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackReady::SetAttackAnimProps
//
//	PURPOSE:	Set animation props.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackReady::SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pAI && pProps ) )
	{
		return;
	}

	super::SetAttackAnimProps( pAI, pProps );

	pProps->Set( kAPG_Posture, kAP_POS_Crouch );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackReady::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackReady::ValidateAction( CAI* pAI )
{
	if( !super::ValidateAction( pAI ) )
	{
		return false;
	}

	// Target is visible.
	// Bail so that we can return to better combat behaviors

	if( pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	// Do not wait ready unless AI has seen his target somewhat recently.

	if( pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() < g_pLTServer->GetTime() - 3.f * 60.f )
	{
		return false;
	}

	// AI checks for knowledge of potential cover.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Node );
	factQuery.SetNodeType( kNode_Cover );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// AI does NOT know of potential cover.

	if( !pFact ) 
	{
		LTVector vTargetPos = pAI->GetAIBlackBoard()->GetBBTargetPosition();
		float fDistSqr = vTargetPos.DistSqr( pAI->GetPosition() );
		float fThreshold = 1.1f * g_pAIDB->GetAIConstantsRecord()->fHoldPositionDistance;

		// Threat has gotten too far away to keep waiting ready.

		if( fDistSqr > fThreshold * fThreshold )
		{
			return false;
		}
	}

	// Keep waiting ready.

	return true;
}


