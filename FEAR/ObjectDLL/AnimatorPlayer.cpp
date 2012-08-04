// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "Stdafx.h"
#include "ObjectMsgs.h"
#include "MsgIDs.h"
#include "SFXMsgIds.h"
#include "AnimatorPlayer.h"
#include "PlayerObj.h"

static VarTrack s_vtPlayerAnimDebug;

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
	m_eLean = eCenter;

	m_eLastMain	= eInvalid;
	m_eLastWeapon = eRifle;
	m_eLastPosture = eUnalert;
	m_eLastMovement = eWalking;
	m_eLastDirection = eNone;
	m_eLastLean = eCenter;

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

    if( m_hObject )
	{
		dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerUpper].m_AnimTracker);
		LTASSERT(LT_OK == dwResult, "TODO: Add description here");

		dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerLower].m_AnimTracker);
		LTASSERT(LT_OK == dwResult, "TODO: Add description here");
	}
}

// ----------------------------------------------------------------------- //

void CAnimatorPlayer::Save(ILTMessage_Write *pMsg)
{
	SAVE_DWORD( m_eLastMain );
	SAVE_DWORD( m_eLastWeapon );
	SAVE_DWORD( m_eLastPosture );
	SAVE_DWORD( m_eLastMovement );
	SAVE_DWORD( m_eLastDirection );
	SAVE_DWORD( m_eLastLean );
	
	SAVE_DWORD( m_eAniTrackerUpper );
	SAVE_DWORD( m_eAniTrackerLower );

	SAVE_HOBJECT( m_hObject );

	SAVE_DWORD( m_cAniTrackers );

	for( int i = 0; i < m_cAniTrackers; ++i )
	{
		SAVE_DWORD( m_aAniTrackers[i].m_AnimTracker );
		SAVE_STDSTRING( m_aAniTrackers[i].m_sWeightset );
	}

}

// ----------------------------------------------------------------------- //

