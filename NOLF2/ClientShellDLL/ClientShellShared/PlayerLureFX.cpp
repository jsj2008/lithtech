// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerLureFX.cpp
//
// PURPOSE : PlayerLure FX - Implementation
//
// CREATED : 01/28/02
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerLureFX.h"
#include "GameClientShell.h"
#include "CMoveMgr.h"

PlayerLureFX::PlayerLureFXList PlayerLureFX::m_lstPlayerLureFXs;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLureFX::PlayerLureFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PlayerLureFX::PlayerLureFX( )
{
	m_eCameraFreedom = kPlayerLureCameraFreedomNone;
	m_bAllowWeapon = false;
	m_bRetainOffsets = false;
	m_nPlayerLureId = 0;
	m_bCalcInitialOffset = true;

	// Add this instance to a list of all playerlurefx's.
	m_lstPlayerLureFXs.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLureFX::~PlayerLureFX
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

PlayerLureFX::~PlayerLureFX( )
{
	// Erase this instance from the list of all PlayerLureFX's.
	PlayerLureFXList::iterator it = m_lstPlayerLureFXs.begin( );
	while( it != m_lstPlayerLureFXs.end( ))
	{
		if( *it == this )
		{
			m_lstPlayerLureFXs.erase( it );
			break;
		}

		it++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLureFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL PlayerLureFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	char szString[256];

	// The actual HOBJECT may not be on the client yet.  We pass the id so we can
	// poll for it later.
	m_nPlayerLureId = pMsg->Readuint32( );
	m_eCameraFreedom = ( PlayerLureCameraFreedom )pMsg->Readuint8( );
	BYTE nFlags = pMsg->Readuint8( );
	pMsg->ReadString( szString, ARRAY_LEN( szString ));
	m_sDeathFX = szString;
	m_bAllowWeapon = !!( nFlags & ePlayerLureFlagsAllowWeapon );
	m_bRetainOffsets = !!( nFlags & ePlayerLureFlagsRetainOffsets );
	m_bBicycle = !!( nFlags & ePlayerLureFlagsBicycle );
	if( m_eCameraFreedom == kPlayerLureCameraFreedomLimited )
	{
		m_fLimitedYawLeft = pMsg->Readfloat( );
		m_fLimitedYawRight = pMsg->Readfloat( );
		m_fLimitedPitchDown = pMsg->Readfloat( );
		m_fLimitedPitchUp = pMsg->Readfloat( );
	}
	

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLureFX::Reset
//
//	PURPOSE:	Resets the lure when the player is commanded to follow it.
//
// ----------------------------------------------------------------------- //

bool PlayerLureFX::Reset( )
{
	// Get the client player object.
	HOBJECT hObj = g_pMoveMgr->GetObject( );
	if( !hObj )
	{
		ASSERT( !"PlayerLureFX::GetOffsetTransform:  Missing client-side player object." );
		return false;
	}

	// Get the client player's position.
	LTransform playerTransform;
	g_pLTClient->GetObjectPos( hObj, &playerTransform.m_Pos );
	g_pLTClient->GetObjectRotation( hObj, &playerTransform.m_Rot );
	playerTransform.m_Scale.Init( 1.0f, 1.0f, 1.0f );

	// Get the playerlure's transform.
	LTransform playerLureTransform;
	g_pLTClient->GetObjectPos( GetServerObj( ), &playerLureTransform.m_Pos );
	g_pLTClient->GetObjectRotation( GetServerObj( ), &playerLureTransform.m_Rot );
	playerLureTransform.m_Scale.Init( 1.0f, 1.0f, 1.0f );

	// Find the offset transform.
	ILTTransform *pTransformLT = g_pLTClient->GetTransformLT();
	pTransformLT->Difference( m_offsetTransform, playerTransform, playerLureTransform );
	m_offsetTransform.m_Scale.Init( 1.0f, 1.0f, 1.0f );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLureFX::GetPlayerLureFX
//
//	PURPOSE:	Gets the playerlurefx object given a playerlureid it's associated with.
//
// ----------------------------------------------------------------------- //

PlayerLureFX* PlayerLureFX::GetPlayerLureFX( uint32 nPlayerLureId )
{
	PlayerLureFX* pPlayerLureFX = NULL;

	// Find the id in the list of playerlurefx's.
	PlayerLureFXList::iterator it = m_lstPlayerLureFXs.begin( );
	while( it != m_lstPlayerLureFXs.end( ))
	{
		if( ( *it )->GetPlayerLureId( ) == nPlayerLureId )
		{
			pPlayerLureFX = *it;
		}

		it++;
	}

	return pPlayerLureFX;
}