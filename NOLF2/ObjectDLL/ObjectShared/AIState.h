// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_STATE_H__
#define __AI_STATE_H__

#include "MusicMgr.h"
#include "AIClassFactory.h"
#include "UberAssert.h"

// Forward Declarations.
struct DamageStruct;
struct AISenseRecord;
class CAISense;
class AIVolume;
class CAI;
class CParsedMsg;

//
// ENUM: Types of states.
//
enum EnumAIStateType
{
	#define STATE_TYPE_AS_ENUM 1
	#include "AIStateTypeEnums.h"
	#undef STATE_TYPE_AS_ENUM

	kState_Count,
};

//
// STRINGS: const strings for state types.
//
static const char* s_aszStateTypes[] =
{
	#define STATE_TYPE_AS_STRING 1
	#include "AIStateTypeEnums.h"
	#undef STATE_TYPE_AS_STRING
};

//
// ENUM: State Status.
//
enum EnumAIStateStatus
{
	kSStat_Invalid,
	kSStat_Asleep,
	kSStat_Attacking,
	kSStat_Awake,
	kSStat_Conscious,
	kSStat_Disappearing,
	kSStat_FailedComplete,
	kSStat_FailedEngage,
	kSStat_FailedSetPath,
	kSStat_FindCover,
	kSStat_Flee,
	kSStat_Following,
	kSStat_GotoCover,
	kSStat_Holding,
	kSStat_HolsterWeapon,
	kSStat_Initialized,
	kSStat_Junction,
	kSStat_Landing,
	kSStat_Lost,
	kSStat_Moving,
	kSStat_Panic,
	kSStat_PathComplete,
	kSStat_Paused,
	kSStat_Posing,
	kSStat_Pursue,
	kSStat_Reappearing,
	kSStat_RegainingConsciousness,
	kSStat_Resurrecting,
	kSStat_Retry,
	kSStat_StateComplete,
	kSStat_TakingOff,
	kSStat_TriplePhaseOne,
	kSStat_TriplePhaseTwo,
	kSStat_TriplePhaseThree,
	kSStat_Unconscious,
	kSStat_Uninitialized,
	kSStat_UseCover,
	kSStat_Waiting,
};


//DECLARE_SPECIFIC_AI_FACTORY(State);


//
// CLASS: State
//
class CAIState : public CAIClassAbstract
{
	public : // Public methods

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(State);

		CAIState( );
		virtual ~CAIState( );

		// Ctors/Dtors/etc

		LTBOOL Init(CAI* pAI);

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Methods

		virtual void PreUpdate();
		virtual void Update();
		virtual void PostUpdate();
		virtual void UpdateAnimation() {}
		virtual void UpdateMusic();

		// Handlers

		virtual void HandleNameValuePair(char *szName, char *szValue);
		virtual void HandleDamage(const DamageStruct& damage) {}
		virtual void HandleModelString(ArgList* pArgList) {}
		virtual void HandleTouch(HOBJECT hObject) {}
		virtual bool HandleCommand(const CParsedMsg &cMsg);
		virtual void HandleSense(CAISense* pAISense) {}
		virtual void HandleSense(AISenseRecord* pAISenseRecord) {}
		virtual void HandleVolumeEnter(AIVolume* pVolume) {}
		virtual void HandleVolumeExit(AIVolume* pVolume) {}

		// Misc

		virtual LTBOOL CanActivate() { return LTTRUE; }
		virtual LTBOOL CanReturn() { return LTFALSE; }
        virtual HSTRING CreateReturnString() { return LTNULL; }
		
		EnumAIStateStatus GetStateStatus() { return m_eStateStatus; }
		virtual void SetStateStatus(EnumAIStateStatus eStateStatus) { m_eStateStatus = eStateStatus; }

		LTBOOL IsFirstUpdate() { return m_bFirstUpdate; }
		virtual LTBOOL CausesAllyDisturbances() const { return LTTRUE; }

		LTBOOL HasNext() { return m_cNexts > 0; }
		void NextOr(const char* szState);

		void PlayFirstSound(LTBOOL bPlay) { m_bPlayFirstSound = bPlay; }
		void SetAllowDialogue(LTBOOL b) { m_bNoCinematics = !b; }

		virtual HMODELANIM GetDeathAni(LTBOOL bFront);
		virtual EnumAIStateType GetBodyState() { return kState_BodyNormal; }

		virtual CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodNone; }

		virtual LTBOOL DelayChangeState() { return LTFALSE; }
		virtual LTBOOL RejectChangeState() { return LTFALSE; }

		virtual LTBOOL CanBeDamagedAsAttachment() { return LTTRUE; }

		virtual LTBOOL CanAnimate() { return LTTRUE; }

		const char* GetName() { return s_aszStateTypes[GetStateType()]; }

	protected : // Protected methods

		// Simple accessors

		LTFLOAT GetElapsedTime() { return m_fElapsedTime; }

	protected : // Protected constants

		enum Constants
		{
			kMaxNexts	= 8,
		};

	protected : // Private member variables

		CAI*				m_pAI;						// Backpointer to our AI
		LTBOOL				m_bNoCinematics;			// Do we not do cinematics in this state?
		LTFLOAT				m_fElapsedTime;				// How long have we been in this state?
		LTBOOL				m_bFirstUpdate;				// Is this our first update?
		LTBOOL				m_bPlayFirstSound;			// Should we play our first sound?
		HSTRING				m_hstrReturn;				// A state we can return to if this fails
		int					m_cNexts;					// How many nexts we have specified
		HSTRING				m_ahstrNexts[kMaxNexts];	// Commands to issue after we're done
		HSTRING				m_hstrFirst;				// Message we send on first update
		EnumAIStateStatus	m_eStateStatus;				// What step of the state are we on?
};					

#endif
