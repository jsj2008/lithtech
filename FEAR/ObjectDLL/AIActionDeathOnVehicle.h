// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDeathOnVehicle.h
//
// PURPOSE : AIActionDeathOnVehicle class definition
//
// CREATED : 01/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DEATH_ON_VEHICLE_H__
#define __AIACTION_DEATH_ON_VEHICLE_H__

#include "AIActionAbstract.h"


// Forward declarations.

class	CAnimationProps;


class CAIActionDeathOnVehicle : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDeathOnVehicle, kAct_DeathOnVehicle );

		CAIActionDeathOnVehicle();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
	
	protected:

		void			GetDeathAnimProps( CAI* pAI, CAnimationProps* pAnimProps, EnumAnimProp eVehicle );
};

// ----------------------------------------------------------------------- //

#endif
