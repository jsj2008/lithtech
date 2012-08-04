// ----------------------------------------------------------------------- //
//
// MODULE  : AIStimulusMgr.h
//
// PURPOSE : AIStimulusMgr class definition
//
// CREATED : 5/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSE_MGR_H__
#define __AISENSE_MGR_H__

#include "AIButeMgr.h"
#include "AIClassFactory.h"
#include "LTObjRef.h"


#pragma warning (disable : 4786)
#include <map>
#include <list>
#include <vector>

// Forward declarations.
class CAIStimulusMgr;
class CAI;
class AIVolume;
class RelationSet;
class RelationData;
class IAISensing;

extern CAIStimulusMgr *g_pAIStimulusMgr;

//
// ENUM: Types of stimulus exclusive bitflags.
//
enum EnumAIStimulusType
{
	kStim_InvalidType = 0,

	#define STIMULUS_TYPE_AS_ENUM 1
	#include "AIStimulusTypeEnums.h"
	#undef STIMULUS_TYPE_AS_ENUM
};

enum EnumAIStimulusID
{
	kStimID_Invalid = -1,
	kStimID_Unset = 0,
};

enum EnumAITargetMatchID
{
	kTMID_Invalid = -1,
};

//
// STRINGS: const strings for stimulus types.
//
static const char* s_aszStimulusTypes[] =
{
	#define STIMULUS_TYPE_AS_STRING 1
	#include "AIStimulusTypeEnums.h"
	#undef STIMULUS_TYPE_AS_STRING
};

typedef ObjRefList RESPONDER_LIST;

//
// CLASS: Record of a single stimulus.
//
class CAIStimulusRecord : public CAIClassAbstract
{

	public :
		DECLARE_AI_FACTORY_CLASS(CAIStimulusRecord);

		CAIStimulusRecord( );
		~CAIStimulusRecord( );

		// Ctors/Dtors/etc

	public:
		enum kDynamicPos_Flag
		{
			kDynamicPos_Clear		= 0x00,
			kDynamicPos_TrackSource	= 0x01, 
			kDynamicPos_TrackTarget	= 0x02,
			kDynamicPos_HasOffset	= 0x04,
		};


		EnumAIStimulusType	m_eStimulusType;		// Type of Stimulus.
		EnumAIStimulusID	m_eStimulusID;			// Registration ID assigned by the StimulusMgr.
		AIBM_Stimulus*		m_pAIBM_Stimulus;		// Pointer to Stimlulus' AI bute file template.
		LTObjRefNotifier	m_hStimulusSource;		// Handle to source of stimulus.
		LTObjRefNotifier	m_hStimulusTarget;		// Handle to target of stimulus (optional).
		EnumAITargetMatchID	m_eTargetMatchID;		// ID used to match stimuli affecting the same object.
		AIVolume*			m_pInformationVolume;	// Volume where stimulus occured, for sense masks.
		RelationData		m_RelationData;			// Data describing who the emitter was
		
		typedef std::vector<CharacterAlignment> _listAlignments;
		_listAlignments		m_RequiredAlignment;	// List of alignments which are allowed to match

		LTVector			m_vStimulusPos;			// Position in world of Stimulus.
		LTVector			m_vStimulusDir;			// Direction in world of Stimulus.
		uint32				m_nStimulusAlarmLevel;	// Alarm level of Stimulus.
		LTFLOAT				m_fDistance;			// Distance from source that Stimulus affects.
		LTFLOAT				m_fTimeStamp;			// When Stimulus occured.
		LTFLOAT				m_fExpirationTime;		// When Stimulus expires.		
		uint8				m_nMaxResponders;		// Max number of AIs to trigger (optional).
		RESPONDER_LIST		m_lstCurResponders;		// List of handles to AI responders.
		
		uint32				m_dwDynamicPosFlags;	// Lookup position of Stimulus source every update.
		LTVector			m_vDynamicSourceOffset; // Position offset from the Stimulus source for updating the Stimulus pos every frame. 

		LTBOOL	IsAIResponding(HOBJECT hAI) const;
		void	ClearResponder(HOBJECT hAI);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);
};


//
// VECTOR: List of all sensing objects.
//
typedef std::vector<IAISensing*> AISENSING_LIST;

//
// MAP: Map of all currently existing stimuli.
//
typedef std::multimap<uint8 /*nAIAlarmLevel*/, CAIStimulusRecord* /*srStimulusRecord*/, std::greater<uint8> > AISTIMULUS_MAP;

//
// MAP: Map of all currently existing stimuli.
//
typedef std::map<HOBJECT /*hTarget*/, EnumAITargetMatchID> AITARGET_MATCH_MAP;


//
// STRUCT: Struct used to sort stimulus by distance to player.
//         Used to render stimulus for debugging.
//
struct STIMULUS_DIST_STRUCT
{
	float fDistSqr;
	CAIStimulusRecord* pRecord;

	bool operator< (const STIMULUS_DIST_STRUCT& stimDistStruct) { return bool(fDistSqr < stimDistStruct.fDistSqr); }
};

//
// LIST: List used to sort stimulus by distance to player, for debug rendering.
//
typedef std::list<STIMULUS_DIST_STRUCT> STIMULUS_DIST_LIST;

