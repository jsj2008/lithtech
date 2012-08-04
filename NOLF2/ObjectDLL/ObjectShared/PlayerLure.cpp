// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerLure.cpp
//
// PURPOSE : PlayerLure implementation
//
// CREATED : 4/21/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ServerSpecialFX.h"
#include "PlayerLure.h"

LINKFROM_MODULE( PlayerLure );

class CPlayerLurePlugin : public IObjectPlugin
{
  public:

    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

  protected:
	  
	  SpecialFXPlugin m_FXPlugin;
};



BEGIN_CLASS( PlayerLure )
	ADD_VISIBLE_FLAG(0, PF_HIDDEN)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_SHADOW_FLAG(0, PF_HIDDEN)
	ADD_STRINGPROP_FLAG( PlayerLureCameraFreedom, "", PF_STATICLIST)
	ADD_STRINGPROP( LimitedYawLeftRight, "-360 360" )
	ADD_STRINGPROP( LimitedPitchDownUp, "-25 180" )
	ADD_BOOLPROP( AllowWeapon, 0 )
	ADD_BOOLPROP( RetainOffsets, 0 )
	ADD_BOOLPROP( Bicycle, 0 )
	ADD_STRINGPROP_FLAG( DeathFX, "<none>", PF_STATICLIST )
END_CLASS_DEFAULT_FLAGS_PLUGIN(PlayerLure, GameBase, NULL, NULL, 0, CPlayerLurePlugin)

#define CAMERA_FREEDOM_NONE			"None"
#define CAMERA_FREEDOM_LIMITED		"Limited"
#define CAMERA_FREEDOM_UNLIMITED	"Unlimited"

uint32 PlayerLure::s_nNextPlayerLureId = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CameraFreedomStringToEnum
//
//	PURPOSE:	Convert PlayerLureCameraFreedom string to enum
//
// ----------------------------------------------------------------------- //

static PlayerLureCameraFreedom CameraFreedomStringToEnum( char const* pszCameraFreedom )
{
	PlayerLureCameraFreedom eCameraFreedom = kPlayerLureCameraFreedomNone;

	if( !stricmp( pszCameraFreedom, CAMERA_FREEDOM_NONE ))
	{
		eCameraFreedom = kPlayerLureCameraFreedomNone;
	}
	else if( !stricmp( pszCameraFreedom, CAMERA_FREEDOM_LIMITED ))
	{
		eCameraFreedom = kPlayerLureCameraFreedomLimited;
	}
	else if( !stricmp( pszCameraFreedom, CAMERA_FREEDOM_UNLIMITED ))
	{
		eCameraFreedom = kPlayerLureCameraFreedomUnlimited;
	}
	else
	{
		ASSERT( !"CameraFreedomStringToEnum: Invalid camera freedom string." );
	}

	return eCameraFreedom;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLure::PlayerLure
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PlayerLure::PlayerLure( )
{
	m_eCameraFreedom = kPlayerLureCameraFreedomNone;
	m_bAllowWeapon = false;
	m_bRetainOffsets = false;
	m_bBicycle = false;
	m_nPlayerLureId = s_nNextPlayerLureId;
	s_nNextPlayerLureId++;

	m_fLimitedYawLeft = -360;
	m_fLimitedYawRight = 360;
	m_fLimitedPitchDown = -25;
	m_fLimitedPitchUp = 180;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLure::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PlayerLure::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 nRes = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp( );
			}

			return nRes;
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 nRes = GameBase::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate( );
			}
			
			return nRes;
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLure::ReadProp
//
//	PURPOSE:	Read object properties.
//
// ----------------------------------------------------------------------- //

