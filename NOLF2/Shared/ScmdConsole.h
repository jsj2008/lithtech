// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdConsole.h
//
// PURPOSE : Client side handling of SCMD commands.  Provides remote control of
//				server.
//
// CREATED : 10/22/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef SCMDCLIENT_H
#define SCMDCLIENT_H

#include "iltmessage.h"

class CParsedMsg;

// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleDriver
//
//	PURPOSE:	Interface for io with client application, like cshell or serverapp.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleDriver
{
	// Override to provide specialized IO with application.
	public:

		// Sends formed message to server.
		virtual bool SendToServer( ILTMessage_Read& msg ) = 0;

		// Writes message to clients output window.
		virtual bool WriteMessage( char const* pszMessge ) = 0;

		// Loads string resource.
		virtual char const* LoadStringResId( uint32 nResId ) = 0;

	// Helper functions.
	public:

		// Loads then writes a string res id.
		bool WriteStringResId( uint32 nResId )
		{
			char const* pszMessage = LoadStringResId( nResId );
			if( !pszMessage || !pszMessage[0] )
				return false;

			return WriteMessage( pszMessage );
		}
};


class ScmdConsole
{
	protected:

		// Not allowed to create directly.  Use Instance().
		ScmdConsole( ) { };

		// Copy ctor and assignment operator not implemented and should never be used.
		ScmdConsole( ScmdConsole const& other );
		ScmdConsole& operator=( ScmdConsole const& other );

	public:

		// This destructor should be private, but if it is, the compiler complains
		// that the Instance function does not have access to it.  Instance should
		// have access since it's a member function.  Compiler bug?
		virtual ~ScmdConsole() { };

	public:

		// Call this to get the singleton instance of the weapon mgr.
		static ScmdConsole& Instance( );

		// Initializes the object.
		virtual bool Init( ScmdConsoleDriver& scmdConsoleDriver ) = 0;

		// Terminates the object.
		virtual void Term( ) = 0;

		// Called to handle messages.
		virtual bool OnMessage( uint8 nMessageId, ILTMessage_Read& msgRead ) = 0;

		// Parses command string and sends SCMD command.
		virtual bool SendCommand( const char* pszCommand ) = 0;

		// Sends pre-parsed SCMD command.
		virtual bool SendParsedCommand( CParsedMsg& parsedMsg ) = 0;

		// Accesses the scmdclientdriver specified in init.
		virtual ScmdConsoleDriver& GetScmdConsoleDriver( ) = 0;
};

#endif // SCMDCLIENT_H