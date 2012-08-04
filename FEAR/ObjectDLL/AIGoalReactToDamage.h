// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToDamage.h
//
// PURPOSE : AIGoalReactToDamage class definition
//
// CREATED : 11/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_REACT_TO_DAMAGE_H__
#define __AIAMGOAL_REACT_TO_DAMAGE_H__

#include "AIGoalAbstract.h"


// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalReactToDamage : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToDamage, kGoal_ReactToDamage );
		
		CAIGoalReactToDamage();
		virtual ~CAIGoalReactToDamage();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAIGoalAbstract overrides.

		virtual void		CalculateGoalRelevance();
		virtual bool		ReplanRequired();
		virtual void		HandleBuildPlanFailure();
		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:
	
		double		m_fCurrentDamageTime;
		double		m_fLastDamageTime;
		
		// If true, replans will be allowed while the AI has currently 
		// executing this goal. If false, replans are only allowed when the
		// goal is not the AI's current goal.
		bool		m_bAllowReplanWhenCurGoal;

		// This enum contains the event this goal must satisfy for planning purposes.
		// By default, this value is kWSE_Damage.  To be satisfied, this event must
		// be contained in the AIs worldstate.
		ENUM_AIWorldStateEvent m_eWorldStateEvent;

		// If this value is true, this goal will play a blended recoil if this goal 
		// fails to successfully build a plan.  By default, this is set to true.
		bool		m_bPlayRecoilOnBuildPlanFailure;
};


#endif
