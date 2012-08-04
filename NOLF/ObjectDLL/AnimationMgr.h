// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATIONMGR_H__
#define __ANIMATIONMGR_H__

#include "AnimationParser.h"

// CAnimationProp

class CAnimationProp
{
	public :

		// Ctors/Dtors/etc

		CAnimationProp()
		{
			m_iIndex = 0;
			m_nValue = 0;
		}

		CAnimationProp(uint32 iIndex, uint32 nValue)
		{
			m_iIndex = iIndex;
			m_nValue = nValue;
		}

		// Save/load

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Comparison

		LTBOOL operator==(const CAnimationProp& Prop) const
		{
			return ((m_iIndex == Prop.m_iIndex) && ((m_nValue == -1) || (Prop.m_nValue == -1) || (Prop.m_nValue == m_nValue)));
		}

		LTBOOL Contains(const CAnimationProp& Prop) const
		{
			return (Prop.m_nValue == 0 || (*this == Prop));
		}

		LTBOOL Disjoint(const CAnimationProp& Prop) const
		{
			return (Prop.m_nValue == 0 || !(*this == Prop));
		}

		// Clear

		void Clear() { m_nValue = 0; }

	protected :

		friend class CAnimationProp;
		friend class CAnimationProps;
		friend class CAnimationMgr;

		uint32	m_iIndex;
		int32	m_nValue;
};

// CAnimationProps

class CAnimationProps
{
	public :

		// Ctors/Dtors/etc

		CAnimationProps();

		// Save/load

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);

		// Comparison

		LTBOOL operator==(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kMaxProps ; iProp++ )
			{
				if ( !(m_aProps[iProp] == Props.m_aProps[iProp]) )
				{
					return LTFALSE;
				}
			}

			return LTTRUE;
		}

		LTBOOL Contains(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kMaxProps ; iProp++ )
			{
				if ( !m_aProps[iProp].Contains(Props.m_aProps[iProp]) )
				{
					return LTFALSE;
				}
			}

			return LTTRUE;
		}

		LTBOOL Disjoint(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kMaxProps ; iProp++ )
			{
				if ( !m_aProps[iProp].Disjoint(Props.m_aProps[iProp]) )
				{
					return LTFALSE;
				}
			}

			return LTTRUE;
		}

		// Simple accessors

		void Clear();
		void Set(const CAnimationProp& Prop);
		LTBOOL IsSet(const CAnimationProp& Prop) const;

		// Debugging

		void GetString(CAnimationMgr* pAnimationMgr, char* szBuffer, uint nBufferLength);

	protected :

		enum Constants
		{
			kMaxProps = 10,
		};

	protected :

		CAnimationProp		m_aProps[kMaxProps];
};

// CAnimationInstance

class CAnimationInstance
{
	public :

		uint32 GetIndex() const { return m_iIndex; }
		HMODELANIM GetAni() const { return m_hAni; }

		LTBOOL IsPitched() const { return m_bPitched; }
		HMODELANIM GetPitchDownAni() const { return m_hAniPitchDown; }
		HMODELANIM GetPitchUpAni() const { return m_hAniPitchUp; }

	protected :

		friend class CAnimationMgr;

		uint32				m_iIndex;
		HMODELANIM			m_hAni;

		LTBOOL				m_bPitched;
		HMODELANIM			m_hAniPitchUp;
		HMODELANIM			m_hAniPitchDown;
};

// CTransitionInstance

class CTransitionInstance
{
	public :

		uint32 GetIndex() const { return m_iIndex; }
		HMODELANIM GetAni() const { return m_hAni; }

	protected :

		friend class CAnimationMgr;

		uint32				m_iIndex;
		HMODELANIM			m_hAni;
};

// CAnimationContext

class CAnimationContext
{
	public :

		// Ctors/Dtors/etc

		void Init(uint32 cAnimationInstances, uint32 cTransitionInstances);
		void Term();

#ifndef _CLIENTBUILD
		// Save/load

		void Save(HMESSAGEWRITE hWrite);
		void Load(HMESSAGEREAD hRead);
#endif _CLIENTBUILD

		// Updates

		void Update();

		// Props

		void Lock();
		void Unlock();
		LTBOOL IsLocked() { return m_bLocked; }

		void ClearProps() { m_Props.Clear(); }
		void SetProp(const CAnimationProp& Prop) { m_Props.Set(Prop); }
		LTBOOL IsPropSet(const CAnimationProp& Prop) const;

		void GetPropsString(char* szBuffer, uint nBufferLength);

		// Pitching

		void SetPitch(LTFLOAT fPitchTarget) { m_fPitchTarget = fPitchTarget; }

