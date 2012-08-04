// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkAbstractOneAnim.h
//
// PURPOSE : AI NavMesh Link OneAnim abstract class definition
//
// CREATED : 11/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_ABSTRACT_ONE_ANIM_H_
#define _AI_NAVMESH_LINK_ABSTRACT_ONE_ANIM_H_

#include "AINavMeshLinkAbstract.h"

LINKTO_MODULE( AINavMeshLinkAbstractOneAnim );

class AINavMeshLinkAbstractOneAnim : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkAbstractOneAnim();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_InvalidType; }

	virtual bool					IsLinkRelevant( CAI* pAI );
	virtual void					ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject );
	virtual bool					IsTraversalInProgress( CAI* pAI );
	virtual bool					IsTraversalComplete( CAI* pAI );
	virtual void					ApplyTraversalEffect( CAI* pAI );
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_ABSTRACT_ONE_ANIM_H_
