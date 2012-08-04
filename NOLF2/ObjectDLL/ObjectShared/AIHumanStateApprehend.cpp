// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStateApprehend.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "TrackedNodeContext.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateApprehend, kState_HumanApprehend);

// ----------------------------------------------------------------------- //

CAIHumanStateApprehend::CAIHumanStateApprehend()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);
	m_fHoldTimer = 0.f;
	m_fCloseEnoughSqr = 0.f;
}

CAIHumanStateApprehend::~CAIHumanStateApprehend()
{
	AI_FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateApprehend::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	LOAD_VECTOR(m_vOrigTargetPos);
	LOAD_FLOAT(m_fHoldTimer);
	LOAD_FLOAT(m_fCloseEnoughSqr);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateApprehend::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	SAVE_VECTOR(m_vOrigTargetPos);
	SAVE_FLOAT(m_fHoldTimer);
	SAVE_FLOAT(m_fCloseEnoughSqr);
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateApprehend::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Walk);
	m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateApprehend::Update()
{
	super::Update();

	// Make sure we have a target

	if ( !GetAI()->HasTarget() )
	{
		m_eStateStatus = kSStat_FailedComplete;
		return;
	}

	switch( m_eStateStatus )
	{
		//
		// Start apprehending.
		//

		case kSStat_Initialized:
			GetAI()->PlaySound( kAIS_Apprehend, LTFALSE );
			m_eStateStatus = kSStat_Holding;

			// Head and Torso tracking.

			GetAI()->SetNodeTrackingTarget( kTrack_AimAt, GetAI()->GetTarget()->GetObject(), "Head" );
			break;

		//
		// Hold.
		//

		case kSStat_Holding:

			// Head and Torso tracking.

			GetAI()->EnableNodeTracking( kTrack_AimAt, GetAI()->GetTarget()->GetObject() );

			// Decrement hold timer, and check if it's time to move.

			m_fHoldTimer -= g_pLTServer->GetFrameTime();

			if( m_fHoldTimer <= 0.f )
			{
				// Record target's original position.
			
				m_vOrigTargetPos = GetAI()->GetTarget()->GetVisiblePosition();

				LTBOOL bDivergePaths = GetAI()->GetBrain()->GetAIDataExist( kAIData_DivergePaths ) && 
					( GetAI()->GetBrain()->GetAIData( kAIData_DivergePaths ) > 0.f );

				if ( !m_pStrategyFollowPath->Set( m_vOrigTargetPos, bDivergePaths ) )
				{
					// WE COULDN'T SET A PATH
					m_eStateStatus = kSStat_FailedComplete;
					return;
				}

				m_eStateStatus = kSStat_Moving;
			}

			break;

		//
		// Walk to target.
		//

		case kSStat_Moving:

			// Head and Torso tracking.

			GetAI()->EnableNodeTracking( kTrack_AimAt, LTNULL );

			// Target moved!

			if( m_vOrigTargetPos != GetAI()->GetTarget()->GetVisiblePosition() )
			{
				m_eStateStatus = kSStat_FailedComplete;
			}

			// Check path progress.

			if( m_pStrategyFollowPath->IsDone() )
			{
				m_eStateStatus = kSStat_StateComplete;
			}

			// Check if we are close enough.

			if( m_vOrigTargetPos.DistSqr( GetAI()->GetPosition() ) <= m_fCloseEnoughSqr )
			{
				m_eStateStatus = kSStat_StateComplete;
			}

			break;
	}

	if ( m_pStrategyFollowPath->IsSet() )
	{
		// TODO: check for strategy failure
		m_pStrategyFollowPath->Update();
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateApprehend::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	switch( m_eStateStatus )
	{
		case kSStat_Moving:
			m_pStrategyFollowPath->UpdateAnimation();
			GetAnimationContext()->SetProp(kAPG_Awareness, kAP_Investigate);
			break;
	}
}
