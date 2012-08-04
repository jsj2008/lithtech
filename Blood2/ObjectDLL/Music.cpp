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
#include "PlayerObj.h"
#include "cpp_server_de.h"
#include "SharedDefs.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::CMusic
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CMusic::CMusic( )
{
	m_pMyObj = DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::~CMusic
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //
CMusic::~CMusic( )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::HandleMusicMessage
//
//	PURPOSE:	Parses music message.  This is the rest of the message after
//				the "Music" argument in the string.
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::HandleMusicMessage( char *pMsg )
{
	if( !pMsg )
		return DFALSE;

	char *pMsgType = strtok( pMsg, " " );
	if( pMsgType )
	{
		// Handle music level change...
		if( _mbsicmp((const unsigned char*) "LEVEL", (const unsigned char*)pMsgType ) == 0 )
		{
			// Get the new level
			char *pLevel = strtok( NULL, " " );

			DBYTE nLevel = 0;

			if( pLevel )
			{
				nLevel = ( DBYTE )atoi( pLevel );
			}

			return SetMusicLevel( nLevel );
		}
		// Handle motif
		else if( _mbsicmp((const unsigned char*) "MOTIF", (const unsigned char*)pMsgType ) == 0 )
		{
			char *pMotifName = strtok( NULL, " " );

			if( pMotifName )
			{
				return PlayMotif( pMotifName, DFALSE );
			}
		}
		else if( _mbsicmp((const unsigned char*) "BREAK", (const unsigned char*)pMsgType ) == 0 )
		{
			char *pBreakName = strtok( NULL, " " );

			if( pBreakName )
			{
				return PlayBreak( pBreakName );
			}
		}
	}
	
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMusic::SetMusicLevel
//
//	PURPOSE:	Sets music level
//
// ----------------------------------------------------------------------- //
DBOOL CMusic::SetMusicLevel( DBYTE nLevel )
{
	if( !g_pServerDE || !m_pMyObj )
		return DFALSE;

	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage( m_pMyObj->GetClient( ), SMSG_MUSIC );
	g_pServerDE->WriteToMessageByte( hMessage, MUSICCMD_LEVEL );
	g_pServerDE->WriteToMessageByte( hMessage, nLevel );
	g_pServerDE->EndMessage(hMessage);

	return DTRUE;
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
	if( !g_pServerDE || !m_pMyObj )
		return DFALSE;

	// Update client's mode info...
	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage( m_pMyObj->GetClient( ), SMSG_MUSIC );
	if( bLoop )
		g_pServerDE->WriteToMessageByte( hMessage, MUSICCMD_MOTIF_LOOP );
	else
		g_pServerDE->WriteToMessageByte( hMessage, MUSICCMD_MOTIF );
	g_pServerDE->WriteToMessageString( hMessage, pMotifName );
	g_pServerDE->EndMessage(hMessage);

	return DTRUE;
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
	if( !g_pServerDE || !m_pMyObj )
		return DFALSE;

	// Update client's mode info...
	HMESSAGEWRITE hMessage = g_pServerDE->StartMessage( m_pMyObj->GetClient( ), SMSG_MUSIC );
	g_pServerDE->WriteToMessageByte( hMessage, MUSICCMD_BREAK );
	g_pServerDE->WriteToMessageString( hMessage, pBreakName );
	g_pServerDE->EndMessage(hMessage);

	return DTRUE;
}
