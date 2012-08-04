// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalChase.h
//
// PURPOSE : AIGoalChase class definition
//
// CREATED : 6/21/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_CHASE_H__
#define __AIGOAL_CHASE_H__

#include "AIGoalAbstractSearch.h"

enum  EnumAIStateStatus;
class AIVolumeJunction;


struct JunctionRecord
{
	AIVolume* pVolume;
	uint8 mskActionVolumes;
};

typedef std::vector<JunctionRecord> JunctionStack;


class CAIGoalChase : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalChase, kGoal_Chase);

		CAIGoalChase( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();
		virtual void RecalcImportance();

		// Command Handling.

		virtual LTBOOL	HandleNameValuePair(const char *szName, const char *szValue);

		// Triggered AI Sounds.

		virtual LTBOOL SelectTriggeredAISound(EnumAISoundType* peAISoundType);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void			HandleStateLookAt();
		void			HandleStateAware();
		virtual void	HandleStateChase();
		void			HandleStateGoto();
		virtual void	HandleStateSearch();
		void			HandleStateDraw();
		void			SetStateChase(LTBOOL bClearState);
		virtual void	SetStateSearch();
		void			SetChaseStatusLost();
		void			GiveUpChase(LTBOOL bSearch);
		void			AwareOnce();

		// Junction Volume Handling.

		void			HandleJunctionVolume();
		JunctionRecord* HandleJunctionStack();

	protected:

		AIVolume*			m_pJunctionVolume;
		AIVolume*			m_pJunctionActionVolume;
		AIVolume*			m_pLastVolume;

		JunctionStack		m_stkJunctions;

		LTBOOL				m_bLost;
		LTBOOL				m_bGiveUpChase;
		LTBOOL				m_bSeekTarget;
		LTBOOL				m_bNeverGiveUp;
		LTBOOL				m_bKeepDistance;
		LTBOOL				m_bContinueLost;
};

#endif
