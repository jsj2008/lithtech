// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChase.cpp
//
// PURPOSE : AIGoalChase implementation
//
// CREATED : 6/21/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalChase.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AIStimulusMgr.h"
#include "AISenseRecorderAbstract.h"
#include "AIVolume.h"
#include "AIVolumeNeighbor.h"
#include "AIHuman.h"
#include "AITarget.h"
#include "PlayerObj.h"
#include "AIUtils.h"
#include "AIPathMgr.h"
#include "AIVolumeMgr.h"
#include "CharacterMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalChase, kGoal_Chase);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalChase::CAIGoalChase()
{
	m_pJunctionVolume		= LTNULL;
	m_pJunctionActionVolume	= LTNULL;
	m_pLastVolume			= LTNULL;
	m_bGiveUpChase			= LTFALSE;
	m_bSeekTarget			= LTFALSE;
	m_bNeverGiveUp			= LTFALSE;
	m_bKeepDistance			= LTTRUE;
	m_bLost					= LTFALSE;
	m_bContinueLost			= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_COBJECT( m_pJunctionVolume );
	SAVE_COBJECT( m_pJunctionActionVolume );
	SAVE_COBJECT( m_pLastVolume );

	JunctionRecord* pRec;
	SAVE_DWORD( m_stkJunctions.size() );
	JunctionStack::iterator it;
	for( it = m_stkJunctions.begin(); it != m_stkJunctions.end(); ++it )
	{
		pRec = &(*it);
		SAVE_COBJECT(pRec->pVolume);
		SAVE_BYTE(pRec->mskActionVolumes)
	}

	SAVE_BOOL(m_bGiveUpChase);
	SAVE_BOOL(m_bSeekTarget);
	SAVE_BOOL(m_bNeverGiveUp);
	SAVE_BOOL(m_bKeepDistance);
	SAVE_BOOL(m_bLost);
	SAVE_BOOL(m_bContinueLost);
}

