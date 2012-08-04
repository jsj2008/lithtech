// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTargetIsAimingAtMe.cpp
//
// PURPOSE : Contains the implementation for the sensor which detects 
//				if the target is aiming at the owner.
//
// CREATED : 4/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorTargetIsAimingAtMe.h"
#include "AI.h"
#include "AIWorldState.h"
#include "AIBlackBoard.h"
#include "AITarget.h"
#include "AIDB.h"
#include "AIWorkingMemory.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTargetIsAimingAtMe, kSensor_TargetIsAimingAtMe );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTargetIsAimingAtMe::UpdateSensor()
//
//	PURPOSE:	Return true if this sensor updated, and the SensorMgr
//              should wait to update others.
//				Returns true if the target characters current weapon 
//				is dangerous, else returns false.  
//
//				TODO: We may want to look at all active weapons instead of
//				just the current.
//
// ----------------------------------------------------------------------- //

bool CAISensorTargetIsAimingAtMe::UpdateSensor()
{
	// Bail if not updating aim.

	if( !m_pAI->GetAIBlackBoard()->GetBBUpdateTargetAim() )
	{
		return false;
	}

	// Determine if Target is looking and aiming at me.

	if( TargetIsLookingAtMe() )
	{
		m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsLookingAtMe, m_pAI->m_hObject, kWST_bool, true );

		if( TargetHasDangerousWeapon() )
		{
			m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject, kWST_bool, true );
		}
		else {
			m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject, kWST_bool, false );
		}
	}
	else {
		m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsLookingAtMe, m_pAI->m_hObject, kWST_bool, false );
		m_pAI->GetAIWorldState()->SetWSProp( kWSK_TargetIsAimingAtMe, m_pAI->m_hObject, kWST_bool, false );
	}

	return true;
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTargetIsAimingAtMe::TargetHasDangerousWeapon()
//
//	PURPOSE:	Returns true if the target characters current weapon 
//				is dangerous, else returns false.  
//
//				TODO: We may want to look at all active weapons instead of
//				just the current.
//
// ----------------------------------------------------------------------- //

bool CAISensorTargetIsAimingAtMe::TargetHasDangerousWeapon()
{
	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();
		AIASSERT(IsCharacter(hTarget), m_pAI->GetHOBJECT(), "CAISensorTargetIsAimingAtMe::TargetHasDangerousWeapon : Target is not a character.");
		CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hTarget);
		if(!pCharacter)
		{
			return false;
		}

		HWEAPON hWeapon = pCharacter->GetArsenal()->GetCurWeaponRecord();
		const AIDB_AIWeaponRecord* pAIWeapon = AIWeaponUtils::GetAIWeaponRecord(
			hWeapon, 
			m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
		if (!pAIWeapon)
		{
			return false;
		}

		return pAIWeapon->bIsDangerous;
	}

	// Target does not have a dangerous weapon.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAITarget::SetTarget()
//
//	PURPOSE:	Returns true if the AI has a target and if the target is
//				looking at the AI.
//
// ----------------------------------------------------------------------- //

bool CAISensorTargetIsAimingAtMe::TargetIsLookingAtMe()
{
	// No character target.

	if( !m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		return false;
	}

	// No character has not been seen recently.

	if( !m_pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
	{
		return false;
	}

	// Target is not completely visible.

	HOBJECT hTarget = m_pAI->GetAIBlackBoard()->GetBBTargetObject();

	CAIWMFact factQuery;
	factQuery.SetFactType(kFact_Character);
	factQuery.SetTargetObject(hTarget);
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if( !( pFact && ( pFact->GetConfidence( CAIWMFact::kFactMask_Stimulus ) >= 1.f ) ) )
	{
		return false;
	}

	// Target does not exist.

	CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hTarget);
	if( !pCharacter )
	{
		return false;
	}

	LTRigidTransform tfView;
	pCharacter->GetViewTransform( tfView );

	LTVector vForward = tfView.m_rRot.Forward();
	vForward.y = 0.f;
	if( vForward != LTVector::GetIdentity() )
	{
		vForward.Normalize();
	}

	LTVector vPos;
	g_pLTServer->GetObjectPos( pCharacter->m_hObject, &vPos);

	LTVector vDir;
	vDir = m_pAI->GetPosition() - vPos;
	vDir.y = 0.f;
	if( vDir != LTVector::GetIdentity() )
	{
		vDir.Normalize();
	}

	// TODO: bute this

	const static float fThreshhold = 0.98f;
	if ( vDir.Dot(vForward) > fThreshhold )
	{
		return true;
	}

	return false;
}
