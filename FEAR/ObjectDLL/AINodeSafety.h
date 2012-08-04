// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSafety.h
//
// PURPOSE : Defines AINodeSafety.  This node is used in conjunction with
//			AINodeSafetyFirePosition.  This node provides the AI with a 
//			location to run for cover, with the firing positions 
//			locations to depart to when the AI decides to attack.
//
// CREATED : 1/28/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODESAFETY_H_
#define _AINODESAFETY_H_

LINKTO_MODULE(AINodeSafety);

#include "AINode.h"
#include "NamedObjectList.h"

class AINodeSafetyFirePosition;

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeSafety
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeSafety : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:
	DEFINE_CAST( AINodeSafety );

	// Ctor/Dtor

	AINodeSafety();
	virtual ~AINodeSafety();

	// Engine 

	virtual void ReadProp(const GenericPropList *pProps);
	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Template Methods

	virtual void AllNodesInitialized();
	virtual bool IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );
	virtual EnumAINodeType GetType() const;
	virtual float GetBoundaryRadiusSqr() const;
	virtual void HandleAIArrival( CAI* pAI );
	virtual bool IsLockedDisabledOrTimedOut( HOBJECT hQueryingAI );

	// Debugging

	virtual int DrawSelf();
	virtual int HideSelf();
	virtual DebugLine::Color GetDebugColor();
	virtual void UpdateDebugDrawStatus( HOBJECT hTarget );

	// Methods for accessing the owned AINodeSafetyFirePosition instances

	int GetFirePositionCount() const;
	AINodeSafetyFirePosition* GetFirePosition( int i ) const;
	double GetExpirationTime( CAI* pAI ) const;

private:
	// This object does not support copying.

	PREVENT_OBJECT_COPYING(AINodeSafety);

	// Supported validators.

	AINodeValidatorDamaged			m_DamagedValidator;
	AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;
	AINodeValidatorThreatRadius		m_ThreatRadiusValidator;
	AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;
	AINodeValidatorThreatFOV		m_ThreatFOV;
	AINodeValidatorExpiration		m_ExpirationValidator;

	// List of FirePosition nodes this node owns.  The node type is validated 
	// on insertion to this list -- an object in this list can safely be cast 
	// to AINodeSafetyFirePosition without type checking.

	CNamedObjectList				m_FireNodes;
};

class AINodeSafetyPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeSafetyPlugin()
	{
		AddValidNodeType(kNode_Safety);
	}
};

#endif // _AINODESAFETY_H_
