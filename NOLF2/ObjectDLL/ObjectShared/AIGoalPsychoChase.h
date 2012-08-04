// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalPsychoChase.h
//
// PURPOSE : AIGoalPsychoChase class definition
//
// CREATED : 2/26/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_PSYCHOCHASE_H__
#define __AIGOAL_PSYCHOCHASE_H__

#include "AIGoalChase.h"


class CAIGoalPsychoChase : public CAIGoalChase
{
	typedef CAIGoalChase super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalPsychoChase, kGoal_PsychoChase);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		virtual void	RecalcImportance();

	protected:

		// State Handling.

		virtual void	HandleStateChase();
};


#endif
