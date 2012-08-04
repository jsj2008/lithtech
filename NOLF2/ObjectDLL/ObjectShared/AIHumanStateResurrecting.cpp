//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateResurrecting.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	28.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	
//
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIHUMANSTATERESURRECTING_H__
#include "AIHumanStateResurrecting.h"		
#endif

#ifndef __AI_HUMAN_H__
#include "AIHuman.h"
#endif

#include "MsgIds.h"
#include "DebrisFuncs.h"

// Forward declarations

// Globals

// Statics
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateResurrecting, kState_HumanResurrecting);


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::CAIHumanStateResurrecting()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
CAIHumanStateResurrecting::CAIHumanStateResurrecting()
{
	// Construct
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::~CAIHumanStateResurrecting()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ CAIHumanStateResurrecting::~CAIHumanStateResurrecting()
{
	// Destruct
	if ( GetAI() && !GetAI()->IsDead() )
	{
		CDestructible* pDestructable = GetAI()->GetDestructible();
		pDestructable->SetNeverDestroy( m_bEntryCanDistruct );
		pDestructable->SetHitPoints( pDestructable->GetMaxHitPoints() );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::Load()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStateResurrecting::Load(ILTMessage_Read *pMsg)
{
	CAIHumanState::Load(pMsg);
	LOAD_TIME(m_fResurrectCompleteTime);
	LOAD_FLOAT(m_fResurrectCompleteDuration);
	LOAD_BOOL(m_bEntryCanDistruct);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStateResurrecting::Save(ILTMessage_Write *pMsg)
{
	CAIHumanState::Save(pMsg);
	SAVE_TIME(m_fResurrectCompleteTime);
	SAVE_FLOAT(m_fResurrectCompleteDuration);
	SAVE_BOOL(m_bEntryCanDistruct);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::Init()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL CAIHumanStateResurrecting::Init(CAIHuman* pAIHuman)
{
	if ( !CAIHumanState::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	// Set the time tracking vars to invalid times
	m_fResurrectCompleteTime = -1;
	m_fResurrectCompleteDuration = -1;

	GetAI()->SetBlinking(LTFALSE);

	m_eStateStatus = kSStat_Resurrecting;

	CDestructible* pDestructable = GetAI()->GetDestructible();
	m_bEntryCanDistruct = pDestructable->GetNeverDestroy();
	pDestructable->SetNeverDestroy( LTFALSE );
	pDestructable->SetHitPoints( pDestructable->GetMaxHitPoints() );

	// Ensure that node tracking is disabled.
	m_pAIHuman->DisableNodeTracking();

	return LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::Update()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStateResurrecting::Update()
{
	CAIHumanState::Update();

	AIASSERT( m_fResurrectCompleteTime!=-1, GetAI()->m_hObject,
		"CAIHumanStateResurrecting::Update: m_fResurrectCompleteTime == -1" );

	if ( m_bFirstUpdate )
	{
		GetAI()->KillDlgSnd();

		// TEMP!!
		// Replace this with FXEd effects setup!
		CLIENTDEBRIS cd;
		cd.rRot			= m_pAI->GetRotation();
		cd.vPos			= m_pAI->GetPosition() + LTVector( 0.0f, 10.0f, 0.0f );
		cd.nDebrisId	= 43;
		::CreatePropDebris( cd.vPos, LTVector(0,1,0), 43 );
	}

	switch ( m_eStateStatus )
	{
		case kSStat_Conscious:
		break;

		case kSStat_RegainingConsciousness:
		{
			if ( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_Stand) )
			{
				m_eStateStatus = kSStat_Conscious;
				GetAI()->SetBlinking(LTTRUE);
			}
		}
		break;

		case kSStat_Resurrecting:
		{
			if ( m_fResurrectCompleteTime < g_pLTServer->GetTime() )
			{
				m_eStateStatus = kSStat_RegainingConsciousness;
			}
		}
		break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::UpdateAnimation()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStateResurrecting::UpdateAnimation()
{
	CAIHumanState::UpdateAnimation();

	switch ( m_eStateStatus )
	{
		case kSStat_Resurrecting:
			GetAI()->GetAnimationContext()->SetProp(kAPG_Posture, kAP_Prone);
			GetAI()->GetAnimationContext()->SetProp(kAPG_Action, kAP_Resurrecting);
			break;

		case kSStat_RegainingConsciousness:
		case kSStat_Conscious:
			GetAI()->GetAnimationContext()->SetProp(kAPG_Posture, kAP_Stand);
			GetAI()->GetAnimationContext()->SetProp(kAPG_WeaponPosition, kAP_Down);
			break;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::HandleDamage()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIHumanStateResurrecting::HandleDamage(const DamageStruct& damage)
{
	CAIHumanState::HandleDamage(damage);
	ResetExpirationTime();
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::RejectChangeState()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL CAIHumanStateResurrecting::RejectChangeState()
{
	return m_eStateStatus != kSStat_Conscious;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::GetDeathAni()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ HMODELANIM CAIHumanStateResurrecting::GetDeathAni(LTBOOL bFront)
{
	if ( !GetAnimationContext() )
		return INVALID_MODEL_ANIM;

	if ( !GetAnimationContext()->IsPropSet(kAPG_Action, kAP_Resurrecting ) )
		return INVALID_MODEL_ANIM;

	return g_pLTServer->GetAnimIndex(GetAI()->GetObject(), "PrUn");
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIHumanStateResurrecting::SetUnconsciousTime()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIHumanStateResurrecting::SetResurrectingTime(LTFLOAT fTime)
{
	m_fResurrectCompleteDuration = fTime;
}

void CAIHumanStateResurrecting::ResetExpirationTime()
{
	m_fResurrectCompleteTime = g_pLTServer->GetTime() + m_fResurrectCompleteDuration;
}