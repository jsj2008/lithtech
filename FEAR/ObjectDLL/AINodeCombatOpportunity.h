// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCombatOpportunity.h
//
// PURPOSE : 
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODECOMBATOPPORTUNITY_H_
#define _AINODECOMBATOPPORTUNITY_H_

#include "AINode.h"
#include "AINodeValidators.h"

LINKTO_MODULE(AINodeCombatOpportunity);

class AICombatOpportunity;

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeCombatOpportunity
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeCombatOpportunity : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:

	// Ctor/Dtor

	AINodeCombatOpportunity();
	virtual ~AINodeCombatOpportunity();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Engine

	virtual void ReadProp(const GenericPropList *pProps);

	// Init

	virtual void InitNode();

	// Lock/Unlock

	virtual void LockNode(HOBJECT hAI);
	virtual void UnlockNode(HOBJECT hAI);

	// Type

	virtual EnumAINodeType	GetType() const { return kNode_CombatOpportunity; }

	// Status

	virtual bool			IsNodeValid( CAI* /*pAI*/, const LTVector& /*vPosAI*/, HOBJECT /*hThreat*/, EnumAIThreatPosition eThreatPos, uint32 /*dwStatusFlags*/ );
	virtual float			GetBoundaryRadiusSqr() const { return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr(); }
	virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_BoundaryRadiusValidator.GetBoundaryAIRegion(); }

	// AICombatOpportunity

	void					SetCombatOpportunity(AICombatOpportunity* pCombatOpportunity);

private:
	PREVENT_OBJECT_COPYING(AINodeCombatOpportunity);

	// Handle to the AICombatOpportunity object this node uses.  This is used to
	// determine if an AINodeCombatOpportunity node is valid.
	LTObjRef				m_hCombatOpportunity;

	// Ensure Player is standing on node that AI wants to use for a combat opportunity.
	AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;

	// Contains the radius the enemy must be outside to for an AI to consider
	// using this node. If the enemy is inside this radius, and if the status flag
	// which test this radius is set, IsNodeValid will fail.
	AINodeValidatorThreatRadius	m_ThreatRadiusValidator;

	// Contains the radius the enemy must be inside for an AI to consider using 
	// this node.  If the enemy is outside of this radius, and if the status flag
	// which tests this radius is set, IsNodeValid will fail.
	AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;
};

#endif // _AINODECOMBATOPPORTUNITY_H_
