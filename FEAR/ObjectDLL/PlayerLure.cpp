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

#include "Stdafx.h"
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
	ADD_STRINGPROP_FLAG( PlayerLureCameraFreedom, "", PF_STATICLIST, "Specifies the amount of restriction to place on the camera rotation.")
	ADD_STRINGPROP( LimitedYawLeftRight, "-360 360", "Specifies camera rotation to left and right of center that is allowed.  Separate left and right value with a space.  Setting left < -180 and right > 180 means unlimited yaw.")
	ADD_STRINGPROP( LimitedPitchDownUp, "-25 180", "Specifies camera rotation down and up from center that is allowed.  Separate down and up value with a space.  Unlimited pitch is not allowed.")
	ADD_BOOLPROP( AllowWeapon, false, "Allow/disallow the weapon during lure.  Default:  False")
	ADD_BOOLPROP( AllowSwitchWeapon, false, "Allow/disalow switching weapons during lure. Default: False" )
	ADD_BOOLPROP( RetainOffsets, false, "Player retains position and rotation offset from PlayerLure.  Default:  False." )
	ADD_BOOLPROP( AllowLean, false, "Allow player to lean while under playerlure.  Default:  False." )
	ADD_BOOLPROP( AllowCrouch, false, "Allow player to crouch while under playerlure.  Default:  False." )
	ADD_BOOLPROP( TrackYaw, true, "Camera tracks yaw changes in playerlure.  Default:  True." )
	ADD_BOOLPROP( TrackPitch, true, "Camera tracks pitch changes in playerlure.  Default:  True." )
	ADD_STRINGPROP_FLAG( DeathFX, "<none>", PF_STATICLIST, "DeathFX to play if player dies while following lure." )
	ADD_BOOLPROP( AllowBodyRotation, false, "Allow the player to control body rotation while lured.  Default:  False")
END_CLASS_FLAGS_PLUGIN(PlayerLure, GameBase, 0, CPlayerLurePlugin, "The PlayerLure object is used to properly KeyFrame the Player using the KeyFramer object." )

CMDMGR_BEGIN_REGISTER_CLASS( PlayerLure )
CMDMGR_END_REGISTER_CLASS( PlayerLure, GameBase )

#define CAMERA_FREEDOM_NONE			"None"
#define CAMERA_FREEDOM_LIMITED		"Limited"
#define CAMERA_FREEDOM_UNLIMITED	"Unlimited"

uint32 PlayerLure::s_nNextPlayerLureId = PlayerLureId_Invalid + 1;

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

	if( !LTStrICmp( pszCameraFreedom, CAMERA_FREEDOM_NONE ))
	{
		eCameraFreedom = kPlayerLureCameraFreedomNone;
	}
	else if( !LTStrICmp( pszCameraFreedom, CAMERA_FREEDOM_LIMITED ))
	{
		eCameraFreedom = kPlayerLureCameraFreedomLimited;
	}
	else if( !LTStrICmp( pszCameraFreedom, CAMERA_FREEDOM_UNLIMITED ))
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
	m_bAllowSwitchWeapon = false;
	m_bAllowLean = false;
	m_bAllowCrouch = false;
	m_bAllowBodyRotation = false;
	m_bTrackPitch = true;
	m_bTrackYaw = true;
	m_bRetainOffsets = false;
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

uint32 PlayerLure::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			uint32 nRes = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
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

void PlayerLure::ReadProp(const GenericPropList *pProps)
{
	const char *pszFreedom = pProps->GetString( "PlayerLureCameraFreedom", "" );
	if( pszFreedom && pszFreedom[0] )
		m_eCameraFreedom = CameraFreedomStringToEnum( pszFreedom );
	
	const char *pszLimitedYaw = pProps->GetString( "LimitedYawLeftRight", "-360 360" );
	if( pszLimitedYaw && pszLimitedYaw[0] )
		sscanf( pszLimitedYaw, "%f %f", &m_fLimitedYawLeft, &m_fLimitedYawRight );
	
	const char *pszLimitedPitch = pProps->GetString( "LimitedPitchDownUp", "-25 180" );
	if( pszLimitedPitch && pszLimitedPitch[0] )
		sscanf( pszLimitedPitch, "%f %f", &m_fLimitedPitchDown, &m_fLimitedPitchUp );
	
	m_bAllowWeapon		= pProps->GetBool( "AllowWeapon", m_bAllowWeapon );
	m_bAllowSwitchWeapon = pProps->GetBool( "AllowSwitchWeapon", m_bAllowSwitchWeapon );
	m_bAllowLean		= pProps->GetBool( "AllowLean", m_bAllowLean );
	m_bAllowCrouch		= pProps->GetBool( "AllowCrouch", m_bAllowCrouch );
	m_bAllowBodyRotation = pProps->GetBool( "AllowBodyRotation", m_bAllowBodyRotation );
	m_bTrackPitch		= pProps->GetBool( "TrackPitch", m_bTrackPitch );
	m_bTrackYaw			= pProps->GetBool( "TrackYaw", m_bTrackYaw );
	m_bRetainOffsets	= pProps->GetBool( "RetainOffsets", m_bRetainOffsets );
	m_sDeathFX			= pProps->GetString( "DeathFX", "" );
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
	cMsg.Writebool( m_bAllowWeapon );
	cMsg.Writebool( m_bAllowSwitchWeapon );
	cMsg.Writebool( m_bAllowLean );
	cMsg.Writebool( m_bAllowCrouch );
	cMsg.Writebool( m_bAllowBodyRotation );
	cMsg.Writebool( m_bTrackPitch );
	cMsg.Writebool( m_bTrackYaw );
	cMsg.Writebool( m_bRetainOffsets );
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
	SAVE_bool( m_bAllowSwitchWeapon );
	SAVE_bool( m_bAllowLean );
	SAVE_bool( m_bAllowCrouch );
	SAVE_bool( m_bAllowBodyRotation );
	SAVE_bool( m_bTrackPitch );
	SAVE_bool( m_bTrackYaw );
	SAVE_bool( m_bRetainOffsets );
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
	LOAD_bool( m_bAllowSwitchWeapon );
	LOAD_bool( m_bAllowLean );
	LOAD_bool( m_bAllowCrouch );
	LOAD_bool( m_bAllowBodyRotation );
	LOAD_bool( m_bTrackPitch );
	LOAD_bool( m_bTrackYaw );
	LOAD_bool( m_bRetainOffsets );
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
//	PURPOSE:	Handle WorldEdit call for listbox.
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

	if( LTStrICmp("PlayerLureCameraFreedom", szPropName ) == 0 )
	{
		strcpy(aszStrings[( *pcStrings )++], CAMERA_FREEDOM_NONE );
		strcpy(aszStrings[( *pcStrings )++], CAMERA_FREEDOM_LIMITED );
		strcpy(aszStrings[( *pcStrings )++], CAMERA_FREEDOM_UNLIMITED );

		return LT_OK;
	}
	else if( LTStrICmp( "DeathFX", szPropName ) == 0 )
	{
		return m_FXPlugin.PopulateStringList( szRezPath,
											  aszStrings,
											  pcStrings,
											  cMaxStrings,
											  cMaxStringLength );
	}

	return LT_UNSUPPORTED;
}
