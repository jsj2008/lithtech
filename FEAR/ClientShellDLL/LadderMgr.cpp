// ----------------------------------------------------------------------- //
//
// MODULE  : LadderMgr.cpp
//
// PURPOSE : Manage the player's interaction with ladders
//
// CREATED : 06/22/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "LadderMgr.h"
#include "LadderFX.h"
#include "PlayerBodyMgr.h"
#include "PlayerMgr.h"
#include "PlayerCamera.h"
#include "CMoveMgr.h"
#include "LTEulerAngles.h"
#include "ClientWeaponMgr.h"
#include "ClientDB.h"
#include "ActivationData.h"

VarTrack	g_vtLadderBottomDist;
VarTrack	g_vtLadderTopDist;
VarTrack	g_vtLadderApproachAngle;
VarTrack	g_vtLadderActivateDist;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::LadderMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

LadderMgr::LadderMgr( ) :
	m_pLadder (NULL),
	m_bRightHand (true),
	m_bStartTop(false),
	m_bSliding (false),
	m_hSlideSound (NULL),
	m_fRungHeight(0),
	m_fTopExtension(0),
	m_vPitchRange(0,0),
	m_vYawRange(0,0),
	m_vEntryPos(0,0,0),
	m_rEntryRot()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::~LadderMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

