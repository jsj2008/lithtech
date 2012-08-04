// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdConsoleDriver_ServerApp.h
//
// PURPOSE : ServerApp driver for scmdconsole
//
// CREATED : 10/24/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef ScmdConsoleDriver_ServerApp_H
#define ScmdConsoleDriver_ServerApp_H

#include "ScmdConsole.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleDriver_ServerApp
//
//	PURPOSE:	Provides specialized ScmdConsoleDriver for serverapp.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleDriver_ServerApp : public ScmdConsoleDriver
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



#endif // ScmdConsoleDriver_ServerApp_H