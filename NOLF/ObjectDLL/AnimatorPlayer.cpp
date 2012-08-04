// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "SFXMsgIds.h"
#include "AnimatorPlayer.h"
#include "PlayerObj.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::CAnimatorPlayer()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAnimatorPlayer::CAnimatorPlayer()
{
	m_eMain	= eInvalid;
	m_eWeapon = eRifle;
	m_ePosture = eUnalert;
	m_eMovement = eWalking;
	m_eDirection = eNone;

	m_eLastMain	= eInvalid;
	m_eLastWeapon = eRifle;
	m_eLastPosture = eUnalert;
	m_eLastMovement = eWalking;
	m_eLastDirection = eNone;

	m_eAniTrackerUpper = eAniTrackerInvalid;
	m_eAniTrackerLower = eAniTrackerInvalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::~CAnimatorPlayer()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAnimatorPlayer::~CAnimatorPlayer()
{
    LTRESULT dwResult;

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerUpper].m_pAnimTracker);
	_ASSERT(LT_OK == dwResult);

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerLower].m_pAnimTracker);
	_ASSERT(LT_OK == dwResult);

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerPitchUp].m_pAnimTracker);
	_ASSERT(LT_OK == dwResult);

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerPitchDown].m_pAnimTracker);
	_ASSERT(LT_OK == dwResult);
}

// ----------------------------------------------------------------------- //

void CAnimatorPlayer::Save(HMESSAGEWRITE hWrite)
{
	SAVE_DWORD(m_eLastMain);
	SAVE_DWORD(m_eLastWeapon);
	SAVE_DWORD(m_eLastPosture);
	SAVE_DWORD(m_eLastMovement);
	SAVE_DWORD(m_eLastDirection);
}

// ----------------------------------------------------------------------- //

