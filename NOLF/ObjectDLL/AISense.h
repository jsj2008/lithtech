// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_SENSE_H__
#define __AI_SENSE_H__

#include "AIButeMgr.h"

class CCharacter;
class CAI;
class CDeathScene;

class CAISense
{
	public : // Enums

		enum Constants
		{
			kNumSenses				= 11,
		};

	public : // Public methods

		// Ctors/dtors/etc

		CAISense();

		virtual void Clear();
		virtual void Init(CAI* pAI);

		// Updates

		virtual void PreUpdate();
        virtual LTBOOL Update(HOBJECT hStimulus, LTFLOAT fTimeDelta) = 0;
		virtual void PostUpdate(LTFLOAT fTimeDelta);

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Methods

        void IncreaseStimulation(LTFLOAT fTimeDelta, LTFLOAT fRateModifier = 1.0f);
        void DecreaseStimulation(LTFLOAT fTimeDelta, LTFLOAT fRateModifier = 1.0f);

		// Simple accessors

		virtual const char* GetName() = 0;
		virtual SenseType GetType() = 0;
		virtual SenseClass GetClass() = 0;

        LTBOOL IsEnabled() const { return m_bEnabled; }

        LTFLOAT GetDistance() const { return m_fDistance; }
        LTFLOAT GetDistanceSqr() const { return m_fDistanceSqr; }

		void SetOutcome(SenseOutcome soOutcome) { m_soOutcome = soOutcome; }
		SenseOutcome GetOutcome() const { return m_soOutcome; }

		HOBJECT GetStimulus() const { return m_hStimulus; }
		const LTVector& GetStimulusPosition() const { return m_vStimulusPosition; }

		// Stimulation

        LTFLOAT GetStimulation() { return m_fStimulation; }
        LTFLOAT GetStimulationTime() { return m_fStimulationTime; }

		void IncreaseFalseStimulation() { m_cFalseStimulation++; }
		int GetFalseStimulation() { return m_cFalseStimulation; }
		int GetFalseStimulationLimit(){ return m_nFalseStimulationLimit; }

        void ClearStimulationPartial() { m_bStimulationPartial = LTFALSE; }
        void ClearStimulationFull() { m_bStimulationFull = LTFALSE; }
        LTBOOL HasStimulationPartial() { return m_bStimulationPartial; }
        LTBOOL HasStimulationFull() { return m_bStimulationFull; }

		// Delay

        LTBOOL IsReacting() { return m_bReacting; }
        void React() { m_bReacting = LTTRUE; }
        LTFLOAT GetReactionDelay() { return (m_fReactionDelay); }
        LTFLOAT GetReactionDelayTimer() { return m_fReactionDelayTimer; }

		// Sense recording

        void SetTimestamp(LTFLOAT fTimestamp) { m_fTimestamp = fTimestamp; }
        LTFLOAT GetTimestamp() { return m_fTimestamp; }

		// Simple accessors

		CAI* GetAI() { return m_pAI; }

	protected : // Protected methods

		friend class CAISenseMgr;
		friend class CCharacterMgr;

		// Simple accessors

        void SetUpdated(LTBOOL bUpdated) { m_bUpdated = bUpdated; }
        LTBOOL IsUpdated() { return m_bUpdated; }

        void SetEnabled(LTBOOL bEnabled) { m_bEnabled = bEnabled; }
        void Ensable() { m_bEnabled = LTTRUE; }
        void Disable() { m_bEnabled = LTFALSE; }

		void SetStimulus(HOBJECT hStimulus);

		// Helpers

		virtual void GetAttributes(int nTemplateID) = 0;

		void GetProperties(GenericProp* pgp);
		virtual const char* GetPropertiesEnabledString() = 0;
		virtual const char* GetPropertiesDistanceString() = 0;

		void ComputeSquares();

		// Engine junk

		virtual void Save(HMESSAGEWRITE hWrite);
		virtual void Load(HMESSAGEREAD hRead);

        void Link(HOBJECT hObject) { if ( hObject ) g_pLTServer->CreateInterObjectLink(m_hObject, hObject); }
        void Unlink(HOBJECT hObject) { if ( hObject ) g_pLTServer->BreakInterObjectLink(m_hObject, hObject); }

	protected : // Protected member variables

		// Simple member variables

		CAI*		m_pAI;								// AI backpointer
		HOBJECT		m_hObject;							// AI HOBJECT

		// These are properties that determine what activates the sense

        LTBOOL       m_bEnabled;                         // Are we enabled?
        LTFLOAT      m_fDistance;                        // The distance at which the sense can perceive its stimulus
        LTFLOAT      m_fDistanceSqr;                     // This value squared, for convenience's sake
        LTBOOL       m_bUpdated;                         // Did we update last frame? If not, we get cleared out

