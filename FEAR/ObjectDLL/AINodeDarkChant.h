// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeDarkChant.h
//
// PURPOSE : 
//
// CREATED : 8/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODEDARKCHANT_H_
#define _AINODEDARKCHANT_H_

LINKTO_MODULE(AINodeDarkChant);

#include "AINode.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeDarkChant
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeDarkChant : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:

	// Ctor/Dtor

	AINodeDarkChant();
	virtual ~AINodeDarkChant();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	// Engine 

	virtual void	ReadProp(const GenericPropList *pProps);

	// Status

	virtual float		GetBoundaryRadiusSqr() const { return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr(); }
	virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_BoundaryRadiusValidator.GetBoundaryAIRegion(); }
	virtual bool	IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );
	DebugLine::Color GetDebugColor();

	// Type

	EnumAINodeType GetType() const { return kNode_DarkChant; }

private:
	PREVENT_OBJECT_COPYING(AINodeDarkChant);

	AINodeValidatorDamaged			m_DamagedValidator;
	AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;
	AINodeValidatorThreatRadius		m_ThreatRadiusValidator;
	AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;
	AINodeValidatorThreatFOV		m_ThreatFOV;
};

class AINodeDarkChantPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeDarkChantPlugin()
	{
		AddValidNodeType(kNode_DarkChant);
	}
};

#endif // _AINODEDARKCHANT_H_
