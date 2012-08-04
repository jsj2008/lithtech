// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStateAttackMove.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "AICentralKnowledgeMgr.h"
#include "TrackedNodeContext.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackMove, kState_HumanAttackMove);

// ----------------------------------------------------------------------- //

CAIHumanStateAttackMove::CAIHumanStateAttackMove()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);

	m_eAttackMove = kAP_None;
	m_bTurnedAround = LTFALSE;
}

CAIHumanStateAttackMove::~CAIHumanStateAttackMove()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
	AI_FACTORY_DELETE(m_pStrategyShoot);

	// Decrement attack counter.

	if( GetAI() )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateAttackMove: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackMove::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyShoot->Load(pMsg);
	m_pStrategyFollowPath->Load(pMsg);

	LOAD_DWORD_CAST( m_eAttackMove, EnumAnimProp );
	LOAD_VECTOR( m_vAttackMoveDest );
	LOAD_BOOL( m_bTurnedAround );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackMove::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyShoot->Save(pMsg);
	m_pStrategyFollowPath->Save(pMsg);

	SAVE_DWORD( m_eAttackMove );
	SAVE_VECTOR( m_vAttackMoveDest );
	SAVE_BOOL( m_bTurnedAround );
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackMove::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, m_pStrategyShoot) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);

	// Keep track of how many AI are attacking the same target.

	if( m_pAIHuman->HasTarget() )
	{
		if( g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()) ) )
		{
			AIASSERT( 0, m_pAIHuman->m_hObject, "CAIHumanStateAttackMove::Init: Already registered attacking count!" );
		}
		else {
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()), LTTRUE);
		}
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackMove::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	HOBJECT hTarget = GetAI()->GetTarget()->GetObject();
	HOBJECT hAI = GetAI()->m_hObject;

	// Head and Torso tracking.
	
	if( m_bFirstUpdate )
	{
		GetAI()->SetNodeTrackingTarget( kTrack_AimAt, hTarget, "Head" );
	}

	// Handle update appropriately for the type of move.

	switch( m_eAttackMove )
	{
		// Step.

		case kAP_ShuffleRight:
		case kAP_ShuffleLeft:
			if( m_bFirstUpdate )
			{
				// Snap AI to face the direction of his torso.

				GetAI()->FaceDir( GetAI()->GetTorsoForward() );
				GetAI()->FaceTargetRotImmediately();
				GetAnimationContext()->ClearLock();
			}
			else if( !GetAnimationContext()->IsLocked() )
			{
				m_eStateStatus = kSStat_StateComplete;
			}

			// Head and Torso tracking.

			GetAI()->EnableNodeTracking( kTrack_AimAt, hTarget );
			break;

		// BackUp.

		case kAP_BackUp:
		case kAP_FlankLeft:
		case kAP_FlankRight:
			if( m_pStrategyFollowPath->IsUnset() )
			{
				if( m_eAttackMove == kAP_BackUp )
				{
					m_pStrategyFollowPath->SetMovement( kAP_BackUp );
				}
				else {
					m_pStrategyFollowPath->SetMovement( kAP_Run );
				}

				m_bTurnedAround = LTFALSE;

				if( !m_pStrategyFollowPath->Set( m_vAttackMoveDest, LTFALSE ) )
				{
					m_eStateStatus = kSStat_FailedComplete;
				}
			}

			if( m_pStrategyFollowPath->IsDone() )
			{
				m_eStateStatus = kSStat_StateComplete;
			}
			else if( m_pStrategyFollowPath->IsSet() )
			{
				SelectMoveAnim();
				m_pStrategyFollowPath->Update();
			}

			if( m_pStrategyShoot->ShouldReload() )
			{
				m_pStrategyShoot->Reload(LTTRUE);
			}

			if( m_pStrategyShoot->IsReloading() ||
				m_pStrategyShoot->IsFiring() ||
				GetAI()->GetTarget()->IsVisibleCompletely() )
			{
				m_pStrategyShoot->Update();
			}

			// Head and Torso tracking.

			GetAI()->EnableNodeTracking( kTrack_AimAt, LTNULL );
			break;

		default:
			AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateAttackMove::Update: Unexpected attack move." );
			m_eStateStatus = kSStat_FailedComplete;
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackMove::SelectMoveAnim()
{
	// Only turn around once.
	// This prevents the AI from flipping out when the players runs around him.

	if( m_bTurnedAround )
	{
		return;
	}

	LTVector vDestDir = m_vAttackMoveDest - GetAI()->GetPosition();
	vDestDir.y = 0.f;
	vDestDir.Normalize();
	LTVector vTargetDir = GetAI()->GetTarget()->GetVisiblePosition() - GetAI()->GetPosition();
	vTargetDir.y = 0.f;
	vTargetDir.Normalize();

	// Should the AI turn around and face forward?

	if( m_pStrategyFollowPath->GetMovement() == kAP_BackUp )
	{
		LTFLOAT fDot = vDestDir.Dot( vTargetDir );	
		if( fDot > c_fFOV160 )
		{
			GetAI()->FaceTargetRotImmediately();
			m_pStrategyFollowPath->SetMovement( kAP_Run );
			m_bTurnedAround = LTTRUE;
		}
	}

	// Should the AI turn around and face backward?

	else {
		vDestDir = -vDestDir;
		LTFLOAT fDot = vDestDir.Dot( vTargetDir );	
		if( fDot > c_fFOV160 )
		{
			GetAI()->FaceTargetRotImmediately();
			m_pStrategyFollowPath->SetMovement( kAP_BackUp );
			m_bTurnedAround = LTTRUE;
		}
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackMove::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	switch( m_eAttackMove )
	{
		// Step.

		case kAP_ShuffleRight:
		case kAP_ShuffleLeft:
			GetAnimationContext()->SetProp( kAPG_Evasive, m_eAttackMove );
			GetAnimationContext()->Lock();
			break;

		// BackUp.

		case kAP_BackUp:
		case kAP_FlankLeft:
		case kAP_FlankRight:
			m_pStrategyShoot->UpdateAnimation();
			m_pStrategyFollowPath->UpdateAnimation();
			break;

		default: AIASSERT( 0, GetAI()->m_hObject, "CAIHumanStateAttackMove::UpdateAnimation: Unexpected attack move." );
	}
}
