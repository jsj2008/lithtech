// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDramaDeath.h
//
// PURPOSE : AIGoalDramaDeath class definition
//
// CREATED : 2/22/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_DRAMA_DEATH_H__
#define __AIGOAL_DRAMA_DEATH_H__

#include "AIGoalAbstract.h"
#include "AnimationMgr.h"


class CAIGoalDramaDeath : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDramaDeath, kGoal_DramaDeath);

		CAIGoalDramaDeath( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Handling.

		void HandleStateLaunch();
		void HandleStateGoto();
		void HandleStateAnimate();

		// Death Animation.

		void SetDeathAnimation();

	protected:

		LTObjRef	m_hDamager;
		LTObjRef	m_hNodeDeath;
		LTBOOL		m_bLaunch;
};


#endif
