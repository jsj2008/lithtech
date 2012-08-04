// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdConsoleDriver_CShell.h
//
// PURPOSE : GameClient.dll driver for scmdclient
//
// CREATED : 10/24/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef SCMDCONSOLEDRIVER_CSHELL_H
#define SCMDCONSOLEDRIVER_CSHELL_H

#include "ScmdConsole.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleDriver_CShell
//
//	PURPOSE:	Provides specialized ScmdConsoleDriver for GameClient.dll.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleDriver_CShell : public ScmdConsoleDriver
{
	// Override to provide specialized IO with application.
	public:

		// Sends formed message to server.
		virtual bool SendToServer( ILTMessage_Read& msg );

		// Writes message to clients output window.
		virtual bool WriteMessage( wchar_t const* pszMessge );

		// Loads string resource.
		virtual wchar_t const* LoadStringResId( const char* szResId );
};



#endif // SCMDCONSOLEDRIVER_CSHELL_H