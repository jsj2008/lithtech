// ----------------------------------------------------------------------- //
//
// MODULE  : SpecialMoveMgr.cpp
//
// PURPOSE : Manage the player's interaction with SpecialMoves
//
// CREATED : 02/07/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "SpecialMoveMgr.h"
#include "SpecialMoveFX.h"
#include "PlayerBodyMgr.h"
#include "PlayerMgr.h"
#include "PlayerCamera.h"
#include "CMoveMgr.h"
#include "ClientWeaponMgr.h"
#include "ClientDB.h"
#include "ActivationData.h"

VarTrack g_vtSpecialMoveInterpolationDelay;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::SpecialMoveMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SpecialMoveMgr::SpecialMoveMgr()
	: m_pSpecialMove(NULL)
	, m_pLastLookedAt(NULL)
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::~SpecialMoveMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SpecialMoveMgr::~SpecialMoveMgr()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::Init
//
//	PURPOSE:	set up instance for use
//
// ----------------------------------------------------------------------- //

void SpecialMoveMgr::Init()
{
	if (!g_vtSpecialMoveInterpolationDelay.IsInitted())
	{
		g_vtSpecialMoveInterpolationDelay.Init(g_pLTClient, "SpecialMoveInterpolationDelay", 0, ClientDB::Instance().GetFloat(ClientDB::Instance().GetClientSharedRecord(), "SpecialMove.0.InterpolationDelay"));
	}

	Reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::Init
//
//	PURPOSE:	reset climbing state
//
// ----------------------------------------------------------------------- //

void SpecialMoveMgr::Reset()
{
	m_pSpecialMove = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::Update
//
//	PURPOSE:	update the specialmove state
//
// ----------------------------------------------------------------------- //

void SpecialMoveMgr::Update()
{
	//if we're not attached, nothing to do
	if (!m_pSpecialMove)
		return;

	// Interpolate into position
	if (m_bInterpolate)
	{
		LTVector vPlayerPos;
		g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPlayerPos);

		// Stop when we're close enough
		if (vPlayerPos.NearlyEquals(m_vDesiredPos, 1.0f))
		{
			m_bInterpolate = false;
		}
		else
		{
			float fPct = LTCLAMP(ObjectContextTimer(g_pMoveMgr->GetObject()).GetTimerElapsedS() / g_vtSpecialMoveInterpolationDelay.GetFloat(), 0.0f, 1.0f);
			vPlayerPos += (m_vDesiredPos - vPlayerPos) * fPct;
			g_pLTClient->SetObjectPos(g_pMoveMgr->GetObject(), vPlayerPos);

			return;
		}
	}

	// Wait until we've finishing putting the weapon away...
	// (this might not be necessary because of the anim checks below, but since the animation takes a frame to kick in,
	// if we're not interpolating we might get into a situation where the animation starts before the weapon change finishes)
	if (m_pSpecialMove->ShouldDisableWeapons())
	{
		CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		if (pWeapon)
		{
			if (pWeapon->GetWeaponRecord() != g_pClientWeaponMgr->GetDefaultWeapon())
				return;

			if (pWeapon->GetState() == W_SELECT)
				return;
		}
	}

	// See if what animation we're playing
	EnumAnimProp eActionProp = CPlayerBodyMgr::Instance().GetCurrentAnimProp(CPlayerBodyMgr::kUpperContext, kAPG_Action);		//!!ARL: Assumes upper context! (bad)

	if (m_pSpecialMove->m_sStimulus.empty())
	{
		// Wait till we're done animating...
		if ((eActionProp != kAP_None) && CPlayerBodyMgr::Instance().IsLocked(kAPG_Action, eActionProp))
		{
			return;
		}

		// See if we just finished our special animation
		if (eActionProp == m_pSpecialMove->m_eAnimation)
		{
			Release();
			return;
		}

	}
	// Do the same for the conditional animation controller...
	else
	{
		if (CPlayerBodyMgr::Instance().ActiveAnimationStimulus())
		{
			return;
		}
		else if (m_bAnimStarted)
		{
			Release();
			return;
		}
	}

	// Start playing our special animation...
	if (eActionProp == kAP_None)
	{
		if (!m_pSpecialMove->m_sStimulus.empty())
		{
			CPlayerBodyMgr::Instance().HandleAnimationStimulus(m_pSpecialMove->m_sStimulus.c_str());
		}
		else if (m_pSpecialMove->m_eAnimation != kAP_None)
		{
			CPlayerBodyMgr::Instance().SetAnimProp(kAPG_Action, m_pSpecialMove->m_eAnimation, CPlayerBodyMgr::kLocked);
		}
		else
		{
			LTERROR("No animation specified!");
		}

		m_bAnimStarted = true;

		// and make sure we're facing the proper direction.
		if (m_pSpecialMove->ShouldPositionPlayer())
		{
			LTRotation rRot = m_pSpecialMove->m_rRot;
			LTVector vUp = rRot.Up();
			rRot.Rotate(vUp, MATH_PI);	//rotate halfway around

			g_pLTClient->SetObjectRotation(g_pMoveMgr->GetObject(), rRot);
		}

		return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::CanReach
//
//	PURPOSE:	can the player reach the SpecialMove object
//
// ----------------------------------------------------------------------- //

bool SpecialMoveMgr::CanReach(CSpecialMoveFX* pSpecialMove) const
{
	if (!pSpecialMove)
		return false;

	if (!pSpecialMove->IsEnabled())
		return false;

	//can't activate while jumping/falling
	if (g_pMoveMgr->IsFalling() && !g_pMoveMgr->IsBodyInLiquid())
		return false;

	//can't activate when you are already climbing
	if (m_pSpecialMove)
		return false;

	return pSpecialMove->CanReach();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::CanLookAt
//
//	PURPOSE:	can the player look at the SpecialMove object
//
// ----------------------------------------------------------------------- //

bool SpecialMoveMgr::CanLookAt(CSpecialMoveFX* pSpecialMove) const
{
	if (!pSpecialMove)
		return false;

	//can't look at while jumping/falling
	if (g_pMoveMgr->IsFalling() && !g_pMoveMgr->IsBodyInLiquid())
		return false;

	//can't look at when you are already special moving
	if (m_pSpecialMove)
		return false;

	return pSpecialMove->CanLookAt();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::HandleLookedAt
//
//	PURPOSE:	send a message to the server when we're looked at so the
//				server can send a command
//
// ----------------------------------------------------------------------- //

void SpecialMoveMgr::HandleLookedAt(CSpecialMoveFX* pSpecialMove)
{
	if (pSpecialMove && !pSpecialMove->CanLookAt())
		pSpecialMove = NULL;

	if (!pSpecialMove)
	{
		if (m_pLastLookedAt)
		{
			m_pLastLookedAt->OnUnLookedAt();
			m_pLastLookedAt = NULL;
		}
		return;
	}

	if (pSpecialMove == m_pLastLookedAt)
	{
		return;
	}

	pSpecialMove->OnLookedAt();
	m_pLastLookedAt = pSpecialMove;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SpecialMoveMgr::Activate
//
//	PURPOSE:	attempt to perform the given SpecialMove
//
// ----------------------------------------------------------------------- //

bool SpecialMoveMgr::Activate(CSpecialMoveFX* pSpecialMove)
{
	if (!pSpecialMove)
		return false;

	LTVector vAccel(0,0,0);
	g_pPhysicsLT->SetAcceleration(g_pMoveMgr->GetObject(), vAccel);
	g_pPhysicsLT->SetVelocity(g_pMoveMgr->GetObject(), vAccel);

	if (pSpecialMove->ShouldPositionPlayer())
	{
		LTVector vPlayerPos;
		g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPlayerPos);
		float fInitialY = vPlayerPos.y;

		LTVector vNormal = pSpecialMove->m_rRot.Forward();
		LTVector vClosest = pSpecialMove->m_bRadial ? pSpecialMove->m_vPos
			: (vPlayerPos - (vNormal * LTPlane(vNormal, pSpecialMove->m_vPos).DistTo(vPlayerPos)));

		m_vDesiredPos = (vClosest + (vNormal * pSpecialMove->m_fActivateDist));
		m_vDesiredPos.y = fInitialY; //ignore vertical
		m_bInterpolate = true;
	}

	m_pSpecialMove = pSpecialMove;

	m_bAnimStarted = false;

	if (m_pSpecialMove->ShouldDisableWeapons())
	{
		CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		if (pWeapon)
		{
			// We don't want to accidently fire off a round or two coming out of a SpecialMove...
			pWeapon->ClearFiring();

			// Temporarily switch to the hands if needed...
			HWEAPON hDefault = g_pClientWeaponMgr->GetDefaultWeapon();
			g_pClientWeaponMgr->ChangeWeapon(hDefault);
		}
	}

	// Send activate command...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_OBJECT_MESSAGE );
	cMsg.WriteObject( m_pSpecialMove->GetServerObj() );
	cMsg.Writeuint32( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SPECIALMOVEFX_ACTIVATED );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

void SpecialMoveMgr::Release()
{
	if( !m_pSpecialMove )
	{
		return;
	}

	// Send release command...
	{
		m_pSpecialMove->OnReleased();

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_OBJECT_MESSAGE );
		cMsg.WriteObject( m_pSpecialMove->GetServerObj() );
		cMsg.Writeuint32( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SPECIALMOVEFX_RELEASED );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

		// Switch back to previous weapon...
		if (m_pSpecialMove->ShouldDisableWeapons())
		{
			g_pClientWeaponMgr->LastWeapon();
		}
	}

	m_pSpecialMove = NULL;

	CActivationData data;
	data.m_hTarget = NULL;
	data.m_nType = MID_ACTIVATE_SPECIALMOVE;

	//notify server
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_ACTIVATE);
		data.Write(cMsg);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}

