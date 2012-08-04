// ----------------------------------------------------------------------- //
//
// MODULE  : CAIWeaponMelee.cpp
//
// PURPOSE : CAIWeaponMelee class implementation
//
// CREATED : 10/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWeaponMelee.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "AnimationContext.h"
#include "AIUtils.h"
#include "AIBlackBoard.h"
#include "AIWorldState.h"
#include "AICoordinator.h"
#include "AISoundMgr.h"
#include "PlayerObj.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( WeaponClass, CAIWeaponMelee, kAIWeaponClassType_Melee );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::CAIWeaponMelee
//
//	PURPOSE:	Handles initializing the CAIWeaponMelee to intert values.
//
// ----------------------------------------------------------------------- //

CAIWeaponMelee::CAIWeaponMelee() : 
	m_fBurstInterval(DBL_MAX)
	, m_nBurstShots(0)
	, m_hLastUserAnimation( INVALID_ANI )
	, m_bForceHit( false )
{

}

void CAIWeaponMelee::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DOUBLE(m_fBurstInterval);
	SAVE_INT(m_nBurstShots);

	SAVE_INT(m_Context.m_cHits);
	SAVE_INT(m_Context.m_cMisses);
	SAVE_INT(m_Context.m_iHit);
	SAVE_INT(m_Context.m_iMiss);

	SAVE_bool(m_bForceHit);

	pMsg->WriteHMODELANIM( m_hLastUserAnimation );
}

void CAIWeaponMelee::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DOUBLE(m_fBurstInterval);
	LOAD_INT(m_nBurstShots);

	LOAD_INT(m_Context.m_cHits);
	LOAD_INT(m_Context.m_cMisses);
	LOAD_INT(m_Context.m_iHit);
	LOAD_INT(m_Context.m_iMiss);

	LOAD_bool(m_bForceHit);

	m_hLastUserAnimation = pMsg->ReadHMODELANIM();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::Init
