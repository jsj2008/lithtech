// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectInterest.cpp
//
// PURPOSE : AITargetSelectInterest class definition
//
// CREATED : 10/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AITargetSelectInterest.h"
#include "AINodeInterest.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectInterest, kTargetSelect_Interest );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectInterest::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectInterest::ValidatePreconditions( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateContextPreconditions.
	// Target is based on knowledge of Interest nodes.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// AI does not know of any Interest nodes.

	if( !pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, kNode_Interest, kNodeStatus_All, NULL, NULL ) )
	{
		return false;
	}

	// Preconditions are met.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectInterest::Activate
//
//	PURPOSE:	Activate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectInterest::Activate( CAI* pAI )
{
	// Intentionally do NOT call super::Activate.
	// Target is based on knowledge of Interest nodes.
	//
	// Be sure to call down to the base class however to insure that any 
	// awareness modifications are applied.
	CAITargetSelectAbstract::Activate( pAI );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// AI does not know of any Interest nodes.

	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindFactNodeMax( pAI, kNode_Interest, kNodeStatus_All, NULL, NULL );
	if( !pFact )
	{
		return;
	}

	// Bail if the AI is already targeting this object.

	HOBJECT hNode = pFact->GetTargetObject();
	if( !( hNode && hNode != pAI->GetAIBlackBoard()->GetBBTargetObject() ) )
	{
		return;
	}
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );

	// Find or create a working memory fact for this target object.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Object );
	factQuery.SetTargetObject( hNode );
	CAIWMFact* pFactTarget = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFactTarget )
	{
		pFactTarget = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Object );
		pFactTarget->SetTargetObject( hNode, 1.f );

		pFactTarget->SetStimulus( kStim_InvalidType, kStimID_Invalid, 0.f );

		pFactTarget->SetPos( pNode->GetPos(), 1.f );
		pFactTarget->SetRadius( 0.f, 1.f );
	}

	// Target the object.

	TargetObject( pAI, pFactTarget );

	// Target a point of interest.

	pAI->GetAIBlackBoard()->SetBBTargetType( kTarget_Interest );

	// Lock the node so no one else targets it.

	pNode->LockNode( pAI->m_hObject );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectInterest::Deactivate
//
//	PURPOSE:	Deactivate selector.
//
// ----------------------------------------------------------------------- //

void CAITargetSelectInterest::Deactivate( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}


	HOBJECT hNode = pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( !IsAINode( hNode ) )
	{
		return;
	}

	// Unlock the node so other AI may target it.
	
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
	pNode->UnlockNode( pAI->m_hObject );

	// Reset the activation time to prevent another AI from
	// looking at this node too soon.

	pNode->ResetActivationTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITargetSelectInterest::Validate
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAITargetSelectInterest::Validate( CAI* pAI )
{
	// Intentionally do NOT call super::Validate.
	// We do not care of the target is out of view.

	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// No longer targeting the interest node!

	HOBJECT hNode = pAI->GetAIBlackBoard()->GetBBTargetObject();
	if( !IsAINode( hNode ) )
	{
		return false;
	}
	AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hNode );
	if( pNode->GetType() != kNode_Interest )
	{
		return false;
	}
	AINodeInterest* pNodeInterest = (AINodeInterest*)pNode;


	// If node has an owner, don't timeout until reaching the owner node.
	// Base timeout on the node arrival time if it is more recent than targeting time.

	bool bCheckTimeout = true;
	double fCheckTime = pAI->GetAIBlackBoard()->GetBBTargetChangeTime();
	if( pNode->GetNodeOwner() )
	{
		// AI is at the node.

		SAIWORLDSTATE_PROP* pProp = pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, pAI->m_hObject );
		if( pProp && pProp->hWSValue && ( pProp->hWSValue == pNode->GetNodeOwner() ) )
		{
			fCheckTime = LTMAX<double>( fCheckTime, pAI->GetAIBlackBoard()->GetBBDestStatusChangeTime() );
		}

		// AI is not yet at the node.

		else {
			bCheckTimeout = false;
		}
	}

	// Node timed out.

	if( bCheckTimeout &&
		( fCheckTime + pNodeInterest->GetLookTime() < g_pLTServer->GetTime() ) )
	{
		return false;
	}

	// Node is invalid.

	if( !pNodeInterest->IsNodeValid( pAI, pAI->GetPosition(), NULL,  kThreatPos_TargetPos, kNodeStatus_All ) )
	{
		return false;
	}

	// Target is still valid.

	return true;
}
