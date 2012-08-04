// ----------------------------------------------------------------------- //
//
// MODULE  : CAICoordinator.cpp
//
// PURPOSE : CAICoordinator class implementation
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AICoordinator.h"
#include "AI.h"
#include "AIDB.h"
#include "AIActivityAbstract.h"
#include "AIActivityAdvanceCover.h"
#include "AIActivityGetToCover.h"
#include "AISensorMgr.h"
#include "AIBlackBoard.h"
#include "Character.h"
#include "CharacterMgr.h"

// Globals / Statics

CAICoordinator* g_pAICoordinator = NULL;

#define SQUAD_THRESH_WIDTH		800.f
#define SQUAD_THRESH_HEIGHT		250.f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAICoordinator::CAICoordinator()
{
	AIASSERT( !g_pAICoordinator, NULL, "CAICoordinator: Singleton already set." );
	g_pAICoordinator = this;

	m_cSquads = 0;
	m_iSquadToUpdate = 0;
	m_fSquadRegenRate = 1.f;
	m_fNextSquadRegenTime = 0.f;
}

CAICoordinator::~CAICoordinator()
{
	AIASSERT( g_pAICoordinator, NULL, "CAICoordinator: No singleton." );
	g_pAICoordinator = NULL;
}

void CAICoordinator::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_lstSquads.size());
	
	for (uint32 i = 0; i < m_lstSquads.size(); ++i)
	{
		m_lstSquads[i]->Save(pMsg);
	}

	SAVE_INT(m_cSquads);
	SAVE_INT(m_iSquadToUpdate);
	SAVE_FLOAT(m_fSquadRegenRate);
	SAVE_TIME(m_fNextSquadRegenTime);
}

