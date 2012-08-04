// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdConsoleDriver_PunkBuster.cpp
//
// PURPOSE : PunkBuster interface to ScmdConsole.
//
// CREATED : 11/21/2005
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ScmdConsoleDriver_PunkBuster.h"
#include "GameServerShell.h"
#include "ipunkbuster.h"
#include "sys/win/mpstrconv.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleDriver_PunkBuster::SendToServer
//
//	PURPOSE:	Sends formed message to server.  
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleDriver_PunkBuster::SendToServer( ILTMessage_Read& msg )
{
	((IServerShell*)g_pGameServerShell)->OnMessage((HCLIENT)&ScmdConsole::Instance(), &msg);
	
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleDriver_PunkBuster::WriteMessage
//
//	PURPOSE:	Writes message to clients output window.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleDriver_PunkBuster::WriteMessage( wchar_t const* pszMessage )
{
	// Check inputs.
	if( !pszMessage || !pszMessage[0] )
	{
		ASSERT( !"ScmdConsoleDriver_PunkBuster::WriteMessage: Invalid inputs." );
		return false;
	}
	
	std::string strMessage = MPW2A(pszMessage).c_str();
	strMessage += '\n';
	g_pGameServerShell->GetPunkBusterServer()->CaptureConsoleOutput((char*)strMessage.c_str(), strMessage.length());

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleDriver_PunkBuster::LoadStringResId
//
//	PURPOSE:	Loads string resource.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

wchar_t const* ScmdConsoleDriver_PunkBuster::LoadStringResId( const char* szResId )
{
	return LoadString( szResId );
}
