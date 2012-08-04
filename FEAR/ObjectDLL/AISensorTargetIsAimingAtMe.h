// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTargetIsAimingAtMe.h
//
// PURPOSE : Contains the declaration for a sensor which detects the enemy 
//				aiming at the owner.
//
// CREATED : 4/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSORTARGETISAIMINGATME_H_
#define __AISENSORTARGETISAIMINGATME_H_

#include "AISensorAbstract.h"

class CAISensorTargetIsAimingAtMe : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTargetIsAimingAtMe, kSensor_TargetIsAimingAtMe );

		// Updating.

		virtual bool	UpdateSensor();

	protected:

		virtual bool TargetHasDangerousWeapon();
		bool TargetIsLookingAtMe();
};

#endif // __AISENSORTARGETISAIMINGATME_H_
