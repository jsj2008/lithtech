// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAssassinate.h
//
// PURPOSE : AIGoalAssassinate class definition
//
// CREATED : 7/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ASSASSINATE_H__
#define __AIGOAL_ASSASSINATE_H__

#include "AIGoalAbstractTargeted.h"

// Forward declarations.
class AINodeAssassinate;


class CAIGoalAssassinate : public CAIGoalAbstractTargeted
{
	typedef CAIGoalAbstractTargeted super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAssassinate, kGoal_Assassinate);

		CAIGoalAssassinate( );
		
		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();

		// Command Handling.

		virtual LTBOOL	HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Handling.

		void HandleStateAssassinate();

	protected:


		LTObjRef			m_hNodeAssassinate;
		LTBOOL				m_bNewTarget;
};


#endif
