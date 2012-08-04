// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTargetIsAimingAtMeParanoid.h
//
// PURPOSE : Paranoid derived version of sensor. 
//           AI always thinks someone is aimaing at him when he's moving.
//
// CREATED : 4/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSORTARGETISAIMINGATME_PARANOID_H_
#define __AISENSORTARGETISAIMINGATME_PARANOID_H_

#include "AISensorTargetIsAimingAtMe.h"

class CAISensorTargetIsAimingAtMeParanoid : public CAISensorTargetIsAimingAtMe
{
	typedef CAISensorTargetIsAimingAtMe super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTargetIsAimingAtMeParanoid, kSensor_TargetIsAimingAtMeParanoid );

		// Updating.

		virtual bool	UpdateSensor();
};

#endif // __AISENSORTARGETISAIMINGATME_PARANOID_H_
