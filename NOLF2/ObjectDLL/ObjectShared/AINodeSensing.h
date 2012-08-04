// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeSensing.h
//
// PURPOSE : AINodeSensing class definition
//
// CREATED : 4/08/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_SENSING_H_
#define _AI_NODE_SENSING_H_

#include "AINode.h"
#include "AISensing.h"
#include "WeaponMgr.h"

LINKTO_MODULE( AINodeSensing );

// Forward declarations.

class CAISenseRecorderAbstract;
class CObjectRelationMgr;
class CAI;

class AINodeSensing : public AINode, public IAISensing
{
	typedef AINode super;

	enum EnumAlarmLevel 
	{
		kNone	= -1,
		kLow	= 0,
		kMedium = 1,
		kHigh	= 2,
	};

	public :

		// Ctors/Dtors/etc

		AINodeSensing();
		virtual ~AINodeSensing();

		// IAISensing members.

		virtual HOBJECT	GetSensingObject() { return m_hObject; }
		virtual LTBOOL	IsSensing() { return LTTRUE; }
		virtual LTBOOL	IsAlert();

		virtual LTFLOAT	GetSenseUpdateRate() const { return m_fSenseUpdateRate; }
		virtual LTFLOAT	GetNextSenseUpdate() const { return m_fNextSenseUpdate; }
		virtual void	SetNextSenseUpdate(LTFLOAT fTime) { m_fNextSenseUpdate = fTime; }

		virtual	void	UpdateSensingMembers() {}

		virtual	LTBOOL	GetDoneProcessingStimuli() const;
		virtual	void	SetDoneProcessingStimuli(LTBOOL bDone);
		virtual void	ClearProcessedStimuli();
		virtual LTBOOL	ProcessStimulus(CAIStimulusRecord* pRecord);

		virtual int		GetIntersectSegmentCount() const;
		virtual void	ClearIntersectSegmentCount();
		virtual void	IncrementIntersectSegmentCount();

		virtual LTBOOL	HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle);
		virtual	void	HandleSenses(uint32 nCycle);
		virtual void	HandleSenseTrigger(AISenseRecord* pSenseRecord);
		virtual LTFLOAT	GetSenseDistance(EnumAISenseType eSenseType);

		virtual uint32  GetCurSenseFlags() const { return m_dwSenses; }

		virtual	const RelationSet& GetSenseRelationSet() const;

		virtual const LTVector& GetSensingPosition() const { return m_vPos; }

		virtual CRange<int>& GetSightGridRangeX() { return m_rngSightGrid; }
		virtual CRange<int>& GetSightGridRangeY() { return m_rngSightGrid; }

		// Engine

		virtual void ReadProp(ObjectCreateStruct *pocs);
		virtual bool OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );
		virtual uint32 EngineMessageFn(uint32 messageID, void *pvData, LTFLOAT fData);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);
		
		// Senses

		void CopySense(CAI* pAI);

		// Type

		EnumAINodeType GetType() { return kNode_Sensing; }

	protected :

		void	InitialUpdate();
		void	ResetSensingNode();
		void	SendSenseCommand();

	protected:

		int							m_nTemplateID;
		uint32						m_dwSenses;
		LTFLOAT						m_fSenseUpdateRate;
		LTFLOAT						m_fNextSenseUpdate;
		uint32						m_nAlarmLevel;
		uint32						m_nHighAlarmThreshold;						
		uint32						m_nMediumAlarmThreshold;						
		LTFLOAT						m_fTimer[3];
		LTFLOAT						m_fResetTime[3];
		HSTRING						m_hstrCmd[3];
		HSTRING						m_hstrCleanupCmd;
		EnumAlarmLevel				m_eCmdToRun;
		EnumAISenseType				m_eSenseType;
		CAISenseRecorderAbstract*	m_pAISenseRecorder;
		CObjectRelationMgr*			m_pRelationMgr;
		CRange<int>					m_rngSightGrid;
};

//-----------------------------------------------------------------

class AINodeSensingPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);

private:
	CWeaponMgrPlugin	m_WeaponMgrPlugin;
};

//-----------------------------------------------------------------

#endif // _AI_NODE_SENSING_H_
