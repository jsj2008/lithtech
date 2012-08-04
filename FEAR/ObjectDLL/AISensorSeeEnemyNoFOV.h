// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeEnemy.h
//
// PURPOSE : AISensorSeeEnemyNoFOV class definition
//
// CREATED : 9/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SEE_ENEMY_NO_FOV_H__
#define __AISENSOR_SEE_ENEMY_NO_FOV_H__

#include "AISensorSeeEnemy.h"


class CAISensorSeeEnemyNoFOV : public CAISensorSeeEnemy
{
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeEnemyNoFOV, kSensor_SeeEnemyNoFOV );

		CAISensorSeeEnemyNoFOV();
};

#endif
