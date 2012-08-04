// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMgr.cpp
//
// PURPOSE : AIActionMgr abstract class implementation
//
// CREATED : 2/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIActionMgr.h"
#include "AIDB.h"


// Globals / Statics

CAIActionMgr* g_pAIActionMgr = NULL;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMgr::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIActionMgr::CAIActionMgr()
{
	AIASSERT( !g_pAIActionMgr, NULL, "CAIActionMgr: Singleton already set." );
	g_pAIActionMgr = this;

	for( int iAction=0; iAction < kAct_Count; ++iAction )
	{
		m_pAIActions[iAction] = NULL;
	}
}

CAIActionMgr::~CAIActionMgr()
{
	AIASSERT( g_pAIActionMgr, NULL, "CAIActionMgr: No singleton." );
	g_pAIActionMgr = NULL;

	TermAIActionMgr();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMgr::InitAIActionMgr
//
//	PURPOSE:	Initialize AIActionMgr.
//
// ----------------------------------------------------------------------- //

void CAIActionMgr::InitAIActionMgr()
{
	// Create an instance of each action in the bute file.
	// Actions are shared, rather than owned by each AI.

	CAIActionAbstract* pAction;
	AIDB_ActionRecord* pActionRecord;
	unsigned int cActions = g_pAIDB->GetNumAIActionRecords();
	for( unsigned int iAction=0; iAction < cActions; ++iAction )
	{
		pActionRecord = g_pAIDB->GetAIActionRecord( iAction );
		if( pActionRecord && ( pActionRecord->eActionClass != kAct_InvalidType ) )
		{
			pAction = AI_FACTORY_NEW_Action( pActionRecord->eActionClass );
			if( pAction )
			{
				m_pAIActions[pActionRecord->eActionType] = pAction;
				pAction->InitAction( pActionRecord );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMgr::TermAIActionMgr
//
//	PURPOSE:	Terminate AIActionMgr.
//
// ----------------------------------------------------------------------- //

void CAIActionMgr::TermAIActionMgr()
{
	// Delete instances of actions.

	for( int iAction=0; iAction < kAct_Count; ++iAction )
	{
		if( m_pAIActions[iAction] )
		{
			AI_FACTORY_DELETE( m_pAIActions[iAction] );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMgr::AI_FACTORY_NEW_Action
//
//	PURPOSE:	Create an action.
//
// ----------------------------------------------------------------------- //

CAIActionAbstract* CAIActionMgr::AI_FACTORY_NEW_Action( EnumAIActionType eActionClass )
{
	// Call AI_FACTORY_NEW for the requested type of action.

	switch( eActionClass )
	{
		#define ACTION_TYPE_AS_SWITCH 1
		#include "AIEnumActionTypes.h"
		#undef ACTION_TYPE_AS_SWITCH

		default: AIASSERT( 0, NULL, "CAIActionMgr::AI_FACTORY_NEW_Action: Unrecognized action type." );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMgr::GetAIAction
//
//	PURPOSE:	Get a pointer to an action.
//
// ----------------------------------------------------------------------- //

CAIActionAbstract* CAIActionMgr::GetAIAction( EnumAIActionType eAction )
{
	if( ( eAction > kAct_InvalidType ) &&
		( eAction < kAct_Count ) )
	{
		return m_pAIActions[eAction];
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMgr::IsInAIActionSet
//
//	PURPOSE:	Returns true if the action is in the specified set, else
//				returns false.
//
// ----------------------------------------------------------------------- //

bool CAIActionMgr::IsActionInAIActionSet(ENUM_AIActionSet eSet, EnumAIActionType eAction)
{
	// Action enum is invalid
	if( ( eAction <= kAct_InvalidType ) ||
		( eAction >= kAct_Count ) )
	{
		return false;
	}
	
	// ActionSet ID is invalid.

	AIDB_ActionSetRecord* pRecord = g_pAIDB->GetAIActionSetRecord( eSet );
	if( !pRecord )
	{
		return false;
	}

	// Mask determines if Action is in ActionsSet.

	return pRecord->ActionMask.test( eAction );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMgr::ActionSupportsAbility
//
//	PURPOSE:	Returns true if the specified action supports the passed in
//				ability, otherwise returns false.
//
// ----------------------------------------------------------------------- //

bool CAIActionMgr::ActionSupportsAbility(EnumAIActionType eAction, ENUM_ActionAbility eAbility)
{
	// Action enum is invalid
	if( ( eAction <= kAct_InvalidType ) ||
		( eAction >= kAct_Count ) )
	{
		return false;
	}
	
	// Null action pointer.
	if (NULL == m_pAIActions[eAction])
	{
		return false;
	}

	// Null action Template
	if (!m_pAIActions[eAction]->GetActionRecord())
	{
		return false;
	}

	return m_pAIActions[eAction]->GetActionRecord()->dwActionAbilitySet.test(eAbility);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIActionMgr::ActionSetSupportsAbility
//
//	PURPOSE:	Returns true if the specified action set supports the passed in
//				ability, otherwise returns false.
//
// ----------------------------------------------------------------------- //

bool CAIActionMgr::ActionSetSupportsAbility(ENUM_AIActionSet eSet, ENUM_ActionAbility eAbility)
{
	// Action Set is invalid
	if (eSet == kAIActionSet_Invalid 
		|| eSet < 0 
		|| (uint32)eSet >= g_pAIDB->GetNumAIActionSetRecords() )
	{
		return false;
	}

	for (int i = 0; i < kAct_Count; ++i)
	{
		EnumAIActionType eAction = (EnumAIActionType)i;
		if (IsActionInAIActionSet(eSet, eAction) 
			&& ActionSupportsAbility(eAction, eAbility))
		{
			return true;
		}
	}

	return false;
}
