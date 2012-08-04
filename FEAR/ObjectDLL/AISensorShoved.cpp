// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorShoved.cpp
//
// PURPOSE : 
//
// CREATED : 11/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AISensorShoved.h"
#include "AIStimulusMgr.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorShoved, kSensor_Shoved );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorShoved::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAISensorShoved::CAISensorShoved()
{
}

CAISensorShoved::~CAISensorShoved()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAISensorShoved::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAISensorShoved
//              
//----------------------------------------------------------------------------

void CAISensorShoved::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

void CAISensorShoved::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAISensorShoved::CreateWorkingMemoryFact
//
//	PURPOSE:	Return the working memory fact that will hold the 
//              memory of sensing this stimulus.
//
// ----------------------------------------------------------------------- //

CAIWMFact* CAISensorShoved::CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord )
{
	// Find an existing memory for being shoved.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Shoved );
	CAIWMFact* pFact = m_pAI->GetAIWorkingMemory()->FindWMFact( factQuery );

	// Create a new memory for being shoved.

	if( !pFact )
	{
		pFact = m_pAI->GetAIWorkingMemory()->CreateWMFact( kFact_Knowledge );
		pFact->SetKnowledgeType( kKnowledge_Shoved );
	}

	if( pFact )
	{
		pFact->SetTime( g_pLTServer->GetTime() );
		pFact->SetSourceObject( pStimulusRecord->m_hStimulusTarget );
	}

	return pFact;
}
