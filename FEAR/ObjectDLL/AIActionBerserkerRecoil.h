// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionBerserkerRecoil.h
//
// PURPOSE : This action handles being kicked while performing a 
//			 berserker attack.
//
// CREATED : 3/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONBERSERKERRECOIL_H_
#define _AIACTIONBERSERKERRECOIL_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionBerserkerRecoil
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionBerserkerRecoil : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionBerserkerRecoil, kAct_BerserkerRecoil );

	// Ctor/Dtor

	CAIActionBerserkerRecoil();
	virtual ~CAIActionBerserkerRecoil();

	virtual bool IsActionComplete( CAI* pAI );
	virtual void ActivateAction(CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void InitAction( AIDB_ActionRecord* pActionRecord );

private:
	PREVENT_OBJECT_COPYING(CAIActionBerserkerRecoil);
};

#endif // _AIACTIONBERSERKERRECOIL_H_

