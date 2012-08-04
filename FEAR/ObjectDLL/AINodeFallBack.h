// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeFallBack.h
//
// PURPOSE : 
//
// CREATED : 9/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODEFALLBACK_H_
#define _AINODEFALLBACK_H_

LINKTO_MODULE(AINodeFallBack);

#include "AINode.h"
#include "AINodeValidators.h"


// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeFallBack
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeFallBack : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:

	// Ctor/Dtor

	AINodeFallBack();
	virtual ~AINodeFallBack();

	// Engine 

	virtual void ReadProp(const GenericPropList *pProps);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Init

	virtual void InitNode();

	// Status

	virtual bool IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );

	// Type

	EnumAINodeType GetType() const { return kNode_FallBack; }

	virtual float	GetBoundaryRadiusSqr() const { return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr(); }
	virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_BoundaryRadiusValidator.GetBoundaryAIRegion(); }
	virtual float	GetThreatRadiusSqr() const { return m_ThreatRadiusValidator.GetThreatRadiusSqr(); }

private:
	PREVENT_OBJECT_COPYING(AINodeFallBack);

	AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;
	AINodeValidatorDamaged			m_DamagedValidator;
	AINodeValidatorThreatFOV		m_ThreatFOVValidator;
	AINodeValidatorLockedByOther	m_LockedByOthersValidator;
	AINodeValidatorThreatRadius	m_ThreatRadiusValidator;
};

class AINodeFallBackPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeFallBackPlugin()
	{
		AddValidNodeType(kNode_FallBack);
	}
};

#endif // _AINODEFALLBACK_H_
