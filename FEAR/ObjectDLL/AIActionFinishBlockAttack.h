// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFinishBlockAttack.h
//
// PURPOSE : 
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONFINISHBLOCKATTACK_H_
#define _AIACTIONFINISHBLOCKATTACK_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionFinishBlockAttack
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionFinishBlockAttack : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFinishBlockAttack, kAct_FinishBlockAttack );

	// Ctor/Dtor

	CAIActionFinishBlockAttack();
	virtual ~CAIActionFinishBlockAttack();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionFinishBlockAttack);
};

#endif // _AIACTIONFINISHBLOCKATTACK_H_
