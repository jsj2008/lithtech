// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalFollow.h
//
// PURPOSE : AIGoalFollow class definition
//
// CREATED : 4/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_FOLLOW_H__
#define __AIAMGOAL_FOLLOW_H__

#include "AIGoalAbstract.h"
#include "AIWorkingMemory.h"

// Forward declarations.

class	CAIWMFact;


// Goto Request Struct.

struct SAI_FOLLOW_REQUEST
{
	SAI_FOLLOW_REQUEST();

	ENUM_FactID		eFactID;
	LTObjRef		hFollow;
};

// ----------------------------------------------------------------------- //

class CAIGoalFollow : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalFollow, kGoal_Follow );
		
		CAIGoalFollow();
		virtual ~CAIGoalFollow();

		// CAIGoalAbstract overrides.

		virtual void		Save(ILTMessage_Write *pMsg);
        virtual void		Load(ILTMessage_Read *pMsg);

		virtual void		CalculateGoalRelevance();
		virtual bool		ReplanRequired();

		virtual void		ActivateGoal();
		virtual void		DeactivateGoal();

		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );

	protected:

		SAI_FOLLOW_REQUEST	m_FollowCurrent;
		SAI_FOLLOW_REQUEST	m_FollowPending;
};


#endif