#define MAX_STIMULUS_RENDER 10



//
// CLASS: Global manager for stimulus.
//
class CAIStimulusMgr : public ILTObjRefReceiver
{
	public : // Public methods

		 CAIStimulusMgr();
		~CAIStimulusMgr();

		void Init();
		void Term();

		virtual void Save(ILTMessage_Write *pMsg);
        virtual void Load(ILTMessage_Read *pMsg);

		// Add/Remove Sensing Objects.

		void AddSensingObject(IAISensing* pSensing);
		void RemoveSensingObject(IAISensing* pSensing);

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Methods

		EnumAIStimulusID	RegisterStimulus(EnumAIStimulusType eStimulusType, HOBJECT hStimulusSource,  
								const LTVector& vPos, LTFLOAT fRadiusFactor);
		EnumAIStimulusID	RegisterStimulus(EnumAIStimulusType eStimulusType, HOBJECT hStimulusSource,  
								HOBJECT hStimulusTarget, const LTVector& vPos, LTFLOAT fRadiusFactor, LTFLOAT fDurationFactor);
		EnumAIStimulusID	RegisterStimulus(EnumAIStimulusType eStimulusType, uint32 nAlarmLevelFactor, HOBJECT hStimulusSource,  
								HOBJECT hStimulusTarget, const LTVector& vPos, LTFLOAT fRadiusFactor);
		EnumAIStimulusID	RegisterStimulus(EnumAIStimulusType eStimulusType, HOBJECT hStimulusSource,  
								const LTVector& vPos, LTFLOAT fRadiusFactor, LTFLOAT fDurationFactor);
		EnumAIStimulusID	RegisterStimulus(EnumAIStimulusType eStimulusType, uint32 nAlarmLevelFactor, HOBJECT hStimulusSource, 
								HOBJECT hStimulusTarget, RelationData cc, const LTVector& vPos, LTFLOAT fRadiusFactor, LTFLOAT fDurationFactor);
		EnumAIStimulusID	RegisterStimulus(EnumAIStimulusType eStimulusType, HOBJECT hStimulusSource,
								HOBJECT hStimulusTarget, CAIStimulusRecord::kDynamicPos_Flag DynamicPosFlag, LTFLOAT fRadiusFactor);
		EnumAIStimulusID	RegisterStimulus(EnumAIStimulusType eStimulusType, HOBJECT hStimulusSource,
								HOBJECT hStimulusTarget, CAIStimulusRecord::kDynamicPos_Flag DynamicPosFlag, const LTVector &vOffset, LTFLOAT fRadiusFactor);

		void	RemoveStimulus(EnumAIStimulusID eStimulusID);

		LTBOOL	StimulusExists(EnumAIStimulusID eStimulusID);

		uint32	GetNumResponders(EnumAIStimulusID eStimulusID);
		void	ClearResponder(EnumAIStimulusID eStimulusID, HOBJECT hResponder);
		void	IgnoreStimulusByTarget(HOBJECT hAI, EnumAIStimulusType eStimulusType, HOBJECT hStimulusTarget);

		uint32	GetNextStimulusResponseIndex() { return m_iNextStimulationResponseIndex++; }

		void	Update();

		void	RenderStimulus(LTBOOL bRender);


		// Static methods

		static const char* StimulusToString(EnumAIStimulusType eStimulusType);
		static EnumAIStimulusType StimulusFromString(char* szStimulusType);

	private:

		EnumAIStimulusID _RegisterStimulus(CAIStimulusRecord* pAIStimulusRecord, uint32 nAlarmLevelFactor,
							   LTFLOAT fRadiusFactor, LTFLOAT fDurationFactor);

		void	UpdateRenderStimulus();
		void	CreateStimulusModel(CAIStimulusRecord* pRecord);

		EnumAITargetMatchID GetTargetMatchID(HOBJECT hTarget);

		bool IsAlignmentInList(const RelationSet& RelationSet,
		const RelationData& RelationData,
		const CAIStimulusRecord::_listAlignments& AlignmentRequirement ) const;

		void	UpdateSensingList();
		LTBOOL	SenseNearestPlayer(IAISensing* pSensing);
		bool	CanSense(IAISensing* pSensing,CAIStimulusRecord* pRecord) const;

	private : // Private member variables

		AISTIMULUS_MAP			m_stmStimuliMap;		// List of existing stimuli, sorted by Alarm level.
		uint32					m_nCycle;				// Cycle counter, for update checks.
		uint32					m_nNextStimulusID;		// Registration ID for stimulus.
		uint32					m_iNextStimulationResponseIndex;	// Unique index for differentiating instances of AIs reacting to stimulus.
		LTBOOL					m_bRenderStimulus;

		uint32					m_nNextTargetMatchID;	// Next ID for matching targets.
		AITARGET_MATCH_MAP		m_mapTargetMatch;		// Map of target handles and associated match IDs.

		LTBOOL					m_bStimulusCriticalSection;

		// Do NOT save the following:

		AISENSING_LIST			m_lstSensing;			// List of sensing objects. Recreated as objects activate/deactivate.
};

#endif
