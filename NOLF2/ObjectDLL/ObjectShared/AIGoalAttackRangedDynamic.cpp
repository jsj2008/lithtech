// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackRangedDynamic.cpp
//
// PURPOSE : AIGoalAttackRangedDynamic implementation
//
// CREATED : 6/5/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackRangedDynamic.h"
#include "AIGoalMgr.h"
#include "AIHumanStateAttackMove.h"
#include "AI.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "AITarget.h"
#include "AnimatorPlayer.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackRangedDynamic, kGoal_AttackRangedDynamic);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRangedDynamic::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttackRangedDynamic::CAIGoalAttackRangedDynamic()
{
	m_fMoveTime = 0.f;

	// BackUp is duplicated to increase the odds agains the duplicate
	// Right and Left verions of the other choices.

	m_eAttackMoves[0] = kAP_ShuffleLeft;
	m_eAttackMoves[1] = kAP_BackUp;
	m_eAttackMoves[2] = kAP_ShuffleRight;
	m_eAttackMoves[3] = kAP_BackUp;
	m_eAttackMoves[4] = kAP_FlankLeft;
	m_eAttackMoves[5] = kAP_BackUp;
	m_eAttackMoves[6] = kAP_FlankRight;
}	

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRangedDynamic::InitGoal
//
//	PURPOSE:	Initialize goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRangedDynamic::InitGoal(CAI* pAI)
{
	super::InitGoal( pAI );

	// Ensure all needed data is present.

	AIASSERT( m_pAI->GetBrain()->GetAIDataExist( kAIData_DynMoveTimeMin ), m_pAI->m_hObject, "CAIGoalAttackRangedDynamic::InitGoal: Brain missing AIData DynMoveTimeMin" );
	AIASSERT( m_pAI->GetBrain()->GetAIDataExist( kAIData_DynMoveTimeMax ), m_pAI->m_hObject, "CAIGoalAttackRangedDynamic::InitGoal: Brain missing AIData DynMoveTimeMax" );
	AIASSERT( m_pAI->GetBrain()->GetAIDataExist( kAIData_DynMoveBackupDist ), m_pAI->m_hObject, "CAIGoalAttackRangedDynamic::InitGoal: Brain missing AIData DynMoveBackupDist" );
	AIASSERT( m_pAI->GetBrain()->GetAIDataExist( kAIData_DynMoveFlankPassDist ), m_pAI->m_hObject, "CAIGoalAttackRangedDynamic::InitGoal: Brain missing AIData DynMoveFlankPassDist" );
	AIASSERT( m_pAI->GetBrain()->GetAIDataExist( kAIData_DynMoveFlankWidthDist ), m_pAI->m_hObject, "CAIGoalAttackRangedDynamic::InitGoal: Brain missing AIData DynMoveFlankWidthDist" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRangedDynamic::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRangedDynamic::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_TIME( m_fMoveTime );
}

void CAIGoalAttackRangedDynamic::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_TIME( m_fMoveTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRangedDynamic::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRangedDynamic::ActivateGoal()
{
	// Bail if we are already in the middle of an attack move.

	if( m_pAI->GetState()->GetStateType() == kState_HumanAttackMove )
	{
		return;
	}

	super::ActivateGoal();

	// Only reset the move time on first call to activate.

	if( m_fMoveTime == 0.f )
	{
		ResetMoveTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRangedDynamic::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRangedDynamic::DeactivateGoal()
{
	super::DeactivateGoal();

	m_fMoveTime = 0.f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRangedDynamic::ResetMoveTime
//
//	PURPOSE:	Choose next randome move time.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRangedDynamic::ResetMoveTime()
{
	m_fMoveTime = g_pLTServer->GetTime() + GetRandom( m_pAI->GetBrain()->GetAIData( kAIData_DynMoveTimeMin ), 
													  m_pAI->GetBrain()->GetAIData( kAIData_DynMoveTimeMax ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRangedDynamic::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRangedDynamic::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch( pState->GetStateType() )
	{
		// Move around between attacks.

		case kState_HumanAttack:
			{
				CAIHumanStateAttack* pAttackState = (CAIHumanStateAttack*)m_pAI->GetState();

				// Do not move immediately after a dodge.

				if( pAttackState->IsDodging() )
				{
					ResetMoveTime();
				}

				// It is time to move between attacks.

				else if( ( !m_pAI->GetAnimationContext()->IsLocked() ) && 
						 ( g_pLTServer->GetTime() > m_fMoveTime ) )
				{
					ResetMoveTime();

					// We are done if we found a valid move.

					if( SelectAttackMove() )
					{
						return;
					}
				}
			}
			break;

		case kState_HumanAttackMove:
			HandleStateAttackMove();
			return;
	}

	super::ActivateGoal();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttack::HandleStateAttackMove
//
//	PURPOSE:	Determine what to do when in state AttackMove.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackRangedDynamic::HandleStateAttackMove()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			m_pGoalMgr->UnlockGoal( this );
			ResetMoveTime();
			SetStateAttack();
			break;

		case kSStat_StateComplete:
			m_pGoalMgr->UnlockGoal( this );
			ResetMoveTime();
			SetStateAttack();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackRangedDynamic::HandleStateAttackMove: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackRangedDynamic::SelectAttackMove
//
//	PURPOSE:	Select a valid move between attacks.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAttackRangedDynamic::SelectAttackMove()
{
	uint32 dwExcludeVolumes =	AIVolume::kVolumeType_Ladder | 
								AIVolume::kVolumeType_Stairs |
								AIVolume::kVolumeType_JumpOver | 
								AIVolume::kVolumeType_JumpUp | 
								AIVolume::kVolumeType_AmbientLife |
								AIVolume::kVolumeType_Teleport;

	LTVector vPos = m_pAI->GetPosition();

	// Randomly select an attack move.

	uint32 iMove = GetRandom( 0, kNumAttackMoves - 1 );
	uint32 iFirstTry = iMove;

	EnumAnimProp eMove = kAP_None;
	LTVector vDest( 0.f, 0.f, 0.f);

	LTBOOL bTriedBackup = LTFALSE;

	// Search for a valid move, starting from a random index.

	while( 1 )
	{
		// Check if move is valid.
	
		switch( m_eAttackMoves[iMove] )
		{
			// Step right.

			case kAP_ShuffleRight:
				vDest = vPos + ( m_pAI->GetTorsoRight() * ( m_pAI->GetBrain()->GetDodgeVectorShuffleDist() + m_pAI->GetRadius() ) );
				eMove = kAP_ShuffleRight;
				break;

			// Step left.

			case kAP_ShuffleLeft:
				vDest = vPos - ( m_pAI->GetTorsoRight() * ( m_pAI->GetBrain()->GetDodgeVectorShuffleDist() + m_pAI->GetRadius() ) );
				eMove = kAP_ShuffleLeft;
				break;

			// BackUp.

			case kAP_BackUp:
				if( m_pAI->GetBrain()->AttacksWhileMoving() )
				{
					vDest = vPos - ( m_pAI->GetTorsoForward() * ( m_pAI->GetBrain()->GetAIData( kAIData_DynMoveBackupDist ) + m_pAI->GetRadius() ) );
					eMove = kAP_BackUp;
				}
				break;

			// Flank left.

			case kAP_FlankLeft:
				if( m_pAI->GetBrain()->AttacksWhileMoving() )
				{
					vDest = m_pAI->GetTarget()->GetVisiblePosition() - ( m_pAI->GetTorsoRight() * m_pAI->GetBrain()->GetAIData( kAIData_DynMoveFlankWidthDist ) );
					vDest += m_pAI->GetTorsoForward() * m_pAI->GetBrain()->GetAIData( kAIData_DynMoveFlankPassDist );
					eMove = kAP_FlankLeft;
				}
				break;

			// Flank right.

			case kAP_FlankRight:
				if( m_pAI->GetBrain()->AttacksWhileMoving() )
				{
					vDest = m_pAI->GetTarget()->GetVisiblePosition() + ( m_pAI->GetTorsoRight() * m_pAI->GetBrain()->GetAIData( kAIData_DynMoveFlankWidthDist ) );
					vDest += m_pAI->GetTorsoForward() * m_pAI->GetBrain()->GetAIData( kAIData_DynMoveFlankPassDist );
					eMove = kAP_FlankRight;
				}
				break;

			// Unexpected.

			default:
				AIASSERT( 0, m_pAI->m_hObject, "CAIGoalAttackRangedDynamic::SelectAttackMove: Unexpected attack move." );
				return LTFALSE;
		}

		// Do an attack move if the destination is in range, and there is a clear path.

		if( ( eMove != kAP_None ) && 
			( ( eMove != kAP_BackUp ) || ( !bTriedBackup ) ) &&
			( g_pAIVolumeMgr->StraightPathExists( m_pAI, vPos, vDest, m_pAI->GetVerticalThreshold(), dwExcludeVolumes, m_pAI->GetLastVolume() ) ) &&
			( !g_pCharacterMgr->RayIntersectAI( vPos, vDest, m_pAI, LTNULL, LTNULL ) ) )
		{
			m_pAI->SetState( kState_HumanAttackMove );
			CAIHumanStateAttackMove* pAttackMoveState = (CAIHumanStateAttackMove*)m_pAI->GetState();
			pAttackMoveState->SetAttackMove( eMove );
			pAttackMoveState->SetAttackMoveDest( vDest );

			m_pGoalMgr->LockGoal( this );
			return LTTRUE;
		}

		if( eMove == kAP_BackUp )
		{
			bTriedBackup = LTTRUE;
		}

		// Try another move.

		iMove = ( iMove + 1 ) % kNumAttackMoves;
		eMove = kAP_None;

		// All moves were tested, and no move was found.

		if( iMove == iFirstTry )
		{
			return LTFALSE;
		}
	}

	// Should never get here.

	AIASSERT( 0, m_pAI->m_hObject, "CAIGoalAttackRangedDynamic::SelectAttackMove: Should never get here." );
	return LTFALSE;
}
