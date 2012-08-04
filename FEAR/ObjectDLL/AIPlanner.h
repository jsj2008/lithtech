// ----------------------------------------------------------------------- //
//
// MODULE  : AIPlanner.h
//
// PURPOSE : AIPlanner class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIPLANNER_H__
#define __AIPLANNER_H__

#include "AIWorldState.h"
#include "AIAStarMachine.h"
#include "AIAStarPlanner.h"
#include "AIActionAbstract.h"

// Forward declarations.

class	CAI;
class	CAIPlanner;
class	CAIGoalAbstract;

// ----------------------------------------------------------------------- //

// A Plan consists of steps. Each step contains an AIAction, and a 
// representation of the world state deltas it expects to achieve.

class CAIPlanStep : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS( CAIPlanStep );

		CAIPlanStep()
		{
			eAIAction = kAct_InvalidType;
			wsWorldState.ResetWS();
		}

		EnumAIActionType	eAIAction;
		CAIWorldState		wsWorldState;
};

typedef std::vector< CAIPlanStep*, LTAllocator<CAIPlanStep*, LT_MEM_TYPE_OBJECTSHELL> >	AIPLAN_STEP_LIST;


class CAIPlan : public CAIClassAbstract
{
	friend class CAIPlanner;

	public:
		DECLARE_AI_FACTORY_CLASS( CAIPlan );

		 CAIPlan();
		~CAIPlan();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		void	ActivatePlan( CAI* pAI );
		void	DeactivatePlan( );
		bool	PlanStepIsComplete();
		bool	PlanStepIsInterruptible();
		bool	AdvancePlan();
		bool	IsPlanValid();
		double	GetPlanActivationTime() const { return m_fPlanActivationTime; }

		CAIActionAbstract* GetCurrentPlanStepAction();

	protected:

		CAI*				m_pAI;
		AIPLAN_STEP_LIST	m_lstAIPlanSteps;
		unsigned int		m_iPlanStep;
		double				m_fPlanActivationTime;
};

// ----------------------------------------------------------------------- //

extern CAIPlanner* g_pAIPlanner;

class CAIPlanner
{
	public:

		CAIPlanner();
		~CAIPlanner();

		void	InitAIPlanner();

		bool	BuildPlan( CAI* pAI, CAIGoalAbstract* pGoal );

		void	MergeWorldStates( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal );

	protected:

		void	EvaluateWorldStateProp( CAI* pAI, SAIWORLDSTATE_PROP& prop );

	protected:

		CAIAStarMachine			m_AStar;
		CAIAStarMapPlanner		m_AStarMapPlanner;	
		CAIAStarStoragePlanner	m_AStarStoragePlanner;
		CAIAStarGoalPlanner		m_AStarGoalPlanner;
};

// ----------------------------------------------------------------------- //

#endif
