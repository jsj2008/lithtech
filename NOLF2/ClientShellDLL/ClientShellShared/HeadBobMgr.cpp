// ----------------------------------------------------------------------- //
//
// MODULE  : HeadBobMgr.cpp
//
// PURPOSE : Head Bob Mgr - Implementation
//
// CREATED : 01/09/00
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HeadBobMgr.h"
#include "GameClientShell.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "CharacterFX.h"
#include "VehicleMgr.h"
#include "ClientWeaponBase.h"
#include "CMoveMgr.h"

CHeadBobMgr* g_pHeadBobMgr = LTNULL;

extern VarTrack	g_vtMaxVehicleYawDiff;
extern SurfaceType g_eClientLastSurfaceType;

VarTrack		g_vtVehicleHeadCantUpRate;
VarTrack		g_vtVehicleHeadCantDownRate;
VarTrack		g_vtMaxVehicleHeadCant;
VarTrack		g_vtVehiclePaceAdjust;
VarTrack		g_vtRunPaceAdjust;
VarTrack		g_vtWalkPaceAdjust;
VarTrack		g_vtBobDecayTime;
VarTrack		g_vtBobV;
VarTrack		g_vtSwayH;
VarTrack		g_vtSwayV;
VarTrack		g_vtMaxBobAmp;
VarTrack		g_vtRollAdjust;
VarTrack		g_vtHeadBobAdjust;
VarTrack		g_vtWeaponSway;

static LTFLOAT   s_fBobDecayStartTime = -1.0f;
static LTFLOAT   s_fBobStartTime      = -1.0f;
static LTBOOL    s_bCanDoLeftFootstep = LTTRUE;
static LTBOOL    s_bCanDoRightFootstep = LTFALSE;



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::CHeadBobMgr
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

