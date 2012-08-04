// ----------------------------------------------------------------------- //
//
// MODULE  :	AIGoalBlitzCharacter.h
//
// PURPOSE :	This goal handles 'blitzing' characters.  In this behavior, 
//				the blitzing AI will run to the characters position using 
//				a specified movement animation using the specified 
//				AIContext to specify the movement animation.
//
// CREATED :	2/26/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALBLITZCHARACTER_H_
#define _AIGOALBLITZCHARACTER_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalBlitzCharacter
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalBlitzCharacter : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalBlitzCharacter, kGoal_BlitzCharacter );

	// Ctor/Dtor

	CAIGoalBlitzCharacter();
	virtual ~CAIGoalBlitzCharacter();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Template methods

	virtual void CalculateGoalRelevance();
	virtual void ActivateGoal();
	virtual void DeactivateGoal();
	virtual void HandleBuildPlanFailure();
	virtual void SetWSSatisfaction( CAIWorldState& WorldState );
	virtual bool IsWSSatisfied( CAIWorldState* pwsWorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalBlitzCharacter);

	virtual EnumAIContext OnGetContext() const;

	// Store the goal Context from the task which activated this goal.
	EnumAIContext m_eActiveGoalContext;
};

#endif // _AIGOALBLITZCHARACTER_H_
