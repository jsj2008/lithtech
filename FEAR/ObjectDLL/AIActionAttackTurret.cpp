// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackTurret.cpp
//
// PURPOSE : AIActionAttackTurret class implementation
//
// CREATED : 6/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionAttackTurret.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AISoundMgr.h"
#include "ServerSoundMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackTurret, kAct_AttackTurret );

#define TURNING_SOUND	"TurningSound"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackTurret::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionAttackTurret::CAIActionAttackTurret()
{
	m_dwTrackerFlags = kTrackerFlag_AimAt | kTrackerFlag_Arm;
	m_bFaceTarget = false;
	m_bValidateVisibility = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackTurret::ValidateContextPreconditions
//
//	PURPOSE:	Return true if real-time preconditions are valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackTurret::ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Do not attack if senses are turned off.

	if( !pAI->GetAIBlackBoard()->GetBBSensesOn() )
	{
		return false;
	}

	// Default validation.

	return super::ValidateContextPreconditions( pAI, wsWorldStateGoal, bIsPlanning );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackTurret::ActivateAction
//
//	PURPOSE:	Activate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackTurret::ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal )
{
	super::ActivateAction( pAI, wsWorldStateGoal );

	// Turrets auto-reload.

	pAI->GetAIBlackBoard()->SetBBAutoReload( true );

	// BlindFire when target is not visible or goes outside of narrow FOV.

	bool bBlindFire = true;
	bool bTargetInFOV = TargetInFOV( pAI );
	if( pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() &&
		bTargetInFOV )
	{
		bBlindFire = false;
	}
	pAI->GetAIBlackBoard()->SetBBBlindFire( bBlindFire );

	// Loop or kill the turning sound.

	PlayTurningSound( pAI, bBlindFire && !bTargetInFOV );

	// Play a target spotted sound if previously scanning.

	EnumAnimProp eAction = pAI->GetAnimationContext()->GetCurrentProp( kAPG_Action );
	if( eAction == kAP_ACT_Alert )
	{
		g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_DisturbanceSeenAlarming, kAISndCat_Always, NULL, 0.f );
	}

	// Torso tracking.

	pAI->GetAIBlackBoard()->SetBBTargetTrackerFlags( m_dwTrackerFlags );
	pAI->GetAIBlackBoard()->SetBBFaceTarget( m_bFaceTarget );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionSuppressionFire::DeactivateAction
//
//	PURPOSE:	Deactivate action.
//
// ----------------------------------------------------------------------- //

void CAIActionAttackTurret::DeactivateAction( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Kill the looping turning sound.

	PlayTurningSound( pAI, false );

	// Timeout expired.
	// Clear target and stop firing.

	if( TimeoutExpired( pAI ) )
	{
		HOBJECT hTarget = pAI->GetAIBlackBoard()->GetBBTargetObject();

		CAIWMFact factQuery;
		factQuery.SetFactType( kFact_Character );
		factQuery.SetTargetObject( hTarget );
		pAI->GetAIWorkingMemory()->ClearWMFact( factQuery );

		pAI->ClearState();

		g_pAISoundMgr->RequestAISound( pAI->m_hObject, kAIS_SearchFailed, kAISndCat_Always, NULL, 0.f );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackTurret::ValidateAction
//
//	PURPOSE:	Return true if action is still valid.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackTurret::ValidateAction( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// Do not attack if senses are turned off.

	if( !pAI->GetAIBlackBoard()->GetBBSensesOn() )
	{
		return false;
	}

	// BlindFire when target goes outside of narrow FOV.

	bool bBlindFire = true;
	if( pAI->GetAIBlackBoard()->GetBBTargetVisibleFromWeapon() &&
		TargetInFOV( pAI ) )
	{
		bBlindFire = false;
	}
	if( bBlindFire != pAI->GetAIBlackBoard()->GetBBBlindFire() )
	{
		return false;
	}

	// Timeout expired.

	if( TimeoutExpired( pAI ) )
	{
		return false;
	}

	// Default validation.

	return super::ValidateAction( pAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackTurret::TargetInFOV
//
//	PURPOSE:	Return true if target is in turret's FOV.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackTurret::TargetInFOV( CAI* pAI )
{
	// Sanity check.

	if( !pAI )
	{
		return false;
	}

	// No ranged weapon!

	CWeapon* pWeapon = pAI->GetAIWeaponMgr()->GetWeaponOfType( kAIWeaponType_Ranged );
	if( !pWeapon )
	{
		return false;
	}

	// Compare the weapon's forward to the target direction.

	LTVector vWeaponForward = pAI->GetWeaponForward( pWeapon );
	LTVector vDirToTarget = pAI->GetAIBlackBoard()->GetBBTargetPosition() - pAI->GetPosition();
	vDirToTarget.Normalize();

	// Target is outside of the weapon's FOV.

	float fFOV20 = cos( DEG2RAD( 10.f ) );
	if( vDirToTarget.Dot( vWeaponForward ) <= fFOV20 )
	{
		return false;
	}

	// Target is inside of the weapon's FOV.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackTurret::TimeoutExpired
//
//	PURPOSE:	Return true if turret's timeout expired.
//
// ----------------------------------------------------------------------- //

bool CAIActionAttackTurret::TimeoutExpired( CAI* pAI )
{
	// SmartObject specifies the timeout for the turret.

	AIDB_SmartObjectRecord* pSmartObjectRecord = g_pAIDB->GetAISmartObjectRecord( m_pActionRecord->eSmartObjectID );
	if( !pSmartObjectRecord )
	{
		return false;
	}

	// Give up after some expiration time.

	if( pSmartObjectRecord->fTimeout != 0.f )
	{
		double fTargetChangeTime = pAI->GetAIBlackBoard()->GetBBTargetChangeTime();
		double fTargetLastVisibleTime = pAI->GetAIBlackBoard()->GetBBTargetLastVisibleTime();

		double fTargetTime = LTMAX( fTargetChangeTime, fTargetLastVisibleTime );
		if( g_pLTServer->GetTime() - fTargetTime > pSmartObjectRecord->fTimeout )
		{
			return true;
		}
	}

	// Timeout has not expired.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionAttackTurret::PlayTurningSound
//
//	PURPOSE:	Toggle playing of looped tunring sound..
//
// ----------------------------------------------------------------------- //

void CAIActionAttackTurret::PlayTurningSound( CAI* pAI, bool bPlaySound )
{
	// Sanity check.

	if( !pAI )
	{
		return;
	}

	// Kill looping sound.

	HLTSOUND hLoopSound;
	if( !bPlaySound )
	{
		hLoopSound = pAI->GetAIBlackBoard()->GetBBLoopingSound();
		if( hLoopSound )
		{
			g_pLTServer->SoundMgr()->KillSound( hLoopSound );
			pAI->GetAIBlackBoard()->SetBBLoopingSound( NULL );
		}
		return;
	}

	// Play looping sound.

	HRECORD hRecord = g_pAIDB->GetMiscRecordLink( TURNING_SOUND );
	const char* pszSoundFile = g_pLTDatabase->GetRecordName( hRecord );
	HRECORD hSR = g_pSoundDB->GetSoundDBRecord( pszSoundFile );

	hLoopSound = g_pServerSoundMgr->PlayDBSoundFromObject( pAI->m_hObject, hSR,
		-1.0f, SOUNDPRIORITY_MISC_LOW, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE,
		SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
		DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);

	pAI->GetAIBlackBoard()->SetBBLoopingSound( hLoopSound );
}
