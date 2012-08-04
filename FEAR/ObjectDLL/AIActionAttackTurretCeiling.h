// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackTurretCeiling.h
//
// PURPOSE : AIActionAttackTurretCeiling class definition
//
// CREATED : 7/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_TURRET_CEILING_H__
#define __AIACTION_ATTACK_TURRET_CEILING_H__

#include "AIActionAttackTurret.h"


class CAIActionAttackTurretCeiling : public CAIActionAttackTurret
{
	typedef CAIActionAttackTurret super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackTurretCeiling, kAct_AttackTurretCeiling );

		CAIActionAttackTurretCeiling();
};

// ----------------------------------------------------------------------- //

#endif
