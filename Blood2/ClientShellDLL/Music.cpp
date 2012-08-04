// ----------------------------------------------------------------------- //
//
// MODULE  : Music.cpp
//
// PURPOSE : Music helper class.  Handles all music triggers
//
// CREATED : 12/18/97
//
// ----------------------------------------------------------------------- //

#include "Music.h"
#include "cpp_client_de.h"
#include "ClientServerShared.h"
#include "client_de.h"
#include "SharedDefs.h"
#include <mbstring.h>



// Transitions arrays are organized so index 0 is the intro from silence,
// index 1 is the ending to silence, and the other indices are transitions
// from the other music levels.  Example:  m_szAmbientTrans index 0=
// intro from silence, index=1 ending to silence, index 2=transition from
// cruising, index 3=transition from harddriving.

static char g_szAmbientTrans[4][PARSE_MAXTOKENSIZE] = 
{
	"sta.sec", "ats.sec", "cta.sec", "hta.sec"
};

static char g_szCruisingTrans[4][PARSE_MAXTOKENSIZE] = 
{
	"stc.sec", "cts.sec", "atc.sec", "htc.sec"
};

static char g_szHarddrivingTrans[4][PARSE_MAXTOKENSIZE] = 
{
	"sth.sec", "cts.sec", "ath.sec", "cth.sec"
};


 
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::CMusic
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CMusic::CMusic( )
{
	m_pClientDE = DNULL;
	m_bUseIma = DFALSE;
	m_eMusicLevel = MUSICLEVEL_SILENCE;
	m_bPlayListInitialized = DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Init
//
//	PURPOSE:	Initialize the music
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::Init( CClientDE *pClientDE, DBOOL bUseIma )
{
	m_pClientDE = pClientDE;
	m_bUseIma = bUseIma;

	if( m_bUseIma )
	{
		if( !pClientDE->InitMusic( "ima.dll" ))
			return DFALSE;
		m_pClientDE->RunConsoleString( "musictype ima" );
	}
	else
	{
		if( !pClientDE->InitMusic( "cdaudio.dll" ))
			return DFALSE;
		m_pClientDE->RunConsoleString( "musictype cdaudio" );
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Term
//
//	PURPOSE:	Terminate the music
//
// ----------------------------------------------------------------------- //
void CMusic::Term( )
{
	TermPlayLists( );

	if( m_pClientDE )
	{
		m_pClientDE->StopMusic( MUSIC_IMMEDIATE );
		m_pClientDE = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Init
//
//	PURPOSE:	Initialize the music
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::InitPlayLists( )
{
	char tokenSpace[PARSE_MAXTOKENS*(PARSE_MAXTOKENSIZE + 1)];
	char *pTokens[PARSE_MAXTOKENS], *pCommandPos, *pCommand;
	int nArgs;

	if( !IsInitialized( ))
		return DFALSE;

	m_bPlayListInitialized = DFALSE;
	m_eMusicLevel = MUSICLEVEL_SILENCE;

	if( m_bUseIma )
	{
		pTokens[0] = m_pClientDE->GetServerConVarValueString( "MusicDirectory" );
		if( !pTokens[0] )
			return DFALSE;
		if( !m_pClientDE->SetMusicDirectory( pTokens[0] ))
			return DFALSE;

		pCommand = m_pClientDE->GetServerConVarValueString( "InstrumentFiles" );
		if( !pCommand )
			return DFALSE;
		m_pClientDE->Parse( pCommand, &pCommandPos, tokenSpace, pTokens, &nArgs );
		if( nArgs != 2 )
			return DFALSE;
		if( !m_pClientDE->InitInstruments( pTokens[0], pTokens[1] ))
			return DFALSE;

		if( !InitTrans( g_szAmbientTrans ))
			return DFALSE;
		if( !InitTrans( g_szCruisingTrans ))
			return DFALSE;
		if( !InitTrans( g_szHarddrivingTrans ))
			return DFALSE;

		if( !m_pClientDE->AddSongToPlayList( "SilenceList", "s.sec" ))
			return DFALSE;

		if( !InitList( "AmbientList", tokenSpace, pTokens ))
			return DFALSE;
		if( !InitList( "CruisingList", tokenSpace, pTokens ))
			return DFALSE;
		if( !InitList( "HarddrivingList", tokenSpace, pTokens ))
			return DFALSE;
	}
	else
	{
		if( !InitList( "CDTrack", tokenSpace, pTokens ))
			return DFALSE;
	}

	m_bPlayListInitialized = DTRUE;
	return DTRUE;
}

DBOOL CMusic::InitTrans( char szTransitions[4][PARSE_MAXTOKENSIZE] )
{
	int i;

	for( i = 3; i >= 0; i-- )
	{
		if( !m_pClientDE->LoadSong( szTransitions[i] ))
			return DFALSE;
	}

	return DTRUE;
}

DBOOL CMusic::InitList( char *szWorldProp,
	char argBuffer[PARSE_MAXTOKENS*(PARSE_MAXTOKENSIZE+1)], char * argPointers[PARSE_MAXTOKENS] )
{
	char *pCommandPos, *pCommand;
	int nArgs, i;

	pCommand = m_pClientDE->GetServerConVarValueString( szWorldProp );
	if( !pCommand )
		return DFALSE;
	m_pClientDE->Parse( pCommand, &pCommandPos, argBuffer, argPointers, &nArgs );
	for( i = 0; i < nArgs; i++ )
	{
		if( !m_pClientDE->AddSongToPlayList( szWorldProp, argPointers[i] ))
			return DFALSE;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::TermPlayLists
//
//	PURPOSE:	Unload the playlists and the songs...
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::TermPlayLists( )
{
	m_bPlayListInitialized = DFALSE;

	if( !IsInitialized( ))
		return DFALSE;

	m_pClientDE->DestroyAllSongs( );
	m_eMusicLevel = MUSICLEVEL_SILENCE;
	return DTRUE;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::HandleMusicMessage
//
//	PURPOSE:	Parses music message.  The original message was of the format
//				"Music <message_data>".  This function must be passed the string
//				after the "Music " and only the "<message_data>" portion.
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::HandleMusicMessage( HMESSAGEREAD hMessage )
{
	char msg[51];

	DBYTE nCommand = m_pClientDE->ReadFromMessageByte( hMessage );

	switch( nCommand )
	{
		// Handle music level change...
		case MUSICCMD_LEVEL:
		{
			DBYTE nLevel;

			nLevel = m_pClientDE->ReadFromMessageByte( hMessage );

			return PlayMusicLevel(( EMusicLevel )nLevel );
		}
		// Handle motif
		case MUSICCMD_MOTIF:
		{
			_mbsncpy((unsigned char*)msg, (const unsigned char*)m_pClientDE->ReadFromMessageString( hMessage ), 50 );

			return PlayMotif( msg, DFALSE );
		}
		case MUSICCMD_MOTIF_LOOP:
		{
			_mbsncpy((unsigned char*)msg, (const unsigned char*)m_pClientDE->ReadFromMessageString( hMessage ), 50 );

			return PlayMotif( msg, DTRUE );
		}
		case MUSICCMD_MOTIF_STOP:
		{
			_mbsncpy((unsigned char*)msg, (const unsigned char*)m_pClientDE->ReadFromMessageString( hMessage ), 50 );

			StopMotif( msg );
			return DTRUE;
		}
		case MUSICCMD_BREAK:
		{
			_mbsncpy((unsigned char*)msg, (const unsigned char*)m_pClientDE->ReadFromMessageString( hMessage ), 50 );

			return PlayBreak( msg );
		}
	}
	
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::PlayMusicLevel
//
//	PURPOSE:	Sets music level
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::PlayMusicLevel( EMusicLevel nLevel )
{
	switch( nLevel )
	{
		case MUSICLEVEL_SILENCE:
		{
			switch( m_eMusicLevel )
			{
				case MUSICLEVEL_AMBIENT:
				{
					return TransitionToLevel( g_szAmbientTrans[1], "SilenceList", MUSICLEVEL_SILENCE, DFALSE );
					break;
				}
				case MUSICLEVEL_CRUISING:
				{
					return TransitionToLevel( g_szCruisingTrans[1], "SilenceList", MUSICLEVEL_SILENCE, DFALSE );
					break;
				}
				case MUSICLEVEL_HARDDRIVING:
				{
					return TransitionToLevel( g_szHarddrivingTrans[1], "SilenceList", MUSICLEVEL_SILENCE, DFALSE );
					break;
				}
				case MUSICLEVEL_SILENCE:
				{
					return ( DBOOL )m_pClientDE->PlayList( "SilenceList", DNULL, DTRUE, MUSIC_NEXTSONG );
					break;
				}
			}
			break;
		}
		case MUSICLEVEL_AMBIENT:
		{
			switch( m_eMusicLevel )
			{
				case MUSICLEVEL_SILENCE:
				{
					return TransitionToLevel( g_szAmbientTrans[0], "AmbientList", MUSICLEVEL_AMBIENT, DTRUE );
					break;
				}
				case MUSICLEVEL_CRUISING:
				{
					return TransitionToLevel( g_szAmbientTrans[2], "AmbientList", MUSICLEVEL_AMBIENT, DFALSE );
					break;
				}
				case MUSICLEVEL_HARDDRIVING:
				{
					return TransitionToLevel( g_szAmbientTrans[3], "AmbientList", MUSICLEVEL_AMBIENT, DFALSE );
					break;
				}
			}
			break;
		}
		case MUSICLEVEL_CRUISING:
		{
			switch( m_eMusicLevel )
			{
				case MUSICLEVEL_SILENCE:
				{
					return TransitionToLevel( g_szCruisingTrans[0], "CruisingList", MUSICLEVEL_CRUISING, DTRUE );
					break;
				}
				case MUSICLEVEL_AMBIENT:
				{
					return TransitionToLevel( g_szCruisingTrans[2], "CruisingList", MUSICLEVEL_CRUISING, DFALSE );
					break;
				}
				case MUSICLEVEL_HARDDRIVING:
				{
					return TransitionToLevel( g_szCruisingTrans[3], "CruisingList", MUSICLEVEL_CRUISING, DFALSE );
					break;
				}
			}
			break;
		}
		case MUSICLEVEL_HARDDRIVING:
		{
			switch( m_eMusicLevel )
			{
				case MUSICLEVEL_SILENCE:
				{
					return TransitionToLevel( g_szHarddrivingTrans[0], "HarddrivingList", MUSICLEVEL_HARDDRIVING, DTRUE );
					break;
				}
				case MUSICLEVEL_AMBIENT:
				{
					return TransitionToLevel( g_szHarddrivingTrans[2], "HarddrivingList", MUSICLEVEL_HARDDRIVING, DFALSE );
					break;
				}
				case MUSICLEVEL_CRUISING:
				{
					return TransitionToLevel( g_szHarddrivingTrans[3], "HarddrivingList", MUSICLEVEL_HARDDRIVING, DFALSE );
					break;
				}
			}
			break;
		}
	}

	return DFALSE;
}

DBOOL CMusic::TransitionToLevel( char *szTransition, char *szList, EMusicLevel eLevel, DBOOL bImmediate )
{
	m_eMusicLevel = eLevel;

	if( !m_bUseIma || !IsPlaylistInitialized( ))
		return DTRUE;

	if( szTransition[0] == 0 )
		return DFALSE;

	return ( DBOOL )m_pClientDE->PlayList( szList, szTransition, DTRUE, bImmediate ? MUSIC_IMMEDIATE : MUSIC_NEXTSONG );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::PlayMotif
//
//	PURPOSE:	Plays motif
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::PlayMotif( char *pMotifName, DBOOL bLoop )
{
	if( !m_bUseIma )
		return DTRUE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::StopMotif
//
//	PURPOSE:	Stops a looping motif
//
// ----------------------------------------------------------------------- //
void CMusic::StopMotif( char *pMotifName )
{
	if( !m_bUseIma )
		return;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::PlayBreak
//
//	PURPOSE:	Plays break
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::PlayBreak( char *pBreakName )
{
	if( !m_bUseIma )
		return DTRUE;

	return DTRUE;
}
