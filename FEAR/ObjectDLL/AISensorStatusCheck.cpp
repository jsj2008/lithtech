// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeDisturbance.cpp
//
// PURPOSE : AISensorStatusCheck class implementation
//
// CREATED : 11/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorStatusCheck.h"
#include "AIStimulusMgr.h"
#include "AISoundMgr.h"
#include "AICoordinator.h"
#include "AIWorkingMemoryCentral.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorStatusCheck, kSensor_StatusCheck );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorStatusCheck::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorStatusCheck::CAISensorStatusCheck()
{
	m_fStatusCheckTime = 0.f;
	m_eStimulusIDToCheck = kStimID_Unset;
	m_hAlly = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorStatusCheck::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorStatusCheck
//              
//----------------------------------------------------------------------------

void CAISensorStatusCheck::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
	SAVE_TIME( m_fStatusCheckTime );
	SAVE_DWORD( m_eStimulusIDToCheck );
	SAVE_HOBJECT( m_hAlly );
}

//----------------------------------------------------------------------------

void CAISensorStatusCheck::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
	LOAD_TIME( m_fStatusCheckTime );
	LOAD_DWORD_CAST( m_eStimulusIDToCheck, EnumAIStimulusID );
	LOAD_HOBJECT( m_hAlly );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorStatusCheck::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorStatusCheck::GetSenseDistSqr( float fStimulusRadius )
{
	float fSenseDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBSeeDistance() + fStimulusRadius;
	return fSenseDistanceSqr * fSenseDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorStatusCheck::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorStatusCheck::UpdateSensor()
{
	// Intentionally skip superclass.

	if( !CAISensorAbstract::UpdateSensor() )
	{
		return false;
	}

	// We're not waiting for anything.

	if( m_fStatusCheckTime == 0.f )
	{
		return false;
	}

	// Not time yet to check the status.

	if( m_fStatusCheckTime > g_pLTServer->GetTime() )
	{
		return false;
	}

	// Bail if someone has performed a status check recently.

	CAIWMFact factTimeQuery;
	factTimeQuery.SetFactType( kFact_Knowledge );
	factTimeQuery.SetKnowledgeType( kKnowledge_NextStatusCheckTime );
	CAIWMFact* pFactTime = g_pAIWorkingMemoryCentral->FindWMFact(factTimeQuery);
	if( pFactTime && ( pFactTime->GetTime() > g_pLTServer->GetTime() ) )
	{
		m_fStatusCheckTime = 0.f;
		m_eStimulusIDToCheck = kStimID_Unset;
		m_hAlly = NULL;
		return false;
	}

	// Bail if someone has already seen this impact.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_WitnessedStimulus );
	factQuery.SetStimulus( kStim_WeaponImpactVisible, m_eStimulusIDToCheck );
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
	if( pFact )
	{
		m_fStatusCheckTime = 0.f;
		m_eStimulusIDToCheck = kStimID_Unset;
		m_hAlly = NULL;
		return false;
	}

	// Record that someone has responded to this impact.

	pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
	pFact->SetKnowledgeType( kKnowledge_WitnessedStimulus, 1.f );
	pFact->SetStimulus( kStim_WeaponImpactVisible, m_eStimulusIDToCheck, 1.f );


	// Vocalize a status check.

	VocalizeStatusCheck();

	// Clear the need to respond to this stimulus.

	m_fStatusCheckTime = 0.f;
	m_eStimulusIDToCheck = kStimID_Unset;
	m_hAlly = NULL;


	// Ensure no one does a status check again for a while.

	pFactTime = g_pAIWorkingMemoryCentral->FindWMFact(factTimeQuery);
	if( !pFactTime )
	{
		pFactTime = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
		pFactTime->SetKnowledgeType( kKnowledge_NextStatusCheckTime, 1.f );
	}
	float fDelay = g_pAIDB->GetAIConstantsRecord()->fAISoundFrequencyEvent;
	pFactTime->SetTime( g_pLTServer->GetTime() + fDelay, 1.f );

	// Clear any previously witnessed impacts.

	CAIWMFact factStimQuery;
	factStimQuery.SetFactType( kFact_Knowledge );
	factStimQuery.SetKnowledgeType( kKnowledge_WitnessedStimulus );
	g_pAIWorkingMemoryCentral->ClearWMFacts( factStimQuery );

	// Always allow other sensors to update.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorStatusCheck::VocalizeStatusCheck.
//
//	PURPOSE:	Vocalize a status check.
//
// ----------------------------------------------------------------------- //

void CAISensorStatusCheck::VocalizeStatusCheck()
{
	// Choose different dialogue of the AI's squad has been eliminated,
	// and he is the only one left.

	bool bSquadEliminated = false;
	ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( m_pAI->m_hObject );
	CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
	if( pSquad && pSquad->GetNumSquadMembers() == 1 )
	{
		bSquadEliminated = true;
	}

	// "What's your status"

	g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_StatusCheck, kAISndCat_Event, NULL, 2.f );

	// Ally is dead.

	if( IsDeadAI( m_hAlly ) )
	{
		// "I need backup, now!"

		if( bSquadEliminated )
		{
			g_pAISoundMgr->RequestAISoundSequence( m_pAI->m_hObject, kAIS_BackupUrgent, m_pAI->m_hObject, kAIS_StatusCheck, kAIS_StatusCheck, kAISndCat_Event, NULL, 2.f );	
		}

		// "We've lost contact"

		else {
			g_pAISoundMgr->RequestAISoundSequence( m_pAI->m_hObject, kAIS_StatusLostContact, m_pAI->m_hObject, kAIS_StatusCheck, kAIS_StatusCheck, kAISndCat_Event, NULL, 2.f );
		}
	}

	// Ally is alive, and responds.
	// "I'm hit!"

	else 
	{
		g_pAISoundMgr->RequestAISoundSequence( m_hAlly, kAIS_StatusOK, m_pAI->m_hObject, kAIS_StatusCheck, kAIS_StatusCheck, kAISndCat_Always, NULL, 0.5f );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorStatusCheck::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorStatusCheck::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// Intentionally do NOT call super::StimulateSensor.
	// This sensor only exists to play dialog based on lack of stimulation.

	// Sanity check.

	if( !pStimulusRecord )
	{
		return false;
	}

	// Template specifies accepted stimuli.

	if( !( m_pSensorRecord->dwStimulusTypes & pStimulusRecord->m_eStimulusType ) )
	{
		return false;
	}

	// Ignore stimuli targeting myself.

	if( pStimulusRecord->m_hStimulusTarget == m_pAI->m_hObject )
	{
		return false;
	}

	// Ignore stimuli targeting something other than an AI.

	if( !IsAI( pStimulusRecord->m_hStimulusTarget ) )
	{
		return false;
	}

	// Bail if we are waiting on something detecting previously.

	if( m_fStatusCheckTime > g_pLTServer->GetTime() )
	{
		return false;
	}

	// Stimulus is too far away to be sensed.

	float fDistanceSqr = pStimulusRecord->m_vStimulusPos.DistSqr( m_pAI->GetPosition() );
	if( fDistanceSqr > GetSenseDistSqr( pStimulusRecord->m_fDistance ) )
	{
		return false;
	}

	// Bail if someone has performed a status check recently.
	// Add a one second leniency on the time, to ensure we catch
	// cases where someone would have seen an impact just as the 
	// timeout expired.

	CAIWMFact factTimeQuery;
	factTimeQuery.SetFactType( kFact_Knowledge );
	factTimeQuery.SetKnowledgeType( kKnowledge_NextStatusCheckTime );
	CAIWMFact* pFactTime = g_pAIWorkingMemoryCentral->FindWMFact(factTimeQuery);
	if( pFactTime && ( pFactTime->GetTime() - 1.f > g_pLTServer->GetTime() ) )
	{
		return false;
	}

	// Bail if someone has already seen this impact.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_WitnessedStimulus );
	factQuery.SetStimulus( kStim_WeaponImpactVisible, pStimulusRecord->m_eStimulusID );
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact(factQuery);
	if( pFact )
	{
		return false;
	}

	// Determine if stimulus can truly be sensed, using optional specific checks.

	float fRateModifier = 1.f;
	bool bStimulated = DoComplexCheck( pStimulusRecord, &fRateModifier );
	if( bStimulated )
	{
		// Record that someone has seen this impact.

		pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
		pFact->SetKnowledgeType( kKnowledge_WitnessedStimulus, 1.f );
		pFact->SetStimulus( kStim_WeaponImpactVisible, pStimulusRecord->m_eStimulusID, 1.f );

		return true;
	}

	// AI cannot see the stimulus.  
	// Check back in a second to see if anyone else has seen it.  
	// The AI should only do a status check if no one can see the threat.

	m_fStatusCheckTime = g_pLTServer->GetTime() + 1.f;
	m_eStimulusIDToCheck = pStimulusRecord->m_eStimulusID;
	m_hAlly = pStimulusRecord->m_hStimulusTarget;

	// Sensor was not actually stimulated.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorStatusCheck::ComplexVisibilityCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorStatusCheck::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* /*pfRateModifier*/ )
{
	HOBJECT hTarget = pStimulusRecord->m_hStimulusTarget;

	// Look at the stimulus target if it exists.
	// Otherwise, look at the stimulus position.

	LTVector vPos;
	if( hTarget )
	{
		g_pLTServer->GetObjectPos( hTarget, &vPos );
	}
	else {
		vPos = pStimulusRecord->m_vStimulusPos;
	}

	// Check visibility.

	bool bVisible;
	bool bFOV = false;
	HOBJECT hBlocking = NULL;
	float fSenseDistanceSqr = GetSenseDistSqr( pStimulusRecord->m_fDistance );
	if( m_pAI->CanSeeThrough() )
	{
		bVisible = m_pAI->IsObjectPositionVisible( CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, m_pAI->GetEyePosition(), NULL, vPos, fSenseDistanceSqr, bFOV, true, &hBlocking );
	}
	else
	{
		bVisible = m_pAI->IsObjectPositionVisible( CAI::DefaultFilterFn, NULL, m_pAI->GetEyePosition(), NULL, vPos, fSenseDistanceSqr, bFOV, false, &hBlocking );
	}

	// Position is visible, or blocked by the disturbance target.

	if( ( !bVisible ) &&
		( hTarget && ( hTarget == hBlocking ) ) )
	{
		bVisible = true;
	}

	return bVisible;
}

