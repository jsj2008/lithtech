// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTargetIsAimingAtMeParanoid.cpp
//
// PURPOSE : Paranoid derived version of sensor. 
//           AI always thinks someone is aiming at him when he's moving.
//
// CREATED : 4/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorTargetIsAimingAtMeParanoid.h"
#include "AI.h"
#include "AITarget.h"
#include "AIWorldState.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTargetIsAimingAtMeParanoid, kSensor_TargetIsAimingAtMeParanoid );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTargetIsAimingAtMeParanoid::UpdateSensor()
//
//	PURPOSE:	Returns true if the target characters current weapon 
//				is dangerous, else returns false.  
//
//				TODO: We may want to look at all active weapons instead of
//				just the current.
//
// ----------------------------------------------------------------------- //

bool CAISensorTargetIsAimingAtMeParanoid::UpdateSensor()
{
	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		// Always assume someone is aiming at me while moving.

		if( !m_pAI->GetAIMovement()->IsUnset() )
		{
			// Damage fact indicates current stimulation due to damage.

			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Damage);
			CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

			// We're sustaining damage, so forget about getting aimed at.
			// This will result in the AI NOT doing evasive maneuvers, and simply running away at top speed!

			if( pFact 
				&& DidDamage( m_pAI, pFact )
				&& pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) > 0.2f )
			{
				m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsLookingAtMe, m_pAI->m_hObject, kWST_bool, false );
				m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject, kWST_bool, false );
			}

			// Assume someone is aiming at me while moving.
			// This will result in the AI doing lots of evasive maneuvers.

			else {
				m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsLookingAtMe, m_pAI->m_hObject, kWST_bool, true );
				m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject, kWST_bool, true );
			}


			return true;
		}
	}

	// Default behavior.

	return super::UpdateSensor();
};

