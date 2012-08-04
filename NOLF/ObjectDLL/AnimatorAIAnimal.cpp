// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "SFXMsgIds.h"
#include "AnimatorAIAnimal.h"
#include "Character.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIAnimal::CAnimatorAIAnimal()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAnimatorAIAnimal::CAnimatorAIAnimal()
{
	m_eMain	= eInvalid;

	m_eLastMain	= eInvalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIAnimal::~CAnimatorAIAnimal()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAnimatorAIAnimal::~CAnimatorAIAnimal()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIAnimal::Init()
//
//	PURPOSE:	Initialize the AIAnimal animator
//
// ----------------------------------------------------------------------- //

void CAnimatorAIAnimal::Init(ILTCSBase *pInterface, HOBJECT hObject)
{
	CAnimator::Init(pInterface, hObject);

	// Setup all ani trackers

#define _A(x) AddAni((x))
#define _N eAniInvalid

	// Get Main anis

    m_AniBase                   = AniAIAnimalMain(LTTRUE,    _A("Base"));
    m_AniIdle                   = AniAIAnimalMain(LTTRUE,    _A("Idle"));
    m_AniWalking                = AniAIAnimalMain(LTTRUE,    _A("Walking"));
    m_AniRunning                = AniAIAnimalMain(LTTRUE,    _A("Running"));
    m_AniBite                   = AniAIAnimalMain(LTFALSE,   _A("Attack"));
    m_AniIdleStand              = AniAIAnimalMain(LTTRUE,    _A("Idle"));
    m_AniIdleLay                = AniAIAnimalMain(LTTRUE,    _A("LayDown"));
    m_AniIdleSit                = AniAIAnimalMain(LTTRUE,    _A("Sit"));
    m_AniFenceJump              = AniAIAnimalMain(LTFALSE,   _A("FenceJump"));
    m_AniFenceBark              = AniAIAnimalMain(LTTRUE,    _A("FenceBark"));
    m_AniFenceUnjump            = AniAIAnimalMain(LTFALSE,   _A("FenceUnjump"));
    m_AniSniffPoodle            = AniAIAnimalMain(LTTRUE,    _A("SniffPoodle"));
    m_AniSwimming               = AniAIAnimalMain(LTTRUE,    _A("Swimming"));
    m_AniMaul                   = AniAIAnimalMain(LTTRUE,    _A("Maul"));

	// Setup our lookup tables

    memset(m_apAniAIAnimalMains, LTNULL, sizeof(void*)*kNumMains);

	m_apAniAIAnimalMains[eIdle]				= &m_AniIdle;
	m_apAniAIAnimalMains[eWalking]			= &m_AniWalking;
	m_apAniAIAnimalMains[eRunning]			= &m_AniRunning;
	m_apAniAIAnimalMains[eBite]				= &m_AniBite;
	m_apAniAIAnimalMains[eIdleStand]		= &m_AniIdleStand;
	m_apAniAIAnimalMains[eIdleSit]			= &m_AniIdleSit;
	m_apAniAIAnimalMains[eIdleLay]			= &m_AniIdleLay;
	m_apAniAIAnimalMains[eFenceJump]		= &m_AniFenceJump;
	m_apAniAIAnimalMains[eFenceBark]		= &m_AniFenceBark;
	m_apAniAIAnimalMains[eFenceUnjump]		= &m_AniFenceUnjump;
	m_apAniAIAnimalMains[eSniffPoodle]		= &m_AniSniffPoodle;
	m_apAniAIAnimalMains[eSwimming]			= &m_AniSwimming;
	m_apAniAIAnimalMains[eMaul]				= &m_AniMaul;

	// The ani tracker that sets our dims is the main

	m_eAniTrackerDims = eAniTrackerMain;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIAnimal::Update()
//
//	PURPOSE:	Updates the AIAnimal animator
//
// ----------------------------------------------------------------------- //

void CAnimatorAIAnimal::Update()
{
	CAnimator::Update();

	if ( m_eMain != eInvalid )
	{
		AniAIAnimalMain* pAniAIAnimalMain = m_apAniAIAnimalMains[m_eMain];

		if ( !pAniAIAnimalMain )
		{
            g_pLTServer->CPrint("missing ani %d", m_eMain);
			return;
		}

		if ( pAniAIAnimalMain->bLoops && !IsAniTrackerLooping(eAniTrackerMain) )
		{
            LoopAniTracker(eAniTrackerMain, LTTRUE);
		}
		else if ( !pAniAIAnimalMain->bLoops && IsAniTrackerLooping(eAniTrackerMain) )
		{
            LoopAniTracker(eAniTrackerMain, LTFALSE);
		}

		SetAni(pAniAIAnimalMain->eAni, eAniTrackerMain);
	}
	else
	{
		_ASSERT(!"CAnimatorAIAnimal::Update - Main ani not set!");
	}

	// Record the state etc

	m_eLastMain = m_eMain;

	// Reset all vars

	m_eMain	= eInvalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIAnimal::ResetAniTracker()
//
//	PURPOSE:	Resets an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimatorAIAnimal::ResetAniTracker(AniTracker eAniTracker)
{
	CAnimator::ResetAniTracker(eAniTracker);

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_CHARACTER_ID);
    g_pLTServer->WriteToMessageObject(hMessage, m_hObject);
	if ( eAniTracker == eAniTrackerMain )
	{
        g_pLTServer->WriteToMessageByte(hMessage, CFX_RESET_TRACKER);
        g_pLTServer->WriteToMessageByte(hMessage, 0);
	}
	else
	{
        _ASSERT(LTFALSE);
	}
    g_pLTServer->EndMessage(hMessage);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIAnimal::SetDims()
//
//	PURPOSE:	Sets the AIAnimal's dims based on the ani
//
// ----------------------------------------------------------------------- //

LTBOOL CAnimatorAIAnimal::SetDims(HMODELANIM hAni)
{
    LTVector vDims;
    LTRESULT dResult = g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, hAni);
    _ASSERT(dResult == LT_OK);

    CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hObject);

	// If we could update the dims, or we're forcing the animation, set it

    if ( pCharacter->SetDims(&vDims, LTFALSE) /*|| IS DEATH ANI*/)
	{
        return LTTRUE;
	}
	else
	{
        return LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIAnimal::IsAnimating*()
//
//	PURPOSE:	Is the parameter responsible for the current animation?
//
// ----------------------------------------------------------------------- //

LTBOOL CAnimatorAIAnimal::IsAnimatingMain(Main eMain) const
{
	return ( m_eLastMain == eMain && !IsAniTrackerDone(eAniTrackerMain) );
}

LTBOOL CAnimatorAIAnimal::IsAnimatingMainDone(Main eMain) const
{
	return ( m_eLastMain == eMain && IsAniTrackerDone(eAniTrackerMain) );
}