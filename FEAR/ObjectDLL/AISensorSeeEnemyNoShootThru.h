// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeEnemyNoShootThru.h
//
// PURPOSE : AISensorSeeEnemyNoShootThru class definition
//
// CREATED : 7/13/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SEE_ENEMY_NO_SHOOT_THRU_H__
#define __AISENSOR_SEE_ENEMY_NO_SHOOT_THRU_H__

#include "AISensorSeeEnemy.h"


class CAISensorSeeEnemyNoShootThru : public CAISensorSeeEnemy
{
	typedef CAISensorSeeEnemy super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeEnemyNoShootThru, kSensor_SeeEnemyNoShootThru );

	protected:

		virtual bool		CheckVisibility( HOBJECT hObject, const LTVector& vPosition, float fSenseDistanceSqr, float* pfVisDistanceSqr );
};

#endif
