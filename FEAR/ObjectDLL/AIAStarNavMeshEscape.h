// ----------------------------------------------------------------------- //
//
// MODULE  : AIStarNavMeshEscape.h
//
// PURPOSE : AStar classes for finding escape paths on a NavMesh.
//
// CREATED : 04/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_ASTAR_NAVMESH_ESCAPE_H_
#define _AI_ASTAR_NAVMESH_ESCAPE_H_

#include "AIAStarNavMesh.h"

// Forward declarations.

class	CAIAStarMapNavMeshEscape;


//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarGoalNavMeshEscape : public CAIAStarGoalNavMesh
{
	typedef CAIAStarGoalNavMesh super;

public:
	 CAIAStarGoalNavMeshEscape();
	~CAIAStarGoalNavMeshEscape();

	// CAIAStarGoalAbstract required functions.

	virtual float	GetHeuristicDistance( CAIAStarNodeAbstract* pAStarNode );
	virtual bool	IsAStarFinished( CAIAStarNodeAbstract* pAStarNode );

	// Initialization.

	void			InitAStarGoalNavMesh( CAIAStarMapNavMesh* pAStarMapNavMesh );

protected:

	CAIAStarMapNavMeshEscape*	m_pAStarMapNavMeshEscape;
};

//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarMapNavMeshEscape : public CAIAStarMapNavMesh
{
	typedef CAIAStarMapNavMesh super;

public:
	CAIAStarMapNavMeshEscape();
	~CAIAStarMapNavMeshEscape();

	// Initialization for search.

	void	InitAStarMapNavMeshSearch( uint32 dwCharTypeMask, const LTVector& vSource, const LTVector& vDanger, float fClearance );

	// CAIAStarMapAbstract required functions.

	virtual ENUM_AStarNodeID GetAStarNeighbor( CAI* pAI, CAIAStarNodeAbstract* pAStarNode, int iNeighbor, CAIAStarStorageAbstract* pAStarStorage );

	// Data access.

	const LTVector& GetDanger() const { return m_vDanger; }
	float			GetClearance() const { return m_fClearance; }

protected:

	LTVector	m_vDanger;
	float		m_fClearance;
};

//-----------------------------------------------------------------

#endif // _AI_ASTAR_NAVMESH_ESCAPE_H_
