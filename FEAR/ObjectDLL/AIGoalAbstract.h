// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstract.h
//
// PURPOSE : AIGoalAbstract abstract class definition
//
// CREATED : 1/28/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ABSTRACT_H__
#define __AIGOAL_ABSTRACT_H__

#include "AIWorldState.h"
#include "AIEnumContexts.h"

//
// ENUM: Types of goals.
//
enum EnumAIGoalType
{
	kGoal_InvalidType= -1,
	#define GOAL_TYPE_AS_ENUM 1
	#include "AIGoalTypeEnums.h"
	#undef GOAL_TYPE_AS_ENUM

	kGoal_Count,
};

//
// STRINGS: const strings for goal types.
//
static const char* s_aszGoalTypes[] =
{
	#define GOAL_TYPE_AS_STRING 1
	#include "AIGoalTypeEnums.h"
	#undef GOAL_TYPE_AS_STRING
};


// Forward declarations.

class	CAI;
class	CAIPlan;
class	CAIActionAbstract;
struct	AIDB_GoalRecord;


// ----------------------------------------------------------------------- //

class CAIGoalAbstract : public CAIClassAbstract
{
	public:

		static const char* const GetGoalTypeName(EnumAIGoalType eType);
		static EnumAIGoalType GetGoalType(const char* const pszGoalTypeName);

	public:
		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC( Goal );

		CAIGoalAbstract();
		virtual ~CAIGoalAbstract();

        virtual void Save(ILTMessage_Write *pMsg);
        virtual void Load(ILTMessage_Read *pMsg);

		virtual void	InitGoal( CAI* pAI, EnumAIGoalType eGoalType, AIDB_GoalRecord* pRecord );
		virtual void	TermGoal();

		virtual void	CalculateGoalRelevance() = 0;
		float			GetGoalRelevance() const { return m_fGoalRelevance; }
		void			ClearGoalRelevance() { m_fGoalRelevance = 0.f; }
		virtual void	HandleBuildPlanFailure() { ClearGoalRelevance(); }
		bool			GetReEvalOnSatisfaction() const;

		virtual double	GetNextRecalcTime() { return m_fNextRecalcTime; }
		virtual void	SetNextRecalcTime();

		virtual float	GetActivateChance() const;
		virtual float	GetInterruptPriority() const;
		virtual bool	CanReactivateDuringTransitions() const;

		virtual void	ActivateGoal();
		virtual void	DeactivateGoal();

		virtual bool	UpdateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& /*WorldState*/ ) {}
		virtual bool	IsWSSatisfied( CAIWorldState* /*pwsWorldState*/ ) { return false; }

		virtual void	UpdateTaskStatus() {}

		virtual bool	ReplanRequired() { return false; }
		bool			BuildPlan();
		void			SetAIPlan( CAIPlan* pPlan );
		void			ActivatePlan();
		virtual bool	IsPlanValid();
		void			ClearPlan();
		bool			IsPlanInterruptible();
		CAIActionAbstract* GetCurrentAction();

		// Returns the EnumAIContext associated with this goal.  This is
		// guaranteed to be either valid AIContext or kContext_Invalid.
		// This function is only valid to call on Active goals.
		EnumAIContext	GetContext() const;

		bool			IsPermanentGoal() { return false; }
		bool			IsAwarenessValid();

		EnumAIGoalType		GetGoalType() const { return m_eGoalType; }

	protected:
		CAI*				m_pAI;
		EnumAIGoalType		m_eGoalType;
		float				m_fGoalRelevance;
		AIDB_GoalRecord*	m_pGoalRecord;
		double				m_fNextRecalcTime;
		double				m_fActivationTime;

	private:
		// This is a template method, overridable by derived classes, for 
		// returning the EnumAIContext for this goal.  This base returns the 
		// AIContext specified in this goals record to preserve the the 
		// previous behavior of this system.
		// 
		// This function is private as it should not be used by derived 
		// classes.  To get the context, use GetContext(), which does 
		// additional validation/reporting.
		virtual EnumAIContext OnGetContext() const;
};


#endif
