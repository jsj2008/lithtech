// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstractSearch.cpp
//
// PURPOSE : AIGoalAbstractSearch implementation
//
// CREATED : 8/20/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalAbstractSearch.h"
#include "AIGoalMgr.h"
#include "AIHumanState.h"
#include "AIHuman.h"
#include "AI.h"
#include "AIUtils.h"
#include "AIRegion.h"
#include "Attachments.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractSearch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAbstractSearch::CAIGoalAbstractSearch()
{
	m_bSearch			= LTFALSE;
	m_bFaceSearch		= LTTRUE;
	m_bEngageSearch		= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractSearch::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractSearch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_BOOL(m_bSearch);
	SAVE_BOOL(m_bFaceSearch);
	SAVE_BOOL(m_bEngageSearch);

	HOBJECT hRegion;
	AISEARCHREGION_LIST::iterator it;
	SAVE_DWORD( m_lstRegions.size() );
	for( it = m_lstRegions.begin(); it != m_lstRegions.end(); ++it )
	{
		hRegion = *it;
		SAVE_HOBJECT( hRegion );
	}
}

void CAIGoalAbstractSearch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_BOOL(m_bSearch);
	LOAD_BOOL(m_bFaceSearch);
	LOAD_BOOL(m_bEngageSearch);

	uint32 cRegions;
	HOBJECT hRegion;
	LOAD_DWORD( cRegions );
	m_lstRegions.reserve( cRegions );
	for( uint32 iRegion=0; iRegion < cRegions; ++iRegion )
	{
		LOAD_HOBJECT( hRegion );
		m_lstRegions.push_back( hRegion );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractSearch::DeactivateGoal
//
//	PURPOSE:	Deactivate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractSearch::DeactivateGoal()
{
	super::DeactivateGoal();

	// Detach search light attachment from Light socket.
	// (e.g. for ninjas with search lanterns).

	if( m_pAI->GetBrain()->GetAIDataExist( kAIData_DisposeLantern ) )
	{
		char szDetach[128];
		sprintf(szDetach, "%s Light", KEY_DETACH );		
		SendTriggerMsgToObject( m_pAI, m_pAI->m_hObject, LTFALSE, szDetach);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractSearch::SetStateSearch
//
//	PURPOSE:	Set state to search, and set state parameters.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractSearch::SetStateSearch()
{
	// Reset default senses, except for SeeAllyDisturbance.

	m_pAI->SetCurSenseFlags( 0xffffffff & ~kSense_SeeAllyDisturbance );

	// Cannot search without a weapon.

	CAIHuman* pAIHuman = (CAIHuman*)m_pAI;
	if( !pAIHuman->GetCurrentWeapon() )
	{
		m_pGoalMgr->LockGoal(this);
		m_pAI->SetState( kState_HumanDraw );
		return;
	}

	// Start searching.

	m_pAI->SetState( kState_HumanSearch );
	if( IsCharacter( m_hStimulusSource ) )
	{
		m_pAI->Target(m_hStimulusSource);
	}

	m_pGoalMgr->LockGoal(this);

	CAIHumanStateSearch* pSearchState = (CAIHumanStateSearch*)m_pAI->GetState();
	if( pSearchState )
	{
		pSearchState->SetPause(LTTRUE);
		pSearchState->SetEngage(m_bEngageSearch);
		pSearchState->SetFace(m_bFaceSearch);

		// If any optional search regions have been specified, search them.
		// Otherwise, search the AIs current region.

		if( !m_lstRegions.empty() )
		{
			AIRegion* pRegion = (AIRegion*)g_pLTServer->HandleToObject( m_lstRegions.front() );
			if( pRegion )
			{
				AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Searching region %s", pRegion->GetName() ) );
			}

			pSearchState->SetDestRegion( m_lstRegions.front() );
			m_lstRegions.erase( m_lstRegions.begin() );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractSearch::HandleStateSearch
//
//	PURPOSE:	Determine what to do when in state Search.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractSearch::HandleStateSearch()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedEngage:
			m_pGoalMgr->UnlockGoal(this);
			m_pAI->SetState( kState_HumanAware );
			break;

		case kSStat_StateComplete:
			if( !m_lstRegions.empty() )
			{
				m_pAI->SetState( kState_HumanIdle );
				SetStateSearch();
			}
			else {
				m_pGoalMgr->UnlockGoal(this);
				m_pAI->SetState( kState_HumanAware );
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAbstractSearch::HandleStateSearch: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractSearch::HandleStateDraw
//
//	PURPOSE:	Determine what to do when in state Draw.
//
// ----------------------------------------------------------------------- //

void CAIGoalAbstractSearch::HandleStateDraw()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_FailedComplete:
			m_pGoalMgr->UnlockGoal( this );
			m_fCurImportance = 0.f;
			break;

		case kSStat_StateComplete:
			SetStateSearch();
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalAbstractSearch::HandleStateDraw: Unexpected State Status.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstractSearch::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalAbstractSearch::HandleNameValuePair(const char *szName, const char *szValue)
{
	ASSERT(szName && szValue);

	if ( !_stricmp(szName, "SEARCH") )
	{
		m_bSearch = IsTrueChar(*szValue);
		return LTTRUE;
	}
	else if ( !_stricmp(szName, "FACESEARCH") )
	{
		m_bFaceSearch = IsTrueChar(*szValue);
		return LTTRUE;
	}
	else if ( !_stricmp(szName, "ENGAGESEARCH") )
	{
		m_bEngageSearch = IsTrueChar(*szValue);
		return LTTRUE;
	}

	return LTFALSE;
}
