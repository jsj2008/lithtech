// ----------------------------------------------------------------------- //
//
// MODULE  : AccuracyVisualization.cpp
//
// PURPOSE : definition of class to visualize perturb
//
// CREATED : 04/11/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "AccuracyVisualization.h"
#include "AccuracyMgr.h"
#include "ClientDB.h"
#include "ltbeziercurve.h"

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

AccuracyVisualization::AccuracyVisualization()
{
	m_hFollowObject		= NULL;
	m_hRecord			= NULL;

	m_bOn				= false;
	m_bWasOn			= false;
	m_bEnabled			= true;

}

// ****************************************************************************************** //

AccuracyVisualization::~AccuracyVisualization()
{
	if( m_sLight.m_hLight )
	{
		g_pLTClient->RemoveObject( m_sLight.m_hLight );
	}

	m_sLight.Init();
}

// ****************************************************************************************** //

bool AccuracyVisualization::Initialize( HOBJECT hFollowObject )
{
	ClientDB& iClientDatabase = ClientDB::Instance();
	HRECORD hRecord = g_pLTDatabase->GetRecord( iClientDatabase.GetFlashlightCategory(), "PerturbVis" );

	// Validate parameters
	if( !hFollowObject || !hRecord )
	{
		LTERROR( "AccuracyVisualization needs an object to follow, and a valid database record!" );
		return false;
	}

	// Save the object that we're wanting to follow, and the database record
	bool bNewRecord = ( ( m_hRecord != hRecord ) ? true : false );
	m_hFollowObject = hFollowObject;

	// Read out the properties of the AccuracyVisualization from the database
	if( bNewRecord )
	{
		// Save the current record...
		m_hRecord = hRecord;

		HATTRIBUTE hLightData = g_pLTDatabase->GetAttribute( m_hRecord, "LightData" );

		// Get all the new values out of the database...
		m_sLight.m_bCreateLight = g_pLTDatabase->GetBool( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "CreateLight" ), 0, true );
		m_sLight.m_sLightTexture = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "LightTexture" ), 0, "" );
		m_sLight.m_sLightType = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "LightType" ), 0, "" );
		m_sLight.m_vLightRadiusRange = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "LightRadiusRange" ), 0, LTVector2() );
		m_sLight.m_nLightColor = g_pLTDatabase->GetInt32( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "LightColor" ), 0, 0xFFFFFFFF );
		m_sLight.m_vLightPositionOffset = g_pLTDatabase->GetVector3( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "LightPositionOffset" ), 0, LTVector() );
		m_sLight.m_vLightRotationOffset = g_pLTDatabase->GetVector3( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "LightRotationOffset" ), 0, LTVector() );
		m_sLight.m_vLightIntensity = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "LightIntensity" ), 0, LTVector2() );
		m_sLight.m_vLightFOV = g_pLTDatabase->GetVector2( CGameDatabaseReader::GetStructAttribute( hLightData, 0, "LightFOV" ), 0, LTVector2() );


		// Make sure the light properties are set properly
		Helper_SetLightProperties();
	}

	return true;
}

// ****************************************************************************************** //


// ****************************************************************************************** //

void AccuracyVisualization::Update( float fFrameTime )
{
	// Get the initial transform of the light
	LTRigidTransform iTrans;
	Helper_GetTransform( iTrans );

	float fPerturb = max(0.1f,CAccuracyMgr::Instance().GetCurrentWeaponPerturb());
	float fFOV = atan2f(fPerturb,1000.0f);
	g_pLTClient->SetLightSpotInfo( m_sLight.m_hLight, fFOV, fFOV, m_sLight.m_vLightRadiusRange.x );



	// ------------------------------------------------------------------------------------------ //
	// Set the final values for the light

	if( m_sLight.m_hLight )
	{
		// Set the final transform!
		LTRigidTransform iLightTrans = iTrans;
//		Helper_OffsetTranform( iLightTrans, 0 );

		g_pLTClient->SetObjectTransform( m_sLight.m_hLight, iLightTrans );
	}

}

// ****************************************************************************************** //

void AccuracyVisualization::Enable( bool bEnable )
{
	if( bEnable )
	{
		m_bEnabled = true;

		if( m_bWasOn )
		{
			TurnOn();
		}
	}
	else
	{
		m_bWasOn = m_bOn;

		m_bEnabled = false;
		TurnOff();
	}
}

// ****************************************************************************************** //

bool AccuracyVisualization::TurnOn()
{
	// Make sure the AccuracyVisualization is enabled and not already turned on (cause I'm turned on!!)
	if( !m_bEnabled || m_bOn )
	{
		return false;
	}

	// Verify that the light has been created...
	Helper_CreateLight();

	if( m_sLight.m_hLight )
	{
		g_pCommonLT->SetObjectFlags( m_sLight.m_hLight, OFT_Flags, FLAG_VISIBLE , FLAG_VISIBLE );
	}

	m_bOn = true;

	// Go ahead and do an update so the light doesn't do some weird interpolation
	Update( 1000.0f );

	return true;
}

