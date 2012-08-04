// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTargetIsAimingAtMeMelee.h
//
// PURPOSE : Contains the declaration for a sensor which detects the enemy 
//				aiming at the owner.
//
// CREATED : 10/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSORTARGETISAIMINGATME_MELEE_H_
#define __AISENSORTARGETISAIMINGATME_MELEE_H_

#include "AISensorTargetIsAimingAtMe.h"

class CAISensorTargetIsAimingAtMeMelee : public CAISensorTargetIsAimingAtMe
{
	typedef CAISensorTargetIsAimingAtMe super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTargetIsAimingAtMeMelee, kSensor_TargetIsAimingAtMeMelee );

private:

		virtual bool TargetHasDangerousWeapon();
};

#endif // __AISENSORTARGETISAIMINGATME_MELEE_H_
