// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackGrenade.h
//
// PURPOSE : AIActionAttackGrenade class definition
//
// CREATED : 4/08/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_GRENADE_H__
#define __AIACTION_ATTACK_GRENADE_H__

#include "AIActionAbstract.h"
#include "AIWeaponMgr.h"

class CAIActionAttackGrenade : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackGrenade, kAct_AttackGrenade );

		CAIActionAttackGrenade();

		// Grenade throw timing.

		void			SetNextThrowTime( CAI* pAI );

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual float	GetActionProbability( CAI* pAI );
		virtual void	FailActionProbability( CAI* pAI );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual	bool	IsActionComplete( CAI* pAI );

protected:
		virtual ENUM_AIWeaponType GetWeaponType() const { return kAIWeaponType_Thrown; }

};

// ----------------------------------------------------------------------- //

#endif
