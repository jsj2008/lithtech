// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromVehicleMelee.h
//
// PURPOSE : This class handles performing melee attacks while riding a 
//			 vehicle.  This makes slightly different assumptions about the
//			 animation to be played than AIActionAttackFromVehicle (which 
//			 was intended for AIs attacking from motorcycles)
//
// CREATED : 3/22/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONATTACKFROMVEHICLEMELEE_H_
#define _AIACTIONATTACKFROMVEHICLEMELEE_H_

#include "AIActionAttackMelee.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionAttackFromVehicleMelee
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionAttackFromVehicleMelee : public CAIActionAttackMelee
{
	typedef CAIActionAttackMelee super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromVehicleMelee, kAct_AttackFromVehicleMelee );

	// Ctor/Dtor

	CAIActionAttackFromVehicleMelee();
	virtual ~CAIActionAttackFromVehicleMelee();

	virtual bool ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void SetAttackAnimProps( CAI* pAI, CAnimationProps* pProps );

private:
	PREVENT_OBJECT_COPYING(CAIActionAttackFromVehicleMelee);
};

#endif // _AIACTIONATTACKFROMVEHICLEMELEE_H_
