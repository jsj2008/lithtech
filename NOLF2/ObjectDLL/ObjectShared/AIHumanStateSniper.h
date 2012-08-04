// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STATE_SNIPER_H__
#define __AI_HUMAN_STATE_SNIPER_H__

#include "AIHumanState.h"


class CAIHumanStateSniper : public CAIHumanStateUseObject
{
	typedef CAIHumanStateUseObject super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateSniper, kState_HumanSniper);

		CAIHumanStateSniper( );
		~CAIHumanStateSniper( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Handlers

		virtual void HandleDamage(const DamageStruct& damage);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood();

		virtual LTBOOL CausesAllyDisturbances() const { return LTFALSE; }
		virtual LTBOOL SetNode(AINodeUseObject* pUseNode);

	protected :

		LTObjRef			m_hTarget;
};

#endif