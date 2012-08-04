// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMountPlayer.h
//
// PURPOSE : 
//
// CREATED : 8/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONMOUNTPLAYER_H_
#define _AIACTIONMOUNTPLAYER_H_

LINKTO_MODULE(AIActionMountPlayer);

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionMountPlayer
//
//	PURPOSE:	Handles jumping onto the player and 'attaching' to them.
//
// ----------------------------------------------------------------------- //

class CAIActionMountPlayer : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionMountPlayer, kAct_MountPlayer );

	// Ctor/Dtor

	CAIActionMountPlayer();
	virtual ~CAIActionMountPlayer();

	// CAIActionAbstract override

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );
	virtual bool	ValidateAction( CAI* pAI );
	virtual void	DeactivateAction( CAI* pAI );
	virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );

private:
	PREVENT_OBJECT_COPYING(CAIActionMountPlayer);
};

#endif // _AIACTIONMOUNTPLAYER_H_
