// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAnimate.h
//
// PURPOSE : AIGoalAnimate class definition
//
// CREATED : 4/17/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_ANIMATE_H__
#define __AIAMGOAL_ANIMATE_H__

#include "AIGoalAbstract.h"
#include "AIWorkingMemory.h"

// Forward declarations.

class CAIWMFact;


// Animation Request Struct.

struct SAI_ANIMATION_REQUEST
{
	SAI_ANIMATION_REQUEST();

	ENUM_FactID		eFactID;
	HMODELANIM		hAni;
	bool			bLoop;
};

// ----------------------------------------------------------------------- //

class CAIGoalAnimate : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalAnimate, kGoal_Animate );
		
		CAIGoalAnimate();
		~CAIGoalAnimate();

		// CAIGoalAbstract overrides.

		virtual void		Save(ILTMessage_Write *pMsg);
		virtual void		Load(ILTMessage_Read *pMsg);

		virtual void		CalculateGoalRelevance();
		virtual bool		ReplanRequired();

		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );

		virtual void		UpdateTaskStatus();

	protected:

		virtual CAIWMFact* GetAnimateTaskFact();
		virtual CAIWMFact* GetAnimateLoopTaskFact();

		SAI_ANIMATION_REQUEST	m_AnimCurrent;
		SAI_ANIMATION_REQUEST	m_AnimPending;
};


#endif
