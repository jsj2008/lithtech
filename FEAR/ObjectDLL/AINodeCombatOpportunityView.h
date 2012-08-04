// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCombatOpportunityView.h
//
// PURPOSE : 
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODECOMBATOPPORTUNITYVIEW_H_
#define _AINODECOMBATOPPORTUNITYVIEW_H_

LINKTO_MODULE(AINodeCombatOpportunityView);

#include "AINode.h"

class AICombatOpportunity;

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeCombatOpportunityView
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeCombatOpportunityView : public AINode
{
	typedef AINode super;
	
public:

	// Ctor/Dtor

	AINodeCombatOpportunityView();
	virtual ~AINodeCombatOpportunityView();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Type

	virtual EnumAINodeType	GetType() const { return kNode_CombatOpportunityView; }

	// Status

	virtual bool			IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );
	virtual float			GetBoundaryRadiusSqr() const { return FLT_MAX; }

	// AICombatOpportunity

	void					SetCombatOpportunity(AICombatOpportunity* pCombatOpportunity);

private:
	PREVENT_OBJECT_COPYING(AINodeCombatOpportunityView);

	// Ensures Player is not standing on node the AI wants to use.
	AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;

	// Handle to the AICombatOpportunity object this node uses.  This is used to
	// determine if an AINodeCombatOpportunity node is valid.
	LTObjRef				m_hCombatOpportunity;
};

#endif // _AINODECOMBATOPPORTUNITYVIEW_H_

