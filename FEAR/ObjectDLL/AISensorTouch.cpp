// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTouch.cpp
//
// PURPOSE : 
//
// CREATED : 8/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorTouch.h"
#include "AIStimulusMgr.h"
#include "AI.h"

LINKFROM_MODULE(AISensorTouch);

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTouch, kSensor_Touch );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTouch::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorTouch::CAISensorTouch()
{
}

CAISensorTouch::~CAISensorTouch()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorTouch::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorTouch
//              
//----------------------------------------------------------------------------

void CAISensorTouch::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAISensorTouch::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTouch::GetSenseDistSqr
//
//	PURPOSE:	Return the square of the distance the AI can sense.
//
// ----------------------------------------------------------------------- //

float CAISensorTouch::GetSenseDistSqr( float fStimulusRadius )
{
	return FLT_MAX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTouch::DoComplexCheck
//
//	PURPOSE:	Return true if the stimulus can be sensed.
//
// ----------------------------------------------------------------------- //

bool CAISensorTouch::DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier )
{
	// Don't sense a touch if touching is turned off.

	if( m_pAI->GetAIBlackBoard()->GetBBHandleTouch() == kTouch_None )
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorTouch::CreateWorkingMemoryFact.
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorTouch::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Update a fact about this event, or create one if there is no such fact.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Touch );
	factQuery.SetSourceObject( pStimulusRecord->m_hStimulusTarget );

	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );
	if ( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
		if ( pFact )
		{
			pFact->SetKnowledgeType( kKnowledge_Touch, 1.f );
			pFact->SetSourceObject( pStimulusRecord->m_hStimulusTarget, 1.f );
		}
	}

	// Insure the time is always updated.

	if ( pFact )
	{
		pFact->SetTime( g_pLTServer->GetTime() );
	}
	return pFact;
}
