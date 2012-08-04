// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarNavMeshSafe.cpp
//
// PURPOSE : AStar classes for finding the safest paths on a NavMesh.
//
// CREATED : 11/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIAStarNavMeshSafe.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarGoalNavMeshSafe
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMeshSafe::Con/Destructor
//              
//	PURPOSE:	Construction / Destruction.
//              
//----------------------------------------------------------------------------

CAIAStarGoalNavMeshSafe::CAIAStarGoalNavMeshSafe()
{
	m_eTargetPoly = kNMPoly_Invalid;
}

CAIAStarGoalNavMeshSafe::~CAIAStarGoalNavMeshSafe()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMeshSafe::GetActualCost
//              
//	PURPOSE:	Return the actual cost from one node to another.
//              
//----------------------------------------------------------------------------

float CAIAStarGoalNavMeshSafe::GetActualCost( CAI* pAI, CAIAStarNodeAbstract* pAStarNodeA, CAIAStarNodeAbstract* pAStarNodeB )
{
	float fCost = super::GetActualCost( pAI, pAStarNodeA, pAStarNodeB );

	// Sanity check.

	if( ( m_eTargetPoly == kNMPoly_Invalid ) ||
		( !pAStarNodeB ) )
	{
		return fCost;
	}

	CAIAStarNodeNavMesh* pNodeB = (CAIAStarNodeNavMesh*)pAStarNodeB;
	CAINavMeshPoly* pNMPolyParent = m_pAStarMapNavMesh->GetNMPoly( pNodeB->eAStarNodeID );
	if( pNMPolyParent )
	{
		// Increase cost if poly contains target.

		static float fPenalty = 10000.f;
		if( pNMPolyParent->GetNMPolyID() == m_eTargetPoly )
		{
			fCost *= fPenalty;
		}

		// Increase cost if poly's neighbor contains target.

		else if( pNMPolyParent->GetNMPolyNeighborEdge( m_eTargetPoly ) )
		{
			fCost *= fPenalty;
		}
	}

	// Return the adjusted cost.

	return fCost;
}

