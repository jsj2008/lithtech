// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSlowMo.cpp
//
// PURPOSE : AISensorSlowMo class implementation
//
// CREATED : 4/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSlowMo.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSlowMo, kSensor_SlowMo );

#define SLOWMO_RECORD				"AlmaSlowMo"
#define TIME_SCALE					"AlmaMultiplier"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSlowMo::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorSlowMo::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Speed up Alma when SlowMo is in progress.

	HRECORD hSlowMoCurrent = g_pGameServerShell->GetSlowMoRecord();
	if( hSlowMoCurrent )
	{
		HRECORD hSlowMoRequired = g_pAIDB->GetMiscRecordLink( SLOWMO_RECORD );
		if( hSlowMoCurrent == hSlowMoRequired )
		{
			float fRate = g_pAIDB->GetMiscFloat( TIME_SCALE );
			m_pAI->GetAnimationContext()->SetOverrideAnimRate( fRate );
			return false;
		}
	}

	// Return to normal speed.

	m_pAI->GetAnimationContext()->ClearOverrideAnimRate();

	// Always allow other sensors to update.

	return false;
}


