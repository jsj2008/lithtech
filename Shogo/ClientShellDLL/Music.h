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

#include "ltbasedefs.h"
#include "clientheaders.h"


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

		LTBOOL						Init( ILTClient *pClientDE, LTBOOL bUseIma );
		void						Term( );
		LTBOOL						IsInitialized( ) { return ( m_pClientDE != NULL ); }

		LTBOOL						InitPlayLists( );
		LTBOOL						TermPlayLists( );
		LTBOOL						IsPlaylistInitialized( ) { return m_bPlayListInitialized; }

		LTBOOL						HandleMusicMessage( ILTMessage_Read* hMessage );

		void						SetMusicLevel( EMusicLevel eLevel ) { m_eMusicLevel = eLevel; }
		EMusicLevel					GetMusicLevel( ) { return m_eMusicLevel; }

		LTBOOL						PlayMusicLevel( EMusicLevel nLevel );
		LTBOOL						PlayMotif( char *pMotifName, LTBOOL bLoop );
		void						StopMotif( char *pMotifName );
		LTBOOL						PlayBreak( char *pBreakName );

		LTBOOL						PlayCDList( ) { return ( LTBOOL )m_pClientDE->PlayList( "CDTrack", LTNULL, LTTRUE, MUSIC_IMMEDIATE ); }

		LTBOOL						UsingIMA( ) { return m_bUseIma; }

	// Member variables...

	private:

		LTBOOL						InitTrans( char szTransitions[4][PARSE_MAXTOKENSIZE] );
		LTBOOL						InitList( char *szWorldProp,
										char argBuffer[PARSE_MAXTOKENS*(PARSE_MAXTOKENSIZE+1)], char * argPointers[PARSE_MAXTOKENS] );
		LTBOOL						TransitionToStop( char *szTransition );
		LTBOOL						TransitionToLevel( char *szTransition, char *szList, EMusicLevel eLevel, LTBOOL bImmediate );

		ILTClient *					m_pClientDE;
		LTBOOL						m_bUseIma;

		EMusicLevel					m_eMusicLevel;
		LTBOOL						m_bPlayListInitialized;
};

#endif // __MUSIC_H__