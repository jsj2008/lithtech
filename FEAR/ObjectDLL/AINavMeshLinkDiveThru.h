// ----------------------------------------------------------------------- //
//
// MODULE  : AINavMeshLinkDiveThru.h
//
// PURPOSE : AI NavMesh Link DiveThru class definition
//
// CREATED : 08/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NAVMESH_LINK_DIVETHRU_H_
#define _AI_NAVMESH_LINK_DIVETHRU_H_

#include "AINavMeshLinkAbstractOneAnim.h"

LINKTO_MODULE( AINavMeshLinkDiveThru );

class AINavMeshLinkDiveThru : public AINavMeshLinkAbstractOneAnim
{
	typedef AINavMeshLinkAbstractOneAnim super;

public:

	AINavMeshLinkDiveThru();

	// AINavMeshLinkAbstract overrides.

	virtual void					Save(ILTMessage_Write *pMsg);
   	virtual void					Load(ILTMessage_Read *pMsg);

	virtual EnumAINavMeshLinkType	GetNMLinkType() const { return kLink_DiveThru; }
	virtual void					ActivateTraversal( CAI* pAI, CAIStateUseSmartObject* pStateUseSmartObject );
	virtual void					ApplyTraversalEffect( CAI* pAI );

	// Engine.

	virtual void					ReadProp(const GenericPropList *pProps);
	virtual void					InitialUpdate();

protected:

	std::string			m_strObject;
	LTObjRef			m_hAnimObject;
};

//-----------------------------------------------------------------

class AINavMeshLinkDiveThruPlugin : public AINavMeshLinkAbstractPlugin
{
protected:
	virtual EnumAINodeType	GetSmartObjectFilterType() const { return kNode_NavMeshLinkDiveThru; }
};

//-----------------------------------------------------------------

#endif // _AI_NAVMESH_LINK_DIVETHRU_H_
