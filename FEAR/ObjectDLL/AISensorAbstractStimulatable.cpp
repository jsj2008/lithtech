// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorAbstractStimulatable.cpp
//
// PURPOSE : AISensorAbstractStimulatable abstract class implementation
//
// CREATED : 3/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorAbstractStimulatable.h"
#include "AI.h"
#include "AIDB.h"
#include "AIUtils.h"
#include "AIStimulusMgr.h"
#include "AIBlackBoard.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorAbstractStimulatable::CAISensorAbstractStimulatable()
{
	m_fStimulationThreshold = 1.f;
	m_fStimulationMax = 1.f;
	m_fLastSensorUpdate = 0.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorAbstractStimulatable::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorAbstractStimulatable
//              
//----------------------------------------------------------------------------

void CAISensorAbstractStimulatable::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	// Reaction delays.

	SAVE_INT(m_lstReactionDelays.size());

	for (std::size_t n = 0; n < m_lstReactionDelays.size(); ++n)
	{
		m_lstReactionDelays[n]->Save(pMsg);
	}

	// Stimuli.

	SAVE_INT( m_lstStimuli.size() );

	StimulusRecordCreateStruct* pSCS;
	STIMULUS_DISPATCH_LIST::iterator itStimulus;
	for( itStimulus = m_lstStimuli.begin(); itStimulus != m_lstStimuli.end(); ++itStimulus )
	{
		pSCS = &( *itStimulus );
		pSCS->Save( pMsg );
	}

	// Last update.

	SAVE_TIME( m_fLastSensorUpdate );
}

//----------------------------------------------------------------------------

