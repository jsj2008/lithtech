// ----------------------------------------------------------------------- //
//
// MODULE  : WorldProperties.cpp
//
// PURPOSE : WorldProperties object - Implementation
//
// CREATED : 9/25/98
//
// (c) 1998-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "WorldProperties.h"
#include "GameServerShell.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "MsgIDs.h"
#include "AIStimulusMgr.h"
#include "ServerMissionMgr.h"
#include "ServerSaveLoadMgr.h"
#include "SoundFilterDB.h"
#include "SoundMixerDB.h"
#include "CommandDB.h"
#include "StringUtilities.h"
#include "GameModeMgr.h"
#include "TeamMgr.h"

LINKFROM_MODULE( WorldProperties );


extern CGameServerShell *g_pGameServerShell;
extern CAIStimulusMgr *g_pAIStimulusMgr;

WorldProperties* g_pWorldProperties = NULL;

BEGIN_CLASS(WorldProperties)

	ADD_REALPROP(FarZ, 100000.0f, "This value sets the distance of the farZ-clipping plane for the level.")
	ADD_BOOLPROP(ClampFarZ, true, "If set to true the FarZ will be calamped to the FogFarZ and the diminsions of the world.")
	ADD_COMMANDPROP_FLAG( LevelEndCmd, "", PF_NOTIFYCHANGE, "Command sent when the level ends.")

	ADD_STRINGPROP_FLAG(GlobalSoundFilter, "UnFiltered", PF_STATICLIST, "This field allows you to choose a sound filter that will be used for the entire level.")
	ADD_STRINGPROP_FLAG(GlobalSoundMixer,  "Main", PF_STATICLIST, "This field allows you to choose a sound mixer that will be used for the entire level.")

	PROP_DEFINEGROUP(FogInfo, PF_GROUP(1), "This is a subset of properties that define the overall fogging effects for the entire level.")
		ADD_BOOLPROP_FLAG(FogEnable, false, PF_GROUP(1), "This flag toggles the fog effects on and off.")
		ADD_COLORPROP_FLAG(FogColor, 127.0f, 127.0f, 127.0f, PF_GROUP(1), "This color picker determines the color of the fog effect.")
		ADD_REALPROP_FLAG(FogNearZ, 1.0f, PF_GROUP(1), "This value determines at what distance the fog is at zero saturation.")
		ADD_REALPROP_FLAG(FogFarZ, 5000.0f, PF_GROUP(1), "This is a value that determines at what distance the fog reaches full saturation.")
		ADD_BOOLPROP_FLAG(SkyFogEnable, false, PF_GROUP(1), "This flag toggles the SkyFog effects on and off.")
		ADD_REALPROP_FLAG(SkyFogNearZ, 100.0f, PF_GROUP(1), "This value determines at what distance the fog is at zero saturation.")
		ADD_REALPROP_FLAG(SkyFogFarZ, 1000.0f, PF_GROUP(1), "This is a value that determines at what distance the fog reaches full saturation.")

	ADD_COLORPROP(AmbientLight, 0, 0, 0, "Ambient light color in the level.")
	ADD_REALPROP(AddAmbientLightLow, 0.0f, "The additional ambient white light to add for detail setting LOW. 0 none, 1 full.")
	ADD_REALPROP(AddAmbientLightMed, 0.0f, "The additional ambient white light to add for detail setting MEDIUM. 0 none, 1 full.")
	ADD_REALPROP(AddAmbientLightHigh, 0.0f, "The additional ambient white light to add for detail setting HIGH. 0 none, 1 full.")
	ADD_COLORPROP(SkyAmbientLight, 0, 0, 0, "Ambient light color in the sky.")

	ADD_STRINGPROP_FLAG(WorldType, "SinglePlayer", PF_STATICLIST, "This is the world type, which controls the types of resources that are prefetched during world processing.")

