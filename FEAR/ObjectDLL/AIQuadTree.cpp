// ----------------------------------------------------------------------- //
//
// MODULE  : AIQuadTree.cpp
//
// PURPOSE : AI QuadTree class implementation.
//
// CREATED : 12/12/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIQuadTree.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"


// Globals

CAIQuadTree* g_pAIQuadTree = NULL;


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIQuadTreeNode
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTreeNode::Con/destructor
//              
//	PURPOSE:	Con/destructor
//              
//----------------------------------------------------------------------------

CAIQuadTreeNode::CAIQuadTreeNode()
{
	m_eQTNodeID = kQTNode_Invalid;

	for( int iNode=0; iNode < 4; ++iNode )
	{
		m_pQTNodeChild[iNode] = NULL;
	}

	m_pNMPolyList = NULL;
	m_cNMPolyListSize = 0;
}

CAIQuadTreeNode::~CAIQuadTreeNode()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTreeNode::FixUpNavMeshPointers
//              
//	PURPOSE:	Fix-up pointers from raw NavMesh data.
//              
//----------------------------------------------------------------------------

void CAIQuadTreeNode::FixUpNavMeshPointers( CAIQuadTreeNode* pAIQuadTreeNodes, ENUM_NMPolyID* pAIQuadTreeNMPolyLists )
{
	// Sanity check.

	if( !( pAIQuadTreeNodes && pAIQuadTreeNMPolyLists ) )
	{
		return;
	}

	// Convert poly list index into pointer.

	uint32 iPolyList = (uint32)m_pNMPolyList;
	m_pNMPolyList = pAIQuadTreeNMPolyLists + iPolyList;

	// Convert child IDs into pointers.

	ENUM_QTNodeID eQTNodeID;
	for( uint32 iChild=0; iChild < 4; ++iChild )
	{
		eQTNodeID = (ENUM_QTNodeID)(uint32)m_pQTNodeChild[iChild];
		if( eQTNodeID == kQTNode_Invalid )
		{
			m_pQTNodeChild[iChild] = NULL;
		}
		else 
		{
			m_pQTNodeChild[iChild] = &( pAIQuadTreeNodes[eQTNodeID] );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTreeNode::GetContainingNMPoly
//              
//	PURPOSE:	Find the NavMesh poly that contains the specified point.
//              
//----------------------------------------------------------------------------

ENUM_NMPolyID CAIQuadTreeNode::GetContainingNMPoly( const LTVector& vPos, uint32 dwCharTypeMask, CAI* pAI )
{
	// The point is not within this QuadTree node.

	if( !m_aabbQTBounds.IntersectPoint( vPos ) )
	{
		return kNMPoly_Invalid;
	}

	// Recurse to check children for containment.

	if( m_pQTNodeChild[0] )
	{
		ENUM_NMPolyID eNMPoly;
		for( int iNode=0; iNode < 4; ++iNode )
		{
			eNMPoly = m_pQTNodeChild[iNode]->GetContainingNMPoly( vPos, dwCharTypeMask );
			if( eNMPoly != kNMPoly_Invalid )
			{
				return eNMPoly;
			}
		}
	}

	// This leaf contains no NavMesh polys.

	if( !m_pNMPolyList )
	{
		return kNMPoly_Invalid;
	}

	// Initially assume none of the polys contain the point.

	ENUM_NMPolyID eBestPoly = kNMPoly_Invalid;

	// Check each poly in leaf for containment.
	// Polys are sorted in decending height, so this will
	// drill down and find the first poly that the point is above,
	// or the lowest poly above the point.

	CAINavMeshPoly* pPoly;
	for( int iPoly=0; iPoly < m_cNMPolyListSize; ++iPoly )
	{
		// Skip poly if it does not exist in NavMesh.

		pPoly = g_pAINavMesh->GetNMPoly( m_pNMPolyList[iPoly] );
		if( !pPoly )
		{
			continue;
		}

		// Skip poly if it does not have the requested character type flags.

		if( !( pPoly->GetNMCharTypeMask() & dwCharTypeMask ) )
		{
			continue;
		}

		// Skip poly if it's associated with a link the AI can't traverse.

		if( !( CAIQuadTree::CanTraverseLink( *pPoly, pAI ) ) )
		{
			continue;
		}

		// Determine if poly contains the point.

		if( pPoly->ContainsPoint2D( vPos ) )
		{
			eBestPoly = pPoly->GetNMPolyID();

			if( pPoly->GetNMPolyAABB()->vMin.y < vPos.y )
			{
				break;
			}
		}
	}

	// Return the best poly, which could be none.

	return eBestPoly;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTreeNode::PrintQTNodes
//              
//	PURPOSE:	Recursively print IDs of nodes.  Useful for 
//              debugging.
//              
//----------------------------------------------------------------------------

void CAIQuadTreeNode::PrintQTNodes()
{
	TRACE( "QTNode: %d\n", m_eQTNodeID );

	if( m_pQTNodeChild[0] )
	{
		for( int iNode=0; iNode < 4; ++iNode )
		{
			m_pQTNodeChild[iNode]->PrintQTNodes();
		}
	}
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//
// CAIQuadTree
//

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTree::Con/destructor
//              
//	PURPOSE:	Con/destructor
//              
//----------------------------------------------------------------------------

CAIQuadTree::CAIQuadTree()
{
	g_pAIQuadTree = this;

	m_pQTNodeRoot = NULL;

	m_bQTInitialized = false;
}

CAIQuadTree::~CAIQuadTree()
{
	TermQuadTree();

	g_pAIQuadTree = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTree::InitQuadTree()
//              
//	PURPOSE:	Initialize QuadTree.
//              
//----------------------------------------------------------------------------

void CAIQuadTree::InitQuadTree()
{
	m_bQTInitialized = true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTree::TermQuadTree()
//              
//	PURPOSE:	Terminate QuadTree.
//              
//----------------------------------------------------------------------------

void CAIQuadTree::TermQuadTree()
{
	m_bQTInitialized = false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTree::GetContainingNMPoly
//              
//	PURPOSE:	Return the NavMesh poly ID for the poly 
//              that contains the point.
//              
//----------------------------------------------------------------------------

ENUM_NMPolyID CAIQuadTree::GetContainingNMPoly( const LTVector& vPos, uint32 dwCharTypeMask, ENUM_NMPolyID ePolyHint, CAI* pAI )
{
	// Insure the quadtree is valid when this is called, or we may crash.  
	// The tree is not deallocated/nullified between level loads.

	if ( !m_bQTInitialized )
	{
		AIASSERT( 0, NULL, "CAIQuadTree::GetContainingNMPoly : Quad tree is not valid." );
		return kNMPoly_Invalid;
	}

  	// First try the hint.

  	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePolyHint );
  	if( pPoly )
  	{
  		// The hint poly still contains the point.
  
  		if( ( pPoly->GetNMCharTypeMask() & dwCharTypeMask ) &&
			pPoly->ContainsPoint2D( vPos ) && 
			CanTraverseLink(*pPoly, pAI) )
  		{
			return ePolyHint;
  		}
  
  		// Try the hint poly's neighbors.
  
  		CAINavMeshPoly* pNeighbor;
  		int cNeighbors = pPoly->GetNumNMPolyEdges();
  		for( int iNeighbor=0; iNeighbor < cNeighbors; ++iNeighbor )
  		{
  			pNeighbor = pPoly->GetNMPolyNeighborAtEdge( iNeighbor );
  			if( pNeighbor && 
				( pNeighbor->GetNMCharTypeMask() & dwCharTypeMask ) &&
				pNeighbor->ContainsPoint2D( vPos ) &&
				CanTraverseLink(*pNeighbor, pAI) )
  			{
  				return pNeighbor->GetNMPolyID();
  			}
  		}
  	}

	// Search the quad tree for the containing poly.

	if( m_pQTNodeRoot )
	{
		return m_pQTNodeRoot->GetContainingNMPoly( vPos, dwCharTypeMask, pAI );
	}

	return kNMPoly_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIQuadTree::GetContainingNMPoly
//              
//	PURPOSE:	Return the NavMesh poly ID for the poly 
//              that contains the point.
//              
//----------------------------------------------------------------------------

bool CAIQuadTree::CanTraverseLink( const CAINavMeshPoly& poly, CAI* pAI )
{
	if( pAI == NULL )
	{
        return true;
	}

	if( poly.GetNMLinkID() != kNMLink_Invalid )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( poly.GetNMLinkID() );
		if( pLink && ( pLink->GetNMLinkType() == kLink_Player ) )
		{
			return false;
		}
	}

	return true;
}
