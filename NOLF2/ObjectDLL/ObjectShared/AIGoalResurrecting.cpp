//----------------------------------------------------------------------------
//              
//	MODULE:		AIGoalResurrecting.cpp
//              
//	PURPOSE:	- implementation
//              
//	CREATED:	28.01.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------


// Includes
#include "stdafx.h"

#ifndef __AIGOALRESURRECTING_H__
#include "AIGoalResurrecting.h"		
#endif

#ifndef __AIGOAL_MGR_H__
#include "AIGoalMgr.h"
#endif

#ifndef __AI_H__
#include "AI.h"
#endif

#ifndef __AIHUMANSTATERESURRECTING_H__
#include "AIHumanStateResurrecting.h"
#endif

// Forward declarations

// Globals

// Statics
DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalResurrecting, kGoal_Resurrecting);

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::CAIGoalResurrecting()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
CAIGoalResurrecting::CAIGoalResurrecting() :
	m_fTimeToResurrect( 0.0f )
{
	// Construct
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::~CAIGoalResurrecting()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ CAIGoalResurrecting::~CAIGoalResurrecting()
{
	// Destruct
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::Save()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIGoalResurrecting::Save(ILTMessage_Write *pMsg)
{
	CAIGoalAbstractSearch::Save(pMsg);
	
	SAVE_FLOAT(m_fTimeToResurrect);
	SAVE_BOOL(m_bReactivateGoalOnUpdate);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::Load()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIGoalResurrecting::Load(ILTMessage_Read *pMsg)
{
	CAIGoalAbstractSearch::Load(pMsg);

	LOAD_FLOAT(m_fTimeToResurrect);
	LOAD_BOOL(m_bReactivateGoalOnUpdate);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::ActivateGoal()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ void CAIGoalResurrecting::ActivateGoal()
{
	CAIGoalAbstractSearch::ActivateGoal();

	// Turn off the senses and lock the goal
	m_pAI->SetCurSenseFlags(kSense_None);
	m_pGoalMgr->LockGoal(this);

	// Set the state.
	m_pAI->ClearAndSetState( kState_HumanResurrecting );
	CAIHumanStateResurrecting* pRes = (CAIHumanStateResurrecting*)m_pAI->GetState();
	pRes->SetResurrectingTime( 10.0f );
	pRes->ResetExpirationTime();

	g_pLTServer->CPrint( "Activating AIGOalResurrecting!" );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::UpdateGoal()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIGoalResurrecting::UpdateGoal()
{
	if (m_bReactivateGoalOnUpdate)
	{
		ActivateGoal();
		m_bReactivateGoalOnUpdate = LTFALSE;
	}

	CAIState* pState = m_pAI->GetState();

	switch(pState->GetStateType())
	{
		case kState_HumanResurrecting:
			HandleStateResurrecting();
			break;

		case kState_HumanAware:
			break;

		case kState_HumanSearch:
			HandleStateSearch();
			break;

		case kState_HumanDraw:
			HandleStateDraw();
			break;

		// Unexpected State.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalResurrecting::UpdateGoal: Unexpected State.");
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::HandleGoalSenseTrigger()
//              
//	PURPOSE:	TEMP -- allow a trigger to cause goal entry.
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL CAIGoalResurrecting::HandleGoalSenseTrigger(AISenseRecord* pSenseRecord )
{
	if ( !CAIGoalAbstractSearch::HandleGoalSenseTrigger( pSenseRecord ))
	{
		return LTFALSE;
	}
	else
	{	
		const CAIGoalAbstract* pCurGoal = m_pAI->GetGoalMgr()->GetCurrentGoal();
		bool bIsThisGoalAIsCurrentGoal = ( (pCurGoal==NULL) ? (false) : (pCurGoal->GetGoalType()==GetGoalType()) );

		if ( !bIsThisGoalAIsCurrentGoal )
		{
			return LTTRUE;
		}
		if( m_pAI->GetState() && m_pAI->GetState()->GetStateType() != kState_HumanResurrecting )
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::HandleNameValuePair()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
/*virtual*/ LTBOOL CAIGoalResurrecting::HandleNameValuePair(const char* szName,const char* szValue)
{
	if ( !strcmp( szName, "RESETGOAL" ))
	{

		const CAIGoalAbstract* pCurGoal = m_pAI->GetGoalMgr()->GetCurrentGoal();
		bool bIsThisGoalAIsCurrentGoal = ( (pCurGoal==NULL) ? (false) : (pCurGoal->GetGoalType()==GetGoalType()) );
		if ( !bIsThisGoalAIsCurrentGoal )
		{
			return LTFALSE;
		}

//		const CAIGoalAbstract* pCurGoal = m_pAI->GetGoalMgr()->GetCurrentGoal();
//		bool bIsThisGoalAIsCurrentGoal = ( (pCurGoal==NULL) ? (false) : (pCurGoal->GetGoalType()==GetGoalType()) );
//		if ( !bIsThisGoalAIsCurrentGoal )
//		{
//			CAIGoalAbstract* pGoal = GetGoalMgr()->FindGoalByType( kGoal_Resurrecting );
//			if ( pGoal != NULL )
//			{
//				// If we have the goal, but it is not active, then do nothing
//				// because the Resurrecting goal either has not started, or
//				// some other goal has trumped it.
//			}
//			else
//			{
//				// If we do not have the goal, then this is an error condition
//				// because we cannot legally be here!
//				AIASSERT( 0, m_pAI->m_hObject, "Reseting Resurrect Goal when not active." );
//			}
//		}

		CAIState* pState = m_pAI->GetState();
		if ( !pState )
		{
			AIASSERT( 0, m_pAI->m_hObject, "Reseting Resurrect Goal with no state." );
			return LTFALSE;
		}

		switch(pState->GetStateType())
		{
			case kState_HumanResurrecting:
				// If we are still resurrecting, do nothing!
				break;

			case kState_HumanAware:
				// If we are currently aware, then reset the Goal
				m_bReactivateGoalOnUpdate = LTTRUE;
				break;

			case kState_HumanSearch:
				// If we are currently searching, then reset the Goal
				m_bReactivateGoalOnUpdate = LTTRUE;
				break;

			// Unexpected State.
			default:
				AIASSERT(0, m_pAI->m_hObject, "CAIGoalResurrecting::UpdateGoal: Unexpected State.");
		}
	}

	return LTFALSE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalResurrecting::HandleStateResurrecting()
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
void CAIGoalResurrecting::HandleStateResurrecting()
{
	switch( m_pAI->GetState()->GetStateStatus() )
	{
		case kSStat_Initialized:
			break;

		case kSStat_Resurrecting:
			break;

		case kSStat_RegainingConsciousness:
			break;

		case kSStat_Conscious:
			{
				// Reset AIs default senses, from aibutes.txt.
				m_pAI->ResetBaseSenseFlags();
				m_pGoalMgr->UnlockGoal(this);

				if ( m_pAI->CanSearch() )
				{
					// Search.
					SetStateSearch();
				}
				else
				{
					// Go aware.
					m_pAI->SetState( kState_HumanAware );
				}

				// Set the importance to something fairly low, so that
				// anything else will be preferable.
				SetCurImportance( 8 );
			}
			break;

		// Unexpected StateStatus.
		default: AIASSERT(0, m_pAI->m_hObject, "CAIGoalUnconscious::HandleStateUnconscious: Unexpected State Status.");
	}
}
