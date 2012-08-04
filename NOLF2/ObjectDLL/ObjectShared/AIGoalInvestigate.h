// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalInvestigate.h
//
// PURPOSE : AIGoalInvestigate class definition
//
// CREATED : 6/7/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_INVESTIGATE_H__
#define __AIGOAL_INVESTIGATE_H__

#include "AIGoalAbstractSearch.h"


// Forward declarations.
class AINodeUseObject;
enum  EnumAISoundType;
enum  EnumAIStimulusID;
enum  EnumAITargetMatchID;
enum  EnumAIStimulusType;

typedef std::multimap< EnumAITargetMatchID, EnumAISenseType > AI_INVESTIGATION_MEMORY_MAP;


class CAIGoalInvestigate : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalInvestigate, kGoal_Investigate);

		CAIGoalInvestigate( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();
		virtual void DeactivateGoal();

		// Updating.

		void UpdateGoal();

		// Damage Handling.

		virtual LTBOOL HandleDamage(const DamageStruct& damage);

		// Volume Handling.

		virtual void HandleVolumeEnter(AIVolume* pVolume);
		virtual void HandleVolumeExit(AIVolume* pVolume);

		// Triggered AI Sounds.

		virtual LTBOOL SelectTriggeredAISound(EnumAISoundType* peAISoundType);

		// Sense Handling.

		virtual LTBOOL HandleGoalSenseTrigger(AISenseRecord* pSenseRecord);

	protected:

		// State Handling.

		void SetStateInvestigate();
		void HandleStateInvestigate();
		void HandleStateUseObject();
		void HandleStateLookAt();
		void SearchOrAware(LTBOOL bOnlyAware);

		// Guard handling.

		LTBOOL HandleGuardLimit();

		// Investigation volume reservation.

		LTBOOL ReserveInvestigationVolume(AIVolume* pVolume);

	protected:

		EnumAISenseType		m_eDisturbanceSenseType;
		EnumAIStimulusType	m_eDisturbanceStimulusType;
		LTObjRef			m_hStimulusTarget;
		LTVector			m_vStimulusPos;
		LTVector			m_vStimulusDir;
		uint32				m_nStimulusAlarmLevel;
		EnumAIStimulusID	m_eStimulusID;
		LTObjRef			m_hNodeUseObject;
		EnumAISoundType		m_eAISound;
		LTBOOL				m_bEnforceGuardLimit;
		LTBOOL				m_bInvestigationFailed;
		AIVolume*			m_pInvestigationVolume;
		LTBOOL				m_bFaceAllyForward;
		LTVector			m_vAllyForward;
		LTBOOL				m_bLookOnly;

		AI_INVESTIGATION_MEMORY_MAP	m_mapInvestMemory;
};


#endif
