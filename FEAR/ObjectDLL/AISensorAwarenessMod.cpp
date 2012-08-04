// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorAwarenessMod.cpp
//
// PURPOSE : AISensorAwarenessMod class implementation
//
// CREATED : 08/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorAwarenessMod.h"
#include "AI.h"
#include "AIBrain.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemoryCentral.h"
#include "AIPathKnowledgeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorAwarenessMod, kSensor_AwarenessMod );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAwarenessMod::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorAwarenessMod::CAISensorAwarenessMod()
{
	m_bTestedLimp = false;
	m_bCanLimp = true;
	m_fFullHealth = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAwarenessMod::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorAwarenessMod::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_bool( m_bTestedLimp );
	SAVE_bool( m_bCanLimp );
	SAVE_FLOAT( m_fFullHealth );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAwarenessMod::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorAwarenessMod::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_bool( m_bTestedLimp );
	LOAD_bool( m_bCanLimp );
	LOAD_FLOAT( m_fFullHealth );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAwarenessMod::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorAwarenessMod::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// Only test awareness mods that are valid for this game, as
	// specified in the AI Constants record.

	AIDB_ConstantsRecord* pAIConstants = g_pAIDB->GetAIConstantsRecord();
	if( !pAIConstants )
	{
		return false;
	}

	EnumAIAwarenessMod eAwarenessMod = m_pAI->GetAIBlackBoard()->GetBBAwarenessMod();

	//
	// Injured.
	//

	if( pAIConstants->bitsValidAwarenessMods.test( kAwarenessMod_Injured ) )
	{
		// AI was already injured.

		if( eAwarenessMod == kAwarenessMod_Injured )
		{
			UpdateLimpTime();
			return false;
		}

		// AI is injured.

		if( ( eAwarenessMod != kAwarenessMod_Injured ) &&
			( IsAIInjured() ) )
		{
			m_pAI->GetAIBlackBoard()->SetBBAwarenessMod( kAwarenessMod_Injured );

			// Some NavMeshLinks are no longer passabel to a limping AI.

			m_pAI->GetPathKnowledgeMgr()->ClearPathKnowledge();
			return true;
		}
	}

	//
	// ImmediateThreat.
	//

	if( pAIConstants->bitsValidAwarenessMods.test( kAwarenessMod_ImmediateThreat ) )
	{
		// AI is alarmed enough to consider the threat immediate.

		if( ( eAwarenessMod != kAwarenessMod_ImmediateThreat ) &&
			( m_pAI->GetAlarmLevel() >= m_pAI->GetBrain()->GetImmediateAlarmThreshold() ) )
		{
			m_pAI->GetAIBlackBoard()->SetBBAwarenessMod( kAwarenessMod_ImmediateThreat );
			return false;
		}
	}

	// Allow other sesnros to update.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAwarenessMod::IsAIInjured
//
//	PURPOSE:	Return true if AI is injured.
//
// ----------------------------------------------------------------------- //

bool CAISensorAwarenessMod::IsAIInjured()
{
	// Limp animations are optional.

	if( !CanAILimp() )
	{
		return false;
	}

	// Bail if limp limits do not exist.

	AIDB_LimitsRecord* pLimpLimits = NULL;
	if ( m_pAI->GetAIAttributes() )
	{
		pLimpLimits = g_pAIDB->GetAILimitsRecord( m_pAI->GetAIAttributes()->eLimpLimitsID );
	}

	if( !pLimpLimits )
	{
		return false;
	}

	// Bail if no destructible.

	CDestructible* pDestructible = m_pAI->GetDestructible();
	if( !pDestructible )
	{
		return false;
	}

	// AI has never been damaged.  Record full health.

	if( pDestructible->GetLastDamageTime() == 0.f )
	{
		m_fFullHealth = pDestructible->GetHitPoints();
		return false;
	}

	// Only limp if damaged recently.

	double fCurTime = g_pLTServer->GetTime();
	if( pDestructible->GetLastDamageTime() < fCurTime - 1.f )
	{
		return false;
	}

	// AI has not hit his limp threshold.

	float fHealth = pDestructible->GetHitPoints();
	if( fHealth / m_fFullHealth > pLimpLimits->fThreshold )
	{
		return false;
	}

	// Someone else may be limping, or may have limped recently.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Limping );

	HOBJECT hLimper = NULL;
	double fLastLimpTime = 0.f;
	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
	if( pFact )
	{
		fLastLimpTime = pFact->GetTime();
		hLimper = pFact->GetTargetObject();
		if( hLimper && IsDeadAI( hLimper ) )
		{
			pFact->SetTargetObject( NULL );
			hLimper = NULL;
		}
	}

	// Only one AI may limp at a time.

	if( hLimper )
	{
		return false;
	}

	// Limit the frequency that any AI limps.

	if( ((float)(fCurTime - fLastLimpTime)) < pLimpLimits->fFrequency )
	{
		return false;
	}

	// Record limp time.
	// Probability may determine that AI does not actually limp, 
	// but no one else should try again until the time limit expires.

	UpdateLimpTime();

	// AI limps with some probability.

	float fRand = GetRandom( 0.f, 1.f );
	if( fRand > pLimpLimits->fProbability )
	{
		return false;
	}

	// Limp!

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAwarenessMod::CanAILimp
//
//	PURPOSE:	Return true if AI has limp animations.
//
// ----------------------------------------------------------------------- //

bool CAISensorAwarenessMod::CanAILimp()
{
	// Only test once.

	if( !m_bTestedLimp )
	{
		// No limits defined.

		if( m_pAI->GetAIAttributes() 
			&& m_pAI->GetAIAttributes()->eLimpLimitsID == kAILimitsID_Invalid )
		{
			m_bCanLimp = false;
		}

		// Search for limp anim.
	
		else
		{
			CAnimationProps Props;
			Props.Set( kAPG_Posture, kAP_POS_Stand );
			Props.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
			Props.Set( kAPG_Weapon, m_pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
			Props.Set( kAPG_Movement, kAP_MOV_Run );
			Props.Set( kAPG_Activity, kAP_ATVT_Limping );
	
			// No limp animations exist.

			if( m_pAI->GetAnimationContext()->CountAnimations( Props ) == 0 )
			{	
				m_bCanLimp = false;
			}
		}

		m_bTestedLimp = true;
	}

	return m_bCanLimp;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorAwarenessMod::UpdateLimpTime
//
//	PURPOSE:	Record the time that this AI last limped.
//
// ----------------------------------------------------------------------- //

void CAISensorAwarenessMod::UpdateLimpTime()
{
	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Limping );

	CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
	if( !pFact )
	{
		pFact = g_pAIWorkingMemoryCentral->CreateWMFact( kFact_Knowledge );
	}

	if( pFact )
	{
		pFact->SetKnowledgeType( kKnowledge_Limping, 1.f );
		pFact->SetTargetObject( m_pAI->m_hObject, 1.f );
		pFact->SetTime( g_pLTServer->GetTime(), 1.f );
	}
}
