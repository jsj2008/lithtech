// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeArmored.h
//
// PURPOSE : AINodeArmored class declaration
//
// CREATED : 6/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_ARMORED_H_
#define _AI_NODE_ARMORED_H_

#include "AINode.h"
#include "AINodeValidators.h"


//---------------------------------------------------------------------------

class AINodeArmored : public AINodeSmartObject
{
	typedef AINodeSmartObject super;

	public :

		// Ctor/Dtor

		AINodeArmored();
		virtual ~AINodeArmored();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Init

		virtual void InitNode();

		// Status

		virtual bool IsNodeValid( CAI* pAI, const LTVector& vPosAI, HOBJECT hThreat, EnumAIThreatPosition eThreatPos, uint32 dwStatusFlags );

		// Grenades.

		virtual bool AllowThrowGrenades() const { return m_bThrowGrenades; }
		virtual bool RequiresStraightPathToThrowGrenades() const { return false; }

		// Methods

		virtual float	GetBoundaryRadiusSqr() const { return m_BoundaryRadiusValidator.GetBoundaryRadiusSqr(); }
		virtual ENUM_AIRegionID	GetBoundaryAIRegion() const { return m_BoundaryRadiusValidator.GetBoundaryAIRegion(); }
		float			GetThreatRadiusSqr() const { return m_ThreatRadiusValidator.GetThreatRadiusSqr(); }
		bool			IsIgnoreDir() const { return m_bIgnoreDir; }

		// Type

		EnumAINodeType GetType() const { return kNode_Armored; }

		// Debug

		virtual	DebugLine::Color GetDebugColor();

	protected :

		bool		m_bIgnoreDir;
		float		m_fFovDp;

		AINodeValidatorDamaged			m_DamagedValidator;
		AINodeValidatorPlayerOnNode		m_PlayerOnNodeValidator;
		AINodeValidatorThreatRadius		m_ThreatRadiusValidator;
		AINodeValidatorBoundaryRadius	m_BoundaryRadiusValidator;

		bool		m_bThrowGrenades;
};

class AINodeArmoredPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeArmoredPlugin()
	{
		AddValidNodeType(kNode_Armored);
	}
};

//---------------------------------------------------------------------------

#endif // _AI_NODE_ARMORED_H_