void CAISensorAbstractStimulatable::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	// Reaction delays.

	int nFacts = 0;
	LOAD_INT(nFacts);

	for (int i = 0; i < nFacts; ++i)
	{
		CAIWMFact* pFact = AI_FACTORY_NEW( CAIWMFact );
		pFact->Load(pMsg);
		m_lstReactionDelays.push_back(pFact);
	}

	// Stimuli.

	int cStimuli;
	LOAD_INT( cStimuli );
	m_lstStimuli.resize( cStimuli );

	StimulusRecordCreateStruct* pSCS;
	STIMULUS_DISPATCH_LIST::iterator itStimulus;
	for( itStimulus = m_lstStimuli.begin(); itStimulus != m_lstStimuli.end(); ++itStimulus )
	{
		pSCS = &( *itStimulus );
		pSCS->Load( pMsg );
	}

	// Last update.

	LOAD_TIME( m_fLastSensorUpdate );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorAbstractStimulatable::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	m_fLastSensorUpdate = g_pLTServer->GetTime();

	// No updating necessary if the list of reaction delays is empty,
	// and the list of stimuli to dispatch is empty.

	if( m_lstReactionDelays.empty() &&
		m_lstStimuli.empty() )
	{
		return false;
	}


	double fTime = g_pLTServer->GetTime();

	// Iterate over all facts, searching for expired delays.

	CAIWMFact* pFact;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	itFact = m_lstReactionDelays.begin();
	while( itFact != m_lstReactionDelays.end() )
	{
		pFact = *itFact;
		if( pFact->GetUpdateTime() <= fTime )
		{
			itFact = m_lstReactionDelays.erase( itFact );
			m_pAI->GetAIBlackBoard()->SetBBSelectTarget( true );
			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		}
		else {
			++itFact;
		}
	}

	// Dispatch all stimuli.

	StimulusRecordCreateStruct* pSCS;
	STIMULUS_DISPATCH_LIST::iterator itStimulus;
	for( itStimulus = m_lstStimuli.begin(); itStimulus != m_lstStimuli.end(); ++itStimulus )
	{
		pSCS = &( *itStimulus );
		g_pAIStimulusMgr->RegisterStimulus( *pSCS );
	}
	m_lstStimuli.resize( 0 );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorAbstractStimulatable::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// Intentionally do NOT call super::StimulateSensor().

	// Sanity checks.

	if( !m_pAI )
	{
		AIASSERT( 0, NULL, "CAISensorAbstractStimulatable::UpdateSensor: AI is NULL." );
		return false;
	}

	if( !m_pSensorRecord )
	{
		AIASSERT( 0, NULL, "CAISensorAbstractStimulatable::UpdateSensor: SensorRecord is NULL." );
		return false;
	}

	if( !pStimulusRecord )
	{
		AIASSERT( 0, NULL, "CAISensorAbstractStimulatable::UpdateSensor: StimulusRecord is NULL." );
		return false;
	}

	// Template specifies accepted stimuli.

	if( !( m_pSensorRecord->dwStimulusTypes & pStimulusRecord->m_eStimulusType ) )
	{
		return false;
	}

	// Stimulus is too far away to be sensed.

	float fDistanceSqr = pStimulusRecord->m_vStimulusPos.DistSqr( m_pAI->GetPosition() );
	if( fDistanceSqr > GetSenseDistSqr( pStimulusRecord->m_fDistance ) )
	{
		return false;
	}

	// Determine if stimulus can truly be sensed, using optional specific checks.

	float fRateModifier = 1.f;
	if( !DoComplexCheck( pStimulusRecord, &fRateModifier ) )
	{
		return false;
	}

	// Stimulus can be sensed, create a new memory fact.

	CAIWMFact* pFact = CreateWorkingMemoryFact( pStimulusRecord );
	if (!pFact)
	{
		return false;
	}


	//
	// Set working memory data and confidence values.
	//

	// Set the flags to something, so that it contains a valid value.  
	// This way code can add flags or query for them without needing 
	// to worry about the flags not existing.
	pFact->SetFactFlags( 0 );

	SetFactTargetObject( pFact, pStimulusRecord );

	// Confidence in a stimulus' position and stimulation are based on its current stimulation.

	AIDB_StimulusRecord* pAIDBStimulus = GetAIDBStimulus( pStimulusRecord );

	float fConfidence = ( pFact->IsSet( CAIWMFact::kFactMask_Stimulus ) ) ? pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) : 0.f;
	fConfidence = IncreaseStimulation( fConfidence, fRateModifier, pStimulusRecord, pAIDBStimulus );
	SetFactStimulus( pFact, pStimulusRecord, fConfidence );

	SetFactStimulusPos( pFact, pStimulusRecord, fConfidence );

	pFact->SetDir( pStimulusRecord->m_vStimulusDir, 1.f );
	pFact->SetRadius( pStimulusRecord->m_fDistance, 1.f );

	// Increment AlarmLevel.

	if( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) >= m_fStimulationThreshold )
	{
		m_pAI->IncrementAlarmLevel( pStimulusRecord->m_nStimulusAlarmLevel );
	}

	// Handle reaction delay.

	if( pStimulusRecord->m_pAIDB_Stimulus->v2ReactionDelay.y > 0.f )
	{
		// Set the initial reaction delay.

		if( pFact->GetUpdateTime() == 0.f )
		{
			double fUpdateTime = pStimulusRecord->m_fTimeStamp +
								GetRandom( pStimulusRecord->m_pAIDB_Stimulus->v2ReactionDelay.x,
											pStimulusRecord->m_pAIDB_Stimulus->v2ReactionDelay.y );
			pFact->SetUpdateTime( fUpdateTime );

			m_lstReactionDelays.push_back( pFact );
		}

		// Wait until the delay expires before updating anymore.

		if( ReactionDelayExists( pFact ) )
		{
			return true;
		}
	}

	// React immediately.

	pFact->SetUpdateTime( pStimulusRecord->m_fTimeStamp );
	if( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) >= m_fStimulationThreshold )
	{
		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}

	// Stimulus accepted.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::ReactionDelayExists
//
//	PURPOSE:	Return true if a reaction delay already exists.
//
// ----------------------------------------------------------------------- //

