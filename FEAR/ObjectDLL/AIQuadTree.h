// ----------------------------------------------------------------------- //
//
// MODULE  : AIQuadTree.h
//
// PURPOSE : AI QuadTree class definition
//
// CREATED : 12/12/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_QUAD_TREE_H_
#define _AI_QUAD_TREE_H_

#include "AIClassFactory.h"
#include "AIEnumNavMeshTypes.h"
#include "AINavMesh.h"

//-----------------------------------------------------------------

// Externs

extern class CAIQuadTree* g_pAIQuadTree;

// Enums.

enum ENUM_QTNodeID { kQTNode_Invalid = -1, };


//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIQuadTreeNode
{
friend class CAINavMeshGenQuadTreeNode;
public:

	 CAIQuadTreeNode();
	~CAIQuadTreeNode();

	// Construction.

	void			FixUpNavMeshPointers( CAIQuadTreeNode* pAIQuadTreeNodes, ENUM_NMPolyID* pAIQuadTreeNMPolyLists );

	// Query.

	ENUM_NMPolyID	GetContainingNMPoly( const LTVector& vPos, uint32 dwCharTypeMask, CAI* pAI = NULL );

	// Debug.

	void			PrintQTNodes();

protected:

	ENUM_QTNodeID		m_eQTNodeID;
	SAABB				m_aabbQTBounds;

	int					m_cNMPolyListSize;
	ENUM_NMPolyID*		m_pNMPolyList;

	CAIQuadTreeNode*	m_pQTNodeChild[4];
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIQuadTree
{
friend class CAINavMeshGen;
public:

	 CAIQuadTree();
	~CAIQuadTree();

	void	InitQuadTree();
	void	TermQuadTree();

	void	SetQuadTreeRoot( CAIQuadTreeNode* pQTNodeRoot ) { m_pQTNodeRoot = pQTNodeRoot; }

	// Query.

	bool			IsAIQuadTreeInitialized() const { return m_bQTInitialized; }
	ENUM_NMPolyID	GetContainingNMPoly( const LTVector& vPos, uint32 dwCharTypeMask, ENUM_NMPolyID ePolyHint, CAI* pAI = NULL );
	static bool		CanTraverseLink( const CAINavMeshPoly& poly, CAI* pAI );

protected:

	bool				m_bQTInitialized;
	CAIQuadTreeNode*	m_pQTNodeRoot;
};

//-----------------------------------------------------------------

#endif // _AI_QUADTREE_H_
