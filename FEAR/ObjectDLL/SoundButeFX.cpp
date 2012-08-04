// ----------------------------------------------------------------------- //
//
// MODULE  : SoundButeFX.cpp
//
// PURPOSE : The SoundButeFX implementation
//
// CREATED : 11/06/01
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "SoundButeFX.h"
	#include "ServerSoundMgr.h"
	#include "CommandMgr.h"
	#include "SoundDB.h"

LINKFROM_MODULE( SoundButeFX );
	
//
// Defines...
//

	#define SOUND_BUTEFX_PARENT			"BaseSFXObjSound"


BEGIN_CLASS( SoundButeFX )

	ADD_STRINGPROP_FLAG( SoundBute, "", PF_STATICLIST, "The name of a sound record that will be played." )
	ADD_BOOLPROP_FLAG( StartOn, true, 0, "Play immediately." )
	ADD_LONGINTPROP_FLAG( NumPlays, -1, 0, "Number of times to play sound before removing object." )
	ADD_REALPROP_FLAG( MinDelay, 0.0f, 0, "Minimum random delay between playing of sound." )
	ADD_REALPROP_FLAG( MaxDelay, 0.0f, 0, "Maximum random delay between playing of sound." )
	ADD_REALPROP_FLAG( InnerRadius, -1.0f, PF_RADIUS, "Inner radius of sound. -1 uses sound record." )
	ADD_REALPROP_FLAG( OuterRadius, -1.0f, PF_RADIUS, "Outer radius of sound. -1 uses sound record." )
	ADD_BOOLPROP_FLAG( WaitUntilFinished, true, 0, "Wait until current sound is done playing before starting new one." )
	ADD_BOOLPROP_FLAG( Ambient, true, 0, "Play sound as ambient." )

	ADD_PREFETCH_RESOURCE_PROPS()
	
END_CLASS_FLAGS_PLUGIN_PREFETCH(SoundButeFX, GameBase, 0, CSoundButeFXPlugin, DefaultPrefetch<SoundButeFX>, "SoundButeFX are used to play a sound that is specified in the game database at a specific point in the level." )

CMDMGR_BEGIN_REGISTER_CLASS( SoundButeFX )

	ADD_MESSAGE( ON,		1,	NULL,	MSG_HANDLER( SoundButeFX, HandleOnMsg ),		"ON", "Starts playing a sound based on the SoundBute specified in the SoundBute property.", "msg SoundButeFX ON" )
	ADD_MESSAGE( OFF,		1,	NULL,	MSG_HANDLER( SoundButeFX, HandleOffMsg ),		"OFF", "Stops any playing sounds", "msg SoundButeFX OFF" )
	ADD_MESSAGE( TOGGLE,	1,	NULL,	MSG_HANDLER( SoundButeFX, HandleToggleMsg ),	"TOGGLE", "Toggles the on/off state of the sound.", "msg SoundButeFX TOGGLE" )

CMDMGR_END_REGISTER_CLASS( SoundButeFX, GameBase )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSoundButeFXPlugin::PreHook_EditStringList
//
//  PURPOSE:	Fill in any string lists we may have...
//
// ----------------------------------------------------------------------- //