bool CAISensorAbstractStimulatable::ReactionDelayExists( CAIWMFact* pFact )
{
	// Sanity check.

	if( !pFact )
	{
		return false;
	}

	// Find an existing reaction delay.

	CAIWMFact* pExistingFact;
	AIWORKING_MEMORY_FACT_LIST::iterator itFact;
	for( itFact = m_lstReactionDelays.begin(); itFact != m_lstReactionDelays.end(); ++itFact )
	{
		pExistingFact = *itFact;
		if( pExistingFact == pFact )
		{
			return true;
		}
	}

	// Reaction delay does not exist.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::DispatchStimulus
//
//	PURPOSE:	Add a stimulus to the list of stimuli to dispatch next update.
//
// ----------------------------------------------------------------------- //

void CAISensorAbstractStimulatable::DispatchStimulus( const StimulusRecordCreateStruct& scs )
{
	m_lstStimuli.push_back( scs );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::SetFactStimulus
//
//	PURPOSE:	Set the stimulus that generated the fact.
//
// ----------------------------------------------------------------------- //

void CAISensorAbstractStimulatable::SetFactStimulus( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord, float fConfidence )
{
	if( !( pFact && pStimulusRecord ) )
	{
		return;
	}

	pFact->SetStimulus( pStimulusRecord->m_eStimulusType,
						pStimulusRecord->m_eStimulusID,
						fConfidence );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::SetFactStimulusPos
//
//	PURPOSE:	Set the position of the stimulus.
//
// ----------------------------------------------------------------------- //

void CAISensorAbstractStimulatable::SetFactStimulusPos( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord, float fConfidence )
{
	if( !( pFact && pStimulusRecord ) )
	{
		return;
	}

	pFact->SetPos( pStimulusRecord->m_vStimulusPos, fConfidence );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::SetFactTargetObject
//
//	PURPOSE:	Set the target object the WMFact.
//
// ----------------------------------------------------------------------- //

void CAISensorAbstractStimulatable::SetFactTargetObject( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord )
{
	// Sanity check.

	if( !( pFact && pStimulusRecord ) )
	{
		return;
	}

	pFact->SetTargetObject( pStimulusRecord->m_hStimulusSource, 1.f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::DestimulateSensor
//
//	PURPOSE:	Handles not getting any stimulus.
//
// ----------------------------------------------------------------------- //

void CAISensorAbstractStimulatable::DestimulateSensor()
{
	if( !m_pAI )
	{
		AIASSERT( 0, NULL, "CAISensorAbstractStimulatable::StimulateSenor: AI is NULL." );
		return;
	}

	// Find existing memories related to this sensor, that 
	// have not been stimulated this update.

	EnumAIStimulusType eStimulusType;
	AIDB_StimulusRecord* pStimulus;
	float fConfidence;

	// Collect all unstimulated facts and decrease their stimulation.

	static AIWORKING_MEMORY_FACT_LIST	s_UnstimulatedFacts;
	s_UnstimulatedFacts.resize(0);

	FindUnstimulatedWorkingMemoryFact(&s_UnstimulatedFacts);

	AIWORKING_MEMORY_FACT_LIST::iterator iter;
	for ( iter = s_UnstimulatedFacts.begin(); iter != s_UnstimulatedFacts.end(); ++iter)
	{
		CAIWMFact* pFact = *iter;
		if( pFact && pFact->IsSet( CAIWMFact::kFactMask_Stimulus ) )
		{
			pFact->GetStimulus( &eStimulusType, NULL );
			pStimulus = g_pAIDB->GetAIStimulusRecord( eStimulusType );
			fConfidence = DecreaseStimulation( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ), pStimulus );

			pFact->SetConfidence( CAIWMFact::kFactMask_Stimulus, fConfidence );
			pFact->SetConfidence( CAIWMFact::kFactMask_Position, fConfidence );

			pFact->SetStimulationDecreaseTime( g_pLTServer->GetTime() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::GetAIDBStimulus()
//
//	PURPOSE:	Get the AIDB stimulus record for a stimulus.
//
// ----------------------------------------------------------------------- //

AIDB_StimulusRecord* CAISensorAbstractStimulatable::GetAIDBStimulus( CAIStimulusRecord* pStimulusRecord )
{
	if( pStimulusRecord )
	{
		return pStimulusRecord->m_pAIDB_Stimulus;
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::IncreaseStimulation()
//
//	PURPOSE:	Increase stimulation of a sense.
//
// ----------------------------------------------------------------------- //

float CAISensorAbstractStimulatable::IncreaseStimulation( float fCurStimulation, float fRateModifier, CAIStimulusRecord* pStimulusRecord, AIDB_StimulusRecord* pStimulus )
{
	if( !pStimulus )
	{
		return 0.f;
	}

	// Stimulation increase rate depends on alertness.

	EnumAIAwareness eAwareness = m_pAI->GetAIBlackBoard()->GetBBAwareness();
	float fStimulationIncreaseRate;
	if( eAwareness == kAware_Alert )
	{
		fStimulationIncreaseRate = pStimulus->fStimulationIncreaseRateAlert;
	}
	else if( eAwareness == kAware_Suspicious )
	{
		fStimulationIncreaseRate = pStimulus->fStimulationIncreaseRateSuspicious;
	}
	else {
		fStimulationIncreaseRate = pStimulus->fStimulationIncreaseRateUnalert;
	}

	// Current stimulation is either StimulationMax, or some formula.  Whichever is less.

	float fTimeDelta = (float)(g_pLTServer->GetTime() - m_fLastSensorUpdate);
	fCurStimulation = LTMIN<float>( m_fStimulationMax, fCurStimulation + 
										( LTMAX<float>(fTimeDelta, m_pAI->GetSenseUpdateRate()) * fStimulationIncreaseRate * fRateModifier ) );

	AITRACE( AIShowSenses, ( m_pAI->m_hObject, "IncreaseStimulution of %s to %f (Stimulus: %s)\n", 
			s_aszSensorTypes[m_eSensorType], fCurStimulation, CAIStimulusMgr::StimulusToString( pStimulus->eStimulusType ) ) );

	// Re-evaluate targets when being stimulated.

	m_pAI->GetAIBlackBoard()->SetBBSelectTarget( true );

	return fCurStimulation;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAbstractStimulatable::DecreaseStimulation
//
//	PURPOSE:	Decay the stimulation of the sense
//
// ----------------------------------------------------------------------- //

float CAISensorAbstractStimulatable::DecreaseStimulation( float fCurStimulation, AIDB_StimulusRecord* pStimulus )
{
	if( !pStimulus )
	{
		return 0.f;
	}

	// Stimulation decrease rate depends on alertness.

	EnumAIAwareness eAwareness = m_pAI->GetAIBlackBoard()->GetBBAwareness();
	float fStimulationDecreaseRate;
	if( eAwareness == kAware_Alert )
	{
		fStimulationDecreaseRate = pStimulus->fStimulationDecreaseRateAlert;
	}
	else if( eAwareness == kAware_Suspicious )
	{
		fStimulationDecreaseRate = pStimulus->fStimulationDecreaseRateSuspicious;
	}
	else {
		fStimulationDecreaseRate = pStimulus->fStimulationDecreaseRateUnalert;
	}

	// Current stimulation is either 0, or some formula.  Whichever is more.

	if( ( fStimulationDecreaseRate > 0.f ) &&
		( fCurStimulation > 0.f ) )
	{
		float fTimeDelta = (float)(g_pLTServer->GetTime() - m_fLastSensorUpdate);
		fCurStimulation = LTMAX<float>(0.0f, fCurStimulation -
										 ( LTMAX<float>(fTimeDelta, m_pAI->GetSenseUpdateRate()) * fStimulationDecreaseRate ) );

		AITRACE( AIShowSenses, ( m_pAI->m_hObject, "DecreaseStimulution of %s to %f\n", s_aszSensorTypes[m_eSensorType], fCurStimulation ) );
	}

	return fCurStimulation;
}
