// ----------------------------------------------------------------------- //
//
// MODULE  : Scanner.h
//
// PURPOSE : An object which scans for the player and then sends a message
//			 (based on old SecurityCamera class)
//
// CREATED : 6/7/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SCANNER_H__
#define __SCANNER_H__

#include "ltengineobjects.h"
#include "Prop.h"
#include "Timer.h"
#include "CommandMgr.h"
#include "AISensing.h"

class CCharacter;
class CObjectRelationMgr;
class RelationData;
class RelationSet;
class CAIStimulusRecord;
enum  EnumAISenseType;

LINKTO_MODULE( Scanner );

class CScanner : public Prop, public IAISensing
{
	public :

		CScanner();
		~CScanner();

        LTBOOL CanSeeObject(ObjectFilterFn fn, HOBJECT hObject);
        LTBOOL CanSeePos(ObjectFilterFn fn, const LTVector& vPos);

		const RelationData& GetRelationData();
		const RelationSet& GetRelationSet();

		void EnableSensing(LTBOOL bEnable);

        static bool DefaultFilterFn(HOBJECT hObj, void *pUserData);
        static bool BodyFilterFn(HOBJECT hObj, void *pUserData);
		static bool SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData);

		enum DetectState { DS_CLEAR, DS_FOCUSING, DS_DETECTED };

		// IAISensing members.

		virtual HOBJECT		GetSensingObject() { return m_hObject; }

		virtual LTBOOL		IsSensing() { return m_bSensing; }
		virtual LTBOOL		IsAlert() { return LTTRUE; }

		virtual LTFLOAT		GetSenseUpdateRate() const { return m_fSenseUpdateRate; }
		virtual LTFLOAT		GetNextSenseUpdate() const { return m_fNextSenseUpdate; }
		virtual void		SetNextSenseUpdate(LTFLOAT fTime) { m_fNextSenseUpdate = fTime; }

		virtual	void		UpdateSensingMembers() {}

		virtual	LTBOOL		GetDoneProcessingStimuli() const;
		virtual	void		SetDoneProcessingStimuli(LTBOOL bDone);
		virtual void		ClearProcessedStimuli();
		virtual LTBOOL		ProcessStimulus(CAIStimulusRecord* pRecord);

		virtual int			GetIntersectSegmentCount() const;
		virtual void		ClearIntersectSegmentCount();
		virtual void		IncrementIntersectSegmentCount();

		virtual LTBOOL		HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle);
		virtual	void		HandleSenses(uint32 nCycle) {}
		virtual void		HandleSenseTrigger(AISenseRecord* pSenseRecord) {}
		virtual LTFLOAT		GetSenseDistance(EnumAISenseType eSenseType);

		virtual uint32		GetCurSenseFlags() const { return m_dwSenses; }

		virtual const RelationSet& GetSenseRelationSet() const;

		virtual const LTVector& GetSensingPosition() const { return m_vScannerPos; }

		virtual CRange<int>& GetSightGridRangeX() { return m_rngSightGrid; }
		virtual CRange<int>& GetSightGridRangeY() { return m_rngSightGrid; }

	protected :

        virtual uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		
		// Required function to initialize the Relation Information.
		// That is, force description of what this object is and how
		// other objects perceive it.
		virtual void InitRelationInformation(void);

		virtual DetectState	UpdateDetect();
		virtual void		SetLastDetectedEnemy(HOBJECT hObj);

        virtual LTVector GetScanPosition() { return m_vScannerPos; }
        virtual LTRotation   GetScanRotation() { LTRotation rRot; g_pLTServer->GetObjectRotation(m_hObject, &rRot); return rRot; }

        void    SetLastDetectedDeathPos(const LTVector &vPos) { m_vLastDetectedDeathPos = vPos; }
		void	SetDestroyedModel();

		virtual void	SetCanProcessDetection(LTBOOL bCanProcess);

        virtual LTFLOAT  GetFocusTime() { return 0.0f; }

		virtual void UpdateRelations(void);
		virtual void PreUpdateRelationsHook(void);

		// [KLS 5/20/02] Override this function defined in Prop so our derived classes can
		// handle destruction on their own
		virtual void HandleDestroy(HOBJECT hDamager) { }

	protected :

        LTFLOAT  m_fFOV;
        LTFLOAT  m_fVisualRange;
        LTFLOAT  m_fVisualRangeSqr;

        LTVector m_vInitialPitchYawRoll;
        LTVector m_vScannerPos;

		HSTRING	m_hstrDestroyedFilename;
		HSTRING	m_hstrDestroyedSkin;

		HSTRING	m_hstrSpotCommand;
		
		LTObjRef m_hLastDetectedEnemy;
        LTVector m_vLastDetectedDeathPos;

		CTimer	m_FocusTimer;

        LTBOOL   m_bCanProcessDetection;

		CObjectRelationMgr* m_pObjectRelationMgr;

		LTObjRef			m_hStimulus;
		EnumAIStimulusID	m_eStimulusID;
		EnumAISenseType		m_eStimulusSenseType;
		LTFLOAT				m_fStimulusTime;

		LTFLOAT				m_fLastResetTime;

		LTBOOL				m_bSensing;

		// IAISensing members.

		uint32						m_dwSenses;
		LTFLOAT						m_fSenseUpdateRate;
		LTFLOAT						m_fNextSenseUpdate;
		CRange<int>					m_rngSightGrid;

		LTBOOL						m_bDoneProcessingStimuli;
		AI_PROCESSED_STIMULI_MAP	m_mapProcessedStimuli;

		int							m_cIntersectSegmentCount;

        // ** EVERYTHING BELOW HERE DOES NOT NEED SAVING

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pInfo);
		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);
};

class CScannerPlugin : public CPropPlugin
{
	public:

		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers );

	protected:

		CCommandMgrPlugin	m_CommandMgrPlugin;
};

#endif // __SCANNER_H__