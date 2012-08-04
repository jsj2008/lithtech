// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__

#include "CommonUtilities.h"
#include "iltmodel.h"

// Enums

enum AniTracker
{
	eAniTrackerInvalid	= -1,
	eAniTrackerMain		= 0,
};

enum Ani
{
	eAniInvalid			= -1,
};

// Structs

struct ANIMTRACKER
{
	ANIMTRACKER()
	{
        m_pAnimTracker  = &m_AnimTracker;
		m_szWeightset	= "";
	}

    LTAnimTracker*    m_pAnimTracker;
    LTAnimTracker     m_AnimTracker;
	const char*		m_szWeightset;
};

// Classes

class CAnimator
{
	public :

		enum Constants
		{
			kMaxAnis		= 256,
			kMaxAniTrackers	= 5,
			kNumPitchWeightsets = 101,
		};

	public :

		// Methods

		virtual void Init(ILTCSBase *pInterface, HOBJECT hObject);

		// Updates

		virtual void Update();

		// Simple accessors

        inline LTBOOL        IsInitialized() const { return m_bInitialized; }
		inline AniTracker	GetDimsTracker() const { return m_eAniTrackerDims; }
		inline int			GetNumTrackers() const { return m_cAniTrackers; }

		// Pitch

		void SetPitch(LTFLOAT fPitch) { m_fPitchTarget = fPitch; }

	protected :

		// Ctors/Dtors/etc

		CAnimator();
		virtual ~CAnimator();

		// AniTracker methods

		AniTracker AddAniTracker(const char* szWeightset);
		virtual void ResetAniTracker(AniTracker eAniTracker);
		void EnableAniTracker(AniTracker eAniTracker);
		void DisableAniTracker(AniTracker eAniTracker);
		void StartAniTracker(AniTracker eAniTracker);
		void StopAniTracker(AniTracker eAniTracker);
        LTBOOL IsAniTrackerDone(AniTracker eAniTracker) const;
        LTBOOL IsAniTrackerLooping(AniTracker eAniTracker) const;
        void LoopAniTracker(AniTracker eAniTracker, LTBOOL bLoop);
        void PositionAniTracker(AniTracker eAniTracker, LTFLOAT fPercent);

		void EnablePitch(Ani eAni);
		void DisablePitch();

		// Ani methods

        Ani AddAni(const char* szAni);
        void SetAni(Ani eAni, AniTracker eAniTracker);
		HMODELANIM GetAni(AniTracker eAniTracker);

		// Misc

        virtual LTBOOL SetDims(HMODELANIM hAni) { return LTTRUE; }

	protected :

		HOBJECT			m_hObject;								// The model we're animating

		HMODELANIM		m_ahAnis[kMaxAnis][3];					// Ani -> HMODELANIM mapping
		int				m_cAnis;								// Number of Anis for this Animator

		ANIMTRACKER		m_aAniTrackers[kMaxAniTrackers];		// The array of animtrackers we'll use
		int				m_cAniTrackers;							// Number of Ani trackers on this Animator

		AniTracker		m_eAniTrackerDims;						// The ani tracker that determines our dims (aside from main)

		AniTracker		m_eAniTrackerPitchDown;
		AniTracker		m_eAniTrackerPitchUp;
		AniTracker		m_eAniTrackerPitch;

		ILTCSBase*		m_pInterface;

		LTBOOL			m_bHackToAvoidTheUsualOneFrameOffBullshit;
		LTFLOAT			m_fPitchTarget;
		LTFLOAT			m_fPitch;
		HMODELWEIGHTSET	m_ahPitchWeightsets[kNumPitchWeightsets];

        LTBOOL           m_bInitialized;                         // Initialized?
};

 #endif // __ANIMATOR_H__