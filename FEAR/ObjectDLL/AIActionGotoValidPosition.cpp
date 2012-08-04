// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoValidPosition.cpp
//
// PURPOSE : Contains the implementation of the 'be to a valid position' action.
//
// CREATED : 4/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionGotoValidPosition.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIStateGoto.h"
#include "NodeTrackerContext.h"
#include "AINavMesh.h"
#include "ltintersect.h"
#include "AINavMeshLinkAbstract.h"
#include "AIPathMgrNavMesh.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoValidPosition, kAct_GotoValidPosition );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoValidPosition::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionGotoValidPosition::CAIActionGotoValidPosition()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoValidPosition::InitAction
//
//	PURPOSE:	Handle initializing the AIAction, setting the 
//				ActionRecord to use, as well as preconditions and effects.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoValidPosition::InitAction( AIDB_ActionRecord* pActionRecord )
{
	super::InitAction( pActionRecord );

	// No preconditions.

	// Set effects.
	// Position is valid

	m_wsWorldStateEffects.SetWSProp( kWSK_PositionIsValid, NULL, kWST_bool, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoValidPosition::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoValidPosition::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Return false if the baseclass fails.

	if (!super::ValidateContextPreconditions(pAI, wsWorldStateGoal, bIsPlanning))
	{
		return false;
	}

	// Only check path if planning.

	if( !bIsPlanning )
	{
		return true;
	}

	// A path exists to a neighbor which is valid.

	LTVector vDestination;
	if (!GetValidPosition(pAI, vDestination))
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoValidPosition::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoValidPosition::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Set the Goto state.

	pAI->SetState( kState_Goto );

	// Set the destination node.

	CAIStateGoto* pGoto = (CAIStateGoto*)pAI->GetState();

	LTVector vDestination;
	if (GetValidPosition(pAI, vDestination))
	{
		pGoto->SetDest( vDestination );
	}
	else
	{
		AIASSERT(0, pAI->GetHOBJECT(), "CAIActionGotoValidPosition::ActivateAction : Failed to find valid position.");
	}

	// Make the AI alert to cause it to run out of the space

	pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Alert );

	pAI->GetAIBlackBoard()->SetBBFaceTarget( false );
	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoValidPosition::IsActionComplete
//
//	PURPOSE:	Determine if action has completed.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoValidPosition::IsActionComplete( CAI* pAI )
{
	// Goto is complete if state is complete.

	if( !pAI->GetState() )
	{
		AIASSERT(0, pAI->GetHOBJECT(), "CAIActionGotoValidPosition::IsActionComplete : AI has no state.");
		return false;
	}

	// Goto is complete.

	if ( pAI->GetState()->GetStateStatus() == kAIStateStatus_Complete )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoValidPosition::ApplyContextEffect
//
//	PURPOSE:	Apply affects to the real game world.
//
// ----------------------------------------------------------------------- //

void CAIActionGotoValidPosition::ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal )
{
	// Actually apply the planner effects, which is not the 
	// default behavior of an Action running in context.

	// If the link the AI is in is not enabled to him, then he is in an invalid position.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pAI->GetCurrentNavMeshLink() );
	if( !pLink || pLink->IsNMLinkEnabledToAI(pAI, !LINK_CHECK_TIMEOUT))
	{
		ApplyWSEffect( pAI, pwsWorldStateCur, pwsWorldStateGoal );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionGotoValidPosition::GetValidPosition
//
//	PURPOSE:	Returns true if a valid position was found, false if it 
//				one was not.  If a position was found, the position is 
//				returned by parameter; otherwise, the parameter is 
//				unchanged.
//
// ----------------------------------------------------------------------- //

bool CAIActionGotoValidPosition::GetValidPosition(CAI* pAI, LTVector& outValidPosition)
{
	// fail if the AI does not currently have a nav mesh poly.
	// find the nearest nav mesh poly that is not a border
	// find the nearest point on the poly
	// return this point

	//----

	// AI is not in a nav mesh poly.

	ENUM_NMPolyID eID = pAI->GetCurrentNavMeshPoly();
	if ( kNMPoly_Invalid == eID)
	{
		return false;
	}

	// Find the closest non edge poly.

	const LTVector vAIPosition(pAI->GetPosition().x, 0, pAI->GetPosition().z);

	float flBestDistance = FLT_MAX;
	LTVector vBestPosition(0.f, 0.f, 0.f);
	LTVector vBestCenter(0.f, 0.f, 0.f);
	CAINavMeshPoly* pNMPoly = g_pAINavMesh->GetNMPoly( eID );
	int nEdges = pNMPoly->GetNumNMPolyEdges();
	for (int i = 0; i < nEdges; ++i)
	{
		CAINavMeshEdge* pEdge = pNMPoly->GetNMPolyEdge(i);
		if (pEdge)
		{
			if (kNMEdgeType_Border == pEdge->GetNMEdgeType())
			{
				// This edge is a border, skip it.
				continue;
			}

			// Verify that the other poly is valid for entry

			CAINavMeshPoly* pNMOtherPoly = NULL;
			if ( eID == pEdge->GetNMPolyIDA())
			{
				pNMOtherPoly = g_pAINavMesh->GetNMPoly( pEdge->GetNMPolyIDB() );
			}
			else
			{
				pNMOtherPoly = g_pAINavMesh->GetNMPoly( pEdge->GetNMPolyIDA() );
			}

			// Verify that the other link, if there is one, is valid for entry

			if (pNMOtherPoly && (kNMLink_Invalid != pNMOtherPoly->GetNMLinkID()))
			{
				AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink(pNMOtherPoly->GetNMLinkID());
				if (pLink)
				{
					if (!pLink->IsNMLinkEnabledToAI(pAI, !LINK_CHECK_TIMEOUT))
					{
						continue;
					}
				}
			}

			// Find the closest point on the poly

			LTVector vEdge0 = pEdge->GetNMEdge0();
			vEdge0.y = 0.f;

			LTVector vEdge1 = pEdge->GetNMEdge1();
			vEdge1.y = 0.f;

			LTVector vPos;
			float flDummy;
			float flDist = LTIntersect::Point_Segment_DistSqr(vAIPosition, vEdge0, vEdge1, vPos, flDummy);

			// Better exit location found.

			if (flDist < flBestDistance)
			{
				flBestDistance = flDist;
				vBestPosition = vPos;
				vBestCenter = pNMOtherPoly->GetNMPolyCenter();
			}
		}
	}

	// Failed to find a valid position.

	if (FLT_MAX == flBestDistance)
	{
		return false;
	}

	// Return the position (adjust it by moving the height to match the AIs 
	// current elevation, and adjust the point by a fraction in the direction the AI is 
	// moving to insure that he gets out of the disabled link.

	LTVector vDirection = vBestPosition - vAIPosition;
	vDirection.Normalize();
	LTVector vDestination = LTVector(vBestPosition.x, pAI->GetPosition().y, vBestPosition.z) + (vDirection * pAI->GetRadius());
	if( g_pAIPathMgrNavMesh->StraightPathExists( pAI, pAI->GetCharTypeMask(), pAI->GetPosition(), vDestination, pAI->GetLastNavMeshPoly(), pAI->GetRadius() ) )
	{
		outValidPosition = vDestination;
		return true;
	}

	// Failed to move to the ideal location.  Try moving to the center of the
	// poly.  This should always succeed.  If it doesn't, add a fix which will 
	// work to handle this case.
	AIASSERT(g_pAIPathMgrNavMesh->HasPath(pAI, pAI->GetCharTypeMask(), vBestCenter), pAI->GetHOBJECT(), 
		"CAIActionGotoValidPosition::GetValidPosition : Failed to find path to the center of a neighboring poly." );

	outValidPosition = vBestCenter;
	return true;
}

