// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSquadCommunicationThreat.cpp
//
// PURPOSE : AISensorSquadCommunicationThreat class implementation
//
// CREATED : 8/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSquadCommunicationThreat.h"
#include "AI.h"
#include "AIStimulusMgr.h"
#include "AIBlackBoard.h"
#include "AICoordinator.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSquadCommunicationThreat, kSensor_SquadCommunicationThreat );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSquadCommunicationThreat::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorSquadCommunicationThreat::CAISensorSquadCommunicationThreat()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorSquadCommunicationThreat::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the sensor.
//              
//----------------------------------------------------------------------------

void CAISensorSquadCommunicationThreat::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAISensorSquadCommunicationThreat::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSquadCommunicationThreat::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorSquadCommunicationThreat::GetSenseDistSqr( float fStimulusRadius )
{
	return FLT_MAX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSquadCommunicationThreat::CreateWorkingMemoryFact.
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorSquadCommunicationThreat::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Threat is referenced by stimulus.

	HOBJECT hThreat = pStimulusRecord->m_hStimulusTarget;

	// Ensure we become alert.

	m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Alert );


	// Find an existing memory for this character.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Character);
	factQuery.SetTargetObject(hThreat);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for this character.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Character );
	}

	// Track squad's knowledge of the target's position,
	// until we actually see the target ourselves.

	uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();
	dwFlags = dwFlags & ~kTargetTrack_Normal;
	dwFlags |= kTargetTrack_Squad;
	m_pAI->GetAIBlackBoard()->SetBBTargetPosTrackingFlags( dwFlags );

	// Re-evaluate targets when a new character is discovered.

	m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSquadCommunicationThreat::SetFactStimulus
//
//	PURPOSE:	Set the stimulus that generated the fact.
//
// ----------------------------------------------------------------------- //

void CAISensorSquadCommunicationThreat::SetFactStimulus( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord, float fConfidence )
{
	if( !( pFact && pStimulusRecord ) )
	{
		return;
	}

	// Set the confidence to 0.0, since we didn't truly see anything.

	pFact->SetStimulus( kStim_CharacterVisible,
						pStimulusRecord->m_eStimulusID,
						0.f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSquadCommunicationThreat::SetFactTargetObject
//
//	PURPOSE:	Set the target object the WMFact.
//
// ----------------------------------------------------------------------- //

void CAISensorSquadCommunicationThreat::SetFactTargetObject( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord )
{
	// Intentionally do NOT call super::SetFactTargetObject.
	// Target the stimulus target rather than the source.

	// Sanity check.

	if( !( pFact && pStimulusRecord ) )
	{
		return;
	}

	// Who is my ally warning me about?

	pFact->SetTargetObject( pStimulusRecord->m_hStimulusTarget, 1.f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSquadCommunicationThreat::DoComplexCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorSquadCommunicationThreat::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier )
{
	// Sanity check.

	if( !( pStimulusRecord && pfRateModifier ) )
	{
		return false;
	}

	// Ignore stimuli if not in a squad.

	ENUM_AI_SQUAD_ID eMySquadID = g_pAICoordinator->GetSquadID( m_pAI->m_hObject );
	if( eMySquadID == kSquad_Invalid )
	{
		return false;
	}

	// Ignore stimuli from other squads.

	ENUM_AI_SQUAD_ID eOtherSquadID = g_pAICoordinator->GetSquadID( pStimulusRecord->m_hStimulusSource );
	if( eMySquadID != eOtherSquadID )
	{
		return false;
	}

	// Accept stimulus.

	return true;
}



