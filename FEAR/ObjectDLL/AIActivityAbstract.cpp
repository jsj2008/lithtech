// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityAbstract.cpp
//
// PURPOSE : AIActivityAbstract abstract class implementation
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActivityAbstract.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AICoordinator.h"
#include "AISquad.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAbstract::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActivityAbstract::CAIActivityAbstract()
{
	m_eActStatus = kActStatus_Invalid;

	m_nActivityPriority = 0;

	m_fNextActivityUpdateTime = 0.f;
	m_fActivityUpdateRate = 0.f;
	m_fActivityActivateTime = 0.f;
	m_fActivityTimeOut = 0.f;
	m_fActivityExpirationTime = 0.f;

	m_pSquad = NULL;
	m_cPotentialParticipants = 0;
}

CAIActivityAbstract::~CAIActivityAbstract()
{
}

void CAIActivityAbstract::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_eActStatus);

	SAVE_INT(m_nActivityPriority);

	SAVE_FLOAT(m_fActivityUpdateRate);
	SAVE_TIME(m_fNextActivityUpdateTime);
	SAVE_TIME(m_fActivityActivateTime);
	SAVE_TIME(m_fActivityTimeOut);
	SAVE_TIME(m_fActivityExpirationTime);

	SAVE_INT(m_cPotentialParticipants);
	for (int i = 0; i < MAX_PARTICIPANTS; ++i)
	{
		SAVE_HOBJECT(m_aPotentialParticipants[i]);
	}

	// Don't save:
	// m_pSquad;
}

