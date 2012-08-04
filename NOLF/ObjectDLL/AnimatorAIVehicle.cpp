// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "SFXMsgIds.h"
#include "AnimatorAIVehicle.h"
#include "Character.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIVehicle::CAnimatorAIVehicle()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAnimatorAIVehicle::CAnimatorAIVehicle()
{
	m_eMain	= eInvalid;
	m_eFire	= eFireAim;

	m_eLastMain	= eInvalid;
	m_eLastFire	= eFireAim;

	m_eAniTrackerFire = eAniTrackerInvalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIVehicle::~CAnimatorAIVehicle()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAnimatorAIVehicle::~CAnimatorAIVehicle()
{
    LTRESULT dwResult;

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerFire].m_pAnimTracker);
	_ASSERT(LT_OK == dwResult);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIVehicle::Init()
//
//	PURPOSE:	Initialize the AIVehicle animator
//
// ----------------------------------------------------------------------- //

void CAnimatorAIVehicle::Init(ILTCSBase *pInterface, HOBJECT hObject)
{
	CAnimator::Init(pInterface, hObject);

	// Setup all ani trackers

	m_eAniTrackerFire = AddAniTracker("Firing");
	EnableAniTracker(m_eAniTrackerFire);
    LoopAniTracker(m_eAniTrackerFire, LTTRUE);

#define _A(x) AddAni((x))
#define _N eAniInvalid

	// Get Main anis

    m_AniBase                   = AniAIVehicleMain(LTTRUE,   _A("Base"));
    m_AniIdle                   = AniAIVehicleMain(LTTRUE,   _A("Closed"));
    m_AniOpenRightDoor          = AniAIVehicleMain(LTFALSE,  _A("Open"));
    m_AniOpenedRightDoor        = AniAIVehicleMain(LTTRUE,   _A("Opened"));
    m_AniCloseRightDoor         = AniAIVehicleMain(LTFALSE,  _A("Close"));
    m_AniClosedRightDoor        = AniAIVehicleMain(LTTRUE,   _A("Closed"));

	// Get Fire anis

	m_AniFireAim				= AniAIVehicleFire(_A("Aiming"));
	m_AniFireFull				= AniAIVehicleFire(_A("Firing"));
	m_AniFire1					= AniAIVehicleFire(_A("Fire1"));
	m_AniFire2					= AniAIVehicleFire(_A("Fire2"));
	m_AniFire3					= AniAIVehicleFire(_A("Fire3"));
	m_AniFire4					= AniAIVehicleFire(_A("Fire4"));

	// Setup our lookup tables

    memset(m_apAniAIVehicleMains, LTNULL, sizeof(void*)*kNumMains);

	m_apAniAIVehicleMains[eIdle]			= &m_AniIdle;
	m_apAniAIVehicleMains[eOpenRightDoor]	= &m_AniOpenRightDoor;
	m_apAniAIVehicleMains[eOpenedRightDoor]	= &m_AniOpenedRightDoor;
	m_apAniAIVehicleMains[eCloseRightDoor]	= &m_AniCloseRightDoor;
	m_apAniAIVehicleMains[eClosedRightDoor]	= &m_AniClosedRightDoor;

	m_apAniAIVehicleFires[eFireAim]			= &m_AniFireAim;
	m_apAniAIVehicleFires[eFireFull]		= &m_AniFireFull;
	m_apAniAIVehicleFires[eFire1]			= &m_AniFire1;
	m_apAniAIVehicleFires[eFire2]			= &m_AniFire2;
	m_apAniAIVehicleFires[eFire3]			= &m_AniFire3;
	m_apAniAIVehicleFires[eFire4]			= &m_AniFire4;

	// The ani tracker that sets our dims is the main

	m_eAniTrackerDims = eAniTrackerMain;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIVehicle::Update()
//
//	PURPOSE:	Updates the AIVehicle animator
//
// ----------------------------------------------------------------------- //

void CAnimatorAIVehicle::Update()
{
	CAnimator::Update();

	if ( m_eMain != eInvalid )
	{
		AniAIVehicleMain* pAniAIVehicleMain = m_apAniAIVehicleMains[m_eMain];

		if ( !pAniAIVehicleMain )
		{
            g_pLTServer->CPrint("missing ani %d", m_eMain);
			return;
		}

		if ( pAniAIVehicleMain->bLoops && !IsAniTrackerLooping(eAniTrackerMain) )
		{
            LoopAniTracker(eAniTrackerMain, LTTRUE);
		}
		else if ( !pAniAIVehicleMain->bLoops && IsAniTrackerLooping(eAniTrackerMain) )
		{
            LoopAniTracker(eAniTrackerMain, LTFALSE);
		}

		SetAni(pAniAIVehicleMain->eAni, eAniTrackerMain);
	}
	else
	{
		_ASSERT(!"CAnimatorAIVehicle::Update - Main ani not set!");
	}

	AniAIVehicleFire* pAniAIVehicleFire = m_apAniAIVehicleFires[m_eFire];
	if ( pAniAIVehicleFire )
	{
		SetAni(pAniAIVehicleFire->eAni, m_eAniTrackerFire);
	}
	else
	{
		_ASSERT(!"CAnimatorAIVehicle::Update - No firing animation!");
	}

	// Record the state etc

	m_eLastMain = m_eMain;
	m_eLastFire = m_eFire;

	// Reset all vars

	m_eMain	= eInvalid;
	m_eFire	= eFireAim;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorAIVehicle::ResetAniTracker()
//
//	PURPOSE:	Resets an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimatorAIVehicle::ResetAniTracker(AniTracker eAniTracker)
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
//	ROUTINE:	CAnimatorAIVehicle::SetDims()
//
//	PURPOSE:	Sets the AIVehicle's dims based on the ani
//
// ----------------------------------------------------------------------- //

LTBOOL CAnimatorAIVehicle::SetDims(HMODELANIM hAni)
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
//	ROUTINE:	CAnimatorAIVehicle::IsAnimating*()
//
//	PURPOSE:	Is the parameter responsible for the current animation?
//
// ----------------------------------------------------------------------- //

LTBOOL CAnimatorAIVehicle::IsAnimatingMain(Main eMain) const
{
	return ( m_eLastMain == eMain && !IsAniTrackerDone(eAniTrackerMain) );
}

LTBOOL CAnimatorAIVehicle::IsAnimatingMainDone(Main eMain) const
{
	return ( m_eLastMain == eMain && IsAniTrackerDone(eAniTrackerMain) );
}