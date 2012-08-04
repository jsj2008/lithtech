// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTaseredLevel2.h
//
// PURPOSE : 
//
// CREATED : 2/16/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONTASERLEVEL2_H_
#define _AIACTIONTASERLEVEL2_H_

#include "AIActionShortRecoil.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionTaseredLevel2
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionTaseredLevel2 : public CAIActionShortRecoil
{
	typedef CAIActionShortRecoil super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTaseredLevel2, kAct_TaseredLevel2 );

	// Ctor/Dtor

	CAIActionTaseredLevel2();
	virtual ~CAIActionTaseredLevel2();

	// CAIActionAbstract members.

public:
	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );

private:
	PREVENT_OBJECT_COPYING(CAIActionTaseredLevel2);
};

#endif // _AIACTIONTASERLEVEL2_H_
