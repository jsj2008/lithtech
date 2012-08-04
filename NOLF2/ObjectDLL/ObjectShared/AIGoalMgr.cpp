// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMgr.cpp
//
// PURPOSE : AIGoalMgr implementation
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIGoalMgr.h"
#include "AIGoalAbstract.h"
#include "AISenseRecorderAbstract.h"
#include "AI.h"
#include "AIState.h"
#include "AIGoalButeMgr.h"
#include "AnimationMgr.h"
#include "ParsedMsg.h"
#include "AIUtils.h"
#include "AIMovement.h"
#include "AIBrain.h"

#define GOAL_CMD_PREFIX		"GOAL_"
#define GOAL_CMD_GOALSET	"GoalSet"
#define GOAL_CMD_ADDGOAL	"AddGoal"
#define GOAL_CMD_REMOVEGOAL	"RemoveGoal"
#define GOAL_CMD_GOALSCRIPT	"GoalScript"

DEFINE_AI_FACTORY_CLASS(CAIGoalMgr);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::Constructor/Destructor
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIGoalMgr::CAIGoalMgr()
{
	m_pAI = LTNULL;
	Init( LTNULL );
}

CAIGoalMgr::~CAIGoalMgr()
{
	Term( LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::Save/Load
//
//	PURPOSE:	Save/Load
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::Save(ILTMessage_Write *pMsg)
{
	// Save list of goals.

	SAVE_DWORD(m_lstGoals.size());

	CAIGoalAbstract* pGoal;
	AIGOAL_LIST::iterator it;
	AIGOAL_DAMAGE_MAP::iterator dit;
	uint32 dwDamagePriority;

	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = *it;
		SAVE_DWORD(pGoal->GetGoalType());
		pGoal->Save(pMsg);

		// See if this goal is a damage handler and save the damage priority if it is...

		dwDamagePriority = 0;
		for( dit = m_mapDamageHandlers.begin(); dit != m_mapDamageHandlers.end(); ++dit )
		{
			if( dit->second == pGoal )
			{
				dwDamagePriority = dit->first;
			}
		}

		SAVE_DWORD( dwDamagePriority );
	}

	// Save pointers.

	SAVE_DWORD( m_pNextGoal ? m_pNextGoal->GetGoalType() : kGoal_InvalidType );
	SAVE_DWORD( m_pCurGoal ? m_pCurGoal->GetGoalType() : kGoal_InvalidType );
	SAVE_DWORD( m_pLastGoal ? m_pLastGoal->GetGoalType() : kGoal_InvalidType );
	SAVE_DWORD( m_pTriggeredGoal ? m_pTriggeredGoal->GetGoalType() : kGoal_InvalidType );
	SAVE_DWORD( m_pLockedGoal ? m_pLockedGoal->GetGoalType() : kGoal_InvalidType );
	SAVE_DWORD( m_pDeactivatingGoal ? m_pDeactivatingGoal->GetGoalType() : kGoal_InvalidType );


	// Save goal script.

	SAVE_DWORD(m_lstScriptGoals.size());

	GOAL_SCRIPT_STRUCT* gss;
	GOAL_SCRIPT_LIST::iterator gsit;
	for(gsit = m_lstScriptGoals.begin(); gsit != m_lstScriptGoals.end(); ++gsit)
	{
		gss = &(*gsit);
		SAVE_DWORD(gss->eGoalType);
		SAVE_HSTRING(gss->hstrNameValuePairs);
	}

	SAVE_DWORD(m_nScriptStep);
	SAVE_BOOL(m_bQueuedGoalScript);

	// Save queued goals.

	SAVE_DWORD( m_lstQueuedGoals.size() );
	QUEUED_AIGOAL_LIST::iterator qgit;
	for(qgit = m_lstQueuedGoals.begin(); qgit != m_lstQueuedGoals.end(); ++qgit)
	{
		SAVE_DWORD( *qgit );
	}

	QUEUED_AIGOAL_ARG_LIST::iterator qgait;
	for(qgait = m_lstQueuedGoalArgs.begin(); qgait != m_lstQueuedGoalArgs.end(); ++qgait)
	{
		SAVE_HSTRING( *qgait );
	}

	// Save queued goal prefix commands.

	SAVE_DWORD( m_lstQueuedPrefixCmdGoals.size() );
	for(qgit = m_lstQueuedPrefixCmdGoals.begin(); qgit != m_lstQueuedPrefixCmdGoals.end(); ++qgit)
	{
		SAVE_DWORD( *qgit );
	}

	for(qgait = m_lstQueuedPrefixCmdGoalArgs.begin(); qgait != m_lstQueuedPrefixCmdGoalArgs.end(); ++qgait)
	{
		SAVE_HSTRING( *qgait );
	}

	SAVE_DWORD(m_iGoalSet);
	SAVE_TIME(m_fGoalSetTime);
	SAVE_DWORD(m_iQueuedGoalSet);

	SAVE_BOOL(m_bKillDialogue);
}

void CAIGoalMgr::Load(ILTMessage_Read *pMsg)
{
	// Load list of goals.

	uint32 cGoals;
	LOAD_DWORD(cGoals);

	CAIGoalAbstract* pGoal;
	EnumAIGoalType eGoalType;
	uint32 dwDamagePriority;
	for(uint32 iGoal=0; iGoal < cGoals; ++iGoal)
	{
		LOAD_DWORD_CAST(eGoalType, EnumAIGoalType);
		pGoal = AI_FACTORY_NEW_Goal( eGoalType );
		AIASSERT(pGoal != LTNULL, m_pAI->m_hObject, "CAIGoalMgr::Load: Failed to create goal.");

		pGoal->InitGoal(m_pAI);
		pGoal->Load(pMsg);
		m_lstGoals.push_back(pGoal);

		LOAD_DWORD( dwDamagePriority );
		if( dwDamagePriority > 0 )
		{
			m_mapDamageHandlers.insert( AIGOAL_DAMAGE_MAP::value_type( dwDamagePriority, pGoal ));
		}
	}

	// Load pointers.

	LOAD_DWORD_CAST( eGoalType, EnumAIGoalType );
	SetNextGoal( FindGoalByType( eGoalType ) );

	LOAD_DWORD_CAST( eGoalType, EnumAIGoalType );
	m_pCurGoal = FindGoalByType( eGoalType );

	LOAD_DWORD_CAST( eGoalType, EnumAIGoalType );
	m_pLastGoal = FindGoalByType( eGoalType );
	LOAD_DWORD_CAST( eGoalType, EnumAIGoalType );
	m_pTriggeredGoal = FindGoalByType( eGoalType );
	LOAD_DWORD_CAST( eGoalType, EnumAIGoalType );
	m_pLockedGoal = FindGoalByType( eGoalType );
	LOAD_DWORD_CAST( eGoalType, EnumAIGoalType );
	m_pDeactivatingGoal = FindGoalByType( eGoalType );

	// Load goal script.

	uint32 cGoalScriptSteps;
	LOAD_DWORD(cGoalScriptSteps);
	m_lstScriptGoals.resize(cGoalScriptSteps);

	GOAL_SCRIPT_STRUCT* gss;
	GOAL_SCRIPT_LIST::iterator gsit;
	for(gsit = m_lstScriptGoals.begin(); gsit != m_lstScriptGoals.end(); ++gsit)
	{
		gss = &(*gsit);
		LOAD_DWORD_CAST(gss->eGoalType, EnumAIGoalType);
		LOAD_HSTRING(gss->hstrNameValuePairs);
	}

	LOAD_DWORD(m_nScriptStep);
	LOAD_BOOL(m_bQueuedGoalScript);


	// Load queued goals.

	LOAD_DWORD( cGoals );
	for(int iGoal=0; iGoal < cGoals; ++iGoal )
	{
		LOAD_DWORD_CAST( eGoalType, EnumAIGoalType);
		m_lstQueuedGoals.push_back( eGoalType );
	}

	HSTRING hstrQueuedGoalArgs;
	for(int iGoal=0; iGoal < cGoals; ++iGoal )
	{
		LOAD_HSTRING( hstrQueuedGoalArgs );
		m_lstQueuedGoalArgs.push_back( hstrQueuedGoalArgs );
	}

	// Load queued goal prefix commands.

	LOAD_DWORD( cGoals );
	for(int iGoal=0; iGoal < cGoals; ++iGoal )
	{
		LOAD_DWORD_CAST( eGoalType, EnumAIGoalType);
		m_lstQueuedPrefixCmdGoals.push_back( eGoalType );
	}

	for(int iGoal=0; iGoal < cGoals; ++iGoal )
	{
		LOAD_HSTRING( hstrQueuedGoalArgs );
		m_lstQueuedPrefixCmdGoalArgs.push_back( hstrQueuedGoalArgs );
	}


	LOAD_DWORD(m_iGoalSet);
	LOAD_TIME(m_fGoalSetTime);
	LOAD_DWORD(m_iQueuedGoalSet);

	LOAD_BOOL(m_bKillDialogue);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::Init
//
//	PURPOSE:	Initialization.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::Init(CAI* pAI)
{
	m_pAI = pAI;

	m_pNextGoal			= LTNULL;
	m_pCurGoal			= LTNULL;
	m_pLastGoal			= LTNULL;
	m_pTriggeredGoal	= LTNULL;
	m_pLockedGoal		= LTNULL;
	m_pDeactivatingGoal	= LTNULL;
	m_nScriptStep		= 0;
	m_bQueuedGoalScript	= LTFALSE;

	m_iGoalSet			= 0;
	m_fGoalSetTime		= 0.f;
	m_iQueuedGoalSet	= -1;

	m_bKillDialogue		= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::Term
//
//	PURPOSE:	Termination.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::Term(LTBOOL bDestroyAll)
{
	if( bDestroyAll || !m_bQueuedGoalScript )
	{
		ClearGoalScript();
	}

	// Delete all goals.
	// Use RemoveGoal so that all goal removals follow the same
	// code path.  When SetGoalSet is called, it may call Term()
	// to clear the goals.  Calling RemoveGoal ensures that there
	// are no dangling pointers.

	CAIGoalAbstract* pGoal;
	AIGOAL_LIST::iterator it = m_lstGoals.begin();
	while( it != m_lstGoals.end() )
	{
		pGoal = (*it);
		if( !bDestroyAll && pGoal->IsPermanentGoal() )
		{
			++it;
		}
		else {
			AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Removing Goal %s.", s_aszGoalTypes[pGoal->GetGoalType()] ) );
			RemoveGoal( pGoal->GetGoalType() );
			it = m_lstGoals.begin();
		}
	}

	for( it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it )
	{
		pGoal = (*it);
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "Goal %s is Permanent. NOT removing.", s_aszGoalTypes[pGoal->GetGoalType()] ) );
	}


	if( bDestroyAll )
	{
		// Cleanup queued goals.

		m_lstQueuedGoals.clear();
		QUEUED_AIGOAL_ARG_LIST::iterator qgait;
		for(qgait = m_lstQueuedGoalArgs.begin(); qgait != m_lstQueuedGoalArgs.end(); ++qgait)
		{
			FREE_HSTRING( *qgait );
		}
		m_lstQueuedGoalArgs.clear();


		// Cleanup queued goal prefix commands.

		m_lstQueuedPrefixCmdGoals.clear();
		for(qgait = m_lstQueuedPrefixCmdGoalArgs.begin(); qgait != m_lstQueuedPrefixCmdGoalArgs.end(); ++qgait)
		{
			FREE_HSTRING( *qgait );
		}
		m_lstQueuedPrefixCmdGoalArgs.clear();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::SetGoalSetNextUpdate
//
//	PURPOSE:	Change goalset next update
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::SetGoalSetNextUpdate(const char* szGoalSet)
{
	m_iQueuedGoalSet = g_pAIGoalButeMgr->GetGoalSetIndex( szGoalSet );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::SetGoalSet()
//
//	PURPOSE:	Set current goal set.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::SetGoalSet(const char* szGoalSet, LTBOOL bClearGoals)
{
	// Get the GoalSet from the bute mgr.
	uint32 iGoalSet = g_pAIGoalButeMgr->GetGoalSetIndex( szGoalSet );
	AIASSERT( iGoalSet != -1, m_pAI->m_hObject, "CAIGoalMgr::SetGoalSet: Invalid GoalSet name." );

	SetGoalSet( iGoalSet, bClearGoals );
}

void CAIGoalMgr::SetGoalSet(uint32 iGoalSet, LTBOOL bClearGoals)
{
	if( iGoalSet != -1 )
	{
		AIGBM_GoalSet* pGoalSet = g_pAIGoalButeMgr->GetGoalSet( iGoalSet );

		// Check if GoalSet requires a specific brain.

		if( !pGoalSet->lstRequiredBrains.empty() )
		{
			int iBrainID = g_pAIButeMgr->GetBrainIDByName( m_pAI->GetBrain()->GetName() );

			AIBRAIN_LIST::iterator it_brain;
			for( it_brain = pGoalSet->lstRequiredBrains.begin(); it_brain != pGoalSet->lstRequiredBrains.end(); ++it_brain )
			{
				// Found a matching brain.

				if( iBrainID == *it_brain )
				{
					break;
				}
			}

			// No matching brain was found. Bail!

			if( it_brain == pGoalSet->lstRequiredBrains.end() )
			{
				AIASSERT1( 0, m_pAI->m_hObject, "Cannot set GoalSet %s. No required brain found in list.", pGoalSet->szName );
				return;
			}
		}

		// Delete existing goals if flag is set.

		if( bClearGoals )
		{
			// Init will clear the QueuedGoalScript flag, so store it in a temp.

			LTBOOL bQueuedGoalScript = m_bQueuedGoalScript;

			Term( LTFALSE );
			Init( m_pAI );

			m_bQueuedGoalScript = bQueuedGoalScript;

			// Allow transitions to finish.  Stomp anything else.

			if( !m_pAI->GetAnimationContext()->IsTransitioning() )
			{
				m_pAI->GetAnimationContext()->ClearSpecial();
			}
		}

		// Call self recursively for included goalsets.

		AIGBM_GoalSet* pIncludedGoalSet;
		AIGOAL_SET_LIST::iterator it_gs;
		for( it_gs = pGoalSet->lstIncludeGoalSets.begin(); it_gs != pGoalSet->lstIncludeGoalSets.end(); ++it_gs )
		{
			pIncludedGoalSet = *it_gs;
			if( pIncludedGoalSet )
			{
				SetGoalSet( pIncludedGoalSet->szName, LTFALSE );
			}
		}

		m_iGoalSet = iGoalSet;
		m_fGoalSetTime = g_pLTServer->GetTime();

		// Check flag for permanent goals.

		LTBOOL bPermanent = pGoalSet->dwGoalSetFlags & AIGBM_GoalSet::kGS_Permanent;

		// Create goals for every entry in map.

		char szNameValuePair[64];
		char *szName, *szValue;

		CAIGoalAbstract* pGoal;
		AIGBM_GoalTemplate* pGoalTemplate;
		SGoalSetData gsd;
		LTFLOAT fTime = g_pLTServer->GetTime();
		AIGOAL_DATA_MAP::iterator it;
		for(it = pGoalSet->mapGoalSet.begin(); it != pGoalSet->mapGoalSet.end(); ++it)
		{
			gsd = it->second;
			pGoalTemplate = g_pAIGoalButeMgr->GetTemplate(it->first);
			pGoal = AddGoal(it->first, pGoalTemplate->fImportance, fTime, LTFALSE);

			pGoal->SetPermanentGoal( bPermanent );

			// Process any additional name value pairs for this goal.
			if(gsd.hstrParams != LTNULL)
			{
				strcpy(szNameValuePair, g_pLTServer->GetStringData(gsd.hstrParams));
				szName = strtok(szNameValuePair, "=");
				while(szName != LTNULL)
				{
					szValue = strtok(LTNULL, " ");
					pGoal->HandleNameValuePair(szName, szValue);
					szName = strtok(LTNULL, "=");
				}
			}
		}
	}

	// Retrigger goals with currently stimulated senses, in
	// case a sense triggered a goalset change.  New goals
	// need to respond to current stimulus.

	m_pAI->GetSenseRecorder()->RepeatHandleSenses();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::AI_FACTORY_NEW_Goal
//
//	PURPOSE:	Create a goal
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract* CAIGoalMgr::AI_FACTORY_NEW_Goal(EnumAIGoalType eGoalType)
{
	// Call AI_FACTORY_NEW for the requested type of goal.

	switch( eGoalType )
	{
		#define GOAL_TYPE_AS_SWITCH 1
		#include "AIGoalTypeEnums.h"
		#undef GOAL_TYPE_AS_SWITCH

		default: AIASSERT( 0, m_pAI->m_hObject, "CAIGoalMgr::CreateFactoryGoal: Unrecognized goal type.");
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::AddGoal
//
//	PURPOSE:	Add a goal
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract* CAIGoalMgr::AddGoal(EnumAIGoalType eGoalType, LTFLOAT fImportance,
									 LTFLOAT fTime, LTBOOL bScripted)
{
	CAIGoalAbstract* pGoal = LTNULL;

	ASSERT(eGoalType != kGoal_InvalidType);
	if( eGoalType != kGoal_InvalidType )
	{
		// Check if AI already has this goal.
		AIGOAL_LIST::iterator it;
		for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
		{
			pGoal = *it;

			// If AI has the goal, return it.
			if(pGoal->GetGoalType() == eGoalType)
			{
				break;
			}
		}

		if( it == m_lstGoals.end() )
		{
			// Add goal to list of goals.
			pGoal = LTNULL;
			pGoal = AI_FACTORY_NEW_Goal( eGoalType );
			if( pGoal == LTNULL )
			{
				AIASSERT( 0, m_pAI->m_hObject, "CAIGoalMgr::AddGoal: Failed to create goal.");
				return LTNULL;
			}

			m_lstGoals.push_back(pGoal);

			// If Goal handles damage, add it to the map of damage handler goals.
			// Map is sorted by DamagePriority.

			AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( pGoal->GetGoalType() );
			AIASSERT( pTemplate, m_pAI->m_hObject, "CAIGoalMgr::AddGoal: Could not find GoalBute template." );
			if( pTemplate && ( pTemplate->nDamagePriority > 0 ) )
			{
				m_mapDamageHandlers.insert( AIGOAL_DAMAGE_MAP::value_type( pTemplate->nDamagePriority, pGoal ) );
			}

			// Initialize goal.
			pGoal->InitGoal(m_pAI, fImportance, fTime);

			pGoal->SetScripted(bScripted);
		}

		// Reset last update time to the current time.
		// This allows other code to detect if a goal was added during the same update.

		pGoal->SetLastUpdateTime( fTime );

		// Clear flag to delete goal next update.

		pGoal->SetDeleteGoalNextUpdate( LTFALSE );
	}

	return pGoal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::AddQueuedGoals
//
//	PURPOSE:	Add a queued goals
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::AddQueuedGoals()
{
	CAIGoalAbstract* pGoal = LTNULL;

	EnumAIGoalType eGoalType;
	HSTRING hstrGoalArgs;

	QUEUED_AIGOAL_ARG_LIST::iterator qgait;
	QUEUED_AIGOAL_LIST::iterator qgit;

	qgait = m_lstQueuedGoalArgs.begin();
	for(qgit = m_lstQueuedGoals.begin(); qgit != m_lstQueuedGoals.end(); ++qgit)
	{
		eGoalType = *qgit;
		hstrGoalArgs = *qgait;

		AIGBM_GoalTemplate* pGoalTemplate = g_pAIGoalButeMgr->GetTemplate( eGoalType );
		AIASSERT( pGoalTemplate, m_pAI->m_hObject, "CAIGoalMgr::AddGoal: Cannot find goal template." );
		if( pGoalTemplate )
		{
			pGoal = AddGoal( eGoalType, pGoalTemplate->fImportance, g_pLTServer->GetTime(), LTFALSE );
			AIASSERT( pGoal, m_pAI->m_hObject, "CAIGoalMgr::AddQueuedGoals: Failed to add goal." );
			if( pGoal )
			{
				AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Added Goal: %s", s_aszGoalTypes[eGoalType] ) );

				// Process any additional name value pairs for this goal.
				if( hstrGoalArgs )
				{
					ConParse parse;
					parse.Init( g_pLTServer->GetStringData( hstrGoalArgs ) );

					if( g_pCommonLT->Parse( &parse ) == LT_OK )
					{
						CParsedMsg cSubMsg( parse.m_nArgs, parse.m_Args );
						pGoal->HandleCommand(cSubMsg);
					}
				}
			}
		}

		++qgait;
	}


	// Clear queued goals.

	m_lstQueuedGoals.clear();
	for(qgait = m_lstQueuedGoalArgs.begin(); qgait != m_lstQueuedGoalArgs.end(); ++qgait)
	{
		FREE_HSTRING( *qgait );
	}
	m_lstQueuedGoalArgs.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::RunQueuedPrefixGoalCommands
//
//	PURPOSE:	Run queued prefix goal commands
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::RunQueuedPrefixGoalCommands()
{
	CAIGoalAbstract* pGoal = LTNULL;

	EnumAIGoalType eGoalType;
	HSTRING hstrGoalArgs;

	QUEUED_AIGOAL_ARG_LIST::iterator qgait;
	QUEUED_AIGOAL_LIST::iterator qgit;

	qgait = m_lstQueuedPrefixCmdGoalArgs.begin();
	for(qgit = m_lstQueuedPrefixCmdGoals.begin(); qgit != m_lstQueuedPrefixCmdGoals.end(); ++qgit)
	{
		eGoalType = *qgit;
		hstrGoalArgs = *qgait;

		pGoal = FindGoalByType( eGoalType );
		if( pGoal && hstrGoalArgs )
		{
			ConParse parse;
			parse.Init( g_pLTServer->GetStringData( hstrGoalArgs ) );

			if( g_pCommonLT->Parse( &parse ) == LT_OK )
			{
				CParsedMsg cMsg( parse.m_nArgs, parse.m_Args );
				pGoal->HandleCommand(cMsg);
			}
		}

		++qgait;
	}

	// Clear queued prefix goal commands.

	m_lstQueuedPrefixCmdGoals.clear();
	for(qgait = m_lstQueuedPrefixCmdGoalArgs.begin(); qgait != m_lstQueuedPrefixCmdGoalArgs.end(); ++qgait)
	{
		FREE_HSTRING( *qgait );
	}
	m_lstQueuedPrefixCmdGoalArgs.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::RemoveGoal
//
//	PURPOSE:	Remove a goal
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::RemoveGoal(EnumAIGoalType eGoalType)
{
	ASSERT(eGoalType != kGoal_InvalidType);

	// Find goal of specified type.
	CAIGoalAbstract* pGoal;
	AIGOAL_LIST::iterator it;
	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = *it;

		if(pGoal->GetGoalType() == eGoalType)
		{
			// Nullify any pointers to this goal.
			if(m_pNextGoal == pGoal)
			{
				SetNextGoal( LTNULL );
			}
			if(m_pCurGoal == pGoal)
			{
				m_pCurGoal =  LTNULL;
			}
			if(m_pLastGoal == pGoal)
			{
				m_pLastGoal = LTNULL;
			}
			if(m_pTriggeredGoal == pGoal)
			{
				m_pTriggeredGoal = LTNULL;
			}
			if(m_pLockedGoal == pGoal)
			{
				m_pLockedGoal = LTNULL;
			}
			if(m_pDeactivatingGoal == pGoal)
			{
				m_pDeactivatingGoal = LTNULL;
			}

			// Remove goal from map of damage handlers.

			AIGOAL_DAMAGE_MAP::iterator dit;
			for( dit = m_mapDamageHandlers.begin(); dit != m_mapDamageHandlers.end(); ++dit )
			{
				if( dit->second == pGoal )
				{
					m_mapDamageHandlers.erase( dit );
					break;
				}
			}

			// Delete the goal.
			AI_FACTORY_DELETE(pGoal);
			m_lstGoals.erase(it);

			return;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::DeleteGoal
//
//	PURPOSE:	Delete a goal next update
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::DeleteGoalNextUpdate(EnumAIGoalType eGoalType)
{
	ASSERT(eGoalType != kGoal_InvalidType);

	// Find goal of specified type.
	CAIGoalAbstract* pGoal;
	AIGOAL_LIST::iterator it;
	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = *it;
		if(pGoal->GetGoalType() == eGoalType)
		{
			pGoal->SetDeleteGoalNextUpdate( LTTRUE );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::LockGoal
//
//	PURPOSE:	Lock goal
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::LockGoal(CAIGoalAbstract* pGoal)
{
	AIASSERT( (m_pLockedGoal == LTNULL) || (m_pLockedGoal == pGoal), m_pAI->m_hObject, "CAIGoalMgr::LockGoal: There is already a locked goal.");
	m_pLockedGoal = pGoal;

	AITRACE(AIShowGoalsVerbose, ( m_pAI->m_hObject, "Locking Goal %s\n", s_aszGoalTypes[pGoal->GetGoalType()] ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::UnlockGoal
//
//	PURPOSE:	Unlock goal
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::UnlockGoal(CAIGoalAbstract* pGoal)
{
	if( m_pLockedGoal == pGoal )
	{
		m_pLockedGoal = LTNULL;
		AITRACE(AIShowGoalsVerbose, ( m_pAI->m_hObject, "Unlocking Goal %s\n", s_aszGoalTypes[pGoal->GetGoalType()] ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::UpdateGoals
//
//	PURPOSE:	Update goals
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::UpdateGoals()
{
//	AITRACE(AIShowGoalsVerbose, ( m_pAI->m_hObject, "CurGoal %s\n", m_pCurGoal ? s_aszGoalTypes[m_pCurGoal->GetGoalType()] : "Null" ) );
//	AITRACE(AIShowGoalsVerbose, ( m_pAI->m_hObject, "NextGoal %s\n", m_pNextGoal ? s_aszGoalTypes[m_pNextGoal->GetGoalType()] : "Null") );
//	AITRACE(AIShowGoalsVerbose, ( m_pAI->m_hObject, "LastGoal %s\n", m_pLastGoal ? s_aszGoalTypes[m_pLastGoal->GetGoalType()] : "Null") );

	if( m_pAI->IsDead() )
	{
		return;
	}

	// Immediately kill any dialogue.

	if( m_bKillDialogue )
	{
		if( m_pAI->IsControlledByDialogue() )
		{
			m_pAI->StopDialogue();
			m_pAI->KillDialogueSound();
		}

		m_bKillDialogue = LTFALSE;
	}

	// Change to queued goalset.

	if( m_iQueuedGoalSet != -1 )
	{
		SetGoalSet( m_iQueuedGoalSet, LTTRUE );
		m_iQueuedGoalSet = -1;
	}

	// Kick off a queued goalscript.

	if( m_bQueuedGoalScript )
	{
		HandleGoalScript();
	}

	// Add a queued goal.

	if( !m_lstQueuedGoals.empty() )
	{
		AddQueuedGoals();
	}

	// Run queued prefix goal commands.

	if( !m_lstQueuedPrefixCmdGoals.empty() )
	{
		RunQueuedPrefixGoalCommands();
	}


	if(m_lstGoals.size() > 0)
	{
		LTFLOAT fTime = g_pLTServer->GetTime();

		// Update Importance values.
		if( !m_pDeactivatingGoal )
		{
			// Update goal importances.

			UpdateGoalTimers(fTime);

			// Check if current goal has dropped to zero importance.

			if( m_pCurGoal
				&& ( !m_pAI->GetAnimationContext()->IsLocked() )
				&& ( !m_pAI->GetAnimationContext()->IsTransitioning() )
				&& ( !m_pAI->GetAIMovement()->IsMovementLocked() )
				&& ( m_pCurGoal->GetCurImportance() <= 0.f ) )
			{
				SetNextGoal( LTNULL );
				m_pLockedGoal	= LTNULL;
				ChooseCurGoal();
			}
		}

		// Check for change in active goal.
		if(m_pNextGoal != m_pLastGoal)
		{
			// Deactivate last goal.
			// Delete last goal if flagged for deletion.
			if(m_pLastGoal != LTNULL)
			{
				// Advance the script, if the last scripted goal sucessfully completed.
				// If another goal interrupted the script, the scripted goal will
				// have an importance greater than 0.

				if( (!m_pDeactivatingGoal) && m_pLastGoal->IsScripted() && ( m_pLastGoal->GetCurImportance() == 0.f ) )
				{
					++m_nScriptStep;
				}

	 			m_pLastGoal->DeactivateGoal();

				// Some goals need to do something on deactivation.
				// (e.g. work goals may play an interruption animation).
				// Bail until deactivation is complete.

				if( IsGoalLocked( m_pLastGoal ) )
				{
					m_pDeactivatingGoal = m_pLastGoal;
					return;
				}

				m_pDeactivatingGoal = LTNULL;
				m_pAI->ClearAndSetState( kState_HumanIdle );
				AIGBM_GoalTemplate* pTemplate = g_pAIGoalButeMgr->GetTemplate( m_pLastGoal->GetGoalType() );
				if(pTemplate->bDeleteOnDeactivation)
				{
					RemoveGoal(m_pLastGoal->GetGoalType());
				}
			}

			if(m_lstScriptGoals.size() > 0)
			{
				HandleGoalScript();
			}

			m_pCurGoal = m_pNextGoal;
			if( m_pCurGoal )
			{
				m_pCurGoal->ActivateGoal();
			}

			m_pLastGoal = m_pCurGoal;
		}

		// Active goal was re-triggered.
		else if( m_pTriggeredGoal && ( !m_pAI->GetAIMovement()->IsMovementLocked() ) )
		{
			m_pTriggeredGoal->ActivateGoal();
		}

		m_pTriggeredGoal = LTNULL;

		// Update the most importance goal.
		if( ( m_pCurGoal != LTNULL ) && ( m_pCurGoal->GetCurImportance() > 0.f ) )
		{
			m_pCurGoal->UpdateGoal();
		}
		else if( ( !m_pAI->GetAIMovement()->IsMovementLocked() )
			&& ( !m_pAI->GetAnimationContext()->IsLocked() )
			&& ( !m_pAI->GetAnimationContext()->IsTransitioning() ) )
		{
			m_pAI->SetAwareness( kAware_Relaxed );

			if( (m_pAI->GetState() == LTNULL) || (m_pAI->GetState()->GetStateType() != kState_HumanIdle) )
			{
				m_pAI->SetState( kState_HumanIdle );
			}
		}

		// Delete any goals flagged for deletion.

		CAIGoalAbstract* pGoal;
		AIGOAL_LIST::iterator it = m_lstGoals.begin();
		while( it != m_lstGoals.end() )
		{
			pGoal = (*it);
			if( pGoal->GetDeleteGoalNextUpdate() )
			{
				RemoveGoal( pGoal->GetGoalType() );
			}
			else {
				++it;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::SetNextGoal
//
//	PURPOSE:	Set current goal. Access function centralizes
//              changes to curGoal for easier debugging!
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::SetNextGoal(CAIGoalAbstract* pGoal)
{
	m_pNextGoal = pGoal;

	// Kill current state if no goal is active.

	if( !m_pNextGoal )
	{
		m_pAI->ClearAndSetState( kState_HumanIdle );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::ChooseCurGoal
//
//	PURPOSE:	Choose current goal based on importances.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::ChooseCurGoal()
{
	AIGOAL_LIST::iterator it;

	LTFLOAT fImportance = 0.f;
	LTFLOAT fCurMax = 0.f;

	// Keep track of current most important goal.

	CAIGoalAbstract* pGoal;
	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = (*it);
		pGoal->RecalcImportance();
		fImportance = pGoal->GetCurImportance();

		if( fImportance > fCurMax )
		{
			SetNextGoal( pGoal );
			fCurMax	= pGoal->GetCurImportance();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::UpdateGoalTimers
//
//	PURPOSE:	Update goal's timers.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::UpdateGoalTimers(LTFLOAT fTime)
{
	AIGOAL_LIST::iterator it;

	LTFLOAT fImportance = 0.f;
	LTFLOAT fCurMax = m_pCurGoal ? m_pCurGoal->GetCurImportance() : 0.f;
	if( fCurMax < 0.f )
	{
		fCurMax = 0.f;
	}

	// Update goals, and keep track of current most important goal.

	CAIGoalAbstract* pGoal;
	CAIGoalAbstract* pNewCurGoal = LTNULL;
	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = (*it);

		fImportance = pGoal->UpdateGoalTimer(fTime);


		// This goal is eligiable to become the CurGoal.
		// Goals may never change while movement is locked.

		if( ( fImportance > fCurMax ) &&
			( !m_pAI->GetAIMovement()->IsMovementLocked() ) )
		{
			// Goals may not change in the middle of locked or transition
			// animations, unless the current goal allows interrupts or the
			// new goal forces interrupts.

			if( ( m_pAI->GetAnimationContext()->IsLocked() ||
				  m_pAI->GetAnimationContext()->IsTransitioning() ) &&
				( !pGoal->DoesForceAnimInterrupt() ) &&
				( m_pCurGoal && !m_pCurGoal->IsLockedAnimInterruptable() ) )
			{
				// Some situations require an immediate response
				// (e.g. AI getting shot) no matter what the animation
				// is doing.

				if( !pGoal->RequiresImmediateResponse() )
				{
					continue;
				}
			}

			// All previous early-outs failed, so record this goal as
			// the next.

			pNewCurGoal = pGoal;
			fCurMax	= pGoal->GetCurImportance();
		}

		pGoal->ClearImmediateResponse();
	}

	if( pNewCurGoal )
	{
		SetNextGoal( pNewCurGoal );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::HandleGoalScript
//
//	PURPOSE:	Set the next goal from a script.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::HandleGoalScript()
{
	m_bQueuedGoalScript = LTFALSE;

	// Check if the script has completed.
	if(m_nScriptStep >= m_lstScriptGoals.size())
	{
		ClearGoalScript();
		return;
	}

	// Add the next goal from the script list.
	GOAL_SCRIPT_STRUCT pgss = m_lstScriptGoals[m_nScriptStep];

	AIGBM_GoalTemplate* pGoalTemplate = g_pAIGoalButeMgr->GetTemplate( pgss.eGoalType );
	CAIGoalAbstract* pGoal = AddGoal(pgss.eGoalType, pGoalTemplate->fImportance, g_pLTServer->GetTime(), LTTRUE);

	// Pass parameters to goals.
	if(pgss.hstrNameValuePairs != LTNULL)
	{
		char szNameValuePairs[128];
		strcpy(szNameValuePairs, g_pLTServer->GetStringData(pgss.hstrNameValuePairs));

		char *szName, *szValue;
		szName = strtok(szNameValuePairs, "=");
		while(szName != LTNULL)
		{
			szValue = strtok(LTNULL, " ");
			pGoal->HandleNameValuePair(szName, szValue);
			szName = strtok(LTNULL, "=");
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::ClearGoalScript
//
//	PURPOSE:	Clear a goal script.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::ClearGoalScript()
{
	GOAL_SCRIPT_LIST::iterator it;
	for(it = m_lstScriptGoals.begin(); it != m_lstScriptGoals.end(); ++it)
	{
		FREE_HSTRING(it->hstrNameValuePairs);
	}
	m_lstScriptGoals.clear();

	m_nScriptStep = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::HandleGoalSenseTriggers
//
//	PURPOSE:	Pass senses to goals
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::HandleGoalSenseTriggers(AISenseRecord* pSenseRecord)
{
	AIASSERT(pSenseRecord && (pSenseRecord->eSenseType != kSense_InvalidType), m_pAI->m_hObject, "CAIGoalMgr::HandleGoalSenseTriggers: Invalid sense type.");

	// Change to queued goalset.

	if( m_iQueuedGoalSet != -1 )
	{
		SetGoalSet( m_iQueuedGoalSet, LTTRUE );
		m_iQueuedGoalSet = -1;
	}

	// Kick off a queued goalscript.

	if( m_bQueuedGoalScript )
	{
		HandleGoalScript();
	}

	// Add a queued goal.

	if( !m_lstQueuedGoals.empty() )
	{
		AddQueuedGoals();
	}

	// Run queued prefix goal commands.

	if( !m_lstQueuedPrefixCmdGoals.empty() )
	{
		RunQueuedPrefixGoalCommands();
	}


	AIGOAL_LIST::iterator it;

	LTFLOAT fImportance = 0.f;
	LTFLOAT fCurMax = m_pCurGoal ? m_pCurGoal->GetCurImportance() : 0.f;
	if( fCurMax < 0.f )
	{
		fCurMax = 0.f;
	}

	CAIGoalAbstract* pGoalSoundMax = LTNULL;
	EnumAISoundType eAISoundMax = kAIS_None;
	EnumAISoundType eAISound;

	// Let goals handle sense, and keep track of current most important goal.

	CAIGoalAbstract* pGoal;
	CAIGoalAbstract* pTriggeredGoal = LTNULL;
	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = (*it);

		if( pGoal->HandleGoalSenseTrigger(pSenseRecord) )
		{
			// Force CurImportance to be reset to BaseImportance
			pGoal->SetCurToBaseImportance();
			pGoal->SetLastUpdateTime(g_pLTServer->GetTime());

			AITRACE(AIShowGoalsVerbose, ( m_pAI->m_hObject, "Goal %s responded to sense %s (%.2f)\n",
				s_aszGoalTypes[pGoal->GetGoalType()],
				CAISenseRecorderAbstract::SenseToString(pSenseRecord->eSenseType),
				pGoal->GetCurImportance() ) );

			// Do not actually trigger anything if a goal is deactivating.

			if( m_pDeactivatingGoal )
			{
				continue;
			}

			fImportance = pGoal->GetCurImportance();

			// Select the triggered sound from the goal with highest
			// importance.  Ignore goals that do not set a sound.

			if( ( !pGoalSoundMax ) || ( fImportance > pGoalSoundMax->GetCurImportance() ) )
			{
				if( pGoal->SelectTriggeredAISound( &eAISound ) )
				{
					eAISoundMax = eAISound;
					pGoalSoundMax = pGoal;
				}
			}

			// Check if goal should be triggered.

			if( (fImportance > fCurMax) &&
				( ( m_pCurGoal && m_pCurGoal->IsLockedAnimInterruptable() )
					|| ( pGoal->DoesForceAnimInterrupt() )
					|| ( !m_pAI->GetAnimationContext()->IsLocked() ) )
				&& ( !m_pAI->GetAnimationContext()->IsTransitioning() )
				&& ( !m_pAI->GetAIMovement()->IsMovementLocked() ) )
			{
				// Unlock locked goal, if another is now more important.
				if(m_pLockedGoal != pGoal)
				{
					m_pLockedGoal = LTNULL;
				}

				pTriggeredGoal = pGoal;
				SetNextGoal( pGoal );
				fCurMax = fImportance;
			}

			// Re-triggered the active goal.
			else if( (fImportance == fCurMax) && (pGoal == m_pCurGoal) )
			{
				pTriggeredGoal = pGoal;
			}
		}
	}

	// Play a triggered sound.

	if( eAISoundMax != kAIS_None )
	{
		m_pAI->PlaySound( eAISoundMax, LTFALSE );

		AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Playing trigger sound for Goal %s\n",
			s_aszGoalTypes[pGoalSoundMax->GetGoalType()] ) );
	}

	if( pTriggeredGoal )
	{
		m_pTriggeredGoal = pTriggeredGoal;

		// Console output.

		AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Goal %s triggered by sense %s (%.2f)\n",
			s_aszGoalTypes[m_pTriggeredGoal->GetGoalType()],
			CAISenseRecorderAbstract::SenseToString(pSenseRecord->eSenseType),
			m_pTriggeredGoal->GetCurImportance() ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::HandleCommand
//
//	PURPOSE:	Handles a command.
//
// ----------------------------------------------------------------------- //

bool CAIGoalMgr::HandleCommand(const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_GoalSet(GOAL_CMD_GOALSET);
	static CParsedMsg::CToken s_cTok_AddGoal(GOAL_CMD_ADDGOAL);
	static CParsedMsg::CToken s_cTok_RemoveGoal(GOAL_CMD_REMOVEGOAL);
	static CParsedMsg::CToken s_cTok_GoalScript(GOAL_CMD_GOALSCRIPT);

	uint8 len = strlen(GOAL_CMD_PREFIX);

	if( _strnicmp( cMsg.GetArg(0), GOAL_CMD_PREFIX, len ) == 0 )
	{
		// Do not actually run commands here here.  Queue up commands to run
		// next update.  This ensures they are run on goals added from commands,
		// which are not actually added until next update.

		EnumAIGoalType eGoalType = GetGoalTypeEnumFromName( cMsg.GetArg(0) + len );
		if( eGoalType != kGoal_InvalidType )
		{
			m_lstQueuedPrefixCmdGoals.push_back( eGoalType );

			// Process any additional name value pairs for this goal.
			if( cMsg.GetArgCount() > 1 )
			{
				char szBuffer[128];
				cMsg.ReCreateMsg( szBuffer, sizeof( szBuffer ), 0 );
				m_lstQueuedPrefixCmdGoalArgs.push_back( g_pLTServer->CreateString( szBuffer ) );
			}
			else {
				m_lstQueuedPrefixCmdGoalArgs.push_back( LTNULL );
			}
		}

		return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_GoalSet )
	{
		// Do not actually set the goalset here.  Set goalset to change to
		// next update.  Commands may be running from inside of goals or states,
		// so changing goals here may crash the game.

		AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Setting GoalSet: %s", cMsg.GetArg(1).c_str() ) );
		SetGoalSetNextUpdate( cMsg.GetArg(1) );
		return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_AddGoal )
	{
		// Find goal by name.
		EnumAIGoalType eGoalType = GetGoalTypeEnumFromName(cMsg.GetArg(1));
		if( (eGoalType == kGoal_InvalidType) || (eGoalType == kGoal_Count))
		{
			AIASSERT( 0, m_pAI->m_hObject, "CAIGoalMgr::HandleCommand: ADDGOAL Invalid goal type." );
			return true;
		}

		// Do not actually add goal now.
		// Queue it for the next update in case we are in the middle of
		// a goal or state.  Add the goal after possible goalset changes.

		m_lstQueuedGoals.push_back( eGoalType );

		// Process any additional name value pairs for this goal.
		if(cMsg.GetArgCount() > 2)
		{
			char szBuffer[128];
			cMsg.ReCreateMsg( szBuffer, sizeof( szBuffer ), 1 );
			m_lstQueuedGoalArgs.push_back( g_pLTServer->CreateString( szBuffer ) );
		}
		else {
			m_lstQueuedGoalArgs.push_back( LTNULL );
		}

		// HACK: for TO2 dialogue object - Talk goal interaction.
		// If we do not immediately add the Talk goal, the dialogue will start
		// right away instead of after AI gets to nodes.
		// Still queue added goals if a goalset change is pending.

		if( m_iQueuedGoalSet == -1 )
		{
			AddQueuedGoals();
		}

		// END HACK

		return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_RemoveGoal )
	{
		// Find goal by name.
		EnumAIGoalType eGoalType = GetGoalTypeEnumFromName(cMsg.GetArg(1));
		if( (eGoalType == kGoal_InvalidType) || (eGoalType == kGoal_Count))
		{
			AIASSERT( 0, m_pAI->m_hObject, "CAIGoalMgr::HandleCommand: REMOVEGOAL Invalid goal type." );
			return true;
		}

		// Do not call RemoveGoal here, because it may not be safe to change the goal and
		// in turn the state, depending on where the trigger was sent from.

		DeleteGoalNextUpdate(eGoalType);

		AITRACE(AIShowGoals, ( m_pAI->m_hObject, "Removed Goal: %s", cMsg.GetArg(1).c_str()) );

		return true;
	}
	else if( cMsg.GetArg(0) == s_cTok_GoalScript )
	{
		// Clear any existing script when a new one is assigned.
		ClearGoalScript();

		char szNameValueBuffer[128];
		szNameValueBuffer[0] = '\0';
		uint iArg;

		// AITRACE output.

		for(iArg=1; iArg < cMsg.GetArgCount(); ++iArg)
		{
			if( strlen( szNameValueBuffer ) + strlen( cMsg.GetArg(iArg) + 1 ) >= 128 )
			{
				AITRACE( AIShowGoals, ( m_pAI->m_hObject, "GoalsScript: %s", szNameValueBuffer ) );
				szNameValueBuffer[0] = '\0';
			}

			strcat( szNameValueBuffer, cMsg.GetArg(iArg) );
			strcat( szNameValueBuffer, " " );
		}
		AITRACE( AIShowGoals, ( m_pAI->m_hObject, "GoalsScript: %s", szNameValueBuffer ) );
		szNameValueBuffer[0] = '\0';


		// Count the number of scripted goals.
		// Goals are tokens without a '='.
		uint32 cGoals = 0;
		for(iArg=1; iArg < cMsg.GetArgCount(); ++iArg)
		{
			if( strchr(cMsg.GetArg(iArg), '=') == LTNULL )
			{
				++ cGoals;
			}
		}
		m_lstScriptGoals.reserve(cGoals);

		GOAL_SCRIPT_STRUCT gss;

		iArg = 1;
		while(iArg < cMsg.GetArgCount())
		{
			// Find goal by name.
			gss.eGoalType = GetGoalTypeEnumFromName(cMsg.GetArg(iArg));
			if( (gss.eGoalType == kGoal_InvalidType) || (gss.eGoalType == kGoal_Count))
			{
				AIASSERT( 0, m_pAI->m_hObject, "CAIGoalMgr::HandleCommand: GOALSCRIPT Invalid goal type." );
				return true;
			}

			// Find parameters for this goal.
			szNameValueBuffer[0] = '\0';
			while( (++iArg < cMsg.GetArgCount()) && (strchr(cMsg.GetArg(iArg), '=') != LTNULL) )
			{
				strcat(szNameValueBuffer, cMsg.GetArg(iArg));
				strcat(szNameValueBuffer, " ");
			}

			if( strlen(szNameValueBuffer) > 0)
			{
				gss.hstrNameValuePairs = g_pLTServer->CreateString(szNameValueBuffer);
			}
			else {
				gss.hstrNameValuePairs = LTNULL;
			}

			m_lstScriptGoals.push_back(gss);
		}

		// Do not actually start the goalscrpit now.
		// Wait until the next update, to ensure we are not in the middle of
		// goal or state code, and to ensure the goalscript is added after
		// a goalset change.

		m_bQueuedGoalScript = LTTRUE;

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::HandleDamage
//
//	PURPOSE:	Handle damage.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIGoalMgr::HandleDamage(const DamageStruct& damage)
{
	CAIGoalAbstract* pGoal;
	AIGOAL_DAMAGE_MAP::iterator it;
	for( it = m_mapDamageHandlers.begin(); it != m_mapDamageHandlers.end(); ++it )
	{
		// Goals return FALSE if they do not handle the damage, or
		// they want to let others handle the damage.

		pGoal = it->second;
		if( pGoal->HandleDamage( damage ) )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::HandleVolume*
//
//	PURPOSE:	Handle volume enter and exit.
//
// ----------------------------------------------------------------------- //

void CAIGoalMgr::HandleVolumeEnter(AIVolume* pVolume)
{
	if( m_pCurGoal )
	{
		m_pCurGoal->HandleVolumeEnter( pVolume );
	}
}

void CAIGoalMgr::HandleVolumeExit(AIVolume* pVolume)
{
	if( m_pCurGoal )
	{
		m_pCurGoal->HandleVolumeExit( pVolume );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::GetAlternateDeathAnimation
//
//	PURPOSE:	Gives goals a chance to choose an appropriate death animation.
//
// ----------------------------------------------------------------------- //

HMODELANIM CAIGoalMgr::GetAlternateDeathAnimation()
{
	if( m_pCurGoal )
	{
		return m_pCurGoal->GetAlternateDeathAnimation();
	}

	// No alternate.

	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::FindGoalByType
//
//	PURPOSE:	Find a goal of some type.
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract* CAIGoalMgr::FindGoalByType(EnumAIGoalType eGoalType)
{
	AIGOAL_LIST::iterator it;
	CAIGoalAbstract* pGoal;
	for(it = m_lstGoals.begin(); it != m_lstGoals.end(); ++it)
	{
		pGoal = (*it);
		if( pGoal->GetGoalType() == eGoalType )
		{
			return pGoal;
		}
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::GetGoalTypeEnumFromName
//
//	PURPOSE:	Handles a command.
//
// ----------------------------------------------------------------------- //

EnumAIGoalType CAIGoalMgr::GetGoalTypeEnumFromName(const char* szGoalType)
{
	// Find goal by name.
	uint32 iGoalType;
	for(iGoalType=0; iGoalType < kGoal_Count; ++iGoalType)
	{
		if( stricmp(s_aszGoalTypes[iGoalType], szGoalType) == 0)
		{
			break;
		}
	}

	return (EnumAIGoalType)iGoalType;
}
