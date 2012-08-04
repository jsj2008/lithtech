// ****************************************************************************************** //
//
// MODULE  : HeadBobMgr.cpp
//
// PURPOSE : Implementation of Head Bob Manager
//
// CREATED : 08/23/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#include "stdafx.h"

#ifndef __HEADBOBMGR_H__
#include "HeadBobMgr.h"
#endif//__HEADBOBMGR_H__

#ifndef __CLIENTDB_H__
#include "ClientDB.h"
#endif//__CLIENTDB_H__

#include "CMoveMgr.h"


// ****************************************************************************************** //

VarTrack g_vtHeadBobDebugMode;
VarTrack g_vtHeadBobSpeedScale;

VarTrack g_vtHeadBobWaveMin[ HeadBobMgr::HBE_COUNT ];
VarTrack g_vtHeadBobWaveMax[ HeadBobMgr::HBE_COUNT ];
VarTrack g_vtHeadBobAmp[ HeadBobMgr::HBE_COUNT ];
VarTrack g_vtHeadBobAmpOffset[ HeadBobMgr::HBE_COUNT ];
VarTrack g_vtHeadBobFlags[ HeadBobMgr::HBE_COUNT ];

// ****************************************************************************************** //

HeadBobMgr::HeadBobMgr()
{
	Reset();

	m_pCycleNotifyFn = NULL;
	m_pCycleNotifyUserData = NULL;

	m_pCustomCycleNotifyFn = NULL;
	m_pCustomCycleNotifyUserData = NULL;
	m_fCustomCycleNotifyRange = 0.0f;
}

// ****************************************************************************************** //

HeadBobMgr::~HeadBobMgr()
{

}

// ****************************************************************************************** //

