// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCritter.cpp
//
// PURPOSE : AISensorCritter class implementation
//
// CREATED : 02/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorCritter.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCritter, kSensor_Critter );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCritter::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorCritter::CAISensorCritter()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCritter::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorCritter::GetSenseDistSqr( float fStimulusRadius )
{
	float fSenseDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBHearDistance() + fStimulusRadius;
	return fSenseDistanceSqr * fSenseDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCritter::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorCritter::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// Run away if an enemy comes close enough.

	if( pStimulusRecord && pStimulusRecord->m_eStimulusType == kStim_CharacterVisible )
	{
		float fSeeDist = m_pAI->GetAIBlackBoard()->GetBBSeeDistance();
		if( m_pAI->GetPosition().DistSqr( pStimulusRecord->m_vStimulusPos ) < fSeeDist * fSeeDist )
		{
			CreateWorkingMemoryFact( pStimulusRecord );
		}
		return false;
	}

	// Default behavior.

	return super::StimulateSensor( pStimulusRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCritter::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorCritter::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Sanity check.

	if( !pStimulusRecord )
	{
		return NULL;
	}

	// Bail if source no longer exists.

	if( !pStimulusRecord->m_hStimulusSource )
	{
		return NULL;
	}

	// Find an existing memory for the desire to flee from this stimulus source.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Desire );
	factQuery.SetDesireType( kDesire_Flee );
	factQuery.SetTargetObject( pStimulusRecord->m_hStimulusSource );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for this stimulus.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
		if( pFact )
		{
			pFact->SetDesireType( kDesire_Flee, 1.f );
			pFact->SetTargetObject( pStimulusRecord->m_hStimulusSource );
		}

		// Re-evaluate goals immediately.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}

	// Critters are paranoid and think someone is aiming at them anytime they sense anything.

	m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject, kWST_bool, true );

	return pFact;
}
