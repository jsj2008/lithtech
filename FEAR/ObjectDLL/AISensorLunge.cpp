// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorLunge.cpp
//
// PURPOSE : AISensorLunge class implementation
//
// CREATED : 4/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorLunge.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "AIBlackBoard.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorLunge, kSensor_Lunge );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLunge::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorLunge::CAISensorLunge()
{
	m_fMaxLungeDist = 0.f;
	m_bCalculatedLungeDist = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLunge::Save
//
//	PURPOSE:	Save the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorLunge::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_FLOAT( m_fMaxLungeDist );
	SAVE_bool( m_bCalculatedLungeDist );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLunge::Load
//
//	PURPOSE:	Load the sensor
//
// ----------------------------------------------------------------------- //

void CAISensorLunge::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_FLOAT( m_fMaxLungeDist );
	LOAD_bool( m_bCalculatedLungeDist );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLunge::CalculateLungeDistance
//
//	PURPOSE:	Determine the max lunge distance, based on an animation.
//
// ----------------------------------------------------------------------- //

float CAISensorLunge::GetLungeDistance()
{
	if( m_bCalculatedLungeDist )
	{
		return m_fMaxLungeDist;
	}

	// Only calculate this once.

	m_bCalculatedLungeDist = true;

	// Find the lunge animation.

	CAnimationProps Props;
	Props.Set( kAPG_Posture, kAP_POS_Stand );
	Props.Set( kAPG_WeaponPosition, kAP_WPOS_Up );
	Props.Set( kAPG_Weapon, m_pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() );
	Props.Set( kAPG_Activity, kAP_ATVT_Lunging );
	Props.Set( kAPG_Action, kAP_ACT_AttackMelee );

	// Bail if no lunge animation.

	if( m_pAI->GetAnimationContext()->CountAnimations( Props ) == 0 )
	{
		return 0.f;
	}

	// Bail if the animation cannot be found.

	HMODELANIM hAni = m_pAI->GetAnimationContext()->GetAni( Props );
	if( hAni == INVALID_MODEL_ANIM )
	{
		return 0.f;
	}

	// Get the transform of the animation.

	LTRigidTransform tTransform;
	if ( !GetAnimationTransform( m_pAI, hAni, tTransform ) )
	{
		return 0.f;
	}

	// The z-translation on the node is the distance the lunge travels.

	m_fMaxLungeDist = tTransform.m_vPos.z;

	// The AI has a non-zero lunge distance, so give him the desire to lunge.
	// The desire records the max lunge distance, so that we don't have
	// to recalculate it in the lunge Action.

	if( m_fMaxLungeDist > 0.f )
	{
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
		pFact->SetDesireType( kDesire_Lunge, 1.f );
		pFact->SetRadius( m_fMaxLungeDist, 1.f );
	}

	return m_fMaxLungeDist;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorLunge::UpdateSensor
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//
// ----------------------------------------------------------------------- //

bool CAISensorLunge::UpdateSensor()
{
	if( !super::UpdateSensor() )
	{
		return false;
	}

	// No lunge animation.
	// Remove myself.

	if( !m_bCalculatedLungeDist )
	{
		GetLungeDistance();
		if( m_fMaxLungeDist == 0.f )
		{
			m_pAI->GetAISensorMgr()->RemoveAISensor( kSensor_Lunge );
			return false;
		}
	}

	// AI doesn't have a target.

	if (!m_pAI->HasTarget( kTarget_Character ))
	{
		return false;
	}

	// Generate a Touch fact if we collided with our target.

	if( ( m_pAI->GetAIBlackBoard()->GetBBHandleTouch() == kTouch_Damage ) && 
		( m_pAI->GetAIBlackBoard()->GetBBCollidedWithTarget() ) )
	{
		HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Knowledge );
		factQuery.SetKnowledgeType( kKnowledge_Touch );
		factQuery.SetSourceObject( hTarget );

		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if ( !pFact )
		{
			pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
			if ( pFact )
			{
				pFact->SetKnowledgeType( kKnowledge_Touch, 1.f );
				pFact->SetSourceObject( hTarget, 1.f );
			}
		}

		// Insure the time is always updated.

		if ( pFact )
		{
			pFact->SetTime( g_pLTServer->GetTime() );
		}
	}

	// Target is not visible.

	if( !m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() )
	{
		return false;
	}

	// Target must be in range.

	LTVector vTarget = m_pAI->GetAIBlackBoard()->GetBBTargetPosition();
	float fDistSqr = vTarget.DistSqr( m_pAI->GetPosition() );

	// Target too far.

	float fRange = GetLungeDistance() + 50.f;
	if( fDistSqr > fRange * fRange )
	{
		return false;
	}

	// Flag that goals should be re-evaluated if the AI 
	// is close enough to a target to lunge.

	m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );

	return true;
}


