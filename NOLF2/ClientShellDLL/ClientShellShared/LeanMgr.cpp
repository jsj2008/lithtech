// ----------------------------------------------------------------------- //
//
// MODULE  : LeanMgr.cpp
//
// PURPOSE : Lean mgr - Implementation
//
// CREATED : 1/8/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "PlayerMgr.h"
	#include "CMoveMgr.h"
	#include "MsgIds.h"
	#include "LeanMgr.h"
	#include "InterfaceMgr.h"
	#include "PlayerCamera.h"
	#include "VehicleMgr.h"

//
// Globals...
//

	VarTrack	g_vtLeanOutTime;
	VarTrack	g_vtLeanCenterTime;
	VarTrack	g_vtLeanRadius;
	VarTrack	g_vtLeanAngle;
	VarTrack	g_vtLeanCamClipDist;

	extern	VarTrack g_vtCameraClipDistance;
	extern CMoveMgr* g_pMoveMgr;


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::CLeanMgr
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CLeanMgr::CLeanMgr( )
:	m_kLeanDir				( kLean_Center ),
	m_dwControlFlags		( 0 ),
	m_dwLastControlFlags	( 0 ),
	m_vRotationPt			( 0.0f, 0.0f, 0.0f ),
	m_vRotationPtOffset		( 0.0f, 0.0f, 0.0f ),
	m_rOrigCamRot			( 0.0f, 0.0f, 0.0f, 1.0f ),
	m_fLeanAngle			( 0.0f ),
	m_fLastLeanAngle		( 0.0f ),
	m_fMaxLeanAngle			( 0.0f ),
	m_fLeanFromAngle		( 0.0f ),
	m_fCenterFromAngle		( 0.0f ),
	m_fStartTime			( 0.0f ),
	m_fEndTime				( 0.0f ),
	m_bFailedToCenter		( false ),
	m_bFailedToLean			( false ),
	m_bLeanedOut			( false ),
	m_bDoneMoving			( false )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::~CLeanMgr
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CLeanMgr::~CLeanMgr( )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::Init
//
//  PURPOSE:	Initialize vars...
//
// ----------------------------------------------------------------------- //

