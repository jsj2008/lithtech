// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTraverseBlockedDoor.cpp
//
// PURPOSE : AIActionTraverseBlockedDoor class implementation
//
// CREATED : 7/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionTraverseBlockedDoor.h"
#include "AINavMeshLinkDoor.h"
#include "AI.h"
#include "AIDB.h"
#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AIState.h"
#include "AIStateUseSmartObject.h"
#include "AIUtils.h"
#include "AITarget.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "AIWorkingMemory.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTraverseBlockedDoor, kAct_TraverseBlockedDoor );

AINavMeshLinkDoor* GetDoorLink(CAI* pAI)
{
	// Is the AI entering a door link.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetCurrentNavMeshLink() );
	if( ( !pLink ) ||
		( pLink->GetNMLinkType() != kLink_Door ) )
	{
		pLink = g_pAINavMesh->GetNMLink(pAI->GetAIBlackBoard()->GetBBEnteringNMLink());
		if( ( !pLink ) ||
			( pLink->GetNMLinkType() != kLink_Door ) )
		{
			return NULL;
		}
	}

	// Is the door link blocked?

	return ( AINavMeshLinkDoor* )pLink;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionTraverseBlockedDoor::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if (!super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// Is the AI entering a door link.

	AINavMeshLinkDoor* pLinkDoor = GetDoorLink(pAI);
	if (!pLinkDoor)
	{
		return false;
	}

	// Is it a rotating door?

	if (!pLinkDoor->IsDoorRotating(pAI))
	{
		return false;
	}

	// Is the door link blocked, or is the AI alert?

	bool bLinkBlocked = pLinkDoor->IsTraversalDoorBlockedToAI(pAI);
	bool bAIAlertAndAbleToRunThrough = pLinkDoor->IsLinkValid(pAI, GetActionRecord()->eActionType, !TRAVERSAL_IN_PROGRESS) && pAI->GetAIBlackBoard()->GetBBAwareness() >= kAware_Suspicious;
	if (!bLinkBlocked && !bAIAlertAndAbleToRunThrough)
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseLink::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionTraverseBlockedDoor::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( pAI->GetState()->GetStateClassType() != kState_UseSmartObject )
	{
		return;
	}

	// Override the default animation with a kick animation.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	if( pStateUseSmartObject )
	{
		pStateUseSmartObject->SetProp( kAPG_Action, kAP_ACT_KickDoor );
	}
}

void CAIActionTraverseBlockedDoor::DeactivateAction(CAI* pAI)
{
	if (!pAI)
	{
		return;
	}

	// Clean up any pushing set for the target

	if (pAI->GetTarget())
	{
		pAI->GetTarget()->SetPushSpeed(0.f);
		pAI->GetTarget()->SetPushMinDist(0.f);
	}

	// If we are done, and if the door is still blocked, then
	// consider it totally blocked for now.
	AINavMeshLinkDoor* pLinkDoor = GetDoorLink(pAI);
	if (pLinkDoor && 
		pLinkDoor->IsTraversalDoorBlocked(pAI))
	{
		// Still blocked?  Then the attempt to open it failed.
		// Change the channel to jammed.
		pLinkDoor->SetDoorBlockedIsJammed(pAI);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionTraverseBlockedDoor::ValidateAction
//
//	PURPOSE:	Blocked door link traversal is only 
//
// ----------------------------------------------------------------------- //

bool CAIActionTraverseBlockedDoor::ValidateAction( CAI* pAI )
{
	if (!super::ValidateAction(pAI))
	{
		return false;
	}

	return true;
}
