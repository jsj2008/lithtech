// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeEnemyNoFOV.cpp
//
// PURPOSE : AISensorSeeEnemyNoFOV class implementation
//
// CREATED : 9/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSeeEnemyNoFOV.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeEnemyNoFOV, kSensor_SeeEnemyNoFOV );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeEnemyNoFOV::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorSeeEnemyNoFOV::CAISensorSeeEnemyNoFOV()
{
	m_bUseFOV = false;
}

