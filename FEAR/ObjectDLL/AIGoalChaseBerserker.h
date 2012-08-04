// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChaseBerserker.h
//
// PURPOSE : 
//
// CREATED : 8/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALCHASEBERSERKER_H_
#define _AIGOALCHASEBERSERKER_H_

LINKTO_MODULE(AIGoalChaseBerserker);

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalChaseBerserker
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalChaseBerserker : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalChaseBerserker, kGoal_ChaseBerserker );

	// Ctor/Dtor

	CAIGoalChaseBerserker();
	virtual ~CAIGoalChaseBerserker();

	// CAIGoalAbstract overrides.

	virtual void CalculateGoalRelevance();
	virtual void SetWSSatisfaction( CAIWorldState& WorldState );
	virtual bool IsWSSatisfied( CAIWorldState* pwsWorldState );
	virtual void HandleBuildPlanFailure();

private:
	PREVENT_OBJECT_COPYING(CAIGoalChaseBerserker);
};

#endif // _AIGOALCHASEBERSERKER_H_
