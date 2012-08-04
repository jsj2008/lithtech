// ----------------------------------------------------------------------- //
//
// MODULE  : AIMgr.cpp
//
// PURPOSE : AIMgr class implementation
//
// CREATED : 2/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIMgr.h"
#include "AICoordinator.h"
#include "AIPlanner.h"
#include "AIActionMgr.h"
#include "AIStimulusMgr.h"
#include "AISoundMgr.h"
#include "AINodeMgr.h"
#include "AINavMesh.h"
#include "AINavMeshGen.h"

#include "AIQuadTree.h"
#include "AIPathMgrNavMesh.h"
#include "AIWorkingMemoryCentral.h"
#include "iltoutconverter.h"
#include "iltoutstream.h"
#include "ltoutnullconverter.h"
#include "AIStreamSim.h"
#include "CharacterMgr.h"
#include "iperformancemonitor.h"
#include <algorithm>

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"
#endif // PLATFORM_XENON

// Set this to 0 to prevent distributing AI sensor updates.
#define DISTRIBUTE_SENSORS 1

// Performance monitoring.
CTimedSystem g_tsAIMgr("AIMgr", "AI");

// Collection of helper functions print various pieces of global AI info.
struct AIDebugUtils
{
	static void PrintUnusedActions()
	{
		// Brute force search for unused actions.

		g_pLTServer->CPrint( "Unused actions:\n" );
		for ( uint32 EachAction = 0; EachAction < kAct_Count; ++EachAction )
		{
			// Skip this action if there is no record for it.

			AIDB_ActionRecord* pAction = g_pAIDB->GetAIActionRecord( EachAction );
			if ( !pAction || kAct_InvalidType == pAction->eActionType )
			{
				continue;
			}

			// Look for the action in an ActionSet.  If found, set bMatchFound to true.

			bool bMatchFound = false;
			for ( uint32 EachActionSet = 0; EachActionSet < g_pAIDB->GetNumAIActionSetRecords(); ++EachActionSet )
			{
				if ( g_pAIDB->GetAIActionSetRecord( EachActionSet )->ActionMask[pAction->eActionType] )
				{
					bMatchFound = true;
					break;
				}
			}

			if ( !bMatchFound )
			{
				g_pLTServer->CPrint( "    %s\n", s_aszActionTypes[pAction->eActionType] );
			}
		}	
	}

	static bool GoalRecordPtr_Greater( AIDB_GoalRecord* pGoal0, AIDB_GoalRecord* pGoal1 )
	{
		return ( pGoal0->fIntrinsicRelevance > pGoal1->fIntrinsicRelevance );
	}

	static void PrintUnusedGoals()
	{
		// Brute force search for unused actions.

		g_pLTServer->CPrint( "Unused Goals:\n" );
		for ( uint32 EachGoal = 0; EachGoal < kAct_Count; ++EachGoal )
		{
			// Skip this Goal if there is no record for it.

			AIDB_GoalRecord* pGoalRecord = g_pAIDB->GetAIGoalRecord( (EnumAIGoalType)EachGoal );
			if ( !pGoalRecord || kGoal_InvalidType == pGoalRecord->eGoalType )
			{
				continue;
			}

			// 

			bool bMatchFound = false;
			for ( uint32 EachGoalSet = 0; EachGoalSet < g_pAIDB->GetNumAIGoalSetRecords(); ++EachGoalSet )
			{
				AIGOAL_TYPE_LIST::iterator itBeginGoalRecord = g_pAIDB->GetAIGoalSetRecord( EachGoalSet )->lstGoalSet.begin();
				AIGOAL_TYPE_LIST::iterator itEndGoalRecord = g_pAIDB->GetAIGoalSetRecord( EachGoalSet )->lstGoalSet.end();
				if ( itEndGoalRecord != std::find( itBeginGoalRecord, itEndGoalRecord, pGoalRecord->eGoalType ) )
				{
					bMatchFound = true;
					break;
				}
			}

			if ( !bMatchFound )
			{
				g_pLTServer->CPrint( "    %s\n", s_aszGoalTypes[pGoalRecord->eGoalType] );
			}
		}	
	}