		// Shared variables

		SenseOutcome	m_soOutcome;					// The outcome of the sense stimulation

		HOBJECT			m_hStimulus;					// The object that is causing the stimulus

		LTVector		m_vStimulusPosition;			// Position of the stimulus for footstep, disturbance, allydeath, allypain

		// Variables for stimulation-based senses

        LTFLOAT          m_fStimulation;                     // Our current level of stimulation
        LTFLOAT          m_fStimulationIncreaseRateAlert;    // The rate at which stimulation increases in the presence of the stimulus, if we are alert
        LTFLOAT          m_fStimulationDecreaseRateAlert;    // The rate at which stimulation decreases in the absence of the stimulus, if we are alert
        LTFLOAT          m_fStimulationIncreaseRateUnalert;  // The rate at which stimulation increases in the presence of the stimulus, if we are unalert
        LTFLOAT          m_fStimulationDecreaseRateUnalert;  // The rate at which stimulation decreases in the absence of the stimulus, if we are unalert
        LTFLOAT          m_fStimulationTime;                 // The last time this sense was stimulated
        CRange<LTFLOAT>  m_rngStimulationThreshhold;         // The partial and full stimulation threshholds
        LTBOOL           m_bStimulationPartial;              // Have we achieved partial stimulation?
        LTBOOL           m_bStimulationFull;                 // Have we achieved full stimulation?
        LTBOOL           m_bIncreasedStimulation;            // A flag (does not need saving) that says whether or not our stimulation was increased this update

		int				m_cFalseStimulation;				// How many false stimulations have we received
		int				m_nFalseStimulationLimit;			// How many false stimulations can we take before we just cheat and get the full stimulation

		// Variables for delay-based senses

        LTBOOL           m_bReacting;                        // Have we started reacting
        LTFLOAT          m_fReactionDelay;                   // How long we should delay a Reaction based on this sense for?
        LTFLOAT          m_fReactionDelayTimer;              // A timer for this delay (counts down)

		// Data we need to preserve for sense recording

        LTFLOAT          m_fTimestamp;                       // The timestamp on the sense (if there was one)
};

// Macro for adding all the sense properties to an AI

