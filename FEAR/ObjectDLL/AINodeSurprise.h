// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSurprise.h
//
// PURPOSE : This file defines the Surprise node.  This node enables AIs
//			 to pop out from hiding and perform fluid attacks against 
//			 their enemies.
//
// CREATED : 2/03/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODESURPRISE_H_
#define _AINODESURPRISE_H_

LINKTO_MODULE(AINodeSurprise);

#include "AINode.h"
#include "VolumeBrush.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		SurpriseVolume
//
//	PURPOSE:	This class is a typed version of the VolumeBrush.  If this
//				class is heavily used, we may want to make a special 'light 
//				weight' volume.  This will enable this change without 
//				requiring level design changes.
//
// ----------------------------------------------------------------------- //

class SurpriseVolume : public VolumeBrush
{
};

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeSurprise
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeSurprise : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:

	// Ctor/Dtor

	AINodeSurprise();
	virtual ~AINodeSurprise();

	// Engine

	virtual uint32 EngineMessageFn(uint32 messageID, void *pvData, float fData);
	virtual void ReadProp(const GenericPropList *pProps);
	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Template Methods

	virtual bool IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );
	virtual EnumAINodeType GetType() const;
	virtual float GetBoundaryRadiusSqr() const;
	virtual DebugLine::Color GetDebugColor();

	// Returns the Action animation prop for the volume valid for using to 
	// attack the passed in object, kAP_Invalid if there are no valid volumes.

	EnumAnimProp GetSurpriseAttackAnimationProp( HOBJECT hThreat ) const;

	// Called when the node is used for a surprise attack.  This handles 
	// disabling the node for the specified amount of time.

	void HandleSurpriseAttack();

private:
	PREVENT_OBJECT_COPYING(AINodeSurprise);

	// Supported validators.

	AINodeValidatorDamaged			m_DamagedValidator;
	AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;
	AINodeValidatorThreatRadius		m_ThreatRadiusValidator;
	AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;
	AINodeValidatorThreatFOV		m_ThreatFOV;

	// Names of the associated volumes.  These do not need to be saved as 
	// they are only used during creation

	std::string m_CloseSurpriseVolumeName;
	std::string m_FarSurpriseVolumeName;

	// Handles to the associated volumes.  If the enemy is inside of one of 
	// these volumes, the AI can apply the associated attack.  These objects 
	// are guaranteed to be SurpriseVolume instances.
	LTObjRef	m_hCloseSurpriseVolume;
	LTObjRef	m_hFarSurpriseVolume;

	double		m_flSuccesfulUseTimeOut;
};

class AINodeSurpriseObjectPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeSurpriseObjectPlugin()
	{
		AddValidNodeType(kNode_Surprise);
	}
};

#endif // _AINODESURPRISE_H_
