// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STATE_LAUNCH_H__
#define __AI_HUMAN_STATE_LAUNCH_H__

#include "AIHumanState.h"


class CAIHumanStateLaunch : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateLaunch, kState_HumanLaunch);

		CAIHumanStateLaunch( );
		~CAIHumanStateLaunch( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
		void SetLaunchSpeed(LTFLOAT fSpeed) { m_fLaunchSpeed = fSpeed; }
		void SetLaunchHeight(LTFLOAT fHeight) { m_fLaunchHeight = fHeight; }
		void SetLaunchDest(const LTVector& vDest) { m_vLaunchDest = vDest; }
		void SetLaunchMovement(EnumAnimProp eMovement) { m_eLaunchMovement = eMovement; }

	protected :

		LTFLOAT			m_fLaunchSpeed;
		LTFLOAT			m_fLaunchHeight;
		LTVector		m_vLaunchDest;
		EnumAnimProp	m_eLaunchMovement;
};

#endif