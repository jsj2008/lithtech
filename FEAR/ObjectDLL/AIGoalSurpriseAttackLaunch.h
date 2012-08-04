// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSurpriseAttackLaunch.h
//
// PURPOSE : Declares the SurpriseAttackLaunch goal.  This goal provides 
//			the AI with a desire to perform a blow from a Surprise node 
//			if the enemy is in place.
//
// CREATED : 2/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALSURPRISEATTACKLAUNCH_H_
#define _AIGOALSURPRISEATTACKLAUNCH_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AIGoalSurpriseAttackLaunch
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalSurpriseAttackLaunch : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalSurpriseAttackLaunch, kGoal_SurpriseAttackLaunch );

	// Ctor/Dtor

	CAIGoalSurpriseAttackLaunch();
	virtual ~CAIGoalSurpriseAttackLaunch();

	// Template methods

	virtual void	CalculateGoalRelevance();
	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );


private:
	PREVENT_OBJECT_COPYING(CAIGoalSurpriseAttackLaunch);
};

#endif // _AIGOALSURPRISEATTACKLAUNCH_H_
