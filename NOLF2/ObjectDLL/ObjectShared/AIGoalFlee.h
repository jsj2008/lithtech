// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFlee.h
//
// PURPOSE : AIGoalFlee class definition
//
// CREATED : 7/24/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_FLEE_H__
#define __AIGOAL_FLEE_H__

#include "AIGoalAbstractStimulated.h"


// Forward Declarations.
class AINode;



class CAIGoalFlee : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalFlee, kGoal_Flee);

		CAIGoalFlee( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Relax.

		void Relax();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void HandleStateFlee();
		void HandleStatePanic();
		void HandleStateGoto();

	protected:

        LTObjRef	m_hDestNode;
		LTObjRef	m_hDangerObject;
		LTFLOAT		m_fRelaxTime;
};


#endif
