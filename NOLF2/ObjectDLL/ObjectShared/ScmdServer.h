// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdServer.h
//
// PURPOSE : Server side handling of SCMD commands.  Provides remote control of
//				server.
//
// CREATED : 10/21/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef SCMDSERVER_H
#define SCMDSERVER_H

#include "iltmessage.h"
#include "ltbasedefs.h"

class ScmdServer
{
	protected:

		// Not allowed to create directly.  Use Instance().
		ScmdServer( ) { };

		// Copy ctor and assignment operator not implemented and should never be used.
		ScmdServer( ScmdServer const& other );
		ScmdServer& operator=( ScmdServer const& other );

	public:

		// This destructor should be private, but if it is, the compiler complains
		// that the Instance function does not have access to it.  Instance should
		// have access since it's a member function.  Compiler bug?
		virtual ~ScmdServer() { };

	public:

		// Call this to get the singleton instance of the weapon mgr.
		static ScmdServer& Instance( );

		// Initializes the object.
		virtual bool Init( ) = 0;

		// Terminates the object.
		virtual void Term( ) = 0;

		// Called to handle messages.
		virtual bool OnMessage( HCLIENT hClient, ILTMessage_Read& msgRead ) = 0;

		// Called when client drops.
		virtual bool OnRemoveClient( HCLIENT hClient ) = 0;

		enum AdminControl
		{
			kAdminControlNone,
			kAdminControlClient,
			kAdminControlServerapp,
		};

		// Gets the current admin control state.
		virtual AdminControl GetAdminControl( ) = 0;

		// Get the HCLIENT logged in as admin, NULL for serverapp.
		virtual HCLIENT GetAdminClient( ) = 0;
};

#endif // SCMDSERVER_H