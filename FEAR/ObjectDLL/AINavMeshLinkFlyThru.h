// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkFlyThru.h
//
// PURPOSE : AI NavMesh Link FlyThru class definition
//
// CREATED : 09/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_FLY_THRU_H_
#define _AI_NAVMESH_LINK_FLY_THRU_H_

#include "AINavMeshLinkAbstractOneAnim.h"

LINKTO_MODULE( AINavMeshLinkFlyThru );

class AINavMeshLinkFlyThru : public AINavMeshLinkAbstractOneAnim
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkFlyThru();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_FlyThru; }

	virtual bool					IsLinkRelevant( CAI* pAI );

	virtual void					ModifyMovement( CAI* pAI, CAIMovement::State eStatePrev, LTVector* pvNewPos, CAIMovement::State* peStateCur );

	virtual bool					AllowStraightPaths() { return true; }

	virtual void					HandleNavMeshLinkEnter( CAI* pAI );
	virtual void					HandleNavMeshLinkExit( CAI* pAI );

protected:

	void							CreateFlyThruFX( CAI* pAI, float fDirMult, const char* pszFX );
};

//-----------------------------------------------------------------

class AINavMeshLinkFlyThruPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkFlyThru; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_CRAWL_H_
