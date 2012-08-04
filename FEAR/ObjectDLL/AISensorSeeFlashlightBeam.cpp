// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeFlashlightBeam.cpp
//
// PURPOSE : AISensorSeeFlashlightBeam class implementation
//
// CREATED : 11/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSeeFlashlightBeam.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeFlashlightBeam, kSensor_SeeFlashlightBeam );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeFlashlightBeam::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorSeeFlashlightBeam::CAISensorSeeFlashlightBeam()
{
	m_fLastStimulationIteration = 0.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorAbstractStimulatable::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorAbstractStimulatable
//              
//----------------------------------------------------------------------------

void CAISensorSeeFlashlightBeam::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_TIME( m_fLastStimulationIteration );
}

//----------------------------------------------------------------------------

void CAISensorSeeFlashlightBeam::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_TIME( m_fLastStimulationIteration );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeFlashlightBeam::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorSeeFlashlightBeam::GetSenseDistSqr( float fStimulusRadius )
{
	float fSenseDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBSeeDistance() + fStimulusRadius;
	return fSenseDistanceSqr * fSenseDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeFlashlightBeam::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorSeeFlashlightBeam::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Sanity check.

	if( !pStimulusRecord )
	{
		return NULL;
	}
	
	// Find an existing memory for this stimulus and source.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Disturbance );
	factQuery.SetTargetObject( pStimulusRecord->m_hStimulusSource );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for this stimulus.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Disturbance );
	}

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeFlashlightBeam::SetFactStimulusPos
//
//	PURPOSE:	Set the position of the stimulus.
//
// ----------------------------------------------------------------------- //

void CAISensorSeeFlashlightBeam::SetFactStimulusPos( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord, float fConfidence )
{
	if( !( pFact && pStimulusRecord ) )
	{
		return;
	}

	// The disturbance originates at the flashlight's source, rather 
	// than where the beam ends.  This results in the AI looking at
	// the flashlight.

	LTVector vSourcePos;
	HOBJECT hFlashlightSource = pStimulusRecord->m_hStimulusSource;
	g_pLTServer->GetObjectPos( hFlashlightSource, &vSourcePos );

	pFact->SetPos( vSourcePos, fConfidence );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeFlashlightBeam::FindUnstimulatedWorkingMemoryFact
//
//	PURPOSE:	Return a working memory fact that has not been 
//              stimulated this update.
//
// ----------------------------------------------------------------------- //

void CAISensorSeeFlashlightBeam::FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList)
{
	if (!pOutFactList)
	{
		return;
	}

	// Find stimuli that have not been updated since the last sensory cycle.

	m_pAI->GetAIWorkingMemory()->CollectFactsUnupdated(kFact_Disturbance, pOutFactList, m_fLastStimulationIteration);

	// Discard any stimuli that are not of the correct type.

	CAIWMFact* pFact;
	EnumAIStimulusType eStimulusType;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact = pOutFactList->begin();
	while( itFact != pOutFactList->end() )
	{
		pFact = *itFact;
		pFact->GetStimulus( &eStimulusType, NULL );

		if( eStimulusType != kStim_FlashlightBeamVisible )
		{
			itFact = pOutFactList->erase( itFact );
		}
		else {
			++itFact;
		}
	}

	// Record the new cutoff time.

	m_fLastStimulationIteration = m_pAI->GetAISensorMgr()->GetStimulusListNewIterationTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeFlashlightBeam::ComplexVisibilityCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeFlashlightBeam::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier )
{
	// Always notice a flashlight instantly if shined on me!

	if( pStimulusRecord->m_hStimulusTarget == m_pAI->m_hObject )
	{
		*pfRateModifier = 99999999.0f;
		return true;
	}

	// Look at the stimulus position.

	LTVector vPos = pStimulusRecord->m_vStimulusPos;

	// Check visibility.

	float fSenseDistanceSqr = GetSenseDistSqr( pStimulusRecord->m_fDistance );
	if( m_pAI->CanSeeThrough() )
	{
		return !!m_pAI->IsObjectPositionVisible( CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, m_pAI->GetEyePosition(), NULL, vPos, fSenseDistanceSqr, true, true );
	}
	else
	{
		return !!m_pAI->IsObjectPositionVisible( CAI::DefaultFilterFn, NULL, m_pAI->GetEyePosition(), NULL, vPos, fSenseDistanceSqr, true, false );
	}
}

