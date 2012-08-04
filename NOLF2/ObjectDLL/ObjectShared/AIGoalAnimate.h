// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAnimate.h
//
// PURPOSE : AIGoalAnimate class definition
//
// CREATED : 7/30/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ANIMATE_H__
#define __AIGOAL_ANIMATE_H__

#include "AIGoalAbstract.h"


class CAIGoalAnimate : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAnimate, kGoal_Animate);

		CAIGoalAnimate( );
		~CAIGoalAnimate( );

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

		void HandleStateAnimate();

	protected:

		LTBOOL		m_bLoop;
		HSTRING		m_hstrAnim;
		LTBOOL		m_bResetAnim;
};


#endif
