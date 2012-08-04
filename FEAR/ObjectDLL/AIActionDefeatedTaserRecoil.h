// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDefeatedTaserRecoil.h
//
// PURPOSE : 
//
// CREATED : 9/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONDEFEATEDTASERRECOIL_H_
#define _AIACTIONDEFEATEDTASERRECOIL_H_

#include "AIActionShortRecoil.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionDefeatedTaserRecoil
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionDefeatedTaserRecoil : public CAIActionShortRecoil
{
	typedef CAIActionShortRecoil super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDefeatedTaserRecoil, kAct_DefeatedTaserRecoil );

	// Ctor/Dtor

	CAIActionDefeatedTaserRecoil();
	virtual ~CAIActionDefeatedTaserRecoil();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual void	ActivateAction(CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );

private:
	PREVENT_OBJECT_COPYING(CAIActionDefeatedTaserRecoil);
};

#endif // _AIACTIONDEFEATEDTASERRECOIL_H_
