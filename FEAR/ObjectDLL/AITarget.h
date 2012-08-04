// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_TARGET_H__
#define __AI_TARGET_H__

#include "AIClassFactory.h"
#include "ltobjref.h"
#include "AITargetSelectAbstract.h"
#include "AIEnumStimulusTypes.h"

// Forward declarations.

class	CAIWMFact;
class	CCharacter;


enum ENUM_AI_TARGET_TYPE
{
	kTarget_None				= 0,
	kTarget_Berserker			= (1<<0),
	kTarget_Character			= (1<<1),
	kTarget_CombatOpportunity	= (1<<2),
	kTarget_Disturbance			= (1<<3),
	kTarget_Interest			= (1<<4),
	kTarget_Leader				= (1<<5),
	kTarget_Object				= (1<<6),
	kTarget_WeaponItem			= (1<<7),
	kTarget_Follower			= (1<<8),
	kTarget_All					= 0xffff,
};

enum ENUM_AI_TARGET_POS_TRACKING_FLAGS
{
	kTargetTrack_None			= 0,
	kTargetTrack_Normal			= 0x01,
	kTargetTrack_Once			= 0x02,
	kTargetTrack_SeekEnemy		= 0x04,
	kTargetTrack_Squad			= 0x10,
};


class CAITarget : public CAIClassAbstract
{
	public : // Public methods

		DECLARE_AI_FACTORY_CLASS(CAITarget);

		CAITarget( );

		HOBJECT GetVisionBlocker() const { return m_hVisionBlocker; }

		void ClearTarget( CAI* pAI );
		void UpdateTarget();

		void ResetNodeTrackingTarget();
		void UpdateNodeTracking();

		void UpdateVisibility(const LTVector& vCheckPos);
		void UpdatePush( CCharacter* pChar );

        const LTVector& GetVisiblePosition() const { return m_vVisiblePosition; }
		float			GetTargetDistSqr() const { return m_fTargetDistSqr; }

		float			GetCurMovementInaccuracy() const { return m_fCurMovementInaccuracy; }

		void SetPushSpeed(float fPushSpeed) { m_fPushSpeed = fPushSpeed; }
		void SetPushMinDist(float fPushMinDist) { m_fPushMinDist = fPushMinDist; m_fPushMinDistSqr = fPushMinDist* fPushMinDist; }

		void GetDebugInfoString(std::string& OutInfo);

	protected : // Protected methods

		friend class CAI;

		void InitTarget(CAI* pAI);
		CAI* GetAI() { return m_pAI; }

		EnumAITargetSelectType	SelectAITargetSelection();
		void					DeselectAITargetSelection();
		void					TrackTargetPosition();

        void SetCanUpdateVisibility(bool bCanUpdate) { m_bCanUpdateVisibility = bCanUpdate; }
		void SetVisiblePosition(const LTVector& vPosition) { m_vVisiblePosition = vPosition; }

        void UpdateShootPosition(const LTVector& vShootPosition, float fError, bool bNewError = true);

		void SetupTargetObject(HOBJECT hObj);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

	private : // Private member variables

		void ClearInvalidCharacterFacts();

		CAI*		m_pAI;
		bool		m_bCanUpdateVisibility;
		LTObjRef	m_hVisionBlocker;

		EnumAITargetSelectType	m_eAITargetSelection;
		EnumCharacterAlignment 	m_eTargetAlignment;

		bool		m_bTriggerTracker;
		bool		m_bTrackingLastVisible;

		float		m_fCurMovementInaccuracy;

		uint32		m_dwLastTargetPosTrackingFlags;

		LTVector	m_vTargetVelocity;
        LTVector	m_vVisiblePosition;
		float		m_fTargetDistSqr;
		int32		m_nPhase;
		float		m_fPhaseStep;

		float		m_fPushSpeed;
		float		m_fPushMinDist;
		float		m_fPushMinDistSqr;
		float		m_fPushThreshold;

		bool		m_bPrimaryOnLeft;
};

#endif
