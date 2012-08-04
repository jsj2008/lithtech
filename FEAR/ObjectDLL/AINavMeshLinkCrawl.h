// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkCrawl.h
//
// PURPOSE : AI NavMesh Link Crawl class definition
//
// CREATED : 09/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_CRAWL_H_
#define _AI_NAVMESH_LINK_CRAWL_H_

#include "AINavMeshLinkAbstractOneAnim.h"

LINKTO_MODULE( AINavMeshLinkCrawl );

class AINavMeshLinkCrawl : public AINavMeshLinkAbstractOneAnim
{
	typedef AINavMeshLinkAbstractOneAnim super;

public:

	AINavMeshLinkCrawl();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_Crawl; }
	virtual void					ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject );
	virtual bool					IsTraversalInProgress( CAI* pAI );
	virtual bool					IsTraversalComplete( CAI* pAI );
	virtual void					HandleNavMeshLinkExit( CAI* pAI );
};

//-----------------------------------------------------------------

class AINavMeshLinkCrawlPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkCrawl; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_CRAWL_H_
