// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorInstantDeath.cpp
//
// PURPOSE : AISensorInstantDeath class implementation
//
// CREATED : 6/10/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorInstantDeath.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorInstantDeath, kSensor_InstantDeath );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorInstantDeath::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorInstantDeath::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// This sensor is somewhat of a hack.  It was added because the AIActionInstantDeath
	// runs a couple frames too late to catch all situations where we would like AI to 
	// die instantly.  Ideally, we would refactor the way we handle damage to allow 
	// more immediate planner reaction.

	// This sensor is irrelevant once the AI has targeted something.

	if( m_pAI->GetAIBlackBoard()->GetBBTargetType() != kTarget_None )
	{
		m_pAI->GetAISensorMgr()->RemoveAISensor( kSensor_InstantDeath );
		return false;
	}

	// Sanity check.

	CDestructible *pDestructible = m_pAI->GetDestructible();
	if( !pDestructible )
	{
		return false;
	}

	// No new damage.

	if( pDestructible->GetLastDamageTime() == 0.f )
	{
		return false;
	}

	// This sensor is irrelevant after the first time we have been damaged.

	m_pAI->GetAISensorMgr()->RemoveAISensor( kSensor_InstantDeath );

	// Don't die instantly if NeverDestroy is set.

	if( pDestructible->GetNeverDestroy() )
	{
		return false;
	}

	// AI cannot take damage.

	if ( !m_pAI->GetDestructible()->GetCanDamage() )
	{
		return false;
	}

	// AI does not accept this type of damage.

	DamageStruct Damage = m_pAI->GetAIBlackBoard()->GetBBLastDamage();
	if( pDestructible->IsCantDamageType( Damage.eType ) )
	{
		return false;
	}

	// Die instantly if taken by surprise and hit in the head or torso.

	if( m_pAI->GetAIBlackBoard()->GetBBTargetType() == kTarget_None )
	{
		// Determine which body part was hit.

		ModelsDB::HNODE hModelNode = m_pAI->GetModelNodeLastHit();
		if( hModelNode )
		{
			EnumAnimProp eBodyAnipProp = g_pModelsDB->GetNodeBodyAnimProp( hModelNode );
			if( ( eBodyAnipProp == kAP_BODY_Head ) ||
				( eBodyAnipProp == kAP_BODY_Torso ) )
			{
				// Die instantly!  Use the data from the last damage received -- just 
				// lower the AI's hitpoints to less than 1.  This will insure this action behaves 
				// exactly like a standard death without any additional prompting.

				pDestructible->SetHitPoints( 0.1f );
				pDestructible->SetArmorPoints( 0.0f );
				Damage.fDamage = 2.0f;
				Damage.DoDamage( m_pAI->m_hObject, m_pAI->m_hObject );

				// Die silently.

				m_pAI->SetLastPainVolume( 0.1f );
				return true;
			}
		}
	}

	// Always allow other sensors to update.

	return false;
}


