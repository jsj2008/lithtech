// ----------------------------------------------------------------------- //
//
// MODULE  : Keyframer.cpp
//
// PURPOSE : Keyframer implementation
//
// CREATED : 10/7/97
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "KeyFramer.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "stdio.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "SoundMgr.h"
#include "ltbeziercurve.h"
#include "VarTrack.h"
#include "CommandMgr.h"
#include "ServerSoundMgr.h"

VarTrack	g_vtDisableKeyframers;

LINKFROM_MODULE( KeyFramer );

//the current version of the keyframer BOD
#define KEYDATA_VERSION				1

//invalid command index that keys can use
#define INVALID_COMMAND_INDEX		0xFFFFFFFF

//the header of the key data. Note that this must sync up with the header in WorldPacker
//convertkeydata.cpp
struct SKeyDataHeader
{
	//the current data version
	uint32	m_nVersion;

	//the number of keys in the file
	uint32	m_nNumKeys;

	//the size of the key data
	uint32	m_nKeyDataSize;
};

//this is the generic key information that all keys provide
struct KeyData
{
	//the time/distance along this path for this key to occur
	float				m_fPathStamp;

	//the index into the command buffer for the key's commands
	uint32				m_nCommandIndex;

	//the transform of this key
	LTRigidTransform	m_tTrans;
};

//this is a key for bezier interpolation that also has tangent data
struct BezierKeyData
{
	//this must come first to allow proper overlaying in memory with the key data
	KeyData		m_BaseKey;

	//the previous tangent position
	LTVector	m_vPrevTangent;

	//the next tangent position
	LTVector	m_vNextTangent;
};

#if defined(PLATFORM_XENON)

// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"

// Type descriptions for the endian swapper
DATATYPE_TO_ENDIANFORMAT(SKeyDataHeader, "3i");
DATATYPE_TO_ENDIANFORMAT(KeyData, "fi7f");
DATATYPE_TO_ENDIANFORMAT(BezierKeyData, "fi7f6f");

#endif // PLATFORM_XENON

//the textual names for each of our values in our potential list of wave types
static const char* c_aWaveTypes[eKFWT_NumWaveTypes] =
{
	"Linear",
	"Sine",
	"SlowOff",
	"SlowOn",
};

//the textual names for each of our values in our potential list of interpolation types
static const char* c_aInterpTypes[eKFIT_NumInterpolationTypes] =
{
	"Linear",
	"Bezier",
};

//utility function that will determine the stride for a specified interpolation type
static uint32 GetKeyStride(EKFInterpType eInterpType)
{
	switch(eInterpType)
	{
	case eKFIT_Linear:
		return sizeof(KeyData);
		break;
	case eKFIT_Bezier:
		return sizeof(BezierKeyData);
		break;
	default:
		LTERROR("Invalid interpolation type specified");
		return 0;
		break;
	}
}

//this utility function will take a percentage along the path in time, the type of wave form that
//should be used, and the total distance of the path, and will return the total distance along the path
//that the keyframer should be at that point of time
static float ConvertTimeToDistance(float fUnitTime, EKFWaveType eWaveType, float fTotalDist)
{
	//this is handled simply through integrating the area under the curve. For example, if the
	//function is a cosine over a 90 degree range (ramps from 0..1, so a smooth in), we integrate
	//from the range of [0..unit time * range] and divide that by the total area 
	switch (eWaveType)
	{
	case eKFWT_Linear:
		{
			//standard linear curve, so the percentage of distance is simply the same as the percentage
			//of time
			return fUnitTime * fTotalDist;
		}
		break;
	case eKFWT_Sine:
		{
			//a sine wave over 180 degrees. The integral of sin is
			//just -cos * du, so we have -cos(t) - -cos(0) = 1 - cos(t)
			float fCoveredArea	= 1.0f - cosf(fUnitTime * MATH_PI);
			//for the total area, t = pi, so the total is 2
			float fInvTotalArea = 0.5f;

			return fCoveredArea * fTotalDist * fInvTotalArea;
		}
		break;
	case eKFWT_SlowOff:
		{
			//a sine wave over 90 degrees. The integral of cos is
			//just sin * du, so we have sin(t) - sin(0) = sin(t)
			float fCoveredArea	= sinf(fUnitTime * MATH_HALFPI);
			//for the total area, t = pi/2, so the total is 1, so no need to apply a scale
			return fCoveredArea * fTotalDist;
		}
		break;
	case eKFWT_SlowOn:
		{
			//a sine wave over 90 degrees. The integral of sin is
			//just -cos * du, so we have -cos(t) - -cos(0) = 1 - cos(t)
			float fCoveredArea	= 1.0f - cosf(fUnitTime * MATH_HALFPI);
			//for the total area, t = pi/2, so the total is 1, so no need to apply a scale
			return fCoveredArea * fTotalDist;
		}
		break;
	default:
		{
			//unexpected type
			LTERROR("Invalid keyframe interpolation type detected");
			return 0.0f;
		}
		break;
	}
}

//this function performs the inverse of the above function, and takes a unit distance and the total time
//each equation is simply the inverse of the above equation, so for more information, please refer
//to the above function
static float ConvertDistanceToTime(float fUnitDist, EKFWaveType eWaveType, float fTotalTime)
{
	switch (eWaveType)
	{
	case eKFWT_Linear:
		{
			return fUnitDist * fTotalTime;
		}
		break;
	case eKFWT_Sine:
		{
			return (acosf(1.0f - 2.0f * fUnitDist) / MATH_PI) * fTotalTime;
		}
		break;
	case eKFWT_SlowOff:
		{
			return (asinf(fUnitDist) / MATH_HALFPI) * fTotalTime;
		}
		break;
	case eKFWT_SlowOn:
		{
			return (acosf(1.0f - fUnitDist) / MATH_HALFPI) * fTotalTime;
		}
		break;
	default:
		{
			//unexpected type
			LTERROR("Invalid keyframe interpolation type detected");
			return 0.0f;
		}
		break;
	}
}