//
//	PURPOSE:	Handles initializing the AIWeapon given a CWeapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMelee::Init(CWeapon* pWeapon, CAI* pAI)
{
	if (!DefaultInit(pWeapon, pAI))
	{
		return;
	}

	// Calculate an initial burst and clear the interval.  This prevents 
	// an AI from stalling immediately after picking up a weapon.

	CalculateBurst( pAI );
	m_fBurstInterval = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::CalculateBurst
//
//	PURPOSE:	Calculate all our burst parameters
//
// ----------------------------------------------------------------------- //

void CAIWeaponMelee::CalculateBurst( CAI* pAI )
{
	DefaultCalculateBurst( pAI, &m_fBurstInterval, &m_nBurstShots );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::Fire
//
//	PURPOSE:	Handles firing a the weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMelee::Fire(CAI* pAI)
{
	if( !m_pWeapon )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIWeaponMelee::Fire: No weapon!" );
		return;
	}

	if( m_eFiringState != kAIFiringState_Firing )
	{
		return;
	}

	// Fire!

	LTVector vTargetPos;
	bool bHit = GetShootPosition( pAI, m_Context, vTargetPos );
	if (DefaultFire(pAI, vTargetPos, m_pAIWeaponRecord->bAIAnimatesReload))
	{
		// Decrement burst count for this shot.
		m_nBurstShots--;
	}

	// Knock the player back, if the hit inflicted damage.

	if( bHit )
	{
		CPlayerObj* pPlayer = NULL;
		HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		if( IsPlayer( hTarget ) )
		{
			pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hTarget );
		}

		if( pPlayer &&
			( m_pAIWeaponRecord->fPlayerPusherRadius > 0.f ) &&
			( m_pAIWeaponRecord->fPlayerPusherForce > 0.f ) &&
			( pPlayer->GetDestructible() ) &&
			( pPlayer->GetDestructible()->GetLastDamageTime() == g_pLTServer->GetTime() ) &&
			( pPlayer->GetDestructible()->GetLastDamager() == pAI->m_hObject ) )
		{
			if( ( pPlayer->GetDestructible()->GetLastArmorAbsorb() > 0.f ) ||
				( pPlayer->GetDestructible()->GetLastDamage() > 0.f ) )
			{
				pPlayer->PushCharacter( pAI->GetPosition(), m_pAIWeaponRecord->fPlayerPusherRadius, 0.f, 0.3f, m_pAIWeaponRecord->fPlayerPusherForce );
			}
		}

		// Only play if we've been targeting someone for at least 5 seconds.
		// This ensures we first play higher priority reaction sounds.

		if( IsCharacter( hTarget ) )
		{
			CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
			if( pChar &&
				( pChar->GetDestructible() ) &&
				( pChar->GetDestructible()->GetLastDamageTime() == g_pLTServer->GetTime() ) &&
				( pChar->GetDestructible()->GetLastDamager() == pAI->m_hObject ) &&
				( pAI->GetAIBlackBoard()->GetBBTargetChangeTime() < g_pLTServer->GetTime() - 5.f ) )
			{
				HOBJECT hAlly = g_pAICoordinator->FindAlly( pAI->m_hObject, hTarget );
				if( hAlly )
				{
					g_pAISoundMgr->RequestAISound( hAlly, kAIS_HitSeenMelee, kAISndCat_Event, hTarget, 0.5f );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::GetShootPosition
//
//	PURPOSE:	Return false if the position returned is an intentional miss.
//
// ----------------------------------------------------------------------- //

bool CAIWeaponMelee::GetShootPosition( CAI* pAI, AimContext& Context, LTVector& outvShootPos )
{
	// Force AI to aim directly at the target.

	if( pAI && m_bForceHit )
	{
		HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		if( IsCharacter( hTarget ) )
		{
			g_pLTServer->GetObjectPos( hTarget, &outvShootPos );
			return true;
		}
	}

	// Default behavior.

	return super::GetShootPosition( pAI, Context, outvShootPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::GetFirePosition
//
//	PURPOSE:	Return the position to fire from.
//
// ----------------------------------------------------------------------- //

LTVector CAIWeaponMelee::GetFirePosition(CAI* pAI)
{
	// Force fire position to come from the edge of the target's radius.
	// This should ensure a successful melee hit.

	if( pAI && m_bForceHit )
	{
		HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		if( IsCharacter( hTarget ) )
		{
			LTVector vTargetPos;
			g_pLTServer->GetObjectPos( hTarget, &vTargetPos );

			LTVector vDir = vTargetPos - pAI->GetPosition();
			vDir.Normalize();

			CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
			LTVector vFirePos = vTargetPos - ( vDir * pChar->GetRadius() );

			return vFirePos;
		}
	}

	// Default behavior.

	return DefaultGetFirePosition(pAI);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::Update
//
//	PURPOSE:	Handle updating the weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMelee::Update(CAI* pAI)
{
	if (!pAI)
	{
		return;
	}

	// Calculate a new burst when out of shots.

	if( m_nBurstShots <= 0 )
	{
		CalculateBurst( pAI );
	}

	// Damage on touch, if set.

	if( pAI->GetAIBlackBoard()->GetBBHandleTouch() == kTouch_Damage )
	{
		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Knowledge );
		factQuery.SetKnowledgeType( kKnowledge_Touch );
		factQuery.SetSourceObject( pAI->GetAIBlackBoard()->GetBBTargetObject() );
		
		CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
		if( pFact )
		{
			m_eFiringState = kAIFiringState_Firing;
			m_bForceHit = true;
			Fire( pAI );
			m_bForceHit = false;

			// Fire only once, until touch is reset.

			pAI->GetAIWorkingMemory()->ClearWMFact( pFact );
			pAI->GetAIBlackBoard()->SetBBHandleTouch( kTouch_None );

			// Find an existing memory for the desire to retreat, or create one.

			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Desire);
			factQuery.SetDesireType(kDesire_Retreat);
			CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
			if( !pFact )
			{
				pFact = pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
				pFact->SetDesireType( kDesire_Retreat, 1.f );
			}
			return;
		}
	}

	// We are not firing.

	EnumAnimProp eProp = pAI->GetAnimationContext()->GetProp( kAPG_Action );
	if( eProp != kAP_ACT_AttackMelee )
	{
		m_eFiringState = kAIFiringState_None;

		if( m_pWeapon )
		{
			m_pWeapon->KillLoopSound();
		}

		// Reload weapon.

		if( eProp == kAP_ACT_Reload )
		{
			Reload(pAI);
		}
		return;
	}


	m_eFiringState = kAIFiringState_Firing;

	// If not already in a fire animation, switch to aiming if it aiming 
	// weapon between bursts.

	EnumAnimProp eCurrentProp = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Action );
	if ( eCurrentProp != kAP_ACT_AttackMelee )
	{
		if( m_fBurstInterval > g_pLTServer->GetTime() )
		{
			Aim(pAI);
		}

		// Don't fire if AI cannot hit its target.

		if( !( pAI->HasTarget( kTarget_Character | kTarget_Object ) && pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() ) )
		{
			m_fBurstInterval = 0.f;
			Aim(pAI);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::UpdateAnimation
//
//	PURPOSE:	Handle any animation updating that must occur after
//			the owners animations update.	
//
// ----------------------------------------------------------------------- //

void CAIWeaponMelee::UpdateAnimation( CAI* pAI )
{
	m_hLastUserAnimation = SyncWeaponAnimation( pAI, m_hLastUserAnimation );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::Aim
//
//	PURPOSE:	Handle Aiming the weapon
//
// ----------------------------------------------------------------------- //

void CAIWeaponMelee::Aim(CAI* pAI)
{
	m_eFiringState = kAIFiringState_Aiming;
	DefaultAim(pAI);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMelee::HandleModelString
//
//	PURPOSE:	Perform custom model string handling
//
// ----------------------------------------------------------------------- //

void CAIWeaponMelee::HandleModelString( CAI* pAI, const CParsedMsg& cParsedMsg )
{
	static CParsedMsg::CToken s_cTok_RigidMeleeAttack("MELEEATTACK");

	// Insure the rigidbody melee attacks decrement the burstshot counter.  
	// If this isn't done, an AI will never go into aim.

	if ( cParsedMsg.GetArg(0) == s_cTok_RigidMeleeAttack )
	{
		m_nBurstShots = LTMAX( 0, m_nBurstShots - 1 ); 
	}
	else
	{
		DefaultHandleModelString(pAI, cParsedMsg );
	}
}
