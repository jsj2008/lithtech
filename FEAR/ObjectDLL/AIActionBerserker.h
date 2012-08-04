// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionBerserker.h
//
// PURPOSE : 
//
// CREATED : 8/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONBERSERKER_H_
#define _AIACTIONBERSERKER_H_

LINKTO_MODULE(AIActionBerserker);

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionBerserker
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionBerserker : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionBerserker, kAct_Berserker );

	// Ctor/Dtor

	CAIActionBerserker();
	virtual ~CAIActionBerserker();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );
	virtual void	DeactivateAction( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionBerserker);
};

#endif // _AIACTIONBERSERKER_H_
