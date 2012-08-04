// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__

#include "CommonUtilities.h"
#include "iltmodel.h"
#include "ltobjref.h"

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
	:	m_AnimTracker	( MAIN_TRACKER ),
		m_sWeightset	( )
	{
	}

	ANIMTRACKERID	m_AnimTracker;
	std::string		m_sWeightset;
};

// Classes

class CAnimator
{
	public :

		enum Constants
		{
			kMaxAnis		= 256,
			kMaxAniTrackers	= 5,
		};

	public :

		// Methods

		virtual void	Init(HOBJECT hObject);

		// Updates

		virtual void	Update();

		// Simple accessors

		inline bool			IsInitialized() const { return m_bInitialized; }
		inline AniTracker	GetDimsTracker() const { return m_eAniTrackerDims; }
		inline int			GetNumTrackers() const { return m_cAniTrackers; }

		virtual void		Disable()
		{
			if( m_bDisabled == true )
				return;

			m_bDisabled = true;
			m_cAniTrackers = 1;
			m_cAnis = 1;
			m_eAniTrackerDims = eAniTrackerInvalid;
			m_hObject = NULL;
			m_bInitialized = false;
		}

		virtual void		Enable( HOBJECT hObject )
		{
			if( m_bDisabled == false )
				return;

			m_bDisabled = false;
		}

		bool IsDisabled() const { return m_bDisabled; }

		
	protected :

		// Ctors/Dtors/etc

		CAnimator();
		virtual ~CAnimator();

		// AniTracker methods

		AniTracker		AddAniTracker(const char* szWeightset);
		virtual void	ResetAniTracker(AniTracker eAniTracker);
		void			EnableAniTracker(AniTracker eAniTracker);
		void			DisableAniTracker(AniTracker eAniTracker);
		void			StartAniTracker(AniTracker eAniTracker);
		void			StopAniTracker(AniTracker eAniTracker);
		bool			IsAniTrackerDone(AniTracker eAniTracker) const;
		bool			IsAniTrackerLooping(AniTracker eAniTracker) const;
		void			LoopAniTracker(AniTracker eAniTracker, bool bLoop);
		void			PositionAniTracker(AniTracker eAniTracker, float fPercent);

		// Ani methods

		Ani				AddAni(const char* szAni);
		void			SetAni(Ani eAni, AniTracker eAniTracker);
		HMODELANIM		GetAni(AniTracker eAniTracker);

		// Misc

		virtual bool	SetDims(HMODELANIM hAni) { LTUNREFERENCED_PARAMETER( hAni ); return true; }

	protected :

		bool m_bDisabled;

		LTObjRef		m_hObject;								// The model we're animating

		HMODELANIM		m_ahAnis[kMaxAnis];						// Ani -> HMODELANIM mapping
		int				m_cAnis;								// Number of Anis for this Animator

		ANIMTRACKER		m_aAniTrackers[kMaxAniTrackers];		// The array of animtrackers we'll use
		int				m_cAniTrackers;							// Number of Ani trackers on this Animator

		AniTracker		m_eAniTrackerDims;						// The ani tracker that determines our dims (aside from main)

		bool			m_bInitialized;							// Initialized?
};

#endif // __ANIMATOR_H__
