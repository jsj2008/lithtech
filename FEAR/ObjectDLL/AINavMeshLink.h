// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLink.h
//
// PURPOSE : AI NavMesh Link class definition
//
// CREATED : 01/08/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_H_
#define _AI_NAVMESH_LINK_H_

#include "AINavMeshLinkAbstract.h"

LINKTO_MODULE( AINavMeshLink );

class AINavMeshLink : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLink();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_Base; }
	virtual bool					IsLinkPassable( CAI* /*pAI*/, ENUM_NMPolyID /*ePolyTo*/ ) { return true; }
	virtual bool					AllowStraightPaths() { return true; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_H_
