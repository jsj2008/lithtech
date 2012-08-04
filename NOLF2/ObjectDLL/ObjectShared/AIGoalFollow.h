// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollow.h
//
// PURPOSE : AIGoalFollow class definition
//
// CREATED : 7/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FOLLOW_H__
#define __AIGOAL_FOLLOW_H__

#include "AIGoalAbstractTargeted.h"
#include "AnimationProp.h"

class CAIGoalFollow : public CAIGoalAbstractTargeted
{
	typedef CAIGoalAbstractTargeted super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFollow, kGoal_Follow);

		CAIGoalFollow( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Handling.

		void HandleStateFollow();
		void HandleStatePanic();

	protected:

		LTBOOL				m_bPanicCanActivate;
		EnumAnimProp		m_eMovement;
		LTFLOAT				m_fRangeTime;
		LTFLOAT				m_fRangeSqr;
};


#endif