void CAICoordinator::Load(ILTMessage_Read *pMsg)
{
	int nSquadCount = 0;
	LOAD_INT(nSquadCount);

	m_lstSquads.reserve(nSquadCount);
	for (int i = 0; i < nSquadCount; ++i)
	{
		CAISquad* pSquad = AI_FACTORY_NEW( CAISquad );
		pSquad->Load(pMsg);
		m_lstSquads.push_back( pSquad );
	}

	LOAD_INT(m_cSquads);
	LOAD_INT(m_iSquadToUpdate);
	LOAD_FLOAT(m_fSquadRegenRate);
	LOAD_TIME(m_fNextSquadRegenTime);

	InitAICoordinator();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::InitAICoordinator
//
//	PURPOSE:	Initialize the coordinator.  This is called on Creation()
//				and on Load().
//
// ----------------------------------------------------------------------- //

void CAICoordinator::InitAICoordinator()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::TermAICoordinator
//
//	PURPOSE:	Terminate the coordinator.
//
// ----------------------------------------------------------------------- //

void CAICoordinator::TermAICoordinator()
{
	// Delete squads.

	CAISquad* pSquad;
	AI_SQUAD_LIST::iterator itSquad;
	for( itSquad = m_lstSquads.begin(); itSquad != m_lstSquads.end(); ++itSquad )
	{
		pSquad = *itSquad;
		AI_FACTORY_DELETE( pSquad );
	}
	m_lstSquads.clear();
	m_cSquads = 0;
	m_iSquadToUpdate = 0;
	m_fSquadRegenRate = 1.f;
	m_fNextSquadRegenTime = 0.f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::GenerateSquads
//
//	PURPOSE:	Generate squads from the list of AI.
//
// ----------------------------------------------------------------------- //

void CAICoordinator::GenerateSquads()
{
	CTList<CCharacter*>* plstChars = g_pCharacterMgr->GetCharacterList( CCharacterMgr::kList_AIs );
	CCharacter** pCur;
	CAI* pCurAI;

	// Create the bounding box representing the clustering area of an AI.
	// An AI will cluster with any squad that overlaps this box.

	LTRect3f AIAABB;
	LTVector vDims( SQUAD_THRESH_WIDTH, SQUAD_THRESH_HEIGHT, SQUAD_THRESH_WIDTH );
	AIAABB.Init( -vDims, vDims );

	ENUM_AI_SQUAD_ID eSquadID;
	CAISquad* pSquad;
	CAISquad* pMergeSquad;
	unsigned int iSquad;
	ENUM_AIActivitySet eActivitySet;
	AIDB_ActivitySetRecord* pActivitySetRecord;

	m_cSquads = 0;
	m_iSquadToUpdate = 0;

	// Place each AI in a squad.

	pCur = plstChars->GetItem( TLIT_FIRST );
	while( pCur )
	{
		pCurAI = (CAI*)*pCur;
		pCur = plstChars->GetItem( TLIT_NEXT );

		// Do not put dead AI in a squad.

		if( IsDeadAI( pCurAI->m_hObject ) )
		{
			continue;
		}

		// Do not put AI who are not sensing in a squad.

		if( !pCurAI->GetAIBlackBoard()->GetBBSensesOn() )
		{
			continue;
		}

		// No AIAttributes for this AI.

		if ( !pCurAI->GetAIAttributes() )
		{
			continue;
		}

		// Some types of AI may not join squads.

		if( !pCurAI->GetAIAttributes()->bCanJoinSquads )
		{
			continue;
		}

		// Do not put AI in a squad if AI has no activity set,
		// or no activities in his activity set.

		eActivitySet = pCurAI->GetAIBlackBoard()->GetBBAIActivitySet();
		pActivitySetRecord = g_pAIDB->GetAIActivitySetRecord( eActivitySet );
		if( !pActivitySetRecord )
		{
			continue;
		}
		if( pActivitySetRecord->ActivityMask.none() )
		{
			continue;
		}

		// Move box to the AI's position.

		AIAABB.Offset( pCurAI->GetPosition() );

		// Initially AI is not in any squad.

		eSquadID = kSquad_Invalid;

		// Check box against each squad.

		for( iSquad = 0; iSquad < m_cSquads; ++iSquad )
		{
			pSquad = m_lstSquads[iSquad];

			// Skip squads that have been invalidated due to merging.

			if( pSquad->GetSquadID() == kSquad_Invalid )
			{
				continue;
			}

			// Skip squad if alignment does not match.

			if( pSquad->GetSquadAlignment() != pCurAI->GetAlignment() )
			{
				continue;
			}

			// AI overlaps bounds of existing squad.

			if( pSquad->OverlapsSquad( AIAABB ) )
			{
				// AI is not currently in any squad.

				if( eSquadID == kSquad_Invalid )
				{
					// Add AI to squad.

					pSquad->AddSquadMember( pCurAI->m_hObject, AIAABB );
					eSquadID = pSquad->GetSquadID();
				}

				// AI is already in another squad.

				else {

					// Merge this squad with the one the AI is already in.

					pMergeSquad = FindSquad( eSquadID );
					if( pMergeSquad )
					{
						pMergeSquad->MergeSquad( pSquad );
					}
				}
			}
		}

		// AI did not overlap with any existing squads, so add a new squad.

		if( eSquadID == kSquad_Invalid )
		{
			// Squads are pooled and recycled, so only create
			// a new squad if the list is full.

			if( m_lstSquads.size() <= m_cSquads )
			{
				pSquad = AI_FACTORY_NEW( CAISquad );
				m_lstSquads.push_back( pSquad );
			}

			// Setup the new squad.

			pSquad = m_lstSquads[m_cSquads];
			pSquad->InitSquad( ( ENUM_AI_SQUAD_ID )m_cSquads, pCurAI->GetAlignment() );
			pSquad->AddSquadMember( pCurAI->m_hObject, AIAABB );
			++m_cSquads;
		}

		AIAABB.Offset( -pCurAI->GetPosition() );
	}

	// Compact squads to get rid of squads invalidated due to merging.

	CompactSquads();

	// Initialize squad activities.

	AI_SQUAD_LIST::iterator itSquad;
	for( itSquad = m_lstSquads.begin(); itSquad != m_lstSquads.end(); ++itSquad )
	{
		pSquad = *itSquad;
		if( pSquad )
		{
			pSquad->InitActivities();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::FindSquad
//
//	PURPOSE:	Returns a pointer to the specified squad, if it exists.
//
// ----------------------------------------------------------------------- //

CAISquad* CAICoordinator::FindSquad( ENUM_AI_SQUAD_ID eSquadID )
{
	// Bail if specified squad's ID is invalid.

	if( eSquadID == kSquad_Invalid )
	{
		return NULL;
	}

	// Find an existing squad with a matching ID.

	CAISquad* pSquad;
	AI_SQUAD_LIST::iterator itSquad;
	for( itSquad = m_lstSquads.begin(); itSquad != m_lstSquads.end(); ++itSquad )
	{
		pSquad = *itSquad;
		if( pSquad->GetSquadID() == eSquadID )
		{
			return pSquad;
		}
	}

	// No match found.

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::CompactSquads
//
//	PURPOSE:	Delete squads invalidated through merging, and
//              re-issue sequential squad IDs to the valid squads.
//
// ----------------------------------------------------------------------- //

void CAICoordinator::CompactSquads()
{
	m_cSquads = 0;

	static AI_SQUAD_LIST lstFreeSquads;
	lstFreeSquads.resize( 0 );

	// Iterate over squads.

	CAISquad* pSquad;
	AI_SQUAD_LIST::iterator itSquad = m_lstSquads.begin();
	while( itSquad != m_lstSquads.end() )
	{
		// Delete squads that were invalidated through merging.

		pSquad = *itSquad;
		if( pSquad->GetSquadID() == kSquad_Invalid )
		{
			lstFreeSquads.push_back( pSquad );
			itSquad = m_lstSquads.erase( itSquad );
		}

		// Re-issue sequential squad IDs to valid squads.

		else {
			pSquad->SetSquadID( ( ENUM_AI_SQUAD_ID )m_cSquads );
			++m_cSquads;
			++itSquad;
		}
	}

	// Add free squads to back of squad list.

	for( itSquad = lstFreeSquads.begin(); itSquad != lstFreeSquads.end(); ++itSquad )
	{
		pSquad = *itSquad;
		m_lstSquads.push_back( pSquad );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::IsInAIActivitySet
//
//	PURPOSE:	Returns true if the activity is in the specified set, else
//				returns false.
//
// ----------------------------------------------------------------------- //

bool CAICoordinator::IsActivityInAIActivitySet( ENUM_AIActivitySet eSet, EnumAIActivityType eActivity )
{
	// Activity enum is invalid.

	if( ( eActivity <= kActivity_InvalidType ) ||
		( eActivity >= kActivity_Count ) )
	{
		return false;
	}

	AIDB_ActivitySetRecord* pRecord = g_pAIDB->GetAIActivitySetRecord( eSet );
	if( !pRecord )
	{
		return false;
	}

	return pRecord->ActivityMask.test( eActivity );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::UpdateAICoordinator
//
//	PURPOSE:	Update the coordinator.
//
// ----------------------------------------------------------------------- //

void CAICoordinator::UpdateAICoordinator()
{
	// NOTE:  Eventually use a scheduler to manage updates...

	double fCurTime = g_pLTServer->GetTime();

	// Iterate over squads, attempting to update each.
	// Stop iterating when a squad has truly updated.

	unsigned int iSquad;
	CAISquad* pSquad;
	for( iSquad=0; iSquad < m_cSquads; ++iSquad )
	{
		pSquad = m_lstSquads[m_iSquadToUpdate];
		m_iSquadToUpdate = ( m_iSquadToUpdate + 1 ) % m_cSquads;

		if( pSquad && pSquad->UpdateSquad() )
		{
			break;
		}
	}
	
	// Regenerate squads at some rate, if all squads 
	// have had an opportunity to update, and no one is
	// currently engaged in an activity.
	// Do not regenerate if a squad has not had a chance 
	// to update all of its activities.

	if( m_fNextSquadRegenTime <= fCurTime )
	{
		for( iSquad=0; iSquad < m_cSquads; ++iSquad )
		{
			pSquad = m_lstSquads[iSquad];
			if( pSquad->IsSquadEngaged() ||
				!pSquad->SquadCompletedUpdate() )
			{
				return;
			}
		}

		GenerateSquads();
		m_fNextSquadRegenTime = fCurTime + m_fSquadRegenRate;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::GetSquadID
//
//	PURPOSE:	Return the SquadID for some AI.
//
// ----------------------------------------------------------------------- //

ENUM_AI_SQUAD_ID CAICoordinator::GetSquadID( HOBJECT hAI ) const
{
	// Sanity check.

	if( !hAI )
	{
		return kSquad_Invalid;
	}

	// Find squad that contains AI.

	for( uint32 iSquad=0; iSquad < m_cSquads; ++iSquad )
	{
		if( m_lstSquads[iSquad] && 
			m_lstSquads[iSquad]->IsSquadMember( hAI ) )
		{
			return m_lstSquads[iSquad]->GetSquadID();
		}
	}

	// No squad found.

	return kSquad_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAICoordinator::FindAlly
//
//	PURPOSE:	Return the handle to an ally of teh specified AI.
//
// ----------------------------------------------------------------------- //

HOBJECT	CAICoordinator::FindAlly( HOBJECT hAI, HOBJECT hVisibleTarget )
{
	// AI is not in a squad.

	ENUM_AI_SQUAD_ID eSquad = GetSquadID( hAI );
	CAISquad* pSquad = FindSquad( eSquad );
	if( !pSquad )
	{
		return NULL;
	}

	// Only one AI in squad.

	uint32 cSquadMembers = pSquad->GetNumSquadMembers();
	if( cSquadMembers == 1 )
	{
		return NULL;
	}

	// Find an ally.

	CAI* pAI;
	LTObjRef* pSquadMembers = pSquad->GetSquadMembers();
	for( uint32 iMember=0; iMember < cSquadMembers; ++iMember )
	{
		// Ignore querying AI.

		if( pSquadMembers[iMember] == hAI )
		{
			continue;
		}

		// An optional target has been specified.

		if( hVisibleTarget && IsAI( pSquadMembers[iMember] ) )
		{
			// Ignore AI that are not targeting the same threat.

			pAI = (CAI*)g_pLTServer->HandleToObject( pSquadMembers[iMember] );
			if( pAI->GetAIBlackBoard()->GetBBTargetObject() != hVisibleTarget )
			{
				continue;
			}

			// Ignore AI that can not see the threat.

			if( !pAI->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
			{
				continue;
			}
		}

		// We found an ally.

		return pSquadMembers[iMember];
	}

	// No allies found.

	return NULL;
}