END_CLASS_FLAGS_PLUGIN_PREFETCH(WorldProperties, GameBase, 0, CWorldPropertiesPlugin, PrefetchPlayer, "This object contains properties, such as fog, that help define the level.")


//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( WorldProperties )
	
	ADD_MESSAGE( FOGCOLOR,		4,	NULL,	MSG_HANDLER( WorldProperties, HandleFogColorMsg ),		"FOGCOLOR <r> <g> <b>", "This message changes the color of the fog color of the world", "msg WorldProperties (FOGCOLOR 255 126 210)" )
	ADD_MESSAGE( FOGENABLE,		2,	NULL,	MSG_HANDLER( WorldProperties, HandleFogEnableMsg ),		"FOGENABLE <1-or-0>", "This message toggles the fog in the world on and off", "msg WorldProperties (FOGENABLE 0)" )
	ADD_MESSAGE( FOGNEARZ,		2,	NULL,	MSG_HANDLER( WorldProperties, HandleFogNearZMsg ),		"FOGNEARZ <near z>", "Changes the near z of the fog plane", "msg WorldProperties (FOGNEARZ 300)" )
	ADD_MESSAGE( FOGFARZ,		2,	NULL,	MSG_HANDLER( WorldProperties, HandleFogFarZMsg ),		"FOGFARZ <far z>", "Changes the far z of the fog plane", "msg WorldProperties (FOGFARZ 300)" )
	ADD_MESSAGE( SKYFOGENABLE,	2,	NULL,	MSG_HANDLER( WorldProperties, HandleSkyFogEnableMsg ),	"SKYFOGENABLE <1-or-0>", "This message toggles the skyfog in the world on and off", "msg WorldProperties (SKYFOGENABLE 0)" )
	ADD_MESSAGE( SKYFOGFARZ,	2,	NULL,	MSG_HANDLER( WorldProperties, HandleSkyFogFarZMsg ),	"SKYFOGFARZ <far z>", "Changes the far z of the skyfog plane", "msg WorldProperties (SKYFOGFARZ 300)" )
	ADD_MESSAGE( SKYFOGNEARZ,	2,	NULL,	MSG_HANDLER( WorldProperties, HandleSkyFogNearZMsg ),	"SKYFOGNEARZ <near z>", "Changes the near z of the skyfog plane", "msg WorldProperties (SKYFOGNEARZ 300)" )
	ADD_MESSAGE( NEXTROUND,		1,	NULL,	MSG_HANDLER( WorldProperties, HandleNextRoundMsg ),		"NEXTROUND", "In a non-cooperative multiplayer game this message will advance the game to the next round.", "msg WorldProperties NEXTROUND" )
	ADD_MESSAGE( ROUNDWON,		2,	NULL,	MSG_HANDLER( WorldProperties, HandleRoundWonMsg ),		"ROUNDWON <0 or 1>", "In a non-cooperative team multiplayer game this message will end the round and the team specified in the message wins the round.", "msg WorldProperties (ROUNDWON 1)" )
	ADD_MESSAGE( ADDTEAMSCORE,	3,	NULL,	MSG_HANDLER( WorldProperties, HandleAddTeamScoreMsg ),		"ADDTEAMSCORE <0 or 1> <bonus>", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( CHECKPOINTSAVE, 	1, 	NULL, 	MSG_HANDLER( WorldProperties, HandleCheckpointSaveMsg ),	"CHECKPOINTSAVE", "Create a checkpoint save.", "msg WorldProperties CHECKPOINTSAVE" )
	ADD_MESSAGE( CANSAVE, 		2, 	NULL, 	MSG_HANDLER( WorldProperties, HandleCanSaveMsg ),		"CANSAVE <0 or 1>", "Allows disabling of creating save games up to a CANSAVE 1", "msg WorldProperties (CANSAVE 0)" )

	ADD_MESSAGE_ARG_RANGE( TRANSMISSION,	2,	3,	NULL,	MSG_HANDLER( WorldProperties, HandleTransmissionMsg ),	"TRANSMISSION <id> <optional: sound id>", "This message will display a text message in the HUD to all clients.  The ID parameter is a string id in the string table of Cres.dll.  The sound ID parameter is the number of a voice file.", "msg WorldProperties (TRANSMISSION 12345 121212)" )
	ADD_MESSAGE( ATTACKINGTEAM, 	2, 	NULL, 	MSG_HANDLER( WorldProperties, HandleAttackingTeamMsg ), 	"ATTACKINGTEAM <0 or 1>", "TODO:CMDDESC", "TODO:CMDEXP" )

CMDMGR_END_REGISTER_CLASS( WorldProperties, GameBase )

static char s_szFarZ[] = "FarZ";
static char s_szClampFarZ[] = "ClampFarZ";
static char s_szFogEnable[] = "FogEnable";
static char s_szFogColor[] = "FogColor";
static char s_szFogR[] = "FogR";
static char s_szFogG[] = "FogG";
static char s_szFogB[] = "FogB";
static char s_szFogNearZ[] = "FogNearZ";
static char s_szFogFarZ[] = "FogFarZ";
static char s_szSkyFogEnable[] = "SkyFogEnable";
static char s_szSkyFogNearZ[] = "SkyFogNearZ";
static char s_szSkyFogFarZ[] = "SkyFogFarZ";
static char s_szSoundFilter[] = "GlobalSoundFilter";
static char s_szSoundMixer[] = "GlobalSoundMixer";
static char s_szAmbientLightR[] = "Light_AmbientR";
static char s_szAmbientLightG[] = "Light_AmbientG";
static char s_szAmbientLightB[] = "Light_AmbientB";
static char s_szSkyAmbientLightR[] = "Light_SkyAmbientR";
static char s_szSkyAmbientLightG[] = "Light_SkyAmbientG";
static char s_szSkyAmbientLightB[] = "Light_SkyAmbientB";
static char s_szWorldType[] = "WorldType";

struct SWorldType
{
	char*						m_pszWorldType;
	WorldProperties::EWorldType	m_eWorldType;
};

static SWorldType s_WorldTypes[] =
{
	{ "SinglePlayer", WorldProperties::eWorldType_SinglePlayer },
	{ "MultiPlayer", WorldProperties::eWorldType_MultiPlayer }
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWorldPropertiesPlugin::Constructor/Destructor()
//
//	PURPOSE:	Handle allocating and deallocating the required plugins
//
// ----------------------------------------------------------------------- //

CWorldPropertiesPlugin::CWorldPropertiesPlugin()
{
	m_pSoundFilterDBPlugin = debug_new(SoundFilterDBPlugin);
	m_pSoundMixerDBPlugin = debug_new(CSoundMixerDBPlugin);
}

CWorldPropertiesPlugin::~CWorldPropertiesPlugin()
{
	if (m_pSoundFilterDBPlugin)
	{
		debug_delete(m_pSoundFilterDBPlugin);
		m_pSoundFilterDBPlugin = NULL;
	}

	if (m_pSoundMixerDBPlugin)
	{
		debug_delete(m_pSoundMixerDBPlugin);
		m_pSoundMixerDBPlugin = NULL;
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWorldPropertiesPlugin::PreHook_PropChanged
//
//  PURPOSE:	Make sure the Command is valid
//
// ----------------------------------------------------------------------- //

LTRESULT CWorldPropertiesPlugin::PreHook_PropChanged( const char *szObjName,
													 const char *szPropName,
												     const int nPropType,
												     const GenericProp &gpPropValue,
												     ILTPreInterface *pInterface,
													 const	char *szModifiers )
{
	if( LT_OK == m_CmdMgrPlugin.PreHook_PropChanged( szObjName,
														 szPropName,
														 nPropType, 
														 gpPropValue,
														 pInterface,
														 szModifiers ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT CWorldPropertiesPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
    if( LTStrIEquals( s_szSoundFilter, szPropName ))
	{
		if (!m_pSoundFilterDBPlugin->PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength))
		{
			return LT_UNSUPPORTED;
		}

		return LT_OK;
	}

    if( LTStrIEquals( s_szSoundMixer, szPropName ))
	{
		if (!m_pSoundMixerDBPlugin->PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength))
		{
			return LT_UNSUPPORTED;
		}

		return LT_OK;
	}

	if( LTStrIEquals( s_szWorldType, szPropName ))
	{
		for( int nIndex = 0; nIndex < WorldProperties::eWorldType_Count; nIndex++ )
		{
			LTStrCpy(aszStrings[(*pcStrings)++], s_WorldTypes[nIndex].m_pszWorldType, cMaxStringLength );
		}

		return LT_OK;	
	}

	return LT_UNSUPPORTED;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::WorldProperties()
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

WorldProperties::WorldProperties()
{
	// There should be only one world properties object per level...
	LTASSERT( !g_pWorldProperties, "WorldProperties ERROR!  More than one WorldProperties object detected!" );
	if (g_pWorldProperties)
	{
        g_pLTServer->CPrint("WorldProperties ERROR!  More than one WorldProperties object detected!");
	}

	g_pWorldProperties = this;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::~WorldProperties()
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

WorldProperties::~WorldProperties( )
{
	g_pWorldProperties = NULL;

	g_pLTServer->SetConsoleVariableString( s_szFogEnable, "" );
	g_pLTServer->SetConsoleVariableString( s_szFogR, "" );
	g_pLTServer->SetConsoleVariableString( s_szFogG, "" );
	g_pLTServer->SetConsoleVariableString( s_szFogB, "" );
	g_pLTServer->SetConsoleVariableString( s_szFogNearZ, "" );
	g_pLTServer->SetConsoleVariableString( s_szFogFarZ, "" );
	g_pLTServer->SetConsoleVariableString( s_szSkyFogEnable, "" );
	g_pLTServer->SetConsoleVariableString( s_szSkyFogNearZ, "" );
	g_pLTServer->SetConsoleVariableString( s_szSkyFogFarZ, "" );
	g_pLTServer->SetConsoleVariableString( s_szSoundFilter, "" );
	g_pLTServer->SetConsoleVariableString( s_szSoundMixer, "" );
	g_pLTServer->SetConsoleVariableString( s_szAmbientLightR, "" );
	g_pLTServer->SetConsoleVariableString( s_szAmbientLightG, "" );
	g_pLTServer->SetConsoleVariableString( s_szAmbientLightB, "" );
	g_pLTServer->SetConsoleVariableString( s_szSkyAmbientLightR, "" );
	g_pLTServer->SetConsoleVariableString( s_szSkyAmbientLightG, "" );
	g_pLTServer->SetConsoleVariableString( s_szSkyAmbientLightB, "" );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// --------------------------------------------------------------------------- //

uint32 WorldProperties::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE )
			{
				ReadProps(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			LTRotation rRot;
			g_pLTServer->GetObjectRotation( m_hObject, &rRot );

			// Get the forward vector...
			LTVector vForward = rRot.Forward();

			// ...convert the xz rotation to an angle...
			float fAngle = static_cast< float >( atan2( vForward.x, vForward.z ) );

			g_pLTServer->SetConsoleVariableFloat("WorldNorth",fAngle);

            g_pLTServer->SetObjectState(m_hObject, OBJSTATE_INACTIVE);
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
		
		case MID_ALLOBJECTSCREATED:
		{
			// Since we must have a WorldProperties object in our levels this is a perfect 
			// place to do things that require every object to be created.

			g_pCommandDB->AllObjectsCreated();
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::ReadProps()
//
//	PURPOSE:	Read in properties
//
// --------------------------------------------------------------------------- //

void WorldProperties::ReadProps(const GenericPropList *pProps)
{
	char szBuf[512] = {0};

	float fFarZ = pProps->GetReal (s_szFarZ, 100000.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szFarZ, fFarZ );


	bool bFogEnable = pProps->GetBool( s_szFogEnable, false );
	g_pLTServer->SetConsoleVariableFloat( s_szFogEnable, (bFogEnable) ? 1.0f : 0.0f );


	float fFogNearZ = pProps->GetReal( s_szFogNearZ, 1.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szFogNearZ, fFogNearZ );


	float fFogFarZ = pProps->GetReal( s_szFogFarZ, 5000.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szFogFarZ, fFogFarZ );


	// Read the fog color into it's seperate components...
	LTVector vFogColor = pProps->GetColor( s_szFogColor, LTVector( 127.0f, 127.0f, 127.0f ));
	g_pLTServer->SetConsoleVariableFloat( s_szFogR, vFogColor.x / 255.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szFogG, vFogColor.y / 255.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szFogB, vFogColor.z / 255.0f );


	bool bSkyFogEnable = pProps->GetBool( s_szSkyFogEnable, false );
	g_pLTServer->SetConsoleVariableFloat( s_szSkyFogEnable, (bSkyFogEnable) ? 1.0f : 0.0f );


	float fSkyFogNearZ = pProps->GetReal( s_szSkyFogNearZ, 100.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szSkyFogNearZ, fSkyFogNearZ );


	float fSkyFogFarZ = pProps->GetReal( s_szSkyFogFarZ, 5000.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szSkyFogFarZ, fSkyFogFarZ );
	
	//determine if we should clamp the far z value
	if( pProps->GetBool( s_szClampFarZ, false ))
	{
		// Override FarZ if FogFarZ is closer
		if (bFogEnable && ((fFogFarZ * 2.0f) < fFarZ))
		{
			g_pLTServer->SetConsoleVariableFloat( s_szFarZ, fFogFarZ * 2.0f );
		}

		// Don't let FarZ get bigger than the dims of the level
		LTVector vWorldMin, vWorldMax;
		g_pLTServer->GetWorldBox(vWorldMin, vWorldMax);
		float fMaxFarZ = (vWorldMin - vWorldMax).Mag();
		if (fFarZ > fMaxFarZ)
		{
			DebugCPrint(1,"!!! FarZ for level is too large!  Please adjust the FarZ value in the WorldProperties object to less than %0.0f.", fMaxFarZ);
			g_pLTServer->SetConsoleVariableFloat( s_szFarZ, fMaxFarZ );
		}
	}

	const char *pszSoundFilter = pProps->GetString( s_szSoundFilter, "" );
	if( pszSoundFilter && pszSoundFilter[0] )
	{
		g_pLTServer->SetConsoleVariableString( s_szSoundFilter, pszSoundFilter );
	}

	const char *pszSoundMixer = pProps->GetString( s_szSoundMixer, "" );
	if( pszSoundMixer && pszSoundMixer[0] )
	{
		g_pLTServer->SetConsoleVariableString( s_szSoundMixer, pszSoundMixer );
	}

	m_sLevelEndCmd = pProps->GetCommand( "LevelEndCmd", "" );


	LTVector vAmbientLightColor = pProps->GetColor( "AmbientLight", LTVector( 0.0f, 0.0f, 0.0f ));
	g_pLTServer->SetConsoleVariableFloat( s_szAmbientLightR, vAmbientLightColor.x / 255.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szAmbientLightG, vAmbientLightColor.y / 255.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szAmbientLightB, vAmbientLightColor.z / 255.0f );
	float fAddAmbient = pProps->GetReal( "AddAmbientLightLow", 0.0f );
	g_pLTServer->SetConsoleVariableFloat( "AddAmbientLightLow", LTCLAMP( fAddAmbient, 0.0f, 1.0f ));
	fAddAmbient = pProps->GetReal( "AddAmbientLightMed", 0.0f );
	g_pLTServer->SetConsoleVariableFloat( "AddAmbientLightMed", LTCLAMP( fAddAmbient, 0.0f, 1.0f ));
	fAddAmbient = pProps->GetReal( "AddAmbientLightHigh", 0.0f );
	g_pLTServer->SetConsoleVariableFloat( "AddAmbientLightHigh", LTCLAMP( fAddAmbient, 0.0f, 1.0f ));

	LTVector vSkyAmbientLightColor = pProps->GetColor( "SkyAmbientLight", LTVector( 0.0f, 0.0f, 0.0f ));
	g_pLTServer->SetConsoleVariableFloat( s_szSkyAmbientLightR, vSkyAmbientLightColor.x / 255.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szSkyAmbientLightG, vSkyAmbientLightColor.y / 255.0f );
	g_pLTServer->SetConsoleVariableFloat( s_szSkyAmbientLightB, vSkyAmbientLightColor.z / 255.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleFogColorMsg
//
//	PURPOSE:	Handle a FOGCOLOR message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleFogColorMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 3 )
	{
		g_pLTServer->SetConsoleVariableString( s_szFogR, crParsedMsg.GetArg(1).c_str() );
		g_pLTServer->SetConsoleVariableString( s_szFogG, crParsedMsg.GetArg(2).c_str() );
		g_pLTServer->SetConsoleVariableString( s_szFogB, crParsedMsg.GetArg(3).c_str() );

		SendClientsChangedValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleFogEnableMsg
//
//	PURPOSE:	Handle a FOGENAGBLE message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleFogEnableMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		g_pLTServer->SetConsoleVariableString( s_szFogEnable, crParsedMsg.GetArg(1).c_str() );
		SendClientsChangedValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleFogNearZMsg
//
//	PURPOSE:	Handle a FOGNEARZ message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleFogNearZMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		g_pLTServer->SetConsoleVariableString( s_szFogNearZ, crParsedMsg.GetArg(1).c_str() );
		SendClientsChangedValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleFogFarZMsg
//
//	PURPOSE:	Handle a FOGFARZ message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleFogFarZMsg(	HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		g_pLTServer->SetConsoleVariableString( s_szFogFarZ, crParsedMsg.GetArg(1).c_str() );
		SendClientsChangedValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleSkyFogEnableMsg
//
//	PURPOSE:	Handle a SKYFOGENABLE message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleSkyFogEnableMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
 	if( crParsedMsg.GetArgCount() > 1 )
	{
		g_pLTServer->SetConsoleVariableString( s_szSkyFogEnable, crParsedMsg.GetArg(1).c_str() );
		SendClientsChangedValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleSkyFogNearZMsg
//
//	PURPOSE:	Handle a SKYFOGNEARZ message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleSkyFogNearZMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		g_pLTServer->SetConsoleVariableString( s_szSkyFogNearZ, crParsedMsg.GetArg(1).c_str() );
		SendClientsChangedValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleSkyFogFarZMsg
//
//	PURPOSE:	Handle a SKYFOGFARZ message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleSkyFogFarZMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		g_pLTServer->SetConsoleVariableString( s_szSkyFogFarZ, crParsedMsg.GetArg(1).c_str() );
		SendClientsChangedValues();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleTransmissionMsg
//
//	PURPOSE:	Handle a TRANSMISSION message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleTransmissionMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		uint32 nSound = 0;
		const char* pszStringID = crParsedMsg.GetArg(1).c_str();

		if( crParsedMsg.GetArgCount() > 2 )
		{
			nSound = (uint32) atol(crParsedMsg.GetArg(2));
		}

		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_EVENT);
		cClientMsg.Writeuint8(kPETransmission);
		cClientMsg.Writeuint32((uint32)-1);
		cClientMsg.Writeuint8( 1 );
		cClientMsg.Writeuint32(IndexFromStringID(pszStringID));
		cClientMsg.Writeuint32(nSound);
		g_pLTServer->SendToClient(cClientMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleNextRoundMsg
//
//	PURPOSE:	Handle a NEXTROUND message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleNextRoundMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( IsMultiplayerGameServer( ))
			g_pServerMissionMgr->NextRound( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleRoundWonMsg
//
//	PURPOSE:	Handle a ROUNDWON message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleRoundWonMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( IsMultiplayerGameServer( ))
	{
		if( crParsedMsg.GetArgCount() > 1 )
		{
			uint8 nTeamId = atoi( crParsedMsg.GetArg( 1 ));
			if( nTeamId < MAX_TEAMS )
			{
				CTeamMgr::Instance( ).WonRound( nTeamId );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleAddTeamScoreMsg
//
//	PURPOSE:	Handle a ADDTEAMSCORE message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleAddTeamScoreMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( !GameModeMgr::Instance( ).m_grbUseTeams)
		return;

	if( crParsedMsg.GetArgCount() <= 2 )
		return;

	uint8 nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	int nBonus = atoi( crParsedMsg.GetArg( 2 ));
	if( nTeamId < MAX_TEAMS )
	{
		CTeamMgr::Instance().AddToScore( nTeamId, nBonus );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleAttackingTeamMsg
//
//	PURPOSE:	Handle a ROUNDWON message...
//
// ----------------------------------------------------------------------- //
void WorldProperties::HandleAttackingTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( !GameModeMgr::Instance( ).m_grbUseTeams)
		return;

	if( crParsedMsg.GetArgCount() <= 1 )
		return;

	uint8 nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	if( nTeamId >= MAX_TEAMS )
		return;

	CTeamMgr::Instance( ).SetAttackingTeam( nTeamId, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::HandleCheckpointSaveMsg
//
//	PURPOSE:	Handle a CHECKPOINTSAVE message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleCheckpointSaveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	g_pServerSaveLoadMgr->SaveCheckpointSave( g_pServerMissionMgr->GetCurrentMission( ), g_pServerMissionMgr->GetCurrentLevel( ));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::CanSave
//
//	PURPOSE:	Handle a CANSAVE message...
//
// ----------------------------------------------------------------------- //

void WorldProperties::HandleCanSaveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	bool bCanSave = atoi( crParsedMsg.GetArg( 1 )) != 0;
	g_pServerSaveLoadMgr->SetCanSaveOverride( bCanSave );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::SendClientsChangedValues
//
//	PURPOSE:	Send the the notification to the clients that the values
//				have changed on the server and that they need to sync their
//				console variables with the server's
//
// ----------------------------------------------------------------------- //

void WorldProperties::SendClientsChangedValues()
{
	SendEmptyClientMsg(MID_CHANGE_WORLDPROPERTIES, NULL);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void WorldProperties::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	SAVE_DWORD(g_pVersionMgr->GetSaveVersion());

	// First save global game server shell stuff...(this only works because the
	// GameServerShell saves the WorldProperties object first)

	g_pGameServerShell->Save(pMsg, dwSaveFlags);

	SAVE_STDSTRING( m_sLevelEndCmd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void WorldProperties::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	uint32 nSaveVersion;
	LOAD_DWORD(nSaveVersion);
	g_pVersionMgr->SetCurrentSaveVersion( nSaveVersion );

	// First load global game server shell stuff...(this only works because the
	// GameServerShell saves the WorldProperties object first)
	g_pGameServerShell->Load(pMsg, dwLoadFlags);

	LOAD_STDSTRING( m_sLevelEndCmd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WorldProperties::SendLevelEndCmd
//
//	PURPOSE:	Sends the level end command.
//
// ----------------------------------------------------------------------- //

void WorldProperties::SendLevelEndCmd( )
{
	if( !m_sLevelEndCmd.empty( ))
	{
		g_pCmdMgr->QueueCommand( m_sLevelEndCmd.c_str( ), m_hObject, m_hObject );

		// Forget level end command, now that we've sent it.
		m_sLevelEndCmd.clear( );
	}
}
