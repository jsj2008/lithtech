// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDisappearReappearEvasive.h
//
// PURPOSE : AIGoalDisappearReappearEvasive class definition
//
// CREATED : 11/12/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_DISAPPEAR_REAPPEAR_EVASIVE_H__
#define __AIGOAL_DISAPPEAR_REAPPEAR_EVASIVE_H__

#include "AIGoalDisappearReappear.h"


class CAIGoalDisappearReappearEvasive : public CAIGoalDisappearReappear
{
	typedef CAIGoalDisappearReappear super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDisappearReappearEvasive, kGoal_DisappearReappearEvasive);

		CAIGoalDisappearReappearEvasive();

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		virtual void RecalcImportance();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		virtual void HandleStateDisappearReappear();

		// Disappearance timing.

		void ResetNextDisappearTime();

	protected:

		LTBOOL	m_bFirstTime;
		LTBOOL	m_bForceDisappear;
		LTFLOAT m_fReappearDistOverride;
		LTFLOAT m_fReappearDelay;
};


#endif
