// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCounterMelee.cpp
//
// PURPOSE : 
//
// CREATED : 9/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorCounterMelee.h"
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIWorkingMemory.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCounterMelee, kSensor_CounterMelee );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorCounterMelee::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorCounterMelee::CAISensorCounterMelee() : 
	m_bBlockOpportunity(false)
	, m_bDodgeOpportunity(false)
{
}

CAISensorCounterMelee::~CAISensorCounterMelee()
{
}

void CAISensorCounterMelee::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_bool( m_bBlockOpportunity );
	SAVE_bool( m_bDodgeOpportunity );
}

void CAISensorCounterMelee::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );
	
	LOAD_bool( m_bBlockOpportunity );
	LOAD_bool( m_bDodgeOpportunity );
}



//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorCounterMelee::UpdateSensor
//              
//	PURPOSE:	Handle updating the CAISensorCounterMelee sensor.  
//				If the AI could not block before but 
//              
//----------------------------------------------------------------------------

bool CAISensorCounterMelee::UpdateSensor()
{
	if ( !super::UpdateSensor() )
	{
		return false;
	}

	// Determine if the AI can block at this time.

	bool bBlockOpportunity = false;
	bool bDodgeOpportunity = false;
	if ( m_pAI->HasTarget( kTarget_Character ) )
	{
		CCharacter* pChar = CCharacter::DynamicCast( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
		if ( pChar )
		{
			bBlockOpportunity = pChar->GetBlockWindowOpen();
			bDodgeOpportunity = pChar->GetDodgeWindowOpen();
		}
	}

	// Enemy just exposed themselves to a counter move.
	// Record this in working memory by setting the desire to perform 
	// a countermelee to 1.0.  This ONLY needs to be updated if last frame
	// the AI couldn't perform either action, and this frame one is valid.

	if ( ( bBlockOpportunity || bDodgeOpportunity )
		&& ( !m_bBlockOpportunity || !m_bDodgeOpportunity ) )
	{
		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Desire );
		queryFact.SetDesireType( kDesire_CounterMelee );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
		if ( !pFact )
		{
			pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Desire );
			pFact->SetDesireType( kDesire_CounterMelee );
		}

		if ( pFact )
		{
			pFact->SetSourceObject( m_pAI->GetAIBlackBoard()->GetBBTargetObject() );
			pFact->SetConfidence( CAIWMFact::kFactMask_DesireType, 1.0f );
			pFact->SetTime( g_pLTServer->GetTime() );
		}
	}

	// Select a new action if a new type of opportunity has opened this update.
	// This needs to happen any time an opportunity opens up; otherwise, 

	if ( bBlockOpportunity && !m_bBlockOpportunity 
		|| bDodgeOpportunity && !m_bDodgeOpportunity )
	{
		m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
	}

	// The window to start a block has elapsed.  Clear the desire to perform 
	// a countermelee move.

	if ( !bBlockOpportunity && m_bBlockOpportunity 
		|| !bDodgeOpportunity && m_bDodgeOpportunity )
	{
		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Desire );
		queryFact.SetDesireType( kDesire_CounterMelee );
		CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( queryFact );
		if ( pFact )
		{
			pFact->SetConfidence( CAIWMFact::kFactMask_DesireType, 0.0f );
		}
	}

	// Update the update history tracking state.

	m_bBlockOpportunity = bBlockOpportunity;
	m_bDodgeOpportunity = bDodgeOpportunity;

	// Sensor did not perform any significant work.  Do not block sensor updating.

	return false;
}
