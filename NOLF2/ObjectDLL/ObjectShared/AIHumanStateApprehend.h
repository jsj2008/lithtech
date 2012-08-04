// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STATE_APPREHEND_H__
#define __AI_HUMAN_STATE_APPREHEND_H__

#include "AIHumanState.h"


class CAIHumanStateApprehend : public CAIHumanState
{
	typedef CAIHumanState super;

	public :
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateApprehend, kState_HumanApprehend);

		CAIHumanStateApprehend( );
		~CAIHumanStateApprehend( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update();
		void UpdateAnimation();

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodAggressive; }
		void SetHoldTime(LTFLOAT fTime) { m_fHoldTimer = fTime; }
		void SetCloseEnoughDist(LTFLOAT fDist) { m_fCloseEnoughSqr = fDist * fDist; }
		
	protected :

		LTVector	m_vOrigTargetPos;
		LTFLOAT		m_fHoldTimer;
		LTFLOAT		m_fCloseEnoughSqr;
};

#endif