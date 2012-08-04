// ----------------------------------------------------------------------- //
//
// MODULE  : CAIWeaponRanged.cpp
//
// PURPOSE : CAIWeaponRanged class implementation
//
// CREATED : 10/9/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWeaponRanged.h"
#include "AI.h"
#include "AIDB.h"
#include "AITarget.h"
#include "AISoundMgr.h"
#include "AICoordinator.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "AnimationContext.h"
#include "AIUtils.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIWorldState.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( WeaponClass, CAIWeaponRanged, kAIWeaponClassType_Ranged );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::CAIWeaponRanged
//
//	PURPOSE:	Handles initializing the CAIWeaponRanged to intert values.
//
// ----------------------------------------------------------------------- //

CAIWeaponRanged::CAIWeaponRanged() : 
	m_fBurstInterval(DBL_MAX)
	, m_nBurstShots(0)
	, m_hLastUserAnimation( INVALID_ANI )
{

}

void CAIWeaponRanged::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DOUBLE(m_fBurstInterval);
	SAVE_INT(m_nBurstShots);

	SAVE_INT(m_AimContext.m_cHits);
	SAVE_INT(m_AimContext.m_cMisses);
	SAVE_INT(m_AimContext.m_iHit);
	SAVE_INT(m_AimContext.m_iMiss);

	pMsg->WriteHMODELANIM( m_hLastUserAnimation );
}

