// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMountNode.cpp
//
// PURPOSE : AIActionMountNode class implementation
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionMountNode.h"
#include "AI.h"
#include "AIStateUseSmartObject.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMountNode, kAct_MountNode );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountNode::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionMountNode::CAIActionMountNode()
{
	m_dwNodeStatusFlags = kNodeStatus_All & ~kNodeStatus_ThreatLookingAtNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountNode::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionMountNode::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set effects.
	// AI has mounted the node.

	m_wsWorldStateEffects.SetWSProp( kWSK_MountedObject, NULL, kWST_Variable, kWSK_UsingObject );

	// Remove super classes' effect.

	m_wsWorldStateEffects.ClearWSProp( kWSK_UsingObject, NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountNode::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionMountNode::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( pAI->GetState()->GetStateClassType() != kState_UseSmartObject )
	{
		return;
	}

	// Set the Mount Action animProp.

	CAIStateUseSmartObject* pStateUseSmartObject = (CAIStateUseSmartObject*)( pAI->GetState() );
	pStateUseSmartObject->SetProp( kAPG_Action, kAP_ACT_Mount );
	pStateUseSmartObject->SetLooping( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountNode::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionMountNode::ValidateAction( CAI* pAI )
{
	// Intentionally do NOT call super::ValidateAction.
	// We don't care if the node is invalid.

	if( !CAIActionAbstract::ValidateAction( pAI ) )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMountNode::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionMountNode::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
}

