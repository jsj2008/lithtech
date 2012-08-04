// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionBerserkerRecoilDismount.h
//
// PURPOSE : 
//
// CREATED : 8/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONBERSERKERRECOILDISMOUNT_H_
#define _AIACTIONBERSERKERRECOILDISMOUNT_H_

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionBerserkerRecoilDismount
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionBerserkerRecoilDismount : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionBerserkerRecoilDismount, kAct_BerserkerRecoilDismount );

	// Ctor/Dtor

	CAIActionBerserkerRecoilDismount();
	virtual ~CAIActionBerserkerRecoilDismount();

	virtual	void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual	bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual bool	IsActionComplete( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionBerserkerRecoilDismount);
};

#endif // _AIACTIONBERSERKERRECOILDISMOUNT_H_
