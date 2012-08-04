// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkJumpOver.h
//
// PURPOSE : AI NavMesh Link JumpOver class definition
//
// CREATED : 08/15/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_JUMPOVER_H_
#define _AI_NAVMESH_LINK_JUMPOVER_H_

LINKTO_MODULE( AINavMeshLinkJumpOver );

class AINavMeshLinkJumpOver : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkJumpOver();

	virtual void					Save(ILTMessage_Write *pMsg);
	virtual void					Load(ILTMessage_Read *pMsg);

	// AINavMeshLinkAbstract overrides.

	virtual void					ReadProp( const GenericPropList *pProps );

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_JumpOver; }
	virtual bool					IsLinkRelevant( CAI* pAI );

	virtual bool					IsLinkValidDest() { return false; }

	virtual void					ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject );
	virtual bool					IsTraversalComplete( CAI* pAI );
	virtual void					ApplyTraversalEffect( CAI* pAI );

	virtual void					ApplyMovementAnimation( CAI* pAI );

protected:

	LTVector	m_vPos;
};


//-----------------------------------------------------------------

class AINavMeshLinkJumpOverPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkJumpOver; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_JUMPOVER_H_
