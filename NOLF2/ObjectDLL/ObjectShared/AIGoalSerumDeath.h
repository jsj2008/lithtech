// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSerumDeath.h
//
// PURPOSE : AIGoalSerumDeath class definition
//
// CREATED : 2/13/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SERUM_DEATH_H__
#define __AIGOAL_SERUM_DEATH_H__

#include "AIGoalAbstract.h"
#include "AnimationMgr.h"


class CAIGoalSerumDeath : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSerumDeath, kGoal_SerumDeath);

		CAIGoalSerumDeath( );

		// Initialization.

		virtual void	InitGoal(CAI* pAI);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

	protected:

		// State Handling.

		void HandleStateAnimate();

	protected:

		CAnimationProps		m_animProps;

		LTFLOAT				m_fSleepTimeMin;
		LTFLOAT				m_fSleepTimeMax;
		LTFLOAT				m_fSleepTimer;
};


#endif
