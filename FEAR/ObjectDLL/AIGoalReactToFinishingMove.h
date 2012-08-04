// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToFinishingMove.h
//
// PURPOSE : 
//
// CREATED : 3/17/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALREACTTOFINISHINGMOVE_H_
#define _AIGOALREACTTOFINISHINGMOVE_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalReactToFinishingMove
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalReactToFinishingMove : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToFinishingMove, kGoal_ReactToFinishingMove );

	// Ctor/Dtor

	CAIGoalReactToFinishingMove();
	virtual ~CAIGoalReactToFinishingMove();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
	virtual void	CalculateGoalRelevance();

private:
	PREVENT_OBJECT_COPYING(CAIGoalReactToFinishingMove);
};

#endif // _AIGOALREACTTOFINISHINGMOVE_H_
