// ----------------------------------------------------------------------- //
//
// MODULE  : AISenseRecorderAbstract.cpp
//
// PURPOSE : AISenseRecorderAbstract implementation
//
// CREATED : 5/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AISenseRecorderAbstract.h"
#include "AI.h"
#include "AIState.h"
#include "AIGoalMgr.h"
#include "AIStimulusMgr.h"
#include "AIUtils.h"
#include "PlayerObj.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISenseRecord::Save/Load()
//
//	PURPOSE:	Save and Load
//
// ----------------------------------------------------------------------- //

void AISenseRecord::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(eSenseType);
	SAVE_DWORD(pAIBM_Last_Stimulus ? pAIBM_Last_Stimulus->eSenseType : kStim_InvalidType);
	SAVE_HOBJECT(hLastStimulusSource);
	SAVE_HOBJECT(hLastStimulusTarget);
	SAVE_DWORD(eLastTargetMatchID);
	SAVE_VECTOR(vLastStimulusPos);
	SAVE_VECTOR(vLastStimulusDir);
	SAVE_DWORD(nLastStimulusAlarmLevel);
	SAVE_DWORD(eLastStimulusID);
	SAVE_FLOAT(fSenseDistance);
	SAVE_FLOAT(fSenseDistanceSqr);
	SAVE_FLOAT(fCurStimulation);
	SAVE_FLOAT(fMaxStimulation);
	SAVE_FLOAT(fReactionDelayTimer);
	SAVE_FLOAT(fReactionDelayTime);
	SAVE_TIME(fLastStimulationTime);
	SAVE_DWORD(nCycle);
	SAVE_BYTE(cFalseStimulation);
	SAVE_INT(ptSightGrid.x);
	SAVE_INT(ptSightGrid.y);
}

void AISenseRecord::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST(eSenseType, EnumAISenseType);

	EnumAIStimulusType eStimulus;
	LOAD_DWORD_CAST(eStimulus, EnumAIStimulusType);
	if(eStimulus != kStim_InvalidType)
	{
		pAIBM_Last_Stimulus = g_pAIButeMgr->GetStimulus(eStimulus);
	}

	LOAD_HOBJECT(hLastStimulusSource);
	LOAD_HOBJECT(hLastStimulusTarget);
	LOAD_DWORD_CAST(eLastTargetMatchID, EnumAITargetMatchID);
	LOAD_VECTOR(vLastStimulusPos);
	LOAD_VECTOR(vLastStimulusDir);
	LOAD_DWORD(nLastStimulusAlarmLevel);
	LOAD_DWORD_CAST(eLastStimulusID, EnumAIStimulusID);
	LOAD_FLOAT(fSenseDistance);
	LOAD_FLOAT(fSenseDistanceSqr);
	LOAD_FLOAT(fCurStimulation);
	LOAD_FLOAT(fMaxStimulation);
	LOAD_FLOAT(fReactionDelayTimer);
	LOAD_FLOAT(fReactionDelayTime);
	LOAD_TIME(fLastStimulationTime);
	LOAD_DWORD(nCycle);
	LOAD_BYTE(cFalseStimulation);
	LOAD_INT(ptSightGrid.x);
	LOAD_INT(ptSightGrid.y);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AISenseRecord::CreateAISenseRecord
//
//	PURPOSE:	Constructing function of a AISenseRecord.
//
// ----------------------------------------------------------------------- //

