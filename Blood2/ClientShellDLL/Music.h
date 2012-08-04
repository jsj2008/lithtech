// ----------------------------------------------------------------------- //
//
// MODULE  : Music.h
//
// PURPOSE : Music helper class.  Handles all music commands.
//
// CREATED : 12/18/97
//
// ----------------------------------------------------------------------- //

#ifndef __MUSIC_H__
#define __MUSIC_H__

#include "basedefs_de.h"
#include "cpp_client_de.h"


class CPlayerObj;

// Class Definition...

class CMusic
{
	public:

	enum EMusicLevel
	{ 
		MUSICLEVEL_SILENCE = 0,
		MUSICLEVEL_AMBIENT,
		MUSICLEVEL_CRUISING,
		MUSICLEVEL_HARDDRIVING
	};

	// Member functions...

	public :

		CMusic( );
		~CMusic( ) { Term( ); }

		DBOOL						Init( CClientDE *pClientDE, DBOOL bUseIma );
		void						Term( );
		DBOOL						IsInitialized( ) { return ( m_pClientDE != NULL ); }

		DBOOL						InitPlayLists( );
		DBOOL						TermPlayLists( );
		DBOOL						IsPlaylistInitialized( ) { return m_bPlayListInitialized; }

		DBOOL						HandleMusicMessage( HMESSAGEREAD hMessage );

		void						SetMusicLevel( EMusicLevel eLevel ) { m_eMusicLevel = eLevel; }
		EMusicLevel					GetMusicLevel( ) { return m_eMusicLevel; }

		DBOOL						PlayMusicLevel( EMusicLevel nLevel );
		DBOOL						PlayMotif( char *pMotifName, DBOOL bLoop );
		void						StopMotif( char *pMotifName );
		DBOOL						PlayBreak( char *pBreakName );

		DBOOL						PlayCDList( ) { return ( DBOOL )m_pClientDE->PlayList( "CDTrack", DNULL, DTRUE, MUSIC_IMMEDIATE ); }

		DBOOL						UsingIMA( ) { return m_bUseIma; }

	// Member variables...

	private:

		DBOOL						InitTrans( char szTransitions[4][PARSE_MAXTOKENSIZE] );
		DBOOL						InitList( char *szWorldProp,
										char argBuffer[PARSE_MAXTOKENS*(PARSE_MAXTOKENSIZE+1)], char * argPointers[PARSE_MAXTOKENS] );
		DBOOL						TransitionToStop( char *szTransition );
		DBOOL						TransitionToLevel( char *szTransition, char *szList, EMusicLevel eLevel, DBOOL bImmediate );

		CClientDE *					m_pClientDE;
		DBOOL						m_bUseIma;

		EMusicLevel					m_eMusicLevel;
		DBOOL						m_bPlayListInitialized;
};

#endif // __MUSIC_H__