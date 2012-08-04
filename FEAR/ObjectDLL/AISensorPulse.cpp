// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPulse.cpp
//
// PURPOSE : AISensorPulse class implementation
//
// CREATED : 4/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

// Includes required for AISensorPulse.h

#include "Stdafx.h"
#include "AISensorAbstract.h"
#include "AISensorPulse.h"

// Includes required for AISensorPulse.cpp

#include "AI.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPulse, kSensor_Pulse );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPulse::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorPulse::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// The only purpose of this sensor is to periodically send
	// a signal that goals should be re-evaluated, even in the
	// absence of actually sensing anything.

	// Flag that goals should be re-evaluated.

	m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );

	// Always allow other sensors to update.

	return false;
}


