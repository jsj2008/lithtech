// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackMeleeKick.h
//
// PURPOSE : 
//
// CREATED : 2/17/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONATTACKMELEEKICK_H_
#define _AIACTIONATTACKMELEEKICK_H_

#include "AIActionAttackMelee.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionAttackMeleeKick
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionAttackMeleeKick : public CAIActionAttackMelee
{
	typedef CAIActionAttackMelee super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackMeleeKick, kAct_AttackMeleeKick );

	// Ctor/Dtor

	CAIActionAttackMeleeKick();
	virtual ~CAIActionAttackMeleeKick();

	virtual ENUM_AIWeaponType	GetWeaponType() const { return kAIWeaponType_Kick; }
	virtual bool				ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );

private:
	PREVENT_OBJECT_COPYING(CAIActionAttackMeleeKick);
};

#endif // _AIACTIONATTACKMELEEKICK_H_

