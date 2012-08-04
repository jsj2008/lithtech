// ----------------------------------------------------------------------- //
//
// MODULE  : AIAStarNavMesh.cpp
//
// PURPOSE : AStar Node, Goal, Storage, and Map classes for finding
//           paths on a NavMesh.
//
// CREATED : 12/02/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIAStarNavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AIUtils.h"


// Factories.

DEFINE_AI_FACTORY_CLASS( CAIAStarNodeNavMesh );


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarGoalNavMesh
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMesh::Con/Destructor
//              
//	PURPOSE:	Construction / Destruction.
//              
//----------------------------------------------------------------------------

CAIAStarGoalNavMesh::CAIAStarGoalNavMesh()
{
	m_eAStarNodeDest = kASTARNODE_Invalid;
	m_pAStarMapNavMesh = NULL;
}

CAIAStarGoalNavMesh::~CAIAStarGoalNavMesh()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMesh::GetHeuristicDistance
//              
//	PURPOSE:	Get a best-guess distance from a node to the goal.
//              
//----------------------------------------------------------------------------

float CAIAStarGoalNavMesh::GetHeuristicDistance( CAIAStarNodeAbstract* pAStarNode )
{
	// Use Manhatten distance (sum of the differences in x,y,z) to get a best guess of
	// the distance from a node to the goal.

	// The start point is the center of the poly.

	LTVector vManhattenDist;
	CAINavMeshPoly* pNMPoly = m_pAStarMapNavMesh->GetNMPoly( pAStarNode->eAStarNodeID );
	if( pNMPoly )
	{
		vManhattenDist = pNMPoly->GetNMPolyCenter();
	}

	pNMPoly = m_pAStarMapNavMesh->GetNMPoly( m_eAStarNodeDest );
	if( pNMPoly )
	{
		vManhattenDist -= pNMPoly->GetNMPolyCenter();
	}

	// Get the absolute value of each component.

	if( vManhattenDist.x < 0.f )
	{
		vManhattenDist.x *= -1.f;
	}
	if( vManhattenDist.y < 0.f )
	{
		vManhattenDist.y *= -1.f;
	}
	if( vManhattenDist.z < 0.f )
	{
		vManhattenDist.z *= -1.f;
	}

	// Return the estimate.

	return ( vManhattenDist.x + vManhattenDist.y + vManhattenDist.z );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMesh::GetActualCost
//              
//	PURPOSE:	Return the actual cost from one node to another.
//              
//----------------------------------------------------------------------------

float CAIAStarGoalNavMesh::GetActualCost( CAI* pAI, CAIAStarNodeAbstract* pAStarNodeA, CAIAStarNodeAbstract* pAStarNodeB )
{
	CAIAStarNodeNavMesh* pNodeA = (CAIAStarNodeNavMesh*)pAStarNodeA;
	CAIAStarNodeNavMesh* pNodeB = (CAIAStarNodeNavMesh*)pAStarNodeB;

	// Add the cost multiplier if the poly being entered has a nav mesh link. 

	float flMultiplierWeight = 1.0f;

	CAINavMeshPoly* pNMPolyParent = m_pAStarMapNavMesh->GetNMPoly( pNodeB->eAStarNodeID );
	if (pAI && pNMPolyParent)
	{
		if( pNMPolyParent->GetNMLinkID() != kNMLink_Invalid )
		{
			AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pNMPolyParent->GetNMLinkID() );
			if (pLink)
			{
				flMultiplierWeight = pLink->GetNMLinkPathingWeight(pAI);
				AIASSERT(flMultiplierWeight >= 1.0, NULL, "");
			}
		}
	}

	// Set the true distance from nodeA to node B.

	return flMultiplierWeight * pNodeA->vTrueEntryPos.Dist( pNodeB->vPotentialEntryPos );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarGoalNavMesh::IsAStarFinished
//              
//	PURPOSE:	Return true if node is null, or matches the destination.
//              
//----------------------------------------------------------------------------

bool CAIAStarGoalNavMesh::IsAStarFinished( CAIAStarNodeAbstract* pAStarNode )
{
	// AStar is finished, because there are no more nodes.

	if( !pAStarNode )
	{
		return true;
	}

	// AStar is finished because the goal has been reached.

	if( pAStarNode->eAStarNodeID == m_eAStarNodeDest )
	{
		return true;
	}

	// AStar is not finished.

	return false;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarStorageNavMesh
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageNavMesh::CreateAStarNode
//              
//	PURPOSE:	Create an AStarNode with a specified ID. 
//              
//----------------------------------------------------------------------------

CAIAStarNodeAbstract* CAIAStarStorageNavMesh::CreateAStarNode( ENUM_AStarNodeID eAStarNode )
{
	// Create a node with a specified AStarNodeID.

	CAIAStarNodeNavMesh* pNode = AI_FACTORY_NEW( CAIAStarNodeNavMesh );
	pNode->eAStarNodeID = eAStarNode;

	return pNode;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarStorageNavMesh::DestroyAStarNode
//              
//	PURPOSE:	Destroy an AStarNode. 
//              
//----------------------------------------------------------------------------

void CAIAStarStorageNavMesh::DestroyAStarNode( CAIAStarNodeAbstract* pAStarNode )
{
	AI_FACTORY_DELETE( pAStarNode );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarMapNavMesh
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::Con/Destructor
//              
//	PURPOSE:	Construction / Destruction.
//              
//----------------------------------------------------------------------------

CAIAStarMapNavMesh::CAIAStarMapNavMesh()
{
	m_pNavMesh = NULL;
	m_pAStarFlags = NULL;
	m_nAStarFlags = 0;
}

CAIAStarMapNavMesh::~CAIAStarMapNavMesh()
{
	debug_deletea( m_pAStarFlags );
	m_nAStarFlags = 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::InitAStarMapNavMesh
//              
//	PURPOSE:	Initialize map by setting pointer to NavMesh. 
//              
//----------------------------------------------------------------------------

void CAIAStarMapNavMesh::InitAStarMapNavMesh( CAINavMesh* pNavMesh )
{
	// Store off a pointer to the nav mesh.

	m_pNavMesh = pNavMesh;

	// Create a flags array as big as the number of NavMesh polys.

	AIASSERT( m_pAStarFlags == NULL, NULL, "CAIAStarMapNavMesh::InitAStarMapNavMesh : Failed to destroy m_pAStarFlags before re-initializing the CAIAStarMapNavMesh." );

	m_nAStarFlags = m_pNavMesh->GetNumNMPolys();
	m_pAStarFlags = debug_newa( unsigned char, m_nAStarFlags );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::TermAStarMapNavMesh
//              
//	PURPOSE:	Releases resources owned by the map, resetting it to its 
//				initial state.
//              
//----------------------------------------------------------------------------

void CAIAStarMapNavMesh::TermAStarMapNavMesh( )
{
	// Delete the Create a flags array as big as the number of NavMesh polys.

	if( m_pAStarFlags )
	{
		debug_deletea( m_pAStarFlags );
		m_pAStarFlags = NULL;

		m_nAStarFlags = 0;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::InitAStarMapNavMeshSearch
//              
//	PURPOSE:	Initialize the map for a search. 
//              
//----------------------------------------------------------------------------

void CAIAStarMapNavMesh::InitAStarMapNavMeshSearch( uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest )
{
	m_dwCharTypeMask = dwCharTypeMask;

	m_vPathSource = vSource;
	m_vPathDest = vDest;

	m_nAStarFlags = m_pNavMesh->GetNumNMPolys();
	memset( m_pAStarFlags, kASTAR_Unchecked, m_nAStarFlags );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::ConvertID_AStarNode2NMPoly
//	ROUTINE:	CAIAStarMapNavMesh::ConvertID_NMPoly2AStarNode
//              
//	PURPOSE:	Convert between AStarNodeIDs and NMPolyIDs.
//              These functions are inlined in the header.
//              The AStarMapNavMesh directly casts the IDs between the enums.
//              Other AStarMaps may not have a 1 to 1 conversion between IDs.
//              For example, if the map is too big to allocate space for all 
//              of the flags, IDs would be converted to fit within a window as 
//              big as the search space would  
//              
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::GetNumAStarNeighbors
//              
//	PURPOSE:	Return number of neighbors for an AStarNode.
//              This total will include edges that do not actually
//              neighbor anyone.
//              
//----------------------------------------------------------------------------

int CAIAStarMapNavMesh::GetNumAStarNeighbors( CAI* pAI, CAIAStarNodeAbstract* pAStarNode )
{
	// Find NavMeshPoly corresponding to node.

	CAINavMeshPoly* pNMPoly = GetNMPoly( pAStarNode->eAStarNodeID );
	if( pNMPoly )
	{
		// The Link determines the number of neighbors.

		if( pAI && pNMPoly->GetNMLinkID() != kNMLink_Invalid )
		{
			AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pNMPoly->GetNMLinkID() );
			if( pLink )
			{
				int cNeighbors;
				if( pLink->GetNumNMLinkNeighbors( &cNeighbors ) )
				{
					return cNeighbors;
				}
			}
		}

		// Return number of node neighbors.
		
		return pNMPoly->GetNumNMPolyEdges();
	}

	// No corresponding NavMeshPoly was found.

	return 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::GetAStarNeighbor
//              
//	PURPOSE:	Return the node ID of a specified neighbor.
//              If neighbor is an edge with no adjacent poly, return Invalid.
//              
//----------------------------------------------------------------------------

ENUM_AStarNodeID CAIAStarMapNavMesh::GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage )
{
	// Find NavMeshPoly corresponding to node.

	CAINavMeshPoly* pNMPoly = GetNMPoly( pAStarNode->eAStarNodeID );
	CAINavMeshPoly* pNMPolyNeighbor = NULL;

	// No corresponding poly.

	if( !pNMPoly )
	{
		return kASTARNODE_Invalid;
	}

	// The Link determines the neighboring poly.

	bool bSetNeighbor = false;
	if( pAI && pNMPoly->GetNMLinkID() != kNMLink_Invalid )
	{
		ENUM_NMLinkID eLink = pNMPoly->GetNMLinkID();
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( eLink );
		if( pLink )
		{
			// Check this Link, because paths are searched from dest to source.

			if( !pLink->IsNMLinkEnabledToAI( pAI, LINK_CHECK_TIMEOUT ) )
			{
				return kASTARNODE_Invalid;
			}

			CAINavMeshPoly* pNMPolyParent = NULL;
			if( pAStarNode->pAStarParent )
			{
				pNMPolyParent = GetNMPoly( pAStarNode->pAStarParent->eAStarNodeID );
			}
			bSetNeighbor = pLink->GetNMLinkNeighborAtEdge( this, iNeighbor, pNMPolyParent, pNMPolyNeighbor );
		}
	}

	// Find a neighboring poly at the specified edge index.

	if( !bSetNeighbor )
	{
		pNMPolyNeighbor = pNMPoly->GetNMPolyNeighborAtEdge( iNeighbor );
	}

	// No neighbor poly exists on the edge with the specified index.

	if( !pNMPolyNeighbor )
	{
		return kASTARNODE_Invalid;
	}

	// Neighbor does not have matching character type restrictions.

	if( !( pNMPolyNeighbor->GetNMCharTypeMask() & m_dwCharTypeMask ) )
	{
		return kASTARNODE_Invalid;
	}

	// The poly's NavMeshLink may prevent the poly from being passable.

	ENUM_NMLinkID eNeighborLink = pNMPolyNeighbor->GetNMLinkID();
	if( pAI && eNeighborLink != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pNeighborLink = g_pAINavMesh->GetNMLink( eNeighborLink );
		if( pNeighborLink )
		{
			if( !pNeighborLink->IsLinkPassable( pAI, pNMPoly->GetNMPolyID() ) )
			{
				return kASTARNODE_Invalid;
			}
		}
	}

	// Poly is passable, so return its node ID.

	return ConvertID_NMPoly2AStarNode( pNMPolyNeighbor->GetNMPolyID() );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::SetupPotentialNeighbor
//              
//	PURPOSE:	Setup parent child relationship.
//              
//----------------------------------------------------------------------------

void CAIAStarMapNavMesh::SetupPotentialNeighbor( CAIAStarNodeAbstract* pAStarNodeParent, CAIAStarNodeAbstract* pAStarNodeChild )
{
	CAIAStarNodeNavMesh* pNodeParent = (CAIAStarNodeNavMesh*)pAStarNodeParent;
	CAIAStarNodeNavMesh* pNodeChild = (CAIAStarNodeNavMesh*)pAStarNodeChild;

	// Child has no parent.

	if( !pNodeParent )
	{
		pNodeChild->vPotentialEntryPos = m_vPathSource;
		pNodeChild->vTrueEntryPos = m_vPathSource;
		return;
	}

	// Can't find parent.

	CAINavMeshPoly* pNMPolyParent = GetNMPoly( pNodeParent->eAStarNodeID );
	if( !pNMPolyParent )
	{
		pNodeChild->vPotentialEntryPos = m_vPathSource;
		pNodeChild->vTrueEntryPos = m_vPathSource;
		return;
	}

	// Parent is a NavMeshLink. Let the Link set the entry pos.

	ENUM_NMPolyID eChild = ConvertID_AStarNode2NMPoly( pNodeChild->eAStarNodeID );
	if( pNMPolyParent && ( pNMPolyParent->GetNMLinkID() != kNMLink_Invalid ) )
	{
		ENUM_NMLinkID eLink = pNMPolyParent->GetNMLinkID();
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( eLink );
		if( pLink )
		{
			LTVector vEntry;
			if( pLink->GetNMLinkOffsetEntryPos( eChild, pNodeParent->vTrueEntryPos, &vEntry ) )
			{
				pNodeChild->vPotentialEntryPos = vEntry;
				return;
			}
		}
	}

	// Default behavior.

	CAINavMeshEdge* pEdge = pNMPolyParent->GetNMPolyNeighborEdge( eChild );
	if( pEdge )
	{
		pNodeChild->vPotentialEntryPos = pEdge->GetNMEdgeMidPt();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::FinalizeNeighbor
//              
//	PURPOSE:	Finalize parent child relationship.
//              
//----------------------------------------------------------------------------

void CAIAStarMapNavMesh::FinalizeNeighbor( CAIAStarNodeAbstract* pAStarNodeChild )
{
	CAIAStarNodeNavMesh* pNodeChild = (CAIAStarNodeNavMesh*)pAStarNodeChild;
	pNodeChild->vTrueEntryPos = pNodeChild->vPotentialEntryPos;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::GetNMEdgeMidPt
//              
//	PURPOSE:	Get the midpoint of a specified NavMesh Edge.
//              
//----------------------------------------------------------------------------

bool CAIAStarMapNavMesh::GetNMEdgeMidPt( ENUM_AStarNodeID eAStarNodeA, ENUM_AStarNodeID eAStarNodeB, LTVector* pvMidPt )
{
	// Find NavMeshPoly corresponding to nodeA.

	CAINavMeshPoly* pNMPolyA = GetNMPoly( eAStarNodeA );
	if( !pNMPolyA )
	{
		return false;
	}

	// Find NavMeshEdge between nodeA and nodeB.

	CAINavMeshEdge*	pEdge = pNMPolyA->GetNMPolyNeighborEdge( ConvertID_AStarNode2NMPoly( eAStarNodeB ) );
	if( pEdge )
	{
		// Return the mid-point of the edge.

		*pvMidPt = pEdge->GetNMEdgeMidPt();
		return true;
	}

	// No edge exists between nodeA and nodeB.

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::SetAStarFlags
//              
//	PURPOSE:	Set AStar flags for a specified node.
//              
//----------------------------------------------------------------------------

void CAIAStarMapNavMesh::SetAStarFlags( ENUM_AStarNodeID eAStarNode, unsigned long dwFlags, unsigned long dwMask )
{
	// Set flags if nodeID is valid.

	if( ( eAStarNode != kASTARNODE_Invalid ) &&
		( eAStarNode < m_pNavMesh->GetNumNMPolys() ) )
	{
		m_pAStarFlags[eAStarNode] &= ~dwMask;
		m_pAStarFlags[eAStarNode] |= dwFlags;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMesh::GetAStarFlags
//              
//	PURPOSE:	Get AStar flags for a specified node.
//              
//----------------------------------------------------------------------------

unsigned long CAIAStarMapNavMesh::GetAStarFlags( ENUM_AStarNodeID eAStarNode )
{
	// Return flags if nodeID is valid.

	if( ( eAStarNode != kASTARNODE_Invalid ) &&
		( eAStarNode < m_pNavMesh->GetNumNMPolys() ) )
	{
		return m_pAStarFlags[eAStarNode];
	}

	return kASTAR_Unchecked;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIAStarMapNavMeshStraightPath
//

CAIAStarMapNavMeshStraightPath::CAIAStarMapNavMeshStraightPath()
{
	m_fPathRadius = 0.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMeshStraightPath::InitAStarMapNavMesh
//              
//	PURPOSE:	Initialize map by setting pointer to NavMesh,
//              and the start and end points of the straight line.
//              
//----------------------------------------------------------------------------

void CAIAStarMapNavMeshStraightPath::InitAStarMapNavMeshSearch( uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, float fRadius )
{
	super::InitAStarMapNavMeshSearch( dwCharTypeMask, vSource, vDest );

	m_fPathRadius = fRadius; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIAStarMapNavMeshStraightPath::GetAStarNeighbor
//              
//	PURPOSE:	Return the node ID of a specified neighbor.
//              If neighbor is an edge with no adjacent poly, return Invalid.
//              
//----------------------------------------------------------------------------

ENUM_AStarNodeID CAIAStarMapNavMeshStraightPath::GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage )
{
	// Superclass checks if there is any neighbor at all.

	ENUM_AStarNodeID eNodeNeighbor = super::GetAStarNeighbor( pAI, pAStarNode, iNeighbor, pAStarStorage );
	if( eNodeNeighbor == kASTARNODE_Invalid )
	{
		return kASTARNODE_Invalid;
	}

	// Get the poly referred to by the current A* node.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ConvertID_AStarNode2NMPoly( pAStarNode->eAStarNodeID ) );
	if( !pPoly )
	{
		return kASTARNODE_Invalid;
	}

	// Get the poly referred to by the neighbor A* node.

	CAINavMeshPoly* pPolyNeighbor = g_pAINavMesh->GetNMPoly( ConvertID_AStarNode2NMPoly( eNodeNeighbor ) );
	if( !pPolyNeighbor )
	{
		return kASTARNODE_Invalid;
	}

	// If neighbor poly has an associated NavMeshLink, do not allow it as 
	// a neighbor for a straight path.

	if( pPolyNeighbor->GetNMLinkID() != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPolyNeighbor->GetNMLinkID() );
		if( !( pLink && pLink->AllowStraightPaths() && pLink->GetNMLinkEnabled() ) )
		{
			return kASTARNODE_Invalid;
		}
	}

	// Neighbor does not have matching character type restrictions.

	if( !( pPolyNeighbor->GetNMCharTypeMask() & m_dwCharTypeMask ) )
	{
		return kASTARNODE_Invalid;
	}

	// Get the edge between the poly and its neighbor.

	CAINavMeshEdge* pEdge = pPoly->GetNMPolyNeighborEdge( ConvertID_AStarNode2NMPoly( eNodeNeighbor ) );
	if( !pEdge )
	{
		return kASTARNODE_Invalid;
	}

	// Trim the edge's line segment to the radius of the path (the radius of the AI).

	LTVector v0 = pEdge->GetNMEdge0();
	LTVector v1 = pEdge->GetNMEdge1();
	TrimLineSegmentByRadius( m_fPathRadius, &v0, &v1, pEdge->IsNMBorderVert0(), pEdge->IsNMBorderVert1() );

	// Determine of the straight path from source to dest intersects the trimmed edge's line segment.

	LTVector vPtIntersect;
	if( kRayIntersect_Failure != RayIntersectLineSegment( v0, v1, m_vPathSource, m_vPathDest, false, &vPtIntersect ) )
	{
		return eNodeNeighbor;
	}

	return kASTARNODE_Invalid;
}



