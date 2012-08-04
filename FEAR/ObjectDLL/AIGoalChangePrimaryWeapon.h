// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChangePrimaryWeapon.h
//
// PURPOSE : This goal handles prompting the AI to change weapons when the 
//			 AIs' primary weapon has no ammo.  For instance, when an AI 
//			 runs out of shells for his shotgun, he should flip it over 
//			 and whack people with it.  This goal prompts the change from
//			 ranged to melee, in this case.
//
// CREATED : 6/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALCHANGEPRIMARYWEAPON_H_
#define _AIGOALCHANGEPRIMARYWEAPON_H_

LINKTO_MODULE(AIGoalChangePrimaryWeapon);


#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalChangePrimaryWeapon
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalChangePrimaryWeapon : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalChangePrimaryWeapon, kGoal_ChangePrimaryWeapon );

	// Ctor/Dtor

	CAIGoalChangePrimaryWeapon();
	virtual ~CAIGoalChangePrimaryWeapon();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	// CAIGoalAbstract

	virtual void	CalculateGoalRelevance();
	virtual void	SetWSSatisfaction( CAIWorldState& /*WorldState*/ );
	virtual bool	IsWSSatisfied( CAIWorldState* /*pwsWorldState*/ );

private:
	PREVENT_OBJECT_COPYING(CAIGoalChangePrimaryWeapon);
};


#endif // _AIGOALCHANGEPRIMARYWEAPON_H_

