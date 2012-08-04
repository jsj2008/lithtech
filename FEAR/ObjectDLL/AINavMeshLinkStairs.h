// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkStairs.h
//
// PURPOSE : AI NavMesh Link Stairs class definition
//
// CREATED : 07/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_STAIRS_H_
#define _AI_NAVMESH_LINK_STAIRS_H_

#include "AINavMeshLinkAbstract.h"

LINKTO_MODULE( AINavMeshLinkStairs );

class AINavMeshLinkStairs : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkStairs();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_Stairs; }

	virtual bool					IsLinkRelevant( CAI* pAI ) { return false; }

	virtual void					ApplyMovementAnimation( CAI* pAI );
	virtual void					HandleNavMeshLinkExit( CAI* pAI );
};


//-----------------------------------------------------------------

class AINavMeshLinkStairsPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkStairs; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_STAIRS_H_
