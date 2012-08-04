// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeDisturbance.cpp
//
// PURPOSE : AISensorSeeDisturbance class implementation
//
// CREATED : 3/31/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSeeDisturbance.h"
#include "AIStimulusMgr.h"
#include "AISoundMgr.h"
#include "AICoordinator.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeDisturbance, kSensor_SeeDisturbance );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDisturbance::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorSeeDisturbance::CAISensorSeeDisturbance()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDisturbance::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorSeeDisturbance::GetSenseDistSqr( float fStimulusRadius )
{
	float fSenseDistanceSqr = m_pAI->GetAIBlackBoard()->GetBBSeeDistance() + fStimulusRadius;
	return fSenseDistanceSqr * fSenseDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDisturbance::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeDisturbance::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// HACK: Ignore dead bodies while advancing or following.
	 
	if( pStimulusRecord && pStimulusRecord->m_eStimulusType == kStim_DeathVisible )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Task );
		factQuery.SetTaskType( kTask_Advance );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( !pFact )
		{
			factQuery.SetTaskType( kTask_Follow );
			pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		}
		if( pFact )
		{
			return false;
		}
	}

	// Default behavior.

	if( !super::StimulateSensor( pStimulusRecord ) )
	{
		return false;
	}

	// AI saw an ally's death or body.

	bool bShooterIsTurret = false;
	if( pStimulusRecord->m_eStimulusType == kStim_DeathVisible )
	{
	}
	else if( pStimulusRecord->m_eStimulusType == kStim_WeaponImpactVisible )
	{
		m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Alert );

		if( IsPlayer( pStimulusRecord->m_hStimulusSource ) )
		{
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( pStimulusRecord->m_hStimulusSource );
			if( pPlayer && pPlayer->GetTurret() )
			{
				bShooterIsTurret = true;
			}
		}

		// Announce seeing the death of an ally.

		if( IsDeadAI( pStimulusRecord->m_hStimulusTarget ) )
		{
			ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( m_pAI->m_hObject );
			CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquad );
			uint32 cBodyCount = m_pAI->GetAIBlackBoard()->GetBBBodyCount();

			// "Heavy armor down!"
			// "Powered armor down!"

			CAI* pAIDead = (CAI*)g_pLTServer->HandleToObject( pStimulusRecord->m_hStimulusTarget );
			if( pAIDead && LTStrIEquals( pAIDead->GetAIAttributes()->strName.c_str(), "HeavyArmor" ) )
			{
				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_HeavyArmorDown, kAISndCat_Event, NULL, 1.f );
			}
			else if( pAIDead && LTStrIEquals( pAIDead->GetAIAttributes()->strName.c_str(), "PoweredArmor" ) )
			{
				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_PoweredArmorDown, kAISndCat_Event, NULL, 1.f );
			}

			// "We've got a sniper!"

			else if(( !bShooterIsTurret ) && 
					( m_pAI->GetAIBlackBoard()->GetBBTargetFirstThreatTime() == 0.f ) )
			{
				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_SniperDetected, kAISndCat_Event, NULL, 0.5f );
			}

			// "I need backup NOW!"

			else if( pSquad && pSquad->GetNumSquadMembers() == 1 )
			{
				// "He wiped out the whole squad!"

				if( cBodyCount > 1 )
				{
					g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_ManDownAll, kAISndCat_Event, NULL, 1.f );
				}

				// "I need backup NOW!"

				else {
					g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_BackupUrgent, kAISndCat_Event, NULL, 1.f );
				}
			}

			// "I've got two men down!"

			else if( cBodyCount == 2 )
			{
				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_ManDownTwo, kAISndCat_ManDown, NULL, 1.f );
			}

			// "I've got three men down!"

			else if( cBodyCount == 3 )
			{
				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_ManDownThree, kAISndCat_ManDown, NULL, 1.f );
			}

			// "Man down!"

			else
			{
				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_ManDown, kAISndCat_ManDown, NULL, 1.f );
			}
		}

		// Saw someone get shot, but not killed.

		else if( !m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
		{
			HOBJECT hAlly = g_pAICoordinator->FindAlly( m_pAI->m_hObject, NULL );
			if( hAlly )
			{
				// "Check it out!"
				// "Roger!"

				g_pAISoundMgr->RequestAISound( hAlly, kAIS_OrderInvestigate, kAISndCat_DisturbanceHeard, NULL, 1.f );
				g_pAISoundMgr->RequestAISoundSequence( m_pAI->m_hObject, kAIS_Affirmative, hAlly, kAIS_OrderInvestigate, kAIS_OrderInvestigate, kAISndCat_Event, NULL, 0.3f );
			}
			else {
				// "What was that?"

				g_pAISoundMgr->RequestAISound( m_pAI->m_hObject, kAIS_DisturbanceHeardAlarming, kAISndCat_DisturbanceHeard, NULL, 1.f );
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDisturbance::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorSeeDisturbance::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
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
//	ROUTINE:	CAISensorSeeDisturbance::ComplexVisibilityCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeDisturbance::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* /*pfRateModifier*/ )
{
	HOBJECT hTarget = pStimulusRecord->m_hStimulusTarget;

	// Look at the stimulus target if it exists.
	// Otherwise, look at the stimulus position.

	LTVector vPos;
	if( hTarget )
	{
		g_pLTServer->GetObjectPos( hTarget, &vPos );
	}
	else if( pStimulusRecord->m_eStimulusType == kStim_DeathVisible )
	{
		hTarget = pStimulusRecord->m_hStimulusSource;
		g_pLTServer->GetObjectPos( hTarget, &vPos );
	}
	else
	{
		vPos = pStimulusRecord->m_vStimulusPos;
	}

	// Check visibility.

	bool bVisible;
	HOBJECT hBlocking = NULL;
	float fSenseDistanceSqr = GetSenseDistSqr( pStimulusRecord->m_fDistance );
	if( m_pAI->CanSeeThrough() )
	{
		bVisible = m_pAI->IsObjectPositionVisible( CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, m_pAI->GetEyePosition(), NULL, vPos, fSenseDistanceSqr, true, true, &hBlocking );
	}
	else
	{
		bVisible = m_pAI->IsObjectPositionVisible( CAI::DefaultFilterFn, NULL, m_pAI->GetEyePosition(), NULL, vPos, fSenseDistanceSqr, true, false, &hBlocking );
	}

	// Position is visible, or blocked by the disturbance target.

	if( ( !bVisible ) &&
		( hTarget && ( hTarget == hBlocking ) ) )
	{
		bVisible = true;
	}

	return bVisible;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDisturbance::IncreaseStimulation
//
//	PURPOSE:	If the AI becomes fully stimuluated, re-evaluate the 
//				target.  As this behavior may or may not be desirable for 
//				different games, allow opting out.
//
// ----------------------------------------------------------------------- //

float CAISensorSeeDisturbance::IncreaseStimulation( float fCurStimulation, float fRateModifier, CAIStimulusRecord* pStimulusRecord, AIDB_StimulusRecord* pStimulus )
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
