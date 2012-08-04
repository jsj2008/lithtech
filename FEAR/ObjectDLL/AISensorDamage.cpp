// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorDamage.cpp
//
// PURPOSE : AISensorDamage class implementation
//
// CREATED : 3/26/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorDamage.h"
#include "AI.h"
#include "AIDB.h"
#include "AIStimulusMgr.h"
#include "AISoundMgr.h"
#include "AIBlackBoard.h"
#include "AIWorldState.h"
#include "AIWorkingMemoryCentral.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorDamage, kSensor_Damage );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamage::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorDamage::CAISensorDamage()
{
	m_fStimulationThreshold = 0.f;
	m_fStimulationMax = 1000.f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorDamage::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorDamage
//
//----------------------------------------------------------------------------
void CAISensorDamage::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAISensorDamage::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamage::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorDamage::GetSenseDistSqr( float fStimulusRadius )
{
	return FLT_MAX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamage::StimulateSensor.
//
//	PURPOSE:	Stimulate the sensor.
//
// ----------------------------------------------------------------------- //

bool CAISensorDamage::StimulateSensor( CAIStimulusRecord* pStimulusRecord )
{
	if( super::StimulateSensor( pStimulusRecord ) )
	{
		// Delay any AI from saying "Nice shot!" for some time
		// after getting shot by the Player.  It sounds weird when 
		// AI say "Nice shot!" after the Player shot them.  Sounds
		// like AI are complimenting the Player, when they are really
		// speaking to each other.

		HOBJECT hDamager = pStimulusRecord->m_hStimulusTarget;
		if( IsPlayer( hDamager ) )
		{
			g_pAISoundMgr->SkipAISound( m_pAI->m_hObject, kAIS_HitSeen );
		}

		// Re-evaluate target if someone other than the current 
		// target inflicted damage.

		if( !IsDamagerTarget( hDamager ) )
		{
			m_pAI->GetAIBlackBoard()->SetBBInvalidateTarget( true );
		}

		// Always reevaluate the behavior.  Taking damage is an event which 
		// needs immediate response if we are playing an animation as a 
		// result.

		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );

		// If the AI was just damaged at a node, record the fact in Central 
		// Memory so all AIs can avoid this node for a while.

		SAIWORLDSTATE_PROP* pProp = m_pAI->GetAIWorldState()->GetWSProp( kWSK_AtNode, m_pAI->m_hObject );
		if (pProp && pProp->hWSValue)
		{
			// An AI has been damaged at this node.

			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Knowledge);
			factQuery.SetKnowledgeType(kKnowledge_DamagedAtNode);
			factQuery.SetTargetObject(pProp->hWSValue);

			CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
			float fCurrentDamage = 0.f;
			if (!pFact)
			{
				pFact = g_pAIWorkingMemoryCentral->CreateWMFact(kFact_Knowledge);
			}
			else
			{
				pFact->GetDamage( NULL, &fCurrentDamage, NULL );
			}

			if (pFact)
			{
				float fDelay = GetRandom( g_pAIDB->GetAIConstantsRecord()->fDamagedAtNodeAvoidanceTimeMin,
										  g_pAIDB->GetAIConstantsRecord()->fDamagedAtNodeAvoidanceTimeMax );

				pFact->SetKnowledgeType( kKnowledge_DamagedAtNode, 1.f );
				pFact->SetTargetObject( pProp->hWSValue, 1.f );
				pFact->SetTime( g_pLTServer->GetTime() + fDelay, 1.f );
				pFact->SetDamage( pStimulusRecord->m_eDamageType, fCurrentDamage + pStimulusRecord->m_fDamageAmount, pStimulusRecord->m_vDamageDir, 1.f );
			}
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamage::IsDamagerTarget
//
//	PURPOSE:	Return true if the damager is already the current target.
//
// ----------------------------------------------------------------------- //

bool CAISensorDamage::IsDamagerTarget( HOBJECT hDamager )
{
	return ( hDamager == m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamage::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorDamage::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Find an existing memory for damage.

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Damage);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for damage.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Damage );
	}

	if( pFact )
	{
		// Damage type and amount.

		pFact->SetDamage( pStimulusRecord->m_eDamageType, pStimulusRecord->m_fDamageAmount, pStimulusRecord->m_vDamageDir, 1.f );

		// Where was I shot?

		pFact->SetPos( pStimulusRecord->m_vStimulusPos, 1.f );
	}

	return pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamage::SetFactTargetObject
//
//	PURPOSE:	Set the target object the WMFact.
//
// ----------------------------------------------------------------------- //

void CAISensorDamage::SetFactTargetObject( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord )
{
	// Intentionally do NOT call super::SetFactTargetObject.
	// Target the stimulus target rather than the source.

	// Sanity check.

	if( !( pFact && pStimulusRecord ) )
	{
		return;
	}

	// Who shot me.

	pFact->SetTargetObject( pStimulusRecord->m_hStimulusTarget, 1.f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamage::FindUnstimulatedWorkingMemoryFact
//
//	PURPOSE:	Return a working memory fact that has not been 
//              stimulated this update.
//
// ----------------------------------------------------------------------- //

void CAISensorDamage::FindUnstimulatedWorkingMemoryFact(AIWORKING_MEMORY_FACT_LIST* pOutFactList)
{
	if (!pOutFactList)
	{
		return;
	}

	m_pAI->GetAIWorkingMemory()->CollectFactsUnupdated( kFact_Damage, pOutFactList, g_pLTServer->GetTime() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorDamage::DoComplexCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorDamage::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier )
{
	return true;
}

