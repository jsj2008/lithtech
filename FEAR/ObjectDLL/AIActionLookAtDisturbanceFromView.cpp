// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLookAtDisturbanceFromView.cpp
//
// PURPOSE : AIActionLookAtDisturbanceFromView class implementation
//
// CREATED : 3/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionLookAtDisturbanceFromView.h"
#include "AINodeTypes.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLookAtDisturbanceFromView, kAct_LookAtDisturbanceFromView );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbanceFromView::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionLookAtDisturbanceFromView::CAIActionLookAtDisturbanceFromView()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbanceFromView::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionLookAtDisturbanceFromView::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions, in addition to those set by AIActionLookAtDisturbance.
	// Must be at a View node.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_AtNodeType, NULL, kWST_EnumAINodeType, kNode_View );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionLookAtDisturbanceFromView::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionLookAtDisturbanceFromView::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// DOESN'T CALL DOWN.
	return true;
}
