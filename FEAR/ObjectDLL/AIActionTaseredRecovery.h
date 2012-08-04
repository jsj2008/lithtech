// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTaseredRecovery.h
//
// PURPOSE : 
//
// CREATED : 2/16/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONTASEREDRECOVERY_H_
#define _AIACTIONTASEREDRECOVERY_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionTaseredRecovery
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionTaseredRecovery : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTaseredRecovery, kAct_TaseredRecovery );

	// Ctor/Dtor

	CAIActionTaseredRecovery();
	virtual ~CAIActionTaseredRecovery();

	// CAIActionShortRecoil members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual void	DeactivateAction( CAI* pAI );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual bool	IsActionComplete( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionTaseredRecovery);
};

#endif // _AIACTIONTASEREDRECOVERY_H_
