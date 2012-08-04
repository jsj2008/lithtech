// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeeDanger.cpp
//
// PURPOSE : AISensorSeeDanger class implementation
//
// CREATED : 5/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorSeeDanger.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIStimulusMgr.h"
#include "AIUtils.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeeDanger, kSensor_SeeDanger );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDanger::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorSeeDanger::CAISensorSeeDanger()
{
	m_bDangerExists = false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorSeeDanger::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorNode 
//              
//----------------------------------------------------------------------------

void CAISensorSeeDanger::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_bool( m_bDangerExists );
}

void CAISensorSeeDanger::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_bool( m_bDangerExists );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDanger::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeDanger::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	// Sanity check.

	if( !pStimulusRecord )
	{
		return false;
	}

	// Treat dangerous stimuli as Danger if we've not yet seen a threat.
	// AI that are set to ImmediateThreat are not startled by danger.

	if( ( m_pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime() == 0.f ) &&
		( m_pAI->GetAIBlackBoard()->GetBBAwarenessMod() != kAwarenessMod_ImmediateThreat ) )
	{
		switch( pStimulusRecord->m_eStimulusType )
		{
			case kStim_PainSound:
			case kStim_DeathSound:
			case kStim_WeaponImpactSound:
				{
					// Only react to stimuli as Danger once.
					// But, if AI were previously aware of the Player, they should
					// still be surprised by a turret.

					// AI has not yet noticed a Player turret.

					bool bDangerExists = false;
					if( IsPlayer( pStimulusRecord->m_hStimulusSource ) )
					{
						CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( pStimulusRecord->m_hStimulusSource );
						if( pPlayer && pPlayer->GetTurret() && !m_pAI->HasTarget( kTarget_Object ) )
						{
							bDangerExists = true;
						}
					}

					// AI has not yet noticed any danger.

					if( !m_pAI->HasTarget( kTarget_Character ) )
					{
						CAIWMFact factQuery;
						factQuery.SetFactType( kFact_Knowledge );
						factQuery.SetKnowledgeType( kKnowledge_FirstDangerTime );
						if( !m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery ) )
						{
							bDangerExists = true;
						}
					}

					// Create knowledge of danger.

					if( bDangerExists )
					{
						CAIWMFact* pDangerFact = CreateWorkingMemoryFact( pStimulusRecord );
						pDangerFact->SetSourceObject( pStimulusRecord->m_hStimulusSource, 1.f );
						pDangerFact->SetPos( pStimulusRecord->m_vStimulusPos, 1.f );
						pDangerFact->SetRadius( pStimulusRecord->m_fDistance, 1.f );
					}
				}
				break;
		}
	}

	return super::StimulateSensor( pStimulusRecord );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDanger::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeDanger::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}
/**
	// AI was aware of danger.

	if(	m_bDangerExists )
	{
		// Danger still exists.

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Danger );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			m_pAI->GetAIWorldState()->SetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject, kWST_bool, false );
		}

		// No more danger.

		else {
			m_pAI->GetAIWorldState()->SetWSProp( kWSK_PositionIsValid, m_pAI->m_hObject, kWST_bool, true );
			m_bDangerExists = false;
		}
	}
**/
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDanger::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorSeeDanger::GetSenseDistSqr( float fStimulusRadius )
{
	float fSenseDistanceSqr = 2.f * fStimulusRadius;
	return fSenseDistanceSqr * fSenseDistanceSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDanger::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorSeeDanger::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Sanity check.

	if( !pStimulusRecord )
	{
		return NULL;
	}
	
	// Ensure AI goes alert at the first sign of danger!!

	m_pAI->GetAIBlackBoard()->SetBBAwareness( kAware_Alert );

	// Grenades are the stimulus target, while other objects are listed as the source.

	HOBJECT hSource = pStimulusRecord->m_hStimulusTarget;
	if( !hSource )
	{
		hSource = pStimulusRecord->m_hStimulusSource;
	}

	// Find an existing memory for this dangerous object.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Danger );
	factQuery.SetSourceObject( hSource );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for this dangerous object.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Danger );
		pFact->SetSourceObject( hSource, 1.f );

		// Record the existence of a disturbance in the AI's world state.

		m_pAI->GetAIWorldState()->SetWSProp( kWSK_DisturbanceExists, m_pAI->m_hObject, kWST_bool, true );

		m_bDangerExists = true;
	}

	// Record knowledge of sensing danger.
	// This prevents AI from reacting to stimuli as Danger more than once.

	CAIWMFact factKnowledgeQuery;
	factKnowledgeQuery.SetFactType( kFact_Knowledge );
	factKnowledgeQuery.SetKnowledgeType( kKnowledge_FirstDangerTime );
	if( !m_pAI->GetAIWorkingMemory()->FindWMFact( factKnowledgeQuery ) )
	{
		CAIWMFact* pKnowledgeFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
		pKnowledgeFact->SetKnowledgeType( kKnowledge_FirstDangerTime, 1.f );
		pKnowledgeFact->SetTime( g_pLTServer->GetTime() );
	}

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDanger::FindUnstimulatedWorkingMemoryFact
//
//	PURPOSE:	Return a working memory fact that has not been 
//              stimulated this update.
//
// ----------------------------------------------------------------------- //

void CAISensorSeeDanger::FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList)
{
	if (!pOutFactList)
	{
		return;
	}

	m_pAI->GetAIWorkingMemory()->CollectFactsUnupdated( kFact_Danger, pOutFactList, g_pLTServer->GetTime() );
/****
	AIWORKING_MEMORY_FACT_LIST::iterator iter;
	for ( iter = pOutFactList->begin(); iter != pOutFactList->end(); ++iter)
	{
		CAIWMFact* pFact = *iter;
		if( pFact && ( pFact->GetSourceObject() == NULL ) ) 
		{
			m_pAI->GetAIWorkingMemory()->ClearWMFact( pFact );
		}
		*iter = NULL;
	}
****/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorSeeDanger::ComplexVisibilityCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorSeeDanger::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* /*pfRateModifier*/ )
{
	// Look at the stimulus position.

	LTVector vPos = pStimulusRecord->m_vStimulusPos;

	// Check visibility.
	// NOTE: Ignoring FOV if alert.

	bool bFOVCheck = ( m_pAI->GetAIBlackBoard()->GetBBAwareness() == kAware_Alert ) ? false : true;
	HOBJECT hTarget = pStimulusRecord->m_hStimulusTarget;
	float fSenseDistanceSqr = GetSenseDistSqr( pStimulusRecord->m_fDistance );
	if( m_pAI->CanSeeThrough() )
	{
		return !!m_pAI->IsObjectPositionVisible( CAI::SeeThroughFilterFn, CAI::SeeThroughPolyFilterFn, m_pAI->GetEyePosition(), hTarget, vPos, fSenseDistanceSqr, bFOVCheck, true );
	}
	else
	{
		return !!m_pAI->IsObjectPositionVisible( CAI::DefaultFilterFn, NULL, m_pAI->GetEyePosition(), hTarget, vPos, fSenseDistanceSqr, bFOVCheck, false );
	}
}

