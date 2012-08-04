// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoTargetLost.cpp
//
// PURPOSE : AIActionGotoTargetLost class implementation
//
// CREATED : 01/18/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoTargetLost.h"
#include "AIQuadTree.h"
#include "AIStateGoto.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoTargetLost, kAct_GotoTargetLost );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTargetLost::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoTargetLost::CAIActionGotoTargetLost()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTargetLost::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoTargetLost::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// At the target's pos.

	m_wsWorldStateEffects.SetWSProp( kWSK_AtTargetPos, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTargetLost::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoTargetLost::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Only check path if planning.

	if( !bIsPlanning )
	{
		return true;
	}

	// This action is only valid once a search has started,
	// and the target has been cleared.

	if( pAI->HasTarget( kTarget_All ) )
	{
		return false;
	}

	// Determine if target is reachable.

	LTVector vDest = pAI->GetAIBlackBoard()->GetBBTargetLostPosition();

	// Already there.

	LTVector vPos = pAI->GetPosition();
	if( ( vPos.x == vDest.x ) &&
		( vPos.z == vDest.z ) )
	{
		// Now do the expensive check to insure dest is in the same poly the
		// AI to take into account the fact that this is a 3D mesh, and that
		// the AI could be directly a floor below the location he wants to 
		// path to.

		// If targeting a character, use the character's last known
		// NavMesh poly as a hint for finding the target's current
		// NavMesh poly.

		ENUM_NMPolyID ePolyHint = kNMPoly_Invalid;
		if( pAI->HasTarget( kTarget_Character ) )
		{
			HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
			CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
			if( pChar )
			{
				ePolyHint = pChar->GetLastNavMeshPoly();
			}
		}

		// Do not go anywhere if the AI is already at the dest.

		ENUM_NMPolyID eLastPoly = pAI->GetLastNavMeshPoly();
		ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly( vDest, pAI->GetCharTypeMask(), ePolyHint, pAI );
		if( ePoly == eLastPoly )
		{
			return false;
		}
	}

	// Return true if a path exists.

	return g_pAIPathMgrNavMesh->HasPath( pAI, pAI->GetCharTypeMask(), vDest );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTarget::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoTargetLost::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	if( pAI->HasTarget( kTarget_All ) )
	{
		return;
	}

	// Set the Goto state.

	pAI->SetState( kState_Goto );
	CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();

	// Set the destination.

	pGoto->SetDest( pAI->GetAIBlackBoard()->GetBBTargetLostPosition() );

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_None );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoTargetLost::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoTargetLost::IsActionComplete( CAI* pAI )
{
	// Goto is complete if state is complete.

	if( ( pAI->GetState() ) &&
		( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete ) )
	{
		return true;
	}

	// Goto is not complete.

	return false;
}