void CAIActivityAbstract::Load(ILTMessage_Read *pMsg)
{
	LOAD_INT_CAST(m_eActStatus, ENUM_AIACTIVITY_STATUS);

	LOAD_INT(m_nActivityPriority);

	LOAD_FLOAT(m_fActivityUpdateRate);
	LOAD_TIME(m_fNextActivityUpdateTime);
	LOAD_TIME(m_fActivityActivateTime);
	LOAD_TIME(m_fActivityTimeOut);
	LOAD_TIME(m_fActivityExpirationTime);

	LOAD_INT(m_cPotentialParticipants);
	for (int i = 0; i < MAX_PARTICIPANTS; ++i)
	{
		LOAD_HOBJECT(m_aPotentialParticipants[i]);
	}

	m_pSquad = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAbstract::InitActivity
//
//	PURPOSE:	Initialize the activity.
//
// ----------------------------------------------------------------------- //

void CAIActivityAbstract::InitActivity()
{
	// Find squad members who could be potential participants of this activity.

	LTObjRef* pSquadMembers = m_pSquad->GetSquadMembers();
	int cSquadMembers = m_pSquad->GetNumSquadMembers();

	CAI* pCurAI;
	m_cPotentialParticipants = 0;
	for( int iMember=0; iMember < cSquadMembers; ++iMember )
	{
		pCurAI = (CAI*)g_pLTServer->HandleToObject( pSquadMembers[iMember] );
		if( !pCurAI )
		{
			continue;
		}

		// Add AI to list if he has this activity in his activity set.

		ENUM_AIActivitySet eActivitySet = pCurAI->GetAIBlackBoard()->GetBBAIActivitySet();
		if(	g_pAICoordinator->IsActivityInAIActivitySet( eActivitySet, GetActivityClassType() ) )
		{
			m_aPotentialParticipants[m_cPotentialParticipants] = pCurAI->m_hObject;
			++m_cPotentialParticipants;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAbstract::IsActivityRelevant
//
//	PURPOSE:	Return true if activity is currently relevant.
//
// ----------------------------------------------------------------------- //

bool CAIActivityAbstract::IsActivityRelevant()
{
	// Sanity check.

	if( !m_pSquad )
	{
		return false;
	}

	// Clear handles to dead AI.

	ClearDeadAI();

	// Reset update timer.

	m_fNextActivityUpdateTime = g_pLTServer->GetTime() + m_fActivityUpdateRate;

	// Zero potential participants.

	if( m_cPotentialParticipants == 0 )
	{
		return false;
	}

	// Look for participants.

	if( !FindActivityParticipants() )
	{
		return false;
	}

	// Found participants.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAdvanceCover::ActivateActivity
//
//	PURPOSE:	Return true if the activity activates successfully.
//
// ----------------------------------------------------------------------- //

bool CAIActivityAbstract::ActivateActivity()
{
	// Set expiration time for activity, to ensure it 
	// bails if something goes awry.

	m_fActivityActivateTime = g_pLTServer->GetTime();
	if( m_fActivityTimeOut > 0.f )
	{
		m_fActivityExpirationTime = m_fActivityActivateTime + m_fActivityTimeOut;
	}

	// Announce squad members activating activity.

	HOBJECT hAI;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		hAI = m_aPotentialParticipants[iPotential];
		if( hAI )
		{
			AITRACE( AIShowActivities, ( hAI, "Activating activity %s", s_aszActivityTypes[GetActivityClassType()] ) );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAbstract::DeactivateActivity
//
//	PURPOSE:	Deactivate an active activity.
//
// ----------------------------------------------------------------------- //

void CAIActivityAbstract::DeactivateActivity()
{
	m_fNextActivityUpdateTime = g_pLTServer->GetTime() + m_fActivityUpdateRate;

	// Announce squad members deactivating activity.

	HOBJECT hAI;
	for( uint32 iPotential=0; iPotential < m_cPotentialParticipants; ++iPotential )
	{
		hAI = m_aPotentialParticipants[iPotential];
		if( hAI )
		{
			AITRACE( AIShowActivities, ( hAI, "Deactivating activity %s", s_aszActivityTypes[GetActivityClassType()] ) );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAbstract::UpdateActivity
//
//	PURPOSE:	Return true if Activity should conrtinue running.
//
// ----------------------------------------------------------------------- //

bool CAIActivityAbstract::UpdateActivity()
{
	// Clear handles to dead AI.

	ClearDeadAI();

	// Activity has expired.

	if( ( m_fActivityExpirationTime > 0.f ) && 
		( m_fActivityExpirationTime < g_pLTServer->GetTime() ) )
	{
		return false;
	}

	// Reset update timer.

	m_fNextActivityUpdateTime = g_pLTServer->GetTime() + m_fActivityUpdateRate;

	// Continue running.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAbstract::ClearDeadAI
//
//	PURPOSE:	Clear handles to dead AI.
//
// ----------------------------------------------------------------------- //

void CAIActivityAbstract::ClearDeadAI()
{
	uint32 iParticipant;
	for( iParticipant=0; iParticipant < m_cPotentialParticipants; ++iParticipant )
	{
		if( m_aPotentialParticipants[iParticipant] &&
			IsDeadAI( m_aPotentialParticipants[iParticipant] ) )
		{
			m_aPotentialParticipants[iParticipant] = NULL;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActivityAbstract::CalcActivityAABB
//
//	PURPOSE:	Calculate the AABB that contains the participants.
//
// ----------------------------------------------------------------------- //

void CAIActivityAbstract::CalcActivityAABB( LTRect3f* pAABB )
{
	// Sanity check.

	if( !pAABB )
	{
		return;
	}

	CAI* pAI;
	uint32 iParticipant;
	bool bFirst = true;
	for( iParticipant=0; iParticipant < m_cPotentialParticipants; ++iParticipant )
	{
		pAI = (CAI*)g_pLTServer->HandleToObject( m_aPotentialParticipants[iParticipant] );
		if( !pAI )
		{
			continue;
		}

		if( bFirst )
		{
			pAABB->Init( pAI->GetPosition(), pAI->GetPosition() );
			bFirst = false;
			continue;
		}

		pAABB->Merge( pAI->GetPosition() );
	}
}

