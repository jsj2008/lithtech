// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionIdleFidget.cpp
//
// PURPOSE : AIActionIdleFidget class implementation
//
// CREATED : 4/8/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionIdleFidget.h"
#include "AIStateUseSmartObject.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdleFidget, kAct_IdleFidget );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleFidget::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionIdleFidget::CAIActionIdleFidget()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleFidget::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionIdleFidget::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Must not be mounting a node.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_MountedObject, NULL, kWST_HOBJECT, 0 );

	// Set effects.
	// AI is idling.

	m_wsWorldStateEffects.SetWSProp( kWSK_Idling, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionIdleFidget::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionIdleFidget::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Bail if Action does not have an existing SmartObject.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return;
	}

	// Set SmartObject state.

	pAI->SetState( kState_UseSmartObject );

	// Set smartobject.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetSmartObject( pSmartObjectRecord );

	// Head tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

