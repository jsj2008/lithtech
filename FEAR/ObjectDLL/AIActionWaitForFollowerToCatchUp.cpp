// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionWaitForFollowerToCatchUp.cpp
//
// PURPOSE : 
//
// CREATED : 4/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionWaitForFollowerToCatchUp.h"
#include "AINodeLead.h"
#include "AIStateUseSmartObject.h"
#include "AISoundMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionWaitForFollowerToCatchUp, kAct_WaitForFollowerToCatchUp );


static bool ExtractFollowerFact( CAI* pAI, HOBJECT& hOutCharacter, AINodeLead*& pOutDestination )
{
	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Task );
	queryFact.SetTaskType( kTask_LeadCharacter );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		return false;
	}

	AINodeLead* pDestination = AINodeLead::DynamicCast( pFact->GetTargetObject() );
	HOBJECT hCharacter = pFact->GetSourceObject();

	if ( IsDeadCharacter( hCharacter )
		|| NULL == pDestination )
	{
		return false;
	}
	hOutCharacter = hCharacter;
	pOutDestination = pDestination;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForFollowerToCatchUp::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionWaitForFollowerToCatchUp::CAIActionWaitForFollowerToCatchUp()
{
}

CAIActionWaitForFollowerToCatchUp::~CAIActionWaitForFollowerToCatchUp()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForFollowerToCatchUp::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionWaitForFollowerToCatchUp::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// AI reacted to follower being out of range.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_FollowerOutOfRange );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForFollowerToCatchUp::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionWaitForFollowerToCatchUp::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Bail if the Action's SmartObject record does not exist.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	AIASSERT(pSmartObjectRecord, pAI->GetHOBJECT(), "AIActionWaitForFollowerToCatchUp action requires a smartobject");
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Fail if there is no valid follower/destination pair.

	HOBJECT hFollower = NULL;
	AINodeLead* pDestination = NULL;
	if ( !ExtractFollowerFact( pAI, hFollower, pDestination ) )
	{
		return false;
	}

	// Fail if the character is inside the resume pathing radius.

	LTVector vFollowerPos;
	g_pLTServer->GetObjectPos( hFollower, &vFollowerPos );
	float flDistanceToFollowerSqr = ( ( pAI->GetPosition() - vFollowerPos ) ).MagSqr();
	if ( pDestination->GetResumePathingRadiusSqr() > flDistanceToFollowerSqr )
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForFollowerToCatchUp::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionWaitForFollowerToCatchUp::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	HOBJECT hFollower = NULL;
	AINodeLead* pDestination = NULL;
	if ( !ExtractFollowerFact( pAI, hFollower, pDestination ) )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionWaitForFollowerToCatchUp::ActivateAction: Failed to find destination or follower." );
		return;
	}

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Play the wait sound.

	g_pAISoundMgr->RequestAISound( pAI->GetHOBJECT(), pDestination->GetAISoundType_LeadWait(), kAISndCat_Always, NULL, 0.f );

	// Set the animation.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Look at and possibility face the follower.

	pAI->GetAIBlackBoard()->SetBBFaceTarget( true );
	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForFollowerToCatchUp::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionWaitForFollowerToCatchUp::DeactivateAction( CAI* pAI )
{
	super::DeactivateAction( pAI );

	if ( IsActionComplete( pAI ) )
	{
		HOBJECT hFollower = NULL;
		AINodeLead* pDestination = NULL;
		if ( ExtractFollowerFact( pAI, hFollower, pDestination ) )
		{
			g_pAISoundMgr->RequestAISound( pAI->GetHOBJECT(), pDestination->GetAISoundType_LeadResume(), kAISndCat_Always, NULL, 0.f );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForFollowerToCatchUp::ValidateAction
//
//	PURPOSE:	Returns true if AI should keep targeting the same target.
//
// ----------------------------------------------------------------------- //

bool CAIActionWaitForFollowerToCatchUp::ValidateAction( CAI* pAI )
{
	// Fail if there is no valid follower/destination pair.

	HOBJECT hFollower = NULL;
	AINodeLead* pDestination = NULL;
	if ( !ExtractFollowerFact( pAI, hFollower, pDestination ) )
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionWaitForFollowerToCatchUp::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionWaitForFollowerToCatchUp::IsActionComplete( CAI* pAI )
{
	HOBJECT hFollower = NULL;
	AINodeLead* pDestination = NULL;
	if ( !ExtractFollowerFact( pAI, hFollower, pDestination ) )
	{
		return true;
	}

	LTVector vFollowerPos;
	g_pLTServer->GetObjectPos( hFollower, &vFollowerPos );
	float flDistanceToFollowerSqr = ( ( pAI->GetPosition() - vFollowerPos ) ).MagSqr();
	if ( pDestination->GetResumePathingRadiusSqr() <= flDistanceToFollowerSqr )
	{
		return false;
	}

	return true;
}
