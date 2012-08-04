// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountVehicle.h
//
// PURPOSE : AIActionDismountVehicle class definition
//
// CREATED : 12/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_DISMOUNT_VEHICLE_H__
#define __AIACTION_DISMOUNT_VEHICLE_H__

#include "AIActionAbstract.h"


class CAIActionDismountVehicle : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountVehicle, kAct_DismountVehicle );

		CAIActionDismountVehicle();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
