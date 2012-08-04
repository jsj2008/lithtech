// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToBerserkerKick.h
//
// PURPOSE : 
//
// CREATED : 3/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALREACTTOBERSERKERKICK_H_
#define _AIGOALREACTTOBERSERKERKICK_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalReactToBerserkerKick
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalReactToBerserkerKick : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToBerserkerKick, kGoal_ReactToBerserkerKick );

	// Ctor/Dtor

	CAIGoalReactToBerserkerKick();
	virtual ~CAIGoalReactToBerserkerKick();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	virtual void	ActivateGoal();
	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
	virtual void	CalculateGoalRelevance();

private:
	PREVENT_OBJECT_COPYING(CAIGoalReactToBerserkerKick);
};

#endif // _AIGOALREACTTOBerserkerKick_H_
