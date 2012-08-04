// (c) 2001 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStateDisappearReappear.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "AIPathMgr.h"
#include "AIUtils.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDisappearReappear, kState_HumanDisappearReappear);

// ----------------------------------------------------------------------- //

CAIHumanStateDisappearReappear::CAIHumanStateDisappearReappear()
{
	m_pStrategyFollowPath = AI_FACTORY_NEW(CAIHumanStrategyFollowPath);

	m_fFadeTime		= 0.f;
	m_fFadeTimer	= 0.f;

	m_bStartFade = LTFALSE;

	m_fReappearDistOverride = 0.f;

	m_fDisappearTime = 0.f;
	m_fReappearDelay = 5.f;
}

CAIHumanStateDisappearReappear::~CAIHumanStateDisappearReappear()
{
	// Make sure AI reappears if she changes states while invisible.
	Reappear();
	g_pLTServer->SetObjectColor( GetAI()->m_hObject, 1.f, 1.f, 1.f, 1.f );

	AI_FACTORY_DELETE(m_pStrategyFollowPath);
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDisappearReappear::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	m_pStrategyFollowPath->Load(pMsg);

	LOAD_VECTOR( m_vDestPos);
	LOAD_FLOAT(	m_fFadeTime	);
	LOAD_FLOAT( m_fFadeTimer );
	LOAD_BOOL( m_bStartFade );
	LOAD_FLOAT( m_fReappearDistOverride );
	LOAD_TIME(	m_fDisappearTime );
	LOAD_FLOAT( m_fReappearDelay );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDisappearReappear::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	m_pStrategyFollowPath->Save(pMsg);

	SAVE_VECTOR(m_vDestPos);
	SAVE_FLOAT(	m_fFadeTime	);
	SAVE_FLOAT( m_fFadeTimer );
	SAVE_BOOL( m_bStartFade );
	SAVE_FLOAT( m_fReappearDistOverride );
	SAVE_TIME(	m_fDisappearTime );
	SAVE_FLOAT( m_fReappearDelay );
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateDisappearReappear::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	if ( !m_pStrategyFollowPath->Init(pAIHuman, LTNULL) )
	{
		return LTFALSE;
	}

	m_pStrategyFollowPath->SetMovement(kAP_Run);
	m_pStrategyFollowPath->SetMedium(CAIHumanStrategyFollowPath::eMediumGround);

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Alert );

	m_fFadeTime = m_pAI->GetBrain()->GetAIData( kAIData_DisappearFadeTime );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDisappearReappear::HandleModelString(ArgList* pArgList)
{
	super::HandleModelString(pArgList);

	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp( szKey, c_szKeyFX ) )
	{
		m_bStartFade = LTTRUE;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDisappearReappear::Update()
{
	super::Update();

	switch( m_eStateStatus )
	{
		//
		// Start disappearing.
		//

		case kSStat_Initialized:
			{
				// Use the distance from the brain, or the override.

				LTFLOAT fReappearDist;
				if( m_fReappearDistOverride > 0.f )
				{
					fReappearDist = m_fReappearDistOverride;
				}
				else {
					fReappearDist = m_pAI->GetBrain()->GetAIData( kAIData_ReappearDist );
				}

				if( !g_pAIPathMgr->FindRandomPosition( GetAI(), GetAI()->GetLastVolume(), GetAI()->GetPosition(), fReappearDist, &m_vDestPos ) )
				{
					AITRACE( AIShowStates, ( GetAI()->m_hObject, "HumanStateDisappearReappear: Could not find random position %.2f dist away!.", fReappearDist ) );
					m_vDestPos = GetAI()->GetPosition();
				}
	
				m_fDisappearTime = g_pLTServer->GetTime();

				GetAI()->PlaySound( kAIS_Disappear, LTFALSE );

				m_eStateStatus = kSStat_Disappearing;
			}
			break;

		//
		// Disappear.
		//

		case kSStat_Disappearing:
			if( GetAI()->HasTarget() )
			{
				GetAI()->FaceTarget();
			}

			UpdateAlpha();

			if( !GetAnimationContext()->IsLocked() )
			{
				m_pStrategyFollowPath->Set( m_vDestPos, LTFALSE );

				m_eStateStatus = kSStat_Moving;

				// Go invisible.

				Disappear();
			}
			break;

		//
		// Run away.
		//

		case kSStat_Moving:

			// Check path progress.

			if( ( m_pStrategyFollowPath->IsDone() || m_pStrategyFollowPath->IsUnset() ) &&
				( g_pLTServer->GetTime() > m_fDisappearTime + m_fReappearDelay ) )
			{
				GetAI()->PlaySound( kAIS_Reappear, LTFALSE );

				m_eStateStatus = kSStat_Reappearing;
				Reappear();
			}

			if ( m_pStrategyFollowPath->IsSet() && ( m_eStateStatus == kSStat_Moving ) )
			{
				m_pStrategyFollowPath->Update();
			}

			break;

		//
		// Reappear.
		//

		case kSStat_Reappearing:
			if( GetAI()->HasTarget() )
			{
				GetAI()->FaceTarget();
			}

			UpdateAlpha();

			if( !GetAnimationContext()->IsLocked() )
			{
				g_pLTServer->SetObjectColor( GetAI()->m_hObject, 1.f, 1.f, 1.f, 1.f );

				m_eStateStatus = kSStat_StateComplete;
			}
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDisappearReappear::Disappear()
{
	SendTriggerMsgToObject(GetAI(), GetAI()->m_hObject, LTFALSE, "HIDDEN 1");

	g_pLTServer->SetObjectColor( GetAI()->m_hObject, 1.f, 1.f, 1.f, 0.f );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDisappearReappear::Reappear()
{
	SendTriggerMsgToObject(GetAI(), GetAI()->m_hObject, LTFALSE, "HIDDEN 0");

	g_pLTServer->SetObjectColor( GetAI()->m_hObject, 1.f, 1.f, 1.f, 0.f );
	m_fFadeTimer = 0.f;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDisappearReappear::UpdateAlpha()
{
	if( !m_bStartFade )
	{
		return;
	}

	m_fFadeTimer += g_pLTServer->GetFrameTime();

	LTFLOAT fAlpha;

	switch( m_eStateStatus )
	{
		case kSStat_Disappearing:
			if( m_fFadeTimer < m_fFadeTime )
			{
				fAlpha = 1.f - ( m_fFadeTimer / m_fFadeTime );
				g_pLTServer->SetObjectColor( GetAI()->m_hObject, 1.f, 1.f, 1.f, fAlpha );
			}
			else {
				Disappear();
			}
			break;

		case kSStat_Reappearing:
			if( m_fFadeTimer < m_fFadeTime )
			{
				fAlpha = m_fFadeTimer / m_fFadeTime;
				g_pLTServer->SetObjectColor( GetAI()->m_hObject, 1.f, 1.f, 1.f, fAlpha );
			}
			else {
				g_pLTServer->SetObjectColor( GetAI()->m_hObject, 1.f, 1.f, 1.f, 1.f );
			}
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateDisappearReappear::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
	GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Up);

	switch( m_eStateStatus )
	{
		case kSStat_Disappearing:
			GetAnimationContext()->SetProp(kAPG_Action, kAP_Disappear);
			if( !GetAnimationContext()->IsLocked() )
			{
				GetAnimationContext()->ClearLock();
				GetAnimationContext()->Lock();
			}
			break;

		case kSStat_Moving:
			m_pStrategyFollowPath->UpdateAnimation();
			break;

		case kSStat_Reappearing:
			GetAnimationContext()->SetProp(kAPG_Action, kAP_Reappear);
			if( !GetAnimationContext()->IsLocked() )
			{
				GetAnimationContext()->ClearLock();
				GetAnimationContext()->Lock();
			}
	}
}
