// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackProne.cpp
//
// PURPOSE : AIGoalAttackProne implementation
//
// CREATED : 6/25/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackProne.h"
#include "AIGoalMgr.h"
#include "AIGoalButeMgr.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "AIUtils.h"
#include "AIHumanStateAttackProne.h"
#include "AIVolumeMgr.h"
#include "AIVolume.h"
#include "AICentralKnowledgeMgr.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackProne, kGoal_AttackProne);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProne::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttackProne::CAIGoalAttackProne()
{
	m_fProneTimeLimit = 0.f;
	m_fMinDistSqr = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProne::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProne::InitGoal(CAI* pAI)
{
	super::InitGoal( pAI );

	// Ensure all needed data is present.

	AIASSERT( m_pAI->GetBrain()->GetAIDataExist( kAIData_ProneTime ), m_pAI->m_hObject, "CAIGoalAttackProne::InitGoal: Brain missing AIData ProneTime" );
	AIASSERT( m_pAI->GetBrain()->GetAIDataExist( kAIData_ProneDistMin ), m_pAI->m_hObject, "CAIGoalAttackProne::InitGoal: Brain missing AIData ProneDistMin" );
	AIASSERT( m_pAI->GetBrain()->GetAIDataExist( kAIData_ProneSlideDist ), m_pAI->m_hObject, "CAIGoalAttackProne::InitGoal: Brain missing AIData ProneSlideDist" );

	// Calculate dist squared.

	m_fMinDistSqr = m_pAI->GetBrain()->GetAIData( kAIData_ProneDistMin );
	m_fMinDistSqr *= m_fMinDistSqr;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProne::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProne::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_TIME( m_fProneTimeLimit );
	SAVE_FLOAT( m_fMinDistSqr );
}

void CAIGoalAttackProne::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_TIME( m_fProneTimeLimit );
	LOAD_FLOAT( m_fMinDistSqr );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProne::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProne::ActivateGoal()
{
	super::ActivateGoal();

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pAI->SetCurrentWeapon( kAIWeap_Ranged );

	// Start attacking.

	if( m_pAI->GetState()->GetStateType() != kState_HumanAttackProne )
	{
		m_pGoalMgr->LockGoal( this );
		m_pAI->SetState( kState_HumanAttackProne );
		m_fProneTimeLimit = g_pLTServer->GetTime() + m_pAI->GetBrain()->GetAIData( kAIData_ProneTime );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProne::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProne::DeactivateGoal()
{
	super::DeactivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProne::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAttackProne:
			HandleStateAttackProne();
			break;

		case kState_HumanIdle:
			if( !m_pAI->GetAnimationContext()->IsTransitioning() )
			{
				m_pGoalMgr->UnlockGoal( this );
				m_fCurImportance = 0.f;
			}
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackProne::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleStateAttackProne
//
//	PURPOSE:	Determine what to do when in state AttackProne.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackProne::HandleStateAttackProne()
{
	// Prone time limit expired.

	if( g_pLTServer->GetTime() > m_fProneTimeLimit )
	{
		m_pAI->SetState( kState_HumanIdle );
		return;
	}

	// Attacking prone.

	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_pAI->SetState( kState_HumanIdle );
			break;

		case kSStat_FailedComplete:
			m_pAI->SetState( kState_HumanIdle );
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackProne::HandleStateAttackProne: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProne::GetAlternateDeathAnimation
//
//	PURPOSE:	Give goal a chance to choose an appropriate death animation.
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIGoalAttackProne::GetAlternateDeathAnimation()
{
	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;

	CAnimationProps animProps;
	animProps.Set( kAPG_Posture, kAP_Prone );
	animProps.Set( kAPG_Weapon, pAIHuman->GetCurrentWeaponProp() );
	animProps.Set( kAPG_Action, kAP_Death );

	// Find a death animation.

	if( m_pAI->GetAnimationContext()->AnimationExists( animProps ) )
	{
		return m_pAI->GetAnimationContext()->GetAni( animProps );
	}

	// No alternate.

	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackProne::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAttackProne::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Keep goal active while stimulated and attacking.

		if( m_pGoalMgr->IsCurGoal( this ) )
		{
			return ( m_pAI->GetState()->GetStateType() == kState_HumanAttackProne );
		}

		// Check range.
		// Need a weapon to activate an attack goal.
		// Target must be visible.

		m_pAI->Target(pSenseRecord->hLastStimulusSource);
		if( m_pAI->HasTarget() 
			&& m_pAI->HasWeapon( kAIWeap_Ranged )
			&& ( ( m_pAI->GetTarget()->IsVisibleCompletely() ) ||
				 ( IsAI( m_pAI->GetTarget()->GetVisionBlocker() ) ) ) )
		{
			// Only 1 AI can attack prone at a time.

			if( g_pAICentralKnowledgeMgr->GetKnowledgeFloat( kCK_NextProneTime, LTNULL ) > g_pLTServer->GetTime() )
			{
				return LTFALSE;
			}

			// Do not go prone if target is too close.

			if( m_pAI->GetTarget()->GetVisiblePosition().DistSqr( m_pAI->GetPosition() ) < m_fMinDistSqr )
			{
				return LTFALSE;
			}

			// Find the direction to the target.

			LTVector vTargetDir = m_pAI->GetTarget()->GetVisiblePosition() - m_pAI->GetPosition();
			vTargetDir.Normalize();

			// Check if there is a straight line path (in volumes) to the target.

			LTVector vOrigin = m_pAI->GetPosition();
			LTVector vDest = vOrigin + ( vTargetDir * m_pAI->GetBrain()->GetAIData( kAIData_ProneSlideDist ) );

			if( !g_pAIVolumeMgr->StraightRadiusPathExists( m_pAI, 
													vOrigin, 
													vDest, 
													m_pAI->GetRadius(), 
													m_pAI->GetVerticalThreshold() * 2.f, 
													AIVolume::kVolumeType_Ladder | 
														AIVolume::kVolumeType_Stairs | 
														AIVolume::kVolumeType_Ledge | 
														AIVolume::kVolumeType_JumpUp | 
														AIVolume::kVolumeType_JumpOver | 
														AIVolume::kVolumeType_AmbientLife |
														AIVolume::kVolumeType_Teleport, 
													m_pAI->GetLastVolume() ) )
			{
				return LTFALSE;
			}

			// Check if there are other AI in the way.

			if( g_pCharacterMgr->RayIntersectAI( vOrigin, vDest, m_pAI, LTNULL, LTNULL ) )
			{
				return LTFALSE;
			}

			// Prevent multiple AI from going prone at once.

			AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( kGoal_AttackProne );
			if( pTemplate )
			{
				LTFLOAT fNextProneTime = g_pLTServer->GetTime() + GetRandom( pTemplate->fFrequencyMin, pTemplate->fFrequencyMax );
				g_pAICentralKnowledgeMgr->RemoveAllKnowledge( kCK_NextProneTime, m_pAI );
				g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_NextProneTime, m_pAI, g_pLTServer->HandleToObject(m_hStimulusSource), LTFALSE, fNextProneTime, LTTRUE);
			}

			// Only actually go prone with a 1 in 3 chance.

			if( GetRandom( 0.f, 3.f ) > 2.f )
			{
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

