// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionPickupWeaponInWorld.h
//
// PURPOSE : 
//
// CREATED : 7/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONPICKUPWEAPONINWORLD_H_
#define _AIACTIONPICKUPWEAPONINWORLD_H_

LINKTO_MODULE(AIActionPickupWeaponInWorld);

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodePickupWeaponInWorld
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionPickupWeaponInWorld : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionPickupWeaponInWorld, kAct_PickupWeaponInWorld );

	// Ctor/Dtor

	CAIActionPickupWeaponInWorld();
	virtual ~CAIActionPickupWeaponInWorld();

	// CAIActionAbstract members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions(CAI* pAI, CAIWorldState& wzWorldStateGoal, bool bIsPlanning);
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual void	DeactivateAction( CAI* pAI );
	virtual bool	ValidateAction( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionPickupWeaponInWorld);
};

#endif // _AIACTIONPICKUPWEAPONINWORLD_H_
