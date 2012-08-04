// ----------------------------------------------------------------------- //
//
// MODULE  : Animator.cpp
//
// PURPOSE : Implementations of Animator classes
//
// CREATED : 08.20.1999
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
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

	m_eAniTrackerPitchDown = eAniTrackerInvalid;
	m_eAniTrackerPitchUp = eAniTrackerInvalid;
	m_eAniTrackerPitch = eAniTrackerInvalid;

    m_bInitialized = LTFALSE;

	m_bHackToAvoidTheUsualOneFrameOffBullshit = LTFALSE;

	m_fPitchTarget = 0.5f;
	m_fPitch = 0.5f;
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

void CAnimator::Init(ILTCSBase *pInterface, HOBJECT hObject)
{
	_ASSERT(hObject);

	m_pInterface = pInterface;

	m_hObject = hObject;

    LTRESULT dwResult = g_pModelLT->GetMainTracker(m_hObject, m_aAniTrackers[eAniTrackerMain].m_pAnimTracker);
	_ASSERT(LT_OK == dwResult);

	m_eAniTrackerDims = eAniTrackerInvalid;

    m_bInitialized = LTTRUE;
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

LTBOOL CAnimator::IsAniTrackerDone(AniTracker eAniTracker) const
{
	_ASSERT(eAniTracker != eAniTrackerInvalid);
    if ( eAniTrackerInvalid == eAniTracker ) return LTTRUE;

    uint32 dwFlags;
    uint32 dwResult = g_pModelLT->GetPlaybackState(m_aAniTrackers[eAniTracker].m_pAnimTracker, dwFlags);
	_ASSERT(LT_OK == dwResult);

    return (dwFlags & MS_PLAYDONE) ? LTTRUE : LTFALSE;
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
	_ASSERT(szWeightset);
	if (!szWeightset) return eAniTrackerInvalid;

    if ( LT_OK != g_pModelLT->AddTracker(m_hObject, &m_aAniTrackers[m_cAniTrackers].m_AnimTracker) )
	{
        _ASSERT(LTFALSE);
		return eAniTrackerInvalid;
	}

	m_aAniTrackers[m_cAniTrackers].m_szWeightset = szWeightset;

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
	_ASSERT((eAniTrackerInvalid != eAniTracker) && (eAniTrackerMain != eAniTracker));
	if ((eAniTrackerInvalid == eAniTracker) || (eAniTrackerMain == eAniTracker)) return;

	HMODELWEIGHTSET hWeightset;

	if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, (char*)m_aAniTrackers[eAniTracker].m_szWeightset, hWeightset) )
	{
        _ASSERT(LTFALSE);
		return;
	}

    if ( LT_OK != g_pModelLT->SetWeightSet(m_aAniTrackers[eAniTracker].m_pAnimTracker, hWeightset) )
	{
        _ASSERT(LTFALSE);
		return;
	}

    g_pModelLT->ResetAnim(m_aAniTrackers[eAniTracker].m_pAnimTracker);
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
	_ASSERT((eAniTrackerInvalid != eAniTracker) && (eAniTrackerMain != eAniTracker));
	if ((eAniTrackerInvalid == eAniTracker) || (eAniTrackerMain == eAniTracker)) return;

	HMODELWEIGHTSET hWeightset;

	if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, (char*)"Null", hWeightset) )
	{
        _ASSERT(LTFALSE);
		return;
	}

    if ( LT_OK != g_pModelLT->SetWeightSet(m_aAniTrackers[eAniTracker].m_pAnimTracker, hWeightset) )
	{
        _ASSERT(LTFALSE);
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
	_ASSERT(eAniTrackerInvalid != eAniTracker);
	if (eAniTrackerInvalid == eAniTracker) return;

    if ( LT_OK != g_pModelLT->SetPlaying(m_aAniTrackers[eAniTracker].m_pAnimTracker, LTTRUE) )
	{
        _ASSERT(LTFALSE);
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
	_ASSERT(eAniTrackerInvalid != eAniTracker);
	if (eAniTrackerInvalid == eAniTracker) return;

    if ( LT_OK != g_pModelLT->SetPlaying(m_aAniTrackers[eAniTracker].m_pAnimTracker, LTFALSE) )
	{
        _ASSERT(LTFALSE);
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

void CAnimator::PositionAniTracker(AniTracker eAniTracker, LTFLOAT fPercent)
{
	_ASSERT(eAniTrackerInvalid != eAniTracker);
	if (eAniTrackerInvalid == eAniTracker) return;

    LTAnimTracker* pTracker = m_aAniTrackers[eAniTracker].m_pAnimTracker;

    uint32 dwAnimLength;
	g_pModelLT->GetCurAnimLength(pTracker, dwAnimLength);

    uint32 dwAnimTime = (uint32)((LTFLOAT)dwAnimLength*fPercent);

	if ( LT_OK != g_pModelLT->SetCurAnimTime(pTracker, dwAnimTime) )
	{
        _ASSERT(LTFALSE);
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
    g_pModelLT->ResetAnim(m_aAniTrackers[eAniTracker].m_pAnimTracker);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::IsAniTrackerLooping()
//
//	PURPOSE:	Gets an ani tracker's looping status
//
// ----------------------------------------------------------------------- //

LTBOOL CAnimator::IsAniTrackerLooping(AniTracker eAniTracker) const
{
    return LT_YES == g_pModelLT->GetLooping(m_aAniTrackers[eAniTracker].m_pAnimTracker);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimator::LoopAniTracker()
//
//	PURPOSE:	Sets an ani tracker's looping status
//
// ----------------------------------------------------------------------- //

void CAnimator::LoopAniTracker(AniTracker eAniTracker, LTBOOL bLoop)
{
    g_pModelLT->SetLooping(m_aAniTrackers[eAniTracker].m_pAnimTracker, bLoop);
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
	sprintf(szOverrideAni, "+%s", szAni);

    HMODELANIM hAni = m_pInterface->GetAnimIndex(m_hObject, (char*)szOverrideAni);

	if ( hAni == INVALID_MODEL_ANIM )
	{
		// Override could not be found. Just use the normal ani.

        hAni = m_pInterface->GetAnimIndex(m_hObject, (char*)szAni);

		if ( hAni == INVALID_MODEL_ANIM )
		{
			// We couldn't even find the normal ani.

//          _ASSERT(LTFALSE);
			return eAniInvalid;
		}
		else
		{
			// We found the normal ani.

			m_ahAnis[m_cAnis][0] = hAni;

			char szPitch[256];

			sprintf(szPitch, "%s*U", szAni);
			m_ahAnis[m_cAnis][1] = m_pInterface->GetAnimIndex(m_hObject, (char*)szPitch);
			
			sprintf(szPitch, "%s*D", szAni);
			m_ahAnis[m_cAnis][2] = m_pInterface->GetAnimIndex(m_hObject, (char*)szPitch);

			return (Ani)m_cAnis++;
		}
	}
	else
	{
		// We found the override ani.

		m_ahAnis[m_cAnis][0] = hAni;

		char szPitch[256];

		sprintf(szPitch, "%s*U", szAni);
		m_ahAnis[m_cAnis][1] = m_pInterface->GetAnimIndex(m_hObject, (char*)szPitch);
		
		sprintf(szPitch, "%s*D", szAni);
		m_ahAnis[m_cAnis][2] = m_pInterface->GetAnimIndex(m_hObject, (char*)szPitch);

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
//	_ASSERT(eAni != eAniInvalid && eAniTracker != eAniTrackerInvalid);
	if ( eAni == eAniInvalid || eAniTracker == eAniTrackerInvalid ) return;

	// Get the animation that's currently playing on the tracker

	HMODELANIM hCurAni = GetAni(eAniTracker);
	HMODELANIM hNewAni = m_ahAnis[eAni][0];

	if ( m_eAniTrackerPitch != eAniTrackerInvalid && m_eAniTrackerPitch == eAniTracker)
	{
		LTBOOL bPitch = m_eAniTrackerPitchDown != eAniTrackerInvalid &&
						m_eAniTrackerPitchUp != eAniTrackerInvalid &&
						(m_ahAnis[eAni][1] != INVALID_MODEL_ANIM) && 
						(m_ahAnis[eAni][2] != INVALID_MODEL_ANIM);

		if ( bPitch )
		{
			EnablePitch(eAni);
		}
		else
		{
			DisablePitch();
		}
	}

	if ( hCurAni != hNewAni )
	{
		// If they're different, play the new one

		if ( eAniTracker == eAniTrackerMain || eAniTracker == m_eAniTrackerDims )
		{
			if ( SetDims(hNewAni) )
			{
                LTRESULT dwResult = g_pModelLT->SetCurAnim(m_aAniTrackers[eAniTracker].m_pAnimTracker, hNewAni);
				_ASSERT(LT_OK == dwResult);
			}
		}
		else
		{
            LTRESULT dwResult = g_pModelLT->SetCurAnim(m_aAniTrackers[eAniTracker].m_pAnimTracker, hNewAni);
			_ASSERT(LT_OK == dwResult);
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

    uint32 dwResult = g_pModelLT->GetCurAnim(m_aAniTrackers[eAniTracker].m_pAnimTracker, hAni);
	if ( dwResult == LT_OK )
	{
		return hAni;
	}
	else
	{
		return INVALID_MODEL_ANIM;
	}
}

// ----------------------------------------------------------------------- //

void CAnimator::EnablePitch(Ani eAni)
{
	if ( m_fPitch == m_fPitchTarget ) return;

	m_fPitch = m_fPitchTarget;

	if ( m_fPitch > .5f )
	{
		g_pModelLT->SetCurAnim(m_aAniTrackers[m_eAniTrackerPitchDown].m_pAnimTracker, m_ahAnis[eAni][1]);
		g_pModelLT->SetWeightSet(m_aAniTrackers[m_eAniTrackerPitchDown].m_pAnimTracker, m_ahPitchWeightsets[0]);

		g_pModelLT->SetCurAnim(m_aAniTrackers[m_eAniTrackerPitchUp].m_pAnimTracker, m_ahAnis[eAni][1]);

//		if ( m_bHackToAvoidTheUsualOneFrameOffBullshit )
		{
			g_pModelLT->SetWeightSet(m_aAniTrackers[m_eAniTrackerPitchUp].m_pAnimTracker, m_ahPitchWeightsets[(int32)((2.0f*(m_fPitch-.5f))*(kNumPitchWeightsets-1))]);
		}

	}
	else
	{
		g_pModelLT->SetCurAnim(m_aAniTrackers[m_eAniTrackerPitchUp].m_pAnimTracker, m_ahAnis[eAni][2]);
		g_pModelLT->SetWeightSet(m_aAniTrackers[m_eAniTrackerPitchUp].m_pAnimTracker, m_ahPitchWeightsets[0]);

		g_pModelLT->SetCurAnim(m_aAniTrackers[m_eAniTrackerPitchDown].m_pAnimTracker, m_ahAnis[eAni][2]);

//		if ( m_bHackToAvoidTheUsualOneFrameOffBullshit )
		{
			g_pModelLT->SetWeightSet(m_aAniTrackers[m_eAniTrackerPitchDown].m_pAnimTracker, m_ahPitchWeightsets[(int32)((2.0f*(.5f-m_fPitch))*(kNumPitchWeightsets-1))]);
		}
	}

//	m_bHackToAvoidTheUsualOneFrameOffBullshit = LTTRUE;
}

// ----------------------------------------------------------------------- //

void CAnimator::DisablePitch()
{
	m_fPitch = -100.0f;
	m_bHackToAvoidTheUsualOneFrameOffBullshit = LTFALSE;

	g_pModelLT->SetCurAnim(m_aAniTrackers[m_eAniTrackerPitchDown].m_pAnimTracker, m_pInterface->GetAnimIndex(m_hObject, "base"));
	g_pModelLT->SetWeightSet(m_aAniTrackers[m_eAniTrackerPitchDown].m_pAnimTracker, m_ahPitchWeightsets[0]);

	g_pModelLT->SetCurAnim(m_aAniTrackers[m_eAniTrackerPitchUp].m_pAnimTracker, m_pInterface->GetAnimIndex(m_hObject, "base"));
	g_pModelLT->SetWeightSet(m_aAniTrackers[m_eAniTrackerPitchUp].m_pAnimTracker, m_ahPitchWeightsets[0]);
}
