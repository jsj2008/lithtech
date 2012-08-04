// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeHide.h
//
// PURPOSE : Declaration of the Hide node.  This node defines a location 
//			for an AI to hide from its target at.
//
// CREATED : 4/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AINODEHIDE_H_
#define __AINODEHIDE_H_

#include "AINode.h"
#include "AINodeValidators.h"

LINKTO_MODULE( AINodeHide );

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeHide
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeHide : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:

	// Ctors/Dtors/etc

	AINodeHide();
	virtual ~AINodeHide();

	// Engine 

	virtual void ReadProp(const GenericPropList *pProps);

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Init

	virtual void InitNode();

	// Boundary Radius.

	virtual float	GetBoundaryRadiusSqr() const { return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr(); }
	virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_BoundaryRadiusValidator.GetBoundaryAIRegion(); }

	// Type

	EnumAINodeType GetType() const { return kNode_Hide; }

	// Status

	virtual bool	IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );

	// Arrival / Departure.

	virtual void HandleAIArrival( CAI* pAI );

	// Masks for dermining why the AI is aborting

	static uint32 GetNodeStatus_Failed() { return 0; }
	static uint32 GetNodeStatus_Irrelevant() { return 0; }
	static uint32 GetNodeStatus_Succeeded() { return 0; }

private:
	AINodeHide(const AINodeHide& src);				// Not implemented
	const AINodeHide& operator=(const AINodeHide&);	// Not implementedi

	AINodeValidatorThreatRadius		m_ThreatRadiusValidator;
	AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;
	AINodeValidatorThreatFOV		m_ThreatFOV;

	float  m_fMinExpiration;
	float  m_fMaxExpiration;
	double m_fExpirationTime;
};

class AINodeHidePlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeHidePlugin()
	{
		AddValidNodeType(kNode_Hide);
	}
};

#endif // __AINODEHIDE_H_
