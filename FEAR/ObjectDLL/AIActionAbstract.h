// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAbstract.h
//
// PURPOSE : AIActionAbstract abstract class definition
//           AI use sequences of AIActions to satisfy AIGoals.
//           The AIPlanner finds the sequences of AIActions.
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ABSTRACT_H__
#define __AIACTION_ABSTRACT_H__

#include "AIClassFactory.h"
#include "AIWorldState.h"

// ----------------------------------------------------------------------- //

//
// ENUM: Types of actions.
//
enum EnumAIActionType
{
	kAct_InvalidType= -1,
	#define ACTION_TYPE_AS_ENUM 1
	#include "AIEnumActionTypes.h"
	#undef ACTION_TYPE_AS_ENUM

	kAct_Count,
};

//
// STRINGS: const strings for action types.
//
static const char* s_aszActionTypes[] =
{
	#define ACTION_TYPE_AS_STRING 1
	#include "AIEnumActionTypes.h"
	#undef ACTION_TYPE_AS_STRING
};

// ----------------------------------------------------------------------- //

//
// ENUM:  of ActionAbilities.
//
enum ENUM_ActionAbility
{
	kActionAbility_InvalidType= -1,

	#define ACTIONABILITY_TYPE_AS_ENUM 1
	#include "AIEnumActionAbilityTypes.h"
	#undef ACTIONABILITY_TYPE_AS_ENUM

	kActionAbility_Count,
};

//
// STRINGS: const strings for ActionAbilities .
//
static const char* s_aszActionAbilityTypes[] =
{
	#define ACTIONABILITY_TYPE_AS_STRING 1
	#include "AIEnumActionAbilityTypes.h"
	#undef ACTIONABILITY_TYPE_AS_STRING
};

// ----------------------------------------------------------------------- //

// Forward declarations.

class	CAI;
class	CAIWorldState;
struct	AIDB_ActionRecord;


// ----------------------------------------------------------------------- //

class CAIActionAbstract : public CAIClassAbstract
{
	public:
		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC( Action );

		CAIActionAbstract();
		virtual ~CAIActionAbstract();

		// Initialization.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );

		// Planning actions.

		virtual bool	CanSolvePlanWS( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal );
		void			SolvePlanWSVariable( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal );
		virtual void	SetPlanWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	ApplyWSEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
		float			GetActionCost();
		float			GetActionPrecedence();
		virtual float	GetActionProbability( CAI* pAI );
		virtual void	FailActionProbability( CAI* /*pAI*/ ) {}

		// Validation.

		virtual bool	ValidateWSEffects( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal );
		virtual bool	ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK );
		virtual bool	ValidateContextPreconditions( CAI* /*pAI*/, CAIWorldState& /*wsWorldStateGoal*/, bool /*bIsPlanning*/ ) { return true; }
		virtual bool	ValidateAction( CAI* /*pAI*/ );

		// Activation and Completion.

		virtual void	ActivateAction( CAI* /*pAI*/, CAIWorldState& /*wsWorldStateGoal*/ );
		virtual void	DeactivateAction( CAI* /*pAI*/ ) {}
		virtual	bool	IsActionComplete( CAI* /*pAI*/ ) { return false; }
		virtual void	ApplyContextEffect( CAI* /*pAI*/, CAIWorldState* /*pwsWorldStateCur*/, CAIWorldState* /*pwsWorldStateGoal*/ ) {}
		virtual bool	IsActionInterruptible( CAI* pAI );

		// Data Access.

		AIDB_ActionRecord*		GetActionRecord() { return m_pActionRecord; }
		CAIWorldState*			GetActionEffects() { return &m_wsWorldStateEffects; }

	protected:

		CAIWorldState			m_wsWorldStatePreconditions;
		CAIWorldState			m_wsWorldStateEffects;

		AIDB_ActionRecord*		m_pActionRecord;
};

// ----------------------------------------------------------------------- //

#endif
