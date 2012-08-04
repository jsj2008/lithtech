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

#include "AIClassFactory.h"
#include "ltobjref.h"
#include "CharacterAlignment.h"
#include "AIEnumStimulusTypes.h"
#include "CharacterAlignment.h"

#pragma warning (disable : 4786)
#include <map>
#include <list>
#include <vector>

// Forward declarations.
class CAIStimulusMgr;
class CAI;
struct AIDB_StimulusRecord;

extern CAIStimulusMgr *g_pAIStimulusMgr;

typedef ObjRefList RESPONDER_LIST;

// ----------------------------------------------------------------------- //
//
//	CLASS:		StimulusRecordCreateStruct
//
//	PURPOSE:	This class is used to Register a stimulus through one of 
//				the RegisterStimulus overloads.  This struct contains all
//				stimulus registration fields.
//
// ----------------------------------------------------------------------- //

struct StimulusRecordCreateStruct
{
	// Once constructed, a stimulus record description is valid, as all 
	// required fields are filled out.  Additional fields may be filled 
	// out as desired.

	StimulusRecordCreateStruct(EnumAIStimulusType eType, EnumCharacterAlignment eAlignment, LTVector vPos, HOBJECT hSource)
		: m_eStimulusType(eType)
        , m_hStimulusSource(hSource)
	    , m_vStimulusPos(vPos)
	    , m_eAlignment(eAlignment)
	    , m_hStimulusTarget(NULL)
	    , m_flAlarmScalar(1.0f)
	    , m_flRadiusScalar(1.0f)
	    , m_flDurationScalar(1.0f)
		, m_dwDynamicPosFlags(0)
		, m_vOffset(0.f, 0.f, 0.f)
		, m_eDamageType(DT_INVALID)
		, m_fDamageAmount(0.f)
		, m_vDamageDir(0.f, 0.f, 0.f)
	{
	}

	// Default constructor is necessary to make stl vectors of stim create structs.

	StimulusRecordCreateStruct()
		: m_eStimulusType(kStim_InvalidType)
		, m_hStimulusSource(NULL)
		, m_vStimulusPos(0.f, 0.f, 0.f)
		, m_eAlignment(kCharAlignment_Invalid)
		, m_hStimulusTarget(NULL)
		, m_flAlarmScalar(1.0f)
		, m_flRadiusScalar(1.0f)
		, m_flDurationScalar(1.0f)
		, m_dwDynamicPosFlags(0)
		, m_vOffset(0.f, 0.f, 0.f)
		, m_eDamageType(DT_INVALID)
		, m_fDamageAmount(0.f)
		, m_vDamageDir(0.f, 0.f, 0.f)
	{
	}

	virtual void	Save(ILTMessage_Write *pMsg);
	virtual void	Load(ILTMessage_Read *pMsg);

	//
	// Mandatory:
	//

	// Type of the stimulus.
	EnumAIStimulusType		m_eStimulusType;
	
	// Object which emitted the stimulus.
	LTObjRef				m_hStimulusSource;

	// Position of the stimulus (if the position is flagged as dynamic,
	// this field may be stomped with the cooresponding object)
	LTVector				m_vStimulusPos;
	
	// Alignment of the object which emitted this stimulus.  This is used
	// to satisfy require stance constraints.
	EnumCharacterAlignment	m_eAlignment;


	//
	// Optional:
	//

	// Handle to the 'target' of the stimulus.  This is used in different 
	// ways by different stimulus types
	LTObjRef				m_hStimulusTarget;
	
	// Scalar to handle the alarm level of a stimulus.  By default, this value
	// is 1.0.  If the database alarm level is 1.0, this may be used to 
	// programatically control alarm level.
	float					m_flAlarmScalar;

	// Scalar to handle the radius of a stimulus.  By default, this value
	// is 1.0.  If the database radius is 1.0, this may be used to 
	// programatically control alarm level.
	float					m_flRadiusScalar;

	// Scalar to handle the duration of a stimulus.  By default, this value
	// is 1.0.  If the database radius is 1.0, this may be used to 
	// programatically control alarm level.
	float					m_flDurationScalar;

	// By default, 0.  This controls how/if the position of the stimulus
	// is updated every frame.
	uint32					m_dwDynamicPosFlags;
	
	// By default, 0, 0, 0.  This controls how to offset the position 
	// stimulus every frame from the position specified through DynamicPos. 
	LTVector				m_vOffset;
	
	// Damage type of the of the stimulus
	DamageType				m_eDamageType;

	// Damage amount of the stimulus.
	float					m_fDamageAmount;

