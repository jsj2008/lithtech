// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPersonalBubble.cpp
//
// PURPOSE : AISensorPersonalBubble class implementation
//
// CREATED : 6/6/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorPersonalBubble.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPersonalBubble, kSensor_PersonalBubble );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPersonalBubble::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorPersonalBubble::CAISensorPersonalBubble()
{
	m_bUseFOV = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPersonalBubble::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorPersonalBubble::GetSenseDistSqr( float fStimulusRadius )
{
	return g_pAIDB->GetAIConstantsRecord()->fPersonalBubbleDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorPersonalBubble::ComplexVisibilityCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorPersonalBubble::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier )
{
	// Fail if Target is outside of the personal bubble.

	float fSenseDistanceSqr = GetSenseDistSqr( pStimulusRecord->m_fDistance );
	if( fSenseDistanceSqr < m_pAI->GetPosition().DistSqr( pStimulusRecord->m_vStimulusPos ) )
	{
		return false;
	}

	// Line of sight check fails.

	if( !super::DoComplexCheck( pStimulusRecord, pfRateModifier ) )
	{
		return false;
	}

	// Sense with the personal bubble slower than actual vision.

	if( pfRateModifier )
	{
		*pfRateModifier = g_pAIDB->GetAIConstantsRecord()->fPersonalBubbleStimulusRateMod;
	}

	return true;
}
