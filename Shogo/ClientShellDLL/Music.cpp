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
#include "clientheaders.h"
#include "ClientServerShared.h"
#include "iltclient.h"
#include "RiotMsgIDs.h"
#include "iltcommon.h"
#include "iltmessage.h"

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
	m_pClientDE = LTNULL;
	m_bUseIma = LTFALSE;
	m_eMusicLevel = MUSICLEVEL_SILENCE;
	m_bPlayListInitialized = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::Init
//
//	PURPOSE:	Initialize the music
//
// ----------------------------------------------------------------------- //
LTBOOL CMusic::Init( ILTClient *pClientDE, LTBOOL bUseIma )
{
	m_pClientDE = pClientDE;
	m_bUseIma = bUseIma;

	if( m_bUseIma )
	{
		if( !pClientDE->InitMusic( "ima.dll" ))
			return LTFALSE;
		m_pClientDE->RunConsoleString( "+musictype ima" );
	}
	else
	{
		if( !pClientDE->InitMusic( "cdaudio.dll" ))
			return LTFALSE;
		m_pClientDE->RunConsoleString( "+musictype cdaudio" );
	}

	return LTTRUE;
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
LTBOOL CMusic::InitPlayLists( )
{
	char tokenSpace[PARSE_MAXTOKENS*(PARSE_MAXTOKENSIZE + 1)];
	char *pTokens[PARSE_MAXTOKENS], *pCommandPos, *pCommand;
	int nArgs;

	if( !IsInitialized( ))
		return LTFALSE;

	m_bPlayListInitialized = LTFALSE;
	m_eMusicLevel = MUSICLEVEL_SILENCE;

	if( m_bUseIma )
	{
		pTokens[0] = m_pClientDE->GetServerConVarValueString( "MusicDirectory" );
		if( !pTokens[0] )
			return LTFALSE;
		if( !m_pClientDE->SetMusicDirectory( pTokens[0] ))
			return LTFALSE;

		pCommand = m_pClientDE->GetServerConVarValueString( "InstrumentFiles" );
		if( !pCommand )
			return LTFALSE;
		m_pClientDE->Parse( pCommand, &pCommandPos, tokenSpace, pTokens, &nArgs );
		if( nArgs != 2 )
			return LTFALSE;
		if( !m_pClientDE->Common()->InitInstruments( pTokens[0], pTokens[1] ))
			return LTFALSE;

		if( !InitTrans( g_szAmbientTrans ))
			return LTFALSE;
		if( !InitTrans( g_szCruisingTrans ))
			return LTFALSE;
		if( !InitTrans( g_szHarddrivingTrans ))
			return LTFALSE;

		if( !m_pClientDE->AddSongToPlayList( "SilenceList", "s.sec" ))
			return LTFALSE;

		if( !InitList( "AmbientList", tokenSpace, pTokens ))
			return LTFALSE;
		if( !InitList( "CruisingList", tokenSpace, pTokens ))
			return LTFALSE;
		if( !InitList( "HarddrivingList", tokenSpace, pTokens ))
			return LTFALSE;
	}
	else
	{
		if( !InitList( "CDTrack", tokenSpace, pTokens ))
			return LTFALSE;
	}

	m_bPlayListInitialized = LTTRUE;
	return LTTRUE;
}

LTBOOL CMusic::InitTrans( char szTransitions[4][PARSE_MAXTOKENSIZE] )
{
	int i;

	for( i = 3; i >= 0; i-- )
	{
		if( !m_pClientDE->LoadSong( szTransitions[i] ))
			return LTFALSE;
	}

	return LTTRUE;
}

LTBOOL CMusic::InitList( char *szWorldProp,
	char argBuffer[PARSE_MAXTOKENS*(PARSE_MAXTOKENSIZE+1)], char * argPointers[PARSE_MAXTOKENS] )
{
	char *pCommandPos, *pCommand;
	int nArgs, i;

	pCommand = m_pClientDE->Common()->GetServerConVarValueString( szWorldProp );
	if( !pCommand )
		return LTFALSE;
	m_pClientDE->Common()->Parse( pCommand, &pCommandPos, argBuffer, argPointers, &nArgs );
	for( i = 0; i < nArgs; i++ )
	{
		if( !m_pClientDE->AddSongToPlayList( szWorldProp, argPointers[i] ))
			return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::TermPlayLists
//
//	PURPOSE:	Unload the playlists and the songs...
//
// ----------------------------------------------------------------------- //
LTBOOL CMusic::TermPlayLists( )
{
	m_bPlayListInitialized = LTFALSE;

	if( !IsInitialized( ))
		return LTFALSE;

	m_pClientDE->DestroyAllSongs( );
	m_eMusicLevel = MUSICLEVEL_SILENCE;
	return LTTRUE;
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
LTBOOL CMusic::HandleMusicMessage( ILTMessage_Read* hMessage )
{
	char msg[51];

	uint8 nCommand = hMessage->Readuint8();

	switch( nCommand )
	{
		// Handle music level change...
		case MUSICCMD_LEVEL:
		{
			uint8 nLevel;

			nLevel = hMessage->Readuint8();

			return PlayMusicLevel(( EMusicLevel )nLevel );
		}
		// Handle motif
		case MUSICCMD_MOTIF:
		{
			hMessage->ReadString(msg, 50);

			return PlayMotif( msg, LTFALSE );
		}
		case MUSICCMD_MOTIF_LOOP:
		{
			hMessage->ReadString(msg, 50);

			return PlayMotif( msg, LTTRUE );
		}
		case MUSICCMD_MOTIF_STOP:
		{
			hMessage->ReadString(msg, 50);

			StopMotif( msg );
			return LTTRUE;
		}
		case MUSICCMD_BREAK:
		{
			hMessage->ReadString(msg, 50);

			return PlayBreak( msg );
		}
	}
	
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::PlayMusicLevel
//
//	PURPOSE:	Sets music level
//
// ----------------------------------------------------------------------- //
LTBOOL CMusic::PlayMusicLevel( EMusicLevel nLevel )
{
	switch( nLevel )
	{
		case MUSICLEVEL_SILENCE:
		{
			switch( m_eMusicLevel )
			{
				case MUSICLEVEL_AMBIENT:
				{
					return TransitionToLevel( g_szAmbientTrans[1], "SilenceList", MUSICLEVEL_SILENCE, LTFALSE );
					break;
				}
				case MUSICLEVEL_CRUISING:
				{
					return TransitionToLevel( g_szCruisingTrans[1], "SilenceList", MUSICLEVEL_SILENCE, LTFALSE );
					break;
				}
				case MUSICLEVEL_HARDDRIVING:
				{
					return TransitionToLevel( g_szHarddrivingTrans[1], "SilenceList", MUSICLEVEL_SILENCE, LTFALSE );
					break;
				}
				case MUSICLEVEL_SILENCE:
				{
					return ( LTBOOL )m_pClientDE->PlayList( "SilenceList", LTNULL, LTTRUE, MUSIC_NEXTSONG );
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
					return TransitionToLevel( g_szAmbientTrans[0], "AmbientList", MUSICLEVEL_AMBIENT, LTTRUE );
					break;
				}
				case MUSICLEVEL_CRUISING:
				{
					return TransitionToLevel( g_szAmbientTrans[2], "AmbientList", MUSICLEVEL_AMBIENT, LTFALSE );
					break;
				}
				case MUSICLEVEL_HARDDRIVING:
				{
					return TransitionToLevel( g_szAmbientTrans[3], "AmbientList", MUSICLEVEL_AMBIENT, LTFALSE );
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
					return TransitionToLevel( g_szCruisingTrans[0], "CruisingList", MUSICLEVEL_CRUISING, LTTRUE );
					break;
				}
				case MUSICLEVEL_AMBIENT:
				{
					return TransitionToLevel( g_szCruisingTrans[2], "CruisingList", MUSICLEVEL_CRUISING, LTFALSE );
					break;
				}
				case MUSICLEVEL_HARDDRIVING:
				{
					return TransitionToLevel( g_szCruisingTrans[3], "CruisingList", MUSICLEVEL_CRUISING, LTFALSE );
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
					return TransitionToLevel( g_szHarddrivingTrans[0], "HarddrivingList", MUSICLEVEL_HARDDRIVING, LTTRUE );
					break;
				}
				case MUSICLEVEL_AMBIENT:
				{
					return TransitionToLevel( g_szHarddrivingTrans[2], "HarddrivingList", MUSICLEVEL_HARDDRIVING, LTFALSE );
					break;
				}
				case MUSICLEVEL_CRUISING:
				{
					return TransitionToLevel( g_szHarddrivingTrans[3], "HarddrivingList", MUSICLEVEL_HARDDRIVING, LTFALSE );
					break;
				}
			}
			break;
		}
	}

	return LTFALSE;
}

LTBOOL CMusic::TransitionToLevel( char *szTransition, char *szList, EMusicLevel eLevel, LTBOOL bImmediate )
{
	m_eMusicLevel = eLevel;

	if( !m_bUseIma || !IsPlaylistInitialized( ))
		return LTTRUE;

	if( szTransition[0] == 0 )
		return LTFALSE;

	return ( LTBOOL )m_pClientDE->PlayList( szList, szTransition, LTTRUE, bImmediate ? MUSIC_IMMEDIATE : MUSIC_NEXTSONG );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::PlayMotif
//
//	PURPOSE:	Plays motif
//
// ----------------------------------------------------------------------- //
LTBOOL CMusic::PlayMotif( char *pMotifName, LTBOOL bLoop )
{
	if( !m_bUseIma )
		return LTTRUE;

	return LTTRUE;
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
LTBOOL CMusic::PlayBreak( char *pBreakName )
{
	if( !m_bUseIma )
		return LTTRUE;

	return LTTRUE;
}
