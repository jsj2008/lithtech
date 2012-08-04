// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionAttackTurret.h
//
// PURPOSE : AIActionAttackTurret class definition
//
// CREATED : 6/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_ATTACK_TURRET_H__
#define __AIACTION_ATTACK_TURRET_H__

#include "AIActionAttack.h"


class CAIActionAttackTurret : public CAIActionAttack
{
	typedef CAIActionAttack super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionAttackTurret, kAct_AttackTurret );

		CAIActionAttackTurret();

		// CAIActionAbstract members.

		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual	bool	ValidateAction( CAI* pAI );

	private:

		bool			TargetInFOV( CAI* pAI );
		bool			TimeoutExpired( CAI* pAI );

		void			PlayTurningSound( CAI* pAI, bool bPlaySound );

	protected:

		uint32			m_dwTrackerFlags;
		bool			m_bFaceTarget;
};

// ----------------------------------------------------------------------- //

#endif
