// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMountVehicle.h
//
// PURPOSE : AIGoalMountVehicle class definition
//
// CREATED : 12/23/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_MOUNT_VEHICLE_H__
#define __AIAMGOAL_MOUNT_VEHICLE_H__

#include "AIGoalGoto.h"


// ----------------------------------------------------------------------- //

class CAIGoalMountVehicle : public CAIGoalGoto
{
	typedef CAIGoalGoto super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalMountVehicle, kGoal_MountVehicle );
		
		CAIGoalMountVehicle();

		// CAIGoalAbstract overrides.

		virtual void		Save(ILTMessage_Write *pMsg);
        virtual void		Load(ILTMessage_Read *pMsg);

		virtual void		CalculateGoalRelevance();
		virtual void		DeactivateGoal();
		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );
		virtual void		UpdateTaskStatus();

	protected:
};


#endif
