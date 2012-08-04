// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTargetIsAimingAtMeMelee.cpp
//
// PURPOSE : Contains the implementation for the sensor which detects 
//				if the target is aiming at the owner.
//
// CREATED : 10/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorTargetIsAimingAtMeMelee.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTargetIsAimingAtMeMelee, kSensor_TargetIsAimingAtMeMelee );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTargetIsAimingAtMe::TargetHasDangerousWeapon()
//
//	PURPOSE:	Returns true if the target characters current weapon 
//				is dangerous, else returns false.  
//
// ----------------------------------------------------------------------- //

bool CAISensorTargetIsAimingAtMeMelee::TargetHasDangerousWeapon()
{
	if( super::TargetHasDangerousWeapon() )
	{
		return true;
	}

	// AI does not have a weapon of the correct type

	if( !AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Melee, CHECK_HOLSTER ) )
	{
		return false;
	}

	// Target is not in range.

	if( !AIWeaponUtils::IsInRange( m_pAI, kAIWeaponType_Melee, CHECK_HOLSTER ) )
	{
		return false;
	}

	// Target is in melee range.

	return true;
}
