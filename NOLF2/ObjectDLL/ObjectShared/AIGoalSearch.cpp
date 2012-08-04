// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSearch.cpp
//
// PURPOSE : AIGoalSearch implementation
//
// CREATED : 1/11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalSearch.h"
#include "AIGoalMgr.h"
#include "AIHuman.h"
#include "AIUtils.h"
#include "AIHumanState.h"
#include "AIRegion.h"
#include "AINode.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSearch, kGoal_Search);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalSearch::CAIGoalSearch()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearch::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalSearch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalSearch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearch::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearch::ActivateGoal()
{
	super::ActivateGoal();	

	m_pAI->SetAwareness( kAware_Suspicious );

	// Ensure that AIs alarm level is high enough that he will
	// decide to search if he is disturbed after the scripted 
	// search ends.

	if( m_pAI->GetAlarmLevel() < m_pAI->GetBrain()->GetMajorAlarmThreshold() ) 
	{
		m_pAI->SetAlarmLevel( m_pAI->GetBrain()->GetMajorAlarmThreshold() + 1 );
	}

	SetStateSearch();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearch::SetStateSearch
//
//	PURPOSE:	Set search state.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearch::SetStateSearch()
{
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
//	ROUTINE:	CAIGoalSearch::UpdateGoal
//
//	PURPOSE:	Update goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalSearch::UpdateGoal()
{
	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		case kState_HumanAware:
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalSearch::UpdateGoal: Unexpected State.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalSearch::HandleNameValuePair
//
//	PURPOSE:	Handles getting a name/value pair.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalSearch::HandleNameValuePair(const char *szName, const char *szValue)
{
	AIASSERT(szName && szValue, m_pAI->m_hObject, "CAIGoalSearch::HandleNameValuePair: Name or value is NULL.");

	if( super::HandleNameValuePair(szName, szValue) )
	{
		return LTTRUE;
	}

	if ( !_stricmp(szName, "REGION") )
	{
		AIASSERT( strlen( szValue ) < 128, m_pAI->m_hObject, "CAIGoalSearch::HandleNameValuePair: Regionlist exceeds 128 chars" );
		char szCopy[128];
		strcpy( szCopy, szValue );

		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Searching regions: %s", szValue ) );

		HOBJECT hRegion;
		char* tok = strtok( szCopy, "," );
		while( tok )
		{
			if( LT_OK == FindNamedObject( tok, hRegion ) )
			{
				if( IsAIRegion( hRegion ) )
				{
					AIRegion* pRegion = (AIRegion*)g_pLTServer->HandleToObject( hRegion );
					if( pRegion->GetNumSearchNodes() == 0 )
					{
						AITRACE( AIShowGoals, ( m_pAI->m_hObject, "%s has ZERO search nodes", tok ) );
					}
					else {
						m_lstRegions.push_back( hRegion );

						// AITRACE output for debugging searches.

						AINodeSearch* pNode;
						AITRACE( AIShowGoals, ( m_pAI->m_hObject, "%s:", tok ) );
						for( uint32 iNode=0; iNode < pRegion->GetNumSearchNodes(); ++iNode )
						{
							pNode = pRegion->GetSearchNode( iNode );
							if( pNode )
							{
								AITRACE( AIShowGoals, ( m_pAI->m_hObject, "  %s", ::ToString( pNode->GetName() ) ) );
							}
						}
					}
				}
				else {
					AIASSERT( 0, m_pAI->m_hObject, "CAIGoalSearch::HandleNameValuePair: List item is not an AI Region" );
				}
			}
			else {
				AIASSERT( 0, m_pAI->m_hObject, "CAIGoalSearch::HandleNameValuePair: Could not find Region" );
			}

			tok = strtok( LTNULL, "," );
		}

		if( m_lstRegions.empty() )
		{
			m_fCurImportance = 0.f;
		}

		return LTTRUE;
	}

	else if ( !_stricmp(szName, "MOVEMENT") )
	{
		EnumAnimProp eMovement = CAnimationMgrList::GetPropFromName( szValue );

		// Force the AI to walk or run while searching by artifically
		// monkeying with his alarm level. The decision to walk or run in 
		// the Search state is based off alarm level.

		switch( eMovement )
		{
			case kAP_Run:
				m_pAI->SetAlarmLevel( m_pAI->GetBrain()->GetImmediateAlarmThreshold() + 1 );
				break;

			default:
				m_pAI->SetAlarmLevel( 0 );
				break;
		}

		return LTTRUE;
	}

	return LTFALSE;
}
