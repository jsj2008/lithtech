// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoNodeOfType.h
//
// PURPOSE : AIActionGotoNodeOfType abstract class definition
//
// CREATED : 10/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_GOTO_NODE_OF_TYPE_H__
#define __AIACTION_GOTO_NODE_OF_TYPE_H__

#include "AIActionGotoAbstract.h"


// Forward declarations.

class	AINode;


class CAIActionGotoNodeOfType : public CAIActionGotoAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionGotoNodeOfType, kAct_GotoNodeOfType );

		CAIActionGotoNodeOfType();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ApplyWSEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
		virtual bool	ValidateWSEffects( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual	bool	IsActionComplete( CAI* pAI );

	protected:

		AINode*			FindNodeOfType( CAI* pAI, EnumAINodeType eNodeType );
};

// ----------------------------------------------------------------------- //

#endif
