// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkLoseTarget.h
//
// PURPOSE : AI NavMesh Link LoseTarget class definition
//
// CREATED : 08/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_LOSE_TARGET_H_
#define _AI_NAVMESH_LINK_LOSE_TARGET_H_

#include "AINavMeshLinkAbstract.h"

LINKTO_MODULE( AINavMeshLinkLoseTarget );

class AINavMeshLinkLoseTarget : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkLoseTarget();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_LoseTarget; }
	virtual bool					IsLinkPassable( CAI* /*pAI*/, ENUM_NMPolyID /*ePolyTo*/ ) { return true; }
	virtual bool					AllowStraightPaths() { return true; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_LOSE_TARGET_H_
