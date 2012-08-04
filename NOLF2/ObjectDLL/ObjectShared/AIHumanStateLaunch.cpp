// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStateLaunch.h"
#include "AIHuman.h"
#include "AIMovement.h"


DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateLaunch, kState_HumanLaunch);

// ----------------------------------------------------------------------- //

CAIHumanStateLaunch::CAIHumanStateLaunch()
{
	m_fLaunchSpeed = 0.f;
	m_fLaunchHeight = 0.f;
	m_eLaunchMovement = kAP_None;
}

CAIHumanStateLaunch::~CAIHumanStateLaunch()
{
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLaunch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_FLOAT( m_fLaunchSpeed );
	LOAD_FLOAT(	m_fLaunchHeight );
	LOAD_VECTOR( m_vLaunchDest );
	LOAD_DWORD_CAST( m_eLaunchMovement, EnumAnimProp );
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLaunch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_FLOAT( m_fLaunchSpeed );
	SAVE_FLOAT(	m_fLaunchHeight );
	SAVE_VECTOR( m_vLaunchDest );
	SAVE_DWORD( m_eLaunchMovement );
}

// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStateLaunch::Init(CAIHuman* pAIHuman)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Ensure that node tracking is disabled.

	m_pAIHuman->DisableNodeTracking();

	m_pAIHuman->SetAwareness( kAware_Alert );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLaunch::Update()
{
	super::Update();

	switch( m_eStateStatus )
	{
		case kSStat_Initialized:
			{
				// Setup the AI Movement to launch AI.

				GetAI()->GetAIMovement()->SetMovementDest( m_vLaunchDest );
				GetAI()->GetAIMovement()->FaceDest( LTTRUE );
				GetAI()->GetAIMovement()->SetSpeed( m_fLaunchSpeed );
				GetAI()->GetAIMovement()->SetParabola( m_fLaunchHeight );
				GetAI()->GetAIMovement()->IgnoreVolumes( LTTRUE );

				m_eStateStatus = kSStat_Moving;
			}
			break;

		case kSStat_Moving:
			if( GetAI()->GetAIMovement()->IsDone() )
			{
				m_eStateStatus = kSStat_StateComplete;
			}
			break;
	}
}

// ----------------------------------------------------------------------- //

void CAIHumanStateLaunch::UpdateAnimation()
{
	super::UpdateAnimation();

	GetAnimationContext()->SetProp( kAPG_Posture, kAP_Stand );
	GetAnimationContext()->SetProp( kAPG_WeaponPosition, kAP_Down );
	GetAnimationContext()->SetProp( kAPG_Movement, m_eLaunchMovement );
}
