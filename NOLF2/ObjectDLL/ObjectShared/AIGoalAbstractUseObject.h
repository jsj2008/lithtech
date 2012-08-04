// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstractUseObject.h
//
// PURPOSE : AIGoalAbstractUseObject class definition
//
// CREATED : 10/26/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_AbstractUseObject_H__
#define __AIGOAL_AbstractUseObject_H__

#include "AIGoalAbstractSearch.h"
#include "AnimationProp.h"
#include "AI.h"

// Forward declarations.
class AINodeUseObject;


class CAIGoalAbstractUseObject : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Goal);

		CAIGoalAbstractUseObject( );
		~CAIGoalAbstractUseObject();

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		virtual void UpdateGoal();

		// Damage Handling.

		virtual HMODELANIM GetAlternateDeathAnimation();

		// Attractor Handling.

		virtual AINode*	HandleGoalAttractors();
		virtual AINode* FindNearestAttractorNode();

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// Node Handling.

		void ClearUseObjectNode();

		// State Handling.

		virtual void SetStateUseObject();
		virtual void HandleStateDraw();
		void HandleStateHolster();
		virtual void HandleStateUseObject();
		void CompleteUseObject();
		virtual EnumAIStateType GetUseObjectState();

	protected:

		LTObjRef			m_hNodeUseObject;
		LTObjRef			m_hLastNodeUseObject;
		LTFLOAT				m_fStimTimeReactMax;
		LTFLOAT				m_fStimTime;
		EnumAnimProp		m_eWeaponPosition;
		LTBOOL				m_bRequireBareHands;
		LTBOOL				m_bAllowDialogue;
		LTBOOL				m_bTurnOnLights;
		LTBOOL				m_bTurnOffLights;
		LTBOOL				m_bHolstered;
		LTBOOL				m_bLockedNode;
		LTBOOL				m_bPlayedSpecialDeathAnim;
};


#endif