BEGIN_CLASS(KeyFramer)
	ADD_STRINGPROP_FLAG(ObjectName, "", 0, "This feild contains the name of the object or worldmodel that is to be affected by the keyframer. This feild can contain the names of several objects and/or worldmodels seperated by semicolons.")
	ADD_STRINGPROP_FLAG(Interpolation, "Linear", PF_STATICLIST, "This indicates how movement between keys will be handled. Linear is much cheaper but doesn't look as smooth for a small number of keys. Bezier is more expensive but smoother and requires tangents.")
	ADD_STRINGPROP_FLAG(BaseKeyName, "", 0, "The base name of the keys that the KeyFramer object will follow. The first key must be named with the base name followed by 0. All subsequent keys must be named with the base name followed by two-digit increments. For example, if the BaseKeyName was 'Base', the keys would be named Base0, Base01, Base02, Base03, etc.")
	ADD_STRINGPROP_FLAG(ExternalKeyFile, "", 0, "If your animation keys are in a separate ." RESEXT_KEY_COMPRESSED " file, exported from max, enter the file path of your animations here.  The KeyFramer will then look at the file for the keys.")
	ADD_REALPROP_FLAG(KeyDataIndex, -1.0f, PF_HIDDEN, "Internal index into the blind object data for the key data")
	ADD_BOOLPROP_FLAG(PushObjects, false, 0, "Determines whether or not the KeyFramer will push objects in the world.")
	ADD_BOOLPROP_FLAG(StartActive, false, 0, "If set to true, the KeyFramer will begin active.")
	ADD_BOOLPROP_FLAG(StartPaused, false, 0, "This flag works in conjunction with the StartActive flag. This flag will tell the KeyFramer to begin in the paused state if active.")
	ADD_BOOLPROP_FLAG(Looping, false, 0, "If set to true the keyframer will continuously run through the key path, returning to the first key in the sequence when reaching the last.")
	ADD_BOOLPROP_FLAG(MaintainOffsets, true, 0, "Tells the KeyFramer whether or not to maintain offsets of position and rotation between the object being keyframed and the keys that make up the path. If set to false, the keyframed object will acquire the exact position and rotation of the keys that make up the key path.")
	ADD_STRINGPROP_FLAG(TargetName, "", 0, "You can specify a prop, worldmodel, or model in this field. The KeyFramer will attempt to continually point it's forward facing vector at this object as it travels along its key path.")
	ADD_VECTORPROP_VAL_FLAG(TargetOffset, 0.0f, 0.0f, 0.0f, 0, "You may enter a target offset in this field. It is entered in the form of x,y,z offset in WorldEdit units.")
	ADD_STRINGPROP_FLAG(ActiveSound, "", PF_STATICLIST, "The name of a sound record that will be played while the KeyFramer object is active.")
	PROP_DEFINEGROUP(Waveform, PF_GROUP(1), "The WaveForm flag is a subset of properties that define the movement characteristics of the KeyFramer object.")
	ADD_REALPROP_FLAG(TotalPathTime, 0.0f, PF_GROUP(1), "This is the approximate time in seconds that it will take for the KeyFramer object to complete it's travel down the key path. Note that by specifying this, it will ignore the time stamps of the keys and instead use the distance between the keys as the primary factor")
	ADD_STRINGPROP_FLAG(Wavetype, "Linear", PF_STATICLIST | PF_GROUP(1), "This is a dropdown list of different Waves that the KeyFramer can use while following it's path.  The KeyFramer will vary it's velocity depending on the wave type chosen. Note that this is only used if the TotalPathTime is non-zero, otherwise it is ignored")
	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH(KeyFramer, GameBase, 0, KeyframerPlugin, DefaultPrefetch<KeyFramer>, "The keyframer is a class that will move the specified objects along a path and can trigger sounds and commands as it moves. The path is specified using either key objects or a path stored in an external key file. The movement rate can either be specified using key times, or can use the distance between keys if TotalPathTime is set to non-zero, in which case it will traverse the full path in that specified amount of time.")


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateGotoMsg
//
//  PURPOSE:	Make sure GOTO message works
//
// ----------------------------------------------------------------------- //

static bool ValidateGotoMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	// Check the External File property to see if the keys exist somewhere else...

	char *pObj = CCommandMgrPlugin::GetCurrentObjectName();

	GenericProp gProp;
	if( pInterface->GetPropGeneric( pObj, "ExternalKeyFile", &gProp ) == LT_OK )
	{
		if( !LTStrEmpty( gProp.GetString() ) )
		{
			return true;
		}
	}

	if( !CCommandMgrPlugin::DoesObjectExist( pInterface, cpMsgParams.m_Args[1] ))
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Could not find object '%s'!", cpMsgParams.m_Args[1] );
		return false;
	}
	
	if( !LTStrIEquals( CCommandMgrPlugin::GetObjectClass( pInterface, cpMsgParams.m_Args[1] ), "Key" ))
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Object '%s' is not of type 'Key'!", cpMsgParams.m_Args[1] );
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateMoveToMsg
//
//  PURPOSE:	Make sure MOVETO message is valid and the command it might send
//
// ----------------------------------------------------------------------- //

static bool ValidateMoveToMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( cpMsgParams.m_nArgs < 2 || cpMsgParams.m_nArgs > 3 )
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Invalid number of arguments!", cpMsgParams.m_Args[1] );
		return false;
	}

	// Check the External File property to see if the keys exist somewhere else...

	char *pObj = CCommandMgrPlugin::GetCurrentObjectName();

	bool bExternalKeyFile = false;
	GenericProp gProp;

	if( pInterface->GetPropGeneric( pObj, "ExternalKeyFile", &gProp ) == LT_OK )
	{
		if( gProp.GetString()[0] )
		{
			bExternalKeyFile = true;
		}
	}

	if( !bExternalKeyFile && !CCommandMgrPlugin::DoesObjectExist( pInterface, cpMsgParams.m_Args[1] ))
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Could not find object '%s'!", cpMsgParams.m_Args[1] );
		return false;
	}

	if( bExternalKeyFile || (LTStrIEquals(CCommandMgrPlugin::GetObjectClass( pInterface, cpMsgParams.m_Args[1] ), "Key" )) )
	{
		if( cpMsgParams.m_nArgs == 3 && cpMsgParams.m_Args[2] )
		{
			CCommandMgrPlugin cmdmgr;

			if( cmdmgr.IsValidCmd( pInterface, cpMsgParams.m_Args[2] ))
			{
				return true;
			}
			
			WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Command '%s' is not valid!", cpMsgParams.m_Args[2] );
			return false;
		}

		return true;
	}

	WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Object '%s' is not of type 'Key'!", cpMsgParams.m_Args[1] );
	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateTargetMsg
//
//  PURPOSE:	Make sure TARGET message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateTargetMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( !CCommandMgrPlugin::DoesObjectExist( pInterface, cpMsgParams.m_Args[1] ))
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Could not find object '%s'!", cpMsgParams.m_Args[1] );
		return false;
	}

	return true;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( KeyFramer )

//				Message			Num Params	Validation FnPtr	Message Handler										Syntax

	ADD_MESSAGE( ON,			1,			NULL,				MSG_HANDLER( KeyFramer, HandleOnMsg ),				"ON",							"Tells the KeyFramer to start moving its listed objects along the path defined by its specified keys starting on the 0 key", "msg KeyFramer ON" )
	ADD_MESSAGE( OFF,			1,			NULL,				MSG_HANDLER( KeyFramer, HandleOffMsg ),				"OFF",							"Tells the KeyFramer to stop moving its listed objects along the path defined by its specified keys. The objects stop wherever they are on the path", "msg KeyFramer OFF" )
	ADD_MESSAGE( PAUSE,			1,			NULL,				MSG_HANDLER( KeyFramer, HandlePauseMsg ),			"PAUSE",						"Tells the KeyFramer to pause the movement of its listed objects along the path defined by its specified keys",	"msg KeyFramer PAUSE" )
	ADD_MESSAGE( RESUME,		1,			NULL,				MSG_HANDLER( KeyFramer, HandleResumeMsg ),			"RESUME",						"Tells the KeyFramer to pause the movement of its listed objects along the path defined by its specified keys", "msg KeyFramer RESUME" )
	ADD_MESSAGE( FORWARD,		1,			NULL,				MSG_HANDLER( KeyFramer, HandleForwardMsg ),			"FORWARD",						"Tells the KeyFramer to move its listed objects along the path defined by its specified keys in a forward direction", "msg KeyFramer FORWARD" )
	ADD_MESSAGE( REVERSE,		1,			NULL,				MSG_HANDLER( KeyFramer, HandleReverseMsg ),			"REVERSE",						"Tells the KeyFramer to move its listed objects along the path defined by its specified keys in a backward direction", "msg KeyFramer REVERSE" )
	ADD_MESSAGE( TOGGLEDIR,		1,			NULL,				MSG_HANDLER( KeyFramer, HandleToggleDirMsg ),		"TOGGLEDIR",					"Tells the KeyFramer to move its listed objects along the path defined by its specified keys in the opposite direction it is currently traveling in", "msg KeyFramer TOGGLEDIR" )
	ADD_MESSAGE( GOTO,			2,			ValidateGotoMsg,	MSG_HANDLER( KeyFramer, HandleGoToMsg ),			"GOTO <key>",					"Tells the KeyFramer to immediately move its listed objects to a specific Key along the path defined by its specified keys", "To tell a set of objects Keyframed by a KeyFramer named \"KeyFramer\" to goto a specific Key named Key03 the command would look like:<BR><BR>msg KeyFramer (GOTO Key03)" )
	ADD_MESSAGE( MOVETO,		-1,			ValidateMoveToMsg,	MSG_HANDLER( KeyFramer, HandleMoveToMsg ),			"MOVETO <key> [destination cmd]","Tells the KeyFramer to move its listed objects to a specific Key along the path defined by its specified keys by traveling the path to that specific Key.  It will also send the command listed in the optional �destion cmd� paramater", "To tell a set of objects Keyframed by a KeyFramer named \"KeyFramer\" to move to a specific Key named Key03 the command would look like:<BR><BR>msg KeyFramer (MOVETO Key03)" )
	ADD_MESSAGE( TARGET,		2,			ValidateTargetMsg,	MSG_HANDLER( KeyFramer, HandleTargetMsg ),			"TARGET <object>",				"Tells the KeyFramer to rotate the Z vector of its listed object toward a specific target object", "To tell a set of objects Keyframed by a KeyFramer named \"KeyFramer\" to face the player the command would look like:<BR><BR>msg KeyFramer (TARGET player)" )
	ADD_MESSAGE( CLEARTARGET,	1,			NULL,				MSG_HANDLER( KeyFramer, HandleClearTargetMsg ),		"CLEARTARGET",					"Tells the KeyFramer to stop aligning the Z vector of its specified objects to face a specified target", "msg KeyFramer CLEARTARGET" )
	ADD_MESSAGE( TARGETOFFSET,	4,			NULL,				MSG_HANDLER( KeyFramer, HandleTargetOffsetMsg ),	"TARGETOFFSET <x> <y> <z>",		"If  a target name is specified, KeyFramed objects will always face the targeted object�s position plus the offset specified in the target offset parameter", "To tell a set of objects Keyframed by a KeyFramer named \"KeyFramer\" that is targeting the player to maintain an offset 5, 5, and 10 the command would look like:<BR><BR>msg KeyFramer (TARGETOFFSET 5 5 10)" )