AISenseRecord* AISenseRecord::CreateAISenseRecord( CAISenseRecorderAbstract& aiSenseRecorderAbstract )
{
	// Create the new senserecord.
	AISenseRecord* pAISenseRecord = new AISenseRecord;
	
	// Set the AIRecorderAbstract in the object refereces.
	if( pAISenseRecord )
	{
		pAISenseRecord->hLastStimulusSource.SetReceiver( aiSenseRecorderAbstract );
		pAISenseRecord->hLastStimulusTarget.SetReceiver( aiSenseRecorderAbstract );
	}

	return pAISenseRecord;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::CAISenseRecorderAbstract()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CAISenseRecorderAbstract::CAISenseRecorderAbstract()
{
	m_pSensing = LTNULL;
	m_bDoneProcessingStimuli = LTTRUE;
	m_cIntersectSegmentCount = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::~CAISenseRecorderAbstract()
//
//	PURPOSE:	Destroy object
//
// ----------------------------------------------------------------------- //

CAISenseRecorderAbstract::~CAISenseRecorderAbstract()
{
	AISENSE_RECORD_MAP::iterator it;
	for(it = m_mapSenseRecords.begin(); it != m_mapSenseRecords.end(); ++it)
	{
		debug_delete(it->second);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::Save/Load()
//
//	PURPOSE:	Save and Load
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(m_mapSenseRecords.size());

	AISENSE_RECORD_MAP::iterator it;
	for(it = m_mapSenseRecords.begin(); it != m_mapSenseRecords.end(); ++it)
	{
		it->second->Save(pMsg);
	}

	SAVE_BOOL( m_bDoneProcessingStimuli );

	SAVE_DWORD( m_mapProcessedStimuli.size() );
	AI_PROCESSED_STIMULI_MAP::iterator psit;
	for( psit = m_mapProcessedStimuli.begin(); psit != m_mapProcessedStimuli.end(); ++psit )
	{
		SAVE_DWORD( psit->first );
	}

	SAVE_DWORD( m_cIntersectSegmentCount );
}

void CAISenseRecorderAbstract::Load(ILTMessage_Read *pMsg)
{
	uint32 cSenses;
	LOAD_DWORD(cSenses);

	AISenseRecord* pSenseRecord;
	for(uint32 iSense=0; iSense < cSenses; ++iSense)
	{
		pSenseRecord = debug_new_AISenseRecord( *this );
		pSenseRecord->Load(pMsg);
		m_mapSenseRecords.insert( AISENSE_RECORD_MAP::value_type(pSenseRecord->eSenseType, pSenseRecord) );
	}

	LOAD_BOOL( m_bDoneProcessingStimuli );

	EnumAIStimulusID eStimID;
	uint32 cStimulus;
	LOAD_DWORD( cStimulus );
	for( uint32 iStimulus=0; iStimulus < cStimulus; ++iStimulus )
	{
		LOAD_DWORD_CAST( eStimID, EnumAIStimulusID );
		m_mapProcessedStimuli.insert( AI_PROCESSED_STIMULI_MAP::value_type( eStimID, 0 ) );
	}

	LOAD_DWORD( m_cIntersectSegmentCount );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::Init(IAISensing* pSensing)
{
	m_pSensing = pSensing;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::ClearSense()
//
//	PURPOSE:	Clears the sense record for a stimulus.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::ClearSense(AISenseRecord* pSenseRecord)
{
	ASSERT(pSenseRecord);

	pSenseRecord->fCurStimulation		= 0.f;
	pSenseRecord->fMaxStimulation		= 0.f;
	pSenseRecord->fReactionDelayTimer	= 0.f;
	pSenseRecord->fReactionDelayTime	= 0.f;
	pSenseRecord->cFalseStimulation		= 0;
	pSenseRecord->nCycle				= 0;
	pSenseRecord->pAIBM_Last_Stimulus	= LTNULL;
	pSenseRecord->hLastStimulusSource	= LTNULL;
	pSenseRecord->hLastStimulusTarget	= LTNULL;
	pSenseRecord->eLastTargetMatchID	= kTMID_Invalid;
	pSenseRecord->vLastStimulusPos		= LTVector( 0.0f, 0.0f, 0.0f );
	pSenseRecord->vLastStimulusDir		= LTVector( 0.0f, 0.0f, 0.0f );
	pSenseRecord->nLastStimulusAlarmLevel = 0;
	pSenseRecord->eLastStimulusID		= kStimID_Unset;
	pSenseRecord->ptSightGrid.x			= m_pSensing->GetSightGridRangeX().GetMin();
	pSenseRecord->ptSightGrid.y			= m_pSensing->GetSightGridRangeY().GetMin();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::AddStimulus()
//
//	PURPOSE:	Creates sense record for a stimulus.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::AddSense(EnumAISenseType eSenseType, LTFLOAT fDistance)
{
	ASSERT(eSenseType != kSense_InvalidType);

	AISenseRecord* pSenseRecord = debug_new_AISenseRecord( *this );

	pSenseRecord->eSenseType			= eSenseType;
	pSenseRecord->fSenseDistance		= fDistance;
	pSenseRecord->fSenseDistanceSqr		= fDistance * fDistance;
	pSenseRecord->fLastStimulationTime	= 0.f;

	ClearSense(pSenseRecord);

	m_mapSenseRecords.insert( AISENSE_RECORD_MAP::value_type(eSenseType, pSenseRecord) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::RemoveSense()
//
//	PURPOSE:	Removes sense record for a stimulus.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::RemoveSense(EnumAISenseType eSenseType)
{
	ASSERT(eSenseType != kSense_InvalidType);

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	if( it != m_mapSenseRecords.end() )
	{
		m_mapSenseRecords.erase( it );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::HasSense()
//
//	PURPOSE:	Queries to determine if a sense record is present.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderAbstract::HasSense(EnumAISenseType eSenseType)
{
	ASSERT(eSenseType != kSense_InvalidType);

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	if( it != m_mapSenseRecords.end() )
	{
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::SetSenseDistance()
//
//	PURPOSE:	Set distance at which AI responds to a stimulus.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::SetSenseDistance(EnumAISenseType eSenseType, LTFLOAT fDistance)
{
	ASSERT(eSenseType != kSense_InvalidType);

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	AISenseRecord* pRecord = it->second;
	pRecord->fSenseDistance = fDistance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::GetSenseDistance()
//
//	PURPOSE:	Get distance at which AI responds to a stimulus.
//
// ----------------------------------------------------------------------- //

LTFLOAT CAISenseRecorderAbstract::GetSenseDistance(EnumAISenseType eSenseType)
{
	ASSERT(eSenseType != kSense_InvalidType);

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	return it->second->fSenseDistance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::GetSenseDistanceSqr()
//
//	PURPOSE:	Get squared distance at which AI responds to a stimulus.
//
// ----------------------------------------------------------------------- //

LTFLOAT CAISenseRecorderAbstract::GetSenseDistanceSqr(EnumAISenseType eSenseType)
{
	ASSERT(eSenseType != kSense_InvalidType);

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	if(it != m_mapSenseRecords.end())
	{
		return it->second->fSenseDistanceSqr;
	}

	return 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::*ProcessingStimuli()
//
//	PURPOSE:	Process stimuli.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderAbstract::GetDoneProcessingStimuli() const
{
	return m_bDoneProcessingStimuli;
}

void CAISenseRecorderAbstract::SetDoneProcessingStimuli(LTBOOL bDone)
{
	m_bDoneProcessingStimuli = bDone;
}

void CAISenseRecorderAbstract::ClearProcessedStimuli()
{
	m_mapProcessedStimuli.clear();
}

LTBOOL CAISenseRecorderAbstract::ProcessStimulus(CAIStimulusRecord* pRecord)
{
	if( m_mapProcessedStimuli.find( pRecord->m_eStimulusID ) == m_mapProcessedStimuli.end() )
	{
		m_mapProcessedStimuli.insert( AI_PROCESSED_STIMULI_MAP::value_type( pRecord->m_eStimulusID, 0 ) );
		return LTTRUE;
	}
		
	return LTFALSE;	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::*IntersectSegmentCount()
//
//	PURPOSE:	Handle IntersectSegment Counting.
//
// ----------------------------------------------------------------------- //

int	CAISenseRecorderAbstract::GetIntersectSegmentCount() const
{
	return m_cIntersectSegmentCount;
}

void CAISenseRecorderAbstract::ClearIntersectSegmentCount()
{
	m_cIntersectSegmentCount = 0;
}

void CAISenseRecorderAbstract::IncrementIntersectSegmentCount()
{
	++m_cIntersectSegmentCount;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::UpdateSenseRecord()
//
//	PURPOSE:	Update the state of a sense due to a stimuli.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderAbstract::UpdateSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle)
{
	if( !pStimulusRecord )
	{
		ASSERT(pStimulusRecord != LTNULL);
		return LTFALSE;
	}

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(pStimulusRecord->m_pAIBM_Stimulus->eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	AISenseRecord* pSenseRecord = it->second;

	// Only sense more recent stimuli, if flag is set to ignore old stimulus.

	if( pStimulusRecord->m_pAIBM_Stimulus->bIgnoreOldStimulus &&
		( pSenseRecord->fLastStimulationTime >= pStimulusRecord->m_fTimeStamp ) )
	{
		return LTFALSE;
	}


	LTFLOAT fRateModifier = 1.0f;

	// Do additional expensive checks here. (e.g. Ray casts).
	if(!HandleSpecificStimuli(pStimulusRecord, &fRateModifier))
	{
		return LTFALSE;
	}

	// Mark the sense as being updated this cycle.  The recorder can look at the 
	// cycle stamp later to determine which senses were updated during one cycle.
	pSenseRecord->nCycle = nCycle;

	// Keep a pointer to the bute file entry for the stimulus.
	AIBM_Stimulus* pAIBM_Stimulus = pStimulusRecord->m_pAIBM_Stimulus;
	pSenseRecord->pAIBM_Last_Stimulus = pAIBM_Stimulus;

	// Mark the timestamp of the stimulating sense.
	pSenseRecord->fLastStimulationTime = pStimulusRecord->m_fTimeStamp;

	// Record info about stimulus.
	pSenseRecord->hLastStimulusSource		= ( HOBJECT )pStimulusRecord->m_hStimulusSource;
	pSenseRecord->hLastStimulusTarget		= ( HOBJECT )pStimulusRecord->m_hStimulusTarget;
	pSenseRecord->eLastTargetMatchID		= pStimulusRecord->m_eTargetMatchID;
	pSenseRecord->vLastStimulusPos			= pStimulusRecord->m_vStimulusPos;
	pSenseRecord->vLastStimulusDir			= pStimulusRecord->m_vStimulusDir;
	pSenseRecord->nLastStimulusAlarmLevel	= pStimulusRecord->m_nStimulusAlarmLevel;
	pSenseRecord->eLastStimulusID			= pStimulusRecord->m_eStimulusID;

	// Randomize reaction delay time.
	// Do not reset the timer if stimulation already started.
	if( pSenseRecord->fCurStimulation <= 0.f )
	{
		pSenseRecord->fReactionDelayTime		= GetRandom( pAIBM_Stimulus->rngReactionDelay.GetMin(), pAIBM_Stimulus->rngReactionDelay.GetMax() );
		pSenseRecord->fReactionDelayTimer		= 0.f;
	}

	// Returns true if sense has reached full stimulation.
	return IncreaseStimulation(pSenseRecord, fRateModifier);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::CopySenseRecord()
//
//	PURPOSE:	Update the state of a sense by copying a sense record.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::CopySenseRecord(AISenseRecord* pOrigSenseRecord)
{
	if( !pOrigSenseRecord )
	{
		ASSERT( pOrigSenseRecord != LTNULL );
		return;
	}

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find( pOrigSenseRecord->eSenseType );
	ASSERT(it != m_mapSenseRecords.end());

	AISenseRecord* pSenseRecord = it->second;

	// If sense was already updated this cycle, do not copy over it.
	if( pSenseRecord->nCycle == pOrigSenseRecord->nCycle )
	{
		return;
	}

	// Mark the sense as being updated this cycle.  The recorder can look at the 
	// cycle stamp later to determine which senses were updated during one cycle.
	pSenseRecord->nCycle = pOrigSenseRecord->nCycle;

	// Keep a pointer to the bute file entry for the stimulus.
	pSenseRecord->pAIBM_Last_Stimulus = pOrigSenseRecord->pAIBM_Last_Stimulus;

	// Mark the timestamp of the stimulating sense.
	pSenseRecord->fLastStimulationTime = pOrigSenseRecord->fLastStimulationTime;

	// Record info about stimulus.
	pSenseRecord->hLastStimulusSource		= pOrigSenseRecord->hLastStimulusSource;
	pSenseRecord->hLastStimulusTarget		= pOrigSenseRecord->hLastStimulusTarget;
	pSenseRecord->eLastTargetMatchID		= pOrigSenseRecord->eLastTargetMatchID;
	pSenseRecord->vLastStimulusPos			= pOrigSenseRecord->vLastStimulusPos;
	pSenseRecord->vLastStimulusDir			= pOrigSenseRecord->vLastStimulusDir;
	pSenseRecord->nLastStimulusAlarmLevel	= pOrigSenseRecord->nLastStimulusAlarmLevel;
	pSenseRecord->eLastStimulusID			= pOrigSenseRecord->eLastStimulusID;

	// Randomize reaction delay time.
	// Do not reset the timer if stimulation already started.
	if( pSenseRecord->fCurStimulation <= 0.f )
	{
		pSenseRecord->fReactionDelayTime		= GetRandom( pSenseRecord->pAIBM_Last_Stimulus->rngReactionDelay.GetMin(), pSenseRecord->pAIBM_Last_Stimulus->rngReactionDelay.GetMax() );
		pSenseRecord->fReactionDelayTimer		= 0.f;
	}

	// Returns true if sense has reached full stimulation.
	// Force full stimulation.
	IncreaseStimulation(pSenseRecord, 9999999.99f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::HandleSpecificStimuli()
//
//	PURPOSE:	Abstract virtual function to allow handling of specific stimuli.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderAbstract::HandleSpecificStimuli(CAIStimulusRecord* pStimulusRecord, LTFLOAT* pfRateModifier)
{
	ASSERT(0);	// Abstract function should never get here.
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::OnLinkBroken
//
//	PURPOSE:	Handles a delete object reference.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	AISENSE_RECORD_MAP::iterator it;
	for(it = m_mapSenseRecords.begin(); it != m_mapSenseRecords.end(); ++it)
	{
		// It is OK for the StimulusTarget to go away.
		// (e.g. when something was destroyed by the enemy)

		AISenseRecord* pSenseRecord = it->second;
		if( &pSenseRecord->hLastStimulusSource == pRef )
		{
			ClearSense( pSenseRecord );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::HandleSenses()
//
//	PURPOSE:	Check if senses were updated this cycle, and react accordingly.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::HandleSenses(uint32 nCycle)
{
	AISENSE_RECORD_MAP::iterator it;
	for(it = m_mapSenseRecords.begin(); it != m_mapSenseRecords.end(); ++it)
	{
		AISenseRecord* pSenseRecord = it->second;
		AIBM_Stimulus* pAIBM_Stimulus = pSenseRecord->pAIBM_Last_Stimulus;

		// If pAIBM_Stimulus is NULL, this sense has never been stimulated.
		if(pAIBM_Stimulus == LTNULL)
		{
			continue;
		}

		// Check for senses that were not updated this cycle.
		// Ignore senses that were not updated this cycle because
		// of a delayed stimulus.
		if((pSenseRecord->nCycle != nCycle) && (pSenseRecord->fReactionDelayTimer == 0.f))
		{
			// Decrease the simulation, if there is any.
			if(pSenseRecord->fCurStimulation > 0.f)
			{
				LTBOOL bFalseStimulation = DecreaseStimulation(pSenseRecord, 1.0f);

				// If not a false stimulation, do not let the state handle the sense.
				if(!bFalseStimulation)
				{
					continue;
				}

				// If we have hit the false stimulation limit, treat it as a real stimulation.

				if( pSenseRecord->cFalseStimulation >= pAIBM_Stimulus->nFalseStimulationLimit )
				{
					pSenseRecord->cFalseStimulation = 0;
					pSenseRecord->fCurStimulation	= pAIBM_Stimulus->rngStimulationThreshhold.GetMax();
					pSenseRecord->fMaxStimulation	= pAIBM_Stimulus->rngStimulationThreshhold.GetMax();
		
					AITRACE(AIShowSenses, (m_pSensing->GetSensingObject(), "Hit FalseStimulution Limit %d of %s\n", pAIBM_Stimulus->nFalseStimulationLimit, SenseToString(pSenseRecord->eSenseType)) );
				}

				// Treat other false stimulations as other senses,
				// if an optional FalseStimulusSense was specified.

				else if( pAIBM_Stimulus->eFalseStimulusSense != kSense_InvalidType )
				{
					AISenseRecord* pFalseStimulationSense = GetSense( pAIBM_Stimulus->eFalseStimulusSense );
					if( pFalseStimulationSense )
					{
						pFalseStimulationSense->pAIBM_Last_Stimulus = pAIBM_Stimulus;
						pFalseStimulationSense->hLastStimulusSource = pSenseRecord->hLastStimulusSource;
						pFalseStimulationSense->vLastStimulusPos = pSenseRecord->vLastStimulusPos;
						pFalseStimulationSense->eLastStimulusID = pSenseRecord->eLastStimulusID;
						pFalseStimulationSense->fCurStimulation = pAIBM_Stimulus->rngStimulationThreshhold.GetMax();
						pFalseStimulationSense->fMaxStimulation = pAIBM_Stimulus->rngStimulationThreshhold.GetMax();
						pFalseStimulationSense->fLastStimulationTime = pSenseRecord->fLastStimulationTime;
						pFalseStimulationSense->nLastStimulusAlarmLevel = 1;
						pFalseStimulationSense->nCycle = nCycle;
					}
				}
			}
		}

		// Invalidate hiding spots if AI can already see player.

		if( ( pSenseRecord->eSenseType == kSense_SeeEnemy ) && 
			IsPlayer( pSenseRecord->hLastStimulusSource ) &&
			IsAI( m_pSensing->GetSensingObject() ) &&
			( pSenseRecord->fCurStimulation >= 0.5f ) )
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( pSenseRecord->hLastStimulusSource );
			CAI* pAI = (CAI*)g_pLTServer->HandleToObject( m_pSensing->GetSensingObject() );
			if( pPlayer && pAI )
			{
				pPlayer->SetVisibleToEnemyAI( pAI, true ); 
			}
		}

		// Check if we're fully stimulated.
		if(pSenseRecord->fCurStimulation >= pAIBM_Stimulus->rngStimulationThreshhold.GetMax())
		{
			// Wait for, or increment, ReactionDelay.
			if(pSenseRecord->fReactionDelayTimer >= pSenseRecord->fReactionDelayTime)
			{
				m_pSensing->HandleSenseTrigger( pSenseRecord );
				pSenseRecord->fReactionDelayTimer = 0.f;
				pSenseRecord->fReactionDelayTime = 0.f;
			}
			else pSenseRecord->fReactionDelayTimer += m_pSensing->GetSenseUpdateRate();
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::RepeatHandleSenses()
//
//	PURPOSE:	Handle currently stimulated senses again. This
//              may be necessary in cases like after a goalset change.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::RepeatHandleSenses()
{
	AISENSE_RECORD_MAP::iterator it;
	for(it = m_mapSenseRecords.begin(); it != m_mapSenseRecords.end(); ++it)
	{
		AISenseRecord* pSenseRecord = it->second;
		AIBM_Stimulus* pAIBM_Stimulus = pSenseRecord->pAIBM_Last_Stimulus;

		// If pAIBM_Stimulus is NULL, this sense has never been stimulated.
		if(pAIBM_Stimulus == LTNULL)
		{
			continue;
		}

		// Check if we're fully stimulated.
		if(pSenseRecord->fCurStimulation >= pAIBM_Stimulus->rngStimulationThreshhold.GetMax())
		{
			// Wait for, or increment, ReactionDelay.
			if(pSenseRecord->fReactionDelayTimer >= pSenseRecord->fReactionDelayTime)
			{
				m_pSensing->HandleSenseTrigger( pSenseRecord );
				pSenseRecord->fReactionDelayTimer = 0.f;
				pSenseRecord->fReactionDelayTime = 0.f;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::HasFullStimulation()
//
//	PURPOSE:	Returns true if the AI has full stimulation for a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderAbstract::HasFullStimulation(EnumAISenseType eSenseType)
{
	ASSERT(eSenseType != kSense_InvalidType);

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	if(it == m_mapSenseRecords.end())
	{
		return LTFALSE;
	}

	AISenseRecord* pSenseRecord = it->second;
	AIBM_Stimulus* pStimulus = pSenseRecord->pAIBM_Last_Stimulus;

	if( pStimulus == LTNULL)
	{
		return LTFALSE;
	}

	return (pSenseRecord->fCurStimulation >= pStimulus->rngStimulationThreshhold.GetMax());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::HasFullStimulation()
//
//	PURPOSE:	Returns true if the AI has full stimulation for a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderAbstract::HasAnyStimulation(EnumAISenseType eSenseTypes)
{
	AIASSERT(eSenseTypes != kSense_InvalidType, m_pSensing->GetSensingObject(), "CAISenseRecorderAbstract::HasAnyStimulation: Invalid sense types.");

	AISenseRecord* pSenseRecord;
	AISENSE_RECORD_MAP::iterator it;
	for( it = m_mapSenseRecords.begin(); it != m_mapSenseRecords.end(); ++it )
	{
		pSenseRecord = it->second;
		if( ( pSenseRecord->eSenseType & eSenseTypes ) && ( pSenseRecord->fCurStimulation > 0.f ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::GetStimulation()
//
//	PURPOSE:	Returns current stimulation for a sense.
//
// ----------------------------------------------------------------------- //

LTFLOAT CAISenseRecorderAbstract::GetStimulation(EnumAISenseType eSenseType)
{
	ASSERT(eSenseType != kSense_InvalidType);

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	ASSERT(it != m_mapSenseRecords.end());

	AISenseRecord* pSenseRecord = it->second;

	return pSenseRecord->fCurStimulation;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::ResetStimulation()
//
//	PURPOSE:	Resets current stimulation for a sense.
//
// ----------------------------------------------------------------------- //

void CAISenseRecorderAbstract::ResetStimulation(EnumAISenseType eSenseTypes)
{
	AIASSERT(eSenseTypes != kSense_InvalidType, m_pSensing->GetSensingObject(), "CAISenseRecorderAbstract::ResetStimulation: Invalid sense types.");

	AISenseRecord* pSenseRecord;
	AISENSE_RECORD_MAP::iterator it;
	for( it = m_mapSenseRecords.begin(); it != m_mapSenseRecords.end(); ++it )
	{
		pSenseRecord = it->second;
		if( pSenseRecord->eSenseType & eSenseTypes )
		{
			pSenseRecord->fCurStimulation = 0.f;
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISenseRecorderAbstract::GetStimulusSource()
//              
//	PURPOSE:	Returns the handle to the stimulus source.  NULL if not found,
//				otherwise a valid handle.
//              
//----------------------------------------------------------------------------
HOBJECT CAISenseRecorderAbstract::GetStimulusSource(EnumAISenseType eSenseType)
{
	AIASSERT(eSenseType != kSense_InvalidType, m_pSensing->GetSensingObject(), "CAISenseRecorderAbstract::HasAnyStimulation: Invalid sense types.");

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	if(it == m_mapSenseRecords.end())
	{
		return NULL;
	}

	AISenseRecord* pSenseRecord = it->second;
	AIBM_Stimulus* pStimulus = pSenseRecord->pAIBM_Last_Stimulus;

	UBER_ASSERT( pStimulus!=NULL, "CAISenseRecorderAbstract::GetFullStimulusSource NULL pStimulus" );

	return ( pSenseRecord->hLastStimulusSource );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::IncreaseStimulation()
//
//	PURPOSE:	Increase stimulation of a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderAbstract::IncreaseStimulation(AISenseRecord* pSenseRecord, LTFLOAT fRateModifier)
{
	AIBM_Stimulus* pStimulus = pSenseRecord->pAIBM_Last_Stimulus;

	// Stimulation increase rate depends on alertness.
	LTFLOAT fStimulationIncreaseRate = m_pSensing->IsAlert() ? (pStimulus->fStimulationIncreaseRateAlert) : (pStimulus->fStimulationIncreaseRateUnalert);

	// Current stimulation is either max, or some formula.  Whichever is less.
	pSenseRecord->fCurStimulation = Min<LTFLOAT>( (pStimulus->rngStimulationThreshhold.GetMax()), 
													pSenseRecord->fCurStimulation 
													+ (m_pSensing->GetSenseUpdateRate() * fStimulationIncreaseRate 
														* fRateModifier));

	// Keep track of the max stimulation, so when can detect coming down
	// from full stimulation.
	if(pSenseRecord->fCurStimulation > pSenseRecord->fMaxStimulation)
	{
		pSenseRecord->fMaxStimulation = pSenseRecord->fCurStimulation;
	}

	AITRACE(AIShowSenses, (m_pSensing->GetSensingObject(), "IncreaseStimulution of %s to %f\n", SenseToString(pSenseRecord->eSenseType), pSenseRecord->fCurStimulation) );

	// Return true if sense should be handled.
	if(pSenseRecord->fCurStimulation == pStimulus->rngStimulationThreshhold.GetMax())
	{
		return LTTRUE;
	}
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::DecreaseStimulation
//
//	PURPOSE:	Decay the stimulation of the sense
//
// ----------------------------------------------------------------------- //

LTBOOL CAISenseRecorderAbstract::DecreaseStimulation(AISenseRecord* pSenseRecord, LTFLOAT fRateModifier)
{
	AIBM_Stimulus* pStimulus = pSenseRecord->pAIBM_Last_Stimulus;

	// Stick at max until HandleSense handles it.
//	if(pSenseRecord->fCurStimulation >= pStimulus->rngStimulationThreshhold.GetMax())
//	{
//		return LTFALSE;
//	}

	LTFLOAT fLastStimulation = pSenseRecord->fCurStimulation;

	// Stimulation decrease rate depends on alertness.
	LTFLOAT fStimulationDecreaseRate = m_pSensing->IsAlert() ? (pStimulus->fStimulationDecreaseRateAlert) : (pStimulus->fStimulationDecreaseRateUnalert);

	// Current stimulation is either 0, or some formula.  Whichever is more.
	pSenseRecord->fCurStimulation = Max<LTFLOAT>(0.0f, pSenseRecord->fCurStimulation
														- (m_pSensing->GetSenseUpdateRate() * fStimulationDecreaseRate 
														* fRateModifier));

	AITRACE(AIShowSenses, (m_pSensing->GetSensingObject(), "DecreaseStimulution of %s to %f\n", SenseToString(pSenseRecord->eSenseType), pSenseRecord->fCurStimulation) );

	// If stimulation has dropped below the partial stimulation
	// threshold, it was a false stimulation.
	if( ( pStimulus->nFalseStimulationLimit > 0 ) && 
		( pSenseRecord->fMaxStimulation < pStimulus->rngStimulationThreshhold.GetMax() ) &&
		( pSenseRecord->fCurStimulation < pStimulus->rngStimulationThreshhold.GetMin() ) && 
		( fLastStimulation >= pStimulus->rngStimulationThreshhold.GetMin() ) )
	{
		++(pSenseRecord->cFalseStimulation);

		AITRACE(AIShowSenses, (m_pSensing->GetSensingObject(), "FalseStimulution %d/%d of %s\n",
			pSenseRecord->cFalseStimulation,
			pStimulus->nFalseStimulationLimit,
			SenseToString(pSenseRecord->eSenseType)) );

		return LTTRUE;
	}

	if( pSenseRecord->fCurStimulation == 0.f )
	{
		pSenseRecord->fMaxStimulation = 0.f;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::GetSense
//
//	PURPOSE:	Get a sense record.
//
// ----------------------------------------------------------------------- //

AISenseRecord* CAISenseRecorderAbstract::GetSense(EnumAISenseType eSenseType)
{
	AIASSERT(eSenseType != kSense_InvalidType, m_pSensing->GetSensingObject(), "CAISenseRecorderAbstract::GetSense: Invalid sense type.");

	// Find sense record for stimulus.
	AISENSE_RECORD_MAP::iterator it = m_mapSenseRecords.find(eSenseType);
	if(it == m_mapSenseRecords.end())
	{
		return NULL;
	}

	return it->second;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISenseRecorderAbstract::SenseToString/SenseFromString
//
//	PURPOSE:	Convert a sense enum to a string and viceversa.
//
// ----------------------------------------------------------------------- //

const char* CAISenseRecorderAbstract::SenseToString(EnumAISenseType eSenseType)
{
	for( uint32 iSense=0; iSense < kSense_Count; ++iSense )
	{
		if( eSenseType & (1 << iSense) )
		{
			return s_aszSenseTypes[iSense];
		}
	}

	return LTNULL;
}

EnumAISenseType CAISenseRecorderAbstract::SenseFromString(char* szSenseType)
{
	for(uint32 iSenseType = 0; iSenseType < kSense_Count; ++iSenseType)
	{
		if( stricmp(szSenseType, s_aszSenseTypes[iSenseType]) == 0 )
		{
			return (EnumAISenseType)(1 << iSenseType);
		}
	}

	return kSense_InvalidType;
}
