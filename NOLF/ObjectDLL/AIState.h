// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_STATE_H__
#define __AI_STATE_H__

#include "MusicMgr.h"

class CAIState
{
	DEFINE_ABSTRACT_FACTORY_METHODS(CAIState);

	public : // Public methods

		// Ctors/Dtors/etc

		LTBOOL Init(CAI* pAI);

		virtual void Load(HMESSAGEREAD hRead);
		virtual void Save(HMESSAGEWRITE hWrite);

		// Methods

		virtual void PreUpdate();
		virtual void Update();
		virtual void PostUpdate();
		virtual void UpdateAnimation() {}
		virtual void UpdateSenses();
		virtual void UpdateMusic();

		// Handlers

		virtual void HandleNameValuePair(char *szName, char *szValue);
		virtual void HandleDamage(const DamageStruct& damage) {}
		virtual void HandleBrokenLink(HOBJECT hObject) {}
		virtual void HandleModelString(ArgList* pArgList) {}
		virtual void HandleTouch(HOBJECT hObject) {}
		virtual LTBOOL HandleCommand(char** pTokens, int nArgs) { return LTFALSE; }
		virtual void HandleSense(CAISense* pAISense) {}

		// Misc

		virtual LTBOOL CanActivate() { return LTTRUE; }
		virtual LTBOOL CanReturn() { return LTFALSE; }
        virtual HSTRING CreateReturnString() { return LTNULL; }
		void ReturnOr(const char* szState);

		LTBOOL HasNext() { return m_cNexts > 0; }
		void NextOr(const char* szState);

		virtual HMODELANIM GetDeathAni(LTBOOL bFront) { return INVALID_ANI; }
		virtual BodyState GetBodyState() { return eBodyStateNormal; }

		virtual CMusicMgr::Mood GetMusicMood() { return CMusicMgr::eMoodNone; }

		virtual LTBOOL DelayChangeState() { return LTFALSE; }
		virtual LTBOOL RejectChangeState() { return LTFALSE; }

		virtual LTBOOL CanBeDamagedAsAttachment() { return LTTRUE; }

		virtual LTBOOL CanAnimate() { return LTTRUE; }

		inline LTBOOL IsAlert() const { return m_bAlert; }

		virtual const char* GetName() { return "AIState"; }

	protected : // Protected methods

		// Simple accessors

		LTFLOAT GetElapsedTime() { return m_fElapsedTime; }

	protected : // Protected constants

		enum Constants
		{
			kMaxNexts	= 8,
		};

	protected : // Private member variables

		CAI*	m_pAI;						// Backpointer to our AI
		LTBOOL	m_bAlert;					// Is this an alert state?
		LTBOOL  m_bNoCinematics;			// Do we not do cinematics in this state?
		LTFLOAT	m_fElapsedTime;				// How long have we been in this state?
		LTBOOL	m_bFirstUpdate;				// Is this our first update?
		LTBOOL	m_bPlayFirstSound;			// Should we play our first sound?
		HSTRING	m_hstrReturn;				// A state we can return to if this fails
		int		m_cNexts;					// How many nexts we have specified
		HSTRING	m_ahstrNexts[kMaxNexts];	// Commands to issue after we're done
		HSTRING	m_hstrFirst;				// Message we send on first update
};

#endif