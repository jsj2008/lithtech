// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDistress.h
//
// PURPOSE : AIGoalDistress class definition
//
// CREATED : 7/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_DISTRESS_H__
#define __AIGOAL_DISTESS_H__

#include "AIGoalAbstractStimulated.h"


class CAIGoalDistress : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalDistress, kGoal_Distress);

		CAIGoalDistress( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateDistress();
		void HandleStatePanic();

	protected:

		LTBOOL				m_bCanActivate;
};


#endif
