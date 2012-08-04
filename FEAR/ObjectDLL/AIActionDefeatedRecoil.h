// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDefeatedRecoil.h
//
// PURPOSE : 
//
// CREATED : 9/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONDEFEATEDRECOIL_H_
#define _AIACTIONDEFEATEDRECOIL_H_

#include "AIActionShortRecoil.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionDefeatedRecoil
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionDefeatedRecoil : public CAIActionShortRecoil
{
	typedef CAIActionShortRecoil super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDefeatedRecoil, kAct_DefeatedRecoil );

	// Ctor/Dtor

	CAIActionDefeatedRecoil();
	virtual ~CAIActionDefeatedRecoil();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual void	ActivateAction(CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	DeactivateAction( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionDefeatedRecoil);
};

#endif // _AIACTIONDEFEATEDRECOIL_H_
