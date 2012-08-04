// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFinishBlockNormal.h
//
// PURPOSE : 
//
// CREATED : 11/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONFINISHBLOCKNORMAL_H_
#define _AIACTIONFINISHBLOCKNORMAL_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionFinishBlockNormal
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionFinishBlockNormal : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFinishBlockNormal, kAct_FinishBlockNormal );

	// Ctor/Dtor

	CAIActionFinishBlockNormal();
	virtual ~CAIActionFinishBlockNormal();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionFinishBlockNormal);
};

#endif // _AIACTIONFINISHBLOCKNORMAL_H_
