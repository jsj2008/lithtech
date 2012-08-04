// ----------------------------------------------------------------------- //
//
// MODULE  : AccuracyMgr.cpp
//
// PURPOSE : implementation of class to handle accuracy penalties
//
// CREATED : 05/10/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AccuracyMgr.h"
#include "CMoveMgr.h"
#include "WeaponDB.h"
#include "PlayerStats.h"
#include "PlayerCamera.h"
#include "AimMgr.h"
#include "HUDDebug.h"


VarTrack    CAccuracyMgr::s_vtFastTurnRate;
VarTrack    CAccuracyMgr::s_vtDebugPerturb;
VarTrack    CAccuracyMgr::s_vtDebugPerturbPercent;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::CAccuracyMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CAccuracyMgr::CAccuracyMgr( ) :
	m_vLastForward( 0.0f, 0.0f, 1.0f )

{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::~CAccuracyMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CAccuracyMgr::~CAccuracyMgr( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::Init
//
//	PURPOSE:	set up instance for use
//
// ----------------------------------------------------------------------- //

void CAccuracyMgr::Init()
{
	s_vtFastTurnRate.Init( g_pLTClient, "FastTurnRate", 0, 2.3f );
	s_vtDebugPerturb.Init( g_pLTClient, "DebugPerturb", 0, 0.0f );
	s_vtDebugPerturbPercent.Init( g_pLTClient, "DebugPerturbPercent", 0, -1.0f );

	Reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::Init
//
//	PURPOSE:	reset perturb values
//
// ----------------------------------------------------------------------- //

void CAccuracyMgr::Reset()
{
	m_fCurrentPerturb = 0.0f;
	m_fTargetPerturb = 0.0f;
	m_vLastForward.Init( 0.0f, 0.0f, 1.0f );

	m_hWpnData = NULL;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::Update
//
//	PURPOSE:	update the current accuracy value
//
// ----------------------------------------------------------------------- //

void CAccuracyMgr::Update()
{

	HWEAPON hWeapon = g_pPlayerStats->GetCurrentWeaponRecord();
	m_hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	float fTimeDelta = ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );

	float fMove = CalculateMovementPerturb();
	float fFire = CalculateFiringPerturb();
	float fTurn = CalculateTurningPerturb();

	m_fTargetPerturb = fMove;
	m_fTargetPerturb = LTMAX(m_fTargetPerturb,fFire);
	m_fTargetPerturb = LTMAX(m_fTargetPerturb,fTurn);

	float fDiff = (m_fTargetPerturb - m_fCurrentPerturb);


	if (fDiff > 0.0f)
	{
		float fDelta = g_pWeaponDB->GetFloat(m_hWpnData,WDB_WEAPON_fPerturbIncreaseRate) * fTimeDelta;
		fDelta = LTMIN(fDelta,fDiff);
		m_fCurrentPerturb += fDelta;
	}
	else if (fDiff < 0.0f)
	{
		float fDelta = g_pWeaponDB->GetFloat(m_hWpnData,WDB_WEAPON_fPerturbDecreaseRate) * fTimeDelta;
		fDelta = LTMAX(fDelta,fDiff);
		m_fCurrentPerturb -= fDelta;
	}

//	m_fCurrentPerturb = LTCLAMP(m_fCurrentPerturb,0.0f,1.0f);
	//can't have a perturb of less than 0, but we don't necessarily have a Max value.
	m_fCurrentPerturb = LTMAX(m_fCurrentPerturb,0.0f);

#ifndef _FINAL
	if (s_vtDebugPerturb.GetFloat() > 0.0f)
	{
		float fPercent = s_vtDebugPerturbPercent.GetFloat();
		if (fPercent >= 0.0f)
		{
//			m_fCurrentPerturb = LTCLAMP(fPercent,0.0f,1.0f);
			//can't have a perturb of less than 0, but we don't necessarily have a Max value.
			m_fCurrentPerturb = LTMAX(m_fCurrentPerturb,0.0f);
		}

		static wchar_t wszDbg[64] = L"";
		LTSNPrintF(wszDbg,LTARRAYSIZE(wszDbg),L"Perturb: %0.2f (move: %0.2f/fire: %0.2f/turn:  %0.2f)",m_fCurrentPerturb,fMove,fFire,fTurn);
		g_pHUDDebug->SetPerturbDebugString(wszDbg);
	}
	else
	{
		g_pHUDDebug->SetPerturbDebugString(NULL);
	}
#endif // _FINAL
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::CalculateMovementPerturb
//
//	PURPOSE:	determine the current penalty due to movement
//
// ----------------------------------------------------------------------- //

float CAccuracyMgr::CalculateMovementPerturb()
{
	if (!m_hWpnData)
		return 0.0f;

	uint32 dwPlayerFlags = g_pPlayerMgr->GetPlayerFlags();

	bool bAim = AimMgr::Instance().IsAiming();
	bool bCrouch = !!(dwPlayerFlags & BC_CFLG_DUCK);
	bool bMoving = !!( dwPlayerFlags & BC_CFLG_MOVING );
	bool bRun = bMoving && !!(dwPlayerFlags & BC_CFLG_RUN);
	bool bJump = !g_pMoveMgr->IsOnGround();

	const char* szState = "";

	if (g_pPlayerMgr->IsUnderwater())
	{
		szState = WDB_WEAPON_fSwim;
	}
	else if (bJump) 
	{
		szState = WDB_WEAPON_fJump;
	}
	else if (bRun) 
	{
		szState = WDB_WEAPON_fRun;
	}
	else if (bAim) 
	{
		if (bCrouch) 
		{
			if (bMoving)
			{
				szState = WDB_WEAPON_fAim_Crouch_Walk;
			}
			else
			{
				szState = WDB_WEAPON_fAim_Crouch;
			}
		}
		else
		{
			if (bMoving)
			{
				szState = WDB_WEAPON_fAim_Walk;
			}
			else
			{
				szState = WDB_WEAPON_fAim;
			}
		}

	}
	else
	{
		if (bCrouch) 
		{
			if (bMoving)
			{
				szState = WDB_WEAPON_fCrouch_Walk;
			}
			else
			{
				szState = WDB_WEAPON_fCrouch;
			}
		}
		else
		{
			if (bMoving)
			{
				szState = WDB_WEAPON_fWalk;
			}
			else
			{
				szState = WDB_WEAPON_fStand;
			}
		}
	}

	LTASSERT((*szState),"Unknown player state.");
	if (*szState)
	{
		return g_pWeaponDB->GetFloat(m_hWpnData,szState);
	}


	return 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::CalculateFiringPerturb
//
//	PURPOSE:	determine the current penalty due to movement
//
// ----------------------------------------------------------------------- //

float CAccuracyMgr::CalculateFiringPerturb()
{
	float fRecoil = g_pPlayerMgr->GetRecoilValue();

	if (fRecoil > 0.0f)
	{
		fRecoil *= g_pWeaponDB->GetFloat(m_hWpnData,WDB_WEAPON_fRecoil);

		return fRecoil;
	}

	return 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::CalculateTurningPerturb
//
//	PURPOSE:	determine the current penalty due to movement
//
// ----------------------------------------------------------------------- //

float CAccuracyMgr::CalculateTurningPerturb()
{
	
	float fTimeDelta = ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );

	// Get the current forward camera.
	LTVector vCurForward = g_pPlayerMgr->GetPlayerCamera( )->GetCameraRotation( ).Forward( );

	// Calculate the amount of change from last camera position.
	float fTurnRate = (1.0f - vCurForward.Dot( m_vLastForward )) / fTimeDelta;
	fTurnRate = LTCLAMP(fTurnRate,0.0f,1.0f);

	// Store the current for next frame.
	m_vLastForward = vCurForward;

	if (!m_hWpnData)
		return 0.0f;

	float fRotPerturb = g_pWeaponDB->GetFloat(m_hWpnData,WDB_WEAPON_fTurn) * fTurnRate;

	fRotPerturb = LTCLAMP(fRotPerturb,0.0f,1.0f);

	return fRotPerturb;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAccuracyMgr::CalculateTurningPerturb
//
//	PURPOSE:	determine the current penalty due to movement
//
// ----------------------------------------------------------------------- //

float CAccuracyMgr::GetCurrentWeaponPerturb() const
{
	if (m_hWpnData) 
	{
		LTVector2 v2Perturb = g_pWeaponDB->GetVector2( m_hWpnData, WDB_WEAPON_v2Perturb );

		if( (v2Perturb.x <= v2Perturb.y) && v2Perturb.y > 0 )
		{
			float fPerturbRange = float(v2Perturb.y - v2Perturb.x);
			return v2Perturb.x + (fPerturbRange * m_fCurrentPerturb);
		}
	}
	return 0.0f;
}