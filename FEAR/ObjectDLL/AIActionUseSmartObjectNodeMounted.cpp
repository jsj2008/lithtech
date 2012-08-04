// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionUseSmartObjectNodeMounted.cpp
//
// PURPOSE : AIActionUseSmartObjectNodeMounted class implementation
//
// CREATED : 04/22/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionUseSmartObjectNodeMounted.h"
#include "AI.h"
#include "AIDB.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionUseSmartObjectNodeMounted, kAct_UseSmartObjectNodeMounted );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNodeMounted::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionUseSmartObjectNodeMounted::CAIActionUseSmartObjectNodeMounted()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNodeMounted::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionUseSmartObjectNodeMounted::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Must have mounted the node.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_MountedObject, NULL, kWST_Variable, kWSK_UsingObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionUseSmartObjectNodeMounted::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionUseSmartObjectNodeMounted::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	if( !super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning ) )
	{
		return false;
	}

	// Only check node if planning.

	if( bIsPlanning )
	{
		// Find which node we are going to from the goal world state.

		SAIWORLDSTATE_PROP* pProp = wsWorldStateGoal.GetWSProp( kWSK_UsingObject, pAI->m_hObject );
		if( !pProp )
		{
			return false;
		}

		// Node's SmartObject must require mounting.

		AINodeSmartObject* pNode = AINodeSmartObject::DynamicCast( pProp->hWSValue );
		if( !( pNode && pNode->GetSmartObject() ) )
		{
			return false;
		}
		AIDB_SmartObjectRecord* pSmartObject = pNode->GetSmartObject();
		if( pSmartObject->Props.Get( kAPG_Posture ) != kAP_POS_Mounted )
		{
			return false;
		}
	}

	return true;
}
