// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSafetyFirePosition.h
//
// PURPOSE : Defines a position an AI can run to after using the owning 
//			AINodeSafety instance for cover or reloading.  This node 
//			depends on the parent AINodeSafety being set.  If it isn't, 
//			this node will not be used.
//
// CREATED : 1/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODESAFETYFIREPOSITION_H_
#define _AINODESAFETYFIREPOSITION_H_

LINKTO_MODULE(AINodeSafetyFirePosition);

#include "AINode.h"

class AINodeSafety;

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeSafetyFirePosition
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeSafetyFirePosition : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:
	DEFINE_CAST( AINodeSafetyFirePosition );

	// Ctor/Dtor

	AINodeSafetyFirePosition();
	virtual ~AINodeSafetyFirePosition();

	// Engine

	virtual void ReadProp(const GenericPropList *pProps);
	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Template methods

	virtual bool IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );
	virtual EnumAINodeType GetType() const;
	virtual DebugLine::Color GetDebugColor();
	virtual void LockNode(HOBJECT hAI);
	virtual void UnlockNode(HOBJECT hAI);

	// Sets this nodes parent AINodeSafety instance.  To use this node, the 
	// parent node must be valid.  In addition, while the AI is at this node, 
	// the parent node is locked.

	void SetParent( const AINodeSafety* const pParent );

private:
	PREVENT_OBJECT_COPYING(AINodeSafetyFirePosition);

	AINodeValidatorDamaged			m_DamagedValidator;
	AINodeValidatorThreatFOV		m_ThreatFOV;
	AINodeValidatorBoundaryRadius	m_BoundaryRadius;

	// This node is the owning AINodeSafety instance.  This handle is validated
	// when set and does not need to be validated each access.

	LTObjRef						m_hAINodeSafetyParent;
};

class AINodeSafetyFirePositionPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeSafetyFirePositionPlugin()
	{
		AddValidNodeType(kNode_SafetyFirePosition);
	}
};

#endif // _AINODESAFETYFIREPOSITION_H_
