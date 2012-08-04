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
	#include "CommandIDs.h"
	#include "MsgIds.h"
	#include "LeanMgr.h"
	#include "InterfaceMgr.h"
	#include "PlayerCamera.h"
	#include "VehicleMgr.h"
	#include "LTEulerAngles.h"
	#include "PlayerLureFX.h"
	#include "bindmgr.h"
	#include "PlayerBodyMgr.h"
	#include "LadderMgr.h"

//
// Globals...
//

	VarTrack	g_vtLeanOutTime;
	VarTrack	g_vtLeanCenterTime;
	VarTrack	g_vtLeanAngle;
	VarTrack	g_vtLeanCameraWeight;

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
	m_bDoneMoving			( false ),
	m_hObject				( NULL )
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

void CLeanMgr::Init( HOBJECT hObject, ModelsDB::HMODEL hModel )
{
	if( !hObject )
		return;

	ModelsDB::HLEAN hLeanRecord = ModelsDB::Instance( ).GetModelLeanRecord( hModel );
	if( !hLeanRecord )
		return;
	
	m_hObject = hObject;

	// Read the database values into console vars for tweaking during development...
	float fLeanAngle = ModelsDB::Instance( ).GetLeanAngle( hLeanRecord );
	float fLeanOutTime = ModelsDB::Instance( ).GetLeanOutTime( hLeanRecord ) / 1000.0f;
	float fLeanCenterTime = ModelsDB::Instance( ).GetLeanCenterTime( hLeanRecord ) / 1000.0f;
	float fLeanCameraWeight = ModelsDB::Instance( ).GetLeanCameraWeight( hLeanRecord );

	g_vtLeanAngle.Init( g_pLTClient, "LeanAngle", NULL, fLeanAngle );
	g_vtLeanOutTime.Init( g_pLTClient, "LeanOutTime", NULL, fLeanOutTime );
	g_vtLeanCenterTime.Init( g_pLTClient, "LeanCenterTime", NULL, fLeanCenterTime );
	g_vtLeanCameraWeight.Init( g_pLTClient, "LeanCameraWeight", NULL, fLeanCameraWeight );

	// Setup the lean node controller...
	m_LeanNodeController.Init( m_hObject, hLeanRecord );
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
	if( g_pGameClientShell->IsGamePaused( ) || !m_hObject )
		return;

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

	m_LeanNodeController.SetLeanAngle( m_fLeanAngle );
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

	if( g_pMoveMgr->GetVehicleMgr( )->IsVehiclePhysics( ))
	{
		PlayerLureFX* pPlayerLureFX = PlayerLureFX::GetPlayerLureFX( g_pMoveMgr->GetVehicleMgr( )->GetPlayerLureId( ));
		if( pPlayerLureFX )
		{
			if( !pPlayerLureFX->GetAllowLean( ))
				return;
		}
	}

	// We can only lean while on the ground...

	if( g_pMoveMgr->CanDoFootstep() && g_pInterfaceMgr->AllowCameraMovement() &&
		g_pPlayerMgr->IsPlayerAlive() && !LadderMgr::Instance().IsClimbing()
		&& !g_pPlayerMgr->IsOperatingTurret( )) 
	{
		if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_LEAN_LEFT ))
		{
			m_dwControlFlags |= BC_CFLG_LEAN_LEFT;
		}

		if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_LEAN_RIGHT ))
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

	m_fStartTime += ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );

	float	fT = (m_fStartTime / m_fEndTime);
	bool	bDone = CalcAngle( m_fLeanAngle, fLeanFromAngle, m_fMaxLeanAngle, m_kLeanDir, m_fEndTime, fT );

	// Save our last lean angle...
	m_fLastLeanAngle = m_fLeanAngle;

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
			cMsg.WriteLTVector( g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( ) );
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

	m_fStartTime += ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );

	float	fT = (m_fStartTime / m_fEndTime);
	bool	bDone = CalcAngle( m_fLeanAngle, fCenterFromAngle, 0.0f, eLeanDirection(-m_kLeanDir), m_fEndTime, fT );

	// Save our current lean angle...
	m_fLastLeanAngle = m_fLeanAngle;

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

	bool	bRet		= false;	// Are we at the target angle?

	if( fPercent > 1.0f ) 
	{
		fAngle = fTarget;
		bRet = true;
	}
	else
	{
	
		float	fRate		= ( fTarget - fInitial ) / fTotalTime;
		float	fAmount		= fRate * ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );


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
//  ROUTINE:	LeanFilterFn
//
//  PURPOSE:	Lean intersect seg filter function.
//
// ----------------------------------------------------------------------- //
static bool LeanFilterFn(HOBJECT hTest, void* /*pUserData*/)
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hTest, OFT_Flags, dwFlags);
	if(!(dwFlags & FLAG_RAYHIT))
	{
		return false;
	}

	if( hTest == g_pMoveMgr->GetObject( ))
		return false;

	if( hTest == g_pMoveMgr->GetServerObject( ))
		return false;

	if( hTest == g_pPlayerMgr->GetPlayerCamera( )->GetCamera( ))
		return false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLeanMgr::GetCameraLeanAngle
//
//  PURPOSE:	Calculate and retrieve the angle that should be applied to the camera while leaning...
//
// ----------------------------------------------------------------------- //

float CLeanMgr::GetCameraLeanAngle( ) const
{
	return (m_fLeanAngle * g_vtLeanCameraWeight.GetFloat( ));
}
