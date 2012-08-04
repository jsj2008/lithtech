// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkClimb.h
//
// PURPOSE : AI NavMesh Link Climb class definition
//
// CREATED : 08/01/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_CLIMB_H_
#define _AI_NAVMESH_LINK_CLIMB_H_

#include "AINavMeshLinkAbstract.h"

LINKTO_MODULE( AINavMeshLinkClimb );

class AINavMeshLinkClimb : public AINavMeshLinkAbstract
{
	typedef AINavMeshLinkAbstract super;

public:

	AINavMeshLinkClimb();


	virtual void					Save(ILTMessage_Write *pMsg);
	virtual void					Load(ILTMessage_Read *pMsg);

	// AINavMeshLinkAbstract overrides.

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_Climb; }
	virtual bool					IsLinkRelevant( CAI* pAI );
	virtual bool					IsLinkValidDest() { return false; }

	virtual void					ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject );
	virtual void					DeactivateTraversal( CAI* pAI );
	virtual bool					IsTraversalComplete( CAI* pAI );
	virtual void					ApplyTraversalEffect( CAI* pAI );

	virtual bool					IsPullStringsModifying( CAI* pAI );
	virtual bool					PullStrings( const LTVector& vPtPrev, const LTVector& vPtNext, CAI* pAI, LTVector* pvNewPos );
	virtual bool					HandleAdvancePath( CAI* pAI );
	virtual void					ApplyMovementAnimation( CAI* pAI );
	virtual void					ModifyMovement( CAI* pAI, CAIMovement::State eStatePrev, LTVector* pvNewPos, CAIMovement::State* peStateCur );
	virtual bool					GetNewPathClearEnteringLink( CAI* pAI );
	virtual ENUM_NMPolyID			GetNewPathSourcePoly( CAI* pAI );

	// Engine.

	virtual void					ReadProp(const GenericPropList *pProps);
	virtual void					InitialUpdate();

protected:
	bool							FinishedClimbingDown( CAI* pAI ) const;


	std::string			m_strObject;
	LTObjRef			m_hAnimObject;
};

//-----------------------------------------------------------------

class AINavMeshLinkClimbPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkClimb; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_CLIMB_H_
