// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRetreat.cpp
//
// PURPOSE : AIGoalRetreat implementation
//
// CREATED : 10/16/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalRetreat.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AIVolumeMgr.h"
#include "AIVolume.h"
#include "AIHuman.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRetreat, kGoal_Retreat);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalRetreat::CAIGoalRetreat()
{
	m_fRetreatSpeed = 200.f;

	m_fRetreatTriggerDistSqr = 128.f * 128.f;
	m_fRetreatJumpDist = 500.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreat::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT(m_fRetreatTriggerDistSqr);
	SAVE_FLOAT(m_fRetreatJumpDist);
	SAVE_FLOAT(m_fRetreatSpeed);
	SAVE_VECTOR(m_vRetreatDest);
}

void CAIGoalRetreat::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT(m_fRetreatTriggerDistSqr);
	LOAD_FLOAT(m_fRetreatJumpDist);
	LOAD_FLOAT(m_fRetreatSpeed);
	LOAD_VECTOR(m_vRetreatDest);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreat::InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime)
{
	super::InitGoal(pAI, fImportance, fTime);

	CAIBrain* pBrain = pAI->GetBrain();

	m_fRetreatSpeed = pBrain->GetAIData(kAIData_RetreatSpeed);

	m_fRetreatTriggerDistSqr = pBrain->GetAIData(kAIData_RetreatTriggerDist);
	m_fRetreatJumpDist = pBrain->GetAIData(kAIData_RetreatJumpDist);
	m_fRetreatTriggerDistSqr *= m_fRetreatTriggerDistSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreat::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hStimulusSource != LTNULL, m_pAI->m_hObject, "CAIGoalRetreat::ActivateGoal: StimulusSource is NULL");

	// Ignore senses other than see enemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal(this);

	m_pAI->Target(m_hStimulusSource);
	
	// Rotate away from destination.

	LTVector vDir = m_vRetreatDest - m_pAI->GetPosition();
	vDir.y = 0.f;
	vDir.Normalize();

	m_pAI->FaceDir( -vDir );
	m_pAI->FaceTargetRotImmediately();

	m_pAI->SetState( kState_HumanLongJump );
	CAIHumanStateLongJump* pLongJumpState = (CAIHumanStateLongJump*)m_pAI->GetState();
	pLongJumpState->SetLongJumpDest( m_vRetreatDest );
	pLongJumpState->SetLongJumpSequence( kAP_RetreatStart, kAP_RetreatFly, kAP_RetreatLand );
	pLongJumpState->SetLongJumpToTarget( LTFALSE );
	pLongJumpState->SetLongJumpSpeed( m_fRetreatSpeed );
	pLongJumpState->FaceDest( LTFALSE );

	m_pAI->PlaySound( kAIS_Retreat, LTFALSE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreat::DeactivateGoal()
{
	super::DeactivateGoal();

	// Ensure everything is reset.

	m_pAI->GetAnimationContext()->SetAnimRate( 1.f );
	m_fCurImportance = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreat::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanLongJump:
			HandleStateLongJump();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalRetreat::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::HandleStateLongJump
//
//	PURPOSE:	Determine what to do when in state Retreat.
//
// ----------------------------------------------------------------------- //

void CAIGoalRetreat::HandleStateLongJump()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_TakingOff:
			break;

		case kSStat_Moving:
			break;

		case kSStat_Landing:
			break;

		case kSStat_StateComplete:
			m_pAI->FaceTarget();
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalRetreat::HandleStateRetreat: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRetreat::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalRetreat::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Check if AI is already jumping.

		if( m_pAI->GetState()->GetStateType() == kState_HumanLongJump )
		{
			return FALSE;
		}

		// Only retreat from characters weilding melee weapons.

		if( IsCharacter( m_hStimulusSource ) )
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject( m_hStimulusSource );
			if( ( !pCharacter ) || ( !pCharacter->HasMeleeWeapon() ) )
			{
				m_fCurImportance = 0.f;
				return LTFALSE;
			}
		}

		LTVector vOrigin = m_pAI->GetPosition();

		// Check if the target is in range.

		LTFLOAT fDistSqr = vOrigin.DistSqr( pSenseRecord->vLastStimulusPos );
		if(fDistSqr > m_fRetreatTriggerDistSqr)
		{
			m_fCurImportance = 0.f;
			return LTFALSE;
		}

		// Check if AI and target are facing each other.

		LTVector vDir = vOrigin - pSenseRecord->vLastStimulusPos;
		vDir.y = 0.f;
		vDir.Normalize();

		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hStimulusSource, &rRot);
		if( ( m_pAI->GetTorsoForward().Dot( rRot.Forward() ) > -0.95f ) ||
			( m_pAI->GetTorsoForward().Dot( vDir ) > -0.95f ) )
		{
			m_fCurImportance = 0.f;
			return LTFALSE;
		}


		vDir = m_pAI->GetTorsoForward();
		vDir.y = 0.f;
		vDir.x *= -1.f;
		vDir.z *= -1.f;

		LTVector vDest = vOrigin + (vDir * m_fRetreatJumpDist);

		// Check if there is a straight line path (in volumes) to the target.

		if( !g_pAIVolumeMgr->StraightRadiusPathExists( m_pAI,
													vOrigin, 
													vDest, 
													m_pAI->GetRadius(), 
													m_pAI->GetVerticalThreshold(),
													AIVolume::kVolumeType_Ladder | AIVolume::kVolumeType_JumpOver | AIVolume::kVolumeType_JumpUp | AIVolume::kVolumeType_AmbientLife | AIVolume::kVolumeType_Teleport, 
													m_pAI->GetLastVolume() ) )
		{
			m_fCurImportance = 0.f;
			return LTFALSE;
		}

		// Ensure there are not AI in the way.

		if( g_pCharacterMgr->RayIntersectAI( vOrigin, vDest, m_pAI, LTNULL, LTNULL ) )
		{
			m_fCurImportance = 0.f;
			return LTFALSE;
		}

		m_pAI->DisableNodeTracking();
			
		m_vRetreatDest = vDest;
		return LTTRUE;
	}

	return LTFALSE;
}

