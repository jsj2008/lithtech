// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarNavMeshEscape.cpp
//
// PURPOSE : AStar classes for finding escape paths on a NavMesh.
//
// CREATED : 04/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIAStarNavMeshEscape.h"
#include "AI.h"
#include "AINavMeshLinkAbstract.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarGoalNavMeshEscape
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMeshEscape::Con/Destructor
//              
//	PURPOSE:	Construction / Destruction.
//              
//----------------------------------------------------------------------------

CAIAStarGoalNavMeshEscape::CAIAStarGoalNavMeshEscape()
{
	m_pAStarMapNavMeshEscape = NULL;
}

CAIAStarGoalNavMeshEscape::~CAIAStarGoalNavMeshEscape()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMeshEscape::InitAStarGoalNavMesh
//              
//	PURPOSE:	Initialize the goal.
//              
//----------------------------------------------------------------------------

void CAIAStarGoalNavMeshEscape::InitAStarGoalNavMesh( CAIAStarMapNavMesh* pAStarMapNavMesh )
{
	super::InitAStarGoalNavMesh( pAStarMapNavMesh );

	m_pAStarMapNavMeshEscape = (CAIAStarMapNavMeshEscape*)pAStarMapNavMesh;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMeshEscape::GetHeuristicDistance
//              
//	PURPOSE:	Get a best-guess distance from a node to the goal.
//              
//----------------------------------------------------------------------------

float CAIAStarGoalNavMeshEscape::GetHeuristicDistance( CAIAStarNodeAbstract* pAStarNode )
{
	// Sanity check.

	if( !m_pAStarMapNavMeshEscape )
	{
		return FLT_MAX;
	}

	// Check the clearance from the danger to the center of the poly.

	CAINavMeshPoly* pNMPoly = m_pAStarMapNavMesh->GetNMPoly( pAStarNode->eAStarNodeID );
	if( !pNMPoly )
	{
		return FLT_MAX;
	}

	float fDist = pNMPoly->GetNMPolyCenter().Dist( m_pAStarMapNavMeshEscape->GetDanger() );

	// Return the estimate, or zero.
	
	float fDiff = m_pAStarMapNavMeshEscape->GetClearance() - fDist;
	if( fDiff < 0.f )
	{
		return 0.f;
	}
	return fDiff;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMeshEscape::IsAStarFinished
//              
//	PURPOSE:	Return true if node is null, or matches the destination.
//              
//----------------------------------------------------------------------------

bool CAIAStarGoalNavMeshEscape::IsAStarFinished( CAIAStarNodeAbstract* pAStarNode )
{
	// Sanity check.

	if( !m_pAStarMapNavMeshEscape )
	{
		return false;
	}

	// AStar is finished, because there are no more nodes.

	if( !pAStarNode )
	{
		return true;
	}

	// Escape path must consist of more than one node, or the AI won't go anywhere!

	if( !pAStarNode->pAStarParent )
	{
		return false;
	}

	// AStar is finished because the clearance distance has been reached.

	CAINavMeshPoly* pNMPoly = m_pAStarMapNavMesh->GetNMPoly( pAStarNode->eAStarNodeID );
	if( pNMPoly )
	{
		// Link is not a valid destination.

		if( pNMPoly->GetNMLinkID() != kNMLink_Invalid )
		{
			AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pNMPoly->GetNMLinkID() );
			if( pLink && !pLink->IsLinkValidDest() )
			{
				return false;
			}
		}

		// We have reached our clearance distance.

		float fClearance = m_pAStarMapNavMeshEscape->GetClearance();
		float fDistSqr = pNMPoly->GetNMPolyCenter().DistSqr( m_pAStarMapNavMeshEscape->GetDanger() );
		if( fDistSqr >= fClearance * fClearance )
		{
			return true;
		}
	}

	// AStar is not finished.

	return false;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarMapNavMeshEscape
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMeshEscape::Con/Destructor
//              
//	PURPOSE:	Construction / Destruction.
//              
//----------------------------------------------------------------------------

CAIAStarMapNavMeshEscape::CAIAStarMapNavMeshEscape()
{
	m_fClearance = 0.f;
}

CAIAStarMapNavMeshEscape::~CAIAStarMapNavMeshEscape()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMeshEscape::InitAStarMapNavMesh
//              
//	PURPOSE:	Initialize map by setting pointer to NavMesh,
//              and the danger position and required clearance.
//              
//----------------------------------------------------------------------------

void CAIAStarMapNavMeshEscape::InitAStarMapNavMeshSearch( uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDanger, float fClearance )
{
	super::InitAStarMapNavMeshSearch( dwCharTypeMask, vSource, vDanger );

	m_vDanger = vDanger;
	m_fClearance = fClearance; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMeshEscape::GetAStarNeighbor
//              
//	PURPOSE:	Return the node ID of a specified neighbor.
//              If neighbor is an edge with no adjacent poly, return Invalid.
//              
//----------------------------------------------------------------------------

ENUM_AStarNodeID CAIAStarMapNavMeshEscape::GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage )
{
	ENUM_AStarNodeID eNodeID;
	eNodeID = super::GetAStarNeighbor( pAI, pAStarNode, iNeighbor, pAStarStorage );

	// Poly doesn't exist.

	CAINavMeshPoly* pNMPoly = GetNMPoly( eNodeID );
	if( !pNMPoly )
	{
		return kASTARNODE_Invalid;
	}

	// Do not pass the danger position to get clearance!

	LTVector vDangerToAI = pAI->GetPosition() - m_vDanger;
	float fMag = vDangerToAI.Mag( );
	if( fMag < 0.0001f )
	{
		// If danger is right on AI, then it doesn't matter which direction AI goes.
		return eNodeID;
	}

	vDangerToAI /= fMag;

	LTVector vDangerToPoly = pNMPoly->GetNMPolyCenter() - m_vDanger;
	fMag = vDangerToPoly.Mag( );
	if( fMag < 0.0001f )
	{
		// Danger is on poly center.  Don't go that way.
		return kASTARNODE_Invalid;
	}

	vDangerToPoly /= fMag;

	float fDot = vDangerToAI.Dot( vDangerToPoly );
	if( fDot < 0.f )
	{
		return kASTARNODE_Invalid;
	}

	// Poly is a valid neighbor.

	return eNodeID;
}

