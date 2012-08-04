// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeekEnemy.cpp
//
// PURPOSE : AISensorSeekEnemy class implementation
//
// CREATED : 12/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSeekEnemy.h"
#include "AI.h"
#include "AIDB.h"
#include "AISensorMgr.h"
#include "AIStimulusMgr.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeekEnemy, kSensor_SeekEnemy );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeekEnemy::CAISensorSeekEnemy
//
//	PURPOSE:	Constructor.
//
// ----------------------------------------------------------------------- //

CAISensorSeekEnemy::CAISensorSeekEnemy()
{
	m_hEnemy = NULL;
	m_bEnemyReset = false;
	m_bSeekSquadEnemy = false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorSeekEnemy::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorSeekEnemy
//
//----------------------------------------------------------------------------

void CAISensorSeekEnemy::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hEnemy );
	SAVE_bool( m_bEnemyReset );
	SAVE_bool( m_bSeekSquadEnemy );
}

void CAISensorSeekEnemy::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hEnemy );
	LOAD_bool( m_bEnemyReset );
	LOAD_bool( m_bSeekSquadEnemy );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeekEnemy::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeekEnemy::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Bail if enemy is gone.

	if( !m_hEnemy )
	{
		SeekEnemy( false );
		m_pAI->GetAISensorMgr()->RemoveAISensor( kSensor_SeekEnemy );
		return false;
	}

	// Enemy may be a character, or something else (e.g. Turret).

	ENUM_AIWMFACT_TYPE eFactType = IsCharacter( m_hEnemy ) ? kFact_Character : kFact_Object;

	// Find an existing memory for this character,
	// or create a new memory for this character.

	CAIWMFact factQuery;
	factQuery.SetFactType( eFactType );
	factQuery.SetTargetObject( m_hEnemy );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( eFactType );

		// Setup faked stimulus with 0.0 confidence, to differentiate from a real stimulus.

		pFact->SetStimulus( kStim_CharacterVisible, g_pAIStimulusMgr->GetNextStimulusID(), 0.f );
		pFact->SetTargetObject( m_hEnemy, 1.f );
		pFact->SetFactFlags( 0 );

		// Set the stimulus distance and direction.

		AIDB_StimulusRecord* pAIDB_Stimulus = g_pAIDB->GetAIStimulusRecord( kStim_CharacterVisible );
		if( pAIDB_Stimulus )
		{
			pFact->SetRadius( pAIDB_Stimulus->fDistance, 1.f );
			pFact->SetDir( LTVector( 0.f, 1.f, 0.f ), 1.f );
		}

		// Re-evaluate targets when a new enemy is discovered.

		m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
	}

	// Clear any stimulus confidence when an enemy is set.

	if( m_bEnemyReset )
	{
		SeekEnemy( true );
		pFact->SetConfidence( CAIWMFact::kFactMask_Stimulus, 0.f );
		m_bEnemyReset = false;
	}

	// Destroy sensor if AI has actually sensed the enemy.

	else if( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) > 0.f )
	{
		SeekEnemy( false );
		m_pAI->GetAISensorMgr()->RemoveAISensor( kSensor_SeekEnemy );
		return false;
	}

	// Sensor forces AI to be 100% sure of target's location.

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hEnemy, &vPos );
	pFact->SetPos( vPos, 1.f );

	// Set the update time.

	pFact->SetUpdateTime( g_pLTServer->GetTime() );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeekEnemy::SeekEnemy
//
//	PURPOSE:	Set variables for seeking an enemy, or not.
//
// ----------------------------------------------------------------------- //

void CAISensorSeekEnemy::SeekEnemy( bool bSeek )
{
	// Stop tracking the target position if no longer seeking.

	uint32 dwFlags = m_pAI->GetAIBlackBoard()->GetBBTargetPosTrackingFlags();
	
	// Seeking a squad enemy.

	if( m_bSeekSquadEnemy )
	{
		if( bSeek )
		{
			dwFlags = kTargetTrack_Squad;
		}
		else {
			dwFlags = kTargetTrack_Normal;
		}
	}

	// Default behavior.

	else
	{
		if( bSeek )
		{
			dwFlags |= kTargetTrack_SeekEnemy;
		}
		else {
			dwFlags = dwFlags & ~kTargetTrack_SeekEnemy;
		}
	}

	m_pAI->GetAIBlackBoard()->SetBBTargetPosTrackingFlags( dwFlags );
}

