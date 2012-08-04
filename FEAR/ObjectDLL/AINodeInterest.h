// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeInterest.h
//
// PURPOSE : AINodeInterest class declaration
//
// CREATED : 10/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_INTEREST_H_
#define _AI_NODE_INTEREST_H_

#include "AINode.h"

LINKTO_MODULE( AINodeInterest );


//---------------------------------------------------------------------------

class AINodeInterest : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeInterest();
		virtual ~AINodeInterest();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Containing NavMesh poly

		virtual bool AllowOutsideNavMesh() { return true; }

		// Status

		virtual bool	IsNodeValid( CAI* /*pAI*/, const LTVector& /*vPosAI*/, HOBJECT /*hThreat*/, EnumAIThreatPosition eThreatPos, uint32 /*dwStatusFlags*/ );

		// Type

		EnumAINodeType GetType() const { return kNode_Interest; }

		// Query.

		float		GetLookTime() const { return m_fLookTime; }

	protected:

		bool		m_bIgnoreDir;
		float		m_fLookTime;

		AINodeValidatorLockedByOther	m_LockedByOthersValidator;
		AINodeValidatorOwnerNotLocked	m_OwnerNotLockedValidator;
		AINodeValidatorAIBackToNode		m_AIBackToNodeValidator;
		AINodeValidatorAIOutsideFOV		m_AIOutsideFOVValidator;
};

//---------------------------------------------------------------------------


#endif // _AI_NODE_INTEREST_H_
