// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackFromSafetyNodeFirePosition.h
//
// PURPOSE : This action handles moving from an AINodeSafety instance to 
//			an AINodeSafetyFirePosition instance to attack the player.
//
// CREATED : 1/31/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONATTACKFROMSAFETYNODEFIREPOSITION_H_
#define _AIACTIONATTACKFROMSAFETYNODEFIREPOSITION_H_

#include "AIActionUseSmartObjectNode.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionAttackFromSafetyNodeFirePosition
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionAttackFromSafetyNodeFirePosition : public CAIActionUseSmartObjectNode
{
	typedef CAIActionUseSmartObjectNode super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackFromSafetyNodeFirePosition, kAct_AttackFromSafetyNodeFirePosition );

	// Ctor/Dtor

	CAIActionAttackFromSafetyNodeFirePosition();
	virtual ~CAIActionAttackFromSafetyNodeFirePosition();

	// CAIActionUseSmartObjectNode members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual void	DeactivateAction( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	virtual bool	IsActionValidForNodeType( EnumAINodeType eNodeType ) const;

	PREVENT_OBJECT_COPYING(CAIActionAttackFromSafetyNodeFirePosition);
};

#endif // _AIACTIONATTACKFROMSAFETYNODEFIREPOSITION_H_