void CLeanMgr::Init( )
{
	g_vtLeanOutTime.Init( g_pLTClient, "LeanOutTime", LTNULL, 0.5f );
	g_vtLeanCenterTime.Init( g_pLTClient, "LeanCenterTime", LTNULL, 0.5f );
	g_vtLeanRadius.Init( g_pLTClient, "LeanRadius", LTNULL, 250.0f );
	g_vtLeanAngle.Init( g_pLTClient, "LeanAngle", LTNULL, 3.0f );
	g_vtLeanCamClipDist.Init( g_pLTClient, "LeanCamClipDist", LTNULL, 20.0f );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::Update
//
//  PURPOSE:	Update the player lean...
//
// ----------------------------------------------------------------------- //

void CLeanMgr::Update( )
{

	if (g_pGameClientShell->IsGamePaused()) return;

	UpdateControlFlags();

	bool	bLeft = !!(m_dwControlFlags & BC_CFLG_LEAN_LEFT);
	bool	bRight = !!(m_dwControlFlags & BC_CFLG_LEAN_RIGHT);
	bool	bLastLeft = !!(m_dwLastControlFlags & BC_CFLG_LEAN_LEFT);
	bool	bLastRight = !!(m_dwLastControlFlags & BC_CFLG_LEAN_RIGHT);

	// Check to see if we should start leaning in a direction...

	if( bLeft && !bLastLeft )
	{
		BeginLean( kLean_Left );
	}
	else if( !bLeft && bLastLeft )
	{
		EndLean( kLean_Left );
	}


	if( bRight && !bLastRight )
	{
		BeginLean( kLean_Right );
	}
	else if( !bRight && bLastRight )
	{
		EndLean( kLean_Right );
	}

	// Should we snap back to the center or keep leaning...

	if( !bLeft && !bRight && (m_kLeanDir != kLean_Center) )
	{
		UpdateCenter( );
	}
	else
	{
		UpdateLean( );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::UpdateControlFlags
//
//  PURPOSE:	Update the command states...
//
// ----------------------------------------------------------------------- //

void CLeanMgr::UpdateControlFlags( )
{
	// Save and clear the flags from the last update...

	m_dwLastControlFlags = m_dwControlFlags;
	m_dwControlFlags = 0;

	// We can only lean while on the ground...

	if( g_pMoveMgr->CanDoFootstep() && g_pInterfaceMgr->AllowCameraMovement() &&
		!g_pPlayerMgr->IsPlayerDead() && !g_pMoveMgr->IsBodyOnLadder() && !g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() ) 
	{
		if( g_pLTClient->IsCommandOn( COMMAND_ID_LEAN_LEFT ))
		{
			m_dwControlFlags |= BC_CFLG_LEAN_LEFT;
		}

		if( g_pLTClient->IsCommandOn( COMMAND_ID_LEAN_RIGHT ))
		{
			m_dwControlFlags |= BC_CFLG_LEAN_RIGHT;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::BeginLean
//
//  PURPOSE:	Start leaning in a direction...
//
// ----------------------------------------------------------------------- //

void CLeanMgr::BeginLean( eLeanDirection kDir )
{
	m_kLeanDir = kDir;

	m_fMaxLeanAngle = DEG2RAD( g_vtLeanAngle.GetFloat() );
	m_fLeanFromAngle = m_fLastLeanAngle;

	m_fStartTime = 0.0f;
	m_fEndTime = g_vtLeanOutTime.GetFloat();

	if( m_bLeanedOut )
	{
		// Send a message to the server to remove the original stimulus.

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
		cMsg.Writeuint8( CP_PLAYER_LEAN );
		cMsg.Writeuint8( PL_CENTER );
		cMsg.WriteLTVector( LTVector( 0, 0, 0) );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}

	// If we are just begining to lean then we are not leaned out...

	m_bLeanedOut = false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::EndLean
//
//  PURPOSE:	End leaning in a direction...
//
// ----------------------------------------------------------------------- //

void CLeanMgr::EndLean( eLeanDirection kDir )
{

	if( m_bLeanedOut )
	{
		// Send a message to the server to remove the original stimulus.

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
		cMsg.Writeuint8( CP_PLAYER_LEAN );
		cMsg.Writeuint8( PL_CENTER );
		cMsg.WriteLTVector( LTVector( 0, 0, 0) );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}
	
	// If we are ending a lean we are not leaned out...

	m_bLeanedOut = false;
	

	bool	bLeft = !!(m_dwControlFlags & BC_CFLG_LEAN_LEFT);
	bool	bRight = !!(m_dwControlFlags & BC_CFLG_LEAN_RIGHT);
	
	if( bLeft || bRight )
	{
		// Readjust the lean...

		m_kLeanDir			= bLeft ? kLean_Left : kLean_Right;
		m_fLeanFromAngle	= m_fLastLeanAngle;
		m_fStartTime		= 0.0f;
		
		float fMovePercent = 1.0f - (float(m_kLeanDir) * (m_fLastLeanAngle / m_fMaxLeanAngle));
		m_fEndTime = g_vtLeanOutTime.GetFloat() * fMovePercent; 
	}
	else
	{
		BeginCenter();
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::BeginCenter
//
//  PURPOSE:	Setup values to snap back to center...
//
// ----------------------------------------------------------------------- //

void CLeanMgr::BeginCenter( )
{
	m_fCenterFromAngle = m_fLastLeanAngle;

	m_kLeanDir = (m_fCenterFromAngle > 0.0f ? kLean_Left : kLean_Right );

	m_fStartTime = 0.0f;
	m_fEndTime = g_vtLeanCenterTime.GetFloat();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::UpdateLean
//
//  PURPOSE:	Update the leaning motion...
//
// ----------------------------------------------------------------------- //

void CLeanMgr::UpdateLean( )
{
	if( !IsLeaning() ) return;


	// Don't start leaning untill we are done moving...

	if( !m_bDoneMoving && (g_pMoveMgr->GetMovementPercent() > MATH_EPSILON) )
	{
		BeginLean( m_kLeanDir );
	}
	else
	{
		m_bDoneMoving = true;
	}

	LTVector vCamPos;
	g_pLTClient->GetObjectPos( g_pPlayerMgr->GetCamera(), &vCamPos );
	
	// Develop the rotation values...

	m_vRotationPt.Init( vCamPos.x, vCamPos.y - g_vtLeanRadius.GetFloat(), vCamPos.z );
	m_vRotationPtOffset.Init( 0.0f, g_vtLeanRadius.GetFloat(), 0.0f );
	

	float fLeanFromAngle	= m_fLeanFromAngle;

	// Did we try to center but went back to leaning before we finished...

	if( m_bFailedToCenter )
	{
		m_bFailedToCenter = false;

		// Recalculate the time...

		if( m_fMaxLeanAngle > 0.0f )
		{
			float fMovePercent = 1.0f - (float(m_kLeanDir) * (m_fLastLeanAngle / m_fMaxLeanAngle));
			m_fEndTime = g_vtLeanOutTime.GetFloat() * fMovePercent; 
		}

		m_fStartTime	= 0.0f;
		fLeanFromAngle	= m_fLastLeanAngle;
	}

	// Find the angle based on the percentage of lean we should be at...

	m_fStartTime += g_pLTClient->GetFrameTime();

	float	fT = (m_fStartTime / m_fEndTime);
	bool	bDone = CalcAngle( m_fLeanAngle, fLeanFromAngle, m_fMaxLeanAngle, m_kLeanDir, m_fEndTime, fT );

	// Save our last lean angle...

	m_fLastLeanAngle = m_fLeanAngle;

	LTRotation	rRot;
	LTVector	vPos;
	CalculateNewPosRot( vPos, rRot, m_fLeanAngle );

	// [KLS 3/22/03] Only adjust the camera in first person...

	if (g_pPlayerMgr->IsFirstPerson())
	{
		g_pLTClient->SetObjectPosAndRotation( g_pPlayerMgr->GetCamera(), &vPos, &rRot );
	}

	if( bDone )
	{
		if( !m_bLeanedOut )
		{
			// We are completely leaned out.

			m_bLeanedOut = true;

			// Send a message to the server to register a stimulus.

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
			cMsg.Writeuint8( CP_PLAYER_LEAN );
			cMsg.Writeuint8( m_kLeanDir == kLean_Left ? PL_LEFT : PL_RIGHT );
			cMsg.WriteLTVector( vPos );
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		}

		m_bFailedToLean = false;
		return;
	}

	m_bFailedToLean = true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::UpdateCenter
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void CLeanMgr::UpdateCenter( )
{
	float fCenterFromAngle	= m_fCenterFromAngle;

	LTVector vCamPos;
	g_pLTClient->GetObjectPos( g_pPlayerMgr->GetCamera(), &vCamPos );
	
	// Develop the rotation values...

	m_vRotationPt.Init( vCamPos.x, vCamPos.y - g_vtLeanRadius.GetFloat(), vCamPos.z );
	m_vRotationPtOffset.Init( 0.0f, g_vtLeanRadius.GetFloat(), 0.0f );

	// Did we try to lean but went back to centering before we finished...

	if( m_bFailedToLean )
	{
		m_bFailedToLean = false;

		// Recalculate the time...

		if( m_fMaxLeanAngle > 0.0f )
		{
			float fMovePercent = float(m_kLeanDir) * (m_fLastLeanAngle / m_fMaxLeanAngle);
			m_fEndTime = g_vtLeanCenterTime.GetFloat() * fMovePercent; 
		}

		m_fStartTime		= 0.0f;
		fCenterFromAngle	= m_fLastLeanAngle;
	}

	// Find the angle based on the percentage of lean we should be at...

	m_fStartTime += g_pLTClient->GetFrameTime();

	float	fT = (m_fStartTime / m_fEndTime);
	bool	bDone = CalcAngle( m_fLeanAngle, fCenterFromAngle, 0.0f, eLeanDirection(-m_kLeanDir), m_fEndTime, fT );
	
	// Save our current lean angle...

	m_fLastLeanAngle = m_fLeanAngle;

	LTRotation	rRot;
	LTVector	vPos;
	CalculateNewPosRot( vPos, rRot, m_fLeanAngle );

	// [KLS 3/22/03] Only adjust the camera in first person...

	if (g_pPlayerMgr->IsFirstPerson())
	{
		g_pLTClient->SetObjectPosAndRotation( g_pPlayerMgr->GetCamera(), &vPos, &rRot );
	}

	if( bDone )
	{
		// We are centered.
		
		m_kLeanDir = kLean_Center;
		m_bFailedToCenter = false;
		m_bDoneMoving = false;
		
		return;
	}

	m_bFailedToCenter = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::CalcAngle
//
//  PURPOSE:	Calculate the new value of fAngle 
//
// ----------------------------------------------------------------------- //

bool CLeanMgr::CalcAngle( float &fAngle, float fInitial, float fTarget, eLeanDirection kDir, float fTotalTime, float fPercent )
{
	// Adjust the target angle based on direction...

	fTarget *= (float)kDir;

	float	fLastAngle = fAngle;
	bool	bRet		= false;	// Are we at the target angle?

	if( fPercent > 1.0f ) 
	{
		fAngle = fTarget;
		bRet = true;
	}
	else
	{
	
		float	fRate		= ( fTarget - fInitial ) / fTotalTime;
		float	fAmount		= fRate * g_pLTClient->GetFrameTime();


		if( kDir != kLean_Center )
		{
			if( kDir == kLean_Right )
			{
				if( fAngle > fTarget )
				{
					fAngle += fAmount;
				}
				else
				{
					fAngle	= fTarget;
					bRet	= true;
				}
			}
			else
			{
				if( fAngle < fTarget )
				{
					fAngle += fAmount;
				}
				else
				{
					fAngle	= fTarget;
					bRet	= true;
				}
			}
		}
		else
		{
			bRet = true;
		}
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::CalculateNewPosRot
//
//  PURPOSE:	Calculate a new Position and Rotation based on current values
//
// ----------------------------------------------------------------------- //

void CLeanMgr::CalculateNewPosRot( LTVector &vOutPos, LTRotation &rOutRot, float fAngle )
{
	float fPitch	= g_pPlayerMgr->GetPitch();
	float fYaw		= g_pPlayerMgr->GetYaw();
	float fRoll		= g_pPlayerMgr->GetRoll();

	// Use the current camera angles and just add the new roll for the camera rotation...

	rOutRot = LTRotation( fPitch, fYaw, fRoll + fAngle );

	// Don't factor in the pitch so the position isn't affected by it...

	LTMatrix	mRotation;
	LTRotation	rRotation( 0.0f, fYaw, fRoll + fAngle );
	
	rRotation.ConvertToMatrix( mRotation );

	vOutPos = (mRotation * m_vRotationPtOffset) + m_vRotationPt;

	// Make sure we are actually in the world before we try to clip...

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	iQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	iQuery.m_From = m_vRotationPt + m_vRotationPtOffset;
	iQuery.m_To = vOutPos;

	if( g_pLTClient->IntersectSegment( &iQuery, &iInfo ))
	{
		vOutPos = iInfo.m_Point + iInfo.m_Plane.m_Normal;
	}

	// Keep us far away from walls to avoid clipping...
	
	float fCamClipDist = g_vtCameraClipDistance.GetFloat();
	g_vtCameraClipDistance.SetFloat( g_vtLeanCamClipDist.GetFloat() );

	g_pPlayerMgr->GetPlayerCamera()->CalcNonClipPos( vOutPos, rOutRot );

	g_vtCameraClipDistance.SetFloat( fCamClipDist );
}