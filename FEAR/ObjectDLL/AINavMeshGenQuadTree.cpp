// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenQuadTree.cpp
//
// PURPOSE : AI NavMesh generator quad tree class implementation.
//
// CREATED : 11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINavMeshGenQuadTree.h"
#include "AINavMeshGenPoly.h"
#include "AINavMeshGen.h"


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::Con/destructor
//              
//	PURPOSE:	Con/destructor
//              
//----------------------------------------------------------------------------

CAINavMeshGenQuadTreeNode::CAINavMeshGenQuadTreeNode()
{
	for( int iNode=0; iNode < 4; ++iNode )
	{
		m_pNMGQTChildNode[iNode] = NULL;
	}

	m_aabbNMGQTBounds.InitAABB();

	m_nNMGQTLevel = kNMGQTLevel_Invalid;
	m_eNMQTNodeID = kNMGQTNode_Invalid;
}

CAINavMeshGenQuadTreeNode::~CAINavMeshGenQuadTreeNode()
{
	for( int iNode=0; iNode < 4; ++iNode )
	{
		CAINavMeshGen::GetAINavMeshGen()->DestroyNMGQTNode( m_pNMGQTChildNode[iNode] );
	}

	m_aabbNMGQTBounds.InitAABB();
	m_lstPolyRefs.resize( 0 );
	m_lstAINodes.resize( 0 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::InitQuadTreeNode
//              
//	PURPOSE:	Initialize quad tree node with bounding box.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::InitQuadTreeNode( const SAINAVMESHGEN_AABB& aabb, int nLevel )
{
	m_aabbNMGQTBounds = aabb;
	m_nNMGQTLevel = nLevel;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::CreateNMGQTChildren
//              
//	PURPOSE:	Create 4 children for a quad tree node.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::CreateNMGQTChildren()
{
	// Create 4 child nodes.

	for( int iNode=0; iNode < 4; ++iNode )
	{
		m_pNMGQTChildNode[iNode] = CAINavMeshGen::GetAINavMeshGen()->CreateNMGQTNode(); 
	}
	
	// Divide the parent bounding box into 4 quadrants.

	LTVector vCenter = ( m_aabbNMGQTBounds.vMin + m_aabbNMGQTBounds.vMax ) * 0.5f;
	SAINAVMESHGEN_AABB aabb;

	aabb = m_aabbNMGQTBounds;
	aabb.vMin.z = vCenter.z;
	aabb.vMax.x = vCenter.x;
	m_pNMGQTChildNode[kNMGQTDir_NW]->InitQuadTreeNode( aabb, m_nNMGQTLevel - 1 );

	aabb.vMin.x = vCenter.x;
	aabb.vMax.x = m_aabbNMGQTBounds.vMax.x;
	m_pNMGQTChildNode[kNMGQTDir_NE]->InitQuadTreeNode( aabb, m_nNMGQTLevel - 1 );

	aabb = m_aabbNMGQTBounds;
	aabb.vMax.z = vCenter.z;
	aabb.vMax.x = vCenter.x;
	m_pNMGQTChildNode[kNMGQTDir_SW]->InitQuadTreeNode( aabb, m_nNMGQTLevel - 1 );

	aabb.vMin.x = vCenter.x;
	aabb.vMax.x = m_aabbNMGQTBounds.vMax.x;
	m_pNMGQTChildNode[kNMGQTDir_SE]->InitQuadTreeNode( aabb, m_nNMGQTLevel - 1 );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::DestroyNMGQTChildren
//              
//	PURPOSE:	Delete 4 child nodes of a node.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::DestroyNMGQTChildren()
{
	for( int iNode=0; iNode < 4; ++iNode )
	{
		CAINavMeshGen::GetAINavMeshGen()->DestroyNMGQTNode( m_pNMGQTChildNode[iNode] );
		m_pNMGQTChildNode[iNode] = NULL;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::AddNMGPoly
//              
//	PURPOSE:	Add a poly to a quad tree node.
//              This will either add the poly to the node's
//              children, or add it to its list of polys
//              if this node is a leaf node.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::AddNMGPoly( CAINavMeshGenPoly* pPoly )
{
	// Add a poly to the leaf by adding it to the poly list.

	if( m_nNMGQTLevel == kNMGQTLevel_Leaf )
	{
		m_lstPolyRefs.push_back( pPoly->GetNMGPolyID() );
		return;
	}

	// Create children if none exist.

	if( !m_pNMGQTChildNode[0] )
	{
		CreateNMGQTChildren();
	}

	// Add the poly to each child who has an intersecting bounding box.

	SAINAVMESHGEN_AABB* paabb = pPoly->GetAABB();
	for( int iNode=0; iNode < 4; ++iNode )
	{
		if( paabb->IntersectAABB( *( m_pNMGQTChildNode[iNode]->GetAABB() ) ) )
		{
			m_pNMGQTChildNode[iNode]->AddNMGPoly( pPoly );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::RemoveNMGPoly
//              
//	PURPOSE:	Remove a poly from all referencing nodes of the quad tree.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::RemoveNMGPoly( CAINavMeshGenPoly* pPoly )
{
	// Remove a poly from a leaf by erasing the poly from the poly list.

	if( m_nNMGQTLevel == kNMGQTLevel_Leaf )
	{
		NMGPOLY_LIST::iterator itPoly;
		for( itPoly = m_lstPolyRefs.begin(); itPoly != m_lstPolyRefs.end(); ++itPoly )
		{
			if( *itPoly == pPoly->GetNMGPolyID() )
			{
				m_lstPolyRefs.erase( itPoly );
				break;
			}
		}

		return;
	}

	// No children to remove from.

	if( !m_pNMGQTChildNode[0] )
	{
		return;
	}

	// Remove the poly from children who have intersecting bounding boxes.

	SAINAVMESHGEN_AABB* paabb = pPoly->GetAABB();
	for( int iNode=0; iNode < 4; ++iNode )
	{
		if( paabb->IntersectAABB( *( m_pNMGQTChildNode[iNode]->GetAABB() ) ) )
		{
			m_pNMGQTChildNode[iNode]->RemoveNMGPoly( pPoly );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::CompactNMGQTNode
//              
//	PURPOSE:	Compact a node if its children are empty.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::CompactNMGQTNode()
{
	// If a node has no children, it cannot compact.

	if( !m_pNMGQTChildNode[0] )
	{
		return;
	}

	// Compact children.
	int iNode;
	for( iNode=0; iNode < 4; ++iNode )
	{
		m_pNMGQTChildNode[iNode]->CompactNMGQTNode();
	}

	// If children are empty, destroy them.

	bool bIsEmpty = true;
	for( iNode=0; iNode < 4; ++iNode )
	{
		if( !m_pNMGQTChildNode[iNode]->IsNMGQTNodeEmpty() )
		{
			bIsEmpty = false;
			break;
		}
	}

	if( bIsEmpty )
	{
		DestroyNMGQTChildren();
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::AddAINode
//              
//	PURPOSE:	Add AINode to quad tree.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::AddAINode( SAINAVMESHGEN_NODE* pNode )
{
	// Sanity check.

	if( !pNode )
	{
		return;
	}

	// Add a node to the leaf by adding it to the node list.

	if( m_nNMGQTLevel == kNMGQTLevel_Leaf )
	{
		m_lstAINodes.push_back( pNode );
		return;
	}

	// Bail if no children exist.

	if( !m_pNMGQTChildNode[0] )
	{
		return;
	}

	// Add the node to the first child who has a containing bounding box.

	for( int iNode=0; iNode < 4; ++iNode )
	{
		if( m_pNMGQTChildNode[iNode]->GetAABB()->ContainsPoint( pNode->vPos ) )
		{
			m_pNMGQTChildNode[iNode]->AddAINode( pNode );
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::IsNMGQTNodeEmpty
//              
//	PURPOSE:	Return true if node has no children, or is 
//              a leaf and has no polys in list.
//              
//----------------------------------------------------------------------------

bool CAINavMeshGenQuadTreeNode::IsNMGQTNodeEmpty()
{
	// A leaf is empty if it has no polys in its list.

	if( m_nNMGQTLevel == kNMGQTLevel_Leaf )
	{
		return m_lstPolyRefs.empty();
	}

	// A node is empty if it has no children.

	return !m_pNMGQTChildNode[0];
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::GetIntersectingPolys
//              
//	PURPOSE:	Get a list of polys whos bounding boxes intersect 
//              with a given bounding box.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::GetIntersectingPolys( SAINAVMESHGEN_AABB* paabb, NMGPOLY_LIST* plstPolys )
{
	// If this is a leaf node, create a list of polys that intersect the
	// specified bounding box.

	if( m_nNMGQTLevel == kNMGQTLevel_Leaf )
	{
		// Skip polys that are already in the list.

		bool bExists;
		CAINavMeshGenPoly* pPoly;
		NMGPOLY_LIST::iterator itPoly, itPolyRtn;
		for( itPoly = m_lstPolyRefs.begin(); itPoly != m_lstPolyRefs.end(); ++itPoly )
		{
			bExists = false;
			for( itPolyRtn = plstPolys->begin(); itPolyRtn != plstPolys->end(); ++itPolyRtn )
			{
				if( *itPoly == *itPolyRtn )
				{
					bExists = true;
					break;
				}
			}

			// Add poly to list if it intersects bounding box.

			if( !bExists )
			{
				pPoly = CAINavMeshGen::GetAINavMeshGen()->GetNMGPoly( *itPoly );
				if( pPoly && pPoly->GetAABB()->IntersectAABB( *paabb ) )
				{
					plstPolys->push_back( *itPoly );
				}
			}
		}

		return;
	}

	// Not a leaf node, so recurse into children,
	// if children exist.
	
	if( m_pNMGQTChildNode[0] )
	{
		for( int iNode=0; iNode < 4; ++iNode )
		{
			if( paabb->IntersectAABB( *( m_pNMGQTChildNode[iNode]->GetAABB() ) ) )
			{
				m_pNMGQTChildNode[iNode]->GetIntersectingPolys( paabb, plstPolys );
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::GetIntersectingPolys
//              
//	PURPOSE:	Get a list of polys whos bounding boxes intersect 
//              with a given point.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::GetIntersectingPolys( const LTVector& vPos, NMGPOLY_LIST* plstPolys )
{
	SAINAVMESHGEN_AABB aabb;
	aabb.vMin = vPos;
	aabb.vMax = vPos;

	GetIntersectingPolys( &aabb, plstPolys );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::GetIntersectingNodes
//              
//	PURPOSE:	Get a list of nodes whos positions intersect 
//              with a given bounding box.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::GetIntersectingNodes( SAINAVMESHGEN_AABB* paabb, AINAVMESHGEN_NODE_PTR_LIST* plstNodes, SAINAVMESHGEN_NODE* pIgnoreNode )
{
	// Sanity check.

	if( !( paabb && plstNodes ) )
	{
		return;
	}

	// If this is a leaf node, create a list of nodes that intersect the
	// specified bounding box.

	if( m_nNMGQTLevel == kNMGQTLevel_Leaf )
	{
		// Skip nodes that are already in the list.

		bool bExists;
		SAINAVMESHGEN_NODE* pNode;
		AINAVMESHGEN_NODE_PTR_LIST::iterator itNode, itNodeRtn;
		for( itNode = m_lstAINodes.begin(); itNode != m_lstAINodes.end(); ++itNode )
		{
			// Ignore the specified node.

			if( pIgnoreNode == *itNode )
			{
				continue;
			}

			bExists = false;
			for( itNodeRtn = plstNodes->begin(); itNodeRtn != plstNodes->end(); ++itNodeRtn )
			{
				if( *itNode == *itNodeRtn )
				{
					bExists = true;
					break;
				}
			}

			// Add node to list if it intersects bounding box.

			if( !bExists )
			{
				pNode = *itNode;
				if( pNode && paabb->ContainsPoint( pNode->vPos ) )
				{
					plstNodes->push_back( pNode );
				}
			}
		}

		return;
	}

	// Not a leaf node, so recurse into children,
	// if children exist.

	if( m_pNMGQTChildNode[0] )
	{
		for( int iNode=0; iNode < 4; ++iNode )
		{
			if( paabb->IntersectAABB( *( m_pNMGQTChildNode[iNode]->GetAABB() ) ) )
			{
				m_pNMGQTChildNode[iNode]->GetIntersectingNodes( paabb, plstNodes, pIgnoreNode );
			}
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::CountNMGQTNodes
//              
//	PURPOSE:	Recursively count the number of nodes in 
//              the current node and below.  Useful for 
//              debugging / profiling.
//              
//----------------------------------------------------------------------------

int CAINavMeshGenQuadTreeNode::CountNMGQTNodes()
{
	int cTotal = 1;
	if( m_pNMGQTChildNode[0] )
	{
		for( int iNode=0; iNode < 4; ++iNode )
		{
			cTotal += m_pNMGQTChildNode[iNode]->CountNMGQTNodes();
		}
	}

	return cTotal;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAINavMeshGenQuadTreeNode::PrintNMGQTNodes
//              
//	PURPOSE:	Recursively print IDs of nodes.  Useful for 
//              debugging.
//              
//----------------------------------------------------------------------------

void CAINavMeshGenQuadTreeNode::PrintNMGQTNodes()
{
	TRACE( "NMGQTNode: %d\n", m_eNMQTNodeID );

	if( m_pNMGQTChildNode[0] )
	{
		for( int iNode=0; iNode < 4; ++iNode )
		{
			m_pNMGQTChildNode[iNode]->PrintNMGQTNodes();
		}
	}
}