void CAnimatorPlayer::Load(ILTMessage_Read *pMsg)
{
	LOAD_DWORD_CAST( m_eMain, Main );
	LOAD_DWORD_CAST( m_eWeapon, Weapon );
	LOAD_DWORD_CAST( m_ePosture, Posture );
	LOAD_DWORD_CAST( m_eMovement, Movement );
	LOAD_DWORD_CAST( m_eDirection, Direction );
	LOAD_DWORD_CAST( m_eLean, Lean );

	LOAD_DWORD_CAST( m_eAniTrackerUpper, AniTracker );
	LOAD_DWORD_CAST( m_eAniTrackerLower, AniTracker );

	LOAD_HOBJECT( m_hObject );

	LOAD_DWORD( m_cAniTrackers );

	for( int i = 0; i < m_cAniTrackers; ++i )
	{
		LOAD_DWORD_CAST( m_aAniTrackers[i].m_AnimTracker, ANIMTRACKERID );
		LOAD_STDSTRING( m_aAniTrackers[i].m_sWeightset );
	}

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

	if (m_hObject)
	{
		dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerUpper].m_AnimTracker);
		m_eAniTrackerUpper = eAniTrackerInvalid;
		LTASSERT(LT_OK == dwResult || LT_NOTFOUND == dwResult, "TODO: Add description here");

		dwResult = g_pModelLT->RemoveTracker(m_hObject, m_aAniTrackers[m_eAniTrackerLower].m_AnimTracker);
		m_eAniTrackerLower = eAniTrackerInvalid;
		LTASSERT(LT_OK == dwResult || LT_NOTFOUND == dwResult, "TODO: Add description here");
	}

	m_cAniTrackers = 1;

	m_cAnis = 1;

	m_eAniTrackerDims = eAniTrackerInvalid;
	m_hObject = NULL;

    m_bInitialized = false;

	Init(hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::Init()
//
//	PURPOSE:	Initialize the Player animator
//
// ----------------------------------------------------------------------- //

void CAnimatorPlayer::Init(HOBJECT hObject)
{
	LTASSERT(!m_bInitialized, "TODO: Add description here");
	if ( m_bInitialized )
	{
		g_pLTServer->CPrint("CAnimatorPlayer already initialized!");
		return;
	}

	if( !s_vtPlayerAnimDebug.IsInitted() )
	{
		s_vtPlayerAnimDebug.Init( g_pLTServer, "PlayerAnimationDebug", NULL, -1.0f );
	}

	StartTimingCounterServer();

	CAnimator::Init(hObject);

	// Setup all ani trackers

	if( m_eAniTrackerUpper == eAniTrackerInvalid )
	{
		m_eAniTrackerUpper = AddAniTracker("Upper");
		EnableAniTracker(m_eAniTrackerUpper);
	}

	if( m_eAniTrackerLower == eAniTrackerInvalid )
	{
		m_eAniTrackerLower = AddAniTracker("Lower");
		EnableAniTracker(m_eAniTrackerLower);
	}
	
#define _A(x) AddAni((x))
#define _N eAniInvalid

	// Get Lower anis

	m_AniLowerBase				= AniPlayerLower				(true,	_A("Base"));

	m_AniStand					= AniPlayerLower				(true,	_A("LSt"));
	m_AniStandLeanL				= AniPlayerLower				(true,	_A("LStL"));
	m_AniStandLeanR				= AniPlayerLower				(true,	_A("LStR"));

	m_AniWalkForward			= AniPlayerLower				(true,	_A("LWF"));
	m_AniWalkBackward			= AniPlayerLower				(true,	_A("LWB"));
	m_AniWalkStrafeLeft			= AniPlayerLowerStrafe			(true,	_A("LWL"));
	m_AniWalkStrafeRight		= AniPlayerLowerStrafe			(true,	_A("LWR"));

	m_AniRunForward				= AniPlayerLower				(true,	_A("LRF"));
	m_AniRunBackward			= AniPlayerLower				(true,	_A("LRB"));
	m_AniRunStrafeLeft			= AniPlayerLowerStrafe			(true,	_A("LRL"));
	m_AniRunStrafeRight			= AniPlayerLowerStrafe			(true,	_A("LRR"));

	m_AniCrouch					= AniPlayerLowerCrouch			(true,	_A("LC"));
	m_AniCrouchLeanL			= AniPlayerLowerCrouch			(true,	_A("LCLL"));
	m_AniCrouchLeanR			= AniPlayerLowerCrouch			(true,	_A("LCLR"));
	m_AniCrouchForward			= AniPlayerLowerCrouch			(true,	_A("LCF"));
	m_AniCrouchBackward			= AniPlayerLowerCrouch			(true,	_A("LCB"));
	m_AniCrouchStrafeLeft		= AniPlayerLowerCrouchStrafe	(true,	_A("LCL"));
	m_AniCrouchStrafeRight		= AniPlayerLowerCrouchStrafe	(true,	_A("LCR"));

	m_AniSwim					= AniPlayerLowerSwim			(true,	_A("LSw"));
	m_AniSwimForward			= AniPlayerLowerSwim			(true,	_A("LSwF"));


	m_AniJumpJump				= AniPlayerLower				(false,	_A("LJJ"));
	m_AniJumpTuck				= AniPlayerLower				(true, 	_A("LJT"));
	m_AniJumpLand				= AniPlayerLower				(false,	_A("LJL"));

	// Get Upper anis

	m_AniUpperBase				= AniPlayerUpper				(true,	_A("Base"),	_N,	_N,	_N,	_N);


	m_AniRifleUnalert			= AniPlayerUpper				(true,	_A("URUn"), _N,				_N,             _N,             _N);
	m_AniRifleAlert				= AniPlayerUpper				(true,	_A("URAl"), _N,				_N,             _N,             _N);
	m_AniRifleAim				= AniPlayerUpper				(true,	_A("URAm"), _A("URSAm"),	_A("URCAm"),    _A("URSwAm"),   _A("URCSAm"));
	m_AniRifleFire				= AniPlayerUpper				(false,	_A("URFi"), _A("URSFi"),	_A("URCFi"),    _A("URSwFi"),   _A("URCSFi"));
	m_AniRifleReload			= AniPlayerUpper				(false,	_A("URRe"), _N,				_A("URCRe"),    _A("URSwRe"),   _N);
	m_AniRifleSelect			= AniPlayerUpper				(false,	_A("URSe"), _N,				_A("URCRe"),    _A("URSwRe"),   _N);
	m_AniRifleDeselect			= AniPlayerUpper				(false,	_A("URDe"), _N,				_A("URCRe"),    _A("URSwRe"),   _N);
	m_AniRifleAimLeanR			= AniPlayerUpper				(false,	_A("URAmR"),_N,				_A("URCAmR"),	_N,				_N);
	m_AniRifleAimLeanL			= AniPlayerUpper				(false,	_A("URAmL"),_N,				_A("URCAmL"),	_N,				_N);
	m_AniRifleFireLeanR			= AniPlayerUpper				(false,	_A("URFiR"),_N,				_A("URCFiR"),	_N,				_N);
	m_AniRifleFireLeanL			= AniPlayerUpper				(false,	_A("URFiL"),_N,				_A("URCFiL"),	_N,				_N);

	m_AniPistolUnalert          = AniPlayerUpper           (true,     _A("UPUn"), _N,             _N,             _N,             _N);
	m_AniPistolAlert            = AniPlayerUpper           (true,     _A("UPAl"), _N,             _N,             _N,             _N);
	m_AniPistolAim              = AniPlayerUpper           (true,     _A("UPAm"), _A("UPSAm"),    _A("UPCAm"),    _N,             _A("UPCSAm"));
	m_AniPistolFire             = AniPlayerUpper           (false,    _A("UPFi"), _A("UPSFi"),    _A("UPCFi"),    _N,             _A("UPCSFi"));
	m_AniPistolReload           = AniPlayerUpper           (false,    _A("UPRe"), _N,             _A("UPCRe"),    _N,             _N);
	m_AniPistolSelect           = AniPlayerUpper           (false,    _A("UPSe"), _N,             _A("UPCRe"),    _N,             _N);
	m_AniPistolDeselect         = AniPlayerUpper           (false,    _A("UPDe"), _N,             _A("UPCRe"),    _N,             _N);
	m_AniPistolAimLeanR			= AniPlayerUpper           (false,	_A("UPAmR"),_N,				_A("UPCAmR"),	_N,				_N);
	m_AniPistolAimLeanL			= AniPlayerUpper           (false,	_A("UPAmL"),_N,				_A("UPCAmL"),	_N,				_N);
	m_AniPistolFireLeanR		= AniPlayerUpper           (false,	_A("UPFiR"),_N,				_A("UPCFiR"),	_N,				_N);
	m_AniPistolFireLeanL		= AniPlayerUpper           (false,	_A("UPFiL"),_N,				_A("UPCFiL"),	_N,				_N);

	m_AniMeleeUnalert           = AniPlayerUpper           (true,     _A("UMUn"), _N,             _N,             _N,             _N);
	m_AniMeleeAlert             = AniPlayerUpper           (true,     _A("UMAl"), _N,             _N,             _N,             _N);
	m_AniMeleeAim               = AniPlayerUpper           (true,     _A("UMAm"), _A("UMSAm"),    _A("UMCAm"),    _N,             _A("UMCSAm"));
	m_AniMeleeFire              = AniPlayerUpper           (false,    _A("UMFi"), _A("UMSFi"),    _A("UMCFi"),    _N,             _A("UMCSFi"));
	m_AniMeleeFire2				= AniPlayerUpper		   (false,	_A("UMFi2"),_A("UMSFi2"),	_A("UMCFi2"),	_N,				_A("UMCSFi"));
	m_AniMeleeFire3				= AniPlayerUpper		   (false,	_A("UMFi3"),_A("UMSFi"),	_A("UMCFi"),	_N,				_A("UMCSFi"));
	m_AniMeleeReload            = AniPlayerUpper           (false,    _A("UMRe"), _N,             _A("UMCRe"),    _N,             _N);
	m_AniMeleeSelect            = AniPlayerUpper           (false,    _A("UMSe"), _N,             _A("UMCRe"),    _N,             _N);
	m_AniMeleeDeselect          = AniPlayerUpper           (false,    _A("UMDe"), _N,             _A("UMCRe"),    _N,             _N);
	m_AniMeleeAimLeanR			= AniPlayerUpper           (false,	_A("UMAmR"),_N,				_A("UMCAmR"),	_N,				_N);
	m_AniMeleeAimLeanL			= AniPlayerUpper           (false,	_A("UMAmL"),_N,				_A("UMCAmL"),	_N,				_N);
	m_AniMeleeFireLeanR			= AniPlayerUpper           (false,	_A("UMFiR"),_N,				_A("UMCFiR"),	_N,				_N);
	m_AniMeleeFireLeanL			= AniPlayerUpper           (false,	_A("UMFiL"),_N,				_A("UMCFiL"),	_N,				_N);

	m_AniThrowUnalert           = AniPlayerUpper           (true,     _A("UTUn"), _N,             _N,             _N,             _N);
	m_AniThrowAlert             = AniPlayerUpper           (true,     _A("UTAl"), _N,             _N,             _N,             _N);
	m_AniThrowAim               = AniPlayerUpper           (true,     _A("UTAm"), _A("UTSAm"),    _A("UTCAm"),    _N,             _A("UTCSAm"));
	m_AniThrowFire              = AniPlayerUpper           (false,    _A("UTFi"), _A("UTSFi"),    _A("UTCFi"),    _N,             _A("UTCSFi"));
	m_AniThrowReload            = AniPlayerUpper           (false,    _A("UTRe"), _N,             _A("UTCRe"),    _N,             _N);
	m_AniThrowSelect            = AniPlayerUpper           (false,    _A("UTSe"), _N,             _A("UTCRe"),    _N,             _N);
	m_AniThrowDeselect          = AniPlayerUpper           (false,    _A("UTDe"), _N,             _A("UTCRe"),    _N,             _N);
	m_AniThrowAimLeanR			= AniPlayerUpper           (false,	_A("UTAmR"),_N,				_A("UTCAmR"),	_N,				_N);
	m_AniThrowAimLeanL			= AniPlayerUpper           (false,	_A("UTAmL"),_N,				_A("UTCAmL"),	_N,				_N);
	m_AniThrowFireLeanR			= AniPlayerUpper           (false,	_A("UTFiR"),_N,				_A("UTCFiR"),	_N,				_N);
	m_AniThrowFireLeanL			= AniPlayerUpper           (false,	_A("UTFiL"),_N,				_A("UTCFiL"),	_N,				_N);

	m_AniRifle2Unalert          = AniPlayerUpper           (true,     _A("UR2Un"), _N,             _N,            _N,             _N);
	m_AniRifle2Alert            = AniPlayerUpper           (true,     _A("UR2Al"), _N,             _N,            _N,             _N);
	m_AniRifle2Aim              = AniPlayerUpper           (true,     _A("UR2Am"), _A("UR2SAm"),   _A("UR2CAm"),  _N,             _A("UR2CSAm"));
	m_AniRifle2Fire             = AniPlayerUpper           (false,    _A("UR2Fi"), _A("UR2SFi"),   _A("UR2CFi"),  _N,             _A("UR2CSFi"));
	m_AniRifle2Reload           = AniPlayerUpper           (false,    _A("UR2Re"), _N,             _A("UR2CRe"),  _N,             _N);
	m_AniRifle2Select           = AniPlayerUpper           (false,    _A("UR2Se"), _N,             _A("UR2CRe"),  _N,             _N);
	m_AniRifle2Deselect         = AniPlayerUpper           (false,    _A("UR2De"), _N,             _A("UR2CRe"),  _N,             _N);
	m_AniRifle2AimLeanR			= AniPlayerUpper           (false,	_A("UR2AmR"),_N,			 _A("UR2CAmR"),	_N,				_N);
	m_AniRifle2AimLeanL			= AniPlayerUpper           (false,	_A("UR2AmL"),_N,			 _A("UR2CAmL"),	_N,				_N);
	m_AniRifle2FireLeanR		= AniPlayerUpper           (false,	_A("UR2FiR"),_N,			 _A("UR2CFiR"),	_N,				_N);
	m_AniRifle2FireLeanL		= AniPlayerUpper           (false,	_A("UR2FiL"),_N,			 _A("UR2CFiL"),	_N,				_N);

	m_AniRifle3Unalert          = AniPlayerUpper           (true,     _A("UR3Un"), _N,             _N,            _N,             _N);
	m_AniRifle3Alert            = AniPlayerUpper           (true,     _A("UR3Al"), _N,             _N,            _N,             _N);
	m_AniRifle3Aim              = AniPlayerUpper           (true,     _A("UR3Am"), _A("UR3SAm"),   _A("UR3CAm"),  _N,             _A("UR3CSAm"));
	m_AniRifle3Fire             = AniPlayerUpper           (false,    _A("UR3Fi"), _A("UR3SFi"),   _A("UR3CFi"),  _N,             _A("UR3CSFi"));
	m_AniRifle3Reload           = AniPlayerUpper           (false,    _A("UR3Re"), _N,             _A("UR3CRe"),  _N,             _N);
	m_AniRifle3Select           = AniPlayerUpper           (false,    _A("UR3Se"), _N,             _A("UR3CRe"),  _N,             _N);
	m_AniRifle3Deselect         = AniPlayerUpper           (false,    _A("UR3De"), _N,             _A("UR3CRe"),  _N,             _N);
	m_AniRifle3AimLeanR			= AniPlayerUpper           (false,	_A("UR3AmR"),_N,			 _A("UR3CAmR"),	_N,				_N);
	m_AniRifle3AimLeanL			= AniPlayerUpper           (false,	_A("UR3AmL"),_N,			 _A("UR3CAmL"),	_N,				_N);
	m_AniRifle3FireLeanR		= AniPlayerUpper           (false,	_A("UR3FiR"),_N,			 _A("UR3CFiR"),	_N,				_N);
	m_AniRifle3FireLeanL		= AniPlayerUpper           (false,	_A("UR3FiL"),_N,			 _A("UR3CFiL"),	_N,				_N);

	m_AniHolsterUnalert         = AniPlayerUpper           (true,     _A("UHUn"), _N,             _N,				_N,             _N);
	m_AniHolsterAlert           = AniPlayerUpper           (true,     _A("UHAl"), _N,             _N,				_N,             _N);
	m_AniHolsterAim             = AniPlayerUpper           (true,     _A("UHAm"), _A("UHSAm"),	_A("UHCAm"),	_N,             _A("UHCSAm"));
	m_AniHolsterFire            = AniPlayerUpper           (false,    _A("UHFi"), _A("UHSFi"),	_A("UHCFi"),	_N,             _A("UHCSFi"));
	m_AniHolsterReload          = AniPlayerUpper           (false,    _A("UHRe"), _N,             _A("UHCRe"),	_N,             _N);
	m_AniHolsterSelect          = AniPlayerUpper           (false,    _A("UHSe"), _N,             _A("UHCRe"),	_N,             _N);
	m_AniHolsterDeselect        = AniPlayerUpper           (false,    _A("UHDe"), _N,             _A("UHCRe"),	_N,             _N);
	m_AniHolsterAimLeanR		= AniPlayerUpper           (false,	_A("UHAmR"),_N,				_A("UHCAmR"),	_N,				_N);
	m_AniHolsterAimLeanL		= AniPlayerUpper           (false,	_A("UHAmL"),_N,				_A("UHCAmL"),	_N,				_N);
	m_AniHolsterFireLeanR		= AniPlayerUpper           (false,	_A("UHFiR"),_N,				_A("UHCFiR"),	_N,				_N);
	m_AniHolsterFireLeanL		= AniPlayerUpper           (false,	_A("UHFiL"),_N,				_A("UHCFiL"),	_N,				_N);

	m_AniPlaceUnalert			= AniPlayerUpper           (true,     _A("UPlUn"), _N,            _N,				_N,             _N);
	m_AniPlaceAlert				= AniPlayerUpper           (true,     _A("UPlAl"), _N,            _N,				_N,             _N);
	m_AniPlaceAim				= AniPlayerUpper           (true,     _A("UPlAm"), _A("UPlSAm"),	_A("UPlCAm"),	_N,             _A("UPlCSAm"));
	m_AniPlaceFire				= AniPlayerUpper           (false,    _A("UPlFi"), _A("UPlSFi"),	_A("UPlCFi"),	_N,             _A("UPlCSFi"));
	m_AniPlaceReload			= AniPlayerUpper           (false,    _A("UPlRe"), _N,            _A("UPlCRe"),	_N,             _N);
	m_AniPlaceSelect			= AniPlayerUpper           (false,    _A("UPlSe"), _N,            _A("UPlCRe"),	_N,             _N);
	m_AniPlaceDeselect			= AniPlayerUpper           (false,    _A("UPlDe"), _N,            _A("UPlCRe"),	_N,             _N);
	m_AniPlaceAimLeanR			= AniPlayerUpper           (false,	_A("UPlAmR"),_N,			_A("UPlCAmR"),	_N,				_N);
	m_AniPlaceAimLeanL			= AniPlayerUpper           (false,	_A("UPlAmL"),_N,			_A("UPlCAmL"),	_N,				_N);
	m_AniPlaceFireLeanR			= AniPlayerUpper           (false,	_A("UPlFiR"),_N,			_A("UPlCFiR"),	_N,				_N);
	m_AniPlaceFireLeanL			= AniPlayerUpper           (false,	_A("UPlFiL"),_N,			_A("UPlCFiL"),	_N,				_N);

	m_AniGadgetUnalert			= AniPlayerUpper           (true,     _A("UGUn"), _N,				_N,				_N,             _N);
	m_AniGadgetAlert			= AniPlayerUpper           (true,     _A("UGAl"), _N,				_N,				_N,             _N);
	m_AniGadgetAim				= AniPlayerUpper           (true,     _A("UGAm"), _A("UGSAm"),	_A("UGCAm"),	_N,             _A("UGCSAm"));
	m_AniGadgetFire				= AniPlayerUpper           (false,    _A("UGFi"), _A("UGSFi"),	_A("UGCFi"),	_N,             _A("UGCSFi"));
	m_AniGadgetReload			= AniPlayerUpper           (false,    _A("UGRe"), _N,				_A("UGCRe"),	_N,             _N);
	m_AniGadgetSelect			= AniPlayerUpper           (false,    _A("UGSe"), _N,				_A("UGCRe"),	_N,             _N);
	m_AniGadgetDeselect			= AniPlayerUpper           (false,    _A("UGDe"), _N,				_A("UGCRe"),	_N,             _N);
	m_AniGadgetAimLeanR			= AniPlayerUpper           (false,	_A("UGAmR"),_N,				_A("UGCAmR"),	_N,				_N);
	m_AniGadgetAimLeanL			= AniPlayerUpper           (false,	_A("UGAmL"),_N,				_A("UGCAmL"),	_N,				_N);
	m_AniGadgetFireLeanR		= AniPlayerUpper           (false,	_A("UGFiR"),_N,				_A("UGCFiR"),	_N,				_N);
	m_AniGadgetFireLeanL		= AniPlayerUpper           (false,	_A("UGFiL"),_N,				_A("UGCFiL"),	_N,				_N);

	m_AniStarsUnalert			= AniPlayerUpper           (true,     _A("USUn"), _N,				_N,				_N,             _N);
	m_AniStarsAlert				= AniPlayerUpper           (true,     _A("USAl"), _N,				_N,				_N,             _N);
	m_AniStarsAim				= AniPlayerUpper           (true,     _A("USAm"), _A("USSAm"),	_A("USCAm"),	_N,             _A("USCSAm"));
	m_AniStarsFire				= AniPlayerUpper           (false,    _A("USFi"), _A("USSFi"),	_A("USCFi"),	_N,             _A("USCSFi"));
	m_AniStarsReload			= AniPlayerUpper           (false,    _A("USRe"), _N,				_A("USCRe"),	_N,             _N);
	m_AniStarsSelect			= AniPlayerUpper           (false,    _A("USSe"), _N,				_A("USCRe"),	_N,             _N);
	m_AniStarsDeselect			= AniPlayerUpper           (false,    _A("USDe"), _N,				_A("USCRe"),	_N,             _N);
	m_AniStarsAimLeanR			= AniPlayerUpper           (false,	_A("USAmR"),_N,				_A("USCAmR"),	_N,				_N);
	m_AniStarsAimLeanL			= AniPlayerUpper           (false,	_A("USAmL"),_N,				_A("USCAmL"),	_N,				_N);
	m_AniStarsFireLeanR			= AniPlayerUpper           (false,	_A("USFiR"),_N,				_A("USCFiR"),	_N,				_N);
	m_AniStarsFireLeanL			= AniPlayerUpper           (false,	_A("USFiL"),_N,				_A("USCFiL"),	_N,				_N);

	// Get Main anis

	m_AniMainBase               = AniPlayerMain            (true,     _A("Base"),			_N,					_N,					_N,					_N);

	m_AniClimb                  = AniPlayerMain            (true,     _A("Cl"),			_N,					_N,					_N,					_N);
	m_AniClimbUp                = AniPlayerMain            (true,     _A("ClU"),			_N,					_N,					_N,					_N);
	m_AniClimbDown              = AniPlayerMain            (true,     _A("ClD"),			_N,					_N,					_N,					_N);
	m_AniSnowmobile             = AniPlayerMain            (true,     _A("LRdS"),			_N,					_N,					_N,					_N);
	
	m_AniDT_Slippery			= AniPlayerMain			   (false,	_A("banana_idle"),	_N,					_N,					_A("banana_in"),	_A("banana_out"));
	m_AniDT_Sleeping			= AniPlayerMain			   (false,	_A("KnockOut_idle"),_N,					_N,					_A("KnockOut_in"),	_A("KnockOut_out"));
	m_AniDT_Stun				= AniPlayerMain			   (false,	_A("Stun_idle"),	_N,					_N,					_A("Stun_in"),		_A("Stun_out"));
	m_AniDT_Laughing			= AniPlayerMain			   (false,	_A("Laugh_idle"),	_A("Laugh_1"),		_A("Laugh_2"),		_A("Laugh_in"),		_A("Laugh_out"));
	m_AniDT_BearTrap			= AniPlayerMain			   (false,	_A("BearTrap_idle"),_A("BearTrap_1"),	_A("BearTrap_2"),	_A("BearTrap_in"),	_A("BearTrap_out"));
	m_AniDT_Glue				= AniPlayerMain			   (false,	_A("BearTrap_idle"),_A("Glue_1"),		_A("Glue_2"),		_A("Glue_in"),		_A("Glue_out"));
	m_AniDT_Hurt				= AniPlayerMain			   (false,	_A("Bleed"),		_N,					_N,					_N,					_N);
	m_AniDT_HurtWalk			= AniPlayerMain			   (false,	_A("WalkBleed"),	_N,					_N,					_N,					_N);


	// Setup our lookup tables

	memset(m_aapAniPlayerUppers, 0, sizeof(void*) * kNumWeapons * kNumPostures * kNumLeans);
	memset(m_aapAniPlayerLowers, 0, sizeof(void*) * kNumMovements * kNumDirections * kNumLeans);
	memset(m_apAniPlayerMains, 0, sizeof(void*) * kNumMains);


	// [eRifle] [Posture] [Lean]

	m_aapAniPlayerUppers[eRifle][eUnalert][eCenter]		= &m_AniRifleUnalert;
	m_aapAniPlayerUppers[eRifle][eUnalert][eLeft]		= &m_AniRifleUnalert;
	m_aapAniPlayerUppers[eRifle][eUnalert][eRight]		= &m_AniRifleUnalert;
														
	m_aapAniPlayerUppers[eRifle][eAlert][eCenter]		= &m_AniRifleAlert;
	m_aapAniPlayerUppers[eRifle][eAlert][eLeft]			= &m_AniRifleAlert;
	m_aapAniPlayerUppers[eRifle][eAlert][eRight]		= &m_AniRifleAlert;
														
	m_aapAniPlayerUppers[eRifle][eAim][eCenter]			= &m_AniRifleAim;
	m_aapAniPlayerUppers[eRifle][eAim][eLeft]			= &m_AniRifleAimLeanL;
	m_aapAniPlayerUppers[eRifle][eAim][eRight]			= &m_AniRifleAimLeanR;
														
	m_aapAniPlayerUppers[eRifle][eFire][eCenter]		= &m_AniRifleFire;
	m_aapAniPlayerUppers[eRifle][eFire][eLeft]			= &m_AniRifleFireLeanL;
	m_aapAniPlayerUppers[eRifle][eFire][eRight]			= &m_AniRifleFireLeanR;
														
	m_aapAniPlayerUppers[eRifle][eFire2][eCenter]		= &m_AniRifleFire;
	m_aapAniPlayerUppers[eRifle][eFire2][eLeft]			= &m_AniRifleFireLeanL;
	m_aapAniPlayerUppers[eRifle][eFire2][eRight]		= &m_AniRifleFireLeanR;
														
	m_aapAniPlayerUppers[eRifle][eFire3][eCenter]		= &m_AniRifleFire;
	m_aapAniPlayerUppers[eRifle][eFire3][eLeft]			= &m_AniRifleFireLeanL;
	m_aapAniPlayerUppers[eRifle][eFire3][eRight]		= &m_AniRifleFireLeanR;
														
	m_aapAniPlayerUppers[eRifle][eReload][eCenter]		= &m_AniRifleReload;
	m_aapAniPlayerUppers[eRifle][eReload][eLeft]		= &m_AniRifleReload;
	m_aapAniPlayerUppers[eRifle][eReload][eRight]		= &m_AniRifleReload;
														
	m_aapAniPlayerUppers[eRifle][eSelect][eCenter]		= &m_AniRifleSelect;
	m_aapAniPlayerUppers[eRifle][eSelect][eLeft]		= &m_AniRifleSelect;
	m_aapAniPlayerUppers[eRifle][eSelect][eRight]		= &m_AniRifleSelect;
														
	m_aapAniPlayerUppers[eRifle][eDeselect][eCenter]	= &m_AniRifleDeselect;
	m_aapAniPlayerUppers[eRifle][eDeselect][eLeft]		= &m_AniRifleDeselect;
	m_aapAniPlayerUppers[eRifle][eDeselect][eRight]		= &m_AniRifleDeselect;
														
	// [ePistol] [Posture] [Lean]

	m_aapAniPlayerUppers[ePistol][eUnalert][eCenter]	= &m_AniPistolUnalert;
	m_aapAniPlayerUppers[ePistol][eUnalert][eLeft]		= &m_AniPistolUnalert;
	m_aapAniPlayerUppers[ePistol][eUnalert][eRight]		= &m_AniPistolUnalert;
														
	m_aapAniPlayerUppers[ePistol][eAlert][eCenter]		= &m_AniPistolAlert;
	m_aapAniPlayerUppers[ePistol][eAlert][eLeft]		= &m_AniPistolAlert;
	m_aapAniPlayerUppers[ePistol][eAlert][eRight]		= &m_AniPistolAlert;
														
	m_aapAniPlayerUppers[ePistol][eAim][eCenter]		= &m_AniPistolAim;
	m_aapAniPlayerUppers[ePistol][eAim][eLeft]			= &m_AniPistolAimLeanL;
	m_aapAniPlayerUppers[ePistol][eAim][eRight]			= &m_AniPistolAimLeanR;
														
	m_aapAniPlayerUppers[ePistol][eFire][eCenter]		= &m_AniPistolFire;
	m_aapAniPlayerUppers[ePistol][eFire][eLeft]			= &m_AniPistolFireLeanL;
	m_aapAniPlayerUppers[ePistol][eFire][eRight]		= &m_AniPistolFireLeanR;
														
	m_aapAniPlayerUppers[ePistol][eFire2][eCenter]		= &m_AniPistolFire;
	m_aapAniPlayerUppers[ePistol][eFire2][eLeft]		= &m_AniPistolFireLeanL;
	m_aapAniPlayerUppers[ePistol][eFire2][eRight]		= &m_AniPistolFireLeanR;
														
	m_aapAniPlayerUppers[ePistol][eFire3][eCenter]		= &m_AniPistolFire;
	m_aapAniPlayerUppers[ePistol][eFire3][eLeft]		= &m_AniPistolFireLeanL;
	m_aapAniPlayerUppers[ePistol][eFire3][eRight]		= &m_AniPistolFireLeanR;
														
	m_aapAniPlayerUppers[ePistol][eReload][eCenter]		= &m_AniPistolReload;
	m_aapAniPlayerUppers[ePistol][eReload][eLeft]		= &m_AniPistolReload;
	m_aapAniPlayerUppers[ePistol][eReload][eRight]		= &m_AniPistolReload;
														
	m_aapAniPlayerUppers[ePistol][eSelect][eCenter]		= &m_AniPistolSelect;
	m_aapAniPlayerUppers[ePistol][eSelect][eLeft]		= &m_AniPistolSelect;
	m_aapAniPlayerUppers[ePistol][eSelect][eRight]		= &m_AniPistolSelect;

	m_aapAniPlayerUppers[ePistol][eDeselect][eCenter]	= &m_AniPistolDeselect;
	m_aapAniPlayerUppers[ePistol][eDeselect][eLeft]		= &m_AniPistolDeselect;
	m_aapAniPlayerUppers[ePistol][eDeselect][eRight]	= &m_AniPistolDeselect;

	// [eMelee] [Posture] [Lean]

	m_aapAniPlayerUppers[eMelee][eUnalert][eCenter]		= &m_AniMeleeUnalert;
	m_aapAniPlayerUppers[eMelee][eUnalert][eLeft]		= &m_AniMeleeUnalert;
	m_aapAniPlayerUppers[eMelee][eUnalert][eRight]		= &m_AniMeleeUnalert;
														
	m_aapAniPlayerUppers[eMelee][eAlert][eCenter]		= &m_AniMeleeAlert;
	m_aapAniPlayerUppers[eMelee][eAlert][eLeft]			= &m_AniMeleeAlert;
	m_aapAniPlayerUppers[eMelee][eAlert][eRight]		= &m_AniMeleeAlert;
														
	m_aapAniPlayerUppers[eMelee][eAim][eCenter]			= &m_AniMeleeAim;
	m_aapAniPlayerUppers[eMelee][eAim][eLeft]			= &m_AniMeleeAimLeanL;
	m_aapAniPlayerUppers[eMelee][eAim][eRight]			= &m_AniMeleeAimLeanR;
														
	m_aapAniPlayerUppers[eMelee][eFire][eCenter]		= &m_AniMeleeFire;
	m_aapAniPlayerUppers[eMelee][eFire][eLeft]			= &m_AniMeleeFireLeanL;
	m_aapAniPlayerUppers[eMelee][eFire][eRight]			= &m_AniMeleeFireLeanR;
														
	m_aapAniPlayerUppers[eMelee][eFire2][eCenter]		= &m_AniMeleeFire2;
	m_aapAniPlayerUppers[eMelee][eFire2][eLeft]			= &m_AniMeleeFireLeanL;
	m_aapAniPlayerUppers[eMelee][eFire2][eRight]		= &m_AniMeleeFireLeanR;
														
	m_aapAniPlayerUppers[eMelee][eFire3][eCenter]		= &m_AniMeleeFire3;
	m_aapAniPlayerUppers[eMelee][eFire3][eLeft]			= &m_AniMeleeFireLeanL;
	m_aapAniPlayerUppers[eMelee][eFire3][eRight]		= &m_AniMeleeFireLeanR;
														
	m_aapAniPlayerUppers[eMelee][eReload][eCenter]		= &m_AniMeleeReload;
	m_aapAniPlayerUppers[eMelee][eReload][eLeft]		= &m_AniMeleeReload;
	m_aapAniPlayerUppers[eMelee][eReload][eRight]		= &m_AniMeleeReload;
														
	m_aapAniPlayerUppers[eMelee][eSelect][eCenter]		= &m_AniMeleeSelect;
	m_aapAniPlayerUppers[eMelee][eSelect][eLeft]		= &m_AniMeleeSelect;
	m_aapAniPlayerUppers[eMelee][eSelect][eRight]		= &m_AniMeleeSelect;
														
	m_aapAniPlayerUppers[eMelee][eDeselect][eCenter]	= &m_AniMeleeDeselect;
	m_aapAniPlayerUppers[eMelee][eDeselect][eLeft]		= &m_AniMeleeDeselect;
	m_aapAniPlayerUppers[eMelee][eDeselect][eRight]		= &m_AniMeleeDeselect;
														
	// [eThrow] [Posture] [Lean]

	m_aapAniPlayerUppers[eThrow][eUnalert][eCenter]		= &m_AniThrowUnalert;
	m_aapAniPlayerUppers[eThrow][eUnalert][eLeft]		= &m_AniThrowUnalert;
	m_aapAniPlayerUppers[eThrow][eUnalert][eRight]		= &m_AniThrowUnalert;
														
	m_aapAniPlayerUppers[eThrow][eAlert][eCenter]		= &m_AniThrowAlert;
	m_aapAniPlayerUppers[eThrow][eAlert][eLeft]			= &m_AniThrowAlert;
	m_aapAniPlayerUppers[eThrow][eAlert][eRight]		= &m_AniThrowAlert;
														
	m_aapAniPlayerUppers[eThrow][eAim][eCenter]			= &m_AniThrowAim;
	m_aapAniPlayerUppers[eThrow][eAim][eLeft]			= &m_AniThrowAimLeanL;
	m_aapAniPlayerUppers[eThrow][eAim][eRight]			= &m_AniThrowAimLeanR;
														
	m_aapAniPlayerUppers[eThrow][eFire][eCenter]		= &m_AniThrowFire;
	m_aapAniPlayerUppers[eThrow][eFire][eLeft]			= &m_AniThrowFireLeanL;
	m_aapAniPlayerUppers[eThrow][eFire][eRight]			= &m_AniThrowFireLeanR;
														
	m_aapAniPlayerUppers[eThrow][eFire2][eCenter]		= &m_AniThrowFire;
	m_aapAniPlayerUppers[eThrow][eFire2][eLeft]			= &m_AniThrowFireLeanL;
	m_aapAniPlayerUppers[eThrow][eFire2][eRight]		= &m_AniThrowFireLeanR;
														
	m_aapAniPlayerUppers[eThrow][eFire3][eCenter]		= &m_AniThrowFire;
	m_aapAniPlayerUppers[eThrow][eFire3][eLeft]			= &m_AniThrowFireLeanL;
	m_aapAniPlayerUppers[eThrow][eFire3][eRight]		= &m_AniThrowFireLeanR;
														
	m_aapAniPlayerUppers[eThrow][eReload][eCenter]		= &m_AniThrowReload;
	m_aapAniPlayerUppers[eThrow][eReload][eLeft]		= &m_AniThrowReload;
	m_aapAniPlayerUppers[eThrow][eReload][eRight]		= &m_AniThrowReload;
														
	m_aapAniPlayerUppers[eThrow][eSelect][eCenter]		= &m_AniThrowSelect;
	m_aapAniPlayerUppers[eThrow][eSelect][eLeft]		= &m_AniThrowSelect;
	m_aapAniPlayerUppers[eThrow][eSelect][eRight]		= &m_AniThrowSelect;
														
	m_aapAniPlayerUppers[eThrow][eDeselect][eCenter]	= &m_AniThrowDeselect;
	m_aapAniPlayerUppers[eThrow][eDeselect][eLeft]		= &m_AniThrowDeselect;
	m_aapAniPlayerUppers[eThrow][eDeselect][eRight]		= &m_AniThrowDeselect;
														
	// [eRifle2] [Posture] [Lean]

	m_aapAniPlayerUppers[eRifle2][eUnalert][eCenter]	= &m_AniRifle2Unalert;
	m_aapAniPlayerUppers[eRifle2][eUnalert][eLeft]		= &m_AniRifle2Unalert;
	m_aapAniPlayerUppers[eRifle2][eUnalert][eRight]		= &m_AniRifle2Unalert;
	
	m_aapAniPlayerUppers[eRifle2][eAlert][eCenter]		= &m_AniRifle2Alert;
	m_aapAniPlayerUppers[eRifle2][eAlert][eLeft]		= &m_AniRifle2Alert;
	m_aapAniPlayerUppers[eRifle2][eAlert][eRight]		= &m_AniRifle2Alert;
														
	m_aapAniPlayerUppers[eRifle2][eAim][eCenter]		= &m_AniRifle2Aim;
	m_aapAniPlayerUppers[eRifle2][eAim][eLeft]			= &m_AniRifle2AimLeanL;
	m_aapAniPlayerUppers[eRifle2][eAim][eRight]			= &m_AniRifle2AimLeanR;
														
	m_aapAniPlayerUppers[eRifle2][eFire][eCenter]		= &m_AniRifle2Fire;
	m_aapAniPlayerUppers[eRifle2][eFire][eLeft]			= &m_AniRifle2FireLeanL;
	m_aapAniPlayerUppers[eRifle2][eFire][eRight]		= &m_AniRifle2FireLeanR;
														
	m_aapAniPlayerUppers[eRifle2][eFire2][eCenter]		= &m_AniRifle2Fire;
	m_aapAniPlayerUppers[eRifle2][eFire2][eLeft]		= &m_AniRifle2FireLeanL;
	m_aapAniPlayerUppers[eRifle2][eFire2][eRight]		= &m_AniRifle2FireLeanR;
														
	m_aapAniPlayerUppers[eRifle2][eFire3][eCenter]		= &m_AniRifle2Fire;
	m_aapAniPlayerUppers[eRifle2][eFire3][eLeft]		= &m_AniRifle2FireLeanL;
	m_aapAniPlayerUppers[eRifle2][eFire3][eRight]		= &m_AniRifle2FireLeanR;
														
	m_aapAniPlayerUppers[eRifle2][eReload][eCenter]		= &m_AniRifle2Reload;
	m_aapAniPlayerUppers[eRifle2][eReload][eLeft]		= &m_AniRifle2Reload;
	m_aapAniPlayerUppers[eRifle2][eReload][eRight]		= &m_AniRifle2Reload;
														
	m_aapAniPlayerUppers[eRifle2][eSelect][eCenter]		= &m_AniRifle2Select;
	m_aapAniPlayerUppers[eRifle2][eSelect][eLeft]		= &m_AniRifle2Select;
	m_aapAniPlayerUppers[eRifle2][eSelect][eRight]		= &m_AniRifle2Select;

	m_aapAniPlayerUppers[eRifle2][eDeselect][eCenter]	= &m_AniRifle2Deselect;
	m_aapAniPlayerUppers[eRifle2][eDeselect][eLeft]		= &m_AniRifle2Deselect;
	m_aapAniPlayerUppers[eRifle2][eDeselect][eRight]	= &m_AniRifle2Deselect;

	// [eRifle3] [Posture] [Lean]

	m_aapAniPlayerUppers[eRifle3][eUnalert][eCenter]	= &m_AniRifle2Unalert;
	m_aapAniPlayerUppers[eRifle3][eUnalert][eLeft]		= &m_AniRifle2Unalert;
	m_aapAniPlayerUppers[eRifle3][eUnalert][eRight]		= &m_AniRifle2Unalert;
	
	m_aapAniPlayerUppers[eRifle3][eAlert][eCenter]		= &m_AniRifle3Alert;
	m_aapAniPlayerUppers[eRifle3][eAlert][eLeft]		= &m_AniRifle3Alert;
	m_aapAniPlayerUppers[eRifle3][eAlert][eRight]		= &m_AniRifle3Alert;
														
	m_aapAniPlayerUppers[eRifle3][eAim][eCenter]		= &m_AniRifle3Aim;
	m_aapAniPlayerUppers[eRifle3][eAim][eLeft]			= &m_AniRifle3AimLeanL;
	m_aapAniPlayerUppers[eRifle3][eAim][eRight]			= &m_AniRifle3AimLeanR;
														
	m_aapAniPlayerUppers[eRifle3][eFire][eCenter]		= &m_AniRifle3Fire;
	m_aapAniPlayerUppers[eRifle3][eFire][eLeft]			= &m_AniRifle3FireLeanL;
	m_aapAniPlayerUppers[eRifle3][eFire][eRight]		= &m_AniRifle3FireLeanR;
														
	m_aapAniPlayerUppers[eRifle3][eFire2][eCenter]		= &m_AniRifle3Fire;
	m_aapAniPlayerUppers[eRifle3][eFire2][eLeft]		= &m_AniRifle3FireLeanL;
	m_aapAniPlayerUppers[eRifle3][eFire2][eRight]		= &m_AniRifle3FireLeanR;
														
	m_aapAniPlayerUppers[eRifle3][eFire3][eCenter]		= &m_AniRifle3Fire;
	m_aapAniPlayerUppers[eRifle3][eFire3][eLeft]		= &m_AniRifle3FireLeanL;
	m_aapAniPlayerUppers[eRifle3][eFire3][eRight]		= &m_AniRifle3FireLeanR;
														
	m_aapAniPlayerUppers[eRifle3][eReload][eCenter]		= &m_AniRifle3Reload;
	m_aapAniPlayerUppers[eRifle3][eReload][eLeft]		= &m_AniRifle3Reload;
	m_aapAniPlayerUppers[eRifle3][eReload][eRight]		= &m_AniRifle3Reload;
														
	m_aapAniPlayerUppers[eRifle3][eSelect][eCenter]		= &m_AniRifle3Select;
	m_aapAniPlayerUppers[eRifle3][eSelect][eLeft]		= &m_AniRifle3Select;
	m_aapAniPlayerUppers[eRifle3][eSelect][eRight]		= &m_AniRifle3Select;

	m_aapAniPlayerUppers[eRifle3][eDeselect][eCenter]	= &m_AniRifle3Deselect;
	m_aapAniPlayerUppers[eRifle3][eDeselect][eLeft]		= &m_AniRifle3Deselect;
	m_aapAniPlayerUppers[eRifle3][eDeselect][eRight]	= &m_AniRifle3Deselect;

	// [eHolster] [Posture] [Lean]

	m_aapAniPlayerUppers[eHolster][eUnalert][eCenter]	= &m_AniHolsterUnalert;
	m_aapAniPlayerUppers[eHolster][eUnalert][eLeft]		= &m_AniHolsterUnalert;
	m_aapAniPlayerUppers[eHolster][eUnalert][eRight]	= &m_AniHolsterUnalert;
	
	m_aapAniPlayerUppers[eHolster][eAlert][eCenter]		= &m_AniHolsterAlert;
	m_aapAniPlayerUppers[eHolster][eAlert][eLeft]		= &m_AniHolsterAlert;
	m_aapAniPlayerUppers[eHolster][eAlert][eRight]		= &m_AniHolsterAlert;
	
	m_aapAniPlayerUppers[eHolster][eAim][eCenter]		= &m_AniHolsterAim;
	m_aapAniPlayerUppers[eHolster][eAim][eLeft]			= &m_AniHolsterAimLeanL;
	m_aapAniPlayerUppers[eHolster][eAim][eRight]		= &m_AniHolsterAimLeanR;
	
	m_aapAniPlayerUppers[eHolster][eFire][eCenter]		= &m_AniHolsterFire;
	m_aapAniPlayerUppers[eHolster][eFire][eLeft]		= &m_AniHolsterFireLeanL;
	m_aapAniPlayerUppers[eHolster][eFire][eRight]		= &m_AniHolsterFireLeanR;

	m_aapAniPlayerUppers[eHolster][eFire2][eCenter]		= &m_AniHolsterFire;
	m_aapAniPlayerUppers[eHolster][eFire2][eLeft]		= &m_AniHolsterFireLeanL;
	m_aapAniPlayerUppers[eHolster][eFire2][eRight]		= &m_AniHolsterFireLeanR;

	m_aapAniPlayerUppers[eHolster][eFire3][eCenter]		= &m_AniHolsterFire;
	m_aapAniPlayerUppers[eHolster][eFire3][eLeft]		= &m_AniHolsterFireLeanL;
	m_aapAniPlayerUppers[eHolster][eFire3][eRight]		= &m_AniHolsterFireLeanR;
	
	m_aapAniPlayerUppers[eHolster][eReload][eCenter]	= &m_AniHolsterReload;
	m_aapAniPlayerUppers[eHolster][eReload][eLeft]		= &m_AniHolsterReload;
	m_aapAniPlayerUppers[eHolster][eReload][eRight]		= &m_AniHolsterReload;

	m_aapAniPlayerUppers[eHolster][eSelect][eCenter]	= &m_AniHolsterSelect;
	m_aapAniPlayerUppers[eHolster][eSelect][eLeft]		= &m_AniHolsterSelect;
	m_aapAniPlayerUppers[eHolster][eSelect][eRight]		= &m_AniHolsterSelect;

	m_aapAniPlayerUppers[eHolster][eDeselect][eCenter]	= &m_AniHolsterDeselect;
	m_aapAniPlayerUppers[eHolster][eDeselect][eLeft]	= &m_AniHolsterDeselect;
	m_aapAniPlayerUppers[eHolster][eDeselect][eRight]	= &m_AniHolsterDeselect;

	// [ePlace] [Posture] [Lean]

	m_aapAniPlayerUppers[ePlace][eUnalert][eCenter]		= &m_AniPlaceUnalert;
	m_aapAniPlayerUppers[ePlace][eUnalert][eLeft]		= &m_AniPlaceUnalert;
	m_aapAniPlayerUppers[ePlace][eUnalert][eRight]		= &m_AniPlaceUnalert;
	
	m_aapAniPlayerUppers[ePlace][eAlert][eCenter]		= &m_AniPlaceAlert;
	m_aapAniPlayerUppers[ePlace][eAlert][eLeft]			= &m_AniPlaceAlert;
	m_aapAniPlayerUppers[ePlace][eAlert][eRight]		= &m_AniPlaceAlert;
	
	m_aapAniPlayerUppers[ePlace][eAim][eCenter]			= &m_AniPlaceAim;
	m_aapAniPlayerUppers[ePlace][eAim][eLeft]			= &m_AniPlaceAimLeanL;
	m_aapAniPlayerUppers[ePlace][eAim][eRight]			= &m_AniPlaceAimLeanR;
	
	m_aapAniPlayerUppers[ePlace][eFire][eCenter]		= &m_AniPlaceFire;
	m_aapAniPlayerUppers[ePlace][eFire][eLeft]			= &m_AniPlaceFireLeanL;
	m_aapAniPlayerUppers[ePlace][eFire][eRight]			= &m_AniPlaceFireLeanR;

	m_aapAniPlayerUppers[ePlace][eFire2][eCenter]		= &m_AniPlaceFire;
	m_aapAniPlayerUppers[ePlace][eFire2][eLeft]			= &m_AniPlaceFireLeanL;
	m_aapAniPlayerUppers[ePlace][eFire2][eRight]		= &m_AniPlaceFireLeanR;

	m_aapAniPlayerUppers[ePlace][eFire3][eCenter]		= &m_AniPlaceFire;
	m_aapAniPlayerUppers[ePlace][eFire3][eLeft]			= &m_AniPlaceFireLeanL;
	m_aapAniPlayerUppers[ePlace][eFire3][eRight]		= &m_AniPlaceFireLeanR;
	
	m_aapAniPlayerUppers[ePlace][eReload][eCenter]		= &m_AniPlaceReload;
	m_aapAniPlayerUppers[ePlace][eReload][eLeft]		= &m_AniPlaceReload;
	m_aapAniPlayerUppers[ePlace][eReload][eRight]		= &m_AniPlaceReload;

	m_aapAniPlayerUppers[ePlace][eSelect][eCenter]		= &m_AniPlaceSelect;
	m_aapAniPlayerUppers[ePlace][eSelect][eLeft]		= &m_AniPlaceSelect;
	m_aapAniPlayerUppers[ePlace][eSelect][eRight]		= &m_AniPlaceSelect;

	m_aapAniPlayerUppers[ePlace][eDeselect][eCenter]	= &m_AniPlaceDeselect;
	m_aapAniPlayerUppers[ePlace][eDeselect][eLeft]		= &m_AniPlaceDeselect;
	m_aapAniPlayerUppers[ePlace][eDeselect][eRight]		= &m_AniPlaceDeselect;

	// [eGadget] [Posture] [Lean]

	m_aapAniPlayerUppers[eGadget][eUnalert][eCenter]	= &m_AniGadgetUnalert;
	m_aapAniPlayerUppers[eGadget][eUnalert][eLeft]		= &m_AniGadgetUnalert;
	m_aapAniPlayerUppers[eGadget][eUnalert][eRight]		= &m_AniGadgetUnalert;
	
	m_aapAniPlayerUppers[eGadget][eAlert][eCenter]		= &m_AniGadgetAlert;
	m_aapAniPlayerUppers[eGadget][eAlert][eLeft]		= &m_AniGadgetAlert;
	m_aapAniPlayerUppers[eGadget][eAlert][eRight]		= &m_AniGadgetAlert;
	
	m_aapAniPlayerUppers[eGadget][eAim][eCenter]		= &m_AniGadgetAim;
	m_aapAniPlayerUppers[eGadget][eAim][eLeft]			= &m_AniGadgetAimLeanL;
	m_aapAniPlayerUppers[eGadget][eAim][eRight]			= &m_AniGadgetAimLeanR;
	
	m_aapAniPlayerUppers[eGadget][eFire][eCenter]		= &m_AniGadgetFire;
	m_aapAniPlayerUppers[eGadget][eFire][eLeft]			= &m_AniGadgetFireLeanL;
	m_aapAniPlayerUppers[eGadget][eFire][eRight]		= &m_AniGadgetFireLeanR;

	m_aapAniPlayerUppers[eGadget][eFire2][eCenter]		= &m_AniGadgetFire;
	m_aapAniPlayerUppers[eGadget][eFire2][eLeft]		= &m_AniGadgetFireLeanL;
	m_aapAniPlayerUppers[eGadget][eFire2][eRight]		= &m_AniGadgetFireLeanR;

	m_aapAniPlayerUppers[eGadget][eFire3][eCenter]		= &m_AniGadgetFire;
	m_aapAniPlayerUppers[eGadget][eFire3][eLeft]		= &m_AniGadgetFireLeanL;
	m_aapAniPlayerUppers[eGadget][eFire3][eRight]		= &m_AniGadgetFireLeanR;
	
	m_aapAniPlayerUppers[eGadget][eReload][eCenter]		= &m_AniGadgetReload;
	m_aapAniPlayerUppers[eGadget][eReload][eLeft]		= &m_AniGadgetReload;
	m_aapAniPlayerUppers[eGadget][eReload][eRight]		= &m_AniGadgetReload;

	m_aapAniPlayerUppers[eGadget][eSelect][eCenter]		= &m_AniGadgetSelect;
	m_aapAniPlayerUppers[eGadget][eSelect][eLeft]		= &m_AniGadgetSelect;
	m_aapAniPlayerUppers[eGadget][eSelect][eRight]		= &m_AniGadgetSelect;

	m_aapAniPlayerUppers[eGadget][eDeselect][eCenter]	= &m_AniGadgetDeselect;
	m_aapAniPlayerUppers[eGadget][eDeselect][eLeft]		= &m_AniGadgetDeselect;
	m_aapAniPlayerUppers[eGadget][eDeselect][eRight]	= &m_AniGadgetDeselect;

	// [eStars] [Posture] [Lean]

	m_aapAniPlayerUppers[eStars][eUnalert][eCenter]		= &m_AniStarsUnalert;
	m_aapAniPlayerUppers[eStars][eUnalert][eLeft]		= &m_AniStarsUnalert;
	m_aapAniPlayerUppers[eStars][eUnalert][eRight]		= &m_AniStarsUnalert;
	
	m_aapAniPlayerUppers[eStars][eAlert][eCenter]		= &m_AniStarsAlert;
	m_aapAniPlayerUppers[eStars][eAlert][eLeft]			= &m_AniStarsAlert;
	m_aapAniPlayerUppers[eStars][eAlert][eRight]		= &m_AniStarsAlert;
	
	m_aapAniPlayerUppers[eStars][eAim][eCenter]			= &m_AniStarsAim;
	m_aapAniPlayerUppers[eStars][eAim][eLeft]			= &m_AniStarsAimLeanL;
	m_aapAniPlayerUppers[eStars][eAim][eRight]			= &m_AniStarsAimLeanR;
	
	m_aapAniPlayerUppers[eStars][eFire][eCenter]		= &m_AniStarsFire;
	m_aapAniPlayerUppers[eStars][eFire][eLeft]			= &m_AniStarsFireLeanL;
	m_aapAniPlayerUppers[eStars][eFire][eRight]			= &m_AniStarsFireLeanR;

	m_aapAniPlayerUppers[eStars][eFire2][eCenter]		= &m_AniStarsFire;
	m_aapAniPlayerUppers[eStars][eFire2][eLeft]			= &m_AniStarsFireLeanL;
	m_aapAniPlayerUppers[eStars][eFire2][eRight]		= &m_AniStarsFireLeanR;

	m_aapAniPlayerUppers[eStars][eFire3][eCenter]		= &m_AniStarsFire;
	m_aapAniPlayerUppers[eStars][eFire3][eLeft]			= &m_AniStarsFireLeanL;
	m_aapAniPlayerUppers[eStars][eFire3][eRight]		= &m_AniStarsFireLeanR;
	
	m_aapAniPlayerUppers[eStars][eReload][eCenter]		= &m_AniStarsReload;
	m_aapAniPlayerUppers[eStars][eReload][eLeft]		= &m_AniStarsReload;
	m_aapAniPlayerUppers[eStars][eReload][eRight]		= &m_AniStarsReload;

	m_aapAniPlayerUppers[eStars][eSelect][eCenter]		= &m_AniStarsSelect;
	m_aapAniPlayerUppers[eStars][eSelect][eLeft]		= &m_AniStarsSelect;
	m_aapAniPlayerUppers[eStars][eSelect][eRight]		= &m_AniStarsSelect;

	m_aapAniPlayerUppers[eStars][eDeselect][eCenter]	= &m_AniStarsDeselect;
	m_aapAniPlayerUppers[eStars][eDeselect][eLeft]		= &m_AniStarsDeselect;
	m_aapAniPlayerUppers[eStars][eDeselect][eRight]		= &m_AniStarsDeselect;

	
	// [eWalking] [Direction] [Lean]
	
	m_aapAniPlayerLowers[eWalking][eNone][eCenter]			= &m_AniStand;
	m_aapAniPlayerLowers[eWalking][eNone][eLeft]			= &m_AniStandLeanL;
	m_aapAniPlayerLowers[eWalking][eNone][eRight]			= &m_AniStandLeanR;

	m_aapAniPlayerLowers[eWalking][eForward][eCenter]		= &m_AniWalkForward;
	m_aapAniPlayerLowers[eWalking][eForward][eLeft]			= &m_AniWalkForward;
	m_aapAniPlayerLowers[eWalking][eForward][eRight]		= &m_AniWalkForward;

	m_aapAniPlayerLowers[eWalking][eBackward][eCenter]		= &m_AniWalkBackward;
	m_aapAniPlayerLowers[eWalking][eBackward][eLeft]		= &m_AniWalkBackward;
	m_aapAniPlayerLowers[eWalking][eBackward][eRight]		= &m_AniWalkBackward;

	m_aapAniPlayerLowers[eWalking][eStrafeLeft][eCenter]	= &m_AniWalkStrafeLeft;
	m_aapAniPlayerLowers[eWalking][eStrafeLeft][eLeft]		= &m_AniWalkStrafeLeft;
	m_aapAniPlayerLowers[eWalking][eStrafeLeft][eRight]		= &m_AniWalkStrafeLeft;
	
	m_aapAniPlayerLowers[eWalking][eStrafeRight][eCenter]	= &m_AniWalkStrafeRight;
	m_aapAniPlayerLowers[eWalking][eStrafeRight][eLeft]		= &m_AniWalkStrafeRight;
	m_aapAniPlayerLowers[eWalking][eStrafeRight][eRight]	= &m_AniWalkStrafeRight;

	
	// [eRunning] [Direction] [Lean]

	m_aapAniPlayerLowers[eRunning][eNone][eCenter]			= &m_AniStand;
	m_aapAniPlayerLowers[eRunning][eNone][eLeft]			= &m_AniStandLeanL;
	m_aapAniPlayerLowers[eRunning][eNone][eRight]			= &m_AniStandLeanR;

	m_aapAniPlayerLowers[eRunning][eForward][eCenter]		= &m_AniRunForward;
	m_aapAniPlayerLowers[eRunning][eForward][eLeft]			= &m_AniRunForward;
	m_aapAniPlayerLowers[eRunning][eForward][eRight]		= &m_AniRunForward;

	m_aapAniPlayerLowers[eRunning][eBackward][eCenter]		= &m_AniRunBackward;
	m_aapAniPlayerLowers[eRunning][eBackward][eLeft]		= &m_AniRunBackward;
	m_aapAniPlayerLowers[eRunning][eBackward][eRight]		= &m_AniRunBackward;
	
	m_aapAniPlayerLowers[eRunning][eStrafeLeft][eCenter]	= &m_AniRunStrafeLeft;
	m_aapAniPlayerLowers[eRunning][eStrafeLeft][eLeft]		= &m_AniRunStrafeLeft;
	m_aapAniPlayerLowers[eRunning][eStrafeLeft][eRight]		= &m_AniRunStrafeLeft;
	
	m_aapAniPlayerLowers[eRunning][eStrafeRight][eCenter]	= &m_AniRunStrafeRight;
	m_aapAniPlayerLowers[eRunning][eStrafeRight][eLeft]		= &m_AniRunStrafeRight;
	m_aapAniPlayerLowers[eRunning][eStrafeRight][eRight]	= &m_AniRunStrafeRight;

	
	// [eCrouching] [Direction] [Lean]
	
	m_aapAniPlayerLowers[eCrouching][eNone][eCenter]		= &m_AniCrouch;
	m_aapAniPlayerLowers[eCrouching][eNone][eLeft]			= &m_AniCrouchLeanL;
	m_aapAniPlayerLowers[eCrouching][eNone][eRight]			= &m_AniCrouchLeanR;

	m_aapAniPlayerLowers[eCrouching][eForward][eCenter]		= &m_AniCrouchForward;
	m_aapAniPlayerLowers[eCrouching][eForward][eLeft]		= &m_AniCrouchForward;
	m_aapAniPlayerLowers[eCrouching][eForward][eRight]		= &m_AniCrouchForward;

	m_aapAniPlayerLowers[eCrouching][eBackward][eCenter]	= &m_AniCrouchBackward;
	m_aapAniPlayerLowers[eCrouching][eBackward][eLeft]		= &m_AniCrouchBackward;
	m_aapAniPlayerLowers[eCrouching][eBackward][eRight]		= &m_AniCrouchBackward;
	
	m_aapAniPlayerLowers[eCrouching][eStrafeLeft][eCenter]	= &m_AniCrouchStrafeLeft;
	m_aapAniPlayerLowers[eCrouching][eStrafeLeft][eLeft]	= &m_AniCrouchStrafeLeft;
	m_aapAniPlayerLowers[eCrouching][eStrafeLeft][eRight]	= &m_AniCrouchStrafeLeft;
	
	m_aapAniPlayerLowers[eCrouching][eStrafeRight][eCenter]	= &m_AniCrouchStrafeRight;
	m_aapAniPlayerLowers[eCrouching][eStrafeRight][eLeft]	= &m_AniCrouchStrafeRight;
	m_aapAniPlayerLowers[eCrouching][eStrafeRight][eRight]	= &m_AniCrouchStrafeRight;

	
	// [eSwimming] [Direction] [Lean]
	
	m_aapAniPlayerLowers[eSwimming][eNone][eCenter]			= &m_AniSwim;
	m_aapAniPlayerLowers[eSwimming][eNone][eLeft]			= &m_AniSwim;
	m_aapAniPlayerLowers[eSwimming][eNone][eRight]			= &m_AniSwim;

	m_aapAniPlayerLowers[eSwimming][eForward][eCenter]		= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eForward][eLeft]		= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eForward][eRight]		= &m_AniSwimForward;

	m_aapAniPlayerLowers[eSwimming][eBackward][eCenter]		= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eBackward][eLeft]		= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eBackward][eRight]		= &m_AniSwimForward;
	
	m_aapAniPlayerLowers[eSwimming][eStrafeLeft][eCenter]	= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eStrafeLeft][eLeft]		= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eStrafeLeft][eRight]	= &m_AniSwimForward;
	
	m_aapAniPlayerLowers[eSwimming][eStrafeRight][eCenter]	= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eStrafeRight][eLeft]	= &m_AniSwimForward;
	m_aapAniPlayerLowers[eSwimming][eStrafeRight][eRight]	= &m_AniSwimForward;

	
	// [eJumping] [Direction] [Lean]

	m_aapAniPlayerLowers[eJumping][eJump][eCenter]			= &m_AniJumpJump;
	m_aapAniPlayerLowers[eJumping][eJump][eLeft]			= &m_AniJumpJump;
	m_aapAniPlayerLowers[eJumping][eJump][eRight]			= &m_AniJumpJump;
	
	m_aapAniPlayerLowers[eJumping][eTuck][eCenter]			= &m_AniJumpTuck;
	m_aapAniPlayerLowers[eJumping][eTuck][eLeft]			= &m_AniJumpTuck;
	m_aapAniPlayerLowers[eJumping][eTuck][eRight]			= &m_AniJumpTuck;
	
	m_aapAniPlayerLowers[eJumping][eLand][eCenter]			= &m_AniJumpLand;
	m_aapAniPlayerLowers[eJumping][eLand][eLeft]			= &m_AniJumpLand;
	m_aapAniPlayerLowers[eJumping][eLand][eRight]			= &m_AniJumpLand;


	
	m_apAniPlayerMains[eClimbing]					= &m_AniClimb;
	m_apAniPlayerMains[eClimbingUp]					= &m_AniClimbUp;
	m_apAniPlayerMains[eClimbingDown]				= &m_AniClimbDown;
	m_apAniPlayerMains[eSnowmobile]					= &m_AniSnowmobile;
	m_apAniPlayerMains[eDT_Slippery]				= &m_AniDT_Slippery;
	m_apAniPlayerMains[eDT_Sleeping]				= &m_AniDT_Sleeping;
	m_apAniPlayerMains[eDT_Stun]					= &m_AniDT_Stun;
	m_apAniPlayerMains[eDT_Laughing]				= &m_AniDT_Laughing;
	m_apAniPlayerMains[eDT_BearTrap]				= &m_AniDT_BearTrap;
	m_apAniPlayerMains[eDT_Glue]					= &m_AniDT_Glue;
	m_apAniPlayerMains[eDT_Hurt]					= &m_AniDT_Hurt;
	m_apAniPlayerMains[eDT_HurtWalk]				= &m_AniDT_HurtWalk;

	// The ani tracker that sets our dims is the lower

	m_eAniTrackerDims = m_eAniTrackerLower;

	EndTimingCounterServer("CAnimatorPlayer::Init took");

	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( m_hObject );
	if( !pPlayer ) return;

	// Set the correct number of trackers and which tracker updates the dims on the client...

	pPlayer->SendAllFXToClients();
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
	if( IsDisabled( ))
		return;

	CAnimator::Update();

	LTASSERT(m_eAniTrackerUpper != eAniTrackerInvalid, "TODO: Add description here");
	LTASSERT(m_eAniTrackerLower != eAniTrackerInvalid, "TODO: Add description here");

	// Set the upper/lower or fully body anis

	if ( m_eMain != eInvalid && m_eLastMain == eInvalid )
	{
		DisableAniTracker(m_eAniTrackerUpper);
		DisableAniTracker(m_eAniTrackerLower);

		if ( !IsAniTrackerLooping(m_eAniTrackerUpper) )
		{
            LoopAniTracker(m_eAniTrackerUpper, true);
		}
		if ( !IsAniTrackerLooping(m_eAniTrackerLower) )
		{
            LoopAniTracker(m_eAniTrackerLower, true);
		}

		SetAni(m_AniUpperBase.eAni, m_eAniTrackerUpper);
		SetAni(m_AniLowerBase.eAni, m_eAniTrackerLower);

		// Play the 'IN' animation if we have one...
		
		AniPlayerMain* pAniPlayerMain = m_apAniPlayerMains[m_eMain];
		
		if( pAniPlayerMain->eAniIn != eAniInvalid )
		{
			SetAni( pAniPlayerMain->eAniIn, eAniTrackerMain );
		}
	}

	if ( m_eMain == eInvalid && m_eLastMain != eInvalid )
	{
		AniPlayerMain* pAniPlayerMain = m_apAniPlayerMains[m_eLastMain];

		if( pAniPlayerMain->eAniOut != eAniInvalid )
		{
			HMODELANIM hCurAni = GetAni( eAniTrackerMain );
			HMODELANIM hNewAni = m_ahAnis[pAniPlayerMain->eAniOut];

			if( hCurAni != hNewAni )
			{
				LoopAniTracker( eAniTrackerMain, false );
				SetAni( pAniPlayerMain->eAniOut, eAniTrackerMain );
			}
		}
		else
		{
			EnableAniTracker(m_eAniTrackerUpper);
			EnableAniTracker(m_eAniTrackerLower);

			if ( !IsAniTrackerLooping(eAniTrackerMain) )
			{
				LoopAniTracker(eAniTrackerMain, true);
			}

			SetAni(m_AniMainBase.eAniIdle, eAniTrackerMain);
		}
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
			LoopAniTracker(eAniTrackerMain, true);
		}
		else if ( !pAniPlayerMain->bLoops && IsAniTrackerLooping(eAniTrackerMain) )
		{
			LoopAniTracker(eAniTrackerMain, false);
		}


		if( pAniPlayerMain->eAniIn != eAniInvalid &&
			pAniPlayerMain->eAniTwitch1 == eAniInvalid &&
			pAniPlayerMain->eAniTwitch2 == eAniInvalid )
		{
			if( IsAniTrackerDone( eAniTrackerMain ))
			{
				LoopAniTracker( eAniTrackerMain, true );
				SetAni( pAniPlayerMain->eAniIdle, eAniTrackerMain );
			}
		}
		else if( pAniPlayerMain->eAniTwitch1 != eAniInvalid && pAniPlayerMain->eAniTwitch2 != eAniInvalid )
		{
			if( IsAniTrackerDone( eAniTrackerMain ))
			{
				LoopAniTracker(	eAniTrackerMain, false );
				
				switch( GetRandom( 1, 3) )
				{
					case 1: SetAni( pAniPlayerMain->eAniIdle, eAniTrackerMain ); break;
					case 2: SetAni( pAniPlayerMain->eAniTwitch1, eAniTrackerMain ); break;
					case 3:	SetAni( pAniPlayerMain->eAniTwitch2, eAniTrackerMain ); break;

					default:
							SetAni( pAniPlayerMain->eAniIdle, eAniTrackerMain );
					break;
				}
			}
		}
		else
		{
			SetAni(pAniPlayerMain->eAniIdle, eAniTrackerMain);
		}
	}
	else
	{
		if( m_eLastMain != eInvalid )
		{
			AniPlayerMain* pAniPlayerMain = m_apAniPlayerMains[m_eLastMain];

			if( pAniPlayerMain->eAniOut != eAniInvalid )
			{
				if( IsAniTrackerDone( eAniTrackerMain ))
				{
					EnableAniTracker(m_eAniTrackerUpper);
					EnableAniTracker(m_eAniTrackerLower);

					if ( !IsAniTrackerLooping(eAniTrackerMain) )
					{
						LoopAniTracker(eAniTrackerMain, true);
					}

					SetAni(m_AniMainBase.eAniIdle, eAniTrackerMain);
				}
				else
				{
					m_eMain = m_eLastMain;
				}
			}
		}

		if( m_eMain == eInvalid )
		{
			AniPlayerUpper* pAniPlayerUpper = m_aapAniPlayerUppers[m_eWeapon][m_ePosture][m_eLean];
			AniPlayerLower* pAniPlayerLower = m_aapAniPlayerLowers[m_eMovement][m_eDirection][m_eLean];

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
				LoopAniTracker(m_eAniTrackerUpper, true);
			}
			else if ( !pAniPlayerUpper->bLoops && IsAniTrackerLooping(m_eAniTrackerUpper) )
			{
				LoopAniTracker(m_eAniTrackerUpper, false);
			}

			if ( pAniPlayerLower->bLoops && !IsAniTrackerLooping(m_eAniTrackerLower) )
			{
				LoopAniTracker(m_eAniTrackerLower, true);
			}
			else if ( !pAniPlayerLower->bLoops && IsAniTrackerLooping(m_eAniTrackerLower) )
			{
				LoopAniTracker(m_eAniTrackerLower, false);
			}

			SetAni(eAniUpper, m_eAniTrackerUpper);
			SetAni(eAniLower, m_eAniTrackerLower);
		}
	}

	// Print out some debug info to help track down an animation issue...

	if( s_vtPlayerAnimDebug.GetFloat( -1.0f ) > -1.0f )
	{
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
		if( pPlayer )
		{
			uint32 dwID = g_pLTServer->GetClientID( pPlayer->GetClient() );
			if( dwID == s_vtPlayerAnimDebug.GetFloat( -1.0f ) )
			{
		g_pLTServer->CPrint( "" );
		g_pLTServer->CPrint( "Main: %i LastMain: %i", m_eMain, m_eLastMain );
		g_pLTServer->CPrint( "Weapon: %i LastWeapon: %i", m_eWeapon, m_eLastWeapon );
		g_pLTServer->CPrint( "Posture: %i LastPosture: %i", m_ePosture, m_eLastPosture );
		g_pLTServer->CPrint( "Movement: %i LastMovement: %i", m_eMovement, m_eLastMovement );
		g_pLTServer->CPrint( "Direction: %i LastDirection: %i", m_eDirection, m_eLastDirection );
		g_pLTServer->CPrint( "Lean: %i LastLean: %i", m_eLean, m_eLastLean );
		g_pLTServer->CPrint( "" );

		HMODELWEIGHTSET hNullWeightset;
		if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, (char*)"Null", hNullWeightset) )
		{
			g_pLTServer->CPrint( "ERROR NULL WeightSet not found!" );
			return;
		}

		HMODELANIM hAni;
		ANIMTRACKERID nTrackerId = m_aAniTrackers[eAniTrackerMain].m_AnimTracker;
		if( LT_OK == g_pModelLT->GetCurAnim( m_hObject, nTrackerId, hAni ))
		{
			HMODELWEIGHTSET hWeightSet;
			if( LT_OK != g_pModelLT->GetWeightSet( m_hObject, nTrackerId, hWeightSet ))
			{
				g_pLTServer->CPrint( "ERROR getting MAIN Tracker WeightSet!" );
			}

			bool bEnabled = (hWeightSet != hNullWeightset);
			bool bLooping = (g_pModelLT->GetLooping( m_hObject, nTrackerId ) == LT_YES);
			bool bPlaying = (g_pModelLT->GetPlaying( m_hObject, nTrackerId ) == LT_YES);

			g_pLTServer->CPrint( "MAIN Tracker Cur Anim: %i Looping: %i Playing: %i Enabled: %i", hAni, bLooping, bPlaying, bEnabled );
		}
		else
		{
			g_pLTServer->CPrint( "ERROR getting MAIN Tracker Cur Anim!" );
		}

		nTrackerId = m_aAniTrackers[m_eAniTrackerUpper].m_AnimTracker;
		if( LT_OK == g_pModelLT->GetCurAnim( m_hObject, nTrackerId, hAni ))
		{
			HMODELWEIGHTSET hWeightSet;
			if( LT_OK != g_pModelLT->GetWeightSet( m_hObject, nTrackerId, hWeightSet ))
			{
				g_pLTServer->CPrint( "ERROR getting UPPER Tracker WeightSet!" );
			}

			bool bEnabled = (hWeightSet != hNullWeightset);
			bool bLooping = (g_pModelLT->GetLooping( m_hObject, nTrackerId ) == LT_YES);
			bool bPlaying = (g_pModelLT->GetPlaying( m_hObject, nTrackerId ) == LT_YES);

			g_pLTServer->CPrint( "UPPER Tracker Cur Anim: %i Looping: %i Playing: %i Enabled: %i", hAni, bLooping, bPlaying, bEnabled );
		}
		else
		{
			g_pLTServer->CPrint( "ERROR getting UPPER Tracker Cur Anim!" );
		}

		nTrackerId = m_aAniTrackers[m_eAniTrackerLower].m_AnimTracker;
		if( LT_OK == g_pModelLT->GetCurAnim( m_hObject, nTrackerId, hAni ))
		{
			HMODELWEIGHTSET hWeightSet;
			if( LT_OK != g_pModelLT->GetWeightSet( m_hObject, nTrackerId, hWeightSet ))
			{
				g_pLTServer->CPrint( "ERROR getting LOWER Tracker WeightSet!" );
			}
			
			bool bEnabled = (hWeightSet != hNullWeightset);
			bool bLooping = (g_pModelLT->GetLooping( m_hObject, nTrackerId ) == LT_YES);
			bool bPlaying = (g_pModelLT->GetPlaying( m_hObject, nTrackerId ) == LT_YES);

			g_pLTServer->CPrint( "LOWER Tracker Cur Anim: %i Looping: %i Playing: %i Enabled: %i", hAni, bLooping, bPlaying, bEnabled );
		}
		else
		{
			g_pLTServer->CPrint( "ERROR getting LOWER Tracker Cur Anim!" );
		}
	}
		}	
	}

	// Record the state etc

	m_eLastMain = m_eMain;
	m_eLastWeapon = m_eWeapon;
	m_eLastPosture = m_ePosture;
	m_eLastMovement	= m_eMovement;
	m_eLastDirection = m_eDirection;
	m_eLastLean	= m_eLean;

	// Reset all vars

	m_eMain	= eInvalid;
	m_eWeapon = eRifle;
	m_ePosture = eUnalert;
	m_eMovement = eWalking;
	m_eDirection = eNone;
	m_eLean = eCenter;

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

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	if ( eAniTracker == eAniTrackerMain )
	{
		cMsg.WriteBits(CFX_RESET_TRACKER, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint8(MAIN_TRACKER);
	}
	else if ( eAniTracker == m_eAniTrackerLower )
	{
		cMsg.WriteBits(CFX_RESET_TRACKER, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint8(1);
	}
	else if ( eAniTracker == m_eAniTrackerUpper )
	{
		cMsg.WriteBits(CFX_RESET_TRACKER, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint8(2);
	}
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::UpdateForceDucking()
//
//	PURPOSE:	Updates the Player's dims based on the ani
//
// ----------------------------------------------------------------------- //

void CAnimatorPlayer::UpdateForceDucking()
{
	HMODELANIM hAni = GetAni(m_eAniTrackerLower);
	
	if ( INVALID_MODEL_ANIM != hAni )
	{
		// Use the dims of the standing ani to see if we no longer need to duck...
		
		LTVector vDims;
		LTRESULT dResult = g_pModelLT->GetModelAnimUserDims(m_hObject, m_AniStand.eAni, &vDims);
		LTASSERT(dResult == LT_OK, "TODO: Add description here");

		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
		if( !pPlayer ) return;		

		// If we could update the dims, or we're forcing the animation, set it

		if ( pPlayer->SetDims(&vDims, false) /*|| IS DEATH ANI*/)
		{
			pPlayer->ForceDuck(false);
		}
		else
		{
			// If we were ducking, and we tried to stand but couldn't, make us continue to duck

			if ((m_eLastMovement == eCrouching) && !(m_eMovement == eCrouching))
			{
				m_eMovement = eCrouching;
				pPlayer->ForceDuck(true);

				if( s_vtPlayerAnimDebug.GetFloat() > 0.0f )
				{
					g_pLTServer->CPrint( "FORCE DUCK TRUE" );
				}
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

bool CAnimatorPlayer::SetDims(HMODELANIM hAni)
{
    LTVector vDims;
    LTRESULT dResult = g_pModelLT->GetModelAnimUserDims(m_hObject, hAni, &vDims);
	LTASSERT(dResult == LT_OK, "TODO: Add description here");

    CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hObject);
	if( !pPlayer ) return false;

	// If we could update the dims, or we're forcing the animation, set it

    if ( pPlayer->SetDims(&vDims, false) /*|| IS DEATH ANI*/)
	{
        return true;
	}
	else
	{
		// If we were ducking, and we tried to stand but couldn't, make us continue to duck

		if ((m_eLastMovement == eCrouching) && !(m_eMovement == eCrouching))
		{
			m_eMovement = eCrouching;
			pPlayer->ForceDuck(true);

			if( s_vtPlayerAnimDebug.GetFloat() > 0.0f )
			{
				g_pLTServer->CPrint( "FORCE DUCK TRUE" );
			}
		}

		return false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::SetMainToBase()
//
//	PURPOSE:	Forces the main tracker to the base animation.
//
// ----------------------------------------------------------------------- //

void CAnimatorPlayer::SetMainToBase()
{
	SetAni(m_AniMainBase.eAniIdle, eAniTrackerMain);

	m_eMain = m_eLastMain = eInvalid; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorPlayer::IsAnimating*()
//
//	PURPOSE:	Is the parameter responsible for the current animation?
//
// ----------------------------------------------------------------------- //

bool CAnimatorPlayer::IsAnimatingMain(Main eMain) const
{
	return ( m_eLastMain == eMain && !IsAniTrackerDone(eAniTrackerMain) );
}

bool CAnimatorPlayer::IsAnimatingWeapon(Weapon eWeapon) const
{
	return ( m_eLastWeapon == eWeapon && !IsAniTrackerDone(m_eAniTrackerUpper) );
}

bool CAnimatorPlayer::IsAnimatingPosture(Posture ePosture) const
{
	return ( m_eLastPosture == ePosture && !IsAniTrackerDone(m_eAniTrackerUpper) );
}

bool CAnimatorPlayer::IsAnimatingMovement(Movement eMovement) const
{
	return ( m_eLastMovement == eMovement && !IsAniTrackerDone(m_eAniTrackerLower) );
}

bool CAnimatorPlayer::IsAnimatingDirection(Direction eDirection) const
{
	return ( m_eLastDirection == eDirection && !IsAniTrackerDone(m_eAniTrackerLower) );
}

bool CAnimatorPlayer::IsAnimatingLean(Lean eLean) const
{
	return ( m_eLastLean == eLean && !IsAniTrackerDone(m_eAniTrackerUpper) );
}

bool CAnimatorPlayer::IsAnimatingMainDone(Main eMain) const
{
	return ( m_eLastMain == eMain && IsAniTrackerDone(eAniTrackerMain) );
}

bool CAnimatorPlayer::IsAnimatingWeaponDone(Weapon eWeapon) const
{
	return ( m_eLastWeapon == eWeapon && IsAniTrackerDone(m_eAniTrackerUpper) );
}

bool CAnimatorPlayer::IsAnimatingPostureDone(Posture ePosture) const
{
	return ( m_eLastPosture == ePosture && IsAniTrackerDone(m_eAniTrackerUpper) );
}

bool CAnimatorPlayer::IsAnimatingMovementDone(Movement eMovement) const
{
	return ( m_eLastMovement == eMovement && IsAniTrackerDone(m_eAniTrackerLower) );
}

bool CAnimatorPlayer::IsAnimatingDirectionDone(Direction eDirection) const
{
	return ( m_eLastDirection == eDirection && IsAniTrackerDone(m_eAniTrackerLower) );
}

bool CAnimatorPlayer::IsAnimatingLeanDone(Lean eLean) const
{
	return ( m_eLastLean == eLean && IsAniTrackerDone(m_eAniTrackerUpper) );
}