LadderMgr::~LadderMgr( )
{
	if (m_hSlideSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hSlideSound);
		m_hSlideSound = NULL;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::Init
//
//	PURPOSE:	set up instance for use
//
// ----------------------------------------------------------------------- //

void LadderMgr::Init()
{
	ClientDB& ClientDatabase = ClientDB::Instance();
	g_vtLadderBottomDist.Init( g_pLTClient, "LadderBottomDist", 0, ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.ApproachDistanceBottom" ) );
	g_vtLadderTopDist.Init( g_pLTClient, "LadderTopDist", 0, ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.ApproachDistanceTop" ) );
	g_vtLadderApproachAngle.Init( g_pLTClient, "LadderApproachAngle", 0, ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.ApproachAngle" ) );
	g_vtLadderActivateDist.Init(g_pLTClient, "LadderActivateDist", 0, ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.ActivateDistance" ) );

	m_fRungHeight = ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.RungHeight");
	m_fTopExtension = ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.TopExtension");

	m_vPitchRange.x = -DEG2RAD(ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.CameraPitchMin"));
	m_vPitchRange.y = -DEG2RAD(ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.CameraPitchMax"));
	m_vYawRange.x = DEG2RAD(ClientDatabase.GetFloat(ClientDatabase.GetClientSharedRecord(), "Ladder.0.CameraYawLimit"));
	m_vYawRange.y = -m_vYawRange.x;

	Reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::Init
//
//	PURPOSE:	reset climbing state
//
// ----------------------------------------------------------------------- //

void LadderMgr::Reset()
{
	m_pLadder = NULL;
	m_bRightHand = true;
	m_bStartTop = false;
	StartSliding(false);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::Update
//
//	PURPOSE:	update the climbing state
//
// ----------------------------------------------------------------------- //

void LadderMgr::Update()
{
	//if we're not attached, nothing to do
	if (!m_pLadder)
		return;

	// See what animation we're playing
	EnumAnimProp eActionProp = CPlayerBodyMgr::Instance( ).GetCurrentAnimProp( CPlayerBodyMgr::kUpperContext, kAPG_Action );
	//still animating...
	if( eActionProp != kAP_None && CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, eActionProp ))
	{
		return;
	}

	//done with current anim
	switch(eActionProp) 
	{
	case kAP_ACT_GetOnLadder:
		{
			LTRotation rRot = m_pLadder->GetRotation();
			//rotate halfway around
			LTVector vUp = rRot.Up();
			rRot.Rotate(vUp,MATH_PI);
			g_pLTClient->SetObjectRotation(g_pMoveMgr->GetObject(), rRot);
			g_pPlayerMgr->GetPlayerCamera()->SetCameraClamping(rRot,m_vPitchRange,m_vYawRange);
			m_bRightHand = true;
			if (IsAtTop()) 
			{
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_GetOffLadderTopLeft, CPlayerBodyMgr::kLocked );
				return;
			}

		}
	case kAP_ACT_GetOnLadderTop:
		{
			LTRotation rRot = m_pLadder->GetRotation();
			//rotate halfway around
			LTVector vUp = rRot.Up();
			rRot.Rotate(vUp,MATH_PI);
			g_pLTClient->SetObjectRotation(g_pMoveMgr->GetObject(), rRot);
			g_pPlayerMgr->GetPlayerCamera()->SetCameraClamping(rRot,m_vPitchRange,m_vYawRange);
			m_bRightHand = true;
		}
		break;
	case kAP_ACT_LadderUpRight:
		{
			if (IsAtTop()) 
			{
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_GetOffLadderTopLeft, CPlayerBodyMgr::kLocked );
				return;
			}
		}
		break;
	case kAP_ACT_LadderUpLeft:
		{
			if (IsAtTop()) 
			{
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_GetOffLadderTopRight, CPlayerBodyMgr::kLocked );
				return;
			}
		}
		break;
	case kAP_ACT_SlideDownLadder:
		{
			if (IsAtBottom()) 
			{
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_GetOffLadder, CPlayerBodyMgr::kLocked );
				return;
			}
			
			
		}
		break;

	case kAP_ACT_GetOffLadder:
	case kAP_ACT_GetOffLadderTopLeft:
	case kAP_ACT_GetOffLadderTopRight:
		{
			ReleaseLadder();
			return;
		}
		break;

	case kAP_ACT_LadderHoldRight:
	case kAP_ACT_LadderHoldLeft:
		//do nothing, but recognize as a ladder anim
		break;
	case kAP_None:
	default:
		{
			//teleport to attachment point, this must be done again 
			g_pLTClient->SetObjectTransform( g_pMoveMgr->GetObject(), LTRigidTransform(m_vEntryPos, m_rEntryRot) );

			//if we were not playing a ladder anim, then we are ready to start...
			if (m_bStartTop)
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_GetOnLadderTop, CPlayerBodyMgr::kLocked );
			else
				CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_GetOnLadder, CPlayerBodyMgr::kLocked );
			return;
		}
	}

	bool bGoingUp = !!(g_pMoveMgr->GetControlFlags() & BC_CFLG_FORWARD);
	bool bGoingDown = !!(g_pMoveMgr->GetControlFlags() & BC_CFLG_REVERSE);
	bool bSliding = false;
	

	if (bGoingUp) 
	{
		if (m_bRightHand)
		{
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_LadderUpRight, CPlayerBodyMgr::kLocked );
		}
		else
		{
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_LadderUpLeft, CPlayerBodyMgr::kLocked );
		}
		m_bRightHand = !m_bRightHand;
	}
	else if (bGoingDown)
	{
		CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_SlideDownLadder, CPlayerBodyMgr::kLocked );
		bSliding = true;
	}
	else
	{
		if (m_bRightHand)
		{
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_LadderHoldRight, CPlayerBodyMgr::kLocked );
		}
		else
		{
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_LadderHoldLeft, CPlayerBodyMgr::kLocked );
		}
	}

	if (bSliding != m_bSliding)
	{
		StartSliding(bSliding);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::CanReachLadder
//
//	PURPOSE:	can the player reach the ladder
//
// ----------------------------------------------------------------------- //

bool LadderMgr::CanReachLadder(CLadderFX *pLadder) const
{
	if (!pLadder)
		return false;

	//can't activate while jumping/falling
	if (g_pMoveMgr->IsFalling() && !g_pMoveMgr->IsBodyInLiquid())
		return false;

	//can't activate when you are already climbing
	if (m_pLadder)
		return false;

	LTVector vPos;
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPos);

	bool bAbove = (pLadder->GetTop().y < (vPos.y+m_fTopExtension));

	//we just want to compare (x,z) distances
	vPos.y = pLadder->GetBottom().y;
	if (pLadder->GetBottom().Dist(vPos) > g_vtLadderActivateDist.GetFloat())
		return false;

	//ok, we're close enough but are we on the right side...
	LTVector vOffset = (vPos - pLadder->GetBottom());
	vOffset.Normalize();
	
	float fAngle = vOffset.Dot(pLadder->GetRotation().Forward());

	if (bAbove)
		return (fAngle < -DEG2RAD(90.0f - g_vtLadderApproachAngle.GetFloat()));
	else
		return (fAngle > DEG2RAD(90.0f - g_vtLadderApproachAngle.GetFloat()));

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::ActivateLadder
//
//	PURPOSE:	attempt to climb the given ladder
//
// ----------------------------------------------------------------------- //

bool LadderMgr::ActivateLadder(CLadderFX *pLadder)
{
	if (!pLadder)
		return false;

	if	(g_pClientWeaponMgr->GetCurrentWeaponState() == W_RELOADING)
		return false;

	LTVector vAccel(0, 0, 0);
	g_pPhysicsLT->SetAcceleration(g_pMoveMgr->GetObject(), vAccel);
	g_pPhysicsLT->SetVelocity(g_pMoveMgr->GetObject(), vAccel);


	if( g_pMoveMgr->IsDucking())
	{
		// Make us stand up before activating the ladder.
		LTVector vStandingDims = g_pMoveMgr->GetStandingDims();
		g_pPhysicsLT->SetObjectDims(g_pMoveMgr->GetObject(), &vStandingDims, SETDIMS_PUSHOBJECTS);
	}
	LTVector vPlayerPos,vPlayerDims;
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPlayerPos);
	g_pPhysicsLT->GetObjectDims(g_pMoveMgr->GetObject(), &vPlayerDims);

	bool bAbove = (pLadder->GetTop().y < (vPlayerPos.y+m_fTopExtension));

	LTRotation rRot = pLadder->GetRotation();

	//rotate halfway around
	LTVector vUp = rRot.Up();
	rRot.Rotate(vUp,MATH_PI);

	if (bAbove)
	{
		//attaching from above

		//face the ladder
		EulerAngles euLadderAngles = Eul_FromQuat( pLadder->GetRotation(), EulOrdYXZr );
		EulerAngles euPlayerAngles = Eul_FromQuat( g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation(), EulOrdYXZr );
		float fYaw = euLadderAngles.x;
		float fPitch = euPlayerAngles.y;
		g_pPlayerMgr->GetPlayerCamera()->SetCameraRotation( LTRotation(fPitch,fYaw,0.0f));
//		g_pPlayerMgr->GetPlayerCamera()->SetCameraClamping(rRot,m_vPitchRange,m_vYawRange);


		// determine attachment point
		m_vEntryPos = pLadder->GetTop();
		m_vEntryPos += rRot.Forward() * g_vtLadderTopDist.GetFloat();
		m_vEntryPos.y += vPlayerDims.y;
		m_vEntryPos.y -= m_fTopExtension;

		m_rEntryRot = pLadder->GetRotation();

		m_bStartTop = true;
	}
	else
	{
		//attaching from below

		//face the ladder
		EulerAngles euLadderAngles = Eul_FromQuat( rRot, EulOrdYXZr );
		EulerAngles euPlayerAngles = Eul_FromQuat( g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation(), EulOrdYXZr );
		float fYaw = euLadderAngles.x;
		float fPitch = euPlayerAngles.y;
		g_pPlayerMgr->GetPlayerCamera()->SetCameraRotation( LTRotation(fPitch,fYaw,0.0f));
		g_pPlayerMgr->GetPlayerCamera()->SetCameraClamping(rRot,m_vPitchRange,m_vYawRange);


		// determine to attachment point
		m_vEntryPos = pLadder->GetBottom();

		//figure out where the players feet are.  Don't let the feet be lower than the bottom of the ladder.
		float fFootHeight = LTMAX((vPlayerPos.y - vPlayerDims.y) - m_vEntryPos.y, 0.0f );

		//find the nearest rung...
		fFootHeight = floorf(fFootHeight / m_fRungHeight) * m_fRungHeight;

		
		m_vEntryPos += pLadder->GetRotation().Forward() * g_vtLadderBottomDist.GetFloat();
		m_vEntryPos.y += fFootHeight + vPlayerDims.y;

		m_rEntryRot = rRot;

		m_bStartTop = false;
	}

	//teleport to attachment point
	g_pLTClient->SetObjectTransform( g_pMoveMgr->GetObject(), LTRigidTransform(m_vEntryPos, m_rEntryRot) );

	m_pLadder = pLadder;
	g_pClientWeaponMgr->DisableWeapons();

	// We don't want to accidentally fire off a round or two coming off of a ladder...
	CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if( pWeapon )
	{
		pWeapon->ClearFiring();
	}

	return true;

}

