// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDeathOnVehicleUnanimated.h
//
// PURPOSE : This action handles dying while on a vehicle, in the case 
//			 where we want the AI to receive his velocity from the 
//			 worldmodel he is attached to more than the animation he was 
//			 playing or via scripted means.
//
// CREATED : 3/23/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONDEATHONVEHICLEUNANIMATED_H_
#define _AIACTIONDEATHONVEHICLEUNANIMATED_H_

#include "AIActionAbstract.h"

// Forward declarations.

class CAIActionDeathOnVehicleUnanimated : public CAIActionAbstract
{
typedef CAIActionAbstract super;

public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDeathOnVehicleUnanimated, kAct_DeathOnVehicleUnanimated );

	CAIActionDeathOnVehicleUnanimated();

	// CAIActionAbstract members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual	bool	IsActionComplete( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

#endif // _AIACTIONDEATHONVEHICLEUNANIMATED_H_
