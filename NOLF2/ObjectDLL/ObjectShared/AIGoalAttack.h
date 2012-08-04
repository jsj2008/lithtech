// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttack.h
//
// PURPOSE : AIGoalAttack class definition
//
// CREATED : 7/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ATTACK_H__
#define __AIGOAL_ATTACK_H__

#include "AIGoalAbstractStimulated.h"
#include "AnimationProp.h"
#include "AINode.h"

// Forward Declarations.

enum EnumAIWeaponType;


class CAIGoalAttack : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttack, kGoal_Attack);

		CAIGoalAttack( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();
		virtual AINode* FindNearestAttractorNode(EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fDistSqr);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Setting.

		virtual void SetStateAttack();
				void SetStatePanic();

		void ActivateChangeWeapons();

		// State Handling.

		void HandleStateAttack();
		void HandleStateFlee();
		void HandleStatePanic();
		void HandleStateHolster();
		void HandleStateDraw();

		void HandlePotentialWeaponChange();
		AINodeChangeWeapons* GetNodeAsChangeWeapons() const;
		const char* const GetWeaponChangeDescription() const;


	protected:

		EnumAIWeaponType	m_eWeaponType;

		LTFLOAT				m_fChaseDelay;
		EnumAnimProp		m_ePosture;
		LTBOOL				m_bPanicCanActivate;

		// Some attacks are node-based (cover, vantage, view).

		LTBOOL				m_bNodeBased;
		LTBOOL				m_bForceSearch;
		LTObjRef			m_hNode;
		LTObjRef			m_hFailedNode;

		// Remember if we checked for a weapon change yet.

		LTBOOL				m_bCheckForWeaponChange;
};


#endif
