// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdConsoleDriver_PunkBuster.h
//
// PURPOSE : PunkBuster interface to ScmdConsole.
//
// CREATED : 11/21/2005
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef SCMDCONSOLEDRIVER_PUNKBUSTER_H
#define SCMDCONSOLEDRIVER_PUNKBUSTER_H

#include "ScmdConsole.h"

class ScmdConsoleDriver_PunkBuster : public ScmdConsoleDriver
{
public:

	// Sends formed message to server.
	virtual bool SendToServer( ILTMessage_Read& msg );

	// Writes message to clients output window.
	virtual bool WriteMessage( wchar_t const* pszMessge );

	// Loads string resource.
	virtual wchar_t const* LoadStringResId( const char* szResId );
};



#endif // SCMDCONSOLEDRIVER_PUNKBUSTER_H

