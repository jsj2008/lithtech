// ----------------------------------------------------------------------- //
//
// MODULE  : AIStarNavMeshSafe.h
//
// PURPOSE : AStar classes for finding the safest paths on a NavMesh.
//
// CREATED : 11/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_ASTAR_NAVMESH_SAFE_H_
#define _AI_ASTAR_NAVMESH_SAFE_H_

#include "AIAStarNavMesh.h"


//-----------------------------------------------------------------
//-----------------------------------------------------------------

class CAIAStarGoalNavMeshSafe : public CAIAStarGoalNavMesh
{
	typedef CAIAStarGoalNavMesh super;

public:
	 CAIAStarGoalNavMeshSafe();
	~CAIAStarGoalNavMeshSafe();

	void			SetTargetNMPoly( ENUM_NMPolyID ePoly ) { m_eTargetPoly = ePoly; }

	// CAIAStarGoalAbstract required functions.

	virtual float	GetActualCost( CAI* pAI, CAIAStarNodeAbstract* pAStarNodeA, CAIAStarNodeAbstract* pAStarNodeB );

protected:

	ENUM_NMPolyID	m_eTargetPoly;
};

//-----------------------------------------------------------------

#endif // _AI_ASTAR_NAVMESH_ESCAPE_H_