bool LadderMgr::CanReleaseLadder() const
{
	if( !m_pLadder )
	{
		return false;
	}

	// See what animation we're playing
	EnumAnimProp eActionProp = CPlayerBodyMgr::Instance( ).GetCurrentAnimProp( CPlayerBodyMgr::kUpperContext, kAPG_Action );
	if (eActionProp == kAP_ACT_GetOnLadder || eActionProp == kAP_ACT_GetOnLadderTop)
	{
		//return true, if 
		return (!CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, eActionProp ));
	}

	return true;

}

void LadderMgr::ReleaseLadder()
{
	if( !m_pLadder )
	{
		return;
	}

	g_pCommonLT->SetObjectFlags(g_pMoveMgr->GetObject(), OFT_Flags, FLAG_GRAVITY, FLAG_GRAVITY);
	m_pLadder = NULL;

	// Re-enable weapons when getting off ladders that are not underwater...
	if( !g_pPlayerMgr->IsUnderwater( ))
		g_pClientWeaponMgr->EnableWeapons();

	g_pPlayerMgr->GetPlayerCamera()->ClearCameraClamping();


	CActivationData data;
	data.m_hTarget = NULL;
	data.m_nType = MID_ACTIVATE_LADDER;

	//notify server
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_ACTIVATE);
	data.Write(cMsg);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	StartSliding(false);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::IsAtTop