		void EnablePitch(const CAnimationInstance& ani);
		void DisablePitch();

		// Special

		void SetSpecial(const char* szName);
		void PlaySpecial();
		void LoopSpecial();
		void LingerSpecial();
		void StopSpecial();
		LTBOOL IsSpecialDone();

	protected :

		enum State
		{
			eStateLock,
			eStatePostLock,
			eStateTransition,
			eStateNormal,
			eStateStartSpecial,
			eStateStartSpecialLoop,
			eStateStartSpecialLinger,
			eStateStopSpecial,
			eStateSpecial,
			eStateSpecialLoop,
			eStateSpecialLinger,
		};

		enum Constants
		{
			kNumPitchWeightsets = 101,
		};

	protected :

		friend class CAnimationMgr;

		HOBJECT					m_hObject;

		HMODELANIM				m_haniSpecial;

		LTBOOL					m_bLocked;

		CAnimationMgr*			m_pAnimationMgr;

		CAnimationProps			m_Props;
		State					m_eState;

		uint32					m_cAnimationInstances;
		CAnimationInstance*		m_aAnimationInstances;
		uint32					m_iAnimation;
		uint32					m_iAnimationFrom;

		uint32					m_cTransitionInstances;
		CTransitionInstance*	m_aTransitionInstances;
		uint32					m_iTransition;

		LTBOOL					m_bHackToAvoidTheUsualOneFrameOffBullshit;
		LTFLOAT					m_fPitchTarget;
		LTFLOAT					m_fPitch;
		HMODELWEIGHTSET			m_ahPitchWeightsets[kNumPitchWeightsets];
		LTAnimTracker			m_trkPitchDown;
		LTAnimTracker			m_trkPitchUp;
};

// CAnimation

class CAnimation
{
	public :

		// Simple accessors

		uint32 GetIndex() const { return m_iIndex; }
		const CAnimationProps& GetProps() const { return m_Props; }
		const char* GetName() const { return m_szName; }

	protected :

		friend class CAnimationMgr;

		char				m_szName[32];
		uint32				m_iIndex;
		CAnimationProps		m_Props;
};

// CTransition

class CTransition
{
	public :

		// Simple accessors

		uint32 GetIndex() const { return m_iIndex; }
		const char* GetName() const { return m_szName; }

	protected :

		friend class CAnimationMgr;

		char				m_szName[32];
		uint32				m_iIndex;

		CAnimationProps		m_PropsInitial;
		CAnimationProps		m_PropsAdd;
		CAnimationProps		m_PropsRemove;
		CAnimationProps		m_PropsConstant;
		CAnimationProps		m_PropsNot;
};

// CAnimationMgr

class CAnimationMgr
{
	public :

		// Ctors/Dtors/etc

		CAnimationMgr();
		~CAnimationMgr() { Term(); }

		LTBOOL Init(const char* szFilename);
		void Term();

		LTBOOL IsInitialized() const { return m_bInitialized; }

		// Contexts

		CAnimationContext* CreateAnimationContext(HOBJECT hObject);
		void DestroyAnimationContext(CAnimationContext* pAnimationContex);

		// Props

		const CAnimationProp& FindAnimationProp(const char* szName) const;
		const char* GetPropName(const CAnimationProp& Prop) const;

		// Animations

		const CAnimation& FindAnimation(const CAnimationProps& Props) const;
		const CAnimation& GetAnimation(uint32 iAnimation) const { return m_aAnimations[iAnimation]; }

		// Transitions

		const CTransition& FindTransition(const CAnimationProps& PropsFrom, const CAnimationProps& PropsTo) const;
		const CTransition& GetTransition(uint32 iTransition) const { return m_aTransitions[iTransition]; }

	protected :

		// Ctors/Dtors/etc

		LTBOOL Init(Animation::CAnimationParser& AnimationParser);

		// Animations

		HMODELANIM GetAnimationInstance(HOBJECT hObject, const char *szName);

	protected :

		enum Constants
		{
			kMaxProps = 128,
			kMaxAnimations = 512,
			kMaxTransitions = 512,
			kMaxPropsPerTransition = 32,
			kMaxPropsPerAnimation = 32,
		};

	protected :

		LTBOOL				m_bInitialized;

		char				m_aszAnimationPropNames[kMaxProps][32];
		CAnimationProp		m_aAnimationProps[kMaxProps];
		uint32				m_cAnimationProps;

		CAnimation*			m_aAnimations;
		uint32				m_cAnimations;

		CTransition*		m_aTransitions;
		uint32				m_cTransitions;
};

#endif