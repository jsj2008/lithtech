// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkJump.h
//
// PURPOSE : AI NavMesh Link Jump class definition
//
// CREATED : 07/30/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_JUMP_H_
#define _AI_NAVMESH_LINK_JUMP_H_

LINKTO_MODULE( AINavMeshLinkJump );

class AINavMeshLinkJump : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkJump();

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_Jump; }
	virtual bool					IsLinkPassable( CAI* pAI, ENUM_NMPolyID ePolyTo );
	virtual bool					IsLinkRelevant( CAI* pAI );
	virtual void					ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject );
	virtual bool					IsTraversalComplete( CAI* pAI );
	virtual void					ApplyTraversalEffect( CAI* pAI );
};


//-----------------------------------------------------------------

class AINavMeshLinkJumpPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkJump; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_JUMP_H_
