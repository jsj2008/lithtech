// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoAbstract.cpp
//
// PURPOSE : AIActionGotoAbstract abstract class implementation
//
// CREATED : 10/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoAbstract.h"
#include "AI.h"
#include "AIDB.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavMesh.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoAbstract::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoAbstract::CAIActionGotoAbstract()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoAbstract::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoAbstract::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// Set preconditions.
	// Must not be mounting a node.

	m_wsWorldStatePreconditions.SetWSProp( kWSK_MountedObject, NULL, kWST_HOBJECT, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoAbstract::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoAbstract::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Auto-reload while running.

	pAI->GetAIBlackBoard()->SetBBAutoReload( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoAbstract::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoAbstract::DeactivateAction( CAI* pAI )
{
	// Turn off automatic reloading.

	pAI->GetAIBlackBoard()->SetBBAutoReload( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoAbstract::ValidateAction
//
//	PURPOSE:	Returns true if the link is valid for walking through, 
//				returns false if it is not.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoAbstract::ValidateAction( CAI* pAI )
{
	if (!super::ValidateAction(pAI))
	{
		return false;
	}

	// Invalidate the AIs plan if the link they are currently in is invalid,
	// as they need to get out of it or do something different.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetAIBlackBoard()->GetBBNextNMLink() );
	if( pLink && !pLink->IsLinkValid( pAI, GetActionRecord()->eActionType, !TRAVERSAL_IN_PROGRESS ))
	{
		// Clear out knowledge regarding paths, as this path is invalid.  If we
		// do not clear it, we will be attempting to generate a path with invalid
		// state information

//		FactDescription factDesc;
//		factDesc.SetFactType(kFact_PathInfo);
//		pAI->GetAIWorkingMemory()->ClearWMFact(factDesc);

		return false;
	}

	return true;
}
