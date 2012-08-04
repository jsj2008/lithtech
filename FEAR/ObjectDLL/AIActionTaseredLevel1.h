// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTaseredLevel1.h
//
// PURPOSE : 
//
// CREATED : 2/16/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONTASERLEVEL1_H_
#define _AIACTIONTASERLEVEL1_H_

#include "AIActionShortRecoil.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionTaseredLevel1
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionTaseredLevel1 : public CAIActionShortRecoil
{
	typedef CAIActionShortRecoil super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionTaseredLevel1, kAct_TaseredLevel1 );

	// Ctor/Dtor

	CAIActionTaseredLevel1();
	virtual ~CAIActionTaseredLevel1();

	// CAIActionAbstract members.

public:
	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );

private:
	PREVENT_OBJECT_COPYING(CAIActionTaseredLevel1);
};

#endif // _AIACTIONTASERLEVEL1_H_
