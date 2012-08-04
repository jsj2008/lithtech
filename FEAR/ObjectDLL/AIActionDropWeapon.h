// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionDropWeapon.h
//
// PURPOSE : 
//
// CREATED : 7/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONDROPWEAPON_H_
#define _AIACTIONDROPWEAPON_H_

LINKTO_MODULE(AIActionDropWeapon);

#include "AIActionAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionDropWeapon
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionDropWeapon : public CAIActionAbstract
{
	typedef CAIActionAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionDropWeapon, kAct_DropWeapon );

	// Ctor/Dtor

	CAIActionDropWeapon();
	virtual ~CAIActionDropWeapon();

	// CAIActionAbstract members.

	virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
	virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
	virtual	bool	IsActionComplete( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAIActionDropWeapon);
};

#endif // _AIACTIONDROPWEAPON_H_
