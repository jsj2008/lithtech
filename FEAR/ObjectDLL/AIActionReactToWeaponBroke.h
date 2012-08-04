// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionReactToWeaponBroke.h
//
// PURPOSE : This class handles playing a reaction animation in response
//			 to the AIs weapon breaking.
//
// CREATED : 3/25/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONREACTTOWEAPONBROKE_H_
#define _AIACTIONREACTTOWEAPONBROKE_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionReactToWeaponBroke
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionReactToWeaponBroke : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionReactToWeaponBroke, kAct_ReactToWeaponBroke );

	// Ctor/Dtor

	CAIActionReactToWeaponBroke();
	virtual ~CAIActionReactToWeaponBroke();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionReactToWeaponBroke);
};

#endif // _AIACTIONREACTTOWEAPONBROKE_H_