LTRESULT CSoundButeFXPlugin::PreHook_EditStringList( const char *szRezPath, 
													 const char *szPropName,
													 char **aszStrings, 
													 uint32 *pcStrings,
													 const uint32 cMaxStrings,
													 const uint32 cMaxStringLength )
{
	if( LTStrIEquals( szPropName, "SoundBute" ))
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
//  ROUTINE:	SoundButeFX::SoundButeFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

SoundButeFX::SoundButeFX( )
:	GameBase		( OT_NORMAL ),
	m_sSoundBute	( ),
	m_bOn			( false ),
	m_nNumPlays		( -1 ),
	m_fMinDelay		( 0.0f ),
	m_fMaxDelay		( 0.0f ),
	m_fInnerRadius	( -1.0f ),
	m_fOuterRadius	( -1.0f ),
	m_bWait			( true ),
	m_bAmbient		( true ),
	m_fLastPlayTime	( 0.0f ),
	m_fDelay		( 0.0f ),
	m_hSound		( NULL ),
	m_nMixChannel	( PLAYSOUND_MIX_DEFAULT )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::~SoundButeFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

SoundButeFX::~SoundButeFX( )
{
	if( m_hSound )
	{
		g_pLTServer->SoundMgr()->KillSound( m_hSound );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::EngineMessageFn
//
//  PURPOSE:	Handle messages from the server...
//
// ----------------------------------------------------------------------- //

uint32 SoundButeFX::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch( messageID )
	{
		case MID_UPDATE :
		{
			Update( );
		}
		break;

		case MID_PRECREATE :
		{
			// Let the GameBase handle the message first

			uint32 dwRet = GameBase::EngineMessageFn( messageID, pData, fData );

			ObjectCreateStruct	*pOCS = (ObjectCreateStruct*)pData;

			if( pOCS )
			{
				if( PRECREATE_WORLDFILE == fData )
				{
					ReadProps( &pOCS->m_cProperties );

					// Can't do anything without a sound record.
					if( m_sSoundBute.empty())
						return 0;
				}
			}

			// Important!! - We already sent the message to the GameBase so DONT do it again.

			return dwRet;
		}
		break;

		case MID_OBJECTCREATED :
		{
			if( m_bOn )
			{
				SetNextUpdate( UPDATE_NEXT_FRAME );
			}
		}
		break;

		case MID_SAVEOBJECT :
		{
			Save( (ILTMessage_Write*)pData );
		}
		break;

		case MID_LOADOBJECT :
		{
			Load( (ILTMessage_Read*)pData );
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::ReadProps
//
//  PURPOSE:	Read the property values...
//
// ----------------------------------------------------------------------- //

void SoundButeFX::ReadProps( const GenericPropList *pProps )
{
	if( !pProps )
		return;

	m_sSoundBute	= pProps->GetString( "SoundBute", "" );
	m_bOn			= pProps->GetBool( "StartOn", m_bOn );
	m_nNumPlays		= pProps->GetLongInt( "NumPlays", m_nNumPlays );
	m_fMinDelay		= pProps->GetReal( "MinDelay", m_fMinDelay );
	m_fMaxDelay		= pProps->GetReal( "MaxDelay", m_fMaxDelay );
	m_fInnerRadius	= pProps->GetReal( "InnerRadius", m_fInnerRadius );
	m_fOuterRadius	= pProps->GetReal( "OuterRadius", m_fOuterRadius );
	m_bWait			= pProps->GetBool( "WaitUntilFinished", m_bWait );
	m_bAmbient		= pProps->GetBool( "Ambient", m_bAmbient );
	m_nMixChannel	= (int16)pProps->GetLongInt( "MixChannel", m_nMixChannel );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::Update
//
//  PURPOSE:	Update the sound, get rid of object if need be or repeat sound...
//
// ----------------------------------------------------------------------- //

void SoundButeFX::Update( )
{
	SetNextUpdate( UPDATE_NEXT_FRAME );

	double fTime = g_pLTServer->GetTime();

	bool bFirstUpdate = false;
	if( m_bOn )
	{
		// See if we need to play the sound
		if( (fTime - m_fLastPlayTime) >= m_fDelay )
		{
			if( !m_bWait || !m_hSound )
			{
				// Check to see if we should go away
				if( m_nNumPlays != -1 )
				{
					if( m_nNumPlays <= 0 )
					{
						g_pLTServer->RemoveObject( m_hObject );
						return;
					}

					m_nNumPlays--;
				}

				// Reset our times...

				m_fLastPlayTime = fTime;
				m_fDelay = GetRandom( m_fMinDelay, m_fMaxDelay );

				// Free the sound if it exists already...

				if( m_hSound )
				{
					g_pLTServer->SoundMgr()->KillSound( m_hSound );
					m_hSound = NULL;
				}

				// Play the sound...

				LTVector vPos;
				g_pLTServer->GetObjectPos( m_hObject, &vPos );

				uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME |
								(m_bAmbient ? PLAYSOUND_AMBIENT : 0);

				// get the HRECORD to send the bute info along..

				HRECORD hSoundBute;
				
				hSoundBute = g_pSoundDB->GetSoundDBRecord( m_sSoundBute.c_str());

				m_hSound = g_pServerSoundMgr->PlayDBSoundFromPos( vPos, hSoundBute, m_fOuterRadius,
					SOUNDPRIORITY_MISC_LOW, dwFlags, SMGR_DEFAULT_VOLUME, 1.0f, m_fInnerRadius,
					DEFAULT_SOUND_CLASS, m_nMixChannel );

				bFirstUpdate= true;
				
			}
		}

	}

	// skipping this update the first time through because there seems to be
	// an issue with starting and stopping a sound within the same frame on
	// the server. -- Terry
	if( m_hSound && !bFirstUpdate)
	{
		bool bDone = false;

		if( g_pLTServer->SoundMgr()->IsSoundDone( m_hSound, bDone ) != LT_OK || bDone )
		{
			g_pLTServer->SoundMgr()->KillSound( m_hSound );
			m_hSound = NULL;

			if( !m_bOn )
				SetNextUpdate( UPDATE_NEVER );
		}
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::HandleOnMsg
//
//  PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void SoundButeFX::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bOn = true;
	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::HandleOffMsg
//
//  PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void SoundButeFX::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bOn = false;

	if( !m_bWait && m_hSound )
	{
		g_pLTServer->SoundMgr()->KillSound( m_hSound );
		m_hSound = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::HandleToggleMsg
//
//  PURPOSE:	Handle a TOGGLE message...
//
// ----------------------------------------------------------------------- //

void SoundButeFX::HandleToggleMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_bOn )
	{
		HandleOffMsg( hSender, crParsedMsg );
	}
	else
	{
		HandleOnMsg( hSender, crParsedMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::Save
//
//  PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void SoundButeFX::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg ) return;

	SAVE_STDSTRING( m_sSoundBute );
	SAVE_BOOL( m_bOn );
	SAVE_INT( m_nNumPlays );
	SAVE_FLOAT( m_fMinDelay );
	SAVE_FLOAT( m_fMaxDelay );
	SAVE_FLOAT( m_fInnerRadius );
	SAVE_FLOAT( m_fOuterRadius );
	SAVE_BOOL( m_bWait );
	SAVE_BOOL( m_bAmbient );
	SAVE_TIME( m_fLastPlayTime );
	SAVE_FLOAT( m_fDelay );
	SAVE_INT( m_nMixChannel );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::Load
//
//  PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void SoundButeFX::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return;

	LOAD_STDSTRING( m_sSoundBute );
	LOAD_BOOL( m_bOn );
	LOAD_INT( m_nNumPlays );
	LOAD_FLOAT( m_fMinDelay );
	LOAD_FLOAT( m_fMaxDelay );
	LOAD_FLOAT( m_fInnerRadius );
	LOAD_FLOAT( m_fOuterRadius );
	LOAD_BOOL( m_bWait );
	LOAD_BOOL( m_bAmbient );
	LOAD_TIME( m_fLastPlayTime );
	LOAD_FLOAT( m_fDelay );
	LOAD_INT(m_nMixChannel );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SoundButeFX::GetPrefetchResourceList
//
//	PURPOSE:	Determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void SoundButeFX::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// get the sound bute
	char szSoundBute[MAX_PATH];
	pInterface->GetPropString(pszObjectName, "SoundBute", szSoundBute, LTARRAYSIZE(szSoundBute), "");

	if (!LTStrEmpty(szSoundBute))
	{	
		// get the record and add all its resources to the list
		HRECORD hSoundBute = g_pSoundDB->GetSoundDBRecord(szSoundBute);
		GetRecordResources(Resources, hSoundBute, true);
	}
}
