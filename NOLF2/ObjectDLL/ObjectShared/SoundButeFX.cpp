// ----------------------------------------------------------------------- //
//
// MODULE  : SoundButeFX.cpp
//
// PURPOSE : The SoundButeFX implementation
//
// CREATED : 11/06/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "SoundButeFX.h"
	#include "ServerSoundMgr.h"
	#include "CommandMgr.h"

LINKFROM_MODULE( SoundButeFX );
	
//
// Defines...
//

	#define SOUND_BUTEFX_PARENT			"BaseSFXObjSound"


BEGIN_CLASS( SoundButeFX )

	ADD_STRINGPROP_FLAG( SoundBute, "None", PF_STATICLIST )
	ADD_BOOLPROP_FLAG( StartOn, LTTRUE, 0 )
	ADD_LONGINTPROP_FLAG( NumPlays, -1, 0 )
	ADD_REALPROP_FLAG( MinDelay, 0.0f, 0 )
	ADD_REALPROP_FLAG( MaxDelay, 0.0f, 0 )
	ADD_REALPROP_FLAG( InnerRadius, -1.0f, PF_RADIUS )
	ADD_REALPROP_FLAG( OuterRadius, -1.0f, PF_RADIUS )
	ADD_BOOLPROP_FLAG( WaitUntilFinished, LTTRUE, 0 )
	ADD_BOOLPROP_FLAG( Ambient, LTTRUE, 0 )

END_CLASS_DEFAULT_FLAGS_PLUGIN( SoundButeFX, GameBase, NULL, NULL, 0, CSoundButeFXPlugin )

CMDMGR_BEGIN_REGISTER_CLASS( SoundButeFX )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( TOGGLE, 1, NULL, "TOGGLE" )

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
	if( !_stricmp( szPropName, "SoundBute" ))
	{
		if( m_SoundButeMgrPlugin.PreHook_EditStringList( szRezPath,
														 szPropName,
														 aszStrings,
														 pcStrings,
														 cMaxStrings,
														 cMaxStringLength,
														 SOUND_BUTEFX_PARENT ) == LT_OK )
		{
			// Sort the list...
			
			qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );

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
	m_hstrSoundBute	( LTNULL ),
	m_bOn			( LTFALSE ),
	m_nNumPlays		( -1 ),
	m_fMinDelay		( 0.0f ),
	m_fMaxDelay		( 0.0f ),
	m_fInnerRadius	( -1.0f ),
	m_fOuterRadius	( -1.0f ),
	m_bWait			( LTTRUE ),
	m_bAmbient		( LTTRUE ),
	m_fLastPlayTime	( 0.0f ),
	m_fDelay		( 0.0f ),
	m_hSound		( LTNULL )
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
	FREE_HSTRING( m_hstrSoundBute );

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

uint32 SoundButeFX::EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData )
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
					ReadProps( pOCS );
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

void SoundButeFX::ReadProps( ObjectCreateStruct *pOCS )
{
	if( !pOCS ) return;

	GenericProp gProp;

	if( g_pLTServer->GetPropGeneric( "SoundBute", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_hstrSoundBute = g_pLTServer->CreateString( gProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "StartOn", &gProp ) == LT_OK )
	{
		m_bOn = gProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "NumPlays", &gProp ) == LT_OK )
	{
		m_nNumPlays = gProp.m_Long;
	}

	if( g_pLTServer->GetPropGeneric( "MinDelay", &gProp ) == LT_OK )
	{
		m_fMinDelay = gProp.m_Float;
	}

	if( g_pLTServer->GetPropGeneric( "MaxDelay", &gProp ) == LT_OK )
	{
		m_fMaxDelay = gProp.m_Float;
	}

	if( g_pLTServer->GetPropGeneric( "InnerRadius", &gProp ) == LT_OK )
	{
		m_fInnerRadius = gProp.m_Float;
	}

	if( g_pLTServer->GetPropGeneric( "OuterRadius", &gProp ) == LT_OK )
	{
		m_fOuterRadius = gProp.m_Float;
	}

	if( g_pLTServer->GetPropGeneric( "WaitUntilFinished", &gProp ) == LT_OK )
	{
		m_bWait = gProp.m_Bool;
	}

	if( g_pLTServer->GetPropGeneric( "Ambient", &gProp ) == LT_OK )
	{
		m_bAmbient = gProp.m_Bool;
	}
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

	LTFLOAT fTime = g_pLTServer->GetTime();

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
					m_hSound = LTNULL;
				}

				// Play the sound...

				LTVector vPos;
				g_pLTServer->GetObjectPos( m_hObject, &vPos );

				uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME |
								(m_bAmbient ? PLAYSOUND_AMBIENT : 0);

				m_hSound = g_pServerSoundMgr->PlaySoundFromPos( vPos, g_pLTServer->GetStringData( m_hstrSoundBute ), m_fOuterRadius,
																SOUNDPRIORITY_MISC_LOW, dwFlags, SMGR_DEFAULT_VOLUME, 1.0f, m_fInnerRadius );

				
			}
		}

	}

	if( m_hSound )
	{
		bool bDone = false;

		if( g_pLTServer->SoundMgr()->IsSoundDone( m_hSound, bDone ) != LT_OK || bDone )
		{
			g_pLTServer->SoundMgr()->KillSound( m_hSound );
			m_hSound = LTNULL;

			if( !m_bOn )
				SetNextUpdate( UPDATE_NEVER );
		}
	}

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SoundButeFX::OnTrigger
//
//  PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool SoundButeFX::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Toggle("TOGGLE");

	if( cMsg.GetArg(0) == s_cTok_On )
	{
		m_bOn = LTTRUE;
	}
	else if( cMsg.GetArg(0) == s_cTok_Off )
	{
		m_bOn = LTFALSE;
	}
	else if( cMsg.GetArg(0) == s_cTok_Toggle )
	{
		m_bOn = !m_bOn;
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	if( m_bOn )
	{
		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
	else if( !m_bWait && m_hSound )
	{
		g_pLTServer->SoundMgr()->KillSound( m_hSound );
		m_hSound = LTNULL;
	}

	return true;
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

	SAVE_HSTRING( m_hstrSoundBute );
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

	LOAD_HSTRING( m_hstrSoundBute );
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
}
