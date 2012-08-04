// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstract.cpp
//
// PURPOSE : AIGoalAbstract abstract class implementation
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIGoalAbstract.h"
#include "AI.h"
#include "AIDB.h"
#include "AIActionMgr.h"
#include "AIPlanner.h"
#include "AISensorMgr.h"
#include "AIBlackBoard.h"
#include "AIGoalMgr.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAIGoalAbstract::GetGoalTypeName
//
//  PURPOSE:	Static function which returns the name of a the passed in 
//				goal.
//
// ----------------------------------------------------------------------- //

const char* const CAIGoalAbstract::GetGoalTypeName(EnumAIGoalType eType)
{
	if (eType < 0 || eType >= kGoal_Count)
	{
		return "";
	}

	return s_aszGoalTypes[eType];
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAIGoalAbstract::GetGoalType
//
//  PURPOSE:	Convert the passed in name to a goal type.
//
// ----------------------------------------------------------------------- //

EnumAIGoalType CAIGoalAbstract::GetGoalType(const char* const pszGoalTypeName)
{
	if (NULL == pszGoalTypeName)
	{
		return kGoal_InvalidType;
	}

	for (int i = 0; i < kGoal_Count; ++i)
	{
		if (LTStrIEquals(s_aszGoalTypes[i], pszGoalTypeName))
		{
			return (EnumAIGoalType)i;
		}
	}

	return kGoal_InvalidType;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAbstract::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAbstract::CAIGoalAbstract()
{
	m_pAI = NULL;
	
	m_eGoalType = kGoal_InvalidType;

	m_fGoalRelevance = 0.f;
	m_fNextRecalcTime = 0.f;

	m_fActivationTime = 0.f;

	m_pGoalRecord = NULL;
}

CAIGoalAbstract::~CAIGoalAbstract()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAbstract::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIGoalAbstract
//              
//----------------------------------------------------------------------------

void CAIGoalAbstract::Save(ILTMessage_Write *pMsg)
{
	SAVE_COBJECT(m_pAI);
	SAVE_INT(m_eGoalType);
	SAVE_FLOAT(m_fGoalRelevance);
	SAVE_DOUBLE(m_fNextRecalcTime);
	SAVE_DOUBLE(m_fActivationTime);
}

void CAIGoalAbstract::Load(ILTMessage_Read *pMsg)
{
	LOAD_COBJECT(m_pAI, CAI);
	LOAD_INT_CAST(m_eGoalType, EnumAIGoalType);
	LOAD_FLOAT(m_fGoalRelevance);
	LOAD_DOUBLE(m_fNextRecalcTime);
	LOAD_DOUBLE(m_fActivationTime);

	m_pGoalRecord = g_pAIDB->GetAIGoalRecord( m_eGoalType );
}


// ----------------------------------------------------------------------- //

void CAIGoalAbstract::InitGoal( CAI* pAI, EnumAIGoalType eGoalType, AIDB_GoalRecord* pRecord )
{
	m_pAI = pAI;
	m_eGoalType = eGoalType;
	m_pGoalRecord = pRecord;

	if( m_pGoalRecord->fRecalcRateMax > 0.f )
	{
		SetNextRecalcTime();
	}

	// Add required sensors.

	EnumAISensorType eSensor;
	AISENSOR_TYPE_LIST::iterator itSensor;
	for( itSensor = m_pGoalRecord->lstSensorTypes.begin(); itSensor != m_pGoalRecord->lstSensorTypes.end(); ++itSensor )
	{
		eSensor = *itSensor;
		m_pAI->GetAISensorMgr()->AddAISensor( eSensor );
	}
}

// ----------------------------------------------------------------------- //

void CAIGoalAbstract::TermGoal()
{
	// Remove required sensors.

	EnumAISensorType eSensor;
	AISENSOR_TYPE_LIST::iterator itSensor;
	for( itSensor = m_pGoalRecord->lstSensorTypes.begin(); itSensor != m_pGoalRecord->lstSensorTypes.end(); ++itSensor )
	{
		eSensor = *itSensor;
		m_pAI->GetAISensorMgr()->RemoveAISensor( eSensor );
	}
}

// ----------------------------------------------------------------------- //

bool CAIGoalAbstract::GetReEvalOnSatisfaction() const
{
	return m_pGoalRecord->bReEvalOnSatisfaction; 
}

// ----------------------------------------------------------------------- //

void CAIGoalAbstract::ActivateGoal()
{
	m_fActivationTime = g_pLTServer->GetTime();

	// Don't allow two goals to use the same path.  Always recalculate,
	// even if the destination is the same.
	m_pAI->GetAIBlackBoard()->SetBBInvalidatePath(true);
}

// ----------------------------------------------------------------------- //

void CAIGoalAbstract::DeactivateGoal()
{
	if( m_pGoalRecord->fFrequency > 0.f )
	{
		m_fNextRecalcTime = g_pLTServer->GetTime() + m_pGoalRecord->fFrequency;
	}

	// Event response is scoped to a goal.  Always reset the event response
	// to complete.  If we don't do this, goals may not be able to refire/
	// may believe they are satisfied.

	m_pAI->GetAIWorldState()->SetWSProp( kWSK_ReactedToWorldStateEvent, m_pAI->m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_Invalid );
}

// ----------------------------------------------------------------------- //

void CAIGoalAbstract::SetNextRecalcTime()
{
	if( m_pGoalRecord->fRecalcRateMax > 0.f )
	{
		m_fNextRecalcTime = g_pLTServer->GetTime() + GetRandom( m_pGoalRecord->fRecalcRateMin, m_pGoalRecord->fRecalcRateMax );
	}
}

// ----------------------------------------------------------------------- //

float CAIGoalAbstract::GetActivateChance() const
{
	return m_pGoalRecord->fActivateChance; 
}

// ----------------------------------------------------------------------- //

float CAIGoalAbstract::GetInterruptPriority() const
{
	return m_pGoalRecord->fInterruptPriority; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalMgr::CanReactivateDuringTransitions
//
//	PURPOSE:	Historically, during a transition, a goal was not allowed 
//				to reactivate.  For instance, if the AI was executing a 
//				ReactToDamage goal, and if the AI took more damage, and if 
//				the AI was playing a transition at that time, the goal 
//				would not reactivate due to a test in the AIGoalMgr.  This
//				is a low risk fix to this issue, enabling particular goals 
//				to ignore this if needed.
//
// ----------------------------------------------------------------------- //

bool CAIGoalAbstract::CanReactivateDuringTransitions() const
{
	return m_pGoalRecord->bCanReactivateDuringTransitions; 
}

// ----------------------------------------------------------------------- //

bool CAIGoalAbstract::BuildPlan()
{
	return g_pAIPlanner->BuildPlan( m_pAI, this );
}

// ----------------------------------------------------------------------- //

void CAIGoalAbstract::SetAIPlan( CAIPlan* pPlan )
{
	if( !pPlan )
	{
		return;
	}

	m_pAI->SetAIPlan( pPlan );
}

// ----------------------------------------------------------------------- //

void CAIGoalAbstract::ActivatePlan()
{
	CAIPlan* pPlan = m_pAI->GetAIPlan();
	if( pPlan )
	{
		pPlan->ActivatePlan( m_pAI );
	}
}

// ----------------------------------------------------------------------- //

bool CAIGoalAbstract::UpdateGoal()
{
	CAIPlan* pPlan = m_pAI->GetAIPlan();
	if( !pPlan )
	{
		return false;
	}

	if( pPlan->PlanStepIsComplete() )
	{
		return pPlan->AdvancePlan();
	}

	return true;
}

// ----------------------------------------------------------------------- //

bool CAIGoalAbstract::IsPlanValid()
{
	CAIPlan* pPlan = m_pAI->GetAIPlan();
	if( !pPlan )
	{
		return false;
	}

	return pPlan->IsPlanValid();
}

// ----------------------------------------------------------------------- //

void CAIGoalAbstract::ClearPlan()
{
	m_pAI->SetAIPlan( NULL );
}

// ----------------------------------------------------------------------- //

bool CAIGoalAbstract::IsPlanInterruptible()
{
	CAIPlan* pPlan = m_pAI->GetAIPlan();
	if( pPlan )
	{
		return pPlan->PlanStepIsInterruptible();
	}

	return true;
}

// ----------------------------------------------------------------------- //

CAIActionAbstract* CAIGoalAbstract::GetCurrentAction()
{
	CAIPlan* pPlan = m_pAI->GetAIPlan();
	if( pPlan )
	{
		return pPlan->GetCurrentPlanStepAction();
	}

	return NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAbstract::GetContext
//              
//	PURPOSE:	Returns the EnumAIContext associated with this goal.  This is 
//				guaranteed to be either valid AIContext or kContext_Invalid.
//
//				This function is only valid to call on Active goals.
//
//----------------------------------------------------------------------------

EnumAIContext CAIGoalAbstract::GetContext() const
{
	// Verify the goal is currently active.

	AIASSERT( m_pAI->GetGoalMgr()->IsCurGoal( this ), m_pAI->GetHOBJECT(), "CAIGoalAbstract::GetContext: This function is only valid while a goal is active." );

	EnumAIContext eContext = OnGetContext();

	// Verify the context to be returned is valid.

	if ( eContext == kContext_Invalid )
	{
		AIASSERT1( 0, NULL, "CAIGoalAbstract::GetAIContext: goal %s returned an out of range AIContext ID.", GetGoalTypeName( GetGoalType() ) );
		return kContext_Invalid;
	}

	if ( eContext < 0 || eContext >= AIContextUtils::Count() )
	{
		AIASSERT1( 0, NULL, "CAIGoalAbstract::GetAIContext: goal %s returned an out of range AIContext ID.", GetGoalTypeName( GetGoalType() ) );
		return kContext_Invalid;
	}

	return eContext;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAbstract::IsAwarenessValid
//              
//	PURPOSE:	Returns true if the AI's awareness is within the range 
//				required by this goal.  Returns false if it is outside this 
//				range.
//
//	NOTES:		This function would ideally be part of a base class 
//				CalculateGoalRelevance.  At some point in the future, it 
//				would be good to roll this back into a base class 
//				CalculateGoalRelevance call.
//
//----------------------------------------------------------------------------

bool CAIGoalAbstract::IsAwarenessValid()
{
	EnumAIAwareness eCurrentAwareness = m_pAI->GetAIBlackBoard()->GetBBAwareness();

	if( eCurrentAwareness < m_pGoalRecord->eMinAwareness 
		|| eCurrentAwareness > m_pGoalRecord->eMaxAwareness )
	{
		return false;
	}

	return true;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIGoalAbstract::OnGetContext()
//              
//	PURPOSE:	This is a template method, overridable by derived classes, 
//				for returning the EnumAIContext for this goal.  This base 
//				returns the AIContext specified in this goals record to 
//				preserve the the previous behavior of this system.
//
//----------------------------------------------------------------------------

EnumAIContext CAIGoalAbstract::OnGetContext() const
{
	return m_pGoalRecord->eAIContext;
}
