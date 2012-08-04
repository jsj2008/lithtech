// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalRespondToAlarm.cpp
//
// PURPOSE : AIGoalRespondToAlarm implementation
//
// CREATED : 11/07/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalRespondToAlarm.h"
#include "AIGoalMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIHumanState.h"
#include "AIHuman.h"
#include "Alarm.h"
#include "AINodeMgr.h"
#include "AINodeGuard.h"
#include "AIVolume.h"
#include "AIRegion.h"
#include "AIStimulusMgr.h"
#include "AIVolumeMgr.h"
#include "AICentralKnowledgeMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalRespondToAlarm, kGoal_RespondToAlarm);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalRespondToAlarm::CAIGoalRespondToAlarm()
{
	m_hAlarm = NULL;
	m_eStimIDAlarm = kStimID_Unset;
	m_bBuildSearchList = LTTRUE;

	m_pResponseVolume = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToAlarm::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_VECTOR(m_vDest);
	SAVE_HOBJECT(m_hAlarm);
	SAVE_DWORD(m_eStimIDAlarm);
	SAVE_BOOL(m_bBuildSearchList);
	SAVE_COBJECT(m_pResponseVolume);
}

void CAIGoalRespondToAlarm::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_VECTOR(m_vDest);
	LOAD_HOBJECT(m_hAlarm);
	LOAD_DWORD_CAST(m_eStimIDAlarm, EnumAIStimulusID);
	LOAD_BOOL(m_bBuildSearchList);
	LOAD_COBJECT(m_pResponseVolume, AIVolume);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToAlarm::ActivateGoal()
{
	super::ActivateGoal();

	// Ensure preconditions are OK: there is an alarm and a region.

	Alarm* pAlarm = (Alarm*)g_pLTServer->HandleToObject( m_hAlarm );
	if( !pAlarm )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalRespondToAlarm::ActivateGoal: Could not find alarm." );
		return;
	}

	if ( ( !m_pAI->HasLastVolume() ) || ( !m_pAI->GetLastVolume()->HasRegion() ) )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalRespondToAlarm::ActivateGoal: AI must be in a region" );
		return;
	}

	AIRegion* pRegion = m_pAI->GetLastVolume()->GetRegion();
	if( !pRegion )
	{
		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalRespondToAlarm::ActivateGoal: Region is NULL" );
		return;
	}

	// Clear old alarm response knowledge.

	if( m_pResponseVolume )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_AlarmResponseVolume, m_pAI, m_pResponseVolume );
	}

	// Only go aware without a weapon.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( !( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
	{
		m_pAI->SetState( kState_HumanAware );
	}

	// Determine state based on which alarm group contains the region.

	else if( pAlarm->IsRegionInRespondGroup( pRegion->m_hObject ) )
	{
		m_pAI->SetState( kState_HumanGoto );
		CAIHumanStateGoto* pGoto = (CAIHumanStateGoto*)m_pAI->GetState();
		pGoto->SetDest( m_vDest );
		pGoto->SetMovement( kAP_Run );
		pGoto->SetWeaponPosition( kAP_Up );

		// Find volume containing alarm.

		AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume( m_pAI->m_hObject, m_vDest, eAxisAll, m_pAI->GetDims().y*2.0f, (AISpatialRepresentation*)m_pAI->GetLastVolume() );
		if( pVolume )
		{
			m_pResponseVolume = pVolume;
		}

		// Ignore senses other than SeeEnemy.
		m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );
	}

	else if( pAlarm->IsRegionInAlertGroup( pRegion->m_hObject ) )
	{
		m_pAI->SetState( kState_HumanAware );
	}

	else {
		AIASSERT( 0, m_pAI->m_hObject, "CAIGoalRespondToAlarm::ActivateGoal: AIs region is not in alarms Respond or Alert group!" );
		m_fCurImportance = 0.f;
		return;
	}

	m_pAI->SetAwareness( kAware_Alert );

	m_pGoalMgr->LockGoal(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToAlarm::DeactivateGoal()
{
	super::DeactivateGoal();

	// Clear old alarm response knowledge.

	if( m_pResponseVolume )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_AlarmResponseVolume, m_pAI, m_pResponseVolume );
		m_pResponseVolume = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToAlarm::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanGoto:
			HandleStateGoto();
			break;

		case kState_HumanAware:

			// Keep goal locked while alarm is still sounding.

			if( m_pGoalMgr->IsGoalLocked( this ) 
				&& ( !g_pAIStimulusMgr->StimulusExists( m_eStimIDAlarm ) ) )
			{
				m_pGoalMgr->UnlockGoal(this);
			}
			break;

		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalRespondToAlarm::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToAlarm::HandleStateGoto()
{
	// Start searching if we made it to the alarm.

	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			m_bBuildSearchList = LTTRUE;
			SetStateSearch();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalRespondToAlarm::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::HandleVolume*
//
//	PURPOSE:	Only allow 1 AI to respond in a volume at a time.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToAlarm::HandleVolumeEnter(AIVolume* pVolume)
{
	if( !m_pResponseVolume )
	{
		return;
	}

	// Only care if we're heading towards the disturbance.

	if( m_pAI->GetState()->GetStateType() != kState_HumanGoto )
	{
		return;
	}

	if( pVolume && ( pVolume == m_pResponseVolume ) )
	{
		// Someone is already responding in this volume.

		if( g_pAICentralKnowledgeMgr->CountTargetMatches( kCK_AlarmResponseVolume, m_pAI, m_pResponseVolume ) > 0 )
		{
			m_pResponseVolume = LTNULL;
			m_bBuildSearchList = LTTRUE;
			SetStateSearch();
			return;
		}
		
		// Register the first AI to respond in this volume.
		// Only one AI may respond in a volume, to prevent clumping.

		g_pAICentralKnowledgeMgr->RegisterKnowledge( kCK_AlarmResponseVolume, m_pAI, m_pResponseVolume, LTTRUE );
	}
}

// ----------------------------------------------------------------------- //

void CAIGoalRespondToAlarm::HandleVolumeExit(AIVolume* pVolume)
{
	if( !m_pResponseVolume )
	{
		return;
	}

	// Only care if we are leaving the alarm.

	if( m_pAI->GetState()->GetStateType() != kState_HumanSearch ) 
	{
		return;
	}

	if( pVolume && ( pVolume == m_pResponseVolume ) )
	{
		g_pAICentralKnowledgeMgr->RemoveKnowledge( kCK_AlarmResponseVolume, m_pAI, m_pResponseVolume );
		m_pResponseVolume = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::SetStateSearch
//
//	PURPOSE:	Set state to search, and set state parameters.
//
// ----------------------------------------------------------------------- //

void CAIGoalRespondToAlarm::SetStateSearch()
{
	if( m_bBuildSearchList )
	{
		m_bBuildSearchList = LTFALSE;

		Alarm* pAlarm = (Alarm*)g_pLTServer->HandleToObject( m_hAlarm );
		if( !pAlarm )
		{
			AIASSERT( 0, m_pAI->m_hObject, "CAIGoalRespondToAlarm::SetStateSearch: Could not find alarm." );
			return;
		}

		// Build list of regions to search.
	
		uint32 cSearchRegions = pAlarm->GetNumSearchRegions();
		m_lstRegions.clear();
		for( uint32 iRegion=0; iRegion < cSearchRegions; ++iRegion )
		{
			m_lstRegions.push_back( pAlarm->GetSearchRegion( iRegion) );
		}
	}

	super::SetStateSearch();

	if( m_pAI->GetState()->GetStateType() == kState_HumanSearch )
	{
		CAIHumanStateSearch* pStateSearch = (CAIHumanStateSearch*)m_pAI->GetState();
		pStateSearch->SetEngage( LTFALSE );
		pStateSearch->SetLimitSearchCount( LTFALSE );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalRespondToAlarm::HandleGoalSenseTrigger
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalRespondToAlarm::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		// Record stimulus ID.

		m_eStimIDAlarm = pSenseRecord->eLastStimulusID;

		// Record which alarm was activated.

		m_hAlarm = pSenseRecord->hLastStimulusTarget;

		// If there is an alarm UseObject node pointing
		// at the alarm object, use the node as the dest.

		AINode* pNode = g_pAINodeMgr->FindUseObjectNode( kNode_Alarm, m_hAlarm, LTTRUE );
		if( pNode )
		{
			m_vDest = pNode->GetPos();
		}
		else {
			g_pLTServer->GetObjectPos( m_hAlarm, &m_vDest );
		}

		return LTTRUE;
	}

	// Bail from the goal if other senses come thru.

	else if( ( m_pGoalMgr->IsCurGoal( this ) ) &&
			 ( m_pAI->GetState()->GetStateType() != kState_HumanGoto ) )
	{
		m_fCurImportance = 0.f;
	}

	return LTFALSE;
}

