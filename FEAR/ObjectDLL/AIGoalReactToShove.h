// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToShove.h
//
// PURPOSE : 
//
// CREATED : 11/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALREACTTOSHOVE_H_
#define _AIGOALREACTTOSHOVE_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalReactToShove
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalReactToShove : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToShove, kGoal_ReactToShove );

	// Ctor/Dtor

	CAIGoalReactToShove();
	virtual ~CAIGoalReactToShove();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	virtual void	CalculateGoalRelevance();
	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalReactToShove);
};

#endif // _AIGOALREACTTOSHOVE_H_
