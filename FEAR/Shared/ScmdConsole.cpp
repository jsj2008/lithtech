// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdConsole.cpp
//
// PURPOSE : Client side handling of SCMD commands.  Provides remote control of
//				server.
//
// CREATED : 10/22/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ScmdConsole.h"
#include "ScmdShared.h"
#include "ParsedMsg.h"
#include "DebugNew.h"
#include "MsgIDs.h"
#include "CommonUtilities.h"
#include "AutoMessage.h"
#include "NetDefs.h"
#include "GameModeMgr.h"

#include <vector>
#include <algorithm>

// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler
//
//	PURPOSE:	Defines interface for scmdconsole commands.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler
{
	public:
		
		// String token.
		virtual CParsedMsgW::CToken const& Token( ) = 0;

		// Enumeration.
		virtual ScmdCommand CommandId( ) = 0;

		// Help resid.
		virtual const char* HelpId( ) = 0;

		// Called to send command.
		virtual bool Send( wchar_t const* pszCommand );

		// Called when response of this command type is received.
		// Overrides return true if message is fully handled, otherwise return false
		// so default receipt behavior runs.
		virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};

// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_Help
//
//	PURPOSE:	Sends/receives help command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_Help : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"Help" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandHelp; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_HELP"; }
		virtual bool Send( wchar_t const* pszCommand );
		virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_Login
//
//	PURPOSE:	Sends/receives login command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_Login : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"Login" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandLogin; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_LOGIN"; }
		virtual bool Send( wchar_t const* pszCommand );
		virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_Logout
//
//	PURPOSE:	Sends/receives logout command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_Logout : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"Logout" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandLogout; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_LOGOUT"; }
		virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_ListClients
//
//	PURPOSE:	Sends/receives listclients command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_ListClients : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"ListClients" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandListClients; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_LISTCLIENTS"; }
		virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_NextMission
//
//	PURPOSE:	Sends/receives nextmission command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_NextMission : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"NextMap" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandNextMission; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_NEXTMISSION"; }
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_NextRound
//
//	PURPOSE:	Sends/receives nextround command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_NextRound: public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"NextRound" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandNextRound; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_NEXTROUND"; }
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_ListMissions
//
//	PURPOSE:	Sends/receives listmissions command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_ListMissions : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"ListMaps" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandListMissions; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_LISTMISSIONS"; }
		virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_SetMission
//
//	PURPOSE:	Sends/receives setmission command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_SetMission : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"SetMap" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandSetMission; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_SETMISSION"; }
		virtual bool Send( wchar_t const* pszCommand );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_BootName
//
//	PURPOSE:	Sends/receives bootname command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_BootName : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"BootName" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandBootName; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_BOOTNAME"; }
		virtual bool Send( wchar_t const* pszCommand );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_BootId
//
//	PURPOSE:	Sends/receives bootid command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_BootId : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"BootId" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandBootId; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_BOOTID"; }
		virtual bool Send( wchar_t const* pszCommand );
};

// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_BanUser
//
//	PURPOSE:	Sends/receives banuser command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_BanUser : public ScmdConsoleCommandHandler
{
public:

	virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"BanUser" ); return cTok; }
	virtual ScmdCommand CommandId( ) { return kScmdCommandBanUser; }
	virtual const char* HelpId( ) { return "IDS_SCMD_HELP_BANUSER"; }
	virtual bool Send( wchar_t const* pszCommand );
};

// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_UnbanUser
//
//	PURPOSE:	Sends/receives unbanuser command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_UnbanUser : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"UnbanUser" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandUnbanUser; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_UNBANUSER"; }
		virtual bool Send( wchar_t const* pszCommand );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_ListUserBans
//
//	PURPOSE:	Sends/receives listuserbans command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_ListUserBans : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"ListUserBans" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandListUserBans; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_LISTUSERBANS"; }
		virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_ListGameOptions
//
//	PURPOSE:	Sends/receives ListGameOptions command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_ListGameOptions : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"ListGameOptions" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandListGameOptions; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_LISTGAMEOPTIONS"; }
		virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_ShowGameOption
