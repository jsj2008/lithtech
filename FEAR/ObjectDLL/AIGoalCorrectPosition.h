// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCorrectPosition.h
//
// PURPOSE : Defines implementation of the 'correct position' desire
//
// CREATED : 4/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOALCORRECTPOSITION_H_
#define __AIGOALCORRECTPOSITION_H_

#include "AIGoalAbstract.h"

class CAIGoalCorrectPosition : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCorrectPosition, kGoal_CorrectPosition );

		CAIGoalCorrectPosition();
		virtual ~CAIGoalCorrectPosition();

		// CAIGoalAbstract overrides.

		virtual void CalculateGoalRelevance();
		virtual void SetWSSatisfaction( CAIWorldState& WorldState );

protected:

private:
	// Not implemented
	CAIGoalCorrectPosition(const CAIGoalCorrectPosition&);					

	// Not implemented
	const CAIGoalCorrectPosition& operator=(const CAIGoalCorrectPosition&);
};


#endif // __AIGOALCORRECTPOSITION_H_
