// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCharge.h
//
// PURPOSE : AIGoalCharge class definition
//
// CREATED : 7/25/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_CHARGE_H__
#define __AIGOAL_CHARGE_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalCharge : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalCharge, kGoal_Charge);

		CAIGoalCharge( );

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

		void HandleStateCharge();

	protected:

		LTFLOAT				m_fAttackDistanceSqr;
		LTFLOAT				m_fYellDistanceSqr;
		LTFLOAT				m_fStopDistanceSqr;
};


#endif
