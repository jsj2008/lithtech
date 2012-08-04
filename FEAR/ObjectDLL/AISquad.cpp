// ----------------------------------------------------------------------- //
//
// MODULE  : CAISquad.cpp
//
// PURPOSE : CAISquad class implementation
//
// CREATED : 6/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISquad.h"
#include "AI.h"
#include "AISoundMgr.h"
#include "AIBlackBoard.h"
#include "CharacterAlignment.h"
#include "CharacterDB.h"

DEFINE_AI_FACTORY_CLASS( CAISquad );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::CompareActivities
//
//	PURPOSE:	Comparison function for qsort.
//
// ----------------------------------------------------------------------- //

int CompareActivities( const void *arg1, const void *arg2 )
{
	if( !( arg1 && arg2 ) )
	{
		return 0;
	}

	// Activities are sorted by priority.

	CAIActivityAbstract* pActivity1 = *( CAIActivityAbstract** )arg1;
	CAIActivityAbstract* pActivity2 = *( CAIActivityAbstract** )arg2;
	if( !( pActivity1 && pActivity2 ) )
	{
		return 0;
	}

	if( pActivity1->GetActivityPriority() > pActivity2->GetActivityPriority() )
	{
		return 1;
	}

	if( pActivity1->GetActivityPriority() < pActivity2->GetActivityPriority() )
	{
		return -1;
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAISquad::CAISquad()
{
	m_eSquadID = kSquad_Invalid;
	m_eAlignment = kCharAlignment_Invalid;

	// Initialize list of squad memebers, and associated bounding box.

	for( int iMember=0; iMember < MAX_AI_SQUAD_SIZE; ++iMember )
	{
		m_aSquadMembers[iMember] = NULL;
	}
	m_cSquadMembers = 0;
	m_SquadAABB.Init();

	m_bSquadMemberDied = false;

	// Give squad a full set of available activities.

	for( int iActivity=0; iActivity < kActivity_Count; ++iActivity )
	{
		m_apAIActivities[iActivity] = AI_FACTORY_NEW_Activity( ( EnumAIActivityType )iActivity );
	}

	// Sort Activities by priority.

	qsort( (void*)m_apAIActivities, (size_t)kActivity_Count, sizeof( CAIActivityAbstract* ), CompareActivities );

	m_pCurActivity = NULL;
	m_iActivityToTest = 0;
	m_bTestedAllActivities = false;
}

CAISquad::~CAISquad()
{
	for( int iActivity=0; iActivity < kActivity_Count; ++iActivity )
	{
		AI_FACTORY_DELETE( m_apAIActivities[iActivity] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::Save/Load
//
//	PURPOSE:	Save/Load the squad.
//
// ----------------------------------------------------------------------- //

void CAISquad::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_eSquadID);
	
	// Save the string, because indices may change if alignment records are added.

	std::string strAlignment = g_pCharacterDB->Alignment2String( m_eAlignment );
	SAVE_STDSTRING( strAlignment );

	SAVE_VECTOR(m_SquadAABB.m_vMax);
	SAVE_VECTOR(m_SquadAABB.m_vMin);

	for (int i = 0; i < MAX_AI_SQUAD_SIZE; ++i)
	{
		SAVE_HOBJECT(m_aSquadMembers[i])
	}

	SAVE_INT(m_cSquadMembers);

	SAVE_bool( m_bSquadMemberDied );

	EnumAIActivityType eCurrentActivityID = kActivity_InvalidType;
	if (m_pCurActivity)
	{
		eCurrentActivityID = m_pCurActivity->GetActivityClassType();
	}
	SAVE_INT(eCurrentActivityID);

	for( int iActivity=0; iActivity < kActivity_Count; ++iActivity )
	{
		m_apAIActivities[iActivity]->Save(pMsg);
	}

	SAVE_INT(m_iActivityToTest);
	SAVE_bool(m_bTestedAllActivities);
}

void CAISquad::Load(ILTMessage_Read *pMsg)
{
	LOAD_INT_CAST(m_eSquadID, ENUM_AI_SQUAD_ID);

	// Load the string, because indices may change if alignment records are added.

	std::string strAlignment;
	LOAD_STDSTRING( strAlignment );
	m_eAlignment = g_pCharacterDB->String2Alignment( strAlignment.c_str() );

	LOAD_VECTOR(m_SquadAABB.m_vMax);
	LOAD_VECTOR(m_SquadAABB.m_vMin);

	for (int i = 0; i < MAX_AI_SQUAD_SIZE; ++i)
	{
		LOAD_HOBJECT(m_aSquadMembers[i])
	}

	LOAD_INT(m_cSquadMembers);

	LOAD_bool( m_bSquadMemberDied );

	EnumAIActivityType eCurrentActivityID;
	LOAD_INT_CAST(eCurrentActivityID, EnumAIActivityType);
	if (kActivity_InvalidType == eCurrentActivityID)
	{
		m_pCurActivity = NULL;
	}
	else
	{
		m_pCurActivity = m_apAIActivities[eCurrentActivityID];
	}

	for( int iActivity=0; iActivity < kActivity_Count; ++iActivity )
	{
		m_apAIActivities[iActivity]->Load(pMsg);
		m_apAIActivities[iActivity]->HookParent(this);
	}

	LOAD_INT(m_iActivityToTest);
	LOAD_bool(m_bTestedAllActivities);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::InitSquad
//
//	PURPOSE:	Initialize the squad.
//
// ----------------------------------------------------------------------- //

void CAISquad::InitSquad( ENUM_AI_SQUAD_ID eSquadID, EnumCharacterAlignment eAlignment )
{
	m_eSquadID = eSquadID;
	m_eAlignment = eAlignment;

	m_SquadAABB.Init();
	m_cSquadMembers = 0;

	m_bSquadMemberDied = false;

	m_pCurActivity = NULL;
	m_iActivityToTest = 0;
	m_bTestedAllActivities = false;

	for( int iActivity=0; iActivity < kActivity_Count; ++iActivity )
	{
		m_apAIActivities[iActivity]->HookParent(this);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::OverlapsSquad
//
//	PURPOSE:	Return true if specified AABB overlaps squad's AABB.
//
// ----------------------------------------------------------------------- //

bool CAISquad::OverlapsSquad( const LTRect3f& AABB ) const
{
	return m_SquadAABB.Overlaps( AABB );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::AddSquadMember
//
//	PURPOSE:	Add handle of an AI to the squad's member list.
//
// ----------------------------------------------------------------------- //

void CAISquad::AddSquadMember( HOBJECT hMember, const LTRect3f& AABB )
{
	// Bail if squad is full.

	if( m_cSquadMembers < MAX_AI_SQUAD_SIZE )
	{
		AITRACE( AIShowSquads, ( hMember, "Joining Squad %d", m_eSquadID ) );

		// Expand the squad's bounds to encompass new member.

		if( m_cSquadMembers == 0 )
		{
			m_SquadAABB.Init( AABB.m_vMin, AABB.m_vMax );
		}
		else {
			m_SquadAABB.Merge( AABB );
		}

		// Add new member to array and increment member count.

		m_aSquadMembers[m_cSquadMembers] = hMember;
		++m_cSquadMembers;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::MergeSquad
//
//	PURPOSE:	Merge one squad into another.
//
// ----------------------------------------------------------------------- //

void CAISquad::MergeSquad( CAISquad* pSquad )
{
	// Add all of the specified squad's members to this squad.

	for( int iMember=0; iMember < pSquad->m_cSquadMembers; ++iMember )
	{
		// Bail is squad is full.

		if( m_cSquadMembers < MAX_AI_SQUAD_SIZE )
		{
			m_aSquadMembers[m_cSquadMembers] = pSquad->m_aSquadMembers[iMember];
			++m_cSquadMembers;
		}
	}

	// Expand this squad's bounds to encompass specified squad's bounds.

	m_SquadAABB.Merge( pSquad->m_SquadAABB );

	// Invalidate specified squad.

	pSquad->m_eSquadID = kSquad_Invalid;
	pSquad->m_cSquadMembers = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::InitActivities
//
//	PURPOSE:	Initialize the squad activities.
//
// ----------------------------------------------------------------------- //

void CAISquad::InitActivities()
{
	for( int iActivity=0; iActivity < kActivity_Count; ++iActivity )
	{
		m_apAIActivities[iActivity]->InitActivity();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::UpdateSquad
//
//	PURPOSE:	Return true if squad truly updated.
//
// ----------------------------------------------------------------------- //

bool CAISquad::UpdateSquad()
{
	HandleSquadDeaths();

	double fCurTime = g_pLTServer->GetTime();

	// Update the currently active activity.

	if( m_pCurActivity )
	{
		if( !m_pCurActivity->UpdateActivity() )
		{
			m_pCurActivity->DeactivateActivity();
			m_pCurActivity = NULL;
		}

		return true;
	}

	// Select a relevant activity.
	// Only test one activity per update, round-robin style.

	for( unsigned int iActivity=0; iActivity < kActivity_Count; ++iActivity )
	{
		CAIActivityAbstract* pActivity = m_apAIActivities[m_iActivityToTest];
		++m_iActivityToTest;
		if( m_iActivityToTest >= kActivity_Count )
		{
			m_iActivityToTest = 0;
			m_bTestedAllActivities = true;
		}

		if( !pActivity )
		{
			continue;
		}

		if( pActivity->GetNumPotentialParticipants() == 0 )
		{
			continue;
		}

		if( fCurTime < pActivity->GetNextActivityUpdateTime() )
		{
			continue;
		}

		if( pActivity->IsActivityRelevant() )
		{
			// If the activity successfully activates, 
			// it becomes the current activity.
	
			if( pActivity->ActivateActivity() )
			{
				m_pCurActivity = pActivity;
				m_pCurActivity->UpdateActivity();
			}
		}
	
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::HasTarget
//
//	PURPOSE:	Return true if anyone in the squad has the sepcified 
//				type of target.
//
// ----------------------------------------------------------------------- //

bool CAISquad::HasTarget( unsigned int dwTargetFlags )
{
	// Find a squad-member with the specified type of target.

	CAI* pAI;
	for( int iMember=0; iMember < m_cSquadMembers; ++iMember )
	{
		if( m_aSquadMembers[iMember] )
		{
			pAI = (CAI*)g_pLTServer->HandleToObject( m_aSquadMembers[iMember] );
			if( pAI->HasTarget( dwTargetFlags ) )
			{
				return true;
			}
		}
	}

	// No target.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::HandleSquadDeaths
//
//	PURPOSE:	Recognize when a squad member has died, and respond.
//
// ----------------------------------------------------------------------- //

void CAISquad::HandleSquadDeaths()
{
	// Search for dead squad members.

	bool bDead = false;
	int iMember;
	for( iMember=0; iMember < m_cSquadMembers; ++iMember )
	{
		if( IsDeadAI( m_aSquadMembers[iMember] ) )
		{
			m_aSquadMembers[iMember] = NULL;
			bDead = true;
			break;
		}
	}

	// Bail if no one died.

	if( !bDead )
	{
		return;
	}
	m_bSquadMemberDied = true;

	// Compact the squad to cover dead member.

	int iDead = iMember;
	for( iMember = iDead; iMember < m_cSquadMembers - 1; ++iMember )
	{
		m_aSquadMembers[iMember] = m_aSquadMembers[iMember+1];
	}
	--m_cSquadMembers;

	// Each squad member keeps a body count of deaths they are aware of.

	CAI* pAI;
	uint32 cBodyCount;
	for( iMember = 0; iMember < m_cSquadMembers; ++iMember )
	{
		if( !IsAI( m_aSquadMembers[iMember] ) )
		{
			continue;
		}

		pAI = (CAI*)g_pLTServer->HandleToObject( m_aSquadMembers[iMember] );
		cBodyCount = pAI->GetAIBlackBoard()->GetBBBodyCount();
		pAI->GetAIBlackBoard()->SetBBBodyCount( cBodyCount + 1 );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::AI_FACTORY_NEW_Activity
//
//	PURPOSE:	Create an activity.
//
// ----------------------------------------------------------------------- //

CAIActivityAbstract* CAISquad::AI_FACTORY_NEW_Activity( EnumAIActivityType eActivityType )
{
	// Call AI_FACTORY_NEW for the requested type of activity.

	switch( eActivityType )
	{
		#define ACTIVITY_TYPE_AS_SWITCH 1
		#include "AIEnumActivityTypes.h"
		#undef ACTIVITY_TYPE_AS_SWITCH

		default: AIASSERT( 0, NULL, "CAISquad::AI_FACTORY_NEW_Activity: Unrecognized activity type." );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::IsSquadEngaged
//
//	PURPOSE:	Return true if squad in engaged in some activity.
//
// ----------------------------------------------------------------------- //

bool CAISquad::IsSquadEngaged() const
{
	if( m_pCurActivity )
	{
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::IsSquadMember
//
//	PURPOSE:	Return true if AI is a member of squad.
//
// ----------------------------------------------------------------------- //

bool CAISquad::IsSquadMember( HOBJECT hAI ) const
{
	// Sanity check.

	if( !hAI )
	{
		return false;
	}

	// Find the squad member.

	for( int iMember=0; iMember < m_cSquadMembers; ++iMember )
	{
		if( m_aSquadMembers[iMember] == hAI )
		{
			return true;
		}
	}

	// AI is not a member.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISquad::SquadCanSeeTarget
//
//	PURPOSE:	Return true if someone in teh squad can see their target.
//
// ----------------------------------------------------------------------- //

bool CAISquad::SquadCanSeeTarget( ENUM_AI_TARGET_TYPE eTargetType ) const
{
	// Find a squad member who can see a target.

	CAI* pMember;
	for( int iMember=0; iMember < m_cSquadMembers; ++iMember )
	{
		pMember = (CAI*)g_pLTServer->HandleToObject( m_aSquadMembers[iMember] );
		if( pMember && pMember->HasTarget( eTargetType ) &&
			pMember->GetAIBlackBoard()->GetBBTargetVisibleFromEye() )
		{
			return true;
		}
	}

	// No one can see the target.

	return false;
}

// ----------------------------------------------------------------------- //

