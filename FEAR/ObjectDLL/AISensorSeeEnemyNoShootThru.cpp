// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeEnemyNoShootThru.cpp
//
// PURPOSE : AISensorSeeEnemyNoShootThru class implementation
//
// CREATED : 7/13/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSeeEnemyNoShootThru.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeEnemyNoShootThru, kSensor_SeeEnemyNoShootThru );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemyNoShootThru::CheckVisibility
//
//	PURPOSE:	Return true if the object position is visible.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeEnemyNoShootThru::CheckVisibility( HOBJECT hObject, const LTVector& vPosition, float fSenseDistanceSqr, float* pfVisDistanceSqr )
{
	// This class and virtual function was necessary to support Turrets not trying 
	// to fire through bullet proof glass.  The normal channels fail on turrets, 
	// because they are setup to blind fire when aware of a target that it cannot 
	// fire at.

	bool bVisible;
	float fVisDistanceSqr;
	if ( m_pAI->CanSeeThrough() )
	{
		bVisible = !!m_pAI->IsObjectPositionVisible( CAI::ShootThroughFilterFn, CAI::ShootThroughPolyFilterFn, m_pAI->GetEyePosition(), hObject, vPosition, fSenseDistanceSqr, m_bUseFOV, false, NULL, &fVisDistanceSqr );
	}
	else
	{
		bVisible = !!m_pAI->IsObjectPositionVisible( CAI::DefaultFilterFn, NULL, m_pAI->GetEyePosition(), hObject, vPosition, fSenseDistanceSqr, m_bUseFOV, false, NULL, &fVisDistanceSqr );
	}

	if( pfVisDistanceSqr )
	{
		*pfVisDistanceSqr = fVisDistanceSqr;
	}
	return bVisible;
}