//
//	PURPOSE:	Sends/receives showgameoption command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_ShowGameOption : public ScmdConsoleCommandHandler
{
public:

	virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"ShowGameOption" ); return cTok; }
	virtual ScmdCommand CommandId( ) { return kScmdCommandShowGameOption; }
	virtual const char* HelpId( ) { return "IDS_SCMD_HELP_SHOWGAMEOPTION"; }
	virtual bool Send( wchar_t const* pszCommand );
	virtual bool Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsoleCommandHandler_SetGameOption
//
//	PURPOSE:	Sends/receives SetGameOption command.
//
// ----------------------------------------------------------------------- //

class ScmdConsoleCommandHandler_SetGameOption : public ScmdConsoleCommandHandler
{
	public:

		virtual CParsedMsgW::CToken const& Token( ) { static CParsedMsgW::CToken cTok( L"SetGameOption" ); return cTok; }
		virtual ScmdCommand CommandId( ) { return kScmdCommandSetGameOption; }
		virtual const char* HelpId( ) { return "IDS_SCMD_HELP_SETGAMEOPTION"; }
		virtual bool Send( wchar_t const* pszCommand );
};


// ----------------------------------------------------------------------- //
//
//	CLASS:		ScmdConsole_Impl
//
//	PURPOSE:	Provides implementation of ScmdConsole.
//
// ----------------------------------------------------------------------- //

class ScmdConsole_Impl : public ScmdConsole
{
	friend class ScmdConsole;

	protected:

		// Not allowed to create directly.  Use Instance().
		ScmdConsole_Impl( );

		// Copy ctor and assignment operator not implemented and should never be used.
		ScmdConsole_Impl( ScmdConsole_Impl const& other );
		ScmdConsole_Impl& operator=( ScmdConsole_Impl const& other );

	public:

		// This destructor should be private, but if it is, the compiler complains
		// that the Instance function does not have access to it.  Instance should
		// have access since it's a member function.  Compiler bug?
		virtual ~ScmdConsole_Impl();

	// Overrides of ScmdConsole interface.
	public:

		// Initializes the object.
		virtual bool Init( ScmdConsoleDriver& scmdClientDriver );

		// Terminates the object.
		virtual void Term( );

		// Called to handle messages.
		virtual bool OnMessage( uint8 nMessageId, ILTMessage_Read& msgRead );

		// Parses command string and sends SCMD command.
		virtual bool SendCommand( const wchar_t* pszCommand );

		// Sends parsed command.
		bool SendParsedCommand( CParsedMsgW& parsedMsg );

		// Accesses the scmdconsoledriver specified in init.
		ScmdConsoleDriver& GetScmdConsoleDriver( ) { return *m_pScmdConsoleDriver; }

	// Additional API.
	public:

		// Get the Implementation instance.
		static ScmdConsole_Impl& Instance_Impl( ) { return *(( ScmdConsole_Impl* )( &ScmdConsole::Instance( ))); }

		// Writes all the commands to the output.
		bool WriteAllCommands( );

		// Sends simple scmd command message w/o parameters.
		bool SendMessage( ScmdCommand eScmdCommand );

	protected:

		// Handles SCMD messages
		bool HandleScmdMessage( ILTMessage_Read& msg );

	protected:

		// Object has been initialized.
		bool m_bInitialized;

		// The scmdconsoledriver specified in Init.
		ScmdConsoleDriver* m_pScmdConsoleDriver;

		// Stores list of command handlers.
		typedef std::vector< 
					ScmdConsoleCommandHandler*, 
					LTAllocator<ScmdConsoleCommandHandler*, LT_MEM_TYPE_GAMECODE> 
				> ScmdConsoleCommandHandlerList;
		ScmdConsoleCommandHandlerList m_ScmdConsoleCommandHandlerList;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole::Instance()
//
//	PURPOSE:	Instatiator of singleton
//
//  This function is the only way to instatiate this object.  It
//  ensures that there is only one object, the singleton.
//
// ----------------------------------------------------------------------- //

ScmdConsole& ScmdConsole::Instance( )
{
	// Putting the singleton as a static function variable ensures that this
	// object is only created if it is used.
	static ScmdConsole_Impl sSingleton;
	return sSingleton;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::ScmdConsole_Impl
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ScmdConsole_Impl::ScmdConsole_Impl( )
{
	m_bInitialized = false;
	m_pScmdConsoleDriver = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::~ScmdConsole_Impl
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ScmdConsole_Impl::~ScmdConsole_Impl( )
{
	Term( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::Init
//
//	PURPOSE:	Initializes object.
//
// ----------------------------------------------------------------------- //

bool ScmdConsole_Impl::Init( ScmdConsoleDriver& scmdClientDriver )
{
	// Start fresh.
	Term( );

	m_bInitialized = true;
	m_pScmdConsoleDriver = &scmdClientDriver;

	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_Help ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_Login ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_Logout ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_ListClients ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_ListMissions ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_NextMission ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_NextRound ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_SetMission ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_BootName ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_BootId ));
#ifdef ENABLE_CDKEY_1_CHECK
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_BanUser ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_UnbanUser ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_ListUserBans ));
#endif
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_ListGameOptions ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_ShowGameOption ));
	m_ScmdConsoleCommandHandlerList.push_back( debug_new( ScmdConsoleCommandHandler_SetGameOption ));

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::Term
//
//	PURPOSE:	Terminates object.
//
// ----------------------------------------------------------------------- //

