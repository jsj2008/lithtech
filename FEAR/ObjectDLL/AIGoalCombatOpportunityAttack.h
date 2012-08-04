// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCombatOpportunity.h
//
// PURPOSE : 
//
// CREATED : 6/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALCOMBATOPPORTUNITYATTACK_H_
#define _AIGOALCOMBATOPPORTUNITYATTACK_H_

LINKTO_MODULE(AIGoalCombatOpportunityAttack);

#include "AIGoalAbstract.h"

class CAIWorldState;

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalCombatOpportunityAttack
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalCombatOpportunityAttack : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCombatOpportunityAttack, kGoal_CombatOpportunityAttack );

	// Ctor/Dtor

	CAIGoalCombatOpportunityAttack();
	virtual			~CAIGoalCombatOpportunityAttack();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	// CAIGoalAbstract overrides 

	virtual void	CalculateGoalRelevance();
	virtual void	HandleBuildPlanFailure();
	virtual void	ActivateGoal();
	virtual void	DeactivateGoal();
	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
	virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalCombatOpportunityAttack);
};

#endif // _AIGOALCOMBATOPPORTUNITY_H_
