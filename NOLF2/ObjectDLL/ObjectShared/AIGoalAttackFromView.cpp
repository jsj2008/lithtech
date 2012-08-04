// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromView.cpp
//
// PURPOSE : AIGoalAttackFromView implementation
//
// CREATED : 7/31/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAttackFromView.h"
#include "AIHumanState.h"
#include "AINodeMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "AIVolume.h"
#include "AIGoalButeMgr.h"
#include "AITarget.h"
#include "AIPathMgr.h"
#include "PlayerObj.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromView, kGoal_AttackFromView);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttackFromView::CAIGoalAttackFromView()
{
	m_bNodeBased	= LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromView::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalAttackFromView::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromView::ActivateGoal()
{
	super::ActivateGoal();

	AIASSERT(m_hNode != LTNULL, m_pAI->m_hObject, "CAIGoalAttackFromView::ActivateGoal: AINodeView is NULL.");

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	m_pGoalMgr->LockGoal(this);

	SetStateAttack();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::SetStateAttack
//
//	PURPOSE:	Set the node and search distance.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromView::SetStateAttack()
{
	m_pAI->SetCurrentWeapon( m_eWeaponType );

	if( m_pAI->GetState()->GetStateType() != kState_HumanAttackFromView )
	{
		AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( GetGoalType() );

		m_pAI->SetState( kState_HumanAttackFromView );

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject(m_hNode);
		AIASSERT( pNode, m_pAI->m_hObject, "CAIGoalAttackFromView::SetStateAttack: ViewNode is NULL." );

		CAIHumanStateAttackFromView* pState = (CAIHumanStateAttackFromView*)m_pAI->GetState();
		pState->SetNode(pNode);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromView::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanAttackFromView:
			HandleStateAttackFromView();
			break;

		case kState_HumanAware:
			break;

		case kState_HumanPanic:
			HandleStatePanic();
			break;

		case kState_HumanCharge:
			HandleStateCharge();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromView::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::HandleStateAttackFromView
//
//	PURPOSE:	Determine what to do when in state AttackFromView.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromView::HandleStateAttackFromView()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Moving:
			break;

		case kSStat_Attacking:
			if(m_pGoalMgr->IsGoalLocked(this))
			{
				m_pGoalMgr->UnlockGoal(this);
			}
			break;

		case kSStat_FailedComplete:
			m_hFailedNode = m_hNode;
			m_hNode = LTNULL;

			// Done attacking from view, so charge!
			// If there are other attractors (cover, view, vantage nodes),
			// a goal will re-activate instead of charging.

			m_pAI->SetState( kState_HumanCharge );
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromView::HandleStateAttackFromView: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::HandleStateCharge
//
//	PURPOSE:	Determine what to do when in state Charge.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackFromView::HandleStateCharge()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			if(m_pGoalMgr->IsGoalLocked(this))
			{
				m_pGoalMgr->UnlockGoal(this);
			}
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAttackFromView::HandleStateCharge: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::FindNearestAttractorNode
//
//	PURPOSE:	Find an attractor node.
//
// ----------------------------------------------------------------------- //

AINode* CAIGoalAttackFromView::FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr)
{
	// Only look for View nodes if the AI has already been alerted to the 
	// enemy's presence.

	if( !m_pAI->HasTarget() )
	{
		return LTNULL;
	}

	m_hStimulusSource = m_pAI->GetTarget()->GetObject();
	CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hStimulusSource);
	
	// Do not attack from view if target is hidden.

	if ( pCharacter->IsHidden() )
	{
		return LTNULL;
	}

	// Find target's current PlayerInfoVolume volume.

	AIInformationVolume *pInfoVolume = pCharacter->GetCurrentInformationVolume();
	if( ( !pInfoVolume ) || ( pInfoVolume->GetVolumeType() != AIInformationVolume::eTypePlayerInfo ) )
	{
		return LTNULL;
	}

	// Find view node corresponding to this playerinfo volume.

	AIVolumePlayerInfo* pPlayerInfoVolume = (AIVolumePlayerInfo*)pInfoVolume;
	AINode* pViewNode = pPlayerInfoVolume->FindViewNode(m_pAI, eNodeType, vPos, LTFALSE);
	if( !pViewNode )
	{
		return LTNULL;
	}

	// Only use a view node if there is no path to the target.

	if( pCharacter->HasCurrentVolume() )
	{
		if( g_pAIPathMgr->HasPath( m_pAI, pCharacter->GetCurrentVolume() ) )
		{
			// Bail if AI can see and path to enemy.

			if( m_pAI->GetTarget()->IsVisibleFromEye() )
			{
				m_fCurImportance = 0.f;
			}

			return LTNULL;
		}
	}

	return pViewNode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAttackFromView::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "SEEKPLAYER") )
	{
		if( IsTrueChar( szValue[0] ) )
		{
			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalAttackFromView: SEEKPLAYER=1" ) );

			CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
			if ( pPlayer )
			{
				m_hStimulusSource = pPlayer->m_hObject;
				m_pAI->Target( pPlayer->m_hObject );
			}
		}

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackFromView::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAttackFromView::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	// Do not call super (AIGoalAttack) because visibilty is not required.

	if( CAIGoalAbstractStimulated::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Check range.
		// Need a weapon to activate an attack goal.
		// AI is not already in this attack goal.

		m_pAI->Target(pSenseRecord->hLastStimulusSource);
		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
		if( pAIHuman->HasTarget() && pAIHuman->HasWeapon( m_eWeaponType ) )
		{
			// If stimulus has no duration (meaning it lasts forever, i.e. visibile 
			// stimuli like an open drawer), ignore if it occured before I 
			// returned to a relaxed state.

			if( pSenseRecord->pAIBM_Last_Stimulus && 
				( pSenseRecord->pAIBM_Last_Stimulus->fDuration == 0.f ) &&
				( m_pAI->GetLastRelaxedTime() > pSenseRecord->fLastStimulationTime ) )
			{
					return LTFALSE;
			}

			// Record whatever triggered this goal.
			m_hStimulusSource = pSenseRecord->hLastStimulusSource;

			HandleGoalAttractors();
			if(m_hNode)
			{
				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}
