// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToWeaponBroke.h
//
// PURPOSE : This class handles AIs reacting to their weapon breaking.  
//
// CREATED : 3/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALREACTTOWEAPONBROKE_H_
#define _AIGOALREACTTOWEAPONBROKE_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalReactToWeaponBroke
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalReactToWeaponBroke : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToWeaponBroke, kGoal_ReactToWeaponBroke );

	// Ctor/Dtor

	CAIGoalReactToWeaponBroke();
	virtual ~CAIGoalReactToWeaponBroke();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	virtual void	CalculateGoalRelevance();
	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalReactToWeaponBroke);
};

#endif // _AIGOALREACTTOWEAPONBROKE_H_
