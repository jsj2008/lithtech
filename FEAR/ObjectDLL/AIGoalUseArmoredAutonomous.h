// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalUseArmoredAutonomous.h
//
// PURPOSE :	Contains the declaration of the UseArmoredAutonomous Goal.   
//
//				This goal allows an AI to autonomously find a valid Armored node.  
//
// CREATED : 6/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOALUSEARMOREDAUTONOMOUS_H_
#define __AIGOALUSEARMOREDAUTONOMOUS_H_

#include "AIGoalUseSmartObjectCombat.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalUseArmoredAutonomous
//
//	PURPOSE:	Handles an AI deciding to go to an armored node without 
//				prompting from a scripted command (such as from a designer 
//				or squad)
//
// ----------------------------------------------------------------------- //

class CAIGoalUseArmoredAutonomous : public CAIGoalUseSmartObjectCombat
{
	typedef CAIGoalUseSmartObjectCombat super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalUseArmoredAutonomous, kGoal_UseArmoredAutonomous );

	CAIGoalUseArmoredAutonomous();
	virtual ~CAIGoalUseArmoredAutonomous();

	// CAIGoalAbstract overrides.

	virtual void CalculateGoalRelevance();
	virtual bool IsWSSatisfied( CAIWorldState* pwsWorldState );
};

#endif // __AIGOALUSEARMOREDAUTONOMOUS_H_
