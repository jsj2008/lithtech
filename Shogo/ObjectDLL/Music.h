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


class CPlayerObj;
class CServerDE;

// Class Definition...

class CMusic
{
	// Member functions...

	public :

		CMusic( );
		~CMusic( );

		DBOOL						Init( CPlayerObj* pMyObj );

		DBOOL						HandleMusicMessage( char *pMsg );

		DBOOL						SetMusicLevel( DBYTE nLevel );
		DBOOL						PlayMotif( char *pMotifName, DBOOL bLoop );
		DBOOL						PlayBreak( char *pBreakName );

	// Member variables...

	private:

		CPlayerObj *				m_pMyObj;		// Object I'm associated with

};

inline DBOOL CMusic::Init( CPlayerObj* pMyObj )
{
	m_pMyObj = pMyObj;
	return DTRUE;
}

#endif // __MUSIC_H__