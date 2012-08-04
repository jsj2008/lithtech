// ****************************************************************************************** //
//
// MODULE  : FlashLight.cpp
//
// PURPOSE : FlashLight class - Implementation
//
// CREATED : 09/01/04
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ****************************************************************************************** //

#include "stdafx.h"

#ifndef __FLASHLIGHT_H__
#include "FlashLight.h"
#endif//__FLASHLIGHT_H__

#ifndef __CLIENTDB_H__
#include "ClientDB.h"
#endif//__CLIENTDB_H__

#ifndef __LTBEZIERCURVE_H__
#include "ltbeziercurve.h"
#endif//__LTBEZIERCURVE_H__

#include "VolumetricLightFX.h"

// ****************************************************************************************** //
// Flicker patterns

const int g_nFlickerPatternSteps = 16;
const int g_nFlickerPatterns = 5;

// ------------------------------------------------------------------------------------------ //

const float g_aFlickerPatterns[ g_nFlickerPatterns ][ g_nFlickerPatternSteps ] =
{
	{ 1.0f, 0.1f, 0.9f, 0.1f, 0.8f, 0.2f, 0.7f, 0.3f, 0.6f, 0.4f, 0.5f, 0.4f, 0.7f, 1.0f, 0.7f, 1.0f },
	{ 1.0f, 0.2f, 0.3f, 0.6f, 0.0f, 0.9f, 0.1f, 0.8f, 0.2f, 0.4f, 0.5f, 0.5f, 0.7f, 0.7f, 0.4f, 1.0f },
	{ 1.0f, 0.3f, 0.3f, 0.6f, 0.4f, 0.9f, 0.1f, 0.8f, 0.2f, 0.7f, 1.0f, 0.6f, 0.5f, 0.0f, 0.7f, 1.0f },
	{ 1.0f, 0.0f, 0.9f, 0.2f, 0.7f, 1.0f, 0.5f, 0.5f, 0.6f, 0.4f, 0.5f, 0.0f, 0.7f, 0.3f, 0.5f, 1.0f },
	{ 1.0f, 0.2f, 0.3f, 0.6f, 0.4f, 0.3f, 0.6f, 0.4f, 0.9f, 0.1f, 1.0f, 0.5f, 0.2f, 0.0f, 0.7f, 1.0f },
};

// ------------------------------------------------------------------------------------------ //

static EEngineLOD StringToLOD(const char* pszString, EEngineLOD eDefault = eEngineLOD_Low)
{
	if(LTStrIEquals("Low", pszString))
		return eEngineLOD_Low;
	if(LTStrIEquals("Medium", pszString))
		return eEngineLOD_Medium;
	if(LTStrIEquals("High", pszString))
		return eEngineLOD_High;
	if(LTStrIEquals("Never", pszString))
		return eEngineLOD_Never;

	//no matches, return the default
	return eDefault;
}

// ****************************************************************************************** //

Flashlight::Flashlight()
{
	m_hFollowObject		= NULL;
	m_hRecord			= NULL;

	m_bOn				= false;
	m_bWasOn			= false;
	m_bEnabled			= true;

	// Database variables
	m_nLights = 0;

	m_sSocket = NULL;
	m_bSendOnOffStateToServer = false;

	m_bTemporaryFlicker = false;
	m_bAlwaysFlicker = false;
	m_vFlickerInterval.Init();
	m_vFlickerDuration.Init();

	m_bLag = false;
	m_fLagCorrectionSpeed = 0.0f;
	m_bLagInverseDirection = false;

	m_bWaver = false;
	m_fWaverPerimeterSize = 0.0f;
	m_fWaverTangentLength = 0.0f;
	m_fWaverSpeed = 0.0f;
	m_fWaverSpeedScale = 1.0f;

	m_vShakeWaveRange.Init();

	// Runtime updating variables
	m_nFlickerPattern = 0;
	m_fFlickerIntervalRemaining = 0.0f;
	m_fFlickerDurationTotal = 0.0f;
	m_fFlickerDurationRemaining = -1.0f;
	m_bFlickerOut = false;

	m_rLagDelta.Init();
	m_rLagTracker.Init();

	m_vWaverPathPt1.Init();
	m_vWaverPathPt2.Init();
	m_vWaverPathPt3.Init();
	m_vWaverPathPt4.Init();
	m_fWaverDurationTotal = 0.0f;
	m_fWaverDurationRemaining = 0.0f;

	m_fShakeStrength = 0.0f;
	m_vShakeWaveLengths.Init();
	m_fShakeDurationTotal = 0.0f;
	m_fShakeDurationRemaining = 0.0f;
}

// ****************************************************************************************** //

