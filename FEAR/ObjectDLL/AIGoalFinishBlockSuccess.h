// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFinishBlockSuccess.h
//
// PURPOSE : 
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALFINISHBLOCKSUCCESS_H_
#define _AIGOALFINISHBLOCKSUCCESS_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalFinishBlockSuccess
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalFinishBlockSuccess : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFinishBlockSuccess, kGoal_FinishBlockSuccess );

	// Ctor/Dtor

	CAIGoalFinishBlockSuccess();
	virtual ~CAIGoalFinishBlockSuccess();

	virtual void	CalculateGoalRelevance();
	virtual void	ActivateGoal();
	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalFinishBlockSuccess);
};

#endif // _AIGOALFINISHBLOCKSUCCESS_H_
