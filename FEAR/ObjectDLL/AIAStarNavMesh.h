// ----------------------------------------------------------------------- //
//
// MODULE  : AIPathMgrNavMesh.h
//
// PURPOSE : AStar Node, Goal, Storage, and Map classes for finding
//           paths on a NavMesh.
//
// CREATED : 12/02/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_ASTAR_NAVMESH_H_
#define _AI_ASTAR_NAVMESH_H_

#include "AINavMesh.h"
#include "AIAStarStorageLinkedList.h"


//-----------------------------------------------------------------

// Forward declarations.

class CAIAStarMapNavMesh;

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarNodeNavMesh : public CAIAStarNodeAbstract
{
public:
	DECLARE_AI_FACTORY_CLASS( CAIAStarNodeNavMesh );

	 CAIAStarNodeNavMesh() {}
	~CAIAStarNodeNavMesh() {}

	// Debug.

	virtual void	DebugPrint() { TRACE( "ASTAR: %d\n", eAStarNodeID ); }

public:

	LTVector	vPotentialEntryPos;
	LTVector	vTrueEntryPos;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarGoalNavMesh : public CAIAStarGoalAbstract
{
public:
	 CAIAStarGoalNavMesh();
	~CAIAStarGoalNavMesh();

	// CAIAStarGoalAbstract required functions.

	virtual void	SetDestNode( ENUM_AStarNodeID eAStarNode ) { m_eAStarNodeDest = eAStarNode; }
	virtual float	GetHeuristicDistance( CAIAStarNodeAbstract* pAStarNode );
	virtual float	GetActualCost( CAI* pAI, CAIAStarNodeAbstract* pAStarNodeA, CAIAStarNodeAbstract* pAStarNodeB );
	virtual bool	IsAStarFinished( CAIAStarNodeAbstract* pAStarNode );

	// Initialization.

	void			InitAStarGoalNavMesh( CAIAStarMapNavMesh* pAStarMapNavMesh ) { m_pAStarMapNavMesh = pAStarMapNavMesh; }

protected:

	ENUM_AStarNodeID		m_eAStarNodeDest;
	CAIAStarMapNavMesh*		m_pAStarMapNavMesh;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarStorageNavMesh : public CAIAStarStorageLinkedList
{
public:

	// CAIAStarStorageLinkedList overrides.

	virtual CAIAStarNodeAbstract*	CreateAStarNode( ENUM_AStarNodeID eAStarNode );
	virtual void					DestroyAStarNode( CAIAStarNodeAbstract* pAStarNode );
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarMapNavMesh : public CAIAStarMapAbstract
{
public:
	 CAIAStarMapNavMesh();
	~CAIAStarMapNavMesh();

	// CAIAStarMapAbstract required functions.

	virtual int				 GetNumAStarNeighbors( CAI* pAI, CAIAStarNodeAbstract* pAStarNode );
	virtual ENUM_AStarNodeID GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage );

	virtual void			SetAStarFlags( ENUM_AStarNodeID eAStarNode, unsigned long dwFlags, unsigned long dwMask );
	virtual unsigned long	GetAStarFlags( ENUM_AStarNodeID eAStarNode );

	virtual void			SetupPotentialNeighbor( CAIAStarNodeAbstract* pAStarNodeParent, CAIAStarNodeAbstract* pAStarNodeChild );
	virtual void			FinalizeNeighbor( CAIAStarNodeAbstract* pAStarNodeChild );

	// Initialization/Termination.

	void	InitAStarMapNavMesh( CAINavMesh* pNavMesh );
	void	TermAStarMapNavMesh( );

	// Initialization for a search.

	void	InitAStarMapNavMeshSearch( uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest );
	
	// ID conversion.

	ENUM_NMPolyID			ConvertID_AStarNode2NMPoly( ENUM_AStarNodeID eAStarNode ) { return ( ENUM_NMPolyID )eAStarNode; }
	ENUM_AStarNodeID		ConvertID_NMPoly2AStarNode( ENUM_NMPolyID eNMPolyID ) { return ( ENUM_AStarNodeID )eNMPolyID; }

	// Geometry access.

	CAINavMeshPoly*			GetNMPoly( ENUM_AStarNodeID eAStarNode ) { return g_pAINavMesh->GetNMPoly( ( ENUM_NMPolyID )eAStarNode ); }
	bool					GetNMEdgeMidPt( ENUM_AStarNodeID eAStarNodeA, ENUM_AStarNodeID eAStarNodeB, LTVector* pvMidPt );

	// Path access.

	const LTVector&			GetPathSource() const { return m_vPathSource; }
	const LTVector&			GetPathDest() const { return m_vPathDest; }

	// Character type restrictions.

	uint32					GetCharTypeMask() const { return m_dwCharTypeMask; }

protected:

	CAINavMesh*			m_pNavMesh;
	unsigned char*		m_pAStarFlags;
	uint32				m_nAStarFlags;
	uint32				m_dwCharTypeMask;
	LTVector			m_vPathSource;
	LTVector			m_vPathDest;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

//
// StraightPath variations.
//

class CAIAStarMapNavMeshStraightPath : public CAIAStarMapNavMesh
{
	typedef CAIAStarMapNavMesh super;

	public:
		CAIAStarMapNavMeshStraightPath();

		// Initialization for a search.

		void	InitAStarMapNavMeshSearch( uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDest, float fRadius );

		// CAIAStarMapAbstract required functions.

		virtual ENUM_AStarNodeID GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage );

	protected:
		
		float		m_fPathRadius;
};

//-----------------------------------------------------------------

#endif // _AI_ASTAR_NAVMESH_H_
