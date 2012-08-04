// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorScanner.cpp
//
// PURPOSE : AISensorScanner class implementation
//
// CREATED : 09/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorScanner.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorScanner, kSensor_Scanner );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorScanner::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorScanner::CAISensorScanner()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorScanner::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorScanner::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorScanner::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorScanner::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorScanner::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorScanner::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Reset to default behavior if not targeting anyone.

	uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();
	if( !m_pAI->HasTarget( kTarget_All ) )
	{
		dwFlags |= kTargetTrack_Normal;
	}

	// Reset to default behavior if target just changed.

	else if( m_pAI->GetAIBlackBoard()->GetBBTargetChangeTime() + 1.f > g_pLTServer->GetTime() )
	{
		dwFlags |= kTargetTrack_Normal;
	}

	// Keep tracking if we see our target.

	else if( m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		dwFlags |= kTargetTrack_Normal;
	}

	// Stop tracking if we've lost our target.

	else 
	{
		// Re-aquire the target if it has come fully into view.

		HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Character );
		factQuery.SetTargetObject( hTarget );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact && pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) == 1.f )
		{
			dwFlags |= kTargetTrack_Normal;
		}

		// Target is still lost.

		else {
			dwFlags = dwFlags & ~kTargetTrack_Normal;
		}
	}

	m_pAI->GetAIBlackBoard()->SetBBTargetPosTrackingFlags( dwFlags );

	// Always allow other sensors to update.

	return false;
}