Flashlight::~Flashlight()
{
	for( uint32 i = 0; i < m_nLights; ++i )
	{
		if( m_aLights[ i ].m_hLight )
		{
			g_pLTClient->RemoveObject( m_aLights[ i ].m_hLight );
		}

		m_aLights[ i ].Init();
	}

	m_nLights = 0;
}

// ****************************************************************************************** //

bool Flashlight::Initialize( HOBJECT hFollowObject, HRECORD hRecord )
{
	// Validate parameters
	if( !hFollowObject || !hRecord )
	{
		LTERROR( "Flashlight needs an object to follow, and a valid database record!" );
		return false;
	}

	// Save the object that we're wanting to follow, and the database record
	bool bNewRecord = ( ( m_hRecord != hRecord ) ? true : false );
	m_hFollowObject = hFollowObject;

	// Read out the properties of the flashlight from the database
	if( bNewRecord )
	{
		// Save the current record...
		m_hRecord = hRecord;

		HATTRIBUTE hLightData = g_pLTDatabase->GetAttribute( m_hRecord, "LightData" );
		m_nLights = g_pLTDatabase->GetNumValues( hLightData );

		// Make sure we don't have more than our max!
		if( m_nLights > MAX_FLASHLIGHT_LIGHTS )
		{
			LTERROR( "Flashlight::Initialize() -- More lights in the database than are supported by the flashlight!" );
			m_nLights = MAX_FLASHLIGHT_LIGHTS;
		}

		// Read in all the light data
		for( uint32 i = 0; i < m_nLights; ++i )
		{
			// Get all the new values out of the database...
			m_aLights[ i ].m_bCreateLight = g_pLTDatabase->GetBool( CGameDatabaseReader::GetStructAttribute( hLightData, i, "CreateLight" ), 0, true );
			m_aLights[ i ].m_sLightTexture = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightTexture" ), 0, "" );
			m_aLights[ i ].m_sLightType = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightType" ), 0, "" );
			m_aLights[ i ].m_vLightRadiusRange = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightRadiusRange" ), 0, LTVector2() );
			m_aLights[ i ].m_nLightColor = g_pLTDatabase->GetInt32( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightColor" ), 0, 0xFFFFFFFF );
			m_aLights[ i ].m_vLightPositionOffset = g_pLTDatabase->GetVector3( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightPositionOffset" ), 0, LTVector() );
			m_aLights[ i ].m_vLightRotationOffset = g_pLTDatabase->GetVector3( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightRotationOffset" ), 0, LTVector() );
			m_aLights[ i ].m_vLightIntensity = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightIntensity" ), 0, LTVector2() );
			m_aLights[ i ].m_vLightFOV = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightFOV" ), 0, LTVector2() );
			m_aLights[ i ].m_sLightShadowLOD = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hLightData, i, "LightShadowLOD" ), 0, "" );

			// volumetric data
			m_aLights[ i ].m_eVolumetricLOD = StringToLOD(g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricLOD" ), 0, "" ), eEngineLOD_Never);
			if (m_aLights[ i ].m_eVolumetricLOD != eEngineLOD_Never)
			{
				m_aLights[ i ].m_fVolumetricNoiseIntensity	= g_pLTDatabase->GetFloat	( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricNoiseIntensity"	 ), 0, 0.0f );
				m_aLights[ i ].m_fVolumetricNoiseScale		= g_pLTDatabase->GetFloat	( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricNoiseScale"		 ), 0, 0.0f );
				m_aLights[ i ].m_nVolumetricColor			= g_pLTDatabase->GetInt32	( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricColor"				 ), 0, 0xFFFFFFFF );
				m_aLights[ i ].m_sVolumetricTexture			= g_pLTDatabase->GetString	( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricTexture"			 ), 0, "" );
				m_aLights[ i ].m_fVolumetricAttenuation		= g_pLTDatabase->GetFloat	( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricAttenuation"		 ), 0, 0.0f );
				m_aLights[ i ].m_bVolumetricAdditive		= g_pLTDatabase->GetBool	( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricAdditive"			 ), 0, false );
				m_aLights[ i ].m_fVolumetricDepth			= g_pLTDatabase->GetFloat	( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricDepth"				 ), 0, 0.0f );
				m_aLights[ i ].m_bVolumetricShadow			= g_pLTDatabase->GetBool	( CGameDatabaseReader::GetStructAttribute( hLightData, i, "VolumetricShadow"			 ), 0, false );
			}
		}

		m_sSocket = g_pLTDatabase->GetString( g_pLTDatabase->GetAttribute( m_hRecord, "Socket" ), 0, "" );
		m_bSendOnOffStateToServer = g_pLTDatabase->GetBool( g_pLTDatabase->GetAttribute( m_hRecord, "SendOnOffStateToServer" ), 0, false );

		m_bAlwaysFlicker = g_pLTDatabase->GetBool( g_pLTDatabase->GetAttribute( m_hRecord, "Flicker" ), 0, false );
		m_vFlickerInterval = g_pLTDatabase->GetVector2( g_pLTDatabase->GetAttribute( m_hRecord, "FlickerInterval" ), 0, LTVector2() );
		m_vFlickerDuration = g_pLTDatabase->GetVector2( g_pLTDatabase->GetAttribute( m_hRecord, "FlickerDuration" ), 0, LTVector2() );

		m_bLag = g_pLTDatabase->GetBool( g_pLTDatabase->GetAttribute( m_hRecord, "Lag" ), 0, false );
		m_fLagCorrectionSpeed = g_pLTDatabase->GetFloat( g_pLTDatabase->GetAttribute( m_hRecord, "LagCorrectionSpeed" ), 0, 0.0f );
		m_bLagInverseDirection = g_pLTDatabase->GetBool( g_pLTDatabase->GetAttribute( m_hRecord, "LagInverseDirection" ), 0, false );

		m_bWaver = g_pLTDatabase->GetBool( g_pLTDatabase->GetAttribute( m_hRecord, "Waver" ), 0, false );
		m_fWaverPerimeterSize = g_pLTDatabase->GetFloat( g_pLTDatabase->GetAttribute( m_hRecord, "WaverPerimeterSize" ), 0, 0.0f );
		m_fWaverTangentLength = g_pLTDatabase->GetFloat( g_pLTDatabase->GetAttribute( m_hRecord, "WaverTangentLength" ), 0, 0.0f );
		m_fWaverSpeed = g_pLTDatabase->GetFloat( g_pLTDatabase->GetAttribute( m_hRecord, "WaverSpeed" ), 0, 0.0f );

		m_vShakeWaveRange = g_pLTDatabase->GetVector2( g_pLTDatabase->GetAttribute( m_hRecord, "ShakeWaveRange" ), 0, LTVector2() );

		HRECORD hFXSequenceRecord = g_pLTDatabase->GetRecordLink( g_pLTDatabase->GetAttribute( m_hRecord, "ClientFXSequence" ), 0, NULL );
		m_fxFlashlightSequence.SetClientFXSequenceRecord( hFXSequenceRecord );

		m_fxFlashlightSequence.SetParentObject( m_hFollowObject );

		m_fMaxCharge = g_pLTDatabase->GetFloat( g_pLTDatabase->GetAttribute( m_hRecord, "MaxCharge" ), 0, 0.0f );
		m_fRechargeRate = g_pLTDatabase->GetFloat( g_pLTDatabase->GetAttribute( m_hRecord, "RechargeRate" ), 0, 0.0f );

		// Make sure the light properties are set properly
		Helper_SetLightProperties();
	}

	return true;
}

// ****************************************************************************************** //

bool Flashlight::Initialize( HOBJECT hFollowObject, const char* sRecord )
{
	ClientDB& iClientDatabase = ClientDB::Instance();
	HRECORD hRecord = g_pLTDatabase->GetRecord( iClientDatabase.GetFlashlightCategory(), sRecord );

	return Initialize( hFollowObject, hRecord );
}

// ****************************************************************************************** //

void Flashlight::Update( float fFrameTime )
{
	// Get the initial transform of the light
	LTRigidTransform iTrans;
	Helper_GetTransform( iTrans );


	// ------------------------------------------------------------------------------------------ //
	// Handle lagging...

	// [ Sing Along ]	We can lag if we wanna... we can leave your light behind,
	//					Cause your light don't lag, and if it don't lag,
	//					Then it's no light of mine.

	if( m_bLag )
	{
		LTRotation rInit;
		m_rLagDelta = ( iTrans.m_rRot.Conjugate() * m_rLagTracker );

		float fSlerpAmount = LTMIN( ( fFrameTime * m_fLagCorrectionSpeed ), 1.0f );
		m_rLagDelta.Slerp( rInit, m_rLagDelta, ( 1.0f - fSlerpAmount ) );

		m_rLagTracker = ( iTrans.m_rRot * m_rLagDelta );
		iTrans.m_rRot = m_bLagInverseDirection ? ( iTrans.m_rRot * m_rLagDelta.Conjugate() ) : m_rLagTracker;
	}


	// ------------------------------------------------------------------------------------------ //
	// Handle wavering...

	LTRotation rWaver;
	Update_Waver( rWaver, fFrameTime );

	iTrans.m_rRot = ( iTrans.m_rRot * rWaver );


	// ------------------------------------------------------------------------------------------ //
	// Handle flickering...

	Update_Flicker( fFrameTime );


	// ------------------------------------------------------------------------------------------ //
	// Handle shaking...

	LTRotation rShake;
	Update_Shake( rShake, fFrameTime );

	iTrans.m_rRot = ( iTrans.m_rRot * rShake );


	// ------------------------------------------------------------------------------------------ //
	// Set the final values for the light

	for( uint32 i = 0; i < m_nLights; ++i )
	{
		if( m_aLights[ i ].m_hLight )
		{
			// Set the final transform!
			LTRigidTransform iLightTrans = iTrans;
			Helper_OffsetTranform( iLightTrans, i );

			g_pLTClient->SetObjectTransform( m_aLights[ i ].m_hLight, iLightTrans );
		}
	}

	// Update the ClientFX Sequence...
	m_fxFlashlightSequence.Update( );
}

// ****************************************************************************************** //

void Flashlight::Enable( bool bEnable, float fFlickerDuration )
{
	if( bEnable )
	{
		m_bEnabled = true;
		m_bFlickerOut = false;

		if( m_bWasOn )
		{
			TurnOn();
			PlayFlicker( fFlickerDuration, true );
		}
	}
	else
	{
		m_bWasOn = m_bOn;

		if( fFlickerDuration > 0.0f )
		{			
			m_bFlickerOut = true;
			PlayFlicker( fFlickerDuration, true );
		}
		else
		{
			m_bFlickerOut = false;
			m_bEnabled = false;
			TurnOff();
		}
	}
}

// ****************************************************************************************** //

bool Flashlight::TurnOn()
{
	// Make sure the flashlight is enabled and not already turned on (cause I'm turned on!!)
	if( !m_bEnabled || m_bOn )
	{
		return false;
	}

	// Verify that the light has been created...
	Helper_CreateLight();

	// Make sure the lights are visible
	for( uint32 i = 0; i < MAX_FLASHLIGHT_LIGHTS; ++i )
	{
		if( m_aLights[ i ].m_hLight )
		{
			g_pCommonLT->SetObjectFlags( m_aLights[ i ].m_hLight, OFT_Flags, ( ( i < m_nLights ) ? FLAG_VISIBLE : 0 ), FLAG_VISIBLE );
		}
	}

	m_bOn = true;

	// Send the state to the server if we want to
	//		but never in MP (for performance reasons)
	if( m_bSendOnOffStateToServer && !IsMultiplayerGameClient())
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
		cMsg.Writeuint8( CP_FLASHLIGHT );
		cMsg.Writeuint8( FL_ON );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}

	// Go ahead and do an update so the light doesn't do some weird interpolation
	Update( 1000.0f );

	// Play the 'on' sound
	if( m_hRecord )
	{
		HATTRIBUTE hAttribute;
		hAttribute = g_pLTDatabase->GetAttribute( m_hRecord, "SoundOn" );

		HRECORD hSound = g_pLTDatabase->GetRecordLink( hAttribute, 0, NULL );

		if( hSound )
		{
			g_pClientSoundMgr->PlayDBSoundLocal( hSound );
		}
	}

	// Begin the ClientFXSequence...
	m_fxFlashlightSequence.SetState( CClientFXSequence::eState_Begin );
	
	return true;
}

// ****************************************************************************************** //

bool Flashlight::TurnOff()
{
	// Make sure the flashlight isn't already turned off (it sucks to be turned off!!)
	if( !m_bOn )
	{
		return false;
	}

	for( uint32 i = 0; i < m_nLights; ++i )
	{
		if( m_aLights[ i ].m_hLight )
		{
			g_pCommonLT->SetObjectFlags( m_aLights[ i ].m_hLight, OFT_Flags, 0, FLAG_VISIBLE );
		}
	}

	m_bOn = false;

	// Send the state to the server if we want to
	if( m_bSendOnOffStateToServer && !IsMultiplayerGameClient() )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
		cMsg.Writeuint8( CP_FLASHLIGHT );
		cMsg.Writeuint8( FL_OFF );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}

	// Play the 'off' sound
	if( m_hRecord )
	{
		HATTRIBUTE hAttribute;
		hAttribute = g_pLTDatabase->GetAttribute( m_hRecord, "SoundOff" );

		HRECORD hSound = g_pLTDatabase->GetRecordLink( hAttribute, 0, NULL );

		if( hSound )
		{
			g_pClientSoundMgr->PlayDBSoundLocal( hSound );
		}
	}

	// Finish the ClientFXSequence...
	m_fxFlashlightSequence.SetState( CClientFXSequence::eState_End );
	
	return true;
}

// ****************************************************************************************** //

void Flashlight::PlayFlicker( float fDuration, bool bTemporary )
{
	if (bTemporary)
	{
		m_bTemporaryFlicker = true;
	}

	// If the flashlight is about to be forced off, use the minimum duration out of the two
	if (m_bFlickerOut && m_fFlickerDurationRemaining > 0.0f)
	{
		m_fFlickerDurationTotal = LTMIN( fDuration, m_fFlickerDurationRemaining );
	}
	else
	{
		m_fFlickerDurationTotal = fDuration;
	}
	m_fFlickerDurationRemaining = m_fFlickerDurationTotal;
	m_nFlickerPattern = GetRandom( 0, ( g_nFlickerPatterns - 1 ) );

	// Play the 'flicker' sound
	if( m_hRecord )
	{
		HATTRIBUTE hAttribute;
		hAttribute = g_pLTDatabase->GetAttribute( m_hRecord, "SoundFlicker" );

		HRECORD hSound = g_pLTDatabase->GetRecordLink( hAttribute, 0, NULL );

		if( hSound )
		{
			g_pClientSoundMgr->PlayDBSoundLocal( hSound );
		}
	}
}

// ****************************************************************************************** //

void Flashlight::PlayShake( float fDuration, float fStrength, float fSpeed )
{
	m_fShakeStrength = fStrength;
	m_vShakeWaveLengths.Init( ( GetRandom( m_vShakeWaveRange.x, m_vShakeWaveRange.y ) * fSpeed ), ( GetRandom( m_vShakeWaveRange.x, m_vShakeWaveRange.y ) * fSpeed ) );
	m_fShakeDurationTotal = fDuration;
	m_fShakeDurationRemaining = m_fShakeDurationTotal;
}

// ****************************************************************************************** //

void Flashlight::PlaySpecialSound( uint32 nIndex )
{
	if( m_hRecord )
	{
		HATTRIBUTE hAttribute;
		hAttribute = g_pLTDatabase->GetAttribute( m_hRecord, "SoundSpecial" );

		HRECORD hSound = g_pLTDatabase->GetRecordLink( hAttribute, nIndex, NULL );

		if( hSound )
		{
			g_pClientSoundMgr->PlayDBSoundLocal( hSound );
		}
	}
}

// ****************************************************************************************** //

void Flashlight::SetWaverSpeedScale( float fScale )
{
	m_fWaverSpeedScale = fScale;
}

// ****************************************************************************************** //

bool Flashlight::IsRecordType( HRECORD hRecord )
{
	return ( m_hRecord == hRecord );
}

// ****************************************************************************************** //

bool Flashlight::IsRecordType( const char* sRecord )
{
	ClientDB& iClientDatabase = ClientDB::Instance();
	HRECORD hRecord = g_pLTDatabase->GetRecord( iClientDatabase.GetFlashlightCategory(), sRecord );

	return IsRecordType( hRecord );
}

// ****************************************************************************************** //

void Flashlight::Save( ILTMessage_Write* pMsg, SaveDataState eSaveDataState )
{
	// SID: Update with the new data!

	pMsg->Writebool( m_bEnabled && !m_bFlickerOut );
	pMsg->Writebool( m_bOn );
	pMsg->Writebool( m_bWasOn );
}

// ****************************************************************************************** //

void Flashlight::Load( ILTMessage_Read* pMsg, SaveDataState eLoadDataState )
{
	// SID: Update with the new data!

	Enable( pMsg->Readbool(), 0.0f );

	if( pMsg->Readbool() )
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}

	m_bWasOn = pMsg->Readbool();
}

// ****************************************************************************************** //

void Flashlight::Helper_CreateLight()
{
	// Get some information about the follow object
	LTRigidTransform iTrans;
	Helper_GetTransform( iTrans );

	for( uint32 i = 0; i < MAX_FLASHLIGHT_LIGHTS; ++i )
	{
		// If a light has already been created or the light is not supposed to be created...  then just move on
		if( m_aLights[ i ].m_hLight || !m_aLights[ i ].m_bCreateLight )
		{
			continue;
		}

		LTRigidTransform iLightTrans = iTrans;
		Helper_OffsetTranform( iLightTrans, i );

		// Fill in the object creation structure and do it!
		ObjectCreateStruct iOCS;

		iOCS.m_ObjectType = OT_LIGHT;
		iOCS.m_Flags = 0;
		iOCS.m_Pos = iLightTrans.m_vPos;
		iOCS.m_Rotation = iLightTrans.m_rRot;

		m_aLights[ i ].m_hLight = g_pLTClient->CreateObject( &iOCS );
	}

	// Go ahead and setup the light properties
	Helper_SetLightProperties();
}

// ****************************************************************************************** //

void Flashlight::Helper_SetLightProperties()
{
	for( uint32 i = 0; i < m_nLights; ++i )
	{
		// Make sure the light is valid
		if( !m_aLights[ i ].m_hLight || !m_aLights[ i ].m_sLightType )
		{
			continue;
		}

		// Set the type of light
		if( LTStrIEquals( m_aLights[ i ].m_sLightType, "Point" ) )
		{
			g_pLTClient->SetLightType( m_aLights[ i ].m_hLight, eEngineLight_Point );
		}
		else if( LTStrIEquals( m_aLights[ i ].m_sLightType, "PointFill" ) )
		{
			g_pLTClient->SetLightType( m_aLights[ i ].m_hLight, eEngineLight_PointFill );
		}
		else if( LTStrIEquals( m_aLights[ i ].m_sLightType, "SpotProjector" ) )
		{
			g_pLTClient->SetLightType( m_aLights[ i ].m_hLight, eEngineLight_SpotProjector );
		}
		else if( LTStrIEquals( m_aLights[ i ].m_sLightType, "BlackLight" ) )
		{
			g_pLTClient->SetLightType( m_aLights[ i ].m_hLight, eEngineLight_BlackLight );
		}

		// Set the shadow LOD of the light
		EEngineLOD eShadowLOD = StringToLOD(m_aLights[ i ].m_sLightShadowLOD);
		g_pLTClient->SetLightDetailSettings( m_aLights[ i ].m_hLight, eEngineLOD_Low, eShadowLOD, eShadowLOD );

		// Set everything else...
		g_pLTClient->SetLightTexture( m_aLights[ i ].m_hLight, m_aLights[ i ].m_sLightTexture );
		g_pLTClient->SetObjectColor( m_aLights[ i ].m_hLight, ( ( ( m_aLights[ i ].m_nLightColor >> 16 ) & 0xFF ) / 255.0f ), ( ( ( m_aLights[ i ].m_nLightColor >> 8 ) & 0xFF ) / 255.0f ), ( ( ( m_aLights[ i ].m_nLightColor >> 0 ) & 0xFF ) / 255.0f ), 1.0f );
		g_pLTClient->SetLightRadius( m_aLights[ i ].m_hLight, m_aLights[ i ].m_vLightRadiusRange.y );
		g_pLTClient->SetLightSpotInfo( m_aLights[ i ].m_hLight, MATH_DEGREES_TO_RADIANS( m_aLights[ i ].m_vLightFOV.x ), MATH_DEGREES_TO_RADIANS( m_aLights[ i ].m_vLightFOV.y ), m_aLights[ i ].m_vLightRadiusRange.x );
		g_pLTClient->SetLightIntensityScale( m_aLights[ i ].m_hLight, m_aLights[ i ].m_vLightIntensity.y );

		// Handle volumetric properties...
		if (m_aLights[ i ].m_pVolumetrictLight)
		{
			// Let's just make a new one to be safe.
			CSFXMgr::DeleteSFX(m_aLights[ i ].m_pVolumetrictLight);
		}
		if (m_aLights[ i ].m_eVolumetricLOD != eEngineLOD_Never)
		{
			VOLUMETRICLIGHTCREATESTRUCT vlcs;
			vlcs.m_eLOD				= m_aLights[ i ].m_eVolumetricLOD;
			vlcs.m_fNoiseIntensity	= m_aLights[ i ].m_fVolumetricNoiseIntensity;
			vlcs.m_fNoiseScale		= m_aLights[ i ].m_fVolumetricNoiseScale;
			vlcs.m_vColor			= LTVector( ( ( ( m_aLights[ i ].m_nVolumetricColor >> 16 ) & 0xFF ) / 255.0f ), ( ( ( m_aLights[ i ].m_nVolumetricColor >> 8 ) & 0xFF ) / 255.0f ), ( ( ( m_aLights[ i ].m_nVolumetricColor >> 0 ) & 0xFF ) / 255.0f ) );
			vlcs.m_sTexture			= (m_aLights[ i ].m_sVolumetricTexture && m_aLights[ i ].m_sVolumetricTexture[0]) ? m_aLights[ i ].m_sVolumetricTexture : m_aLights[ i ].m_sLightTexture;
			vlcs.m_fAttenuation		= m_aLights[ i ].m_fVolumetricAttenuation;
			vlcs.m_bAdditive		= m_aLights[ i ].m_bVolumetricAdditive;
			vlcs.m_fDepth			= m_aLights[ i ].m_fVolumetricDepth;
			vlcs.m_bShadow			= m_aLights[ i ].m_bVolumetricShadow;
			vlcs.hServerObj			= m_aLights[ i ].m_hLight;
			vlcs.m_fFarZ			= m_aLights[ i ].m_vLightRadiusRange.y;

			m_aLights[ i ].m_pVolumetrictLight = static_cast< CVolumetricLightFX* >(g_pGameClientShell->GetSFXMgr()->CreateSFX((uint8)SFX_VOLUMETRICLIGHT_ID, &vlcs));
		}
	}

	// Make sure all appropriate visibility flags are set!
	for( uint32 i = 0; i < MAX_FLASHLIGHT_LIGHTS; ++i )
	{
		if( m_aLights[ i ].m_hLight )
		{
			g_pCommonLT->SetObjectFlags( m_aLights[ i ].m_hLight, OFT_Flags, ( ( m_bOn && ( i < m_nLights ) ) ? FLAG_VISIBLE : 0 ), FLAG_VISIBLE );
		}
	}
}

// ****************************************************************************************** //

void Flashlight::Helper_GetTransform( LTRigidTransform& iTrans )
{
	// Make sure we have an object to follow... otherwise we won't have a valid transform
	if( !m_hFollowObject )
	{
		iTrans.Init();
		return;
	}

	// If we have a valid socket name, use that as the initial transform
	bool bSocketUsed = false;

	if( m_sSocket && m_sSocket[0] )
	{
		HMODELSOCKET hSocket;

		if( LT_OK == g_pModelLT->GetSocket( m_hFollowObject, m_sSocket, hSocket ) )
		{
			LTTransform iTemp;

			if( LT_OK == g_pModelLT->GetSocketTransform( m_hFollowObject, hSocket, iTemp, true ) )
			{
				iTrans.m_vPos = iTemp.m_vPos;
				iTrans.m_rRot = iTemp.m_rRot;
				bSocketUsed = true;
			}
		}
	}

	// If we didn't use a socket for the transform... get at the objects transform
	if( !bSocketUsed )
	{
		g_pLTClient->GetObjectTransform( m_hFollowObject, &iTrans );
	}
}

// ****************************************************************************************** //

void Flashlight::Helper_OffsetTranform( LTRigidTransform& iTrans, uint32 nLight )
{
	// Calculate a final transform for this light
	LTRotation rLightOffset( m_aLights[ nLight ].m_vLightRotationOffset.x, m_aLights[ nLight ].m_vLightRotationOffset.y, m_aLights[ nLight ].m_vLightRotationOffset.z );

	iTrans.m_rRot = ( rLightOffset * iTrans.m_rRot );

	iTrans.m_vPos += ( iTrans.m_rRot.Right() * m_aLights[ nLight ].m_vLightPositionOffset.x );
	iTrans.m_vPos += ( iTrans.m_rRot.Up() * m_aLights[ nLight ].m_vLightPositionOffset.y );
	iTrans.m_vPos += ( iTrans.m_rRot.Forward() * m_aLights[ nLight ].m_vLightPositionOffset.z );
}

// ****************************************************************************************** //

void Flashlight::Update_Waver( LTRotation& rOffset, float fFrameTime )
{
	// See if we want to waver at all
	if( !m_bWaver )
	{
		rOffset.Init();
		return;
	}

	// Update our waver duration
	m_fWaverDurationRemaining -= ( fFrameTime * m_fWaverSpeedScale );

	// If our waver path is over, create a new one...
	if( m_fWaverDurationRemaining <= 0.0f )
	{
		// Create a new random path...
		float fTangentAngle = GetRandom( 0.0f, MATH_CIRCLE );

		m_vWaverPathPt1 = m_vWaverPathPt4;
		m_vWaverPathPt2 = -m_vWaverPathPt3;
		m_vWaverPathPt3.Init( ( LTCos( fTangentAngle ) * m_fWaverTangentLength ), ( LTSin( fTangentAngle ) * m_fWaverTangentLength ), 0.0f );
		m_vWaverPathPt4.Init( GetRandom( -m_fWaverPerimeterSize, m_fWaverPerimeterSize ), GetRandom( -m_fWaverPerimeterSize, m_fWaverPerimeterSize ), 0.0f );

		// Get the length of this path
		float fPathLength = Bezier_SegmentLength( m_vWaverPathPt1, m_vWaverPathPt2, m_vWaverPathPt3, m_vWaverPathPt4 );

		// Calculate the time it'll take to traverse this path
		m_fWaverDurationTotal = ( fPathLength / m_fWaverSpeed );
		m_fWaverDurationRemaining = m_fWaverDurationTotal;
	}

	// Calculate our rotation offset...
	LTVector vOutput;
	float fPathT = ( ( m_fWaverDurationTotal - m_fWaverDurationRemaining ) / m_fWaverDurationTotal );
	Bezier_Evaluate( vOutput, m_vWaverPathPt1, m_vWaverPathPt2, m_vWaverPathPt3, m_vWaverPathPt4, fPathT );

	rOffset = LTRotation( vOutput.x, vOutput.y, 0.0f );
}

// ****************************************************************************************** //

void Flashlight::Update_Flicker( float fFrameTime )
{
	// See if we want to flicker at all
	if( !m_bAlwaysFlicker && !m_bTemporaryFlicker)
	{
		for( uint32 i = 0; i < m_nLights; ++i )
		{
			if ( m_aLights[ i ].m_hLight )
				g_pLTClient->SetLightIntensityScale( m_aLights[ i ].m_hLight, m_aLights[ i ].m_vLightIntensity.y );
		}

		return;
	}

	// If we're not flickering right now, update the interval
	if( m_fFlickerDurationRemaining <= 0.0f )
	{
		m_fFlickerIntervalRemaining -= fFrameTime;

		// If we're waiting to flicker, remain at the max intensity
		if( m_fFlickerIntervalRemaining > 0.0f )
		{
			for( uint32 i = 0; i < m_nLights; ++i )
			{
				if ( m_aLights[ i ].m_hLight )
					g_pLTClient->SetLightIntensityScale( m_aLights[ i ].m_hLight, m_aLights[ i ].m_vLightIntensity.y );
			}

			return;
		}

		// Otherwise, start a flicker pattern!
		PlayFlicker( GetRandom( m_vFlickerDuration.x, m_vFlickerDuration.y ), false );
	}

	// Update our flicker duration
	m_fFlickerDurationRemaining -= fFrameTime;

	if( m_fFlickerDurationRemaining <= 0.0f )
	{
		// Set the intensity back to the maximum...
		for( uint32 i = 0; i < m_nLights; ++i )
		{
			if ( m_aLights[ i ].m_hLight )
				g_pLTClient->SetLightIntensityScale( m_aLights[ i ].m_hLight, m_aLights[ i ].m_vLightIntensity.y );
		}

		// Setup a new interval time
		m_fFlickerIntervalRemaining = GetRandom( m_vFlickerInterval.x, m_vFlickerInterval.y );

		// If we're flickering out... then turn everything off
		if( m_bFlickerOut )
		{
			m_bFlickerOut = false;
			m_bEnabled = false;
			TurnOff();
		}
		//if we were doing a temporary flicker, we're done with it...
		m_bTemporaryFlicker = false;

		return;
	}

	// If we made it down here, then we've got a flicker pattern that we're following... so update the intensity based on it
	float fFlickerPatternScale = ( ( m_fFlickerDurationTotal - m_fFlickerDurationRemaining ) / m_fFlickerDurationTotal );
	uint32 nFlickerPatternPos = ( uint32 )( fFlickerPatternScale * g_nFlickerPatternSteps );
	float fFlickerIntensityScale = g_aFlickerPatterns[ m_nFlickerPattern ][ LTMIN( nFlickerPatternPos, ( g_nFlickerPatternSteps - 1 ) ) ];

	//DebugCPrint(0,"Flashlight Flicker at: %0.2f: %d [%0.2f]",RealTimeTimer::Instance().GetTimerAccumulatedS(),nFlickerPatternPos,fFlickerIntensityScale);

	for( uint32 i = 0; i < m_nLights; ++i )
	{
		if ( m_aLights[ i ].m_hLight )
			g_pLTClient->SetLightIntensityScale( m_aLights[ i ].m_hLight, ( m_aLights[ i ].m_vLightIntensity.x + ( ( m_aLights[ i ].m_vLightIntensity.y - m_aLights[ i ].m_vLightIntensity.x ) * fFlickerIntensityScale ) ) );
	}
}

// ****************************************************************************************** //

void Flashlight::Update_Shake( LTRotation& rOffset, float fFrameTime )
{
	// If we're not shaking, just initialize the rotation
	if( m_fShakeDurationRemaining <= 0.0f )
	{
		rOffset.Init();
		return;
	}

	// Update our shake duration
	m_fShakeDurationRemaining -= fFrameTime;

	// Create the offset rotation
	float fScale = ( m_fShakeDurationRemaining / m_fShakeDurationTotal );
	float fStrength = ( fScale * m_fShakeStrength );

	rOffset = LTRotation( LTCos( fScale * m_vShakeWaveLengths.x ) * fStrength, LTSin( fScale * m_vShakeWaveLengths.y ) * fStrength, 0.0f );
}

