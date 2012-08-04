// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCounterMelee.cpp
//
// PURPOSE : 
//
// CREATED : 9/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalCounterMelee.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIGoalMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCounterMelee, kGoal_CounterMelee );

static CAIWMFact* GetCounterMeleeDesireFact( CAI* pAI )
{
	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Desire );
	queryFact.SetDesireType( kDesire_CounterMelee );
	CAIWMFact* pFact = pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
	if ( NULL == pFact )
	{
		return NULL;
	}

	if ( 1.0f != pFact->GetConfidence( CAIWMFact::kFactMask_DesireType ) )
	{
		return NULL;
	}

	return pFact;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCounterMelee::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCounterMelee::CAIGoalCounterMelee() : 
	m_flLastCounterMeleeTime( 0.0f )
{
}

CAIGoalCounterMelee::~CAIGoalCounterMelee()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalCounterMelee::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalCounterMelee
//              
//----------------------------------------------------------------------------

void CAIGoalCounterMelee::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_TIME( m_flLastCounterMeleeTime );
}

void CAIGoalCounterMelee::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_TIME( m_flLastCounterMeleeTime );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCounterMelee::ActivateGoal
//
//	PURPOSE:	Activate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCounterMelee::ActivateGoal()
{
	super::ActivateGoal();

	m_pAI->GetAIBlackBoard()->SetBBUpdateTargetAim( false);

	// Store the time of this melee attack to avoid duplicate responses to 
	// a single event.

	CAIWMFact* pFact = GetCounterMeleeDesireFact( m_pAI );
	if ( pFact )
	{
		m_flLastCounterMeleeTime = pFact->GetTime();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCounterMelee::DeactivateGoal
//
//	PURPOSE:	Deactivate the goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCounterMelee::DeactivateGoal()
{
	super::DeactivateGoal();

	m_pAI->GetAIBlackBoard()->SetBBUpdateTargetAim( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCounterMelee::CalculateGoalRelevance
//
//	PURPOSE:	Calculate the current goal relevance.
//
// ----------------------------------------------------------------------- //

void CAIGoalCounterMelee::CalculateGoalRelevance()
{
	// Already countering a melee.

	if( m_pAI->GetGoalMgr()->IsCurGoal( this ) )
	{
		if (!IsWSSatisfied(m_pAI->GetAIWorldState()))
		{
			m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
			return;
		}
	}

	// Fail if the AI has a ranged weapon.  AIs can only block with melee 
	// weapons. BJL: Do we want to check having a weapon, or the current 
	// weapon?

	if ( AIWeaponUtils::HasWeaponType( m_pAI, kAIWeaponType_Ranged, CHECK_HOLSTER ) )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// AI knows about an incoming melee blow.

	bool bLogging = false;
	CAIWMFact* pFact = GetCounterMeleeDesireFact( m_pAI );
	if ( !pFact )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	// Fail if this desire is not newer than a attack the AI already responded 
	// to.

	if ( pFact->GetTime() <= m_flLastCounterMeleeTime )
	{
		m_fGoalRelevance = 0.0f;
		return;
	}

	ENUM_RangeStatus eEnemyRangeStatus = kRangeStatus_Invalid;
	ENUM_AIWeaponType eAIWeaponType = kAIWeaponType_None;
	if ( m_pAI->HasTarget( kTarget_Character ) )
	{
		CCharacter* pEnemy = CCharacter::DynamicCast( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
		if ( pEnemy )
		{
			const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord( pEnemy->GetArsenal()->GetCurWeaponRecord(), m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );
			if ( pRecord )
			{
				eEnemyRangeStatus = AIWeaponUtils::GetWeaponRangeStatus( pRecord, m_pAI->GetAIBlackBoard()->GetBBTargetPosition(), m_pAI->GetPosition() );
				eAIWeaponType = pRecord->eAIWeaponType;
			}
		}
	}

	// This is really a diagnostic check.  The goalmgr enforces this failure, 
	// but we need to report it for now.

	if ( m_pGoalRecord->fInterruptPriority == 0 
		&& m_pAI->GetAnimationContext()->IsLocked() )
	{
		AITRACE( AIShowBlocking, ( m_pAI->m_hObject, "CAIGoalCounterMelee : Cannot interrupt this animation." ) );
		m_fGoalRelevance = 0.f;
		return;
	}

	// Fail if the enemy is not in range
	
	if ( eEnemyRangeStatus != kRangeStatus_Ok )
	{
		AITRACE( AIShowBlocking, ( m_pAI->m_hObject, "CAIGoalCounterMelee : Enemy is out of range." ) );
		m_fGoalRelevance = 0.f;
		return;
	}

	// Fail if the enemy does not have a melee weapon

	if ( eAIWeaponType != kAIWeaponType_Melee )
	{
		AITRACE( AIShowBlocking, ( m_pAI->m_hObject, "CAIGoalCounterMelee : Enemy is using a ranged weapon." ) );
		m_fGoalRelevance = 0.f;
		return;
	}

	// Success!

	AITRACE( AIShowBlocking, ( m_pAI->m_hObject, "CAIGoalCounterMelee : Attempt to plan a counter move." ) );
	m_fGoalRelevance = m_pGoalRecord->fIntrinsicRelevance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCounterMelee::SetWSSatisfaction
//
//	PURPOSE:	Set the WorldState satisfaction conditions.
//
// ----------------------------------------------------------------------- //

void CAIGoalCounterMelee::SetWSSatisfaction( CAIWorldState& WorldState )
{
	WorldState.SetWSProp( kWSK_ReactedToWorldStateEvent, NULL, kWST_ENUM_AIWorldStateEvent, kWSE_IncomingMeleeAttack );
}