void PlayerLure::ReadProp( )
{
	GenericProp genProp;

	if( g_pLTServer->GetPropGeneric( "PlayerLureCameraFreedom", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		m_eCameraFreedom = CameraFreedomStringToEnum( genProp.m_String );
	}

	if( g_pLTServer->GetPropGeneric( "LimitedYawLeftRight", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		sscanf( genProp.m_String, "%f %f", &m_fLimitedYawLeft, &m_fLimitedYawRight );
	}

	if( g_pLTServer->GetPropGeneric( "LimitedPitchDownUp", &genProp ) == LT_OK && genProp.m_String[0] )
	{
		sscanf( genProp.m_String, "%f %f", &m_fLimitedPitchDown, &m_fLimitedPitchUp );
	}

	if( g_pLTServer->GetPropGeneric( "AllowWeapon", &genProp ) == LT_OK )
	{
		m_bAllowWeapon = !!genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "RetainOffsets", &genProp ) == LT_OK )
	{
		m_bRetainOffsets = !!genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "Bicycle", &genProp ) == LT_OK )
	{
		m_bBicycle = !!genProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "DeathFX", &genProp ) == LT_OK )
	{
		if( genProp.m_String[0] )
		{
			m_sDeathFX = genProp.m_String;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLure::InitialUpdate
//
//	PURPOSE:	Setup the object.
//
// ----------------------------------------------------------------------- //

void PlayerLure::InitialUpdate( )
{
	// Set the object flags.
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES, ( FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES ));

	// Set our special effect message.
	CAutoMessage cMsg;
	cMsg.Writeuint8( SFX_PLAYERLURE_ID );
	// This is our unique id.
	cMsg.Writeuint32( m_nPlayerLureId );
	cMsg.Writeuint8( m_eCameraFreedom );
	BYTE nFlags = 0;
	if( m_bAllowWeapon )
		nFlags |= ePlayerLureFlagsAllowWeapon;
	if( m_bRetainOffsets )
		nFlags |= ePlayerLureFlagsRetainOffsets;
	if( m_bBicycle )
		nFlags |= ePlayerLureFlagsBicycle;
	cMsg.Writeuint8( nFlags );
	cMsg.WriteString( m_sDeathFX.c_str( ));
	if( m_eCameraFreedom == kPlayerLureCameraFreedomLimited )
	{
		cMsg.Writefloat( m_fLimitedYawLeft );
		cMsg.Writefloat( m_fLimitedYawRight );
		cMsg.Writefloat( m_fLimitedPitchDown );
		cMsg.Writefloat( m_fLimitedPitchUp );
	}
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLure::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PlayerLure::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_BYTE( m_eCameraFreedom );
	SAVE_bool( m_bAllowWeapon );
	SAVE_bool( m_bRetainOffsets );
	SAVE_bool( m_bBicycle );
	SAVE_CHARSTRING( m_sDeathFX.c_str( ));
	SAVE_DWORD( m_nPlayerLureId );

	SAVE_FLOAT( m_fLimitedYawLeft );
	SAVE_FLOAT( m_fLimitedYawRight );
	SAVE_FLOAT( m_fLimitedPitchDown );
	SAVE_FLOAT( m_fLimitedPitchUp );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLure::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PlayerLure::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;
	char szString[256];

	LOAD_BYTE_CAST( m_eCameraFreedom, PlayerLureCameraFreedom );
	LOAD_bool( m_bAllowWeapon );
	LOAD_bool( m_bRetainOffsets );
	LOAD_bool( m_bBicycle );
	LOAD_CHARSTRING( szString, ARRAY_LEN( szString ));
	m_sDeathFX = szString;
	LOAD_DWORD( m_nPlayerLureId );

	// Make sure the global playerlure id is beyond our loaded id.
	if( s_nNextPlayerLureId <= m_nPlayerLureId )
		s_nNextPlayerLureId = m_nPlayerLureId + 1;

	LOAD_FLOAT( m_fLimitedYawLeft );
	LOAD_FLOAT( m_fLimitedYawRight );
	LOAD_FLOAT( m_fLimitedPitchDown );
	LOAD_FLOAT( m_fLimitedPitchUp );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PlayerLure::WriteVehicleMessage
//
//	PURPOSE:	Write out the vehicle message for the playerlure.
//
// ----------------------------------------------------------------------- //

bool PlayerLure::WriteVehicleMessage( ILTMessage_Write& msg )
{
	// Write our unique pointer as our id.
	msg.Writeuint32( m_nPlayerLureId );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerLurePlugin::PreHook_EditStringList
//
//	PURPOSE:	Handle dedit call for listbox.
//
// ----------------------------------------------------------------------- //

LTRESULT CPlayerLurePlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	// Handle lure type...

	if( _strcmpi("PlayerLureCameraFreedom", szPropName ) == 0 )
	{
		strcpy(aszStrings[( *pcStrings )++], CAMERA_FREEDOM_NONE );
		strcpy(aszStrings[( *pcStrings )++], CAMERA_FREEDOM_LIMITED );
		strcpy(aszStrings[( *pcStrings )++], CAMERA_FREEDOM_UNLIMITED );

		return LT_OK;
	}
	else if( _strcmpi( "DeathFX", szPropName ) == 0 )
	{
		return m_FXPlugin.PopulateStringList( szRezPath,
											  aszStrings,
											  pcStrings,
											  cMaxStrings,
											  cMaxStringLength );
	}

	return LT_UNSUPPORTED;
}