	static void PrintGoals()
	{
		//
		// Print all of the goals used in this game, sorted by relevance.
		//

		// Add all of the goals to a list.

		std::vector< AIDB_GoalRecord*, LTAllocator<AIDB_GoalRecord*, LT_MEM_TYPE_OBJECTSHELL> > GoalList;
		for ( uint32 EachGoal = 0; EachGoal < kGoal_Count; ++EachGoal )
		{
			AIDB_GoalRecord* pGoalRecord = g_pAIDB->GetAIGoalRecord( (EnumAIGoalType)EachGoal );
			if ( pGoalRecord && kGoal_InvalidType != pGoalRecord->eGoalType )
			{
				GoalList.push_back( pGoalRecord );
			}
		}

		// Sort the goals.

		std::sort( GoalList.begin(), GoalList.end(), GoalRecordPtr_Greater );

		// Print out the goals/values.

		g_pLTServer->CPrint( "All goals sorted by Relevance:\n" ); 

		for ( uint32 EachSortedGoal = 0; EachSortedGoal < GoalList.size(); ++EachSortedGoal )
		{
			g_pLTServer->CPrint( " (%-8.3f) %s", 
				GoalList[EachSortedGoal]->fIntrinsicRelevance
				, CAIGoalAbstract::GetGoalTypeName( GoalList[EachSortedGoal]->eGoalType ) );
		}
	}
};

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIMgr::CAIMgr()
{
	m_pAICoordinator = debug_new( CAICoordinator );
	m_pAIPlanner = debug_new( CAIPlanner );
	m_pAIActionMgr = debug_new( CAIActionMgr );
	m_pAIStimulusMgr = debug_new( CAIStimulusMgr );
	m_pAISoundMgr = debug_new( CAISoundMgr );
	m_pAINodeMgr = debug_new( CAINodeMgr );
	m_pAINavMesh = debug_new( CAINavMesh );
	m_pAIQuadTree = debug_new( CAIQuadTree );
	m_pAIPathMgrNavMesh = debug_new( CAIPathMgrNavMesh );
	m_pAICentralMemory = debug_new( CAIWorkingMemoryCentral );
	m_pAITargetSelectMgr = debug_new( CAITargetSelectMgr );

	m_hNextAI = NULL;
}

