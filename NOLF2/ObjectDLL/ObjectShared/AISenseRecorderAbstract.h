// ----------------------------------------------------------------------- //
//
// MODULE  : AISenseRecorderAbstract.h
//
// PURPOSE : AISenseRecorderAbstract abstract class definition
//
// CREATED : 5/18/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSE_RECORDER_ABSTRACT_H__
#define __AISENSE_RECORDER_ABSTRACT_H__

#include "AIButeMgr.h"
#include "AISensing.h"

class CAIStimulusRecord;
class CAISenseRecorderAbstract;
enum  EnumAIStimulusID;
enum  EnumAITargetMatchID;


//
// ENUM: Types of senses exclusive bitflags.
//
enum EnumAISenseType
{
	kSense_InvalidType	= 0,
	kSense_None			= 0,

	#define SENSE_TYPE_AS_ENUM 1
	#include "AISenseTypeEnums.h"
	#undef SENSE_TYPE_AS_ENUM

	kSense_All			= 0xffffffff,
};


//
// STRUCT: Sense Record for current state of one sense for an AI 
//


// Use this macro to create AISenseRecords.
#ifdef _DEBUG
#define debug_new_AISenseRecord( aiSenseRecorderAbstract ) AISenseRecord::CreateAISenseRecord( aiSenseRecorderAbstract );
#else // _DEBUG
#define debug_new_AISenseRecord( aiSenseRecorderAbstract ) AISenseRecord::CreateAISenseRecord( aiSenseRecorderAbstract );
#endif // _DEBUG

struct AISenseRecord
{
	private:

		// Prevent direct creation by making constructor private.  Call CreateAISenseRecord to
		// create a new AISenseRecord.
		AISenseRecord(  )
			:	pAIBM_Last_Stimulus	( LTNULL )
		{
		}

	public:

	// To create an AISenseRecord, you must call this function.
	static AISenseRecord* CreateAISenseRecord( CAISenseRecorderAbstract& aiSenseRecorderAbstract );

	EnumAISenseType		eSenseType;				// Type of Sense.
	AIBM_Stimulus*		pAIBM_Last_Stimulus;	// AI bute mgr template for last Stimulus. 
	LTObjRefNotifier	hLastStimulusSource;	// Source of last stimulus.
	LTObjRefNotifier	hLastStimulusTarget;	// Target of last stimulus.
	EnumAITargetMatchID	eLastTargetMatchID;		// Target match ID of last stimulus.
	LTVector			vLastStimulusPos;		// Position in world of last stimulus.
	LTVector			vLastStimulusDir;		// Direction in world of last stimulus.
	uint32				nLastStimulusAlarmLevel;// Alarm level of last stimulus.
	EnumAIStimulusID	eLastStimulusID;		// StimulusMgr ID of last stimulus.
	LTFLOAT				fSenseDistance;			// Distance AI notices Stimulus for this Sense.
	LTFLOAT				fSenseDistanceSqr;		// Squared distance AI notices Stimulus for this Sense.
	LTFLOAT				fCurStimulation;		// Current stimulation, between 0.00 and 1.00.
	LTFLOAT				fMaxStimulation;		// Max stimulation recorded.
	LTFLOAT				fReactionDelayTimer;	// Delay after full stimulation reached.
	LTFLOAT				fReactionDelayTime;		// Randomized delay time for timer.
	LTFLOAT				fLastStimulationTime;	// Time last stimulation occured.
	uint32				nCycle;					// AISenseMgr cycle of last update.
	uint8				cFalseStimulation;		// Count of false stimulations.
	CPoint				ptSightGrid;			// Last visual scanning point.

    void Save(ILTMessage_Write *pMsg);
    void Load(ILTMessage_Read *pMsg);
};

//
// MAP: 
//
typedef std::map<EnumAISenseType, AISenseRecord*> AISENSE_RECORD_MAP;

//
// CLASS: Sense Recorder for use by an AI.
//
class CAISenseRecorderAbstract : public ILTObjRefReceiver
{
	public : // Public methods

		 CAISenseRecorderAbstract();
		~CAISenseRecorderAbstract();

        virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		void	Init(IAISensing* pSensing);
		void	AddSense(EnumAISenseType eSenseType, LTFLOAT fDistance);
		void	RemoveSense(EnumAISenseType eSenseType);
		LTBOOL	HasSense(EnumAISenseType eSenseType);

		void	SetSenseDistance(EnumAISenseType eSenseType, LTFLOAT fDistance);
		LTFLOAT	GetSenseDistance(EnumAISenseType eSenseType);
		LTFLOAT	GetSenseDistanceSqr(EnumAISenseType eSenseType);

		LTBOOL	GetDoneProcessingStimuli() const;
		void	SetDoneProcessingStimuli(LTBOOL bDone);
		void	ClearProcessedStimuli();
		LTBOOL	ProcessStimulus(CAIStimulusRecord* pRecord);

		int		GetIntersectSegmentCount() const;
		void	ClearIntersectSegmentCount();
		void	IncrementIntersectSegmentCount();

		LTBOOL	UpdateSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle);
		void	CopySenseRecord(AISenseRecord* pOrigStimulusRecord);
		
		void	HandleSenses(uint32 nCycle);
		void	RepeatHandleSenses();
		LTBOOL	HasFullStimulation(EnumAISenseType eSenseType);
		LTBOOL  HasAnyStimulation(EnumAISenseType eSenseTypes);
		LTFLOAT	GetStimulation(EnumAISenseType eSenseType);
		HOBJECT	GetStimulusSource(EnumAISenseType eSenseType);
		void	ResetStimulation(EnumAISenseType eSenseTypes);

		AISenseRecord* GetSense(EnumAISenseType eSenseType);

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

			// Abstract methods

		virtual	LTBOOL HandleSpecificStimuli(CAIStimulusRecord* pStimulusRecord, 
											 LTFLOAT* pfRateModifier) = 0;

			// Static methods

		static const char* SenseToString(EnumAISenseType eSenseType);
		static EnumAISenseType SenseFromString(char* szSenseType);

	private: // Private methods.
		
		void	ClearSense(AISenseRecord* pSenseRecord);

		LTBOOL	IncreaseStimulation(AISenseRecord* pSenseRecord, LTFLOAT fRateModifier);
		LTBOOL	DecreaseStimulation(AISenseRecord* pSenseRecord, LTFLOAT fRateModifier);

	protected : // Protected member variables

		IAISensing*			m_pSensing;			// Sensing owner of this SenseRecorder.
		AISENSE_RECORD_MAP	m_mapSenseRecords;	// List SenseRecords for this AI.

		LTBOOL						m_bDoneProcessingStimuli;
		AI_PROCESSED_STIMULI_MAP	m_mapProcessedStimuli;

		int							m_cIntersectSegmentCount;
};

#endif
