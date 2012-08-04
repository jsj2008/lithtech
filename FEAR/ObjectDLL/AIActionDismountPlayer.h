// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDismountPlayer.h
//
// PURPOSE : 
//
// CREATED : 8/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONDISMOUNTPLAYER_H_
#define _AIACTIONDISMOUNTPLAYER_H_

LINKTO_MODULE(AIActionDismountPlayer);

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionDismountPlayer
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionDismountPlayer : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDismountPlayer, kAct_DismountPlayer );

	// Ctor/Dtor

	CAIActionDismountPlayer();
	virtual ~CAIActionDismountPlayer();

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );


private:
	PREVENT_OBJECT_COPYING(CAIActionDismountPlayer);
};

#endif // _AIACTIONDISMOUNTPLAYER_H_
