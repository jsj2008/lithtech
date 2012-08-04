// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCircleFlamePot.h
//
// PURPOSE : This goal implements the very constrained flame pot behavior.
//			 In this behavior, AIs attempt to stay out of a hazard area, 
//			 circling the player.  If the player gets too close, the AI 
//			 will lunge in, then dash back out afterwards.  Occasionally 
//			 AIs will dart in to attack on their own.
//
// CREATED : 4/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALCIRCLEFLAMEPOT_H_
#define _AIGOALCIRCLEFLAMEPOT_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalCircleFlamePot
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalCircleFlamePot : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCircleFlamePot, kGoal_CircleFlamePot );

	// Ctor/Dtor

	CAIGoalCircleFlamePot();
	virtual ~CAIGoalCircleFlamePot();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	virtual void CalculateGoalRelevance();
	virtual void SetWSSatisfaction( CAIWorldState& WorldState );
	virtual bool IsWSSatisfied( CAIWorldState* /*pwsWorldState*/ );
	virtual void DeactivateGoal();

private:
	PREVENT_OBJECT_COPYING(CAIGoalCircleFlamePot);
};

#endif // _AIGOALCIRCLEFLAMEPOT_H_
