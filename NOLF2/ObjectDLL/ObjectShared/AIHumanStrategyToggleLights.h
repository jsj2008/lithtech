// (c) 2002 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_STRATEGY_TOGGLE_LIGHTS_H__
#define __AI_HUMAN_STRATEGY_TOGGLE_LIGHTS_H__

#include "AIHumanStrategy.h"


class CAIHumanStrategyToggleLights : public CAIHumanStrategy
{
	typedef CAIHumanStrategy super;

	protected :

		enum State
		{
			kUnset,
			kMoving,
			kSwitching,
			kDone,
		};

	public : // Public member variables

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyToggleLights, kStrat_HumanToggleLights);

		CAIHumanStrategyToggleLights( );

		// Ctors/Dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman, CAIHumanStrategyFollowPath* pStrategyFollowPath);

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Updates

		virtual void Update();
		virtual LTBOOL UpdateAnimation();

		// Model Strings

		void HandleModelString(ArgList* pArgList);

		// State

		LTBOOL Set(LTBOOL bTurnOffSource, LTBOOL bTurnOnDest, const LTVector& vDestPos);
		LTBOOL Set(LTBOOL bTurnOffSource, LTBOOL bTurnOnDest, const LTVector& vDestPos, const LTVector& vDestDir);
		LTBOOL ResetIfDestStateChanged();
		void   Reset() { m_eState = kUnset; }

		LTBOOL IsUnset() { return m_eState == kUnset; }
		LTBOOL IsSet() { return ( (m_eState == kSwitching) || (m_eState == kMoving) ); }
		LTBOOL IsDone() { return m_eState == kDone; }

	protected:

		LTBOOL TurnOffSource(AIVolume* pVolumeSource);
		LTBOOL TurnOnDest(AIVolume* pVolumeDest);

	protected :

		State						m_eState;
		CAIHumanStrategyFollowPath* m_pStrategyFollowPath;
		LTObjRef					m_hLightSwitchNode;
		AIVolume*					m_pVolume;
		AIVolume*					m_pNextVolume;
		AIVolume*					m_pDestVolume;
		LTBOOL						m_bDoneLitCondition;
};

#endif