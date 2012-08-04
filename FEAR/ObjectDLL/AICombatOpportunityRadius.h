// ----------------------------------------------------------------------- //
//
// MODULE  : AICombatOpportunityRadius.h
//
// PURPOSE : 
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AICOMBATOPPORTUNITYRADIUS_H_
#define __AICOMBATOPPORTUNITYRADIUS_H_

#include "GameBase.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AICombatOpportunityRadius
//
//	PURPOSE:	This object is meant to be used with the 
//				AICombatOpportunity object system to allow level design a
//				way of naming a mobile area of space.  For instance, this
//				radius may be keyframed along with an exploding barrel,
//				allowing the AI to reason about when he should shoot the 
//				barrel.
//
// ----------------------------------------------------------------------- //

class AICombatOpportunityRadius : public GameBase
{
	typedef GameBase super;
	
public:
	static AICombatOpportunityRadius* HandleToObject(HOBJECT hObject);

	// Ctor/Dtor

	AICombatOpportunityRadius();
	virtual ~AICombatOpportunityRadius();

	// Save/Load

	void Load(ILTMessage_Read *pMsg);
	void Save(ILTMessage_Write *pMsg);

	// Engine

	virtual uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);
	void			ReadProp(const GenericPropList *pProps);

	// Queries

	// This is the primary containment test function.  Returns true when the 
	// passed in position is outside the inner radius, and inside the outer 
	// radius.  Otherwise, returns false.
	bool		Contains(const LTVector& vPosition) const;

	// Returns the current location of the AICombatOpportunityRadius object.
	LTVector	GetPosition() const;
	
	// Returns the radius of the AICombatOpportunityRadius object.
	float		GetOuterRadius() const;

private:
	PREVENT_OBJECT_COPYING(AICombatOpportunityRadius);

	// The position must be inside this radius.
	float	m_flOuterRadius;

	// The target must be outside this radius.
	float	m_flInnerRadius;

	// For debugging.  If true, the DEBUG_DRAWRADIUS command will toggle 
	// drawing a line (when queried for containment) from the center of the
	// radius to the object.
#ifndef _FINAL
	bool	m_bDebugDraw;
#endif

	// Message Handlers...

	DECLARE_MSG_HANDLER( AICombatOpportunityRadius, HandleDebugDrawRadiusMsg);
};

#endif // __AICOMBATOPPORTUNITYRADIUS_H_