void CAnimatorPlayer::Load(HMESSAGEREAD hRead)
{
	LOAD_DWORD_CAST(m_eLastMain, Main);
	LOAD_DWORD_CAST(m_eLastWeapon, Weapon);
	LOAD_DWORD_CAST(m_eLastPosture, Posture);
	LOAD_DWORD_CAST(m_eLastMovement, Movement);
	LOAD_DWORD_CAST(m_eLastDirection, Direction);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::Reset()
//
//	PURPOSE:	Initialize the Player animator
//
// ----------------------------------------------------------------------- //

void CAnimatorPlayer::Reset(HOBJECT hObject)
{
    LTRESULT dwResult;

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerUpper].m_pAnimTracker);
    _ASSERT(LT_OK == dwResult);

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerLower].m_pAnimTracker);
    _ASSERT(LT_OK == dwResult);

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerPitchUp].m_pAnimTracker);
    _ASSERT(LT_OK == dwResult);

    dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerPitchDown].m_pAnimTracker);
    _ASSERT(LT_OK == dwResult);

	m_cAniTrackers = 1;

	m_cAnis = 1;

	m_eAniTrackerDims = eAniTrackerInvalid;
	m_hObject = NULL;

    m_bInitialized = LTFALSE;

	Init(g_pLTServer, hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::Init()
//
//	PURPOSE:	Initialize the Player animator
//
// ----------------------------------------------------------------------- //

void CAnimatorPlayer::Init(ILTCSBase* pInterface, HOBJECT hObject)
{
	_ASSERT(!m_bInitialized);
	if ( m_bInitialized )
	{
		g_pLTServer->CPrint("CAnimatorPlayer already initialized!");
		return;
	}

	StartTimingCounter();

	CAnimator::Init(pInterface, hObject);

	// Setup all ani trackers

	m_eAniTrackerPitch = m_eAniTrackerUpper = AddAniTracker("Upper");
	EnableAniTracker(m_eAniTrackerUpper);

	m_eAniTrackerLower = AddAniTracker("Lower");
	EnableAniTracker(m_eAniTrackerLower);

	m_eAniTrackerPitchDown = AddAniTracker("Morph0");
	m_eAniTrackerPitchUp = AddAniTracker("Morph0");

#define _A(x) AddAni((x))
#define _N eAniInvalid

	// Get Lower anis

    m_AniLowerBase              = AniPlayerLower            (LTTRUE,     _A("Base"));

    m_AniStand                  = AniPlayerLower            (LTTRUE,     _A("LSt"));

    m_AniWalkForward            = AniPlayerLower            (LTTRUE,     _A("LWF"));
    m_AniWalkBackward           = AniPlayerLower            (LTTRUE,     _A("LWB"));
    m_AniWalkStrafeLeft         = AniPlayerLowerStrafe      (LTTRUE,     _A("LWL"));
    m_AniWalkStrafeRight        = AniPlayerLowerStrafe      (LTTRUE,     _A("LWR"));

    m_AniRunForward             = AniPlayerLower            (LTTRUE,     _A("LRF"));
    m_AniRunBackward            = AniPlayerLower            (LTTRUE,     _A("LRB"));
    m_AniRunStrafeLeft          = AniPlayerLowerStrafe      (LTTRUE,     _A("LRL"));
    m_AniRunStrafeRight         = AniPlayerLowerStrafe      (LTTRUE,     _A("LRR"));

    m_AniCrouch                 = AniPlayerLowerCrouch      (LTTRUE,     _A("LC"));
    m_AniCrouchForward          = AniPlayerLowerCrouch      (LTTRUE,     _A("LCF"));
    m_AniCrouchBackward         = AniPlayerLowerCrouch      (LTTRUE,     _A("LCB"));
    m_AniCrouchStrafeLeft       = AniPlayerLowerCrouchStrafe(LTTRUE,     _A("LCL"));
    m_AniCrouchStrafeRight      = AniPlayerLowerCrouchStrafe(LTTRUE,     _A("LCR"));

    m_AniSwim                   = AniPlayerLowerSwim        (LTTRUE,     _A("LSw"));
    m_AniSwimForward            = AniPlayerLowerSwim        (LTTRUE,     _A("LSwF"));

    m_AniJumpJump               = AniPlayerLower            (LTFALSE,    _A("LJJ"));
    m_AniJumpTuck               = AniPlayerLower            (LTTRUE,     _A("LJT"));
    m_AniJumpLand               = AniPlayerLower            (LTFALSE,    _A("LJL"));

	// Get Upper anis

    m_AniUpperBase              = AniPlayerUpper           (LTTRUE,     _A("Base"), _N,             _N,             _N,             _N);

    m_AniRifleUnalert           = AniPlayerUpper           (LTTRUE,     _A("URUn"), _N,             _N,             _N,             _N);
    m_AniRifleAlert             = AniPlayerUpper           (LTTRUE,     _A("URAl"), _N,             _N,             _N,             _N);
    m_AniRifleAim               = AniPlayerUpper           (LTTRUE,     _A("URAm"), _A("URSAm"),    _A("URCAm"),    _A("URSwAm"),   _A("URCSAm"));
    m_AniRifleFire              = AniPlayerUpper           (LTFALSE,    _A("URFi"), _A("URSFi"),    _A("URCFi"),    _A("URSwFi"),   _A("URCSFi"));
    m_AniRifleReload            = AniPlayerUpper           (LTFALSE,    _A("URRe"), _N,             _A("URCRe"),    _A("URSwRe"),   _N);
    m_AniRifleSelect            = AniPlayerUpper           (LTFALSE,    _A("URSe"), _N,             _A("URCRe"),    _A("URSwRe"),   _N);
    m_AniRifleDeselect          = AniPlayerUpper           (LTFALSE,    _A("URDe"), _N,             _A("URCRe"),    _A("URSwRe"),   _N);

    m_AniPistolUnalert          = AniPlayerUpper           (LTTRUE,     _A("UPUn"), _N,             _N,             _N,             _N);
    m_AniPistolAlert            = AniPlayerUpper           (LTTRUE,     _A("UPAl"), _N,             _N,             _N,             _N);
    m_AniPistolAim              = AniPlayerUpper           (LTTRUE,     _A("UPAm"), _A("UPSAm"),    _A("UPCAm"),    _N,             _A("UPCSAm"));
    m_AniPistolFire             = AniPlayerUpper           (LTFALSE,    _A("UPFi"), _A("UPSFi"),    _A("UPCFi"),    _N,             _A("UPCSFi"));
    m_AniPistolReload           = AniPlayerUpper           (LTFALSE,    _A("UPRe"), _N,             _A("UPCRe"),    _N,             _N);
    m_AniPistolSelect           = AniPlayerUpper           (LTFALSE,    _A("UPSe"), _N,             _A("URCRe"),    _N,             _N);
    m_AniPistolDeselect         = AniPlayerUpper           (LTFALSE,    _A("UPDe"), _N,             _A("URCRe"),    _N,             _N);

    m_AniMeleeUnalert           = AniPlayerUpper           (LTTRUE,     _A("UMUn"), _N,             _N,             _N,             _N);
    m_AniMeleeAlert             = AniPlayerUpper           (LTTRUE,     _A("UMAl"), _N,             _N,             _N,             _N);
    m_AniMeleeAim               = AniPlayerUpper           (LTTRUE,     _A("UMAm"), _A("UMSAm"),    _A("UMCAm"),    _N,             _A("UMCSAm"));
    m_AniMeleeFire              = AniPlayerUpper           (LTFALSE,    _A("UMFi"), _A("UMSFi"),    _A("UMCFi"),    _N,             _A("UMCSFi"));
    m_AniMeleeReload            = AniPlayerUpper           (LTFALSE,    _A("UMRe"), _N,             _A("UMCRe"),    _N,             _N);
    m_AniMeleeSelect            = AniPlayerUpper           (LTFALSE,    _A("UMSe"), _N,             _A("UMCRe"),    _N,             _N);
    m_AniMeleeDeselect          = AniPlayerUpper           (LTFALSE,    _A("UMDe"), _N,             _A("UMCRe"),    _N,             _N);

    m_AniThrowUnalert           = AniPlayerUpper           (LTTRUE,     _A("UTUn"), _N,             _N,             _N,             _N);
    m_AniThrowAlert             = AniPlayerUpper           (LTTRUE,     _A("UTAl"), _N,             _N,             _N,             _N);
    m_AniThrowAim               = AniPlayerUpper           (LTTRUE,     _A("UTAm"), _A("UTSAm"),    _A("UTCAm"),    _N,             _A("UTCSAm"));
    m_AniThrowFire              = AniPlayerUpper           (LTFALSE,    _A("UTFi"), _A("UTSFi"),    _A("UTCFi"),    _N,             _A("UTCSFi"));
    m_AniThrowReload            = AniPlayerUpper           (LTFALSE,    _A("UTRe"), _N,             _A("UTCRe"),    _N,             _N);
    m_AniThrowSelect            = AniPlayerUpper           (LTFALSE,    _A("UTSe"), _N,             _A("UTCRe"),    _N,             _N);
    m_AniThrowDeselect          = AniPlayerUpper           (LTFALSE,    _A("UTDe"), _N,             _A("UTCRe"),    _N,             _N);

    m_AniSunglassesUnalert      = AniPlayerUpper           (LTTRUE,     _A("Sung"), _N,				_N,				_N,             _N);
    m_AniSunglassesAlert        = AniPlayerUpper           (LTTRUE,     _A("Sung"), _N,				_N,				_N,             _N);
    m_AniSunglassesAim          = AniPlayerUpper           (LTTRUE,     _A("Sung"), _N,				_N,				_N,             _N);
    m_AniSunglassesFire         = AniPlayerUpper           (LTFALSE,    _A("Sung"), _N,				_N,				_N,             _N);
    m_AniSunglassesReload       = AniPlayerUpper           (LTFALSE,    _A("Sung"), _N,				_N,				_N,             _N);
    m_AniSunglassesSelect       = AniPlayerUpper           (LTFALSE,    _A("Sung"), _N,				_N,				_N,             _N);
    m_AniSunglassesDeselect     = AniPlayerUpper           (LTFALSE,    _A("Sung"), _N,				_N,				_N,             _N);

	// Get Main anis

    m_AniMainBase               = AniPlayerMain            (LTTRUE,     _A("Base"));

    m_AniClimb                  = AniPlayerMain            (LTTRUE,     _A("Cl"));
    m_AniClimbUp                = AniPlayerMain            (LTTRUE,     _A("ClU"));
    m_AniClimbDown              = AniPlayerMain            (LTTRUE,     _A("ClD"));
    m_AniMotorcycle             = AniPlayerMain            (LTTRUE,     _A("LRdM"));
    m_AniSnowmobile             = AniPlayerMain            (LTTRUE,     _A("LRdS"));

	// Setup our lookup tables

    memset(m_aapAniPlayerUppers, LTNULL, sizeof(void*)*kNumWeapons*kNumPostures);
    memset(m_aapAniPlayerLowers, LTNULL, sizeof(void*)*kNumMovements*kNumDirections);
    memset(m_apAniPlayerMains, LTNULL, sizeof(void*)*kNumMains);

	m_aapAniPlayerUppers[eRifle][eUnalert]			= &m_AniRifleUnalert;
	m_aapAniPlayerUppers[eRifle][eAlert]			= &m_AniRifleAlert;
	m_aapAniPlayerUppers[eRifle][eAim]				= &m_AniRifleAim;
	m_aapAniPlayerUppers[eRifle][eFire]				= &m_AniRifleFire;
	m_aapAniPlayerUppers[eRifle][eReload]			= &m_AniRifleReload;
	m_aapAniPlayerUppers[eRifle][eSelect]			= &m_AniRifleSelect;
	m_aapAniPlayerUppers[eRifle][eDeselect]			= &m_AniRifleDeselect;

	m_aapAniPlayerUppers[ePistol][eUnalert]			= &m_AniPistolUnalert;
	m_aapAniPlayerUppers[ePistol][eAlert]			= &m_AniPistolAlert;
	m_aapAniPlayerUppers[ePistol][eAim]				= &m_AniPistolAim;
	m_aapAniPlayerUppers[ePistol][eFire]			= &m_AniPistolFire;
	m_aapAniPlayerUppers[ePistol][eReload]			= &m_AniPistolReload;
	m_aapAniPlayerUppers[ePistol][eSelect]			= &m_AniPistolSelect;
	m_aapAniPlayerUppers[ePistol][eDeselect]		= &m_AniPistolDeselect;

	m_aapAniPlayerUppers[eMelee][eUnalert]			= &m_AniMeleeUnalert;
	m_aapAniPlayerUppers[eMelee][eAlert]			= &m_AniMeleeAlert;
	m_aapAniPlayerUppers[eMelee][eAim]				= &m_AniMeleeAim;
	m_aapAniPlayerUppers[eMelee][eFire]				= &m_AniMeleeFire;
	m_aapAniPlayerUppers[eMelee][eReload]			= &m_AniMeleeReload;
	m_aapAniPlayerUppers[eMelee][eSelect]			= &m_AniMeleeSelect;
	m_aapAniPlayerUppers[eMelee][eDeselect]			= &m_AniMeleeDeselect;

	m_aapAniPlayerUppers[eThrow][eUnalert]			= &m_AniThrowUnalert;
	m_aapAniPlayerUppers[eThrow][eAlert]			= &m_AniThrowAlert;
	m_aapAniPlayerUppers[eThrow][eAim]				= &m_AniThrowAim;
	m_aapAniPlayerUppers[eThrow][eFire]				= &m_AniThrowFire;
	m_aapAniPlayerUppers[eThrow][eReload]			= &m_AniThrowReload;
	m_aapAniPlayerUppers[eThrow][eSelect]			= &m_AniThrowSelect;
	m_aapAniPlayerUppers[eThrow][eDeselect]			= &m_AniThrowDeselect;

	m_aapAniPlayerUppers[eSunglasses][eUnalert]		= &m_AniSunglassesUnalert;
	m_aapAniPlayerUppers[eSunglasses][eAlert]		= &m_AniSunglassesAlert;
	m_aapAniPlayerUppers[eSunglasses][eAim]			= &m_AniSunglassesAim;
	m_aapAniPlayerUppers[eSunglasses][eFire]		= &m_AniSunglassesFire;
	m_aapAniPlayerUppers[eSunglasses][eReload]		= &m_AniSunglassesReload;
	m_aapAniPlayerUppers[eSunglasses][eSelect]		= &m_AniSunglassesSelect;
	m_aapAniPlayerUppers[eSunglasses][eDeselect]	= &m_AniSunglassesDeselect;

	m_aapAniPlayerLowers[eWalking][eNone]			= &m_AniStand;
	m_aapAniPlayerLowers[eWalking][eForward]		= &m_AniWalkForward;
	m_aapAniPlayerLowers[eWalking][eBackward]		= &m_AniWalkBackward;
	m_aapAniPlayerLowers[eWalking][eStrafeLeft]		= &m_AniWalkStrafeLeft;
	m_aapAniPlayerLowers[eWalking][eStrafeRight]	= &m_AniWalkStrafeRight;

	m_aapAniPlayerLowers[eRunning][eNone]			= &m_AniStand;
	m_aapAniPlayerLowers[eRunning][eForward]		= &m_AniRunForward;
	m_aapAniPlayerLowers[eRunning][eBackward]		= &m_AniRunBackward;
	m_aapAniPlayerLowers[eRunning][eStrafeLeft]		= &m_AniRunStrafeLeft;
	m_aapAniPlayerLowers[eRunning][eStrafeRight]	= &m_AniRunStrafeRight;

	m_aapAniPlayerLowers[eCrouching][eNone]			= &m_AniCrouch;
	m_aapAniPlayerLowers[eCrouching][eForward]		= &m_AniCrouchForward;
	m_aapAniPlayerLowers[eCrouching][eBackward]		= &m_AniCrouchBackward;
	m_aapAniPlayerLowers[eCrouching][eStrafeLeft]	= &m_AniCrouchStrafeLeft;
	m_aapAniPlayerLowers[eCrouching][eStrafeRight]	= &m_AniCrouchStrafeRight;

	m_aapAniPlayerLowers[eSwimming][eNone]			= &m_AniSwim;
	m_aapAniPlayerLowers[eSwimming][eForward]		= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eBackward]		= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eStrafeLeft]	= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eStrafeRight]	= &m_AniSwimForward;

	m_aapAniPlayerLowers[eJumping][eJump]			= &m_AniJumpJump;
	m_aapAniPlayerLowers[eJumping][eTuck]			= &m_AniJumpTuck;
	m_aapAniPlayerLowers[eJumping][eLand]			= &m_AniJumpLand;

	m_apAniPlayerMains[eClimbing]					= &m_AniClimb;
	m_apAniPlayerMains[eClimbingUp]					= &m_AniClimbUp;
	m_apAniPlayerMains[eClimbingDown]				= &m_AniClimbDown;
	m_apAniPlayerMains[eMotorcycle]					= &m_AniMotorcycle;
	m_apAniPlayerMains[eSnowmobile]					= &m_AniSnowmobile;

	// The ani tracker that sets our dims is the lower

	m_eAniTrackerDims = m_eAniTrackerLower;

	for ( uint32 iPitchWeightset = 0 ; iPitchWeightset < kNumPitchWeightsets ; iPitchWeightset++ )
	{
		char szPitchWeightset[512];
		LTFLOAT fWeightsetAmount = 100.0f/(LTFLOAT)(kNumPitchWeightsets-1);
		sprintf(szPitchWeightset, "Morph%d", (int32)(iPitchWeightset*fWeightsetAmount));

		if ( LT_OK != g_pModelLT->FindWeightSet(hObject, szPitchWeightset, m_ahPitchWeightsets[iPitchWeightset]) )
		{
//			g_pLTServer->CPrint("could not find pitch weightset %s", szPitchWeightset);
		}
	}

	DisablePitch();

	EndTimingCounter("CAnimatorPlayer::Init took");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::Update()
