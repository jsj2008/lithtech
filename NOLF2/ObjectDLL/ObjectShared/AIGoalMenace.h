// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMenace.h
//
// PURPOSE : AIGoalMenace class definition
//
// CREATED : 7/20/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_MENACE_H__
#define __AIGOAL_MENACE_H__

#include "AIGoalAbstractUseObject.h"


class CAIGoalMenace : public CAIGoalAbstractUseObject
{
	typedef CAIGoalAbstractUseObject super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalMenace, kGoal_Menace);

		CAIGoalMenace( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();

	protected:

		// State Handling.

		virtual void SetStateUseObject();
		virtual void HandleStateDraw();
};


#endif