void CAIGoalChase::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_COBJECT( m_pJunctionVolume, AIVolume );
	LOAD_COBJECT( m_pJunctionActionVolume, AIVolume );
	LOAD_COBJECT( m_pLastVolume, AIVolume );

	uint32 cJunctionRecords;
	LOAD_DWORD( cJunctionRecords );
	m_stkJunctions.resize( cJunctionRecords );

	JunctionRecord* pRec;
	JunctionStack::iterator it;
	for( it = m_stkJunctions.begin(); it != m_stkJunctions.end(); ++it )
	{
		pRec = &(*it);
		LOAD_COBJECT( pRec->pVolume, AIVolume );
		LOAD_BYTE(pRec->mskActionVolumes)
	}

	LOAD_BOOL(m_bGiveUpChase);
	LOAD_BOOL(m_bSeekTarget);
	LOAD_BOOL(m_bNeverGiveUp);
	LOAD_BOOL(m_bKeepDistance);
	LOAD_BOOL(m_bLost);
	LOAD_BOOL(m_bContinueLost);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::ActivateGoal()
{
	super::ActivateGoal();

	if( m_pAI->GetState()->GetStateType() == kState_HumanChase )
	{
		return;
	}

	// If seeking target, find a player and use as stimulus.

	if( m_bSeekTarget )
	{
		CPlayerObj* pPlayer = g_pCharacterMgr->FindRandomPlayer();
		if( pPlayer )
		{
			m_hStimulusSource = pPlayer->m_hObject;
		}
	}

	ASSERT(m_hStimulusSource != LTNULL);

	// Draw a weapon if necessary.

	if( !m_pAI->GetPrimaryWeapon() )
	{
		AIASSERT( m_pAI->HasHolsterString(), m_pAI->m_hObject, "CAIGoalChase::ActivateGoal: AI has no current weapon or holster string." );
		m_pAI->SetState( kState_HumanDraw );
		return;
	}

	SetStateChase( LTFALSE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::DeactivateGoal()
{
	super::DeactivateGoal();
	m_bLost = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::SetStateChase
//
//	PURPOSE:	Setup the chase state.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::SetStateChase(LTBOOL bClearState)
{
	// Forget any memory of junction volumes.
	m_stkJunctions.clear();
	m_bLost = LTFALSE;
	m_bGiveUpChase = LTFALSE;

	// Unlock goal in case it was locked while lost.
	// Keep it unlocked so that AI may be attracted to
	// vantage, cover, and view nodes.

	if( m_pGoalMgr->IsGoalLocked(this) )
	{
		m_pGoalMgr->UnlockGoal(this);
	}

	// Ignore senses other than SeeEnemy.
	m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );

	if( bClearState )
	{
		m_pAI->ClearAndSetState( kState_HumanChase );
	}
	else {
		m_pAI->SetState( kState_HumanChase );
	}

	CAIHumanStateChase* pStateChase = (CAIHumanStateChase*)m_pAI->GetState();
	pStateChase->KeepDistance( m_bKeepDistance );

	// Only seek target once. Reset through a command.
	// AI ignores junctions while seeking.

	if( m_bSeekTarget )
	{
		pStateChase->SeekTarget( LTTRUE );

		if( !m_bNeverGiveUp )
		{
			m_bSeekTarget = LTFALSE;
		}
	}

	m_pAI->Target(m_hStimulusSource);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanChase:
			HandleStateChase();
			break;

		case kState_HumanGoto:
			HandleStateGoto();
			break;

		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanAware:
			HandleStateAware();
			break;

		case kState_HumanLookAt:
			HandleStateLookAt();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// Unexpected State.
		default: AIASSERT( 0, m_pAI->m_hObject, "CAIGoalChase::UpdateGoal: Unexpected State." );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleStateDraw
//
//	PURPOSE:	Determine what to do when in state Draw.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::HandleStateDraw()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			SetStateChase( LTFALSE );
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalChase::HandleStateDraw: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleStateChase
//
//	PURPOSE:	Determine what to do when in state Chase.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::HandleStateChase()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Pursue:
			// Ignore senses other than SeeEnemy.
			m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );
			break;

		case kSStat_Lost:
			// Reset default senses, except for SeeAllyDisturbance.
			m_pAI->SetCurSenseFlags( 0xffffffff & ~kSense_SeeAllyDisturbance );
			break;

		case kSStat_Retry:
			// Reset default senses, except for SeeAllyDisturbance.
			m_pAI->SetCurSenseFlags( 0xffffffff & ~kSense_SeeAllyDisturbance );
			break;

		case kSStat_Junction:
			// Ignore senses other than SeeEnemy.
			m_pAI->SetCurSenseFlags( kSense_SeeEnemy | kSense_SeeDangerousProjectile | kSense_SeeCatchableProjectile );
			HandleJunctionVolume();
			break;

		// Try the next unchecked path from a junction.
		case kSStat_FailedComplete:
			AwareOnce();
			break;

		case kSStat_StateComplete:
//			m_bGiveUpChase = LTTRUE;
//			AwareOnce();
			m_fCurImportance = 0.f;
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalChase::HandleStateChase: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::SetChaseStatusLost
//
//	PURPOSE:	Set the Chase state to the Lost status.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::SetChaseStatusLost()
{
	m_pAI->SetState( kState_HumanChase );
	CAIHumanStateChase* pStateChase = (CAIHumanStateChase*)m_pAI->GetState();
	pStateChase->SetStateStatus( kSStat_Lost );

	pStateChase->SetLastVolume( m_pLastVolume );

	pStateChase->SetJunctionActionVolume( m_pJunctionActionVolume );

	pStateChase->SetJunctionVolume( m_pJunctionVolume );

	m_pGoalMgr->LockGoal(this);

	// Reset default senses, except for SeeAllyDisturbance.

	m_pAI->SetCurSenseFlags( 0xffffffff & ~kSense_SeeAllyDisturbance );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleJunctionVolume
//
//	PURPOSE:	Determine what to do when in a junction volume.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::HandleJunctionVolume()
{
	m_bLost = LTTRUE;

	AIVolume* pLastVolume;
	AIVolume* pCorrectVolume = LTNULL;
	AIVolume* pJunctionActionVolume;
	AIVolumeJunction* pJunctionVolume;

	// If the current state is chase or search, get the new junction volume info.
	switch( m_pAI->GetState()->GetStateType() )
	{
		case kState_HumanChase:
			{
				CAIHumanStateChase* pStateChase = (CAIHumanStateChase*)m_pAI->GetState();
				pLastVolume = pStateChase->GetLastVolume();
				pCorrectVolume = pStateChase->GetJunctionCorrectVolume();
				m_pLastVolume = pLastVolume;
				pJunctionVolume = (AIVolumeJunction*)pStateChase->GetJunctionVolume();
				m_pJunctionVolume = pJunctionVolume;
			}
			break;

		case kState_HumanSearch:
			{
				CAIHumanStateSearch* pStateSearch = (CAIHumanStateSearch*)m_pAI->GetState();
				pLastVolume = pStateSearch->GetLastVolume();
				m_pLastVolume = pLastVolume;
				pJunctionVolume = (AIVolumeJunction*)pStateSearch->GetJunctionVolume();
				m_pJunctionVolume = pJunctionVolume;
			}
			break;

		default:
			pLastVolume = m_pLastVolume;
			pJunctionVolume = (AIVolumeJunction*)m_pJunctionVolume;
			break;
	}

	// Ensure we have a last volume.  If not, give up.
	if( !m_pLastVolume )
	{
		m_bGiveUpChase = LTTRUE;
		AwareOnce();
		return;
	}

	JunctionAction eJunctionAction;

	// Record this junction in the junction memory stack, or move it to the top of the stack.

	JunctionRecord* pRec = HandleJunctionStack();

	// No actions are left for this junction.

	if( (pRec == LTNULL) || (pRec->mskActionVolumes == 0) ||
		( !(pJunctionVolume->GetAction(m_pAI, &(pRec->mskActionVolumes), pLastVolume, pCorrectVolume, &pJunctionActionVolume, &eJunctionAction) ) ) )
	{
		AITRACE(AIShowJunctions, ( m_pAI->m_hObject, "Exhuasted junction %s\n",
				 pJunctionVolume->GetName() ) );

		m_bGiveUpChase = LTTRUE;
		AwareOnce();
		return;
	}

	m_pJunctionActionVolume = pJunctionActionVolume;


	// Always peek into the correct volume before entering.

	if( m_pJunctionActionVolume == pCorrectVolume )
	{
		AIVolumeNeighbor* pNeighborLast = g_pAIVolumeMgr->FindNeighbor( m_pAI, m_pLastVolume, m_pAI->GetLastVolume() );
		AIVolumeNeighbor* pNeighborNext = g_pAIVolumeMgr->FindNeighbor( m_pAI, m_pAI->GetLastVolume(), m_pJunctionActionVolume );
		if( pNeighborLast && pNeighborNext )
		{
			if( pNeighborLast->GetConnectionDir() != -pNeighborNext->GetConnectionDir() )
			{
				eJunctionAction = eJunctionActionPeek;
			}
		}
	}


	switch ( eJunctionAction )
	{
		case eJunctionActionSearch:
			AITRACE(AIShowJunctions, ( m_pAI->m_hObject, "Junction search into %s\n",
					pJunctionActionVolume->GetName() ) );
			{
				m_pAI->SetState( kState_HumanGoto );
				CAIHumanStateGoto* pStateGoto = (CAIHumanStateGoto*)m_pAI->GetState();

				// If the action volume is a door volume, run through it to the
				// following volume.

				AIVolume* pDestVolume = LTNULL;
				if( !pJunctionActionVolume->HasDoors() )
				{
					pDestVolume = pJunctionActionVolume;
				}
				else {
					AIVolumeNeighbor* pNeighbor;
					uint32 cNeighbors = pJunctionActionVolume->GetNumNeighbors();
					for( uint32 iNeighbor=0; iNeighbor < cNeighbors; ++iNeighbor )
					{
						pNeighbor = pJunctionActionVolume->GetNeighborByIndex( iNeighbor );
						if( pNeighbor->GetVolume() != pJunctionVolume )
						{
							pDestVolume = pNeighbor->GetVolume();
							break;
						}
					}
				}

				if( pDestVolume )
				{
					pStateGoto->SetDest( pDestVolume->GetCenter() );
					pStateGoto->SetMovement( kAP_Run );
				}
				else {
					// For some reason, the action volume was a door volume
					// connected to no other volumes!!

					AITRACE(AIShowJunctions, ( m_pAI->m_hObject, "Junction trying to search into a door volume with no neighbors!\n" ) );
					HandleJunctionVolume();
					return;
				}
			}
			break;

		// Look into a hallway or room.
		case eJunctionActionPeek:
			AITRACE(AIShowJunctions, ( m_pAI->m_hObject, "Junction peek into %s\n",
					pJunctionActionVolume->GetName() ) );
			{
				AIVolumeNeighbor* pNeighbor = g_pAIVolumeMgr->FindNeighbor( m_pAI, m_pLastVolume, m_pAI->GetLastVolume() );
				if( pNeighbor )
				{
					m_pAI->FaceDir( -pNeighbor->GetConnectionPerpDir() );
				}

				m_pAI->SetState( kState_HumanLookAt );
				CAIHumanStateLookAt* pLookAtState = (CAIHumanStateLookAt*)m_pAI->GetState();
				pLookAtState->SetPos( pJunctionActionVolume->GetCenter() );
				pLookAtState->SetAISound( kAIS_Search );

				// If peaking into the correct volume (where the target really is)
				// pause first.

				if( m_pJunctionActionVolume == pCorrectVolume )
				{
					pLookAtState->SetPause( LTTRUE );
				}
			}
			break;

		// Run through this junction.
		case eJunctionActionContinue:
			AITRACE(AIShowJunctions, ( m_pAI->m_hObject, "Junction continue into %s\n",
					pJunctionActionVolume->GetName() ) );

			// If continuing into the correct volume (where the target really is)
			// pause first.

			if( m_pJunctionActionVolume == pCorrectVolume )
			{
				AwareOnce();
				m_bContinueLost = LTTRUE;
			}
			else {
				SetChaseStatusLost();
			}
			break;

		case eJunctionActionNothing:
			AITRACE(AIShowJunctions, ( m_pAI->m_hObject, "Junction has no JunctionAction!\n" ) );
			HandleJunctionVolume();
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleJunctionStack
//
//	PURPOSE:	Keep track of junctions visited.
//
// ----------------------------------------------------------------------- //

JunctionRecord* CAIGoalChase::HandleJunctionStack()
{
	// Look for a record for this junction volume in the stack.

	JunctionRecord jrRec;
	JunctionStack::iterator it;
	for(it = m_stkJunctions.begin(); it != m_stkJunctions.end(); ++it)
	{
		// If a record is found, move it to the back.

		if( it->pVolume == m_pJunctionVolume )
		{
			jrRec.pVolume = it->pVolume;
			jrRec.mskActionVolumes = it->mskActionVolumes;
			m_stkJunctions.erase(it);

			m_stkJunctions.push_back(jrRec);
			return &(m_stkJunctions.back());
		}
	}

	// Create a new record.

	AIVolumeJunction* pJunctionVolume = (AIVolumeJunction*)m_pJunctionVolume;
	AIVolume* pLastVolume = (AIVolume*)m_pLastVolume;

	jrRec.pVolume = m_pJunctionVolume;
	pJunctionVolume->RecordActionVolumeMask(pLastVolume, &(jrRec.mskActionVolumes));
	if(jrRec.mskActionVolumes)
	{
		m_stkJunctions.push_back(jrRec);
		return &(m_stkJunctions.back());
	}
	else {
		return LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::AwareOnce
//
//	PURPOSE:	Look around suspiciously, and then do something else.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::AwareOnce()
{
	// This function just makes the AI animate one Aware loop.
	// When the Aware state completes, it tries the next junction option or searches.

	// Reset default senses, except for SeeAllyDisturbance.

	m_pAI->SetCurSenseFlags( 0xffffffff & ~kSense_SeeAllyDisturbance );

	// Look around once.

	m_pAI->SetState( kState_HumanAware );
	CAIHumanStateAware* pStateAware = (CAIHumanStateAware*)m_pAI->GetState();
	pStateAware->SetPlayOnce( LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::GiveUpChase
//
//	PURPOSE:	Go Aware until goals importance decays to zero.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::GiveUpChase(LTBOOL bSearch)
{
	m_bGiveUpChase = LTTRUE;

	// Search or go aware.

	if( bSearch && m_pAI->CanSearch() )
	{
		SetStateSearch();
	}
	else
	{
		if( m_pGoalMgr->IsGoalLocked(this) )
		{
			m_pGoalMgr->UnlockGoal(this);
		}

		m_fCurImportance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleStateLookAt
//
//	PURPOSE:	Determine what to do when in state LookAt.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::HandleStateLookAt()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			{
				// Try again with a different action for the junction.
				HandleJunctionVolume();
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalChase::HandleStateLookAt: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleStateAware
//
//	PURPOSE:	Determine what to do when in state Aware.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::HandleStateAware()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		// The state will only complete if SetPlayOnce is set to LTTRUE.
		// Otherwise, it loops until the goal deactivates.
		case kSStat_StateComplete:
			{
				if( m_bContinueLost )
				{
					m_bContinueLost = LTFALSE;
					SetChaseStatusLost();
					return;
				}

				if( m_bGiveUpChase )
				{
					GiveUpChase(LTTRUE);
				}
				else {

					// Make sure we have junctions.
					if( m_stkJunctions.size() == 0 )
					{
						GiveUpChase( LTTRUE );
					}

					// Try another option from the current junction.

					else if( m_pAI->GetLastVolume() && ( m_pAI->GetLastVolume() == m_stkJunctions.back().pVolume ) )
					{
						HandleJunctionVolume();
					}

					// Go back to the last junction, and try unchecked actions.

					else {
						GiveUpChase( LTTRUE );
						/***
						m_pGoalMgr->LockGoal(this);
						m_pAI->SetState( kState_HumanChase );
						CAIHumanStateChase* pStateChase = (CAIHumanStateChase*)m_pAI->GetState();
						AIVolumeJunction* pVolume = (AIVolumeJunction*)m_stkJunctions.back().pVolume;
						pStateChase->SetJunctionVolume( pVolume );
						pStateChase->SetStateStatus( kSStat_Retry );
						***/
					}
				}
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalChase::HandleStateAware: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalGoto::HandleStateGoto
//
//	PURPOSE:	Determine what to do when in state Goto.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::HandleStateGoto()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_StateComplete:
			if( m_pAI->CanSearch() )
			{
				SetStateSearch();
			}
			else {
				AwareOnce();
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalChase::HandleStateGoto: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::SetStateSearch
//
//	PURPOSE:	Set state to search, and set state parameters.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::SetStateSearch()
{
	super::SetStateSearch();

	// Reset default senses, except for SeeAllyDisturbance.

	m_pAI->SetCurSenseFlags( 0xffffffff & ~kSense_SeeAllyDisturbance );

	CAIHumanStateSearch* pSearchState = (CAIHumanStateSearch*)m_pAI->GetState();
	pSearchState->IgnoreJunctions( m_bGiveUpChase );
	pSearchState->SetPause(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleStateSearch
//
//	PURPOSE:	Determine what to do when in state Search.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::HandleStateSearch()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Junction:
			HandleJunctionVolume();
			break;

		case kSStat_StateComplete:
			if( m_bGiveUpChase )
			{
				GiveUpChase(LTFALSE);
			}
			else {
				AwareOnce();
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalChase::HandleStateSearch: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::RecalcImportance
//
//	PURPOSE:	Lose interest in chasing if we're not alert.
//
// ----------------------------------------------------------------------- //

void CAIGoalChase::RecalcImportance()
{
	// Lose interest in chasing if the AI has lost track of his target.
	// Chase normally does not lose importance, because we want to ensure
	// the AI does not lose interest in an enemy when they go out of sight,
	// or the AI goes into cover.
	// If the AI is not attacking, and has fallen back to searching, the
	// AI should only start chasing again if new SeeEnemy stimulus comes in.

	if( ( !m_pGoalMgr->IsCurGoal(this) ) &&
		( !m_bSeekTarget ) &&
		( !m_bNeverGiveUp ) &&
		( m_fCurImportance > 0.f ) &&
		( m_pAI->GetAwareness() != kAware_Alert ) )
	{
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalChase: Setting importance to 0.0" ) );
		m_fCurImportance = 0.f;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalChase::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "SEEKPLAYER") )
	{
		if( IsTrueChar( szValue[0] ) )
		{
			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalChase: SEEKPLAYER=1" ) );

			// Activate this goal, and seek out the target.

			SetCurToBaseImportance();
			m_bSeekTarget = LTTRUE;

			// Update the alarm level with new stimulus.

			AIBM_Stimulus* pStimulus = g_pAIButeMgr->GetStimulus( kStim_EnemyVisible );
			if( pStimulus )
			{
				m_eSenseType = pStimulus->eSenseType;
				m_pAI->IncrementAlarmLevel( pStimulus->nAlarmLevel );
			}
		}

		return LTTRUE;
	}

	else if ( !_stricmp(szName, "NEVERGIVEUP") )
	{
		if( IsTrueChar( szValue[0] ) )
		{
			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalChase: NEVERGIVEUP=1" ) );
			m_bNeverGiveUp = LTTRUE;
		}
	}

	else if ( !_stricmp(szName, "KEEPDISTANCE") )
	{
		m_bKeepDistance = IsTrueChar( szValue[0] );
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalChase: KEEPDISTANCE=%d", m_bKeepDistance ? 1 : 0 ) );

		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::SelectTriggeredAISound
//
//	PURPOSE:	Select appropriate AI sound.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalChase::SelectTriggeredAISound(EnumAISoundType* peAISoundType)
{
	// Chase state overrides dialogue.
	// Let other goals (investigate) play dialogue when AI is in other states (e.g. search).

	if( m_pAI->GetState()->GetStateType() == kState_HumanChase )
	{
		*peAISoundType = kAIS_None;
		return LTTRUE;
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalChase::HandleSense
//
//	PURPOSE:	React to a sense.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalChase::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord)
{
	if( super::HandleGoalSenseTrigger(pSenseRecord) )
	{
		CAIHuman* pAIHuman = (CAIHuman*)m_pAI;

		if( ( pSenseRecord->eSenseType == kSense_SeeEnemy ) &&
			( m_pAI->GetPrimaryWeapon() || pAIHuman->HasHolsterString() ) )
		{
			m_pAI->Target(m_hStimulusSource);
			if( m_pAI->HasTarget() && m_pAI->GetTarget()->GetCharacter()->HasLastVolume() )
			{
				// Handle AIs that only use the preferred paths.

				if( m_pAI->GetBrain()->GetAIDataExist( kAIData_MinPathWeight ) )
				{
					LTFLOAT fMinPathWeight = m_pAI->GetBrain()->GetAIData( kAIData_MinPathWeight );
					AIVolume* pVolumeLast = m_pAI->GetTarget()->GetCharacter()->GetLastVolume();
					if( ( fMinPathWeight > 0.f ) && ( pVolumeLast->GetPathWeight( LTTRUE, LTFALSE ) > fMinPathWeight ) )
					{
						return LTFALSE;
					}
				}

				// jeffo 5/14/02
				// Removed Brain range status check.  State should take care of not pathing
				// if too close to the target, but goal should always be set to base importance
				// when the target is seen.  This way chase stays resident as a fall-back during
				// combat.

				return LTTRUE;
			}
		}
		else if( m_pGoalMgr->IsCurGoal( this ) &&
				 ( !m_bNeverGiveUp ) &&
				 ( pSenseRecord->eSenseType != kSense_SeeEnemy ) )
		{
			// If the sense is something other than SeeEnemy, stop chasing.
			// If an investigate goal is present, it will take over.

			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "CAIGoalChase: Sensed something, setting importance to 0.0" ) );
			m_fCurImportance = 0.f;
		}
	}

	return LTFALSE;
}

