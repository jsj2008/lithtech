// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshGenQuadTree.h
//
// PURPOSE : AI NavMesh generator quad tree class definition
//
// CREATED : 11/02
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_GEN_QUAD_TREE_H_
#define _AI_NAVMESH_GEN_QUAD_TREE_H_

#include "AINavMeshGenTypes.h"

//-----------------------------------------------------------------

typedef std::multimap<
			float, 
			ENUM_NMGPolyID,
			std::less<float>,
			LTAllocator<float, LT_MEM_TYPE_OBJECTSHELL> 
		> NMGPOLY_HEIGHT_MAP;

//-----------------------------------------------------------------

class CAINavMeshGenQuadTreeNode
{
friend class CAINavMeshGen;
public:

	enum ENUM_NMGQTLevel
	{
		kNMGQTLevel_Invalid = -1,
		kNMGQTLevel_Leaf = 0,
		kNMGQTLevel_Root = 4,
	};

	enum ENUM_NMGQTDir
	{
		kNMGQTDir_NW = 0,
		kNMGQTDir_NE = 1,
		kNMGQTDir_SW = 2,
		kNMGQTDir_SE = 3,
	};

public:

	 CAINavMeshGenQuadTreeNode();
	~CAINavMeshGenQuadTreeNode();

	void	InitQuadTreeNode( const SAINAVMESHGEN_AABB& aabb, int nLevel );

	// Quad Tree construction.

	void	CreateNMGQTChildren();
	void	DestroyNMGQTChildren();
	void	AddNMGPoly( CAINavMeshGenPoly* pPoly );
	void	RemoveNMGPoly( CAINavMeshGenPoly* pPoly );
	void	CompactNMGQTNode();
	void	AddAINode( SAINAVMESHGEN_NODE* pNode );

	// Query.

	void	GetIntersectingPolys( SAINAVMESHGEN_AABB* paabb, NMGPOLY_LIST* plstPolys );
	void	GetIntersectingPolys( const LTVector& vPos, NMGPOLY_LIST* plstPolys );
	void	GetIntersectingNodes( SAINAVMESHGEN_AABB* paabb, AINAVMESHGEN_NODE_PTR_LIST* plstNodes, SAINAVMESHGEN_NODE* pIgnoreNode );

	// Data access.

	ENUM_NMGQTNodeID	GetNMQTNodeID() const { return m_eNMQTNodeID; }
	bool				IsNMGQTNodeEmpty();
	SAINAVMESHGEN_AABB*	GetAABB() { return &m_aabbNMGQTBounds; }
	uint32				GetNumNMQTNodePolyRefs() const { return m_lstPolyRefs.size(); }
	const NMGPOLY_LIST*	GetNMQTNodePolyRefs() const { return &m_lstPolyRefs; }
	CAINavMeshGenQuadTreeNode* GetNMQTChildNode( uint32 iNode )	{ return ( iNode < 4 ) ? m_pNMGQTChildNode[iNode] : NULL; }

	// Debug.

	void	PrintNMGQTNodes();
	int		CountNMGQTNodes();

protected:

	CAINavMeshGenQuadTreeNode*	m_pNMGQTChildNode[4];

	ENUM_NMGQTNodeID			m_eNMQTNodeID;
	SAINAVMESHGEN_AABB			m_aabbNMGQTBounds;
	int							m_nNMGQTLevel;
	NMGPOLY_LIST				m_lstPolyRefs;
	AINAVMESHGEN_NODE_PTR_LIST	m_lstAINodes;
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_GEN_QUAD_TREE_H_
