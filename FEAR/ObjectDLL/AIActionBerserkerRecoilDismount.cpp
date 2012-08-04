// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionBerserkerRecoilDismount.cpp
//
// PURPOSE : 
//
// CREATED : 8/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionBerserkerRecoilDismount.h"
#include "AI.h"
#include "AIWorldState.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionBerserkerRecoilDismount, kAct_BerserkerRecoilDismount );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoilDismount::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionBerserkerRecoilDismount::CAIActionBerserkerRecoilDismount()
{
}

CAIActionBerserkerRecoilDismount::~CAIActionBerserkerRecoilDismount()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoilDismount::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionBerserkerRecoilDismount::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Reacted to being kicked during a berserker attack.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_BerserkerKicked );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoilDismount::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionBerserkerRecoilDismount::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if ( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Fail if there is no smartobject (one is required, so assert if there
	// isn't one)

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if ( !pSmartObjectRecord )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionBerserkerRecoilDismount::ValidateContextPreconditions: Action does not define a smartobject; AI cannot respond to being kicked." );
		return false;
	}

	// Fail if the AI is not attached to a player.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( !pFact )
	{
		return false;
	}

	// Fail if the AI has not been kicked enough times to be knocked off yet.
	// The index we are comparing against needs to be kept in sync with the 
	// index specifying the bodystate, so that the player and AI agree on 
	// when the next kick will be a separation

	if ( pFact->GetIndex() > 1 )
	{
		return false;
	}

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoilDismount::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionBerserkerRecoilDismount::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );

	// Set state state.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Clean up the player attachment without notification, as the player 
	// aborted the behavior.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( pFact )
	{
		pAI->GetAIWorkingMemory()->ClearWMFact( pFact );
	}
	AITRACE( AIShowBerserker, ( pAI->GetHOBJECT(), "(Dismount) Kick count: %d", 0 ) );

	// Insure the dismount flag is cleared.  When this recoil is executed,
	// the AI starts out mounted but should be dismounted by the end.

	pAI->GetAIWorldState()->SetWSProp( kWSK_MountedObject, pAI->GetHOBJECT(), kWST_HOBJECT, 0 );
	pAI->GetAIWorldState()->SetWSProp( kWSK_UsingObject, pAI->GetHOBJECT(), kWST_HOBJECT, 0 );

	// Remove all of the kicked facts, as kicks should not persist across 
	// berserker sessions.

	CAIWMFact queryKickedFact;
	queryKickedFact.SetFactType( kFact_Knowledge );
	queryKickedFact.SetKnowledgeType( kKnowledge_BerserkerKicked );
	pAI->GetAIWorkingMemory()->ClearWMFacts( queryKickedFact );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoilDismount::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionBerserkerRecoilDismount::IsActionComplete( CAI* pAI )
{
	// Drawing is complete when state has completed.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Drawing is not complete.

	return false;
}
