// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStateAttackProne.h"
#include "AICentralKnowledgeMgr.h"
#include "AIHuman.h"
#include "AITarget.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateAttackProne, kState_HumanAttackProne);


// ----------------------------------------------------------------------- //

CAIHumanStateAttackProne::CAIHumanStateAttackProne()
{
	m_pStrategyShoot = AI_FACTORY_NEW(CAIHumanStrategyShootBurst);
	m_fStayProneTime = 0.f;
	m_fLastFiredTime = 0.f;
	m_fHalfMinDistSqr = 0.f;
}

CAIHumanStateAttackProne::~CAIHumanStateAttackProne()
{
	AI_FACTORY_DELETE(m_pStrategyShoot);

	// Decrement attack counter.

	if( GetAI() )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_Attacking, GetAI() );
		AIASSERT( !g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, GetAI() ), GetAI()->m_hObject, "~CAIHumanStateAttackProne: Too many attackers registered!" );
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProne::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyShoot->Load(pMsg);
	LOAD_TIME( m_fStayProneTime );
	LOAD_TIME( m_fLastFiredTime );
	LOAD_FLOAT( m_fHalfMinDistSqr );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProne::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyShoot->Save(pMsg);
	SAVE_TIME( m_fStayProneTime );
	SAVE_TIME( m_fLastFiredTime );
	SAVE_FLOAT( m_fHalfMinDistSqr );
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateAttackProne::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyShoot->Init(pAIHuman) )
	{
		return LTFALSE;
	}

	m_fHalfMinDistSqr = 0.5f * m_pAI->GetBrain()->GetAIData( kAIData_ProneDistMin );
	m_fHalfMinDistSqr *= m_fHalfMinDistSqr;

	// Keep track of how many AI are attacking the same target.

	if( m_pAIHuman->HasTarget() )
	{
		if( g_pAICentralKnowledgeMgr->CountMatches( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()) ) )
		{
			AIASSERT( 0, m_pAIHuman->m_hObject, "CAIHumanStateAttackProne::Init: Already registered attacking count!" );
		}
		else {
			g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_Attacking, m_pAIHuman, g_pLTServer->HandleToObject(m_pAIHuman->GetTarget()->GetObject()), LTTRUE );
		}
	}

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProne::Update()
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

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	// Find the direction to the target.

	LTVector vTargetDir = m_pAI->GetTarget()->GetVisiblePosition() - m_pAI->GetPosition();
	vTargetDir.Normalize();

	// Snap AI to face the direction of his torso.

	if( m_bFirstUpdate )
	{
		GetAI()->FaceDir( vTargetDir );
		GetAI()->FaceTargetRotImmediately();
		GetAnimationContext()->ClearLock();
		return;
	}

	// Do no processing while transitioning into prone position.

	LTFLOAT fCurTime = g_pLTServer->GetTime();
	if( GetAnimationContext()->IsTransitioning() )
	{
		m_fStayProneTime = fCurTime + 3.f;
		return;
	}


	LTFLOAT fDot = GetAI()->GetForwardVector().Dot( vTargetDir );

	// Be sure to stay down at least 3 seconds.

	if( m_fStayProneTime < fCurTime )
	{
		// Bail if the target is outside of fov.
	
		if( fDot < c_fFOV60 ) 
		{
			m_eStateStatus = kSStat_StateComplete;
			return;
		}

		// Bail if target is within half of the minimum prone distance.

		if( m_fHalfMinDistSqr > GetAI()->GetTarget()->GetVisiblePosition().DistSqr( GetAI()->GetPosition() ) )
		{
			m_eStateStatus = kSStat_StateComplete;
			return;
		}
	}

	// Reload.

	if( m_pStrategyShoot->ShouldReload() )
	{
		m_pStrategyShoot->Reload(LTTRUE);
		return;
	}

	// Shoot if we are in the middle of shooting or reloading.

	if( m_pStrategyShoot->IsReloading() ||
		m_pStrategyShoot->IsFiring() )
	{
		m_pStrategyShoot->Update();
		m_fLastFiredTime = fCurTime;
		return;
	}

	// Shoot if target is visible, and within a small fov.

	if(	( GetAI()->GetTarget()->IsVisibleCompletely() ) &&
		( fDot > c_fFOV45 ) )
	{
		m_pStrategyShoot->Update();
		m_fLastFiredTime = fCurTime;
	}

	// Bail if haven't shot in 3 seconds.

	else if( ( m_fLastFiredTime + 3.f < fCurTime ) &&
			 ( m_fStayProneTime < fCurTime ) )
	{
		m_eStateStatus = kSStat_StateComplete;
		return;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProne::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Prone);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	m_pStrategyShoot->UpdateAnimation();
}

// ----------------------------------------------------------------------- //

void CAIHumanStateAttackProne::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage(damage);

	m_eStateStatus = kSStat_StateComplete;
}
