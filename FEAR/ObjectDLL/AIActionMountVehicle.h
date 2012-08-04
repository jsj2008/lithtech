// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMountVehicle.h
//
// PURPOSE : AIActionMountVehicle class definition
//
// CREATED : 12/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_MOUNT_VEHICLE_H__
#define __AIACTION_MOUNT_VEHICLE_H__

#include "AIActionUseSmartObjectNode.h"


class CAIActionMountVehicle : public CAIActionUseSmartObjectNode
{
	typedef CAIActionUseSmartObjectNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMountVehicle, kAct_MountVehicle );

		// CAIActionAbstract members.

		virtual bool			ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void			ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void			ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
};

// ----------------------------------------------------------------------- //

#endif