CMDMGR_END_REGISTER_CLASS( KeyFramer, GameBase )


// ----------------------------------------------------------------------- //
//
// External WorldEdit behavior hooks
//
// ----------------------------------------------------------------------- //

LTRESULT KeyframerPlugin::PreHook_EditStringList( const char * szRezPath, 
												  const char *szPropName,
												  char **aszStrings,
												  uint32 *pcStrings,
												  const uint32 cMaxStrings,
												  const uint32 cMaxStringLength )
{
	LTASSERT(szPropName && aszStrings && pcStrings, "TODO: Add description here");

	// See if we can handle the list...

	if( LTStrIEquals( szPropName, "Wavetype" ) )
	{
		// Fill in the list with our wave types...
		for( uint32 i = 0; i < eKFWT_NumWaveTypes; i++ )
		{
			LTStrCpy( aszStrings[(*pcStrings)++], c_aWaveTypes[i], cMaxStringLength );
		}

		return LT_OK;
	}
	else if( LTStrIEquals( szPropName, "Interpolation" ) )
	{
		// Fill in the list with our interpolation types...
		for( uint32 i = 0; i < eKFIT_NumInterpolationTypes; i++ )
		{
			LTStrCpy( aszStrings[(*pcStrings)++], c_aInterpTypes[i], cMaxStringLength );
		}

		return LT_OK;
	}
	else if( LTStrIEquals( szPropName, "ActiveSound" ))
	{
		if( CSoundDBPlugin::Instance().PreHook_EditStringList( szRezPath,
			szPropName,
			aszStrings,
			pcStrings,
			cMaxStrings,
			cMaxStringLength ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::KeyFramer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

KeyFramer::KeyFramer()
:	GameBase				( OT_NORMAL ),
	m_sObjectName			( ),
	m_sTargetName			( ),
	m_sBaseKeyName			( ),
	m_sActiveSndRecord		( ),
	m_sDestCmd				( ),
	m_hTargetObject			( NULL ),
	m_hActiveSnd			( NULL ),
	m_hActiveSndObj			( NULL ),
	m_bMaintainOffsets		( true ),
	m_bStartActive			( false ),
	m_bStartPaused			( false ),
	m_bLooping				( false ),
	m_bActive				( false ),
	m_bPaused				( false ),
	m_bUseDistance			( false ),
	m_bPushObjects			( true ),
	m_pKeys					( NULL ),
	m_pNextKey				( NULL ),
	m_pPrevKey				( NULL ),
	m_pLastKey				( NULL ),
	m_pDestinationKey		( NULL ),
	m_nKeyDataIndex			( 0xffffffff ),
	m_nNumKeys				( 0 ),
	m_fCurTime				( 0.0f ),
	m_fEndTime				( 0.0f ),
	m_fDistancePathTime		( 0.0f ),
	m_fTotalDistance		( 0.0f ),
	m_eDirection			( KFD_FORWARD ),
	m_eWaveform				( eKFWT_Linear ),
	m_eInterpType			( eKFIT_Linear ),
	m_nKeyStride			( 0 ),
	m_vTargetOffset			( 0.0f, 0.0f, 0.0f),
	m_fKeyPercent			( 0.0f ),
	m_bInitializedObjRefs	( false )
{
	//setup our console variables
	if (!g_vtDisableKeyframers.IsInitted())
	{
		g_vtDisableKeyframers.Init(g_pLTServer, "DisableKeyframers", NULL, 0.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::~KeyFramer()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

KeyFramer::~KeyFramer()
{
	//stop the sound that is associated with us
	StopActiveSound();

	//free all the objects and their offsets
	m_TrackedObjects.clear();

	//free the keys we are associated with
	FreeKeys();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoActive
//
//	PURPOSE:	Start the KeyFramer going!
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoActive(bool bReset)
{
	if (m_bActive || !m_pKeys || !m_pLastKey || m_TrackedObjects.empty()) 
		return;

	// Go active
	m_bActive = true;

	if (bReset)
	{
		m_fCurTime		= (m_eDirection == KFD_FORWARD) ? 0.0f : GetPathEndTime();
		m_pNextKey		= (m_eDirection == KFD_FORWARD) ? m_pKeys : m_pLastKey;
		m_pPrevKey		= m_pNextKey;
		m_fKeyPercent	= 0.0f;
	}

    SetNextUpdate(UPDATE_NEXT_FRAME);


	// Start active sound...
	if( !m_sActiveSndRecord.empty() && g_pServerSoundMgr)
	{
		StopActiveSound();

		//just use the first keyframed object as the object that the sound is linked to
		if (m_TrackedObjects[0].m_hObject)
		{
			HRECORD hSoundRecord = CSoundDB::Instance().GetSoundDBRecord(m_sActiveSndRecord.c_str());
			if(hSoundRecord)
			{
				m_hActiveSndObj = m_TrackedObjects[0].m_hObject;
				m_hActiveSnd = g_pServerSoundMgr->PlayDBSoundFromObject(m_hActiveSndObj, hSoundRecord);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoInActive
//
//	PURPOSE:	Stop the KeyFramer...
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoInActive()
{
	m_bActive = false;
    SetNextUpdate(UPDATE_NEVER);
	StopActiveSound();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CreateKeyList
//
//	PURPOSE:	Create our list of keys.
//
// ----------------------------------------------------------------------- //

bool KeyFramer::CreateKeyList()
{
	//clear out any existing data
	FreeKeys();

	// make sure we have valid blind data
	if( m_nKeyDataIndex == 0xffffffff )
		return false;

	// get the blind data for this keyframer
	uint8* blindData = NULL;
	uint32 blindDataSize = 0;
	if( g_pLTServer->GetBlindObjectData( m_nKeyDataIndex, KEYFRAMER_BLINDOBJECTID, blindData, blindDataSize ) != LT_OK )
		return false;

	//get the header from this blind object data
	SKeyDataHeader KeyHeader = *((SKeyDataHeader*)blindData);
	uint8* pKeyData = blindData + sizeof(SKeyDataHeader);

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&KeyHeader);
#endif // PLATFORM_XENON

	//check the version and interpolation or there are no keys
	if(	(KeyHeader.m_nVersion != KEYDATA_VERSION) ||
		(KeyHeader.m_nNumKeys == 0))
	{
		FreeKeys();
		return false;
	}

	//extract information out of our buffer
	m_pCommandBuffer = (const char*)(blindData + KeyHeader.m_nKeyDataSize);

	m_pKeys		= (const KeyData*)pKeyData;
	m_nNumKeys	= KeyHeader.m_nNumKeys;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(const_cast<KeyData*>(m_pKeys), KeyHeader.m_nNumKeys);
#endif // PLATFORM_XENON

	m_pLastKey = GetKeyFromIndex(m_nNumKeys - 1);
	m_fEndTime = m_pLastKey->m_fPathStamp;

	// See if we need to calculate the velocity info...
	if (m_bUseDistance)
	{
		// Calculate the total distance between all the keys...
		m_fTotalDistance = 0.0f;

		//run through the keys and calculate the total distance and store the distance in the time stamps
		for(uint32 nCurrKey = 0; nCurrKey < m_nNumKeys - 1; nCurrKey++)
		{
			//determine the current and next key that we will be finding the distance between
			KeyData& CurrKey = *(KeyData*)(pKeyData + m_nKeyStride * nCurrKey);
			KeyData& NextKey = *(KeyData*)(pKeyData + m_nKeyStride * (nCurrKey + 1));

			//assign the total distance thus far to the key
			CurrKey.m_fPathStamp = m_fTotalDistance;

			// Two keys occupying the same space is unsupported...
			float fKeyDistance = CalcKeyDistance(&CurrKey, &NextKey);
			LTASSERT( fKeyDistance > 0.0f, "Two keys occupy the same space.  This will cause unknown behavior with the keyframer!" );
			
			m_fTotalDistance += fKeyDistance;
		}

		//set the last key to just use the total distance since that isn't included in the above loop
		KeyData& LastKey = *(KeyData*)(pKeyData + m_nKeyStride * (m_nNumKeys - 1));
		LastKey.m_fPathStamp = m_fTotalDistance;
	}

	return true;
}

//called to free all of our keys and all the associated data
void KeyFramer::FreeKeys()
{
	//the keys just point into the blind object data, so we don't need to actually free them
	
	//clear out our associated information
	m_pCommandBuffer	= NULL;
	m_pKeys				= NULL;
	m_pLastKey			= NULL;
	m_fTotalDistance	= 0.0f;
	m_fEndTime			= 0.0f;
	m_nNumKeys			= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::CreateObjectList
//
//	PURPOSE:	Create our list of objects to control.
//
// ----------------------------------------------------------------------- //

bool KeyFramer::CreateObjectList()
{
	// We need the keys and the object name.
	if( !m_pKeys || m_sObjectName.empty())
		return false;

	// Delete any old object list.
	m_TrackedObjects.clear();

	// Find all the objects with the given names (m_sObjectName may be
	// of the form: name1;name2;name3...
	ConParse parse;
	parse.Init(m_sObjectName.c_str( ));

	while (g_pCommonLT->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0)
		{
			//find all objects with the provided name
			ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
			g_pLTServer->FindNamedObjects(parse.m_Args[0], objArray);

			//and now add each of those objects to our list
			uint32 numObjects = objArray.NumObjects();
			for (uint32 nCurrObject = 0; nCurrObject < numObjects; nCurrObject++)
			{
				//get the object we will be adding to the list
				HOBJECT hObject = objArray.GetObject(nCurrObject);

				//determine the transform of this object
				LTRigidTransform tObjTrans;
				g_pLTServer->GetObjectTransform(hObject, &tObjTrans);

				//setup our new tracked object
				STrackedObject NewObj;
				NewObj.m_hObject		= hObject;
				NewObj.m_tRelativeTrans = m_pKeys->m_tTrans.GetDifference(tObjTrans);

				//and create a link to this object so that we can be notified when it is removed
                g_pLTServer->CreateInterObjectLink(m_hObject, hObject);

				//and add this object to our list
				m_TrackedObjects.push_back(NewObj);
			}
		}
	}

	//return failure if we didn't find any objects at all
	if (m_TrackedObjects.empty())
	{
		return false;
	}

	return true;
}

//given a key that has been passed, this will handle triggering any messages and will
//detect if this is the destination key, and return whether or not it should continue moving
//beyond this key
bool KeyFramer::HandleKeyEvents(const KeyData* pKey)
{
	//see if this key has a command or not
	if(pKey->m_nCommandIndex != INVALID_COMMAND_INDEX)
	{
		//this points to the command string with the sound string following immediately after
		const char* pszCommand = m_pCommandBuffer + pKey->m_nCommandIndex;
		if(!LTStrEmpty(pszCommand))
		{
			g_pCmdMgr->QueueCommand( pszCommand, m_hObject, m_hObject );
		}

		//move past the command to the sound
		const char* pszSound = pszCommand + LTStrLen(pszCommand) + 1;
		if(!LTStrEmpty(pszSound))
		{
			HRECORD hSound = CSoundDB::Instance().GetSoundDBRecord(pszSound);
			if(hSound)
			{
				g_pServerSoundMgr->PlayDBSoundFromPos(pKey->m_tTrans.m_vPos, hSound);
			}
		}
	}
	
	// If we're moving to a destination, see if we have reached it yet...
	if (pKey == m_pDestinationKey)
	{
		ReachedDestination();

		//we shouldn't keep traversing since we hit our destination
		return false;
	}
	
	//we did not hit the destination so we want to keep traversing
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ReachedDestination
//
//	PURPOSE:	Handle reaching the destination key...
//
// ----------------------------------------------------------------------- //

void KeyFramer::ReachedDestination()
{
	// Default behavior...Pause the keyframer...
	Pause();

	// If we have a destination command, process it...
	if( !m_sDestCmd.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sDestCmd.c_str(), m_hObject, m_hObject );

		// The destination command is no longer needed....
		m_sDestCmd.clear();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 KeyFramer::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProps(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			AllObjectsCreated();
		}
		break;

		case MID_LINKBROKEN :
		{
			HandleLinkBroken((HOBJECT)pData);
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

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleLinkBroken()
//
//	PURPOSE:	Handle MID_LINKBROKEN engine message
//
// --------------------------------------------------------------------------- //

void KeyFramer::HandleLinkBroken(HOBJECT hLink)
{
	if (!hLink) 
		return;

	// Kill active sound if object associated with it goes away...
	if (m_hActiveSndObj == hLink)
	{
		StopActiveSound();
		m_hActiveSndObj = NULL;
	}

	//run through our list of objects and find the matching object and remove it from our list
	for(TTrackedObjectList::iterator itObj = m_TrackedObjects.begin(); itObj != m_TrackedObjects.end(); itObj++)
	{
		if(itObj->m_hObject == hLink)
		{
			//we found our match, so remove this object
			m_TrackedObjects.erase(itObj);
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleOnMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	On();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleOffMsg
//
//	PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleOffMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	Off();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandlePauseMsg
//
//	PURPOSE:	Handle a PAUSE message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandlePauseMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	Pause();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleResumeMsg
//
//	PURPOSE:	Handle a RESUME message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleResumeMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	Resume();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleForwardMsg
//
//	PURPOSE:	Handle a FORWARD message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleForwardMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	Forward();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleReverseMsg
//
//	PURPOSE:	Handle a REVERSE message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleReverseMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	Reverse();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleToggleDirMsg
//
//	PURPOSE:	Handle a TOGGLEDIR message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleToggleDirMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	ToggleDir();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleGoToMsg
//
//	PURPOSE:	Handle a GOTO message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleGoToMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		GoToKey( crParsedMsg.GetArg(1) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleMoveToMsg
//
//	PURPOSE:	Handle a MOVETO message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleMoveToMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	MoveToKey( crParsedMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleTargetMsg
//
//	PURPOSE:	Handle a TARGET message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleTargetMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		SetTarget( crParsedMsg.GetArg(1) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleClearTargetMsg
//
//	PURPOSE:	Handle a CLEARTARGET message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleClearTargetMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	SetTarget( NULL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::HandleTargetOffsetMsg
//
//	PURPOSE:	Handle a TARGETOFFSET message...
//
// ----------------------------------------------------------------------- //

void KeyFramer::HandleTargetOffsetMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 3 )
	{
		m_vTargetOffset.x = (float) atof( crParsedMsg.GetArg(1) );
		m_vTargetOffset.y = (float) atof( crParsedMsg.GetArg(2) );
		m_vTargetOffset.z = (float) atof( crParsedMsg.GetArg(3) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ReadProps
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool KeyFramer::ReadProps(const GenericPropList *pProps)
{
	m_sObjectName		= pProps->GetString( "ObjectName", "" );

	m_sTargetName		= pProps->GetString( "TargetName", "" );
	m_vTargetOffset		= pProps->GetVector( "TargetOffset", m_vTargetOffset );
	m_sBaseKeyName		= pProps->GetString( "BaseKeyName", "" );

	float fKeyDataIndex	= pProps->GetReal( "KeyDataIndex", -1.0f );
	m_nKeyDataIndex		= (fKeyDataIndex >= 0.0f ? (uint32)fKeyDataIndex : 0xffffffff);

	m_sActiveSndRecord	= pProps->GetString( "ActiveSound", "" );
	m_bStartActive		= pProps->GetBool( "StartActive", m_bStartActive );
	m_bStartPaused		= pProps->GetBool( "StartPaused", m_bStartPaused );
	m_bLooping			= pProps->GetBool( "Looping", m_bLooping );
	m_bMaintainOffsets	= pProps->GetBool( "MaintainOffsets", m_bMaintainOffsets );
	m_bPushObjects		= pProps->GetBool( "PushObjects", m_bPushObjects );

	m_fDistancePathTime	= pProps->GetReal( "TotalPathTime", m_fDistancePathTime );
	m_bUseDistance		= (m_fDistancePathTime > 0.0 ? true : false);

	// Get the waveform...
	const char *pszWaveType = pProps->GetString( "Wavetype", "Linear" );
	if( pszWaveType )
	{
		for( uint32 i = 0; i < eKFWT_NumWaveTypes; i++ )
		{
			if( LTStrIEquals( pszWaveType, c_aWaveTypes[i] ))
			{
				m_eWaveform = static_cast<EKFWaveType>(i);
				break;
			}
		}
	}

	// Get the interpolation type
	const char *pszInterpType = pProps->GetString( "Interpolation", "Linear" );
	if( pszInterpType )
	{
		for( uint32 i = 0; i < eKFIT_NumInterpolationTypes; i++ )
		{
			if( LTStrIEquals( pszInterpType, c_aInterpTypes[i] ))
			{
				m_eInterpType = static_cast<EKFInterpType>(i);
				break;
			}
		}
	}

	//calculate the stride of our key
	m_nKeyStride = GetKeyStride(m_eInterpType);

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::SetTarget
//
//	PURPOSE:	Set the target object...
//
// ----------------------------------------------------------------------- //

void KeyFramer::SetTarget(const char* pName)
{
	//clear out our previous name
	m_hTargetObject = NULL;

	//find the object that is named
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(const_cast<char*>(pName), objArray);

	//and set it as our target object if we have a match
	if (objArray.NumObjects() > 0)
	{
		m_hTargetObject = objArray.GetObject(0);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void KeyFramer::InitialUpdate()
{
	char szName[256] = {0};
	g_pLTServer->GetObjectName( m_hObject, szName, LTARRAYSIZE(szName));

	//setup our actual key list
	CreateKeyList();
}

//given two keys and a time value that lies between them, this will compute the percentage that
//the time value is between those keys
float KeyFramer::GetKeyPercentage(const KeyData* pKey1, const KeyData* pKey2, float fTimeVal, float fTotalTime) const
{
	//now compute the percentage between keys
	if(pKey1 == pKey2)
	{
		//both keys are the same, so it doesn't matter what percentage we use, but use one
		//that will avoid interpolation (0 or 1)
		return 1.0f;
	}
	else
	{
		float fTimeVal = m_fCurTime;

		//if the keys are in distance space, make sure that our time value is as well
		if(m_bUseDistance)
			fTimeVal = ConvertTimeToDistance(m_fCurTime / fTotalTime, m_eWaveform, m_fTotalDistance);

		//find the range between the keys
		float fKeyDist = pKey2->m_fPathStamp - pKey1->m_fPathStamp;
		if ( 0.0f == fKeyDist )
		{
			return 1.0f;
		}
		else
		{
			//and compute the percentage between the keys
			return (fTimeVal - pKey1->m_fPathStamp) / fKeyDist;
		}
	}
}

//handles moving the keyframer forward given a specified interval of time
void KeyFramer::MoveKeyframerForward(float fFrameTime, float fTotalTime)
{
	//start out the amount of time we need to handle as our frame time
	float fTimeDelta = fFrameTime;

	//handle the case where we aren't starting on a key frame, either by just using up the update
	//between the key frame, or moving to the first key frame
	float fCurKeyTime = GetKeyTime(m_pNextKey);
	if(fCurKeyTime > m_fCurTime)
	{
		//just move forward to that key frame if there is room
		float fMoveAmount	= LTMIN(fTimeDelta, fCurKeyTime - m_fCurTime);

		//move ourselves forward, and remove this time from the update delta
		m_fCurTime		+= fMoveAmount;
		fTimeDelta		-= fMoveAmount;
	}

	//once we have hit this part, we know we are sitting on a key frame and we want to handle moving
	//to the next key frame until we run out of updating time
	while((fTimeDelta > 0.0f) && m_bActive)
	{
		if(!HandleKeyEvents(m_pNextKey))
			break;

		//now we need to move past this key string

		//handle the case where we need to wrap around to the beginning of the animation
		if(m_pNextKey == m_pLastKey)
		{
			//handle the two cases of where we might be looping, and where we aren't
			if(m_bLooping)
			{
				//update our time to reflect the new position
				m_fCurTime = 0.0f;

				//and update our frame references to match
				m_pNextKey		= m_pKeys;
				m_pPrevKey		= m_pKeys;
				m_fKeyPercent	= 0.0f;
			}
			else
			{
				//signify that we are stopped by moving this animation past the last key frame time,
				//and stopping our updating
				m_fCurTime		= fTotalTime;

				//and put ourselves to sleep
				GoInActive();
				break;
			}
		}
		else
		{
			//move to the next key frame
			m_pPrevKey = m_pNextKey;
			m_pNextKey = GetNextKey(m_pNextKey);

			//we can move forward, so move to the next key frame if there is enough time
			float fMoveAmount = LTMIN(fTimeDelta, GetKeyTime(m_pNextKey) - m_fCurTime);

			//update our time to reflect the new position
			fTimeDelta	-= fMoveAmount;
			m_fCurTime	+= fMoveAmount;
		}		
	}

	//determine the percentage between the keys
	m_fKeyPercent = GetKeyPercentage(m_pPrevKey, m_pNextKey, m_fCurTime, fTotalTime);	
}

//handles moving the keyframer backward given a specified interval of time
void KeyFramer::MoveKeyframerBackward(float fFrameTime, float fTotalTime)
{
	//start out the amount of time we need to handle as our frame time
	float fTimeDelta = fFrameTime;

	//handle the case where we aren't starting on a key frame, either by just using up the update
	//between the key frame, or moving to the first key frame
	float fCurKeyTime = GetKeyTime(m_pPrevKey);
	if(fCurKeyTime < m_fCurTime)
	{
		//just move forward to that key frame if there is room
		float fMoveAmount	= LTMIN(fTimeDelta, m_fCurTime - fCurKeyTime);

		//move ourselves forward, and remove this time from the update delta
		m_fCurTime		-= fMoveAmount;
		fTimeDelta		-= fMoveAmount;
	}

	//once we have hit this part, we know we are sitting on a key frame and we want to handle moving
	//to the next key frame until we run out of updating time
	while((fTimeDelta > 0.0f) && m_bActive)
	{
		if(!HandleKeyEvents(m_pPrevKey))
			break;

		//now we need to move past this key string

		//handle the case where we need to wrap around to the beginning of the animation
		if(m_pPrevKey == m_pKeys)
		{
			//handle the two cases of where we might be looping, and where we aren't
			if(m_bLooping)
			{
				//update our time to reflect the new position
				m_fCurTime		= fTotalTime;

				//and update our frame references to match
				m_pNextKey		= m_pLastKey;
				m_pPrevKey		= m_pLastKey;
				m_fKeyPercent	= 0.0f;
			}
			else
			{
				//signify that we are stopped by moving this animation past the last key frame time,
				//and stopping our updating
				m_fCurTime		= 0.0f;

				//and put ourselves to sleep
				GoInActive();
				break;
			}
		}
		else
		{
			//move to the next key frame
			m_pNextKey = m_pPrevKey;
			m_pPrevKey = GetPrevKey(m_pPrevKey);

			//we can move forward, so move to the next key frame if there is enough time
			float fMoveAmount = LTMIN(fTimeDelta, m_fCurTime - GetKeyTime(m_pPrevKey));

			//update our time to reflect the new position
			fTimeDelta	-= fMoveAmount;
			m_fCurTime	-= fMoveAmount;
		}		
	}

	//determine the percentage between the keys
	m_fKeyPercent = GetKeyPercentage(m_pPrevKey, m_pNextKey, m_fCurTime, fTotalTime);	
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void KeyFramer::Update()
{
	// If we should be disabled stop updating us...
	if (g_vtDisableKeyframers.GetFloat() != 0.0f)
	{
		// For now this is a one-way trip for testing...go to sleep and
		// never wake up...
        Pause();
		return;
	}

	//we shouldn't ever reach here if we are active since we should only receive updates when active
	LTASSERT(m_bActive, "Warning: Keyframer was found updating while inactive. This wastes performance");

	//update our current time
	float fTimeDelta	= g_pLTServer->GetFrameTime();
	float fTotalTime	= GetPathEndTime();

	//handle the forward case
	if(m_eDirection == KFD_FORWARD)
	{
		MoveKeyframerForward(fTimeDelta, fTotalTime);		
	}
	//handle the backward case
	else
	{
		MoveKeyframerBackward(fTimeDelta, fTotalTime);		
	}

	//now position all of the objects that we are keyframing accordingly

	//update our objects to the resulting position. Note that a key can disable it, but at that point
	//we stop iterating through the keys so this should reflect the most up to date position prior to
	//the keyframer being stopped
	UpdateObjects(false);
	
	//if we are still active at this point, we need to subscribe for another update
	if(m_bActive)
		SetNextUpdate(UPDATE_NEXT_FRAME);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::UpdateObjects()
//
//	PURPOSE:	Update the object(s) position(s) / rotation(s)
//
// ----------------------------------------------------------------------- //

void KeyFramer::UpdateObjects(bool bTeleport)
{
	//determine our current keyframer transform
	LTRigidTransform tKeyTrans = CalcCurTransform(m_pPrevKey, m_pNextKey, m_fKeyPercent);

	//now run through our objects and apply that transform
	for(TTrackedObjectList::iterator itObj = m_TrackedObjects.begin(); itObj != m_TrackedObjects.end(); itObj++)
	{
		//first off calculate the transform for this object
		LTRigidTransform tObjTransform = tKeyTrans;
		if (m_bMaintainOffsets)
		{
			tObjTransform *= itObj->m_tRelativeTrans;
		}

		//handle moving the object to the appropriate position
		if (m_bPushObjects && !bTeleport)
		{
            g_pLTServer->Physics()->MoveObject(itObj->m_hObject, tObjTransform.m_vPos, 0);
		}
		else
		{
			g_pLTServer->SetObjectPos(itObj->m_hObject, tObjTransform.m_vPos);
		}

		//if we are targeting an object, override the orientation to face the target
		if (m_hTargetObject)
		{
			//calculate the target object's position and apply our offset
			LTRigidTransform tTargetTrans;
			g_pLTServer->GetObjectTransform(m_hTargetObject, &tTargetTrans);
			LTVector vTargetPos = tTargetTrans * m_vTargetOffset;

			//calculate the direction we need to look to aim at the target
			LTVector vDir = vTargetPos - tObjTransform.m_vPos;
			vDir.Normalize();

			//and create a rotation that aims down that direction
			tObjTransform.m_rRot = LTRotation(vDir, tObjTransform.m_rRot.Up());
		}
		
		//now apply the calculated rotation to the object
		if (m_bPushObjects && !bTeleport)
		{
            g_pLTServer->RotateObject(itObj->m_hObject, tObjTransform.m_rRot);
		}
		else
		{
			g_pLTServer->SetObjectRotation(itObj->m_hObject, tObjTransform.m_rRot);
		}
	}
}

//called to calculate the current transform of the keyframer using the two position keys
//and the percentage between the keys
LTRigidTransform KeyFramer::CalcCurTransform(const KeyData* pKey1, const KeyData* pKey2, float fKeyInterp) const
{
	//sanity check on our data
	LTASSERT(pKey1 && pKey2, "Error: Invalid state when trying to evaluate keyframer position");

	//first handle evaluation of the position
	LTRigidTransform tRV;

	//examine our interpolation type to properly evaluate the position
	switch(m_eInterpType)
	{
	case eKFIT_Linear:
		{
			//handle linear interpolation for position, and spherical interpolation for the rotation
			tRV.m_vPos = pKey1->m_tTrans.m_vPos.Lerp(pKey2->m_tTrans.m_vPos, fKeyInterp);
			tRV.m_rRot.Slerp(pKey1->m_tTrans.m_rRot, pKey2->m_tTrans.m_rRot, fKeyInterp);
		}
		break;
	case eKFIT_Bezier:
		{
			//convert to our bezier keys
			const BezierKeyData* pBez1 = (const BezierKeyData*)pKey1;
			const BezierKeyData* pBez2 = (const BezierKeyData*)pKey2;

			//evaluate the bezier curve for the position, and perform a spherical interpolation for
			//the rotation
			Bezier_Evaluate(tRV.m_vPos, pKey1->m_tTrans.m_vPos, pBez1->m_vNextTangent, 
							pBez2->m_vPrevTangent, pKey2->m_tTrans.m_vPos, fKeyInterp);

			tRV.m_rRot.Slerp(pKey1->m_tTrans.m_rRot, pKey2->m_tTrans.m_rRot, fKeyInterp);
		}
		break;
	default:
		{
			LTERROR("Invalid keyframer interpolation type specified");
		}
		break;
	}

	return tRV;
}

//called to calculate the distance between the specified keys
float KeyFramer::CalcKeyDistance(const KeyData* pKey1, const KeyData* pKey2) const
{
	//sanity check on our data
	LTASSERT(pKey1 && pKey2, "Error: Invalid state when trying to evaluate key distances");

	//examine our interpolation type to properly evaluate the distance
	switch(m_eInterpType)
	{
	case eKFIT_Linear:
		{
			return pKey1->m_tTrans.m_vPos.Dist(pKey2->m_tTrans.m_vPos);
		}
		break;
	case eKFIT_Bezier:
		{
			//convert to our bezier keys
			const BezierKeyData* pBez1 = (const BezierKeyData*)pKey1;
			const BezierKeyData* pBez2 = (const BezierKeyData*)pKey2;

			//the number of times to subdivide the curve to approximate the segment length
			static const uint32 knSubdivAmount = 10;
			return Bezier_SegmentLength(pKey1->m_tTrans.m_vPos, pBez1->m_vNextTangent, 
										pBez2->m_vPrevTangent, pKey2->m_tTrans.m_vPos, knSubdivAmount);
		}
		break;
	default:
		{
			LTERROR("Invalid keyframer interpolation type specified");
			return 0.0f;
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GoToKey
//
//	PURPOSE:	Set the current key to the specified key
//
// ----------------------------------------------------------------------- //

void KeyFramer::GoToKey(const char* pKeyName)
{
	if (!pKeyName) return;

	// Find the specified key...

	const KeyData* pCurKey = FindKey(pKeyName);

	// Couldn't find the key...
	if (!pCurKey)
	{
        g_pLTServer->CPrint("ERROR in KeyFramer::GoToKey() - Couldn't find key '%s'", pKeyName);
		return;
	}

	// Make sure the keyframer isn't paused...
	Resume();

	// Set the current key and current time...
	m_pNextKey		= pCurKey;
	m_pPrevKey		= pCurKey;
	m_fKeyPercent	= 0.0f;
	m_fCurTime		= GetKeyTime(pCurKey);

	// Move/Rotate the objects as necessary...
	UpdateObjects(true);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::MoveToKey
//
//	PURPOSE:	Set the destination key to the specified key
//
// ----------------------------------------------------------------------- //

void KeyFramer::MoveToKey(const CParsedMsg &cMsg)
{
	if (cMsg.GetArgCount() < 2) 
		return;

	//extract the name of the key
	const char* pKeyName = cMsg.GetArg(1);

	//find the key that we are seeking to
	const KeyData* pCur = (m_eDirection == KFD_FORWARD) ? m_pPrevKey : m_pNextKey;

	bool bIsAtOrBefore = false;
	m_pDestinationKey = FindKey(pKeyName, pCur, &bIsAtOrBefore);

	if (!m_pDestinationKey)
	{
		g_pLTServer->CPrint("ERROR in KeyFramer::MoveToKey() - Couldn't find key '%s'", pKeyName);
		return;
	}

	//see if we have a destination command, if so, store it so we can send it when
	//we hit the target
	if (cMsg.GetArgCount() > 2)
		m_sDestCmd = cMsg.GetArg(2).c_str();
	else
		m_sDestCmd.clear();

	// Make sure the keyframer isn't paused...
	Resume();

	if (m_pDestinationKey == pCur)
	{
		// See if we are actually at the key position already
		if( (m_pPrevKey == m_pNextKey) ||
			((pCur == m_pPrevKey) && (m_fKeyPercent <= 0.0f)) ||
			((pCur == m_pNextKey) && (m_fKeyPercent >= 1.0f)))
		{
			HandleKeyEvents(m_pDestinationKey);
			return;
		}
		else
		{
			ToggleDir();
			return;
		}
	}
	else
	{
		// Check to see if we need to change directions to get to the
		// destination key...
		if (bIsAtOrBefore)
		{
			if (m_eDirection == KFD_REVERSE)
			{
				ToggleDir();
			}
		}
		else
		{
			if (m_eDirection == KFD_FORWARD)
			{
				ToggleDir();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::FindKey
//
//	PURPOSE:	Find the specified key
//
// ----------------------------------------------------------------------- //

const KeyData* KeyFramer::FindKey(const char* pKeyName, const KeyData* pTest, bool* pbAtOrBefore)
{
	//default the return data
	if(pbAtOrBefore)
		*pbAtOrBefore = false;

	uint32 nBaseKeyNameLen = LTStrLen( m_sBaseKeyName.c_str() );

	// don't bother comparing if the bases don't even match
	if( LTSubStrICmp( m_sBaseKeyName.c_str(), pKeyName, nBaseKeyNameLen ) != 0 )
		return NULL;

	// make sure there's more to the test name
	uint32 keyNameLen = LTStrLen( pKeyName );
	if( keyNameLen <= nBaseKeyNameLen )
		return NULL;

	//extract out the index from the key
	int32 nIndex = atoi(pKeyName + nBaseKeyNameLen);

	//see if it is within range
	if((nIndex < 0) || ((uint32)nIndex >= m_nNumKeys))
		return NULL;

	//handle updating the test
	if(pTest && pbAtOrBefore)
	{
		uint32 nTestIndex = GetKeyIndex(pTest);
		*pbAtOrBefore = (nTestIndex <= (uint32)nIndex);
	}

	return GetKeyFromIndex((uint32)nIndex);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::On()
//
//	PURPOSE:	Turn keyframer on
//
// --------------------------------------------------------------------------- //

void KeyFramer::On()
{
	m_bPaused = false;

	// Check if we didn't get to create our objectlist yet.  This can happen
	// if the object didn't exist at the time the keyframer was created.
	if( m_TrackedObjects.empty() )
	{
		if( !CreateObjectList( ))
			return;
	}

	GoActive( true );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Off()
//
//	PURPOSE:	Turn keyframer off
//
// --------------------------------------------------------------------------- //

void KeyFramer::Off()
{
	GoInActive();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Pause()
//
//	PURPOSE:	Pause keyframer
//
// --------------------------------------------------------------------------- //

void KeyFramer::Pause()
{
	if (m_bActive && !m_bPaused)
	{
		m_bPaused = true;
		GoInActive();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Resume()
//
//	PURPOSE:	Resume keyframer
//
// --------------------------------------------------------------------------- //

void KeyFramer::Resume()
{
	if (m_bPaused)
	{
		m_bPaused = false;
		GoActive(false);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Forward()
//
//	PURPOSE:	Change keyframer direction to forward
//
// --------------------------------------------------------------------------- //

void KeyFramer::Forward()
{
	//flip our direction if necessary
	if(m_eDirection == KFD_REVERSE)
	{
		//change our facing
		m_eDirection = KFD_FORWARD;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Reverse()
//
//	PURPOSE:	Change keyframer direction to backward
//
// --------------------------------------------------------------------------- //

void KeyFramer::Reverse()
{
	//flip our direction if necessary
	if(m_eDirection == KFD_FORWARD)
	{
		//change our facing
		m_eDirection = KFD_REVERSE;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::ToggleDir()
//
//	PURPOSE:	Toggle keyframer direction
//
// --------------------------------------------------------------------------- //

void KeyFramer::ToggleDir()
{
	if (m_eDirection == KFD_FORWARD)
	{
		Reverse();
	}
	else
	{
		Forward();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void KeyFramer::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) 
		return;

    SAVE_HOBJECT(m_hActiveSndObj);
    SAVE_HOBJECT(m_hTargetObject);
	SAVE_VECTOR(m_vTargetOffset);
    SAVE_FLOAT(m_fDistancePathTime);
    SAVE_FLOAT(m_fTotalDistance);
    SAVE_FLOAT(m_fCurTime);
    SAVE_FLOAT(m_fKeyPercent);
	SAVE_STDSTRING(m_sObjectName);
	SAVE_STDSTRING(m_sTargetName);
	SAVE_STDSTRING(m_sBaseKeyName);
	SAVE_STDSTRING(m_sActiveSndRecord);
	SAVE_STDSTRING(m_sDestCmd);
	SAVE_DWORD(m_nKeyDataIndex);
    SAVE_BOOL(m_bUseDistance);
    SAVE_BOOL(m_bStartActive);
    SAVE_BOOL(m_bStartPaused);
    SAVE_BOOL(m_bLooping);
	SAVE_BOOL(m_bMaintainOffsets);
    SAVE_BOOL(m_bActive);
    SAVE_BOOL(m_bPaused);
    SAVE_BOOL(m_bPushObjects);
    SAVE_BYTE(m_eDirection);
    SAVE_BYTE(m_eWaveform);
	SAVE_BYTE(m_eInterpType);
	SAVE_BYTE(m_nKeyStride);

	// Determine the position in the list of m_pNextKey, m_pPrevKey, and
	// m_pDestKey

	int nNextKeyIndex   = (m_pNextKey) ? (int)GetKeyIndex(m_pNextKey) : -1;
	int nPrevKeyIndex	= (m_pPrevKey) ? (int)GetKeyIndex(m_pPrevKey) : -1;
	int nDestKeyIndex   = (m_pDestinationKey) ? (int)GetKeyIndex(m_pDestinationKey) : -1;

    // Save out the positions of our pointer data members...
	SAVE_INT(nNextKeyIndex);
    SAVE_INT(nPrevKeyIndex);
    SAVE_INT(nDestKeyIndex);


	// Save the number of objects to be key-framed...

    uint8 nNumInList = m_TrackedObjects.size();
    SAVE_BYTE(nNumInList);

	// Save the offsets and rotations for each object...
	for(TTrackedObjectList::iterator itObj = m_TrackedObjects.begin(); itObj != m_TrackedObjects.end(); itObj++)
	{
		SAVE_HOBJECT(itObj->m_hObject);
		SAVE_VECTOR( itObj->m_tRelativeTrans.m_vPos );
		SAVE_ROTATION( itObj->m_tRelativeTrans.m_rRot );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void KeyFramer::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	LOAD_HOBJECT(m_hActiveSndObj);
	LOAD_HOBJECT(m_hTargetObject);
    LOAD_VECTOR(m_vTargetOffset);
    LOAD_FLOAT(m_fDistancePathTime);
    LOAD_FLOAT(m_fTotalDistance);
    LOAD_FLOAT(m_fCurTime);
    LOAD_FLOAT(m_fKeyPercent);
	LOAD_STDSTRING(m_sObjectName);
	LOAD_STDSTRING(m_sTargetName);
	LOAD_STDSTRING(m_sBaseKeyName);
	LOAD_STDSTRING(m_sActiveSndRecord);
	LOAD_STDSTRING(m_sDestCmd);
	LOAD_DWORD(m_nKeyDataIndex);
    LOAD_BOOL(m_bUseDistance);
    LOAD_BOOL(m_bStartActive);
    LOAD_BOOL(m_bStartPaused);
    LOAD_BOOL(m_bLooping);
	LOAD_BOOL(m_bMaintainOffsets);
    LOAD_BOOL(m_bActive);
    LOAD_BOOL(m_bPaused);
    LOAD_BOOL(m_bPushObjects);
    LOAD_BYTE_CAST(m_eDirection, KFDirection);
    LOAD_BYTE_CAST(m_eWaveform, EKFWaveType);
	LOAD_BYTE_CAST(m_eInterpType, EKFInterpType);
	LOAD_BYTE_CAST(m_nKeyStride, uint32);

	// Build the m_pKeys data member...
	CreateKeyList();

	// Determine the positions of our pointer data members...
    int nNextKeyIndex;
	LOAD_INT(nNextKeyIndex);
    int nPrevKeyIndex;
	LOAD_INT(nPrevKeyIndex);
    int nDestKeyIndex;
	LOAD_INT(nDestKeyIndex);

	m_pNextKey			= (nNextKeyIndex < 0) ? NULL : GetKeyFromIndex(nNextKeyIndex);
	m_pPrevKey			= (nPrevKeyIndex < 0) ? NULL : GetKeyFromIndex(nPrevKeyIndex);
	m_pDestinationKey	= (nDestKeyIndex < 0) ? NULL : GetKeyFromIndex(nDestKeyIndex);

	// Load the number of objects we're supposed to key frame...
    uint8 nNumInList;
	LOAD_BYTE(nNumInList);

	m_TrackedObjects.clear();
	m_TrackedObjects.resize( nNumInList );

	//load in each of our objects as well as their transform
	for(uint32 nCurrObject = 0; nCurrObject < nNumInList; nCurrObject++)
	{
		LOAD_HOBJECT(m_TrackedObjects[nCurrObject].m_hObject);
		LOAD_VECTOR(m_TrackedObjects[nCurrObject].m_tRelativeTrans.m_vPos);
		LOAD_ROTATION(m_TrackedObjects[nCurrObject].m_tRelativeTrans.m_rRot);
	}

	// If we were active, restart active sound...
	if (m_bActive)
	{
		m_bActive = false; // need to clear this first...
		GoActive(false);   // don't reset any values...
	}

	//we were definitely created from a save, so we don't need to initialize our object references
	m_bInitializedObjRefs = true;
}

//called to stop the active sound. This will only release the sound handle,
//but will not clear out any of the stored sound data (attached object, radius, name)
void KeyFramer::StopActiveSound()
{
	if (m_hActiveSnd)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hActiveSnd);
		m_hActiveSnd = NULL;
	}
}

//called once all objects have been created to handle setting up our object list
void KeyFramer::AllObjectsCreated()
{
	//see if we have already initialized object references, if so, we don't need to do it again
	if(m_bInitializedObjRefs)
		return;

	//all objects have been created, so we can now setup our object list
	CreateObjectList();

	if( !m_sTargetName.empty() )
	{
		SetTarget( m_sTargetName.c_str() );
	}

	//handle activating if possible (must be active to pause)
	if (m_bStartActive || m_bStartPaused)
	{
		GoActive(true);
	}

	//pause if appropriate
	if (m_bStartPaused)
	{
		Pause();
	}

	// Make sure we're not eating ticks if we're not active...
	if (!m_bActive)
	{
		SetNextUpdate(UPDATE_NEVER);
	}

	//make sure to set this since we can have this functionality called again since we are changing
	//the update delta which re-adds us to the update list
	m_bInitializedObjRefs = true;
}

//given a key, this will convert it to linear time, even if it is stored as a distance
float KeyFramer::GetKeyTime(const KeyData* pKey) const
{
	//see if we can just use the key's time directly
	if(!m_bUseDistance)
		return pKey->m_fPathStamp;

	//we need to convert it from distance to time
	return ConvertDistanceToTime(pKey->m_fPathStamp / m_fTotalDistance, m_eWaveform, GetPathEndTime());
}

//given a valid key, this will return the index of the key
uint32 KeyFramer::GetKeyIndex(const KeyData* pKey) const
{
	LTASSERT((pKey >= m_pKeys) && (pKey <= m_pLastKey), "Error: Invalid key passed into GetKeyIndex");
	uint32 nKeyDiff = (uint32)((const uint8*)pKey - (const uint8*)m_pKeys);

	LTASSERT(nKeyDiff % m_nKeyStride == 0, "Error: Misaligned key passed into GetKeyIndex");
	return nKeyDiff / m_nKeyStride;
}

//given an index, this will return a key
const KeyData* KeyFramer::GetKeyFromIndex(uint32 nIndex) const
{
	LTASSERT(nIndex < m_nNumKeys, "Error: Invalid index access into the list of keys");
	return (const KeyData*)((const uint8*)m_pKeys + nIndex * m_nKeyStride);
}

//given a key, this will get the key immediately before it (this assumes that
//the passed in key is not the first key)
const KeyData* KeyFramer::GetPrevKey(const KeyData* pKey) const
{
	LTASSERT(pKey != m_pKeys, "Error: Attempted to access the key before the first key!");
	return (const KeyData*)((const uint8*)pKey - m_nKeyStride);
}

//given a key, this will get the key immediately after it (this assumes that the
//passed in key is not the last key)
const KeyData* KeyFramer::GetNextKey(const KeyData* pKey) const
{
	LTASSERT(pKey != m_pLastKey, "Error: Attempted to access the key before the first key!");
	return (const KeyData*)((const uint8*)pKey + m_nKeyStride);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	KeyFramer::GetPrefetchResourceList
//
//	PURPOSE:	determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void KeyFramer::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	AddSoundResourceToObjectGatherer(pInterface, Resources, pszObjectName, "ActiveSound");
}

