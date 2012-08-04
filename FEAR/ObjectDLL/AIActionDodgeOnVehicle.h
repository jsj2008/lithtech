// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDodgeOnVehicle.h
//
// PURPOSE : AIActionDodgeOnVehicle class definition
//
// CREATED : 01/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DODGE_ON_VEHICLE_H__
#define __AIACTION_DODGE_ON_VEHICLE_H__

#include "AIActionAbstract.h"


// Forward declarations.

class	CAnimationProps;


class CAIActionDodgeOnVehicle : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDodgeOnVehicle, kAct_DodgeOnVehicle );

		CAIActionDodgeOnVehicle();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