CAIMgr::~CAIMgr()
{
	debug_delete( m_pAICoordinator );
	debug_delete( m_pAIPlanner );
	debug_delete( m_pAIActionMgr );
	debug_delete( m_pAIStimulusMgr );
	debug_delete( m_pAISoundMgr );
	debug_delete( m_pAINodeMgr );
	debug_delete( m_pAINavMesh );
	debug_delete( m_pAIQuadTree );
	debug_delete( m_pAIPathMgrNavMesh );
	debug_delete( m_pAICentralMemory );
	debug_delete( m_pAITargetSelectMgr );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::InitAI
//
//	PURPOSE:	Initialize AIMgr.
//
// ----------------------------------------------------------------------- //

void CAIMgr::InitAI()
{
	m_pAIActionMgr->InitAIActionMgr();
	m_pAIPlanner->InitAIPlanner();
	m_pAITargetSelectMgr->InitAITargetSelectMgr();
	m_hNextAI = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::TermAI
//
//	PURPOSE:	Terminate AIMgr.
//
// ----------------------------------------------------------------------- //

void CAIMgr::TermAI()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::PreStartWorld
//
//	PURPOSE:	Setup AI things that happen before the new world has started.
//
// ----------------------------------------------------------------------- //

void CAIMgr::PreStartWorld( bool bSwitchingWorlds )
{
	// Only term these things if we are starting a new world.

	if( bSwitchingWorlds )
	{
		m_pAINodeMgr->Term();
		m_pAINavMesh->TermNavMesh();
		m_pAIQuadTree->TermQuadTree();
	}

	// Always term these things.

	m_pAICoordinator->TermAICoordinator();
	m_pAIStimulusMgr->Term();
	m_pAISoundMgr->TermAISoundMgr();
	m_pAIPathMgrNavMesh->TermPathMgrNavMesh();
	m_pAICentralMemory->Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::PostStartWorld
//
//	PURPOSE:	Setup AI things that happen after the world has started.
//
// ----------------------------------------------------------------------- //

void CAIMgr::PostStartWorld()
{
	SetupNavMesh();

	m_pAINavMesh->InitNavMesh();
	m_pAIQuadTree->InitQuadTree();
	m_pAICoordinator->InitAICoordinator();
	m_pAIStimulusMgr->Init();
	m_pAISoundMgr->InitAISoundMgr();
	m_pAICentralMemory->Init();
	m_pAIPathMgrNavMesh->InitPathMgrNavMesh();
	m_pAINodeMgr->Init();

#ifndef _FINAL
	m_pAINodeMgr->Verify();
#endif

	m_hNextAI = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::SetupNavMesh
//
//	PURPOSE:	Setup the NavMesh.
//
// ----------------------------------------------------------------------- //

void CAIMgr::SetupNavMesh()
{
	// The NavMesh only needs to be processed if we are
	// changing worlds.  If we did not change worlds,
	// these functions will bail, because the NavMesh
	// will still have m_bNMInitialized set to true.

	if( !m_pAINavMesh->IsNavMeshInitialized() )
	{
		m_pAINavMesh->SortAINavMeshLinks();

		// Pack the NavMesh in-game.
		// This should be toggle-able once we can pack the NavMesh in the tools.

		CAINavMeshGen AINavMeshGen(NULL);
		
		// get the blind object data
		uint8* pDataRaw = NULL;
		uint32 nDataRawSize = 0;
		if( !CAINavMesh::GetNavMeshBlindObjectData(pDataRaw, nDataRawSize) )
		{
			m_pAINavMesh->TermNavMesh();
			m_pAIQuadTree->SetQuadTreeRoot( NULL );
			return;
		}

		// contains the navmesh data after the header
		uint8* pNavMeshData = pDataRaw + sizeof(uint32);
		uint32 nNavMeshDataSize = nDataRawSize - sizeof(uint32);
		
		if( CAINavMesh::IsNavMeshBlindDataProcessed(pDataRaw, nDataRawSize) )
		{
			// Handle endian conversion, as tools pack in little endian.  
			// Everything in the nav mesh is 32-bit so convert it all in a single 
			// operation.

			LTASSERT( ( (uint32)pNavMeshData % sizeof(uint32) ) == 0, "Unaligned nav mesh pointer encountered" );
			LTASSERT( ( ( ( (uint32)pNavMeshData + nNavMeshDataSize ) ) % sizeof(uint32) ) == 0, "Unaligned nav mesh size encountered" );

			#if defined(PLATFORM_XENON)
				// XENON: Swap data at runtime
				LittleEndianToNative( (uint32*)pNavMeshData, nNavMeshDataSize / sizeof(uint32) );
			#endif // PLATFORM_XENON

			// it's already processed
			m_pAINavMesh->RuntimeSetup( pNavMeshData, false );
		}
		else
		{
			if( AINavMeshGen.ImportRawNavMesh(pNavMeshData) )
			{
				Tuint8List ProcessedData;
				ILTOutStream* pProcessedStream = streamsim_OutMemStream(ProcessedData);
				if(!pProcessedStream)
				{
					m_pAINavMesh->TermNavMesh();
					m_pAIQuadTree->SetQuadTreeRoot( NULL );
					return;
				}

				ILTOutConverter* pProcessedConverter = new LTOutNullConverter(*pProcessedStream);
				if(!pProcessedConverter)
				{
					m_pAINavMesh->TermNavMesh();
					m_pAIQuadTree->SetQuadTreeRoot( NULL );
					return;
				}

				AINavMeshGen.InitNavMeshGen( LTVector( 0.f, 0.f, 0.f ) );
				if( !AINavMeshGen.ExportPackedNavMesh(*pProcessedConverter) )
				{
					m_pAINavMesh->TermNavMesh();
					m_pAIQuadTree->SetQuadTreeRoot( NULL );
					return;
				}
				AINavMeshGen.TermNavMeshGen();

				//release our converter stream
				LTSafeRelease(pProcessedConverter);

				// copy buffer
				uint32 nDataSize = ProcessedData.size();
				uint8* pDataBuffer = debug_newa(uint8, nDataSize);
				if( !pDataBuffer )
				{
					m_pAINavMesh->TermNavMesh();
					m_pAIQuadTree->SetQuadTreeRoot( NULL );
					return;
				}
				memcpy( pDataBuffer, &ProcessedData[0], nDataSize );

				// Initialize the run-time NavMesh with the packed data.
				m_pAINavMesh->RuntimeSetup( pDataBuffer, true );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::Save
//
//	PURPOSE:	Save the AIMgr.
//
// ----------------------------------------------------------------------- //

void CAIMgr::Save( ILTMessage_Write *pMsg )
{
	m_pAIStimulusMgr->Save(pMsg);
	m_pAICentralMemory->Save(pMsg);
	m_pAINodeMgr->Save(pMsg);
	m_pAICoordinator->Save(pMsg);
	m_pAIPathMgrNavMesh->Save(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::Load
//
//	PURPOSE:	Load the AIMgr.
//
// ----------------------------------------------------------------------- //

void CAIMgr::Load( ILTMessage_Read *pMsg )
{
	m_pAIStimulusMgr->Load(pMsg);
	m_pAICentralMemory->Load(pMsg);
	m_pAINodeMgr->Load(pMsg);
	m_pAICoordinator->Load(pMsg);
	m_pAIPathMgrNavMesh->Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::Update
//
//	PURPOSE:	Update the AIMgr.
//
// ----------------------------------------------------------------------- //

void CAIMgr::Update()
{
	// Do not update any AI systems if the game is 
	// paused (e.g. alt-tabbed away).

	if( g_pGameServerShell->IsPaused() )
	{
		return;
	}

	//track our performance

	CTimedSystemBlock TimingBlock(g_tsAIMgr);
	
	// Update the AI's senses.

	g_pAIStimulusMgr->Update();

	// Update the AI's sounds.

	g_pAISoundMgr->UpdateAISoundMgr();

	// Update the AI Coordinator.

	g_pAICoordinator->UpdateAICoordinator();

	// Update AI sensors.

	UpdateAISensors();

	// Collect garbage from Working Memory.

	CollectGarbage();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::UpdateAISensors
//
//	PURPOSE:	Update the AI's sensors.
//
// ----------------------------------------------------------------------- //

void CAIMgr::UpdateAISensors()
{
	CTList<CCharacter*>* plstChars = g_pCharacterMgr->GetCharacterList( CCharacterMgr::kList_AIs );
	CCharacter** pNext;
	CAI* pCurAI;

	// No AI exist.

	if( plstChars->GetLength() == 0 )
	{
		return;
	}

	// Find the next AI to update.
	// AI update round robin, so everyone gets the opportunity to
	// do a distributed update.

	pNext = plstChars->GetItem( TLIT_FIRST );
	while( m_hNextAI && ( (*pNext)->m_hObject != m_hNextAI ) )
	{
		pNext = plstChars->GetItem( TLIT_NEXT );
	}
	if( !pNext )
	{
		pNext = plstChars->GetItem( TLIT_FIRST );
	}

	HOBJECT hFirstToUpdate = (*pNext)->m_hObject;

	// Iterate over all existing AI.

	bool bWrapped = false;
	bool bUpdateDistributedSensors = true;
	while( true )
	{
		// Bail once we have updated everyone.

		if( bWrapped )
		{
			break;
		}

		// Look ahead at the next AI to update, and flag if we've wrapped.

		pCurAI = (CAI*)*pNext;
		pNext = plstChars->GetItem( TLIT_NEXT );
		if( !pNext )
		{
			pNext = plstChars->GetItem( TLIT_FIRST );
		}
		if( (*pNext)->m_hObject == hFirstToUpdate )
		{
			bWrapped = true;
		}

		// Do not update sensors of dead AI.

		if( pCurAI->GetDestructible()->IsDead() )
		{
			continue;
		}

		// Update all AI's sensors.
		// Stop updating distributed sensors once someone has performed
		// an expensive update.

		if( pCurAI->GetAISensorMgr()->UpdateSensors( bUpdateDistributedSensors ) )
		{
#if DISTRIBUTE_SENSORS
			m_hNextAI = (*pNext)->m_hObject;
			bUpdateDistributedSensors = false;
#endif
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::CollectGarbage
//
//	PURPOSE:	Truly delete facts marked for deletion.
//
// ----------------------------------------------------------------------- //

void CAIMgr::CollectGarbage()
{
	// Central memory garbage collection.

	m_pAICentralMemory->CollectGarbage();

	// Individual AI working memory garbage collection.

	CAI::AIList::const_iterator it = CAI::GetAIList().begin();
	for (; it != CAI::GetAIList().end(); ++it)
	{
		CAI* pAI = *it;
		if (pAI)
		{
			pAI->GetAIWorkingMemory()->CollectGarbage();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIMgr::OnDebugCmd
//
//	PURPOSE:	Handle routing AIDebug console commands to the 
//			appropriate subsystem.
//
// ----------------------------------------------------------------------- //

void CAIMgr::OnDebugCmd( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Hand the message to the correct subsystem.

	static CParsedMsg::CToken s_cTok_AIStimulusMgr("AIStimulusMgr");
	static CParsedMsg::CToken s_cTok_AIWorkingMemoryCentral("AIWorkingMemoryCentral");
	static CParsedMsg::CToken s_cTok_ListGoals("ListGoals");
	static CParsedMsg::CToken s_cTok_ListUnusedActions("ListUnusedActions");
	static CParsedMsg::CToken s_cTok_ListUnusedGoals("ListUnusedGoals");

	if ( crParsedMsg.GetArg(0) == s_cTok_AIStimulusMgr )
	{
		m_pAIStimulusMgr->OnAIDebugCmd( hSender, crParsedMsg );
	}
	else if ( crParsedMsg.GetArg(0) == s_cTok_AIWorkingMemoryCentral )
	{
		m_pAICentralMemory->OnAIDebugCmd( hSender, crParsedMsg );
	}
	else if ( crParsedMsg.GetArg(0) == s_cTok_ListGoals )
	{
		AIDebugUtils::PrintGoals();
	}	
	else if ( crParsedMsg.GetArg(0) == s_cTok_ListUnusedActions )
	{
		AIDebugUtils::PrintUnusedActions();
	}
	else if ( crParsedMsg.GetArg(0) == s_cTok_ListUnusedGoals )
	{
		AIDebugUtils::PrintUnusedGoals();
	}
	else
	{
		g_pLTServer->CPrint( "No AI command named: %s", crParsedMsg.GetArg(0).c_str() );
	}
}