// ****************************************************************************************** //

bool AccuracyVisualization::TurnOff()
{
	// Make sure the AccuracyVisualization isn't already turned off (it sucks to be turned off!!)
	if( !m_bOn )
	{
		return false;
	}

	if( m_sLight.m_hLight )
	{
		g_pCommonLT->SetObjectFlags( m_sLight.m_hLight, OFT_Flags, 0, FLAG_VISIBLE );
	}

	m_bOn = false;


	return true;
}

// ****************************************************************************************** //

void AccuracyVisualization::Helper_CreateLight()
{
	// Get some information about the follow object
	LTRigidTransform iTrans;
	Helper_GetTransform( iTrans );

	// If a light has already been created or the light is not supposed to be created...  then just move on
	if( !m_sLight.m_hLight && m_sLight.m_bCreateLight )
	{
		LTRigidTransform iLightTrans = iTrans;
		Helper_OffsetTranform( iLightTrans, 0 );

		// Fill in the object creation structure and do it!
		ObjectCreateStruct iOCS;

		iOCS.m_ObjectType = OT_LIGHT;
		iOCS.m_Flags = 0;
		iOCS.m_Pos = iLightTrans.m_vPos;
		iOCS.m_Rotation = iLightTrans.m_rRot;

		m_sLight.m_hLight = g_pLTClient->CreateObject( &iOCS );
	}

	// Go ahead and setup the light properties
	Helper_SetLightProperties();
}

// ****************************************************************************************** //

void AccuracyVisualization::Helper_SetLightProperties()
{
	// Make sure the light is valid
	if( m_sLight.m_hLight && m_sLight.m_sLightType )
	{
		g_pLTClient->SetLightType( m_sLight.m_hLight, eEngineLight_SpotProjector );
		// Set the shadow LOD of the light
		EEngineLOD eShadowLOD = eEngineLOD_Never;
		g_pLTClient->SetLightDetailSettings( m_sLight.m_hLight, eEngineLOD_Low, eShadowLOD, eShadowLOD );

		// Set everything else...
		g_pLTClient->SetLightTexture( m_sLight.m_hLight, m_sLight.m_sLightTexture );
		g_pLTClient->SetObjectColor( m_sLight.m_hLight, ( ( ( m_sLight.m_nLightColor >> 16 ) & 0xFF ) / 255.0f ), ( ( ( m_sLight.m_nLightColor >> 8 ) & 0xFF ) / 255.0f ), ( ( ( m_sLight.m_nLightColor >> 0 ) & 0xFF ) / 255.0f ), 1.0f );
		g_pLTClient->SetLightRadius( m_sLight.m_hLight, m_sLight.m_vLightRadiusRange.y );
		g_pLTClient->SetLightSpotInfo( m_sLight.m_hLight, MATH_DEGREES_TO_RADIANS( m_sLight.m_vLightFOV.x ), MATH_DEGREES_TO_RADIANS( m_sLight.m_vLightFOV.y ), m_sLight.m_vLightRadiusRange.x );
		g_pLTClient->SetLightIntensityScale( m_sLight.m_hLight, m_sLight.m_vLightIntensity.y );
	}

	if( m_sLight.m_hLight )
	{
		g_pCommonLT->SetObjectFlags( m_sLight.m_hLight, OFT_Flags, (  m_bOn  ? FLAG_VISIBLE : 0 ), FLAG_VISIBLE );
	}
}

// ****************************************************************************************** //

void AccuracyVisualization::Helper_GetTransform( LTRigidTransform& iTrans )
{
	// Make sure we have an object to follow... otherwise we won't have a valid transform
	if( !m_hFollowObject )
	{
		iTrans.Init();
		return;
	}

	g_pLTClient->GetObjectTransform( m_hFollowObject, &iTrans );

}

// ****************************************************************************************** //

void AccuracyVisualization::Helper_OffsetTranform( LTRigidTransform& iTrans, uint32 nLight )
{
	// Calculate a final transform for this light
	LTRotation rLightOffset( m_sLight.m_vLightRotationOffset.x, m_sLight.m_vLightRotationOffset.y, m_sLight.m_vLightRotationOffset.z );

	iTrans.m_rRot = ( rLightOffset * iTrans.m_rRot );

	iTrans.m_vPos += ( iTrans.m_rRot.Right() * m_sLight.m_vLightPositionOffset.x );
	iTrans.m_vPos += ( iTrans.m_rRot.Up() * m_sLight.m_vLightPositionOffset.y );
	iTrans.m_vPos += ( iTrans.m_rRot.Forward() * m_sLight.m_vLightPositionOffset.z );
}