void ScmdConsole_Impl::Term( )
{
	// Remove the scmdcommandhandler's we added in init.
	ScmdConsoleCommandHandlerList::iterator iter = m_ScmdConsoleCommandHandlerList.begin( );
	while( iter != m_ScmdConsoleCommandHandlerList.end( ))
	{
		debug_delete( *iter );
		iter = m_ScmdConsoleCommandHandlerList.erase( iter );
	}

	m_bInitialized = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::OnMessage
//
//	PURPOSE:	Handles reading of SCMD message header.
//
// ----------------------------------------------------------------------- //

bool ScmdConsole_Impl::OnMessage( uint8 nMessageId, ILTMessage_Read& msgRead )
{
	// Make sure we're initialized.
	if( !m_bInitialized )
		return false;

	switch( nMessageId )
	{
		case MID_SCMD_RESPONSE:			
			HandleScmdMessage( msgRead );
			return true;

		default:
			break;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::SendCommand
//
//	PURPOSE:	Parses command string and sends SCMD command.
//
// ----------------------------------------------------------------------- //

bool ScmdConsole_Impl::SendCommand( const wchar_t* pszCommand )
{
	// Make sure we're initialized.
	if( !m_bInitialized )
		return false;

	// ConParse does not destroy the string, so this is safe.
	ConParseW parse;
	parse.Init(( wchar_t* )pszCommand );

	while( g_pCommonLT->Parse( &parse ) == LT_OK )
	{
		// Don't parse empty messages
		if ( !parse.m_nArgs || !parse.m_Args[0] )
			continue;

		CParsedMsgW cCurMsg( parse.m_nArgs, parse.m_Args );
		if( !SendParsedCommand( cCurMsg ))
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::SendParsedCommand
//
//	PURPOSE:	Sends parsed command message.
//
// ----------------------------------------------------------------------- //

bool ScmdConsole_Impl::SendParsedCommand( CParsedMsgW& parsedMsg )
{
	// Make sure we're initialized.
	if( !m_bInitialized )
		return false;

	static CParsedMsgW::CToken cTok_SCMD( L"SCMD" );

	// Check if this isn't a scmd command.
	if( parsedMsg.GetArg( 0 ) != cTok_SCMD )
		return false;

	// Write the command to the console.
	wchar_t szSentCommand[512];
	parsedMsg.ReCreateMsg( szSentCommand, ARRAY_LEN( szSentCommand ), 0 );
	GetScmdConsoleDriver( ).WriteMessage( szSentCommand );

	// Check if they didn't specify the command.
	if( parsedMsg.GetArgCount( ) < 2 )
	{
		WriteAllCommands( );
		return true;
	}

	// Provides space for recreating command message.
	wchar_t szCommand[512] = L"";

	// Get the command token to look for.
	CParsedMsgW::CToken const& commandTok = parsedMsg.GetArg( 1 );

	// Iterate over all the available commands looking for a matching token.
	// Once found, tell it to send the command.
	for( ScmdConsoleCommandHandlerList::iterator iter = m_ScmdConsoleCommandHandlerList.begin( );
		iter != m_ScmdConsoleCommandHandlerList.end( ); iter++ )
	{
		ScmdConsoleCommandHandler* pScmdConsoleCommandHandler = *iter;

		// Check if this command matches our token.
		if( pScmdConsoleCommandHandler->Token( ) == commandTok )
		{
			// Recreate the command message so the command can handle on its own.
			parsedMsg.ReCreateMsg( szCommand, ARRAY_LEN( szCommand ), 2 );
			if( !pScmdConsoleCommandHandler->Send( szCommand ))
			{
				// Remind them how to do this command.
				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( 
					pScmdConsoleCommandHandler->HelpId( ));
			}

			return true;
		}
	}

	// Command wasn't found.  Remind them of all the commands.
	WriteAllCommands( );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::HandleScmdMessage
//
//	PURPOSE:	Handles reading of SCMD command.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsole_Impl::HandleScmdMessage( ILTMessage_Read& msg )
{
	// Read the scmd command.
	ScmdCommand eScmdCommand = ( ScmdCommand )msg.Readuint8( );
	
	ScmdCommandStatus eScmdCommandStatus = ( ScmdCommandStatus )msg.Readuint8( );

	// Iterate over all the available commands looking for a matching command id.
	// Once found, tell it to receive the command.
	for( ScmdConsoleCommandHandlerList::iterator iter = m_ScmdConsoleCommandHandlerList.begin( );
		iter != m_ScmdConsoleCommandHandlerList.end( ); iter++ )
	{
		ScmdConsoleCommandHandler* pScmdConsoleCommandHandler = *iter;

		// Check if this commandhandler matches our command received.
		if( pScmdConsoleCommandHandler->CommandId( ) == eScmdCommand )
		{
			CLTMsgRef_Read cSubMsg( msg.SubMsg( msg.Tell( )));

			// If false return, then do some default behavior.
			pScmdConsoleCommandHandler->Receive( eScmdCommandStatus, *cSubMsg );

			return true;
		}
	}

	// Command id not found.
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::WriteAllCommands
//
//	PURPOSE:	Writes all commands to output.
//
// ----------------------------------------------------------------------- //

bool ScmdConsole_Impl::WriteAllCommands( )
{
	// Write out the header.
	GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_WRITEALLCOMMANDSHEADER" );

	// Iterate over all the available commands writing out their help strings.
	for( ScmdConsoleCommandHandlerList::iterator iter = m_ScmdConsoleCommandHandlerList.begin( );
		iter != m_ScmdConsoleCommandHandlerList.end( ); iter++ )
	{
		ScmdConsoleCommandHandler* pScmdConsoleCommandHandler = *iter;
		GetScmdConsoleDriver( ).WriteStringResId( pScmdConsoleCommandHandler->HelpId( ));
	}

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsole_Impl::SendMessage
//
//	PURPOSE:	Sends scmd command message w/o parameters.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsole_Impl::SendMessage( ScmdCommand eScmdCommand )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( eScmdCommand == ( uint8 )eScmdCommand );
	cMsg.Writeuint8(( uint8 )eScmdCommand );
	GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	LTUNREFERENCED_PARAMETER( msg );

	bool bReturn = false;

	// Handle the generic errors.
	switch( eScmdCommandStatus )
	{
		case kScmdCommandStatusOk:
		{
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_COMMANDOK" );
		}
		break;

		case kScmdCommandStatusFailed:
		{
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_COMMANDFAILED" );
			bReturn = true;
		}
		break;

		case kScmdCommandStatusNotLoggedIn:
		{
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_NOTLOGGEDIN" );
			bReturn = true;
		}
		break;

		default:
			break;
	}

	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).OnCommandStatusError( eScmdCommandStatus );

	return bReturn;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler::Send
//
//	PURPOSE:	Sends simple command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler::Send( wchar_t const* pszCommand )
{
	LTUNREFERENCED_PARAMETER( pszCommand );

	// Send the message.
	if( !ScmdConsole_Impl::Instance_Impl( ).SendMessage( CommandId( )))
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_Help::Send
//
//	PURPOSE:	Sends help command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_Help::Send( wchar_t const* pszCommand )
{
	LTUNREFERENCED_PARAMETER( pszCommand );

	// Write out all the commands.
	if( !ScmdConsole_Impl::Instance_Impl( ).WriteAllCommands( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_Help::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_Help::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	LTUNREFERENCED_PARAMETER( eScmdCommandStatus );
	LTUNREFERENCED_PARAMETER( msg );

	// Won't ever be called.  Help handled locally.

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_Login::Send
//
//	PURPOSE:	Sends login command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_Login::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));

	uint32 nHashedPassword = str_Hash( pszCommand );
	cMsg.Writeuint32( nHashedPassword );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_Login::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_Login::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	switch( eScmdCommandStatus )
	{
		case kScmdCommandStatusOk:
		{
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_LOGINOK" );
			return true;
		}
		break;

		case kScmdCommandStatusAdminAlreadyLoggedIn:
		{
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_ADMINLOGGEDIN" );
			return true;
		}
		break;

		case kScmdCommandStatusIncorrectPassword:
		{
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_INCORRECTPASSWORD" );
			return true;
		}
		break;

		default:
		{
			return ScmdConsoleCommandHandler::Receive( eScmdCommandStatus, msg );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_Logout::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_Logout::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	switch( eScmdCommandStatus )
	{
		case kScmdCommandStatusOk:
		{
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_LOGGEDOUTOK" );
			return true;
		}
		break;

		default:
		{
			return ScmdConsoleCommandHandler::Receive( eScmdCommandStatus, msg );
		}
		break;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_Clients::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

struct ScmdClientInfo
{
	uint16	m_nClientID;
	uint16	m_nPing;
	uint8	m_aIP[4];
	wchar_t	m_szName[MAX_PLAYER_NAME];
};

struct ScmdClientInfoSort
{
	bool operator()( const ScmdClientInfo& x, const ScmdClientInfo& y) const
	{
		return ( x.m_nClientID < y.m_nClientID );
	}
};

bool ScmdConsoleCommandHandler_ListClients::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	switch( eScmdCommandStatus )
	{
		case kScmdCommandStatusOk:
		{
			ScmdClientInfo scmdClientInfo;
			typedef std::vector< ScmdClientInfo, LTAllocator<ScmdClientInfo, LT_MEM_TYPE_GAMECODE> > ScmdClientInfoList;
			ScmdClientInfoList scmdClientInfoList;

			// Read in all the clients' info.
			for(;;)
			{
				scmdClientInfo.m_nClientID = msg.Readuint16( );

				// Check if we reached the end of the list.
				if( scmdClientInfo.m_nClientID == ( uint16 )-1 )
				{
					break;
				}

				scmdClientInfo.m_nPing = msg.Readuint16( );
				scmdClientInfo.m_aIP[0] = msg.Readuint8( );
				scmdClientInfo.m_aIP[1] = msg.Readuint8( );
				scmdClientInfo.m_aIP[2] = msg.Readuint8( );
				scmdClientInfo.m_aIP[3] = msg.Readuint8( );
				msg.ReadWString( scmdClientInfo.m_szName, ARRAY_LEN( scmdClientInfo.m_szName ));
				
				scmdClientInfoList.push_back( scmdClientInfo );
			}

			// Write out the format message.
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_LISTCLIENTSHEADER" );

			// Check if there are no clients in the list.
			if( scmdClientInfoList.empty( ))
			{
				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_NOCLIENTS" );
				return true;
			}

			// Sort the list by client ids.
			std::sort( scmdClientInfoList.begin( ), scmdClientInfoList.end( ), ScmdClientInfoSort( ));

			wchar_t szClientInfo[128] = L"";

			// Write out the info.
			for( ScmdClientInfoList::iterator iter = scmdClientInfoList.begin( );
				iter != scmdClientInfoList.end( ); iter++ )
			{
				ScmdClientInfo& clientInfo = *iter;
				
				// Format the info string.
				LTSNPrintF( szClientInfo, ARRAY_LEN( szClientInfo ), L"%d) [%s] [%d] [%d.%d.%d.%d]", 
					clientInfo.m_nClientID, clientInfo.m_szName, clientInfo.m_nPing, 
					clientInfo.m_aIP[0], clientInfo.m_aIP[1], clientInfo.m_aIP[2], clientInfo.m_aIP[3] );
				szClientInfo[ ARRAY_LEN( szClientInfo ) - 1 ] = 0;

				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szClientInfo );
			}

			return true;
		}
		break;

		default:
		{
			return ScmdConsoleCommandHandler::Receive( eScmdCommandStatus, msg );
		}
		break;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_ListMissions::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_ListMissions::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	switch( eScmdCommandStatus )
	{
		case kScmdCommandStatusOk:
		{
			uint8 nCurrentMission = msg.Readuint8( );
			uint8 nNumMissions = msg.Readuint8( );

			wchar_t szMission[128] = L"";
			wchar_t szOutput[128] = L"";

			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_LISTMISSIONSHEADER" );

			uint16 nMissionIndex = 0;
			for( int i = 0; i < nNumMissions; i++ )
			{
				msg.ReadWString( szMission, ARRAY_LEN( szMission ));

				// Write out the mission and indicate if it's the current mission.
				bool bIsCurrentMission = ( nMissionIndex == nCurrentMission );
				LTSNPrintF( szOutput, ARRAY_LEN( szOutput ), L"%d) [%s]%s", nMissionIndex, 
					szMission, (( bIsCurrentMission ) ? L" [*]" : L"" ));
				szOutput[ ARRAY_LEN( szOutput ) - 1 ] = 0;
				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szOutput );

				nMissionIndex++;
			}

			return true;
		}
		break;

		default:
		{
			return ScmdConsoleCommandHandler::Receive( eScmdCommandStatus, msg );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_SetMission::Send
//
//	PURPOSE:	Sends SetMission command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_SetMission::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	// Convert to a number.
	uint8 nMissionIndex = ( uint8 )LTStrToLong( pszCommand );
	
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));
	cMsg.Writeuint8( nMissionIndex );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_BootName::Send
//
//	PURPOSE:	Sends bootname command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_BootName::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));
	cMsg.WriteWString( pszCommand );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_BootId::Send
//
//	PURPOSE:	Sends bootid command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_BootId::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	// Convert to a number.
	uint16 nClientId = ( uint16 )LTStrToLong( pszCommand );
	
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));
	cMsg.Writeuint16( nClientId );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}

/*// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_AddBan::Send
//
//	PURPOSE:	Sends addban command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_AddBan::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));
	cMsg.WriteWString( pszCommand );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_BanUser::Send
//
//	PURPOSE:	Sends banuser command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_BanUser::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	// Convert to a number.
	//uint16 nClientId = ( uint16 )_wtoi( pszCommand );
	uint16 nClientId = ( uint16 )LTStrToLong( pszCommand );

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));
	cMsg.Writeuint16( nClientId );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_UnbanUser::Send
//
//	PURPOSE:	Sends unbanuser command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_UnbanUser::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	// Convert to a number.
	uint8 nBanId = ( uint8 )LTStrToLong( pszCommand );	

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));
	cMsg.Writeuint8( nBanId );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_ListUserBans::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_ListUserBans::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	switch( eScmdCommandStatus )
	{
		case kScmdCommandStatusOk:
		{
			uint16 nNumBans = msg.Readuint16( );

			// Check if there are no clients in the list.
			if( !nNumBans )
			{
				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_NOUSERBANS" );
				return true;
			}

			// Write out the format message.
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_LISTUSERBANSHEADER" );

			char szBannedIP[16] = "";
			wchar_t szMessage[128] = L"";

			// Write out the info.
			for( int i = 0; i < nNumBans; i++ )
			{
				msg.ReadString( szBannedIP, ARRAY_LEN( szBannedIP ));
				LTSNPrintF( szMessage, LTARRAYSIZE(szMessage), L"%d) [%ls]", i, szBannedIP );
				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szMessage );
			}

			return true;
		}
		break;

		default:
		{
			return ScmdConsoleCommandHandler::Receive( eScmdCommandStatus, msg );
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	WriteOutGameOption
//
//	PURPOSE:	Writes out game option.
//
//	ARGUMENTS:	nIndex - The index to write into output.
//				pszValueFormat - the printf format specifier to use when writing value out.
//				nLabelId - the CRES string id for the label.
//				value - the value to write into output.
//
//	RETURN:		true if successful.
//
// ----------------------------------------------------------------------- //

template< class T >
static bool WriteOutGameOption( uint32 nIndex, wchar_t const* pszValueFormat, const char* szLabelId, T value )
{
	if( !pszValueFormat )
		return false;

	wchar_t szFormat[256] = L"";
	wchar_t szOut[256] = L"";

	// Create the format string using the passed in format for the value.
	LTSNPrintF( szFormat, LTARRAYSIZE(szFormat), L"%%d) [%%s][%s]", pszValueFormat );
	wchar_t const* pszLabel = ScmdConsole::Instance( ).GetScmdConsoleDriver( ).LoadStringResId( szLabelId );
	LTSNPrintF( szOut, LTARRAYSIZE(szOut), szFormat, nIndex, pszLabel, value );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szOut );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_ListGameOptions::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_ListGameOptions::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	switch( eScmdCommandStatus )
	{
		case kScmdCommandStatusOk:
		{
			// Write the header out.
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteStringResId( "IDS_SCMD_LISTGAMEOPTIONSHEADER" );

			// Iterate through the list of gamerules.  Only count them if they were shown on the scmd list.
			uint32 nIndex = 0;
			wchar_t szValue[256] = L"";
			wchar_t szOut[256] = L"";

			for( GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( ); 
				iter != GameRule::GetGameRuleList().end( ); iter++ )
			{
				GameRule* pGameRule = *iter;

				// Skip rules not shown on the list.
				if( !pGameRule->IsCanModify( ) || !pGameRule->IsShowInOptions( ) || !pGameRule->IsCanModifyAtRuntime( ))
					continue;

				// Create the format string using the passed in format for the value.
				char const* pszLabelId = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( pGameRule->GetStruct( ), 0, "Label" ), 0, "" );

				GameRuleEnum* pGameRuleEnum = NULL;
				GameRuleBool* pGameRuleBool = NULL;

				// Enum's need special case handling, since the need to show their range of values.
				if( pGameRuleEnum = dynamic_cast< GameRuleEnum* >( pGameRule ))
				{
					uint32 nValueIndex = pGameRuleEnum->GetRawValueToIndex( *pGameRuleEnum );
					pGameRuleEnum->ToString( szValue, LTARRAYSIZE( szValue ), true );
					LTSNPrintF( szOut, LTARRAYSIZE(szOut), L"%d) [%s][%d:%s]", nIndex, LoadString( pszLabelId ), nValueIndex, szValue );
				}
				else if( pGameRuleBool = dynamic_cast< GameRuleBool* >( pGameRule ))
				{
					pGameRuleBool->ToString( szValue, LTARRAYSIZE( szValue ), true );
					LTSNPrintF( szOut, LTARRAYSIZE(szOut), L"%d) [%s][%d:%s]", nIndex, LoadString( pszLabelId ), 
						*pGameRuleBool ? 1 : 0, szValue );
				}
				else
				{
					pGameRule->ToString( szValue, LTARRAYSIZE( szValue ), true );
					LTSNPrintF( szOut, LTARRAYSIZE(szOut), L"%d) [%s][%s]", nIndex, LoadString( pszLabelId ), szValue );
				}

				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szOut );

				nIndex++;
			}

			return true;
		}
		break;

		default:
		{
			return ScmdConsoleCommandHandler::Receive( eScmdCommandStatus, msg );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_ShowGameOption::Send
//
//	PURPOSE:	Sends showgameoption command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_ShowGameOption::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	// Parse the message.
	ConParseW parse;
	parse.Init( pszCommand );
	g_pCommonLT->Parse( &parse );
	if( parse.m_nArgs < 1 )
		return false;

	CParsedMsgW cCurMsg( parse.m_nArgs, parse.m_Args );

	// Convert the first parameter to the game option index.
	uint8 nGameOption = ( uint8 )LTStrToLong( cCurMsg.GetArg( 0 )); 

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));
	cMsg.Writeuint8( nGameOption );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_ShowGameOption::Receive
//
//	PURPOSE:	Handles receiving response.
//
//	RETURN:		true if handled.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_ShowGameOption::Receive( ScmdCommandStatus eScmdCommandStatus, ILTMessage_Read& msg )
{
	switch( eScmdCommandStatus )
	{
	case kScmdCommandStatusOk:
		{
			// Get the game option they are setting.
			uint8 nGameOption = msg.Readuint8( );

			// Iterate through the list of gamerules.  Only count them if they were shown on the scmd list.
			uint32 nIndex = 0;
			wchar_t szValue[256] = L"";
			wchar_t szOut[256] = L"";

			GameRule* pFoundGameRule = NULL;

			for( GameRule::GameRuleList::iterator iter = GameRule::GetGameRuleList().begin( ); 
				iter != GameRule::GetGameRuleList().end( ); iter++ )
			{
				GameRule* pGameRule = *iter;

				// Skip rules not shown on the list.
				if( !pGameRule->IsCanModify( ) || !pGameRule->IsShowInOptions( ) || !pGameRule->IsCanModifyAtRuntime( ))
					continue;

				// See if this is the option to show.
				if( nIndex == nGameOption )
				{
					pFoundGameRule = pGameRule;
					break;
				}

				nIndex++;
			}

			// Check if we didn't have a rule that matched in the input.
			if( !pFoundGameRule )
				return true;

			// Write out the header.
			char const* pszLabelId = g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( pFoundGameRule->GetStruct( ), 0, "Label" ), 0, "" );
			LTSNPrintF( szOut, LTARRAYSIZE(szOut), L"%d) [%s]", nGameOption, LoadString( pszLabelId ));
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szOut );

			// Write out the help.
			ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( LoadString( 
				g_pLTDatabase->GetString( CGameDatabaseReader::GetStructAttribute( 
				pFoundGameRule->GetStruct( ), 0, "Help" ), 0, "" )));

			GameRuleEnum* pGameRuleEnum = NULL;
			GameRuleBool* pGameRuleBool = NULL;

			// Enum's need special case handling, since the need to show their range of values.
			if( pGameRuleEnum = dynamic_cast< GameRuleEnum* >( pFoundGameRule ))
			{
				// Get the struct of values.
				HATTRIBUTE hStruct = pGameRuleEnum->GetStruct( );
				HATTRIBUTE hValues = CGameDatabaseReader::GetStructAttribute( hStruct, 0, "Values" );
				if( !hValues )
				{
					return true;
				}

				// Get the current index value.
				uint32 nCurValueIndex = pGameRuleEnum->GetRawValueToIndex( *pGameRuleEnum );

				// Iterate through the values and write out each one.  Put a * by
				// the current value.
				uint32 nNumValues = g_pLTDatabase->GetNumValues( hValues );
				for( uint32 nIndex = 0; nIndex < nNumValues; nIndex++ )
				{
					HATTRIBUTE hString = CGameDatabaseReader::GetStructAttribute( hValues, nIndex, "String" );
					LTSNPrintF( szOut, LTARRAYSIZE( szOut ), L"  [%d:%s]%s", nIndex,
						LoadString( g_pLTDatabase->GetString( hString, 0, "" )),
						( nIndex == nCurValueIndex ) ? L"*" : L"" );
					ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szOut );
				}
			}
			else if( pGameRuleBool = dynamic_cast< GameRuleBool* >( pFoundGameRule ))
			{
				pGameRuleBool->ToString( szValue, LTARRAYSIZE( szValue ), true );
				LTSNPrintF( szOut, LTARRAYSIZE(szOut), L"  [%d:%s]", *pGameRuleBool ? 1 : 0, szValue );
				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szOut );
			}
			else
			{
				// Create the format string using the passed in format for the value.
				pFoundGameRule->ToString( szValue, LTARRAYSIZE( szValue ), true );
				LTSNPrintF( szOut, LTARRAYSIZE(szOut), L"  [%s]", szValue );
				ScmdConsole::Instance( ).GetScmdConsoleDriver( ).WriteMessage( szOut );
			}
			
			return true;
		}
		break;

	default:
		{
			return ScmdConsoleCommandHandler::Receive( eScmdCommandStatus, msg );
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ScmdConsoleCommandHandler_SetGameOption::Send
//
//	PURPOSE:	Sends setgameoption command.
//
//	RETURN:		false on error.
//
// ----------------------------------------------------------------------- //

bool ScmdConsoleCommandHandler_SetGameOption::Send( wchar_t const* pszCommand )
{
	// Check inputs.
	if( !pszCommand || !pszCommand[0] )
	{
		return false;
	}

	// Parse the message.
	ConParseW parse;
	parse.Init( pszCommand );
	g_pCommonLT->Parse( &parse );
	if( parse.m_nArgs < 2 )
		return false;

	CParsedMsgW cCurMsg( parse.m_nArgs, parse.m_Args );

	// Convert the first parameter to the game option index.
	uint8 nGameOption = ( uint8 )LTStrToLong( cCurMsg.GetArg( 0 )); 

	// Convert the rest to the value.
	wchar_t szVal[256];
	cCurMsg.ReCreateMsg( szVal, ARRAY_LEN( szVal ), 1 );

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SCMD_COMMAND );
	ASSERT( CommandId( ) == ( uint8 )CommandId( ));
	cMsg.Writeuint8(( uint8 )CommandId( ));
	cMsg.Writeuint8( nGameOption );
	cMsg.WriteWString( szVal );
	ScmdConsole::Instance( ).GetScmdConsoleDriver( ).SendToServer( *cMsg.Read( ));

	return true;
}

