// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkDuckUnder.h
//
// PURPOSE : AI NavMesh Link DuckUnder class definition
//
// CREATED : 11/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_DUCKUNDER_H_
#define _AI_NAVMESH_LINK_DUCKUNDER_H_

#include "AINavMeshLinkAbstractOneAnim.h"

LINKTO_MODULE( AINavMeshLinkDuckUnder );

class AINavMeshLinkDuckUnder : public AINavMeshLinkAbstractOneAnim
{
	typedef AINavMeshLinkAbstractOneAnim super;

public:

	AINavMeshLinkDuckUnder();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_DuckUnder; }
};


//-----------------------------------------------------------------

class AINavMeshLinkDuckUnderPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkDuckUnder; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_DUCKUNDER_H_