//
//	PURPOSE:	is the player at the top of the ladder
//
// ----------------------------------------------------------------------- //

bool LadderMgr::IsAtTop() const
{
	if (!m_pLadder)
		return false;

	LTVector vPos,vPlayerDims;
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPos);
	g_pPhysicsLT->GetObjectDims(g_pMoveMgr->GetObject(), &vPlayerDims);

	float fTopOfDims = vPos.y + vPlayerDims.y;
	float fTopOfLadder = m_pLadder->GetTop().y - m_fTopExtension;

	return (fTopOfDims >= fTopOfLadder);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::IsAtBottom
//
//	PURPOSE:	is the player at the bottom of the ladder
//
// ----------------------------------------------------------------------- //

bool LadderMgr::IsAtBottom() const
{
	if (!m_pLadder)
		return false;

	LTVector vPos,vPlayerDims;
	g_pLTClient->GetObjectPos(g_pMoveMgr->GetObject(), &vPos);
	g_pPhysicsLT->GetObjectDims(g_pMoveMgr->GetObject(), &vPlayerDims);

	float fFootHeight = vPos.y - vPlayerDims.y;
	float fBottomRung = m_pLadder->GetBottom().y + m_fRungHeight;

	return (fFootHeight < fBottomRung);

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::StartSliding
//
//	PURPOSE:	start/stop sliding sound
//
// ----------------------------------------------------------------------- //

void LadderMgr::StartSliding( bool bSliding )
{
	m_bSliding = bSliding;
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
	cMsg.Writeuint8( CP_LADDER_SLIDE );
	cMsg.Writebool( bSliding );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

	if (bSliding)
	{
		HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord("Ladder_Slide");
		if (hSoundRec)
		{
			m_hSlideSound = g_pClientSoundMgr->PlayDBSoundLocal( hSoundRec, SOUNDPRIORITY_PLAYER_LOW, (PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENT | PLAYSOUND_LOOP) );
		}
	}
	else
	{
		if (m_hSlideSound)
		{
			g_pLTClient->SoundMgr()->KillSound(m_hSlideSound);
			m_hSlideSound = NULL;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LadderMgr::IsDismounting
//
//	PURPOSE:	is the player playing one of the "Get off the ladder" animations
//
// ----------------------------------------------------------------------- //

bool LadderMgr::IsDismounting() const
{
	//if we're not attached, we're not dismounting
	if (!m_pLadder)
		return false;

	// See what animation we're playing
	EnumAnimProp eActionProp = CPlayerBodyMgr::Instance( ).GetCurrentAnimProp( CPlayerBodyMgr::kUpperContext, kAPG_Action );

	return  (  (eActionProp == kAP_ACT_GetOffLadder) ||
				(eActionProp == kAP_ACT_GetOffLadderTopLeft) ||
				(eActionProp == kAP_ACT_GetOffLadderTopRight)
			);

}