CHeadBobMgr::CHeadBobMgr()
{
	g_pHeadBobMgr	= this;

	m_fBobHeight	= 0.0f;
	m_fBobAmp		= 0.0f;
	m_fBobPhase		= 0.0f;
	m_fSwayPhase	= 0.0f;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::Init
//
//	PURPOSE:	Init
//
// --------------------------------------------------------------------------- //

LTBOOL CHeadBobMgr::Init()
{
    g_vtVehicleHeadCantUpRate.Init(g_pLTClient, "VehicleHeadCantUpRate", NULL, 1.0f);
    g_vtVehicleHeadCantDownRate.Init(g_pLTClient, "VehicleHeadCantDownRate", NULL, 1.0f);
    g_vtMaxVehicleHeadCant.Init(g_pLTClient, "VehicleMaxHeadCant", NULL, 20.0f);
    g_vtVehiclePaceAdjust.Init(g_pLTClient, "BobSwayVehiclePaceAdjust", NULL, 0.0f);

    g_vtRunPaceAdjust.Init(g_pLTClient, "BobSwayRunPaceAdjust", NULL, 1.5f);
    g_vtWalkPaceAdjust.Init(g_pLTClient, "BobSwayWalkPaceAdjust", NULL, 1.0f);

    g_vtBobDecayTime.Init(g_pLTClient, "BobDecayTime", NULL, 0.1f);
    g_vtBobV.Init(g_pLTClient, "BobV", NULL, 0.45f);
    g_vtSwayH.Init(g_pLTClient, "SwayH", NULL, 0.005f);
    g_vtSwayV.Init(g_pLTClient, "SwayV", NULL, 0.002f);
    g_vtMaxBobAmp.Init(g_pLTClient, "MaxBobAmp", NULL, 10.0f);

    g_vtRollAdjust.Init(g_pLTClient, "BobRollAdjust", NULL, 0.005f);
    g_vtHeadBobAdjust.Init(g_pLTClient, "HeadBob", NULL, 1.0f);
    g_vtWeaponSway.Init(g_pLTClient, "WeaponSway", NULL, 1.0f);

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::Update
//
//	PURPOSE:	Update all variables
//
// --------------------------------------------------------------------------- //

void CHeadBobMgr::Update()
{
	// Don't allow head bobbing or canting to update when paused...

	if( g_pGameClientShell->IsGamePaused() )
		return;

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

	// We check CanDoFootstep instead of on ground since CanDoFootstep
	// handles stairs much better...

	if ( (!g_pPlayerMgr->IsSpectatorMode() && g_pPlayerMgr->IsFirstPerson() && pMoveMgr->CanDoFootstep() &&
		 g_pPlayerMgr->IsPlayerMovementAllowed() && pMoveMgr->GetAllowMovement()) ||
		 pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() )
	{
		UpdateHeadBob();
		UpdateHeadCant();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::UpdateHeadBob
//
//	PURPOSE:	Adjusts the head bobbing & swaying
//
// --------------------------------------------------------------------------- //

void CHeadBobMgr::UpdateHeadBob()
{
	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

    LTBOOL bZoomed = g_pPlayerMgr->IsZoomed();

    uint32 dwPlayerFlags = g_pPlayerMgr->GetPlayerFlags();

	// This frame time is used since unlike ClientDE::GetFrameTime() the
	// max value is controlled by the game...

    LTFLOAT fFrameTime = g_pGameClientShell->GetFrameTime();

    LTFLOAT fTime      = g_pLTClient->GetTime();
    LTBOOL bRunning    = (LTBOOL) !!(dwPlayerFlags & BC_CFLG_RUN);
    LTFLOAT fMoveDist  = pMoveMgr->GetVelocity().Mag() * fFrameTime;

    LTBOOL bFootstep = LTFALSE;
	LTBOOL bLeftFoot = LTFALSE;
    LTFLOAT fPace = 0.0f;

	if (pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
	{
		fPace = MATH_CIRCLE * g_vtVehiclePaceAdjust.GetFloat();
	}
    else if (bRunning)
	{
		fPace = MATH_CIRCLE * g_vtRunPaceAdjust.GetFloat();
	}
	else
	{
		fPace = MATH_CIRCLE * g_vtWalkPaceAdjust.GetFloat();
	}

	// Make sure bob phase and sway phase start at the right values...

	if (m_fBobAmp == 0.0f)
	{
		m_fBobPhase  = 0.0f;
		m_fSwayPhase = 0.0f;
	}
	else  // Normal processing...
	{
		// Bob phase should be between MATH_PI and MATH_CIRCLE so that the
		// sin(m_fBobPhase) is always between -1 and 0...

		m_fBobPhase += (fFrameTime * fPace);

		if (m_fBobPhase > MATH_CIRCLE)
		{
			m_fBobPhase -= MATH_PI;
		}
		else if (m_fBobPhase < MATH_PI)
		{
			m_fBobPhase += MATH_PI;
		}

		m_fSwayPhase += (fFrameTime * fPace);

		if (m_fSwayPhase > MATH_CIRCLE)
		{
			m_fSwayPhase -= MATH_CIRCLE;
		}
	}


	// See if it is time to play a footstep sound...

	if ((m_fSwayPhase > MATH_CIRCLE * 0.25f) &&
		(m_fSwayPhase <= MATH_CIRCLE * 0.75f))
	{
		if (s_bCanDoLeftFootstep)
		{
			bLeftFoot = LTFALSE;
            bFootstep = LTTRUE;
            s_bCanDoLeftFootstep = LTFALSE;
            s_bCanDoRightFootstep = LTTRUE;
		}
	}
	else if (m_fSwayPhase > MATH_CIRCLE * 0.75f)
	{
		if (s_bCanDoRightFootstep)
		{
			bLeftFoot = LTTRUE;
            bFootstep = LTTRUE;
            s_bCanDoLeftFootstep = LTTRUE;
            s_bCanDoRightFootstep = LTFALSE;
		}
	}


    LTBOOL bMoving = LTFALSE;
    LTFLOAT t;

	uint32 dwTestFlags = (BC_CFLG_MOVING); // | BC_CFLG_DUCK);
	if (fMoveDist > 0.1f)
	{
		bMoving = !!(dwPlayerFlags & dwTestFlags);
	}


	// If we're not moving, decay the head bob...

	if (!bMoving)
	{
		s_fBobStartTime = -1.0f;

		if (s_fBobDecayStartTime < 0.0f)
		{
			// Calculate what the current bobamp percent is...

			t = (1.0f - m_fBobAmp / g_vtMaxBobAmp.GetFloat());

			s_fBobDecayStartTime = fTime - (g_vtBobDecayTime.GetFloat() * t);
		}

        LTFLOAT fDur = (fTime - s_fBobDecayStartTime);
		if (fDur <= g_vtBobDecayTime.GetFloat())
		{
			t = fDur / g_vtBobDecayTime.GetFloat();	// 0 to 1
			t = WaveFn_SlowOff(t);
			t = 1.0f - t;				// 1 to 0

			m_fBobAmp = t * g_vtMaxBobAmp.GetFloat();

			if (m_fBobAmp < 0.0f)
			{
				m_fBobAmp = 0.0f;
			}
		}
		else
		{
			m_fBobAmp = 0.0f;
		}
	}
	else  // We're moving...
	{
		s_fBobDecayStartTime = -1.0f;

		// If we just started bobing, ramp up the bob...

		if (s_fBobStartTime < 0.0f)
		{
			// Calculate what the current bobamp percent is...

			t = m_fBobAmp / g_vtMaxBobAmp.GetFloat();

			s_fBobStartTime = fTime - (g_vtBobDecayTime.GetFloat() * t);
		}

        LTFLOAT fDur = (fTime - s_fBobStartTime);
		if (fDur <= g_vtBobDecayTime.GetFloat())
		{
			t = fDur / g_vtBobDecayTime.GetFloat();	// 0 to 1
			t = WaveFn_SlowOn(t);

			m_fBobAmp = t * g_vtMaxBobAmp.GetFloat();

			if (m_fBobAmp > g_vtMaxBobAmp.GetFloat())
			{
				m_fBobAmp = g_vtMaxBobAmp.GetFloat();
			}
		}
		else
		{
			m_fBobAmp = g_vtMaxBobAmp.GetFloat();
		}
	}


	// Update the bob...

	if (!bZoomed)
	{
		m_fBobHeight = g_vtBobV.GetFloat() * m_fBobAmp * (float)sin(m_fBobPhase);
	}


	// Update the weapon model bobbing...

	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	if ( pClientWeapon && !bZoomed )
	{
		LTFLOAT fSwayHeight = g_vtSwayV.GetFloat() * m_fBobAmp * (float)sin(m_fSwayPhase * 2);
		LTFLOAT fSwayWidth  = g_vtSwayH.GetFloat() * m_fBobAmp * (float)sin(m_fSwayPhase - (MATH_PI/3));

		// No weapon bob if vehicle mode...

		if (pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
		{
			fSwayWidth = fSwayHeight = 0.0f;
		}

		fSwayHeight *= g_vtWeaponSway.GetFloat();
		fSwayWidth *= g_vtWeaponSway.GetFloat();

		pClientWeapon->UpdateBob(fSwayWidth, fSwayHeight);
	}


	// Update the head cant...

	if (!bZoomed && !pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
	{
		LTFLOAT fRollAdjust = g_vtRollAdjust.GetFloat() * (float)sin(m_fSwayPhase);

		// Turn head bob up/down...

		fRollAdjust *= g_vtHeadBobAdjust.GetFloat();

		if (m_fBobAmp == 0.0f)
		{
			fRollAdjust = 0.0f;
		}

		g_pPlayerMgr->SetRoll(fRollAdjust);
	}


	// Play foot step sounds at the appropriate time...

	if (bMoving && bFootstep)
	{
		CCharacterFX* pCharFX = pMoveMgr->GetCharacterFX();
		if (pCharFX)
		{
			SurfaceType eSurf = pMoveMgr->GetStandingOnSurface();
			eSurf = (eSurf == ST_UNKNOWN ? pCharFX->GetLastSurface() : eSurf);

			LTVector vPos;
			g_pLTClient->GetObjectPos(pMoveMgr->GetObject(), &vPos);
			pCharFX->PlayMovementSound(vPos, eSurf, bLeftFoot);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::AdjustCameraPos()
//
//	PURPOSE:	Adjust the camera's bob position...
//
// ----------------------------------------------------------------------- //

void CHeadBobMgr::AdjustCameraPos(LTVector &vPos)
{
	vPos.y += m_fBobHeight * g_vtHeadBobAdjust.GetFloat();
}

// Limits the angle to a -PI to PI range.
static inline float LimitToPosNegPi( float fAngle )
{
	// Copy the angle and make sure it's under 2 pi.
	float fNewAngle = ( float )fmod( fAngle, MATH_CIRCLE );

	if( fNewAngle > MATH_PI )
		fNewAngle = fNewAngle - MATH_CIRCLE;
	else if( fNewAngle < -MATH_PI )
		fNewAngle = fNewAngle + MATH_CIRCLE;
	
	return fNewAngle;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHeadBobMgr::UpdateHeadCant()
//
//	PURPOSE:	Update head tilt when strafing
//
// ----------------------------------------------------------------------- //

void CHeadBobMgr::UpdateHeadCant()
{
	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

	CVehicleMgr* pVehicleMgr = pMoveMgr->GetVehicleMgr();
	if (!pVehicleMgr) return;

	LTBOOL bVehicleTurning	= pVehicleMgr->IsTurning();
	int    nVehicleTurnDir	= pVehicleMgr->GetTurnDirection();

	CantType eCantType = eCantNone;
	LTFLOAT fMaxCant = 0.0f, fCantRate = 0.0f;
	
	if (bVehicleTurning && pMoveMgr->CanDoFootstep())
	{
		eCantType = nVehicleTurnDir > 0 ? eCantRight : eCantLeft;
	}


	fMaxCant = DEG2RAD(g_vtMaxVehicleHeadCant.GetFloat());
	LTFLOAT fMinCant = fMaxCant * 0.25f;

	LTFLOAT fRoll = LimitToPosNegPi( g_pPlayerMgr->GetRoll() - pVehicleMgr->GetVehicleContourRoll() );

	switch (eCantType)
	{
		case eCantRight :
		{
			if (fRoll > 0.0f)
			{
				fCantRate = g_vtVehicleHeadCantDownRate.GetFloat();
			}
			else
			{
				fCantRate = g_vtVehicleHeadCantUpRate.GetFloat();
			}
		}
		break;
		case eCantLeft :
		{
			if (fRoll < 0.0f)
			{
				fCantRate = g_vtVehicleHeadCantDownRate.GetFloat();
			}
			else
			{
				fCantRate = g_vtVehicleHeadCantUpRate.GetFloat();
			}
		}
		break;
		case eCantNone:
		{
			fCantRate = g_vtVehicleHeadCantDownRate.GetFloat();
		}
		break;

		default :
		break;
	}

	if (fabs(fRoll) < fMinCant)
	{
		fCantRate *= 0.5f;
	}

	// This frame time is used since unlike ClientDE::GetFrameTime() the
	// max value is controlled by the game...

	LTFLOAT fFrameTime = g_pGameClientShell->GetFrameTime();

	LTFLOAT fDelta = fCantRate * fFrameTime;

	switch (eCantType)
	{
		case eCantRight :
		{
			fRoll -= fDelta;

			if (fRoll < -fMaxCant)
			{
				fRoll = -fMaxCant;
			}
		}
		break;

		case eCantLeft :
		{
			fRoll += fDelta;

			if (fRoll > fMaxCant)
			{
				fRoll = fMaxCant;
			}
		}
		break;

		case eCantNone:
		default :
		{
			// We are not canting so move us toward zero...

			if (fRoll != 0.0f)
			{
				if (fRoll < 0.0f)
				{
					fRoll += fDelta;

					if (fRoll > 0.0f)
					{
						fRoll = 0.0f;
					}
				}
				else
				{
					fRoll -= fDelta;

					if (fRoll < 0.0f)
					{
						fRoll = 0.0f;
					}
				}
 			}
		}
		break;
	}

	// Let the vehicle mgr adjust the value...

	pVehicleMgr->AdjustCameraRoll(fRoll);

	// Set the new value...

	g_pPlayerMgr->SetRoll(fRoll);
}