void CAIWeaponRanged::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DOUBLE(m_fBurstInterval);
	LOAD_INT(m_nBurstShots);

	LOAD_INT(m_AimContext.m_cHits);
	LOAD_INT(m_AimContext.m_cMisses);
	LOAD_INT(m_AimContext.m_iHit);
	LOAD_INT(m_AimContext.m_iMiss);

	m_hLastUserAnimation = pMsg->ReadHMODELANIM( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::Init
//
//	PURPOSE:	Handles initializing the AIWeapon given a CWeapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::Init(CWeapon* pWeapon, CAI* pAI)
{
	if (!DefaultInit(pWeapon, pAI))
	{
		return;
	}

	CalculateBurst(pAI);
	m_fBurstInterval = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::Deselect
//
//	PURPOSE:	Deselects the weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::Deselect(CAI* pAI)
{
	if( !pAI )
	{
		return;
	}

	m_eFiringState = kAIFiringState_None;
	UpdateWeaponAnimation(pAI);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::Fire
//
//	PURPOSE:	Handles firing a the weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::Fire(CAI* pAI)
{
	if( !m_pWeapon )
	{
		AIASSERT( 0, pAI->GetHOBJECT(), "CAIWeaponRanged::Fire: No weapon!" );
		return;
	}

	if( ( m_eFiringState != kAIFiringState_Firing ) &&
		( m_eFiringState != kAIFiringState_CineFiring ) )
	{
		return;
	}

	LTVector vTargetPos;
	bool bHit = GetShootPosition( pAI, m_AimContext, vTargetPos );

	bool bAnimateReloads = ( m_eFiringState == kAIFiringState_CineFiring ) ? false : m_pAIWeaponRecord->bAIAnimatesReload;

	if( DefaultFire( pAI, vTargetPos, bAnimateReloads ) )
	{
		// Increment the count of shots fired at this target.

		uint32 cShots = pAI->GetAIBlackBoard()->GetBBShotsFiredAtTarget();
		pAI->GetAIBlackBoard()->SetBBShotsFiredAtTarget( cShots + 1 );

		// Decrement burst count for this shot.
		m_nBurstShots--;
	}

	// Say something if we hit, or missed when we should have hit.

	if( bHit )
	{
		CCharacter* pChar = NULL;
		HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();
		if( IsCharacter( hTarget ) )
		{
			pChar = (CCharacter*)g_pLTServer->HandleToObject( hTarget );
		}

		// Hit!
		// Only play if we've been targeting someone for at least 5 seconds.
		// This ensures we first play higher priority reaction sounds.

		if( pChar &&
			( pChar->GetDestructible() ) &&
			( pChar->GetDestructible()->GetLastDamageTime() == g_pLTServer->GetTime() ) &&
			( pChar->GetDestructible()->GetLastDamager() == pAI->m_hObject ) &&
			( pAI->GetAIBlackBoard()->GetBBTargetChangeTime() < g_pLTServer->GetTime() - 5.f ) )
		{
			HOBJECT hAlly = g_pAICoordinator->FindAlly( pAI->m_hObject, hTarget );
			if( hAlly )
			{
				g_pAISoundMgr->RequestAISound( hAlly, kAIS_HitSeen, kAISndCat_Event, hTarget, 0.5f );
			}
			else 
			{
				g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_Hit, kAISndCat_Event, hTarget, 0.5f );
			}
		}

		// Miss!

		else {

			// Ally berates AI for missing.

			ENUM_AI_SQUAD_ID eSquadID = g_pAICoordinator->GetSquadID( pAI->m_hObject );
			CAISquad* pSquad = g_pAICoordinator->FindSquad( eSquadID );
			if( pSquad && pSquad->GetNumSquadMembers() > 1 )
			{
				// Select an ally to speak.

				LTObjRef* pMembers = pSquad->GetSquadMembers();
				if( pMembers )
				{
					int cMembers = pSquad->GetNumSquadMembers();
					for( int iMember = 0; iMember < cMembers; ++iMember )
					{
						if( pMembers[iMember] != pAI->m_hObject )
						{
							//g_pAISoundMgr->RequestAISound( pMembers[iMember], kAIS_Miss, kAISndCat_Event, hTarget, 0.5f );
							break;
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::IsAIWeaponReadyToFire
//
//	PURPOSE:	Return true if weapon is ready to fire.
//
// ----------------------------------------------------------------------- //

bool CAIWeaponRanged::IsAIWeaponReadyToFire()
{
	// Weapon is not ready to fire between bursts.

	if( m_fBurstInterval > g_pLTServer->GetTime() )
	{
		return false;
	}
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::CalculateBurst
//
//	PURPOSE:	Calculate all our burst parameters
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::CalculateBurst( CAI* pAI )
{
	DefaultCalculateBurst( pAI, &m_fBurstInterval, &m_nBurstShots );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::ClearBurstInterval
//
//	PURPOSE:	Clears the burst interval, so AI can fire immediately.
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::ClearBurstInterval()
{
	// Some weapons do not allow programmatic clearing of 
	// the burst interval (e.g. missile launcher).

	if( !m_pAIWeaponRecord->bStrictBursts )
	{
		m_fBurstInterval = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::Update
//
//	PURPOSE:	Handle updating the weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::Update(CAI* pAI)
{
	if (!pAI)
	{
		return;
	}

	// Calculate a new burst when out of shots.

	if( m_nBurstShots <= 0 )
	{
		CalculateBurst(pAI);
	}

	EnumAnimProp eProp = pAI->GetAnimationContext()->GetProp( kAPG_Action );

	// We are not firing.

	if( eProp != kAP_ACT_Fire && 
		eProp != kAP_ACT_FireNode )
	{
		m_eFiringState = kAIFiringState_None;

		m_iAnimRandomSeed = (uint32)-1;
		pAI->GetAnimationContext()->SetRandomSeed( m_iAnimRandomSeed );

		ClearBurstInterval();

		// Reload weapon.
		// Ensure that the reload animation has really started
		// before adding more ammo to the gun. Otherwise, if AI is 
		// interrupted (e.g. damaged) before strating the transition,
		// the AI will immediately transition out before reloading.

		if( ( eProp == kAP_ACT_Reload ) &&
			( pAI->GetAnimationContext()->GetCurrentProp( kAPG_Action ) == kAP_ACT_Reload ) &&
			( !pAI->GetAnimationContext()->IsTransitioning() ) )
		{
			Reload(pAI);
		}
		else {
			UpdateWeaponAnimation(pAI);
		}
		return;
	}

	m_eFiringState = kAIFiringState_Firing;

	// If we are firing at from a node, don't constrain the aiming.
	// A node may play a 'get up on desk, fire off 3 shots, and climb down'
	// animation.  Burst intervals, the direction the gun is pointing before 
	// the AI climbs up on a desk, etc don't matter.  We don't want to kick an 
	// AI into aim and then lock it when a specific animation is suppose to be 
	// played.

	if ( eProp != kAP_ACT_FireNode )
	{
		// Do not fire bursts while firing suppression.

		bool bFireBursts = true;
		if( pAI->GetAIBlackBoard()->GetBBSuppressionFire() )
		{
			bFireBursts = m_pAIWeaponRecord->bSuppressionBursts;
		}

		// Aiming weapon between bursts.

		double fCurTime = g_pLTServer->GetTime();
		if( bFireBursts && 
			( m_fBurstInterval > fCurTime ) )
		{
			Aim(pAI);
		}

		// Pause before firing.

		if( pAI->GetAIBlackBoard()->GetBBFirePauseTimeLimit() > fCurTime )
		{
			Aim(pAI);
		}

		// Don't check visibility if we're blind firing or suppression firing.

		bool bCheckVisibility = true;
		if( ( pAI->GetAIBlackBoard()->GetBBBlindFire() ) ||
			( pAI->GetAIBlackBoard()->GetBBSuppressionFire() ) )
		{
			bCheckVisibility = false;
		}

		// Don't fire if AI cannot hit its target.

		if( bCheckVisibility &&
			( ( !pAI->HasTarget( kTarget_Character | kTarget_CombatOpportunity | kTarget_Object ) ) ||
			( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() ) ||
			( pAI->GetTarget()->GetVisionBlocker() ) ) )
		{
			ClearBurstInterval();
			Aim(pAI);
		}

		// Don't fire at crazy angles off of the gun.
		// Restrict AI to only fire at targets within 60 degrees of the barrel.

		if( ( m_eFiringState == kAIFiringState_Firing ) &&
			( m_pAIWeaponRecord->bRestrictFireCone ) )
		{
			LTVector vWeaponForward;
			LTVector vWeaponPosition;

			// If we have a rotational offset specified, then we need to get 
			// the rotation and perform an additional matrix multiply.  As Fear 
			// does not want this overhead, this code pathway should only be run
			// if a rotation is set.

			if ( LTRotation::GetIdentity() == m_pAIWeaponRecord->mFireRotationOffset )
			{
				vWeaponForward = pAI->GetWeaponForward( m_pWeapon );
				vWeaponPosition = pAI->GetPosition();
			}
			else
			{
				LTRotation mWeaponSocketRotation;
				pAI->GetAIWeaponMgr()->GetWeaponOrientation( m_pWeapon, vWeaponPosition, mWeaponSocketRotation );
				LTRotation rGunRotation = mWeaponSocketRotation * m_pAIWeaponRecord->mFireRotationOffset;
				vWeaponForward = rGunRotation.Forward();
			}

			// Get the direction from the gun to the target

			LTVector vShootDir = pAI->GetAIBlackBoard()->GetBBTargetPosition() - vWeaponPosition;
			vShootDir.Normalize();
			float fDot = vWeaponForward.Dot( vShootDir );
			if( fDot < c_fFOV60 )
			{
				Aim(pAI);
			}
		}
	}

	if( m_eFiringState != kAIFiringState_None )
	{
		RandomizeFireAnimation( pAI );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::UpdateAnimation
//
//	PURPOSE:	Handle any animation updating that must occur after
//			the owners animations update.	
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::UpdateAnimation( CAI* pAI )
{
	m_hLastUserAnimation = SyncWeaponAnimation( pAI, m_hLastUserAnimation );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::Aim
//
//	PURPOSE:	Handle Aiming the weapon
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::Aim(CAI* pAI)
{
	m_eFiringState = kAIFiringState_Aiming;
	DefaultAim(pAI);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponRanged::RandomizeFireAnimation
//
//	PURPOSE:	Select random seed for set of fire and aim animations.
//
// ----------------------------------------------------------------------- //

void CAIWeaponRanged::RandomizeFireAnimation( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Do not randomize if ranged weapon is not the primary weapon.

	if( pAI->GetAIBlackBoard()->GetBBPrimaryWeaponProp() != m_pAIWeaponRecord->eAIWeaponAnimProp )
	{
		return;
	}

	// Select a random seed.

	if( ( m_fRandomSeedSelectionTime < pAI->GetAIBlackBoard()->GetBBStateChangeTime() ) ||
		( m_fRandomSeedSelectionTime < pAI->GetAIBlackBoard()->GetBBMovementDirChangeTime() ) )
	{
		CAnimationProps Props = pAI->GetAnimationContext()->GetProps();
		Props.Set( kAPG_Weapon, m_pAIWeaponRecord->eAIWeaponAnimProp );
		m_iAnimRandomSeed = pAI->GetAnimationContext()->ChooseRandomSeed( Props );
		m_fRandomSeedSelectionTime = g_pLTServer->GetTime();
	}

	// Set the random seed.

	pAI->GetAnimationContext()->SetRandomSeed( m_iAnimRandomSeed );
}
