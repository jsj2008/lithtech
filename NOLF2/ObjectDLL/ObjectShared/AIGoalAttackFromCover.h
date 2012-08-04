// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackFromCover.h
//
// PURPOSE : AIGoalAttackFromCover class definition
//
// CREATED : 7/31/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_FROM_COVER_H__
#define __AIGOAL_ATTACK_FROM_COVER_H__

#include "AIGoalAttack.h"


class CAIGoalAttackFromCover : public CAIGoalAttack
{
	typedef CAIGoalAttack super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackFromCover, kGoal_AttackFromCover);

		CAIGoalAttackFromCover( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode* FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr);

	protected:

		AINodeChangeWeapons* GetNodeAsChangeWeapons() const;
		const char* const GetWeaponChangeDescription() const;

		virtual void SetStateAttack();

		// State Handling.

		void HandleStateUseObject();
		void HandleStateAttackFromCover();
		void HandleStateCharge();
		void CoverOrCharge();

	protected:

};


#endif
