// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionBerserkerRecoil.cpp
//
// PURPOSE : 
//
// CREATED : 3/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionBerserkerRecoil.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionBerserkerRecoil, kAct_BerserkerRecoil );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoil::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionBerserkerRecoil::CAIActionBerserkerRecoil()
{
}

CAIActionBerserkerRecoil::~CAIActionBerserkerRecoil()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoil::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionBerserkerRecoil::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// Reacted to being kicked during a berserker attack.

	m_wsWorldStateEffects.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_BerserkerKicked );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoil::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionBerserkerRecoil::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
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
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIActionBerserkerRecoil::ValidateContextPreconditions: Action does not define a smartobject; AI cannot respond to being kicked." );
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

	// Success!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoil::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionBerserkerRecoil::ActivateAction(CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );

	// Set state state.

	pAI->SetState( kState_UseSmartObject );
	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Decrement the kick count.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	pFact->SetIndex( pFact->GetIndex() - 1 );

	AITRACE( AIShowBerserker, ( pAI->GetHOBJECT(), "(Recoiling) Kick count: %d", pFact->GetIndex() ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionBerserkerRecoil::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionBerserkerRecoil::IsActionComplete( CAI* pAI )
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
