// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalWork.h
//
// PURPOSE : AIGoalWork class definition
//
// CREATED : 7/2/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_WORK_H__
#define __AIGOAL_WORK_H__

#include "AIGoalAbstractUseObject.h"


class CAIGoalWork : public CAIGoalAbstractUseObject
{
	typedef CAIGoalAbstractUseObject super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalWork, kGoal_Work);

		 CAIGoalWork( );
		~CAIGoalWork( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

	protected:

		// Special deactivation handling.

		LTBOOL HandleSpecialDamageDeactivation();
		LTBOOL HandleWorkInterruptionDeactivation();

	protected:

		enum EnumWorkDeactivationState
		{
			kWDS_None,
			kWDS_Interrupting,
			kWDS_Transitioning,
			kWDS_DoneTransitioning,
		};

	protected:
	
		EnumWorkDeactivationState	m_eDeactivationState;
};

//--------------------------------------------------------------------------

// The following classes are identical in every way to GoalWork.
// They only exist to split up what an AI is attracted to.
// They have different attractors in aigoals.txt
// Some are derived from CAIGoalAbstractUseObject instead of work, because
// we do not want AI to holster weapons or toggle lights.

class CAIGoalPlacePoster : public CAIGoalWork
{
	public:	DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalPlacePoster, kGoal_PlacePoster);
};

class CAIGoalEnjoyPoster : public CAIGoalWork
{
	public:	DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalEnjoyPoster, kGoal_EnjoyPoster);
};

class CAIGoalDestroy : public CAIGoalAbstractUseObject
{
	public:	DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDestroy, kGoal_Destroy);
};

class CAIGoalRide : public CAIGoalAbstractUseObject
{
	public:	DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRide, kGoal_Ride);
};

#endif
