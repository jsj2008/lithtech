// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionUseSmartObjectNode.h
//
// PURPOSE : AIActionUseSmartObjectNode abstract class definition
//
// CREATED : 1/29/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_USE_NODE_H__
#define __AIACTION_USE_NODE_H__

#include "AIActionAbstract.h"

class CAIActionUseSmartObjectNode : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionUseSmartObjectNode, kAct_UseSmartObjectNode );

		CAIActionUseSmartObjectNode();

		// Node dependency.

		HOBJECT			GetNodeDependency( HOBJECT hNode );

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	SetPlanWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual bool	ValidateWSPreconditions( CAI* pAI, CAIWorldState& wsWorldStateCur, CAIWorldState& wsWorldStateGoal, ENUM_AIWORLDSTATE_PROP_KEY* pFailedWSK );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual bool	IsActionComplete( CAI* pAI );
		virtual bool	ValidateAction( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

	protected:
		virtual bool	IsActionValidForNodeType( EnumAINodeType eNodeType ) const;

		uint32				m_dwNodeStatusFlags;
};

// ----------------------------------------------------------------------- //

#endif