#define ADD_SENSEPROPS(group) \
	ADD_STRINGPROP_FLAG(CanSeeEnemy,					"", group) \
	ADD_STRINGPROP_FLAG(SeeEnemyDistance,				"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanSeeAllyDeath,				"", group) \
	ADD_STRINGPROP_FLAG(SeeAllyDeathDistance,			"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanHearAllyDeath,				"", group) \
	ADD_STRINGPROP_FLAG(HearAllyDeathDistance,			"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanHearAllyPain,				"", group) \
	ADD_STRINGPROP_FLAG(HearAllyPainDistance,			"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanSeeEnemyFlashlight,			"", group) \
	ADD_STRINGPROP_FLAG(SeeEnemyFlashlightDistance,		"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanSeeEnemyFootprint,			"", group) \
	ADD_STRINGPROP_FLAG(SeeEnemyFootprintDistance,		"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanHearEnemyFootstep,			"",	group) \
	ADD_STRINGPROP_FLAG(HearEnemyFootstepDistance,		"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanHearEnemyDisturbance,		"",	group) \
	ADD_STRINGPROP_FLAG(HearEnemyDisturbanceDistance,	"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanHearEnemyWeaponImpact,		"", group) \
	ADD_STRINGPROP_FLAG(HearEnemyWeaponImpactDistance,	"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanHearEnemyWeaponFire,			"", group) \
	ADD_STRINGPROP_FLAG(HearEnemyWeaponFireDistance,	"", group|PF_RADIUS) \
	ADD_STRINGPROP_FLAG(CanHearAllyWeaponFire,			"", group) \
	ADD_STRINGPROP_FLAG(HearAllyWeaponFireDistance,		"", group|PF_RADIUS) \

// All the senses are defined with these here macros

#define BEGIN_DELAY_SENSE_DEFINITION(sense) \
class CAISense##sense## : public CAISense { \
	public : \
 		virtual const char* GetName() { return #sense""; } \
 		virtual SenseType GetType() { return (st##sense##); } \
		virtual SenseClass GetClass() { return (scDelay); } \
	protected : \
		friend class CAISenseMgr; \
		virtual const char* GetPropertiesEnabledString() { return "Can"#sense; } \
		virtual const char* GetPropertiesDistanceString() { return #sense"Distance"; } \
		virtual void GetAttributes(int nTemplateID) \
		{ \
 			m_bEnabled = g_pAIButeMgr->GetTemplate(nTemplateID)->bCan##sense##; \
			m_fDistance = g_pAIButeMgr->GetTemplate(nTemplateID)->f##sense##Distance; \
			m_fReactionDelay = g_pAIButeMgr->GetSenses()->f##sense##ReactionDelay; \
		} \
        virtual LTBOOL Update(HOBJECT hStimulus, LTFLOAT fTimeDelta);

#define END_DELAY_SENSE_DEFINITION() };

#define BEGIN_STIMULATION_SENSE_DEFINITION(sense) \
class CAISense##sense## : public CAISense { \
	public : \
 		virtual const char* GetName() { return #sense""; } \
 		virtual SenseType GetType() { return (st##sense##); } \
		virtual SenseClass GetClass() { return (scStimulation); } \
	protected : \
		friend class CAISenseMgr; \
		virtual const char* GetPropertiesEnabledString() { return "Can"#sense; } \
		virtual const char* GetPropertiesDistanceString() { return #sense"Distance"; } \
		virtual void GetAttributes(int nTemplateID) \
		{ \
 			m_bEnabled = g_pAIButeMgr->GetTemplate(nTemplateID)->bCan##sense##; \
			m_fDistance = g_pAIButeMgr->GetTemplate(nTemplateID)->f##sense##Distance; \
			m_fStimulationIncreaseRateAlert = g_pAIButeMgr->GetSenses()->f##sense##StimulationIncreaseRateAlert; \
			m_fStimulationDecreaseRateAlert = g_pAIButeMgr->GetSenses()->f##sense##StimulationDecreaseRateAlert; \
			m_fStimulationIncreaseRateUnalert = g_pAIButeMgr->GetSenses()->f##sense##StimulationIncreaseRateUnalert; \
			m_fStimulationDecreaseRateUnalert = g_pAIButeMgr->GetSenses()->f##sense##StimulationDecreaseRateUnalert; \
			m_rngStimulationThreshhold = g_pAIButeMgr->GetSenses()->rng##sense##StimulationThreshhold; \
			m_nFalseStimulationLimit = g_pAIButeMgr->GetSenses()->n##sense##FalseStimulationLimit; \
		} \
        virtual LTBOOL Update(HOBJECT hStimulus, LTFLOAT fTimeDelta);

#define END_STIMULATION_SENSE_DEFINITION() };

// SeeEnemy

BEGIN_STIMULATION_SENSE_DEFINITION(SeeEnemy)

	public :

		CAISenseSeeEnemy();

	protected :

		virtual void Save(HMESSAGEWRITE hWrite);
		virtual void Load(HMESSAGEREAD hRead);

	protected : // Protected member variables

		int			m_nGridX;
		int			m_nGridY;
		CRange<int>	m_rngGridX;
		CRange<int>	m_rngGridY;

END_STIMULATION_SENSE_DEFINITION()

// SeeEnemyFootprint

BEGIN_DELAY_SENSE_DEFINITION(SeeEnemyFootprint)
END_DELAY_SENSE_DEFINITION()

// SeeEnemyFlashlight

BEGIN_STIMULATION_SENSE_DEFINITION(SeeEnemyFlashlight)
END_STIMULATION_SENSE_DEFINITION()

// HearEnemyWeaponFire

BEGIN_DELAY_SENSE_DEFINITION(HearEnemyWeaponFire)
END_DELAY_SENSE_DEFINITION()

// HearEnemyWeaponImpact

BEGIN_DELAY_SENSE_DEFINITION(HearEnemyWeaponImpact)
END_DELAY_SENSE_DEFINITION()

// HearEnemyFootstep

BEGIN_STIMULATION_SENSE_DEFINITION(HearEnemyFootstep)
END_STIMULATION_SENSE_DEFINITION()

// HearEnemyDisturbance

BEGIN_DELAY_SENSE_DEFINITION(HearEnemyDisturbance)
END_DELAY_SENSE_DEFINITION()

// SeeAllyDeath

BEGIN_DELAY_SENSE_DEFINITION(SeeAllyDeath)
END_DELAY_SENSE_DEFINITION()

// HearAllyDeath

BEGIN_DELAY_SENSE_DEFINITION(HearAllyDeath)
END_DELAY_SENSE_DEFINITION()

// HearAllyPain

BEGIN_DELAY_SENSE_DEFINITION(HearAllyPain)
END_DELAY_SENSE_DEFINITION()

// HearAllyWeaponFire

BEGIN_DELAY_SENSE_DEFINITION(HearAllyWeaponFire)
END_DELAY_SENSE_DEFINITION()

class CAISenseMgr : DEFINE_FACTORY_CLASS(CAISenseMgr)
{
	DEFINE_FACTORY_METHODS(CAISenseMgr);

	public : // Public methods

		// Ctors/dtors/etc

		void Clear();
		void Init(CAI* pAI);

		// Updates

		void Update();
		void UpdateSense(SenseType st);

        void StopUpdating() { m_bStopUpdating = LTTRUE; }

		void SetUpdateRate(LTFLOAT fUpdateRate) { m_fUpdateRate = fUpdateRate; }
		LTFLOAT GetUpdateRate() const { return m_fUpdateRate; }

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Simple accessors

		LTBOOL IsAlert() const;

        void SetEnabled(LTBOOL bEnabled) { m_bEnabled = bEnabled; }
		LTBOOL IsEnabled() { return m_bEnabled; }

		CAISense* GetSense(SenseType st) { return m_apSenses[st]; }

		// Helpers

		void GetProperties(GenericProp* pgp);
		void GetAttributes(int nTemplateID);

		void ComputeSquares();

		// Engine methods

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

	protected : // Protected member variables

		CAI*			m_pAI;								// AI backpointer
        LTBOOL          m_bEnabled;							// Are we enabled?
        LTBOOL			m_bStopUpdating;					// Does not need to be saved. Reset ever frame.
		LTFLOAT			m_fUpdateRate;						// How often we update
		LTFLOAT			m_fNextUpdateTime;					// Our next update time
															
		CAISense*		m_apSenses[CAISense::kNumSenses];	// Array of pointers to all the senses

		CAISenseSeeEnemy				m_SenseSeeEnemy;				// The senses
		CAISenseSeeEnemyFootprint		m_SenseSeeEnemyFootprint;
		CAISenseSeeEnemyFlashlight		m_SenseSeeEnemyFlashlight;
		CAISenseHearEnemyWeaponFire		m_SenseHearEnemyWeaponFire;
		CAISenseHearEnemyWeaponImpact	m_SenseHearEnemyWeaponImpact;
		CAISenseHearEnemyFootstep		m_SenseHearEnemyFootstep;
		CAISenseHearEnemyDisturbance	m_SenseHearEnemyDisturbance;
		CAISenseSeeAllyDeath			m_SenseSeeAllyDeath;
		CAISenseHearAllyDeath			m_SenseHearAllyDeath;
		CAISenseHearAllyPain			m_SenseHearAllyPain;
		CAISenseHearAllyWeaponFire		m_SenseHearAllyWeaponFire;

};

class CAISenseRecord : DEFINE_FACTORY_CLASS(CAISenseRecord)
{
	DEFINE_FACTORY_METHODS(CAISenseRecord);

	public :

		// Ctors/Dtors/etc

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Conversion/Comparison methods

		void FromSense(CAISense* pAISense);
        LTBOOL IsSame(CAISenseRecord* pAISenseRecord);

		// Simple accessors

        void SetLifetime(LTFLOAT fLifetime) { m_fLifetime = fLifetime; }
        LTBOOL UpdateLifetime(LTFLOAT fTime) { return m_fLifetime > fTime; }

		HOBJECT GetObject() { return m_hObject; }
        LTFLOAT GetTimestamp() { return m_fTimestamp; }
		SenseType GetSenseType() { return m_stType; }
		SenseOutcome GetSenseOutcome() { return m_soOutcome; }

	protected :

		HOBJECT			m_hObject;
        LTFLOAT         m_fTimestamp;
        LTFLOAT         m_fLifetime;
		SenseType		m_stType;
		SenseOutcome	m_soOutcome;
};

class CAISenseRecorder : DEFINE_FACTORY_CLASS(CAISenseRecorder)
{
	DEFINE_FACTORY_METHODS(CAISenseRecorder);

	public : // Public methods

		// Ctors/Dtors/etc

		CAISenseRecorder();

		void Init(HOBJECT hOwner);

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Updates

		void Update();

		// Handlers

		void HandleBrokenLink(HOBJECT hObject);

		// Recording methods

        LTBOOL IsRecorded( CAISense* pAISense);
		void Record( CAISense* pAISense);

	protected : // Protected methods

		// Helpers

        void Link(HOBJECT hObject) { if ( hObject && m_hOwner ) g_pLTServer->CreateInterObjectLink(m_hOwner, hObject); }
        void Unlink(HOBJECT hObject) { if ( hObject && m_hOwner ) g_pLTServer->BreakInterObjectLink(m_hOwner, hObject); }

	protected :

		HOBJECT					m_hOwner;
		CTList<CAISenseRecord*>	m_alstpSenseRecords[CAISense::kNumSenses];
};

#endif