//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStateSentryChallenge.h
//              
//	PURPOSE:	CAIHumanStateSentryChallenge declaration
//              
//	CREATED:	05.12.2001
//
//	(c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __AIHUMANSTATESENTRYCHALLENGE_H__
#define __AIHUMANSTATESENTRYCHALLENGE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "AIHumanState.h"

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:		Challenge
//              
//	PURPOSE:	Helper object for tracking the scan state between a scanning
//				character and scanned.  Mainly a separate class to insure
//				separation of the Challenge itself from the AI system using
//				the results of the challenge.
//              
//----------------------------------------------------------------------------
class Challenge
{
public:
	enum eChallengeResult
	{
		kCR_Invalid = 0,
		kCR_Pass,
		kCR_Fail,
		kCR_Unknown,
	};

	Challenge() { m_eChallengeResult = kCR_Invalid; }

	void Load(ILTMessage_Read *pMsg);
	void Save(ILTMessage_Write *pMsg);

	// Modifying Methods

	void DoChallenge( const CCharacter* pChallengedChar );
	void Clear() { m_eChallengeResult = kCR_Unknown; }
	void SetResult( eChallengeResult eResult ) { m_eChallengeResult = eResult; }

	// Non Modifying Methods
	
	eChallengeResult GetResult() const { return m_eChallengeResult; }
	bool HasValidResult() const { return ( m_eChallengeResult != kCR_Invalid ); }

private:
	// If not INVALID, this is the results of the challenge
	eChallengeResult m_eChallengeResult;
};


//----------------------------------------------------------------------------
//              
//	CLASS:		CLastKnownPosition
//              
//	PURPOSE:	Helper class for remembering the last location the AI saw an 
//				Object
//              
//----------------------------------------------------------------------------
class CLastKnownPosition
{
public:
	CLastKnownPosition() { m_bVisibilityCheckCached = false; }

	void Load(ILTMessage_Read *pMsg);
	void Save(ILTMessage_Write *pMsg);

	// Modifying Methods

	LTBOOL	IsVisibile(CAI* pAI, HOBJECT hTarget);
	void	Update(CAI* pAI, HOBJECT hTarget);
	void	Clear() { m_bVisibilityCheckCached = false; }

	// Non Modifying Methods

	float	GetRangeSqr(CAI* pAI) const;
	const LTVector& GetLKP(void) const { return m_vLastKnownPosition; }

private:
	void	GenerateVisibility(CAI* pAI, HOBJECT hTarget);
	void	SetLKP(HOBJECT hTarget, const LTVector* pvLastKnownPosition=NULL);

	LTVector	m_vLastKnownPosition;		// Position the enemy was last seen
	LTBOOL		m_bVisibilityCheckResult;	// Current visual state
	LTBOOL		m_bVisibilityCheckCached;	// Cache valid flag
};

//----------------------------------------------------------------------------
//              
//	CLASS:	CAIHumanStateSentryChallenge
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CAIHumanStateSentryChallenge : public CAIHumanState
{
	public :
		enum eActionType
		{
			kFailedAction,
			kPassedAction,
			kChallengeAction,
		};

		// Ctors/dtors/etc

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(State, CAIHumanStateSentryChallenge, kState_HumanSentryChallenge);

		CAIHumanStateSentryChallenge( );
		~CAIHumanStateSentryChallenge( );

		// Ctors/dtors/etc

		virtual LTBOOL Init(CAIHuman* pAIHuman);
		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Update

		void Update(void);
		void UpdateAnimation(void);

		// Handlers

		void	HandleModelString(ArgList* pArgList);

		// Simple acccessors

		CMusicMgr::Mood GetMusicMood(void) { return CMusicMgr::eMoodAggressive; }
		
		void SetObjectToSentryChallenge( HOBJECT hObject);
		void SetAnimProp(eActionType PropSet, EnumAnimPropGroup eGroup, EnumAnimProp eProp);
		void SetChallengeResult( const Challenge& refResult ) { m_Challenge = refResult; }

	protected:
		enum eHumanChallengeState
		{
			eChasing,
			eScanning,
			eReactingToScan,
			eDone,
			eStateComplete,
		};

		bool IsStateStillValid(void) const;

		void SetState(eHumanChallengeState eState);
		
		void HandleReactToScan();
		void HandleScanning();
		void HandleChasing();
		void HandleDone();

		void ActOnChallengeResult();

	private:


		// Working methods:
		void MoveCloser(void);
		void MaybePlaySound(void);
		void GenerateCommunicationInfo(void);

		// Query methods:
		const CCharacter* GetChallengeCharacter( ) const { return (CCharacter*)g_pLTServer->HandleToObject(m_hToChallenge); }
		bool ShouldMoveCloser(void);

		// Save:

		// Handle to the object to challenge
		LTObjRef	m_hToChallenge;


		// Prop set containing all of the props to be used for the challenge
		// animation.
		CAnimationProps		m_ChallengeProps;

		// Properies to be used if the challenger LIKES
		CAnimationProps		m_PassProps;	

		// Properies to be used if the challenger HATES
		CAnimationProps		m_FailProps;	

		CLastKnownPosition	m_LastKnownPosition;
		Challenge			m_Challenge;
		RelationDescription m_RD;
		uint32				m_iRecipientMask;

		eHumanChallengeState m_eState;

		// How close we need to be to be able to scan
		float			m_fCloseEnoughToScanSqr;

		// Dont Save:
};

#endif // __AIHUMANSTATESENTRYCHALLENGE_H__

