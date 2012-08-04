// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalStalk.h
//
// PURPOSE : Stalking is the act of finding a covered location while 
//				traversing a path.  Stalking interrupts the behavior to
//				get the AI to a safe place.  From here, the AI will move 
//				along the path.
//
// CREATED : 5/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOALSTALK_H_
#define __AIGOALSTALK_H_

#include "AIGoalAbstract.h"

class CAIGoalStalk : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalStalk, kGoal_Stalk );
		
		CAIGoalStalk();
		virtual ~CAIGoalStalk();

		// CAIGoalAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	ActivateGoal( );
		virtual void	DeactivateGoal( );

		virtual void	CalculateGoalRelevance();
		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );
		virtual bool	IsPlanValid();

	private:
		LTVector		m_vPendingStalkingPosition;
		LTObjRef		m_hPendingStalkingNode;
		LTObjRef		m_hStalkingNode;
		LTObjRef		m_hPreviousStalkingNode;
};


#endif // __AIGOALSTALK_H_
