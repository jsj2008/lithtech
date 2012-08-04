// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "Stdafx.h"
#include "AIState.h"
#include "AI.h"
#include "AITarget.h"
#include "AIMovement.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIState::CAIState() : 
	m_pAI(NULL),
	m_eStateStatus(kAIStateStatus_Uninitialized)
{
}

CAIState::~CAIState()
{
///	m_pAI->GetSenseMgr()->Clear();

	// It is very strange to destruct a state without an AI.
	// Any constructed State should have Init called to set the pointer.

	AIASSERT( m_pAI, NULL, "CAIState::~CAIState: State has NULL m_pAI" );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Init
//
//	PURPOSE:	Initializes the state
//
// ----------------------------------------------------------------------- //

bool CAIState::Init(CAI* pAI)
{
	m_pAI = pAI;

	if( m_pAI->HasTarget( kTarget_Character | kTarget_Object ) )
	{
		m_pAI->GetTarget()->SetPushSpeed( 0.f );
	}

	m_pAI->GetAIMovement()->ClearMovement();

	m_eStateStatus = kAIStateStatus_Initialized;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Load
//
//	PURPOSE:	Restores the state
//
// ----------------------------------------------------------------------- //

void CAIState::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST(m_eStateStatus, ENUM_AIStateStatus);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIState::Save
//
//	PURPOSE:	Saves the state
//
// ----------------------------------------------------------------------- //

void CAIState::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD(m_eStateStatus);
}
