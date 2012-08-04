// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdConsoleDriver_CShell.h
//
// PURPOSE : CShell driver for scmdclient
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
//	PURPOSE:	Provides specialized ScmdConsoleDriver for Cshell.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleDriver_CShell : public ScmdConsoleDriver
{
	// Override to provide specialized IO with application.
	public:

		// Sends formed message to server.
		virtual bool SendToServer( ILTMessage_Read& msg );

		// Writes message to clients output window.
		virtual bool WriteMessage( char const* pszMessge );

		// Loads string resource.
		virtual char const* LoadStringResId( uint32 nResId );
};



#endif // SCMDCONSOLEDRIVER_CSHELL_H