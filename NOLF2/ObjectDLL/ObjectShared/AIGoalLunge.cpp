// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalLunge.cpp
//
// PURPOSE : AIGoalLunge implementation
//
// CREATED : 10/10/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalLunge.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AISenseRecorderAbstract.h"
#include "AIVolumeMgr.h"
#include "AIVolume.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "AIUtils.h"
#include "AICentralKnowledgeMgr.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalLunge, kGoal_Lunge);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalLunge::CAIGoalLunge()
{
	m_fLungeSpeed = 600.f;

	m_bLunged = LTFALSE;

	m_fLungeDistSqrMin = 300.f * 300.f;
	m_fLungeDistSqrMax = 500.f * 500.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalLunge::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT(m_fLungeDistSqrMin);
	SAVE_FLOAT(m_fLungeDistSqrMax);
	SAVE_FLOAT(m_fLungeSpeed);
	SAVE_VECTOR(m_vLungeDest);
	SAVE_VECTOR(m_vOrigPos);
	SAVE_BOOL(m_bLunged);
}

void CAIGoalLunge::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT(m_fLungeDistSqrMin);
	LOAD_FLOAT(m_fLungeDistSqrMax);
	LOAD_FLOAT(m_fLungeSpeed);
	LOAD_VECTOR(m_vLungeDest);
	LOAD_VECTOR(m_vOrigPos);
	LOAD_BOOL(m_bLunged);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalLunge::InitGoal(CAI* pAI, LTFLOAT fImportance, LTFLOAT fTime)
{
	super::InitGoal(pAI, fImportance, fTime);

	CAIBrain* pBrain = pAI->GetBrain();

	m_fLungeSpeed = pBrain->GetAIData(kAIData_LungeSpeed);

	m_fLungeDistSqrMin = pBrain->GetAIData(kAIData_LungeDistMin);
	m_fLungeDistSqrMax = pBrain->GetAIData(kAIData_LungeDistMax);
	m_fLungeDistSqrMin *= m_fLungeDistSqrMin;
	m_fLungeDistSqrMax *= m_fLungeDistSqrMax;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalLunge::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hStimulusSource != LTNULL, m_pAI->m_hObject, "CAIGoalLunge::ActivateGoal: StimulusSource is NULL");

	// Only one AI at a time may lunge.

	if( 0 != g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_AttackLunge, m_pAI, g_pLTServer->HandleToObject(m_pAI->GetTarget()->GetObject()) ) )
	{
		m_fCurImportance = 0.f;
		return;
	}

	// Ignore senses other than see enemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal(this);

	m_pAI->Target(m_hStimulusSource);
	
	m_vOrigPos = m_pAI->GetPosition();
	m_bLunged = LTFALSE;

	m_pAI->SetState( kState_HumanLongJump );
	CAIHumanStateLongJump* pLongJumpState = (CAIHumanStateLongJump*)m_pAI->GetState();
	pLongJumpState->SetLongJumpDest( m_vLungeDest );
	pLongJumpState->SetLongJumpSequence( kAP_LungeStart, kAP_LungeFly, kAP_LungeLand );
	pLongJumpState->SetLongJumpToTarget( LTFALSE );
	pLongJumpState->SetLongJumpSpeed( m_fLungeSpeed );
	pLongJumpState->CountAttackers( LTTRUE );

	// Snap to face direction of the jump.

	LTVector vFace = m_vLungeDest - m_vOrigPos;
	vFace.y = 0.f;
	vFace.Normalize();

	m_pAI->FaceDir( vFace );
	m_pAI->FaceTargetRotImmediately();

	m_pAI->PlaySound( kAIS_Lunge, LTFALSE );

	// Count lungers.

	g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_AttackLunge, m_pAI, g_pLTServer->HandleToObject(m_pAI->GetTarget()->GetObject()), LTTRUE );

	// If we have a distance in front of the target we want to stop at, then set it
	// otherwise report that a default value is being used (instead of asserting or 
	// unexpected default use)
	if ( m_pAI->GetBrain()->GetAIDataExist(kAIData_LungeStopDistance) == LTTRUE )
	{
		float flStopDist = m_pAI->GetBrain()->GetAIData(kAIData_LungeStopDistance);
		pLongJumpState->SetStopDistance( flStopDist );
	}
	else
	{
		AIASSERT2("%s", m_pAI->GetHOBJECT(), "No LungeStopDistance AIDATA set for %s in AIBrain, using default value: %.2f.\n", m_pAI->GetName(), pLongJumpState->GetStopDistance() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalLunge::DeactivateGoal()
{
	super::DeactivateGoal();

	// Ensure everything is reset.

	m_pAI->GetAnimationContext()->SetAnimRate( 1.f );
	m_fCurImportance = 0.f;

	// Decrement counters.

	g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_AttackLunge, m_pAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::Retreat
//
//	PURPOSE:	Retreat from the landing spot.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalLunge::Retreat()
{
	// Get retreat values.

	CAIBrain* pBrain = m_pAI->GetBrain();
	if( ( !pBrain->GetAIDataExist( kAIData_RetreatSpeed ) ) || ( pBrain->GetAIDataExist( kAIData_RetreatJumpDist ) ) )
	{
		return LTFALSE;
	}

	LTFLOAT fRetreatSpeed = pBrain->GetAIData(kAIData_RetreatSpeed);
	LTFLOAT fRetreatJumpDist = pBrain->GetAIData(kAIData_RetreatJumpDist);

	// Ensure there is a target to retreat from.

	if( !m_pAI->HasTarget() )
	{
		return LTFALSE;
	}

	// Only retreat if target is weilding a melee weapon.

	CAITarget* pTarget = m_pAI->GetTarget();
	if( ( !pTarget->GetCharacter() ) || ( !pTarget->GetCharacter()->HasMeleeWeapon() ) )
	{
		return LTFALSE;
	}

	// Check if the target is in front of AI.

	LTVector vPos = m_pAI->GetPosition();
	LTVector vTargetDir = pTarget->GetVisiblePosition() - vPos;
	if( m_pAI->GetTorsoForward().Dot( vTargetDir ) < 0.f ) 
	{
		return LTFALSE;
	}

	// Calculate destination.

	LTVector vDir = m_vOrigPos - vPos;
	vDir.y = 0.f;
	vDir.Normalize();

	m_pAI->FaceDir( -vDir );
	m_pAI->FaceTargetRotImmediately();

	LTVector vDest = vPos + (vDir * fRetreatJumpDist);

	// Ensure there is a straight path to dest.

	if( !g_pAIVolumeMgr->StraightRadiusPathExists( m_pAI,
												vPos, 
												vDest, 
												m_pAI->GetRadius(), 
												m_pAI->GetVerticalThreshold(),
												AIVolume::kVolumeType_Ladder | AIVolume::kVolumeType_JumpOver | AIVolume::kVolumeType_JumpUp | AIVolume::kVolumeType_AmbientLife | AIVolume::kVolumeType_Teleport, 
												m_pAI->GetLastVolume() ) )
	{
		return LTFALSE;
	}

	// Ensure there are not AI in the way.

	if( g_pCharacterMgr->RayIntersectAI( vPos, vDest, m_pAI, LTNULL, LTNULL ) )
	{
		return LTFALSE;
	}

	// Setup retreat.

	m_pAI->ClearAndSetState( kState_HumanLongJump );
	CAIHumanStateLongJump* pLongJumpState = (CAIHumanStateLongJump*)m_pAI->GetState();
	pLongJumpState->SetLongJumpDest( vDest );
	pLongJumpState->SetLongJumpSequence( kAP_RetreatStart, kAP_RetreatFly, kAP_RetreatLand );
	pLongJumpState->SetLongJumpToTarget( LTFALSE );
	pLongJumpState->SetLongJumpSpeed( fRetreatSpeed );
	pLongJumpState->FaceDest( LTFALSE );

	m_pAI->PlaySound( kAIS_Retreat, LTFALSE );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalLunge::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanLongJump:
			HandleStateLongJump();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalLunge::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::HandleStateLongJump
//
//	PURPOSE:	Determine what to do when in state Lunge.
//
// ----------------------------------------------------------------------- //

void CAIGoalLunge::HandleStateLongJump()
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

		case kSStat_FailedComplete:			
			m_pAI->FaceTarget();
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			if( m_bLunged || !Retreat() )
			{
				m_pAI->FaceTarget();
				m_fCurImportance = 0.f;
			}
			m_bLunged = LTTRUE;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalLunge::HandleStateLongJump: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalLunge::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalLunge::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Check if AI is already jumping.

		if( m_pAI->GetState()->GetStateType() == kState_HumanLongJump )
		{
			return FALSE;
		}

		// Only one AI at a time may lunge.

		if( 0 != g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_AttackLunge, m_pAI, g_pLTServer->HandleToObject(m_pAI->GetTarget()->GetObject()) ) )
		{
			m_fCurImportance = 0.f;
			return LTFALSE;
		}

		LTVector vOrigin = m_pAI->GetPosition();
		LTVector vDest = pSenseRecord->vLastStimulusPos;

		// Check if the destination is in range.

		LTFLOAT fDistSqr = vOrigin.DistSqr( vDest );
		if( (fDistSqr < m_fLungeDistSqrMin) || (fDistSqr > m_fLungeDistSqrMax) )
		{
			m_fCurImportance = 0.f;
			return LTFALSE;
		}

		// Check if there is a straight line path (in volumes) to the target.

		if( !g_pAIVolumeMgr->StraightRadiusPathExists( m_pAI,
													vOrigin, 
													vDest, 
													m_pAI->GetRadius(), 
													m_pAI->GetVerticalThreshold() * 2.f, 
													AIVolume::kVolumeType_Ladder | AIVolume::kVolumeType_JumpOver | AIVolume::kVolumeType_AmbientLife | AIVolume::kVolumeType_Teleport, 
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

		m_vLungeDest = vDest;
		return LTTRUE;
	}

	return LTFALSE;
}

