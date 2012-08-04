// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorHearDisturbance.cpp
//
// PURPOSE : AISensorHearDisturbance class implementation
//
// CREATED : 2/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorHearDisturbance.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemoryCentral.h"
#include "AIStimulusMgr.h"
#include "AICoordinator.h"
#include "AISoundMgr.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorHearDisturbance, kSensor_HearDisturbance );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorHearDisturbance::CAISensorHearDisturbance()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorHearDisturbance::GetSenseDistSqr( float fStimulusRadius )
{
	float fSenseDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBHearDistance() + fStimulusRadius;
	return fSenseDistanceSqr * fSenseDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorHearDisturbance::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// Stimuli resulting from physics collisions specify both the 
	// source and target of the stimulus (the two collision actors).

	if( pStimulusRecord->m_hStimulusTarget )
	{
		// Character disturbed an object that is a non-character.
		// If we do not hate the character, we should ignore future disturbances
		// from this object (e.g. object continues colliding with the world 
		// after it gets knocked over by some character).

		HOBJECT hChar, hObject;
		if( DidCharacterDisturbObject( pStimulusRecord, &hChar, &hObject ) )
		{
			// Keep track of which character caused a disturbance with this object.

			RecordLastDisturbanceSourceCharacter( hChar, hObject );
		}

		// Ignore this disturbance.

		if( IgnoreDisturbanceBetweenObjects( pStimulusRecord ) )
		{
			return false;
		}
	}

	// Default behavior.

	return super::StimulateSensor( pStimulusRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::DidCharacterDisturbObject.
//
//	PURPOSE:	Return true if exactly one of the disturbance actors 
//              is a character.  
//              Return false if both are characters.
//              Return false if character is disturbing the main world.
//
// ----------------------------------------------------------------------- //

bool CAISensorHearDisturbance::DidCharacterDisturbObject( CAIStimulusRecord* pStimulusRecord, HOBJECT* phChar, HOBJECT* phObject )
{
	// Sanity check.

	if( !( pStimulusRecord && phChar && phObject ) )
	{
		return false;
	}

	HOBJECT hSource = pStimulusRecord->m_hStimulusSource;
	HOBJECT hTarget = pStimulusRecord->m_hStimulusTarget;

	// Main world is being disturbed.

	if( IsMainWorld( hSource ) ||
		IsMainWorld( hTarget ) )
	{
		return false;
	}

	// Both are characters.

	bool bSourceIsChar = IsCharacter( hSource );
	bool bTargetIsChar = IsCharacter( hTarget );
	if( bSourceIsChar && bTargetIsChar )
	{
		return false;
	}

	// Source is character.

	if( bSourceIsChar )
	{
		*phChar = hSource;
		*phObject = hTarget;
		return true;
	}

	// Target is character.

	if( bTargetIsChar )
	{
		*phChar = hTarget;
		*phObject = hSource;
		return true;
	}

	// None are characters.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::RecordLastDisturbanceSourceCharacter.
//
//	PURPOSE:	Record which character last disturbed this object.
//
// ----------------------------------------------------------------------- //

void CAISensorHearDisturbance::RecordLastDisturbanceSourceCharacter( HOBJECT hChar, HOBJECT hObject )
{
	// Sanity check.

	if( !hObject )
	{
		return;
	}

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_LastDisturbanceSourceCharacter );
	factQuery.SetTargetObject( hObject );

	// Record who last disturbed this object.

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
	if( !pFact )
	{
		pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
		pFact->SetKnowledgeType( kKnowledge_LastDisturbanceSourceCharacter, 1.f );
		pFact->SetTargetObject( hObject, 1.f );
	}
	if( pFact )
	{
		pFact->SetSourceObject( hChar, 1.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::IgnoreDisturbancesBetweenObjects.
//
//	PURPOSE:	Return true if AI should ignore the disturbance caused by 
//              the source and target of the stimulus.
//
// ----------------------------------------------------------------------- //

bool CAISensorHearDisturbance::IgnoreDisturbanceBetweenObjects( CAIStimulusRecord* pStimulusRecord )
{
	// Sanity check.

	if( !pStimulusRecord )
	{
		return false;
	}

	HOBJECT hSource = pStimulusRecord->m_hStimulusSource;
	HOBJECT hTarget = pStimulusRecord->m_hStimulusTarget;

	// We are ignoring the source object.

	if( ( !IsCharacter( hSource ) ) &&
		( !IsMainWorld( hSource ) ) )
	{
		if( IgnoreDisturbanceFromObject( hSource ) )
		{
			return true;
		}
	}

	// We are ignoring the target object.

	if( ( !IsCharacter( hTarget ) ) &&
		( !IsMainWorld( hTarget ) ) )
	{
		if( IgnoreDisturbanceFromObject( hTarget ) )
		{
			return true;
		}
	}

	// Do not ignore this disturbance.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::IgnoreDisturbanceFromObject.
//
//	PURPOSE:	Return true if AI should ignore a disturbance caused by 
//              this object.
//
// ----------------------------------------------------------------------- //

bool CAISensorHearDisturbance::IgnoreDisturbanceFromObject( HOBJECT hObject )
{
	// Sanity check.

	if( !hObject )
	{
		return false;
	}

	// Ignore this object if we do not hate the last character that caused
	// a disturbance with this object.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_LastDisturbanceSourceCharacter );
	factQuery.SetTargetObject( hObject );
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
	if( pFact && IsCharacter( pFact->GetSourceObject() ) )
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( pFact->GetSourceObject() );
		if( kCharStance_Hate != g_pCharacterDB->GetStance( m_pAI->GetAlignment(), pChar->GetAlignment() ) )
		{
			return true;
		}
	}

	// Do not ignore this object.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorHearDisturbance::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
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

	// If the disturbance originated from a lost target,
	// AI should now know where the target is.

	if( m_pAI->HasTarget( kTarget_Character ) &&
		( m_pAI->GetAIBlackBoard()->GetBBTargetObject() == pStimulusRecord->m_hStimulusSource ) &&
		( m_pAI->GetAIBlackBoard()->GetBBTargetPosUpdateTime() < g_pLTServer->GetTime() - 5.f ) &&
		( !( m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags() & kTargetTrack_Normal ) ) )
	{
		uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();
		dwFlags |= kTargetTrack_Once;
		m_pAI->GetAIBlackBoard()->SetBBTargetPosTrackingFlags( dwFlags );

		// Announce hearing the lost target.

		ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( m_pAI->m_hObject );
		CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
		if( pSquad && !pSquad->SquadCanSeeTarget( kTarget_Character ) )
		{
			g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_HearLostTarget, kAISndCat_DisturbanceHeard, NULL, 0.5f );
		}
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
//	ROUTINE:	CAISensorHearDisturbance::IncreaseStimulation
//
//	PURPOSE:	If the AI becomes fully stimuluated, re-evaluate the 
//				target.  As this behavior may or may not be desirable for 
//				different games, allow opting out.
//
// ----------------------------------------------------------------------- //

float CAISensorHearDisturbance::IncreaseStimulation( float fCurStimulation, float fRateModifier, CAIStimulusRecord* pStimulusRecord, AIDB_StimulusRecord* pStimulus )
{
	float fStimulation = super::IncreaseStimulation( fCurStimulation, fRateModifier, pStimulusRecord, pStimulus );

	if( ( fCurStimulation < 1.f ) && ( fStimulation >= 1.f ) )
	{
		if ( g_pAIDB->GetAIConstantsRecord()->bDisturbancesCauseTargetSelection )
		{
			m_pAI->GetAIBlackBoard()->SetBBSelectTarget( true );
		}
	}

	return fStimulation;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorHearDisturbance::FindUnstimulatedWorkingMemoryFact
//
//	PURPOSE:	Return a working memory fact that has not been 
//              stimulated this update.
//
// ----------------------------------------------------------------------- //

void CAISensorHearDisturbance::FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList)
{
	if (!pOutFactList)
	{
		return;
	}

	// Give AI a one second grace period to notice disturbances, before their stimulation decreases.

	double fComparisonTime = m_pAI->GetAISensorMgr()->GetStimulusListNewIterationTime();
	fComparisonTime -= 1.f;
	m_pAI->GetAIWorkingMemory()->CollectFactsUnupdated(kFact_Disturbance, pOutFactList, fComparisonTime);
}
