// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFinishBlockFailure.h
//
// PURPOSE : 
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALFINISHBLOCKFAILURE_H_
#define _AIGOALFINISHBLOCKFAILURE_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalFinishBlockFailure
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalFinishBlockFailure : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFinishBlockFailure, kGoal_FinishBlockFailure );

	// Ctor/Dtor

	CAIGoalFinishBlockFailure();
	virtual ~CAIGoalFinishBlockFailure();

	virtual void	CalculateGoalRelevance();
	virtual void	ActivateGoal();
	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalFinishBlockFailure);
};

#endif // _AIGOALFINISHBLOCKFAILURE_H_
