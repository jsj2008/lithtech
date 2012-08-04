// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSleep.h
//
// PURPOSE : AIGoalSleep class definition
//
// CREATED : 7/11/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SLEEP_H__
#define __AIGOAL_SLEEP_H__

#include "AIGoalWork.h"


class CAIGoalSleep : public CAIGoalWork
{
	typedef CAIGoalWork super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSleep, kGoal_Sleep);

		CAIGoalSleep( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		virtual void UpdateGoal();
		
		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);
};


#endif
