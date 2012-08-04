// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkPlayer.h
//
// PURPOSE : AI NavMesh Link Player class definition
//           AI may not traverse AINavMeshLinkPlayer polys.
//           These links are used to include polys in AIRegions
//           that are not traversable by AI.
//
// CREATED : 09/26/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_PLAYER_H_
#define _AI_NAVMESH_LINK_PLAYER_H_

#include "AINavMeshLinkAbstract.h"

LINKTO_MODULE( AINavMeshLinkPlayer );

//
// AI may not traverse AINavMeshLinkPlayer polys.
// These links are used to include polys in AIRegions
// that are not traversable by AI.
//

class AINavMeshLinkPlayer : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkPlayer();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_Player; }

	virtual bool	IsLinkPassable( CAI* /*pAI*/, ENUM_NMPolyID /*ePolyTo*/ ) { return false; }
	virtual bool	GetNumNMLinkNeighbors( int* pcNeighbors );

	virtual bool	IsLinkRelevant( CAI* /*pAI*/ ) { return false; }

	// Query.

	bool			FindNearestNavMeshPos( CAI* pAI, const LTVector& vPos, LTVector* pvNavMeshPos, ENUM_NMPolyID* peNavMeshPoly );
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_PLAYER_H_
