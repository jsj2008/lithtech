// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionSurpriseAttackLaunch.h
//
// PURPOSE : 
//
// CREATED : 2/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONSURPRISEATTACKLAUNCH_H_
#define _AIACTIONSURPRISEATTACKLAUNCH_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionSurpriseAttackLaunch
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionSurpriseAttackLaunch : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionSurpriseAttackLaunch, kAct_SurpriseAttackLaunch );

	// Ctor/Dtor

	CAIActionSurpriseAttackLaunch();
	virtual ~CAIActionSurpriseAttackLaunch();

	// Template methods:

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual void	DeactivateAction( CAI* pAI );
	virtual bool	IsActionComplete( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionSurpriseAttackLaunch);
};

#endif // _AIACTIONSURPRISEATTACKLAUNCH_H_
