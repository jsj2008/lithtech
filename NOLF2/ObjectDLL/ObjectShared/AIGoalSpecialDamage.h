// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSpecialDamage.h
//
// PURPOSE : AIGoalSpecialDamage class definition
//
// CREATED : 3/21/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SPECIAL_DAMAGE_H__
#define __AIGOAL_SPECIAL_DAMAGE_H__

#include "AIGoalAbstractSearch.h"
#include "AnimationMgr.h"

enum DamageType;
enum EnumAIStimulusID;

class CAIGoalSpecialDamage : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSpecialDamage, kGoal_SpecialDamage);

		CAIGoalSpecialDamage( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		virtual void UpdateGoal();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);
		virtual HMODELANIM GetAlternateDeathAnimation();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

		// Access.

		void InterruptSpecialDamage(LTBOOL bSearch);
		void PauseSpecialDamage(LTBOOL bPause);

	protected:

		// State Handling.

		void HandleStateGoto();
		void HandleStateAware();
		void HandleStatePanic();
		void HandleStateUseObject();
		void HandleStateIdle();

		// Activity.

		void HandleActivity();

		// Awareness.

		void RestoreAwareness();

	protected:

		DamageType			m_eDamageType;
		EnumAnimProp		m_eDamageAnim;
		EnumAIStimulusID	m_eSpecialDamageStimID;
		LTBOOL				m_bIncapacitated;
		LTBOOL				m_bProgressiveDamage;
		LTFLOAT				m_fProgressiveMinTime;
		LTObjRef			m_hDamager;
		LTBOOL				m_bFleeing;
		LTFLOAT				m_fRelaxTime;
		LTBOOL				m_bInfinite;
		bool				m_bNeedsClearState;
};


#endif