bool HeadBobMgr::Initialize()
{
	g_vtHeadBobDebugMode.Init(							g_pLTClient, "HeadBobDebugMode",				NULL, 0.0f );
	g_vtHeadBobSpeedScale.Init(							g_pLTClient, "HeadBobSpeedScale",				NULL, 1.0f );

	g_vtHeadBobWaveMin[ HBE_CAMERAOFFSET_X ].Init(		g_pLTClient, "HeadBobCameraOffsetXWaveMin",		NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_CAMERAOFFSET_X ].Init(		g_pLTClient, "HeadBobCameraOffsetXWaveMax",		NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_CAMERAOFFSET_X ].Init(			g_pLTClient, "HeadBobCameraOffsetXAmp",			NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_CAMERAOFFSET_X ].Init(	g_pLTClient, "HeadBobCameraOffsetXAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_CAMERAOFFSET_X ].Init(		g_pLTClient, "HeadBobCameraOffsetXFlags",		NULL, 0.0f );
	g_vtHeadBobWaveMin[ HBE_CAMERAOFFSET_Y ].Init(		g_pLTClient, "HeadBobCameraOffsetYWaveMin",		NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_CAMERAOFFSET_Y ].Init(		g_pLTClient, "HeadBobCameraOffsetYWaveMax",		NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_CAMERAOFFSET_Y ].Init(			g_pLTClient, "HeadBobCameraOffsetYAmp",			NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_CAMERAOFFSET_Y ].Init(	g_pLTClient, "HeadBobCameraOffsetYAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_CAMERAOFFSET_Y ].Init(		g_pLTClient, "HeadBobCameraOffsetYFlags",		NULL, 0.0f );
	g_vtHeadBobWaveMin[ HBE_CAMERAOFFSET_Z ].Init(		g_pLTClient, "HeadBobCameraOffsetZWaveMin",		NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_CAMERAOFFSET_Z ].Init(		g_pLTClient, "HeadBobCameraOffsetZWaveMax",		NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_CAMERAOFFSET_Z ].Init(			g_pLTClient, "HeadBobCameraOffsetZAmp",			NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_CAMERAOFFSET_Z ].Init(	g_pLTClient, "HeadBobCameraOffsetZAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_CAMERAOFFSET_Z ].Init(		g_pLTClient, "HeadBobCameraOffsetZFlags",		NULL, 0.0f );

	g_vtHeadBobWaveMin[ HBE_CAMERAROTATION_X ].Init(	g_pLTClient, "HeadBobCameraRotationXWaveMin",	NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_CAMERAROTATION_X ].Init(	g_pLTClient, "HeadBobCameraRotationXWaveMax",	NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_CAMERAROTATION_X ].Init(		g_pLTClient, "HeadBobCameraRotationXAmp",		NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_CAMERAROTATION_X ].Init(	g_pLTClient, "HeadBobCameraRotationXAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_CAMERAROTATION_X ].Init(		g_pLTClient, "HeadBobCameraRotationXFlags",		NULL, 2.0f );
	g_vtHeadBobWaveMin[ HBE_CAMERAROTATION_Y ].Init(	g_pLTClient, "HeadBobCameraRotationYWaveMin",	NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_CAMERAROTATION_Y ].Init(	g_pLTClient, "HeadBobCameraRotationYWaveMax",	NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_CAMERAROTATION_Y ].Init(		g_pLTClient, "HeadBobCameraRotationYAmp",		NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_CAMERAROTATION_Y ].Init(	g_pLTClient, "HeadBobCameraRotationYAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_CAMERAROTATION_Y ].Init(		g_pLTClient, "HeadBobCameraRotationYFlags",		NULL, 2.0f );
	g_vtHeadBobWaveMin[ HBE_CAMERAROTATION_Z ].Init(	g_pLTClient, "HeadBobCameraRotationZWaveMin",	NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_CAMERAROTATION_Z ].Init(	g_pLTClient, "HeadBobCameraRotationZWaveMax",	NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_CAMERAROTATION_Z ].Init(		g_pLTClient, "HeadBobCameraRotationZAmp",		NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_CAMERAROTATION_Z ].Init(	g_pLTClient, "HeadBobCameraRotationZAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_CAMERAROTATION_Z ].Init(		g_pLTClient, "HeadBobCameraRotationZFlags",		NULL, 2.0f );

	g_vtHeadBobWaveMin[ HBE_WEAPONOFFSET_X ].Init(		g_pLTClient, "HeadBobWeaponOffsetXWaveMin",		NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_WEAPONOFFSET_X ].Init(		g_pLTClient, "HeadBobWeaponOffsetXWaveMax",		NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_WEAPONOFFSET_X ].Init(			g_pLTClient, "HeadBobWeaponOffsetXAmp",			NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_WEAPONOFFSET_X ].Init(	g_pLTClient, "HeadBobWeaponOffsetXAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_WEAPONOFFSET_X ].Init(		g_pLTClient, "HeadBobWeaponOffsetXFlags",		NULL, 0.0f );
	g_vtHeadBobWaveMin[ HBE_WEAPONOFFSET_Y ].Init(		g_pLTClient, "HeadBobWeaponOffsetYWaveMin",		NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_WEAPONOFFSET_Y ].Init(		g_pLTClient, "HeadBobWeaponOffsetYWaveMax",		NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_WEAPONOFFSET_Y ].Init(			g_pLTClient, "HeadBobWeaponOffsetYAmp",			NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_WEAPONOFFSET_Y ].Init(	g_pLTClient, "HeadBobWeaponOffsetYAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_WEAPONOFFSET_Y ].Init(		g_pLTClient, "HeadBobWeaponOffsetYFlags",		NULL, 0.0f );
	g_vtHeadBobWaveMin[ HBE_WEAPONOFFSET_Z ].Init(		g_pLTClient, "HeadBobWeaponOffsetZWaveMin",		NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_WEAPONOFFSET_Z ].Init(		g_pLTClient, "HeadBobWeaponOffsetZWaveMax",		NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_WEAPONOFFSET_Z ].Init(			g_pLTClient, "HeadBobWeaponOffsetZAmp",			NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_WEAPONOFFSET_Z ].Init(	g_pLTClient, "HeadBobWeaponOffsetZAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_WEAPONOFFSET_Z ].Init(		g_pLTClient, "HeadBobWeaponOffsetZFlags",		NULL, 0.0f );

	g_vtHeadBobWaveMin[ HBE_WEAPONROTATION_X ].Init(	g_pLTClient, "HeadBobWeaponRotationXWaveMin",	NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_WEAPONROTATION_X ].Init(	g_pLTClient, "HeadBobWeaponRotationXWaveMax",	NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_WEAPONROTATION_X ].Init(		g_pLTClient, "HeadBobWeaponRotationXAmp",		NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_WEAPONROTATION_X ].Init(	g_pLTClient, "HeadBobWeaponRotationXAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_WEAPONROTATION_X ].Init(		g_pLTClient, "HeadBobWeaponRotationXFlags",		NULL, 2.0f );
	g_vtHeadBobWaveMin[ HBE_WEAPONROTATION_Y ].Init(	g_pLTClient, "HeadBobWeaponRotationYWaveMin",	NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_WEAPONROTATION_Y ].Init(	g_pLTClient, "HeadBobWeaponRotationYWaveMax",	NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_WEAPONROTATION_Y ].Init(		g_pLTClient, "HeadBobWeaponRotationYAmp",		NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_WEAPONROTATION_Y ].Init(	g_pLTClient, "HeadBobWeaponRotationYAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_WEAPONROTATION_Y ].Init(		g_pLTClient, "HeadBobWeaponRotationYFlags",		NULL, 2.0f );
	g_vtHeadBobWaveMin[ HBE_WEAPONROTATION_Z ].Init(	g_pLTClient, "HeadBobWeaponRotationZWaveMin",	NULL, 0.0f );
	g_vtHeadBobWaveMax[ HBE_WEAPONROTATION_Z ].Init(	g_pLTClient, "HeadBobWeaponRotationZWaveMax",	NULL, 0.0f );
	g_vtHeadBobAmp[ HBE_WEAPONROTATION_Z ].Init(		g_pLTClient, "HeadBobWeaponRotationZAmp",		NULL, 0.0f );
	g_vtHeadBobAmpOffset[ HBE_WEAPONROTATION_Z ].Init(	g_pLTClient, "HeadBobWeaponRotationZAmpOffset",	NULL, 0.0f );
	g_vtHeadBobFlags[ HBE_WEAPONROTATION_Z ].Init(		g_pLTClient, "HeadBobWeaponRotationZFlags",		NULL, 2.0f );

	return true;
}

// ****************************************************************************************** //

void HeadBobMgr::Release()
{

}

// ****************************************************************************************** //

void HeadBobMgr::OnEnterWorld()
{
	Reset();
}

// ****************************************************************************************** //

void HeadBobMgr::Update( float fFrameTime )
{
	// Skip this if the game is paused, or we have no record data to use...
	if( !m_hRecord || ( GetScale() <= 0.0f ) )
	{
		return;
	}

	// See if we're using debug console commands instead of the record data
	bool bDebug = ( g_vtHeadBobDebugMode.GetFloat() != 0.0f );

	// Update the cycle position
	if( m_fSpeed <= 0.0f )
	{
		m_fCyclePosition = 0.0f;
	}
	else
	{
		float fSpeedScale = ( bDebug ? g_vtHeadBobSpeedScale.GetFloat() : m_fSpeedScale );
		if (g_pMoveMgr)
		{
			fSpeedScale *= g_pMoveMgr->GetDamageMovementMultiplier();
		}
		float fPrevPosition = m_fCyclePosition;

		m_fCyclePosition += ( m_fSpeed * fSpeedScale * fFrameTime * m_fCycleDirection );

		// Do our custom callback if we have one
		if( m_pCustomCycleNotifyFn )
		{
			bool bCrossBoundary = false;
			bool bMaxExtent, bApproachFromCenter;

			// Crossing upper boundary from center...
			if( ( fPrevPosition <= m_fCustomCycleNotifyRange ) && ( m_fCyclePosition > m_fCustomCycleNotifyRange ) )
			{
				bCrossBoundary = true;
				bMaxExtent = true;
				bApproachFromCenter = true;
			}
			// Crossing upper boundary from edge...
			else if( ( fPrevPosition >= m_fCustomCycleNotifyRange ) && ( m_fCyclePosition < m_fCustomCycleNotifyRange ) )
			{
				bCrossBoundary = true;
				bMaxExtent = true;
				bApproachFromCenter = false;
			}
			// Crossing lower boundary from center...
			else if( ( fPrevPosition >= -m_fCustomCycleNotifyRange ) && ( m_fCyclePosition < -m_fCustomCycleNotifyRange ) )
			{
				bCrossBoundary = true;
				bMaxExtent = false;
				bApproachFromCenter = true;
			}
			// Crossing lower boundary from edge...
			if( ( fPrevPosition <= -m_fCustomCycleNotifyRange ) && ( m_fCyclePosition > -m_fCustomCycleNotifyRange ) )
			{
				bCrossBoundary = true;
				bMaxExtent = false;
				bApproachFromCenter = false;
			}

			// Execute the callback
			if( bCrossBoundary )
			{
				( *m_pCustomCycleNotifyFn )( bMaxExtent, m_pCustomCycleNotifyUserData, bApproachFromCenter );
			}
		}

		// Handle changing directions of the cycle
		if( ( m_fCycleDirection == 1.0f ) && ( m_fCyclePosition > 1.0f ) )
		{
			m_fCyclePosition = ( 1.0f - ( m_fCyclePosition - 1.0f ) );
			m_fCycleDirection = -1.0f;

			if( m_pCycleNotifyFn )
			{
				( *m_pCycleNotifyFn )( true, m_pCycleNotifyUserData, true );
			}
		}
		else if( ( m_fCycleDirection == -1.0f ) && ( m_fCyclePosition < -1.0f ) )
		{
			m_fCyclePosition = ( -1.0f - ( m_fCyclePosition + 1.0f ) );
			m_fCycleDirection = 1.0f;

			if( m_pCycleNotifyFn )
			{
				( *m_pCycleNotifyFn )( false, m_pCycleNotifyUserData, true );
			}
		}
	}

	// Update the transition time
	if( m_fTransitionTime > 0.0f )
	{
		m_fTransitionTime -= fFrameTime;

		if( m_fTransitionTime < 0.0f )
		{
			m_fTransitionTime = 0.0f;
		}
	}

	// Adjust the headbob by the appropriate scaling factor...
	ClientDB &rClientDatabase = ClientDB::Instance( );
	HRECORD hIdle = rClientDatabase.GetRecord( rClientDatabase.GetHeadBobCategory( ), "Idle" );
	float fHeadBobMod = GetConsoleFloat( ((m_hRecord == hIdle) ? "IdleBreathing" : "HeadBob"), 1.0f );
	
	// Go through each element and update it
	float fDelta, fWave, fInterp;

	for( uint32 i = 0; i < HBE_COUNT; ++i )
	{
		float fWaveMin = ( bDebug ? g_vtHeadBobWaveMin[ i ].GetFloat() : m_aHBDRecord[ i ].m_vWave.x );
		float fWaveMax = ( bDebug ? g_vtHeadBobWaveMax[ i ].GetFloat() : m_aHBDRecord[ i ].m_vWave.y );
		float fAmplitude = ( bDebug ? g_vtHeadBobAmp[ i ].GetFloat() : m_aHBDRecord[ i ].m_vAmplitude.x );
		float fAmplitudeOffset = ( bDebug ? g_vtHeadBobAmpOffset[ i ].GetFloat() : m_aHBDRecord[ i ].m_vAmplitude.y );
		uint32 nFlags = ( bDebug ? ( uint32 )g_vtHeadBobFlags[ i ].GetFloat() : m_aHBDRecord[ i ].m_nFlags );

		// Calculate the current amplitude of this parameter
		if( ( m_fSpeed > 0.0f ) && ( fAmplitude != 0.0f ) )
		{
			fDelta = ( ( fWaveMax - fWaveMin ) * 0.5f );
			fWave = ( ( fWaveMin + fDelta ) + ( fDelta * m_fCyclePosition ) );

			if( nFlags & HeadBobElementData::HBEDF_SINE )
			{
				fWave = LTSin( fWave );
			}

			m_aAmpsActive[ i ] = ( fAmplitudeOffset + ( fWave * fAmplitude ) ) * fHeadBobMod;
		}
		else
		{
			m_aAmpsActive[ i ] = 0.0f;
		}

		// Handle transitioning...
		if( m_fTransitionTime > 0.0f )
		{
			fInterp = ( 1.0f - ( m_fTransitionTime / m_fTotalTransitionTime ) );
			m_aAmpsActive[ i ] = LTLERP( m_aAmpsTransFrom[ i ], m_aAmpsActive[ i ], fInterp );
		}
	}
}

// ****************************************************************************************** //

void HeadBobMgr::SetRecord( HRECORD hRecord, float fTransitionTime )
{
	// If we're already using this record... don't do anything
	if( hRecord == m_hRecord )
	{
		return;
	}

	// Set the record values
	m_hRecord = hRecord;

	// Handle setting up the transition data
	memcpy( m_aAmpsTransFrom, m_aAmpsActive, sizeof( float ) * HBE_COUNT );
	m_fTotalTransitionTime = fTransitionTime;
	m_fTransitionTime = fTransitionTime;

	// Fill in the parameters for this record
	FillHeadBobElementData( m_hRecord, m_aHBDRecord );
}

// ****************************************************************************************** //

void HeadBobMgr::SetRecord( const char* sRecord, float fTransitionTime )
{
	ClientDB& iClientDatabase = ClientDB::Instance();
	HRECORD hRecord = g_pLTDatabase->GetRecord( iClientDatabase.GetHeadBobCategory(), sRecord );

	SetRecord( hRecord, fTransitionTime );
}

// ****************************************************************************************** //

void HeadBobMgr::SetSpeed( float fSpeed )
{
	m_fSpeed = fSpeed;
}

// ****************************************************************************************** //

float HeadBobMgr::GetSpeed() const
{
	return m_fSpeed;
}

// ****************************************************************************************** //

void HeadBobMgr::SetScale( float fScale )
{
	m_fScale = fScale;
}

// ****************************************************************************************** //

float HeadBobMgr::GetScale() const
{
	if (g_pMoveMgr)
	{
		return (m_fScale *  g_pMoveMgr->GetDamageMovementMultiplier() );
	}
	return m_fScale;
}

// ****************************************************************************************** //

void HeadBobMgr::SetCycleNotifyFn( HeadBobCycleNotifyFn pFn, void* pUserData )
{
	m_pCycleNotifyFn = pFn;
	m_pCycleNotifyUserData = pUserData;
}

// ****************************************************************************************** //

void HeadBobMgr::SetCustomCycleNotifyFn( HeadBobCycleNotifyFn pFn, void* pUserData, float fRange )
{
	m_pCustomCycleNotifyFn = pFn;
	m_pCustomCycleNotifyUserData = pUserData;
	m_fCustomCycleNotifyRange = LTCLAMP( fRange, -1.0f, 1.0f );
}

// ****************************************************************************************** //

LTVector HeadBobMgr::GetCameraOffsets() const
{
	LTVector v(	m_aAmpsActive[ HBE_CAMERAOFFSET_X ],
				m_aAmpsActive[ HBE_CAMERAOFFSET_Y ],
				m_aAmpsActive[ HBE_CAMERAOFFSET_Z ] );

	return ( v * GetScale() );
}

// ****************************************************************************************** //

LTVector HeadBobMgr::GetCameraRotations() const
{
	LTVector v(	m_aAmpsActive[ HBE_CAMERAROTATION_X ],
				m_aAmpsActive[ HBE_CAMERAROTATION_Y ],
				m_aAmpsActive[ HBE_CAMERAROTATION_Z ] );

	return ( v * GetScale() );
}

// ****************************************************************************************** //

LTVector HeadBobMgr::GetWeaponOffsets() const
{
	LTVector v(	m_aAmpsActive[ HBE_WEAPONOFFSET_X ],
				m_aAmpsActive[ HBE_WEAPONOFFSET_Y ],
				m_aAmpsActive[ HBE_WEAPONOFFSET_Z ] );

	return ( v * GetScale() );
}

// ****************************************************************************************** //

LTVector HeadBobMgr::GetWeaponRotations() const
{
	LTVector v(	m_aAmpsActive[ HBE_WEAPONROTATION_X ],
				m_aAmpsActive[ HBE_WEAPONROTATION_Y ],
				m_aAmpsActive[ HBE_WEAPONROTATION_Z ] );

	return ( v * GetScale() );
}

// ****************************************************************************************** //

void HeadBobMgr::Reset()
{
	m_hRecord = NULL;

	m_fSpeed = 1.0f;
	m_fScale = 1.0f;
	m_fCyclePosition = 0.0f;
	m_fCycleDirection = 1.0f;
	m_fTotalTransitionTime = 0.0f;
	m_fTransitionTime = 0.0f;
	m_fSpeedScale = 1.0f;

	for( uint32 i = 0; i < HBE_COUNT; ++i )
	{
		m_aHBDRecord[ i ].Init();
		m_aAmpsTransFrom[ i ] = 0.0f;
		m_aAmpsActive[ i ] = 0.0f;
	}
}

// ****************************************************************************************** //

void HeadBobMgr::FillHeadBobElementData( HRECORD hRecord, HeadBobElementData* pData )
{
	ClientDB& iClientDatabase = ClientDB::Instance();
	HATTRIBUTE hBaseAttribute, hValueAttribute;

	// Get the speed scale
	hBaseAttribute = g_pLTDatabase->GetAttribute( hRecord, "SpeedScale" );
	m_fSpeedScale = g_pLTDatabase->GetFloat( hBaseAttribute, 0, 1.0f );

	// Camera offsets
	hBaseAttribute = g_pLTDatabase->GetAttribute( hRecord, "CameraOffsets" );

	for( uint32 i = 0; i < 3; ++i )
	{
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "WaveType" );
		pData[ HBE_CAMERAOFFSET_X + i ].m_nFlags = LTStrIEquals( g_pLTDatabase->GetString( hValueAttribute, 0, "" ), "Sine" ) ? HeadBobElementData::HBEDF_SINE : HeadBobElementData::HBEDF_LINEAR;
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "WaveRange" );
		pData[ HBE_CAMERAOFFSET_X + i ].m_vWave = g_pLTDatabase->GetVector2( hValueAttribute, 0, LTVector2() );
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "Amplitude" );
		pData[ HBE_CAMERAOFFSET_X + i ].m_vAmplitude.x = g_pLTDatabase->GetFloat( hValueAttribute, 0, 0.0f );
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "AmplitudeOffset" );
		pData[ HBE_CAMERAOFFSET_X + i ].m_vAmplitude.y = g_pLTDatabase->GetFloat( hValueAttribute, 0, 0.0f );
	}

	// Camera rotations
	hBaseAttribute = g_pLTDatabase->GetAttribute( hRecord, "CameraRotations" );

	for( uint32 i = 0; i < 3; ++i )
	{
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "WaveType" );
		pData[ HBE_CAMERAROTATION_X + i ].m_nFlags = LTStrIEquals( g_pLTDatabase->GetString( hValueAttribute, 0, "" ), "Sine" ) ? HeadBobElementData::HBEDF_SINE : HeadBobElementData::HBEDF_LINEAR;
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "WaveRange" );
		pData[ HBE_CAMERAROTATION_X + i ].m_vWave = g_pLTDatabase->GetVector2( hValueAttribute, 0, LTVector2() );
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "Amplitude" );
		pData[ HBE_CAMERAROTATION_X + i ].m_vAmplitude.x = g_pLTDatabase->GetFloat( hValueAttribute, 0, 0.0f );
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "AmplitudeOffset" );
		pData[ HBE_CAMERAROTATION_X + i ].m_vAmplitude.y = g_pLTDatabase->GetFloat( hValueAttribute, 0, 0.0f );
	}

	// Weapon offsets
	hBaseAttribute = g_pLTDatabase->GetAttribute( hRecord, "WeaponOffsets" );

	for( uint32 i = 0; i < 3; ++i )
	{
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "WaveType" );
		pData[ HBE_WEAPONOFFSET_X + i ].m_nFlags = LTStrIEquals( g_pLTDatabase->GetString( hValueAttribute, 0, "" ), "Sine" ) ? HeadBobElementData::HBEDF_SINE : HeadBobElementData::HBEDF_LINEAR;
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "WaveRange" );
		pData[ HBE_WEAPONOFFSET_X + i ].m_vWave = g_pLTDatabase->GetVector2( hValueAttribute, 0, LTVector2() );
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "Amplitude" );
		pData[ HBE_WEAPONOFFSET_X + i ].m_vAmplitude.x = g_pLTDatabase->GetFloat( hValueAttribute, 0, 0.0f );
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "AmplitudeOffset" );
		pData[ HBE_WEAPONOFFSET_X + i ].m_vAmplitude.y = g_pLTDatabase->GetFloat( hValueAttribute, 0, 0.0f );
	}

	// Weapon rotations
	hBaseAttribute = g_pLTDatabase->GetAttribute( hRecord, "WeaponRotations" );

	for( uint32 i = 0; i < 3; ++i )
	{
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "WaveType" );
		pData[ HBE_WEAPONROTATION_X + i ].m_nFlags = LTStrIEquals( g_pLTDatabase->GetString( hValueAttribute, 0, "" ), "Sine" ) ? HeadBobElementData::HBEDF_SINE : HeadBobElementData::HBEDF_LINEAR;
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "WaveRange" );
		pData[ HBE_WEAPONROTATION_X + i ].m_vWave = g_pLTDatabase->GetVector2( hValueAttribute, 0, LTVector2() );
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "Amplitude" );
		pData[ HBE_WEAPONROTATION_X + i ].m_vAmplitude.x = g_pLTDatabase->GetFloat( hValueAttribute, 0, 0.0f );
		hValueAttribute = iClientDatabase.GetStructAttribute( hBaseAttribute, i, "AmplitudeOffset" );
		pData[ HBE_WEAPONROTATION_X + i ].m_vAmplitude.y = g_pLTDatabase->GetFloat( hValueAttribute, 0, 0.0f );
	}
}

