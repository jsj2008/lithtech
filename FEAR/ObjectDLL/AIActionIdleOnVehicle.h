// ----------------------------------------------------------------------- //
//
// MODULE  : AIIdleOnVehicle.h
//
// PURPOSE : AIIdleOnVehicle class definition
//
// CREATED : 12/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_IDLE_ON_VEHICLE_H__
#define __AIACTION_IDLE_ON_VEHICLE_H__

#include "AIActionIdle.h"


class CAIActionIdleOnVehicle : public CAIActionIdle
{
	typedef CAIActionIdle super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionIdleOnVehicle, kAct_IdleOnVehicle );

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
