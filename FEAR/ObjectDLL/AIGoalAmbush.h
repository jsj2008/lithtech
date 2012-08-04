// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAmbush.h
//
// PURPOSE : AIGoalAmbush class definition
//
// CREATED : 3/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_AMBUSH_H__
#define __AIAMGOAL_AMBUSH_H__

#include "AIGoalGoto.h"

// Forward declarations.


// ----------------------------------------------------------------------- //

class CAIGoalAmbush : public CAIGoalGoto
{
	typedef CAIGoalGoto super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalAmbush, kGoal_Ambush );

		CAIGoalAmbush();
		virtual ~CAIGoalAmbush();

		// CAIGoalAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	CalculateGoalRelevance();
		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );

		virtual void	ActivateGoal();
		virtual void	DeactivateGoal();

		virtual void	UpdateTaskStatus();

	protected:

		bool	m_bThreatSighted;
};


#endif
