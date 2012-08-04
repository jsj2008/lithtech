// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkDuckRun.h
//
// PURPOSE : AI NavMesh Link DuckRun class definition
//
// CREATED : 02/10/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_DUCK_RUN_H_
#define _AI_NAVMESH_LINK_DUCK_RUN_H_

#include "AINavMeshLinkAbstract.h"

LINKTO_MODULE( AINavMeshLinkDuckRun );

class AINavMeshLinkDuckRun : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkDuckRun();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_DuckRun; }

	virtual bool					IsLinkRelevant( CAI* pAI ) { return false; }

	virtual void					ApplyMovementAnimation( CAI* pAI );
};


//-----------------------------------------------------------------

class AINavMeshLinkDuckRunPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkDuckRun; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_STAIRS_H_
