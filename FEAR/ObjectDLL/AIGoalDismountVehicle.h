// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalDismountVehicle.h
//
// PURPOSE : AIGoalDismountVehicle class definition
//
// CREATED : 12/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_DISMOUNT_VEHICLE_H__
#define __AIAMGOAL_DISMOUNT_VEHICLE_H__

#include "AIGoalAbstract.h"
#include "AIWorkingMemory.h"

// Forward declarations.

class	CAIWMFact;

// ----------------------------------------------------------------------- //

class CAIGoalDismountVehicle : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalDismountVehicle, kGoal_DismountVehicle );
		
		CAIGoalDismountVehicle();

		// CAIGoalAbstract overrides.

		virtual void		Save(ILTMessage_Write *pMsg);
        virtual void		Load(ILTMessage_Read *pMsg);

		virtual void		CalculateGoalRelevance();
		virtual void		ActivateGoal();
		virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool		IsWSSatisfied( CAIWorldState* pwsWorldState );
		virtual void		UpdateTaskStatus();
};


#endif
