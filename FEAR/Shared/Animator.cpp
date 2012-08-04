// ----------------------------------------------------------------------- //
//
// MODULE  : Animator.cpp
//
// PURPOSE : Implementations of Animator classes
//
// CREATED : 08.20.1999
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Animator.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::CAnimator()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAnimator::CAnimator()
{
	m_cAnis = 1;
	m_cAniTrackers = 1;

	m_eAniTrackerDims = eAniTrackerInvalid;
	m_hObject = NULL;

	m_bInitialized = false;
	m_bDisabled = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::CAnimator()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAnimator::~CAnimator()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::Init()
//
//	PURPOSE:	Init
//
// ----------------------------------------------------------------------- //

void CAnimator::Init(HOBJECT hObject)
{
	LTASSERT(hObject, "Invalid object specified");

	m_hObject = hObject;

	m_aAniTrackers[eAniTrackerMain].m_AnimTracker = MAIN_TRACKER;

	m_eAniTrackerDims = eAniTrackerInvalid;

    m_bInitialized = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void CAnimator::Update()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::IsAniTrackerDone()
//
//	PURPOSE:	Checks to see if the Ani tracker playback state is MS_PLAYDONE
//
// ----------------------------------------------------------------------- //

bool CAnimator::IsAniTrackerDone(AniTracker eAniTracker) const
{
	LTASSERT(eAniTracker != eAniTrackerInvalid, "Invalid tracker specified");
    if ( eAniTrackerInvalid == eAniTracker ) return true;

    uint32 dwFlags;
    uint32 dwResult = g_pModelLT->GetPlaybackState(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, dwFlags);
	LTASSERT(LT_OK == dwResult, "Invalid playback state query result");

    return (dwFlags & MS_PLAYDONE) ? true : false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::AddAniTracker()
//
//	PURPOSE:	Generates a hokey AniTracker value given a name
//
// ----------------------------------------------------------------------- //

AniTracker CAnimator::AddAniTracker(const char* szWeightset)
{
	LTASSERT(szWeightset, "Invalid weight set specified");
	if (!szWeightset) return eAniTrackerInvalid;

	// Pick an arbitrary animation tracker ID (FIXTRACKER)
	m_aAniTrackers[m_cAniTrackers].m_AnimTracker = m_cAniTrackers;
    if ( LT_OK != g_pModelLT->AddTracker(m_hObject, m_aAniTrackers[m_cAniTrackers].m_AnimTracker, true) )
	{
        LTERROR( "Unable to add animation tracker");
		return eAniTrackerInvalid;
	}

	m_aAniTrackers[m_cAniTrackers].m_sWeightset = szWeightset;

	return (AniTracker)m_cAniTrackers++;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::EnableAniTracker()
//
//	PURPOSE:	Enables an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimator::EnableAniTracker(AniTracker eAniTracker)
{
	LTASSERT((eAniTrackerInvalid != eAniTracker) && (eAniTrackerMain != eAniTracker), "Invalid tracker specified");
	if ((eAniTrackerInvalid == eAniTracker) || (eAniTrackerMain == eAniTracker)) return;

	HMODELWEIGHTSET hWeightset;

	if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, m_aAniTrackers[eAniTracker].m_sWeightset.c_str(), hWeightset) )
	{
        LTERROR( "Unable to find weight set");
		return;
	}

    if ( LT_OK != g_pModelLT->SetWeightSet(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, hWeightset) )
	{
        LTERROR( "Unable to set weight set");
		return;
	}

    g_pModelLT->ResetAnim(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::DisableAniTracker()
//
//	PURPOSE:	Disables an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimator::DisableAniTracker(AniTracker eAniTracker)
{
	LTASSERT((eAniTrackerInvalid != eAniTracker) && (eAniTrackerMain != eAniTracker), "Invalid tracker specified");
	if ((eAniTrackerInvalid == eAniTracker) || (eAniTrackerMain == eAniTracker)) return;

	HMODELWEIGHTSET hWeightset;

	if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, (char*)"Null", hWeightset) )
	{
        LTERROR( "Unable to find weight set");
		return;
	}

    if ( LT_OK != g_pModelLT->SetWeightSet(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, hWeightset) )
	{
        LTERROR( "Unable to set weight set");
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::StartAniTracker()
//
//	PURPOSE:	Starts an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimator::StartAniTracker(AniTracker eAniTracker)
{
	LTASSERT(eAniTrackerInvalid != eAniTracker, "Invalid tracker specified");
	if (eAniTrackerInvalid == eAniTracker) return;

    if ( LT_OK != g_pModelLT->SetPlaying(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, true) )
	{
        LTERROR( "Unable to play animation");
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::StopAniTracker()
//
//	PURPOSE:	Stops an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimator::StopAniTracker(AniTracker eAniTracker)
{
	LTASSERT(eAniTrackerInvalid != eAniTracker, "Invalid tracker specified");
	if (eAniTrackerInvalid == eAniTracker) return;

    if ( LT_OK != g_pModelLT->SetPlaying(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, false) )
	{
        LTERROR( "Unable to stop animation");
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::StopAniTracker()
//
//	PURPOSE:	Stops an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimator::PositionAniTracker(AniTracker eAniTracker, float fPercent)
{
	LTASSERT(eAniTrackerInvalid != eAniTracker, "Invalid tracker specified");
	if (eAniTrackerInvalid == eAniTracker) return;

    ANIMTRACKERID pTracker = m_aAniTrackers[eAniTracker].m_AnimTracker;

    uint32 dwAnimLength;
	g_pModelLT->GetCurAnimLength(m_hObject, pTracker, dwAnimLength);

    uint32 dwAnimTime = (uint32)((float)dwAnimLength*fPercent);

	if ( LT_OK != g_pModelLT->SetCurAnimTime(m_hObject, pTracker, dwAnimTime) )
	{
        LTERROR( "Unable to change animation position");
		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::ResetAniTracker()
//
//	PURPOSE:	Resets an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimator::ResetAniTracker(AniTracker eAniTracker)
{
    g_pModelLT->ResetAnim(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::IsAniTrackerLooping()
//
//	PURPOSE:	Gets an ani tracker's looping status
//
// ----------------------------------------------------------------------- //

bool CAnimator::IsAniTrackerLooping(AniTracker eAniTracker) const
{
    return LT_YES == g_pModelLT->GetLooping(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::LoopAniTracker()
//
//	PURPOSE:	Sets an ani tracker's looping status
//
// ----------------------------------------------------------------------- //

void CAnimator::LoopAniTracker(AniTracker eAniTracker, bool bLoop)
{
    g_pModelLT->SetLooping(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, bLoop == true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::AddAni()
//
//	PURPOSE:	Generates a hokey Ani value given an ani name
//
// ----------------------------------------------------------------------- //

Ani CAnimator::AddAni(const char* szAni)
{
	// Look for override first. Override is "+" + szAni;

	char szOverrideAni[128];
	LTSNPrintF( szOverrideAni, LTARRAYSIZE( szOverrideAni ), "+%s", szAni);

    HMODELANIM hAni = g_pLTBase->GetAnimIndex(m_hObject, (char*)szOverrideAni);

	if ( hAni == INVALID_MODEL_ANIM )
	{
		// Override could not be found. Just use the normal ani.

        hAni = g_pLTBase->GetAnimIndex(m_hObject, (char*)szAni);

		if ( hAni == INVALID_MODEL_ANIM )
		{
			// We couldn't even find the normal ani.

			//LTERROR( "Normal animation not found");
			return eAniInvalid;
		}
		else
		{
			// We found the normal ani.

			m_ahAnis[m_cAnis] = hAni;
			return (Ani)m_cAnis++;
		}
	}
	else
	{
		// We found the override ani.

		m_ahAnis[m_cAnis] = hAni;
		return (Ani)m_cAnis++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::SetAni()
//
//	PURPOSE:	Starts playing an animation on a given ani tracker
//
// ----------------------------------------------------------------------- //

void CAnimator::SetAni(Ani eAni, AniTracker eAniTracker)
{
//	LTASSERT(eAni != eAniInvalid && eAniTracker != eAniTrackerInvalid, "Invalid tracker specified");
	if ( eAni == eAniInvalid || eAniTracker == eAniTrackerInvalid ) return;

	// Get the animation that's currently playing on the tracker

	HMODELANIM hCurAni = GetAni(eAniTracker);
	HMODELANIM hNewAni = m_ahAnis[eAni];

	if ( hCurAni != hNewAni )
	{
		// If they're different, play the new one

		if ( eAniTracker == eAniTrackerMain || eAniTracker == m_eAniTrackerDims )
		{
			if ( SetDims(hNewAni) )
			{
                LTRESULT dwResult = g_pModelLT->SetCurAnim(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, hNewAni, true);
				LTASSERT(LT_OK == dwResult, "Error when changing animations");
			}
		}
		else
		{
            LTRESULT dwResult = g_pModelLT->SetCurAnim(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, hNewAni, true);
			LTASSERT(LT_OK == dwResult, "Error when changing animations");
		}
	}
	else if ( hCurAni == hNewAni )
	{
		// If they're the same, reset it if it's done

		if ( IsAniTrackerDone(eAniTracker) )
		{
			ResetAniTracker(eAniTracker);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::GetAni()
//
//	PURPOSE:	Gets the HMODELANIM currently playing on a tracker
//
// ----------------------------------------------------------------------- //

HMODELANIM CAnimator::GetAni(AniTracker eAniTracker)
{
	HMODELANIM hAni;

    uint32 dwResult = g_pModelLT->GetCurAnim(m_hObject, m_aAniTrackers[eAniTracker].m_AnimTracker, hAni);
	if ( dwResult == LT_OK )
	{
		return hAni;
	}
	else
	{
		return INVALID_MODEL_ANIM;
	}
}