//
//	PURPOSE:	Updates the Player animator
//
// ----------------------------------------------------------------------- //

void CAnimatorPlayer::Update()
{
	CAnimator::Update();

	_ASSERT(m_eAniTrackerUpper != eAniTrackerInvalid);
	_ASSERT(m_eAniTrackerLower != eAniTrackerInvalid);

	// Set the upper/lower or fully body anis

	if ( m_eMain != eInvalid && m_eLastMain == eInvalid )
	{
		DisableAniTracker(m_eAniTrackerUpper);
		DisableAniTracker(m_eAniTrackerLower);

		if ( !IsAniTrackerLooping(m_eAniTrackerUpper) )
		{
            LoopAniTracker(m_eAniTrackerUpper, LTTRUE);
		}
		if ( !IsAniTrackerLooping(m_eAniTrackerLower) )
		{
            LoopAniTracker(m_eAniTrackerLower, LTTRUE);
		}

		SetAni(m_AniUpperBase.eAni, m_eAniTrackerUpper);
		SetAni(m_AniLowerBase.eAni, m_eAniTrackerLower);
	}

	if ( m_eMain == eInvalid && m_eLastMain != eInvalid )
	{
		EnableAniTracker(m_eAniTrackerUpper);
		EnableAniTracker(m_eAniTrackerLower);

		if ( !IsAniTrackerLooping(eAniTrackerMain) )
		{
            LoopAniTracker(eAniTrackerMain, LTTRUE);
		}

		SetAni(m_AniMainBase.eAni, eAniTrackerMain);
	}

	if ( m_eMain != eInvalid )
	{
		AniPlayerMain* pAniPlayerMain = m_apAniPlayerMains[m_eMain];

		if ( !pAniPlayerMain )
		{
            g_pLTServer->CPrint("missing ani %d", m_eMain);
			return;
		}

		if ( pAniPlayerMain->bLoops && !IsAniTrackerLooping(eAniTrackerMain) )
		{
            LoopAniTracker(eAniTrackerMain, LTTRUE);
		}
		else if ( !pAniPlayerMain->bLoops && IsAniTrackerLooping(eAniTrackerMain) )
		{
            LoopAniTracker(eAniTrackerMain, LTFALSE);
		}

		SetAni(pAniPlayerMain->eAni, eAniTrackerMain);
	}
	else
	{
		AniPlayerUpper* pAniPlayerUpper = m_aapAniPlayerUppers[m_eWeapon][m_ePosture];
		AniPlayerLower* pAniPlayerLower = m_aapAniPlayerLowers[m_eMovement][m_eDirection];

		if ( !pAniPlayerUpper || !pAniPlayerLower )
		{
            g_pLTServer->CPrint("missing ani %d/%d  %d/%d", m_eWeapon, m_ePosture, m_eMovement, m_eDirection);
			return;
		}

		Ani eAniUpper = pAniPlayerUpper->eAni;
		Ani eAniLower = pAniPlayerLower->eAni;

		if ( pAniPlayerLower->IsStrafe() )
		{
			if ( pAniPlayerLower->IsCrouch() )
			{
				if ( pAniPlayerUpper->eAniStrafeCrouch != eAniInvalid )
				{
					eAniUpper = pAniPlayerUpper->eAniStrafeCrouch;
				}
			}
			else
			{
				if ( pAniPlayerUpper->eAniStrafe != eAniInvalid )
				{
					eAniUpper = pAniPlayerUpper->eAniStrafe;
				}
			}
		}
		else if ( pAniPlayerLower->IsCrouch() )
		{
			if ( pAniPlayerUpper->eAniCrouch != eAniInvalid )
			{
				eAniUpper = pAniPlayerUpper->eAniCrouch;
			}
		}
		else if ( pAniPlayerLower->IsSwim() )
		{
			if ( pAniPlayerUpper->eAniSwim != eAniInvalid )
			{
				eAniUpper = pAniPlayerUpper->eAniSwim;
			}
		}

		if ( pAniPlayerUpper->bLoops && !IsAniTrackerLooping(m_eAniTrackerUpper) )
		{
            LoopAniTracker(m_eAniTrackerUpper, LTTRUE);
		}
		else if ( !pAniPlayerUpper->bLoops && IsAniTrackerLooping(m_eAniTrackerUpper) )
		{
            LoopAniTracker(m_eAniTrackerUpper, LTFALSE);
		}

		if ( pAniPlayerLower->bLoops && !IsAniTrackerLooping(m_eAniTrackerLower) )
		{
            LoopAniTracker(m_eAniTrackerLower, LTTRUE);
		}
		else if ( !pAniPlayerLower->bLoops && IsAniTrackerLooping(m_eAniTrackerLower) )
		{
            LoopAniTracker(m_eAniTrackerLower, LTFALSE);
		}

		SetAni(eAniUpper, m_eAniTrackerUpper);
		SetAni(eAniLower, m_eAniTrackerLower);
	}

	// Record the state etc

	m_eLastMain = m_eMain;
	m_eLastWeapon = m_eWeapon;
	m_eLastPosture = m_ePosture;
	m_eLastMovement	= m_eMovement;
	m_eLastDirection = m_eDirection;

	// Reset all vars

	m_eMain	= eInvalid;
	m_eWeapon = eRifle;
	m_ePosture = eUnalert;
	m_eMovement = eWalking;
	m_eDirection = eNone;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::ResetAniTracker()
//
//	PURPOSE:	Resets an anitracker
//
// ----------------------------------------------------------------------- //

void CAnimatorPlayer::ResetAniTracker(AniTracker eAniTracker)
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
	else if ( eAniTracker == m_eAniTrackerLower )
	{
        g_pLTServer->WriteToMessageByte(hMessage, CFX_RESET_TRACKER);
        g_pLTServer->WriteToMessageByte(hMessage, 1);
	}
	else if ( eAniTracker == m_eAniTrackerUpper )
	{
        g_pLTServer->WriteToMessageByte(hMessage, CFX_RESET_TRACKER);
        g_pLTServer->WriteToMessageByte(hMessage, 2);
	}
    g_pLTServer->EndMessage(hMessage);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::UpdateDims()
//
//	PURPOSE:	Updates the Player's dims based on the ani
//
// ----------------------------------------------------------------------- //

void CAnimatorPlayer::UpdateDims()
{
	HMODELANIM hAni = GetAni(m_eAniTrackerLower);
	
	if ( INVALID_MODEL_ANIM != hAni )
	{
		LTVector vDims;
		LTRESULT dResult = g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, hAni);
		_ASSERT(dResult == LT_OK);

		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);

		pPlayer->ForceDuck(LTFALSE);

		// If we could update the dims, or we're forcing the animation, set it

		if ( pPlayer->SetDims(&vDims, LTFALSE) /*|| IS DEATH ANI*/)
		{
		}
		else
		{
			// If we were ducking, and we tried to stand but couldn't, make us continue to duck

			if ((m_eLastMovement == eCrouching) && !(m_eMovement == eCrouching))
			{
				m_eMovement = eCrouching;
				pPlayer->ForceDuck(LTTRUE);
			}
			else
			{
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::SetDims()
//
//	PURPOSE:	Sets the Player's dims based on the ani
//
// ----------------------------------------------------------------------- //

LTBOOL CAnimatorPlayer::SetDims(HMODELANIM hAni)
{
    LTVector vDims;
    LTRESULT dResult = g_pLTServer->GetModelAnimUserDims(m_hObject, &vDims, hAni);
    _ASSERT(dResult == LT_OK);

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);

	pPlayer->ForceDuck(LTFALSE);

	// If we could update the dims, or we're forcing the animation, set it

    if ( pPlayer->SetDims(&vDims, LTFALSE) /*|| IS DEATH ANI*/)
	{
        return LTTRUE;
	}
	else
	{
		// If we were ducking, and we tried to stand but couldn't, make us continue to duck

		if ((m_eLastMovement == eCrouching) && !(m_eMovement == eCrouching))
		{
			m_eMovement = eCrouching;
			pPlayer->ForceDuck(LTTRUE);
		}
		else
		{
		}

        return LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::IsAnimating*()
//
//	PURPOSE:	Is the parameter responsible for the current animation?
//
// ----------------------------------------------------------------------- //

LTBOOL CAnimatorPlayer::IsAnimatingMain(Main eMain) const
{
	return ( m_eLastMain == eMain && !IsAniTrackerDone(eAniTrackerMain) );
}

LTBOOL CAnimatorPlayer::IsAnimatingWeapon(Weapon eWeapon) const
{
	return ( m_eLastWeapon == eWeapon && !IsAniTrackerDone(m_eAniTrackerUpper) );
}

LTBOOL CAnimatorPlayer::IsAnimatingPosture(Posture ePosture) const
{
	return ( m_eLastPosture == ePosture && !IsAniTrackerDone(m_eAniTrackerUpper) );
}

LTBOOL CAnimatorPlayer::IsAnimatingMovement(Movement eMovement) const
{
	return ( m_eLastMovement == eMovement && !IsAniTrackerDone(m_eAniTrackerLower) );
}

LTBOOL CAnimatorPlayer::IsAnimatingDirection(Direction eDirection) const
{
	return ( m_eLastDirection == eDirection && !IsAniTrackerDone(m_eAniTrackerLower) );
}

LTBOOL CAnimatorPlayer::IsAnimatingMainDone(Main eMain) const
{
	return ( m_eLastMain == eMain && IsAniTrackerDone(eAniTrackerMain) );
}

LTBOOL CAnimatorPlayer::IsAnimatingWeaponDone(Weapon eWeapon) const
{
	return ( m_eLastWeapon == eWeapon && IsAniTrackerDone(m_eAniTrackerUpper) );
}

LTBOOL CAnimatorPlayer::IsAnimatingPostureDone(Posture ePosture) const
{
	return ( m_eLastPosture == ePosture && IsAniTrackerDone(m_eAniTrackerUpper) );
}

LTBOOL CAnimatorPlayer::IsAnimatingMovementDone(Movement eMovement) const
{
	return ( m_eLastMovement == eMovement && IsAniTrackerDone(m_eAniTrackerLower) );
}

LTBOOL CAnimatorPlayer::IsAnimatingDirectionDone(Direction eDirection) const
{
	return ( m_eLastDirection == eDirection && IsAniTrackerDone(m_eAniTrackerLower) );
}