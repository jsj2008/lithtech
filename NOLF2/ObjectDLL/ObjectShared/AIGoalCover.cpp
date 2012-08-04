// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCover.cpp
//
// PURPOSE : AIGoalCover implementation
//
// CREATED : 7/19/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalCover.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "AIGoalButeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalCover, kGoal_Cover);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalCover::CAIGoalCover()
{
	m_hNodeCover		= LTNULL;
	m_hLastNodeCover	= LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_HOBJECT( m_hNodeCover );
	SAVE_HOBJECT( m_hLastNodeCover );
}

void CAIGoalCover::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_HOBJECT( m_hNodeCover );
	LOAD_HOBJECT( m_hLastNodeCover );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hNodeCover != LTNULL, m_pAI->m_hObject, "CAIGoalCover::ActivateGoal: AINodeCover is NULL.");

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal(this);

	m_pAI->SetState( kState_HumanCover );

	CAIHumanStateCover* pState = (CAIHumanStateCover*)m_pAI->GetState();
	pState->SetCoverNode( m_hNodeCover );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::DeactivateGoal()
{
	super::DeactivateGoal();

	m_hLastNodeCover = m_hNodeCover;
	m_hNodeCover = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanCover:
			HandleStateCover();
			break;

		case kState_HumanAware:
			break;

		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalCover::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::HandleStateCover
//
//	PURPOSE:	Determine what to do when in state Cover.
//
// ----------------------------------------------------------------------- //

void CAIGoalCover::HandleStateCover()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_UseCover:
			break;

		case kSStat_StateComplete:
			{
				CAIHuman* pAIHuman = (CAIHuman*)m_pAI;

				// Search.
				if( m_pAI->CanSearch() && 
					( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
				{
					SetStateSearch();
				}
				else {
					// Go aware.
					m_pGoalMgr->UnlockGoal(this);
					m_pAI->SetState( kState_HumanAware );
				}
			}
			break;

		case kSStat_FailedComplete:
			m_pGoalMgr->UnlockGoal(this);
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalCover::HandleStateCover: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::HandleGoalAttractors
//
//	PURPOSE:	React to an attractor.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalCover::HandleGoalAttractors()
{
	AINode* pNode = LTNULL;

	// Do not use cover nodes without a weapon.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( !( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
	{
		return LTNULL;
	}

	if(m_hStimulusSource != LTNULL)
	{
		if( !m_pGoalMgr->IsCurGoal( this ) )
		{
			AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );
			AIASSERT(pTemplate->cAttractors > 0, m_pAI->m_hObject, "CAIGoalCover::HandleGoalAttractors: Goal has no attractors.");


			// If the AI was just shot, look 5x further for cover.
			// Maybe this should be buted.

			LTFLOAT fSearchFactor = 1.f;
			if( ( g_pLTServer->GetTime() - m_pAI->GetLastPainTime() ) < 1.f )
			{
				fSearchFactor = 5.f;
			}		

			// Lock the failed cover node, so that we don't try to use it again.
			BlockAttractorNodeFromSearch( m_hLastNodeCover );

			// Look for a cover node that covers from the threat.
			// If one is found, this goal activates.
			for(uint32 iAttractor=0; iAttractor < pTemplate->cAttractors; ++iAttractor)
			{
				pNode = g_pAINodeMgr->FindNearestNodeFromThreat(m_pAI, kNode_Cover, m_pAI->GetPosition(), m_hStimulusSource, fSearchFactor);
				if(pNode != LTNULL)	
				{
					m_hNodeCover = pNode->m_hObject;
					SetCurToBaseImportance();
					m_pAI->Target(m_hStimulusSource);
					m_hStimulusSource = LTNULL;
				}
				else {
					m_hNodeCover = LTNULL;
					m_fCurImportance = 0.f;
				}
			}

			// If we locked a node prior to the search, unlock it.
			UnblockAttractorNodeFromSearch( m_hLastNodeCover );
		}
	}

	return pNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::HandleDamage
//
//	PURPOSE:	Handle damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalCover::HandleDamage(const DamageStruct& damage)
{
	super::HandleDamage( damage );

	HandleGoalAttractors();
	if( m_hNodeCover )
	{
		// Activate the goal.

		SetCurToBaseImportance();
	}

	// Always return false to allow normal damage handling.

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalCover::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalCover::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// Do not use cover nodes without a ranged weapon.

	if( !( m_pAI->HasWeapon( kAIWeap_Ranged ) || 
		 ( m_pAI->GetHolsterWeaponType() == kAIWeap_Ranged ) ) )
	{
		return LTFALSE;
	}

	if(super::HandleGoalSenseTrigger(pSenseRecord))
	{
		m_pAI->IncrementAlarmLevel( pSenseRecord->nLastStimulusAlarmLevel );

		HandleGoalAttractors();
		if( m_hNodeCover )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

