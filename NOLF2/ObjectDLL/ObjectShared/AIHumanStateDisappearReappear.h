// (c) 2001 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STATE_DISAPPEARREAPPEAR_H__
#define __AI_HUMAN_STATE_DISAPPEARREAPPEAR_H__

#include "AIHumanState.h"


class CAIHumanStateDisappearReappear : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateDisappearReappear, kState_HumanDisappearReappear);

		CAIHumanStateDisappearReappear( );
		~CAIHumanStateDisappearReappear( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAlpha();
		void UpdateAnimation();

		virtual void HandleModelString(ArgList* pArgList);

		// Disappearing.

		void Disappear();
		void Reappear();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
		void SetReappearDistOverride(LTFLOAT fReappearDist) { m_fReappearDistOverride = fReappearDist; }
		void SetReappearDelay(LTFLOAT fReappearDelay) { m_fReappearDelay = fReappearDelay; }

	protected :

		LTVector	m_vDestPos;
		LTFLOAT		m_fFadeTime;
		LTFLOAT		m_fFadeTimer;
		LTBOOL		m_bStartFade;
		LTFLOAT		m_fReappearDistOverride;

		LTFLOAT		m_fDisappearTime;
		LTFLOAT		m_fReappearDelay;
};

#endif