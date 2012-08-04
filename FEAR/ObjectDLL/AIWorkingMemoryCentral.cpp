// ----------------------------------------------------------------------- //
//
// MODULE  : AIWorkingMemoryCentral.cpp
//
// PURPOSE : Special AIWorkingMemory derived class which is shared between
//			all AIs for coordination reasons.  Reasons for using this
//			include:
//
//			1) Preventing events from happening too frequently -- player 
//				experience engineering.
//			2) Coordinating ai activitities.
//
// CREATED : 12/5/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWorkingMemoryCentral.h"

CAIWorkingMemoryCentral* g_pAIWorkingMemoryCentral = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemoryCentral::Init
//
//	PURPOSE:	Resets the working memory, insuring that all assets are 
//				properly released.
//
//
// ----------------------------------------------------------------------- //
void CAIWorkingMemoryCentral::Init()
{
	ResetWorkingMemory();
	g_pAIWorkingMemoryCentral = this;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemoryCentral::Term
//
//	PURPOSE:	Resets the working memory, insuring that all assets are 
//				properly released.
//
// ----------------------------------------------------------------------- //
void CAIWorkingMemoryCentral::Term()
{
	ResetWorkingMemory();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorkingMemoryCentral::OnAIDebugCmd
//
//	PURPOSE:	Handling printing out information on the state of central 
//				working memory.
//
//	TODO:		Figure out how to print more useful information here.  
//				Currently this is very limited due to the structure of 
//				facts; there is no associated string table, nor is there a
//				clean way to go from facttype to subtype field.
//
// ----------------------------------------------------------------------- //

void CAIWorkingMemoryCentral::OnAIDebugCmd( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_List("List");

	if ( s_cTok_List == crParsedMsg.GetArg(1) )
	{
		g_pLTServer->CPrint( "Printing all facts..." );

		AIWORKING_MEMORY_FACT_LIST::const_iterator itEachFact = GetFactList()->begin();
		AIWORKING_MEMORY_FACT_LIST::const_iterator itEnd = GetFactList()->end();
		for ( ; itEachFact != itEnd; ++itEachFact )
		{
			// Print out extra data about common types of facts.  To determine
			// what these numbers mean, look at AIWorkingMemory.h, which 
			// contains the enums.

			int iFactType = (*itEachFact)->GetFactType();
			int iKnowledgeType = (*itEachFact)->IsSet(CAIWMFact::kFactMask_KnowledgeType) ? (*itEachFact)->GetKnowledgeType() : kKnowledge_InvalidType;
			int iTaskType = (*itEachFact)->IsSet(CAIWMFact::kFactMask_TaskType) ? (*itEachFact)->GetTaskType() : kTask_InvalidType;
			int iDesireType = (*itEachFact)->IsSet(CAIWMFact::kFactMask_DesireType) ? (*itEachFact)->GetDesireType() : kDesire_InvalidType;
			
			g_pLTServer->CPrint( "\t FactType: %d, Task: %d, Knowledge: %d, Desire: %d", iFactType, iTaskType, iKnowledgeType, iDesireType );
		}
	}
	else
	{
		g_pLTServer->CPrint( "AIWorkingMemoryCentral: No command named: %s", crParsedMsg.GetArg(1).c_str() );
	}
}

