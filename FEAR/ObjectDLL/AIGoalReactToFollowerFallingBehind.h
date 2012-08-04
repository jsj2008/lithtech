// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToFollowerFallingBehind.h
//
// PURPOSE : 
//
// CREATED : 4/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALREACTTOFOLLOWERFALLINGBEHIND_H_
#define _AIGOALREACTTOFOLLOWERFALLINGBEHIND_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalReactToFollowerFallingBehind
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalReactToFollowerFallingBehind : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToFollowerFallingBehind, kGoal_ReactToFollowerFallingBehind );

	// Ctor/Dtor

	CAIGoalReactToFollowerFallingBehind();
	virtual ~CAIGoalReactToFollowerFallingBehind();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	virtual void CalculateGoalRelevance();
	virtual void SetWSSatisfaction( CAIWorldState& WorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalReactToFollowerFallingBehind);
};

#endif // _AIGOALREACTTOFOLLOWERFALLINGBEHIND_H_
