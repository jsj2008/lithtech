// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdConsoleDriver_ServerApp.cpp
//
// PURPOSE : ServerApp driver for scmdconsole
//
// CREATED : 10/24/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScmdConsoleDriver_ServerApp.h"
#include "ServerDlg.h"
#include "CommonUtilities.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleDriver_ServerApp::SendToServer
//
//	PURPOSE:	Sends formed message to server.  
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleDriver_ServerApp::SendToServer( ILTMessage_Read& msg )
{
	if( g_pDialog->GetServer( )->SendToServerShell( msg ) != LT_OK )
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleDriver_ServerApp::WriteMessage
//
//	PURPOSE:	Writes message to clients output window.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleDriver_ServerApp::WriteMessage( char const* pszMessage )
{
	// Check inputs.
	if( !pszMessage || !pszMessage[0] )
	{
		ASSERT( !"ScmdConsoleDriver_ServerApp::WriteMessage: Invalid inputs." );
		return false;
	}

	g_pDialog->WriteConsoleString( pszMessage );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleDriver_ServerApp::LoadStringResId
//
//	PURPOSE:	Loads string resource.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

char const* ScmdConsoleDriver_ServerApp::LoadStringResId( uint32 nResId )
{
	static char szStringBuffer[512] = "";

	::LoadString( ::GetModuleHandle( NULL ), nResId, szStringBuffer, ARRAY_LEN( szStringBuffer ));

	return szStringBuffer;
}
