// ----------------------------------------------------------------------- //
//
// MODULE  : CommandObject.cpp
//
// PURPOSE : The CommandObject implementation
//
// CREATED : 10/11/01
//
// (c) 2001-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "CommandObject.h"
	#include <algorithm>


LINKFROM_MODULE( CommandObject );

//
// Defines...
//

	#define EVNTED_BASE_STRING	"Event"

	#define ADD_COMMAND_EDITOR_TRACK( base_string, track, flags ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E0, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E1, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E2, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E3, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E4, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E5, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E6, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E7, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E8, "", flags, "Internal command field" ) \
				ADD_COMMANDPROP_FLAG( base_string##T##track##E9, "", flags, "Internal command field" )

	#define ADD_EVENT_EDITOR_INFO( tracks, objects_per_track, base_string, flags ) \
				ADD_STRINGPROP_FLAG( EventDialogInfo, #tracks" "#objects_per_track" "#base_string, flags, "Properties for a single event track" ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 0, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 1, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 2, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 3, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 4, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 5, PF_NOTIFYCHANGE | flags )



BEGIN_CLASS( CommandObject )

	PROP_DEFINEGROUP( EventCommands, PF_GROUP(1) | PF_EVENT, "This brings up the Command Manager dialog box for easy creation and editing of event commands." )
		ADD_EVENT_EDITOR_INFO( 6, 10, Event, PF_GROUP(1) )
	
	ADD_COMMANDPROP_FLAG( FinishedCommand, "", PF_NOTIFYCHANGE, "A command that will be executed when the CommandObject is done sending event commands or is turned off through a message." )
	ADD_LONGINTPROP_FLAG( NumberOfActivations, -1, 0, "This specifies how many times the CommandObject may be activated.  Once the CommandObject has been activated NumberOfActivations times it gets removed and can no longer be activated.  Specify -1 for infinite activations." )
	ADD_BOOLPROP_FLAG( Locked, false, 0, "If true the CommandObject will be locked and unactivatable until unlocked." )

END_CLASS_FLAGS_PLUGIN( CommandObject, GameBase, 0, CCommandObjectPlugin, "The CommandObject is a multitrack command manager that allows for the manipulation and execution of multiple commands on a timeline." )


CMDMGR_BEGIN_REGISTER_CLASS( CommandObject )

	ADD_MESSAGE( ON,		1,	NULL,	MSG_HANDLER( CommandObject, HandleOnMsg ),		"ON", "Tells the CommandObject to begin running its time line and sending its commands", "msg CommandObject ON" )
	ADD_MESSAGE( OFF,		1,	NULL,	MSG_HANDLER( CommandObject, HandleOffMsg ),		"OFF", "Tells the CommandObject to stop running its time line and sends no more commands", "msg CommandObject OFF" )
	ADD_MESSAGE( PAUSE,		1,	NULL,	MSG_HANDLER( CommandObject, HandlePauseMsg ),	"PAUSE", "Tells the CommandObject to pause its time and halt sending the commands until it is resumed.", "msg CommandObject PAUSE" )
	ADD_MESSAGE( RESUME,	1,	NULL,	MSG_HANDLER( CommandObject, HandleResumeMsg ),	"RESUME", "Tells the CommandObject to resume its time from a pause and continue sending its commands.", "msg CommandObject RESUME" )
	ADD_MESSAGE( LOCK,		1,	NULL,	MSG_HANDLER( CommandObject, HandleLockMsg ),	"LOCK", "Locks the CommandObject so that it ignores all commands other than UNLOCK and REMOVE", "msg CommandObject LOCK" )
	ADD_MESSAGE( UNLOCK,	1,	NULL,	MSG_HANDLER( CommandObject, HandleUnlockMsg ),	"UNLOCK", "unlocks the CommandObject", "msg CommandObject UNLOCK" )

CMDMGR_END_REGISTER_CLASS( CommandObject, GameBase )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCommandObjectPlugin::PreHook_PropChanged
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
LTRESULT CCommandObjectPlugin::PreHook_PropChanged( const	char		*szObjName,
												    const	char		*szPropName,
												    const	int			nPropType,
												    const	GenericProp	&gpPropValue,
												    ILTPreInterface	*pInterface,
													const	char		*szModifiers )
{
	if( LTStrIEquals( szPropName, "FinishedCommand" ))
	{
		if( LT_OK == m_CmdMgrPlugin.PreHook_PropChanged( szObjName,
														 szPropName,
														 nPropType, 
														 gpPropValue,
														 pInterface,
														 szModifiers ))
		{
			return LT_OK;
		}
	}	
	else if( nPropType == LT_PT_COMMAND && gpPropValue.GetCommand()[0] )
	{
		//we need to skip over the time portion of the command string, which is
		//of the form 'time|command', so scan through for a pipe
		const char* pszCommand = gpPropValue.GetCommand();

		//run through the command and see if we can find a pipe, if so, we need to set the
		//command up so that it starts after the pipe
		const char* pszFindPipe = pszCommand;
		while(*pszFindPipe != '\0')
		{
			//see if we hit the pipe
			if(*pszFindPipe == '|')
			{
				//we did, so we can update where the command is located and stop looking
				pszCommand = pszFindPipe + 1;
				break;
			}
			pszFindPipe++;
		}

		GenericProp gp(pszCommand, LT_PT_COMMAND);
		
		if( LT_OK == m_CmdMgrPlugin.PreHook_PropChanged( szObjName,
														 szPropName,
														 nPropType, 
														 gp,
														 pInterface,
														 szModifiers ))
		{
			return LT_OK;
		}
	}	

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::CommandObject
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CommandObject::CommandObject()
:	GameBase			( OT_NORMAL ),
	m_fTotalTime		( 0.0f ),
	m_sFinishedCmd		( ),
	m_bLocked			( false ),
	m_nNumActivations	( -1 ),
	m_nNumTimesActivated( 0 )
{

}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::~CommandObject
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CommandObject::~CommandObject()
{
	EVENT_CMD_VECTOR::iterator iter;
	EVENT_CMD_STRUCT	*pEvntCmdStruct = NULL;

	// Free the allocated list of Event commands...

	for( iter = m_vecEventCmds.begin(); iter != m_vecEventCmds.end(); ++iter )
	{
		pEvntCmdStruct = *iter;
		debug_delete( pEvntCmdStruct );
	}

	m_vecEventCmds.clear();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 CommandObject::EngineMessageFn( uint32 messageID, void *pvData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE )
			{
				ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pvData;
				ReadProp( &pOCS->m_cProperties );

				// Ensure the object will never be sent to the client.
				if( pOCS )
					pOCS->m_Flags |= FLAG_NOTINWORLDTREE;
			}
		}
		break;

		case MID_OBJECTCREATED:
		{
			// Start off with no updates...
			if( fData != OBJECTCREATED_SAVEGAME )
			{
				m_Timer.SetEngineTimer( SimulationTimer::Instance());
				SetNextUpdate( UPDATE_NEVER );
			}
		}
		break;

		case MID_UPDATE:
		{
			SetNextUpdate( UPDATE_NEXT_FRAME );
			Update( );
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save( (ILTMessage_Write*)pvData );
		}
		break;

		case MID_LOADOBJECT:
		{
			Load( (ILTMessage_Read*)pvData );
		}
		break;
	};

	return GameBase::EngineMessageFn( messageID, pvData, fData );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::ReadProps
//
//  PURPOSE:	Read the property values...
//
// ----------------------------------------------------------------------- //

bool CommandObject::ReadProp( const GenericPropList *pProps )
{
	LTASSERT( pProps != NULL, "Invalid Object Create Struct" );
	
	char szPropName[32] = {0};

	// Make sure our vector has enough space to hold the max commmands...

	m_vecEventCmds.reserve( EVNTED_MAX_TRACKS );

	// Loop over every property string to see if there is a command to process...

	std::string	sEventString;
	std::string	sTimeStamp;

	for( int nTrack = 0; nTrack < EVNTED_MAX_TRACKS; ++nTrack )
	{
		for( int nObj = 0; nObj < EVNTED_MAX_OBJ_PER_TRACK; ++nObj )
		{
			LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "%sT%iE%i", EVNTED_BASE_STRING, nTrack, nObj );
			
			sEventString = pProps->GetCommand( szPropName, "" );
			if( !sEventString.empty() )
			{
				std::string::size_type	nCmdPos = sEventString.find( "|" );

				// Make sure we don't have an empty event...

				if( nCmdPos == std::string::npos ) 
					continue;

				// Ok we have a command and time stamp, create the event and add it to the list...

				EVENT_CMD_STRUCT *cmdStruct = debug_new(EVENT_CMD_STRUCT);
				m_vecEventCmds.push_back( cmdStruct );

				sTimeStamp = sEventString.substr( 0, nCmdPos );
				
				// TimeStamp is in milliseconds so just convert it...
				cmdStruct->m_fTime = (float)atoi( sTimeStamp.c_str() ) / 1000.0f;
				
				if( cmdStruct->m_fTime > m_fTotalTime )
					m_fTotalTime = cmdStruct->m_fTime;

				cmdStruct->m_sCommand = sEventString.substr( nCmdPos + 1 );
			}
		}
	}

	// Shrink-to-fit using the swap method to release excess memory...
	
	EVENT_CMD_VECTOR( m_vecEventCmds ).swap( m_vecEventCmds );

	// Read the rest of the properties...

	m_sFinishedCmd		= pProps->GetCommand( "FinishedCommand", "" );
	m_nNumActivations	= pProps->GetLongInt( "NumberOfActivations", m_nNumActivations );
	m_bLocked			= pProps->GetBool( "Locked", m_bLocked );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::HandleOnMsg
//
//  PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void CommandObject::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Begin the chain of events...

	if( !m_vecEventCmds.empty() && !m_bLocked )
	{
		if( !m_Timer.IsStarted( ))
		{
			m_Timer.Start( m_fTotalTime );
			++m_nNumTimesActivated;
		}

		SetNextUpdate( UPDATE_NEXT_FRAME );
	}

	// Save the object which turned us on.

	m_hActivator = hSender;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::HandleOffMsg
//
//  PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void CommandObject::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// End the chain of events...

	m_Timer.Stop();
	SetNextUpdate( UPDATE_NEVER );

	// Clear the object which turned us on.

	m_hActivator = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::HandlePauseMsg
//
//  PURPOSE:	Handle a PAUSE message...
//
// ----------------------------------------------------------------------- //

void CommandObject::HandlePauseMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Pause the chain of events...

	if( m_Timer.IsStarted( ) && !m_Timer.IsPaused())
	{
		m_Timer.Pause();
		SetNextUpdate( UPDATE_NEVER );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::HandleResumeMsg
//
//  PURPOSE:	Handle a RESUME message...
//
// ----------------------------------------------------------------------- //

void CommandObject::HandleResumeMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Continue the events from where previously paused...

	if( m_Timer.IsStarted() && m_Timer.IsPaused() )
	{
		m_Timer.Resume();	
		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::HandleLockMsg
//
//  PURPOSE:	Handle a LOCK message...
//
// ----------------------------------------------------------------------- //

void CommandObject::HandleLockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bLocked = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::HandleUnlockMsg
//
//  PURPOSE:	Handle a UNLOCK message...
//
// ----------------------------------------------------------------------- //

void CommandObject::HandleUnlockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bLocked = false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::Update
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

void CommandObject::Update( )
{
	EVENT_CMD_VECTOR::iterator iter;
	EVENT_CMD_STRUCT	*pEvntCmdStruct = NULL;

	// Process any command whos time stamp is reached...

	for( iter = m_vecEventCmds.begin(); iter != m_vecEventCmds.end(); ++iter )
	{
		pEvntCmdStruct = *iter;

		// Did we already process this command...

		if( !pEvntCmdStruct->m_bProcessed )
		{
			if( pEvntCmdStruct->m_fTime <= m_Timer.GetElapseTime() )
			{
				// Ok we are supposed to process this one, send it...

				pEvntCmdStruct->m_bProcessed = true;

				if( !pEvntCmdStruct->m_sCommand.empty() )
				{
					ILTBaseClass* pSender = g_pLTServer->HandleToObject(m_hActivator);
					g_pCmdMgr->QueueCommand( pEvntCmdStruct->m_sCommand.c_str(), pSender, pSender );
				}
			}
		}
	}

	// Don't update anymore if we are finished...

	if( m_Timer.IsTimedOut())
	{
		m_Timer.Stop();

		// Send a finished command if we have one...

		if( !m_sFinishedCmd.empty() )
		{
			ILTBaseClass* pSender = g_pLTServer->HandleToObject(m_hActivator);
			g_pCmdMgr->QueueCommand( m_sFinishedCmd.c_str(), pSender, pSender );
		}

		SetNextUpdate( UPDATE_NEVER );

		// Reset the commands

		for( iter = m_vecEventCmds.begin(); iter != m_vecEventCmds.end(); ++iter )
		{
			pEvntCmdStruct = *iter;
			pEvntCmdStruct->m_bProcessed = false;
		}

		// If we have a limited number of activations remove us when the limit is reached...
		if( m_nNumActivations > 0 )
		{
			if( m_nNumTimesActivated >= m_nNumActivations )
			{
				g_pLTServer->RemoveObject( m_hObject );
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::Save
//
//  PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CommandObject::Save( ILTMessage_Write *pMsg )
{
	m_Timer.Save( *pMsg );
	
	SAVE_HOBJECT( m_hActivator );
	SAVE_FLOAT( m_fTotalTime );
	SAVE_STDSTRING( m_sFinishedCmd );
	SAVE_bool( m_bLocked );
	SAVE_INT( m_nNumActivations );
	SAVE_INT( m_nNumTimesActivated );

	// Save every Event command struct...

	SAVE_INT( m_vecEventCmds.size() );

	EVENT_CMD_VECTOR::iterator iter;
	EVENT_CMD_STRUCT	*pEvntCmdStruct = NULL;

	for( iter = m_vecEventCmds.begin(); iter != m_vecEventCmds.end(); ++iter )
	{
		pEvntCmdStruct = *iter; 

		SAVE_FLOAT( pEvntCmdStruct->m_fTime );
		SAVE_BOOL( pEvntCmdStruct->m_bProcessed );
		SAVE_STDSTRING( pEvntCmdStruct->m_sCommand );

	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::Load
//
//  PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CommandObject::Load( ILTMessage_Read *pMsg )
{
	m_Timer.Load( *pMsg );

	LOAD_HOBJECT( m_hActivator );
	LOAD_FLOAT( m_fTotalTime );
	LOAD_STDSTRING( m_sFinishedCmd );
	LOAD_bool( m_bLocked );
	LOAD_INT( m_nNumActivations );
	LOAD_INT( m_nNumTimesActivated );
	
	// Free any event commands we already have...

	EVENT_CMD_VECTOR::iterator iter;
	EVENT_CMD_STRUCT	*pEvntCmdStruct = NULL;

	for( iter = m_vecEventCmds.begin(); iter != m_vecEventCmds.end(); ++iter )
	{
		pEvntCmdStruct = *iter;

		debug_delete( pEvntCmdStruct );
		pEvntCmdStruct = NULL;
	}

	m_vecEventCmds.clear();

	// Load all event commands...

	int nNumEventCmds = 0;
	LOAD_INT( nNumEventCmds );

	for( int i = 0; i < nNumEventCmds; ++i )
	{
		pEvntCmdStruct = debug_new( EVENT_CMD_STRUCT );
		m_vecEventCmds.push_back( pEvntCmdStruct );

		LOAD_FLOAT( pEvntCmdStruct->m_fTime );
		LOAD_BOOL( pEvntCmdStruct->m_bProcessed );
		LOAD_STDSTRING( pEvntCmdStruct->m_sCommand );
	}
}
