// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalGoto.h
//
// PURPOSE : AIGoalGoto class definition
//
// CREATED : 8/07/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_GOTO_H__
#define __AIGOAL_GOTO_H__

#include "AIGoalAbstract.h"
#include "AnimationProp.h"


class CAIGoalGoto : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalGoto, kGoal_Goto);

		CAIGoalGoto( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Handling.

		void HandleStateGoto();

	protected:

		LTObjRef		m_hDestNode;
		EnumAnimProp	m_eMovement;
		EnumAnimProp	m_eAwareness;
};


#endif
