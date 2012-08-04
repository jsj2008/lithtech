// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDefeated.h
//
// PURPOSE : 
//
// CREATED : 9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALDEFEATED_H_
#define _AIGOALDEFEATED_H_

#include "AIGoalReactToDamage.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalDefeated
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalDefeated : public CAIGoalReactToDamage
{
	typedef CAIGoalReactToDamage super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalDefeated, kGoal_Defeated );

	// Ctor/Dtor

	CAIGoalDefeated();
	virtual bool	ReplanRequired();

private:
	PREVENT_OBJECT_COPYING(CAIGoalDefeated);
};

#endif // _AIGOALDEFEATED_H_
