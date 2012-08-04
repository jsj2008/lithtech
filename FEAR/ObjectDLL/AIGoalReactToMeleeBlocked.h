// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToMeleeBlocked.h
//
// PURPOSE : 
//
// CREATED : 9/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGoalReactToMeleeBlocked_H_
#define _AIGoalReactToMeleeBlocked_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalReactToMeleeBlocked
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalReactToMeleeBlocked : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToMeleeBlocked, kGoal_ReactToMeleeBlocked );

	// Ctor/Dtor

	CAIGoalReactToMeleeBlocked();
	virtual ~CAIGoalReactToMeleeBlocked();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// CAIGoalAbstract overrides.

	virtual void	CalculateGoalRelevance();
	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalReactToMeleeBlocked);
};

#endif // _AIGoalReactToMeleeBlocked_H_