	// Damage direction of the stimulus.
	LTVector				m_vDamageDir;
};

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

		CAIStimulusRecord*	m_pStimulusRecordNext;

		EnumAIStimulusType	m_eStimulusType;		// Type of Stimulus.
		EnumAIStimulusID	m_eStimulusID;			// Registration ID assigned by the StimulusMgr.
		AIDB_StimulusRecord*m_pAIDB_Stimulus;		// Pointer to Stimlulus' AI bute file template.
		LTObjRefNotifier	m_hStimulusSource;		// Handle to source of stimulus.
		LTObjRefNotifier	m_hStimulusTarget;		// Handle to target of stimulus (optional).
		STANCE_BITS			m_bitsRequiredStance;	// List of stances which are allowed to match

		LTVector				m_vStimulusPos;			// Position in world of Stimulus.
		LTVector				m_vStimulusDir;			// Direction in world of Stimulus.
		uint32					m_nStimulusAlarmLevel;	// Alarm level of Stimulus.
		float					m_fDistance;			// Distance from source that Stimulus affects.
		double					m_fTimeStamp;			// When Stimulus occured.
		double					m_fExpirationTime;		// When Stimulus expires.		
		RESPONDER_LIST			m_lstCurResponders;		// List of handles to AI responders.
		EnumCharacterAlignment	m_eAlignment;			// Alignment of the character causing the stimulus.

		DamageType				m_eDamageType;			// Optional damage type of the stimulus.
		float					m_fDamageAmount;		// Optional damage amount of the stimulus.
		LTVector				m_vDamageDir;			// Optional damage direction of the stimulus.

		uint32				m_dwDynamicPosFlags;	// Lookup position of Stimulus source every update.
		LTVector			m_vDynamicSourceOffset; // Position offset from the Stimulus source for updating the Stimulus pos every frame. 

		bool	IsAIResponding(HOBJECT hAI) const;
		void	ClearResponder(HOBJECT hAI);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);
};


//
// VECTOR: List of all sensing objects.
//
typedef std::vector<
			CAI*, 
			LTAllocator<CAI*, LT_MEM_TYPE_OBJECTSHELL> 
		> AISENSING_LIST;


//
// STRUCT: Struct used to sort stimulus by distance to player.
//         Used to render stimulus for debugging.
//
struct STIMULUS_DIST_STRUCT
{
	float fDistSqr;
	CAIStimulusRecord* pRecord;

	bool operator< (const STIMULUS_DIST_STRUCT& stimDistStruct) const { return bool(fDistSqr < stimDistStruct.fDistSqr); }
};

//
// LIST: List used to sort stimulus by distance to player, for debug rendering.
//
typedef std::list<
			STIMULUS_DIST_STRUCT,
			LTAllocator<STIMULUS_DIST_STRUCT, LT_MEM_TYPE_OBJECTSHELL> 
		> STIMULUS_DIST_LIST;

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

		void AddSensingObject(CAI* pSensing);
		void RemoveSensingObject(CAI* pSensing);

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Methods

		EnumAIStimulusID	RegisterStimulus(const StimulusRecordCreateStruct& rDesc);

		void	InsertStimulusRecord( CAIStimulusRecord* pStimulusRecord );
		void	RemoveStimulus(EnumAIStimulusID eStimulusID);

		EnumAIStimulusID	GetNextStimulusID();
		bool				StimulusExists(EnumAIStimulusID eStimulusID);
		uint32				CountStimulusRecords() const;
		CAIStimulusRecord*	GetStimulusRecord( EnumAIStimulusID eStimulusID );

		uint32	GetNumResponders(EnumAIStimulusID eStimulusID);
		void	ClearResponder(EnumAIStimulusID eStimulusID, HOBJECT hResponder);
		void	IgnoreStimulusByTarget(HOBJECT hAI, EnumAIStimulusType eStimulusType, HOBJECT hStimulusTarget);

		uint32	GetNextStimulusResponseIndex() { return m_iNextStimulationResponseIndex++; }

		void	Update();

		void	RenderStimulus(bool bRender);
		void	OnAIDebugCmd( HOBJECT hSender, const CParsedMsg& cParsedMsg );


		// Static methods

		static const char* StimulusToString(EnumAIStimulusType eStimulusType);
		static EnumAIStimulusType StimulusFromString(const char* szStimulusType);

	private:

		void	UpdateRenderStimulus();
		void	CreateStimulusModel(CAIStimulusRecord* pRecord);

		bool IsStanceInList( EnumCharacterAlignment eAlignmentA,
							EnumCharacterAlignment eAlignmentB,
							const STANCE_BITS& bitsStanceRequirements ) const;

		void	UpdateSensingList();
		bool	SenseNearestPlayer(CAI* pSensing);
		bool	CanSense(CAI* pSensing,CAIStimulusRecord* pRecord) const;

	private : // Private member variables

		CAIStimulusRecord*		m_pStimuliListHead;		// Head of the list of existing stimuli, sorted by Alarm level.
		uint32					m_nCycle;				// Cycle counter, for update checks.
		uint32					m_nNextStimulusID;		// Registration ID for stimulus.
		uint32					m_iNextStimulationResponseIndex;	// Unique index for differentiating instances of AIs reacting to stimulus.
		bool					m_bRenderStimulus;

		bool					m_bStimulusCriticalSection;

		// Do NOT save the following:

		AISENSING_LIST			m_lstSensing;			// List of sensing objects. Recreated as objects activate/deactivate.
};

#endif

