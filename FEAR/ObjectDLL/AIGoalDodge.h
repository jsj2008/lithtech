// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDodge.h
//
// PURPOSE : AIGoalDodge class definition
//
// CREATED : 3/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_DODGE_H__
#define __AIAMGOAL_DODGE_H__

#include "AIGoalAbstract.h"


// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalDodge : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalDodge, kGoal_Dodge );
		
		CAIGoalDodge();
		virtual ~CAIGoalDodge();

		// CAIGoalAbstract overrides.

		virtual void		SetNextRecalcTime();
		virtual double		GetNextRecalcTime();
		virtual void		CalculateGoalRelevance();
		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();

		virtual float		GetActivateChance() const;

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:

		// This value never changes, so does not need to be saved.
		bool				m_bAlwaysDodgeWhenShot;
};


#endif
