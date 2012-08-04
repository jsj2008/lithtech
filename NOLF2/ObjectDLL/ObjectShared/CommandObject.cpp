// ----------------------------------------------------------------------- //
//
// MODULE  : CommandObject.cpp
//
// PURPOSE : The CommandObject implementation
//
// CREATED : 10/11/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "ObjectMsgs.h"
	#include "ParsedMsg.h"
	#include "CommandObject.h"


LINKFROM_MODULE( CommandObject );

//
// Defines...
//

	#define EVNTED_BASE_STRING	"Event"

	#define ADD_COMMAND_EDITOR_TRACK( base_string, track, flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E0, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E1, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E2, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E3, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E4, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E5, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E6, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E7, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E8, "", flags ) \
				ADD_STRINGPROP_FLAG( ##base_string##T##track##E9, "", flags )

	#define ADD_EVENT_EDITOR_INFO( tracks, objects_per_track, base_string, flags ) \
				ADD_STRINGPROP_FLAG( EventDialogInfo, #tracks" "#objects_per_track" "#base_string, flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 0, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 1, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 2, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 3, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 4, PF_NOTIFYCHANGE | flags ) \
				ADD_COMMAND_EDITOR_TRACK( base_string, 5, PF_NOTIFYCHANGE | flags )



BEGIN_CLASS( CommandObject )

	PROP_DEFINEGROUP( EventCommands, PF_GROUP(1) | PF_EVENT )
		ADD_EVENT_EDITOR_INFO( 6, 10, Event, PF_GROUP(1) )
	
	ADD_STRINGPROP_FLAG( FinishedCommand, "", PF_NOTIFYCHANGE )
	ADD_LONGINTPROP_FLAG( NumberOfActivations, -1, 0 )
	ADD_BOOLPROP_FLAG( Locked, LTFALSE, 0 )

END_CLASS_DEFAULT_FLAGS_PLUGIN( CommandObject, GameBaseLite, NULL, NULL, CF_CLASSONLY, CCommandObjectPlugin )


CMDMGR_BEGIN_REGISTER_CLASS( CommandObject )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( START, 1, NULL, "START" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( STOP, 1, NULL, "STOP" )
	CMDMGR_ADD_MSG( ABORT, 1, NULL, "ABORT" )
	CMDMGR_ADD_MSG( PAUSE, 1, NULL, "PAUSE" )
	CMDMGR_ADD_MSG( RESUME, 1, NULL, "RESUME" )
	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )

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
	if( !_stricmp( szPropName, "FinishedCommand" ))
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
	else if( nPropType == LT_PT_STRING && gpPropValue.m_String[0] )
	{
		char strPropVal[256];
		SAFE_STRCPY( strPropVal, gpPropValue.m_String );

		char *pTime = strtok( strPropVal, "|" );
		char *pCmd = strtok( LTNULL, "\0" );
		
		GenericProp gp;

		if(pCmd)
		{
			SAFE_STRCPY( gp.m_String, pCmd );
		}
		else
		{
			gp.m_String[0] = '\0';
		}

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
:	GameBaseLite			( false ),
	m_fTotalTime		( 0.0f ),
	m_hstrFinishedCmd	( LTNULL ),
	m_bLocked			( LTFALSE ),
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
	EVENT_CMD_LIST::iterator iter;
	EVENT_CMD_STRUCT	*pEvntCmdStruct = LTNULL;

	// Free the allocated list of Event commands...

	for( iter = m_lstEventCmds.begin(); iter != m_lstEventCmds.end(); ++iter )
	{
		pEvntCmdStruct = *iter;

		FREE_HSTRING( pEvntCmdStruct->m_hstrCommand );
		debug_delete( pEvntCmdStruct );
	}

	m_lstEventCmds.clear();

	FREE_HSTRING( m_hstrFinishedCmd );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::EngineMessageFn
//
//  PURPOSE:	Handel messages from the engine
//
// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::ReadProps
//
//  PURPOSE:	Read the property values...
//
// ----------------------------------------------------------------------- //

bool CommandObject::ReadProp( ObjectCreateStruct *pOCS )
{
	if ( !GameBaseLite::ReadProp( pOCS ) )
	{
		return false;
	}

	ASSERT( pOCS != LTNULL );
	
	GenericProp gProp;

	char szPropName[32] = {0};

	// Loop over every property string to see if there is a coomad to process...

	for( int nTrack = 0; nTrack < EVNTED_MAX_TRACKS; ++nTrack )
	{
		for( int nObj = 0; nObj < EVNTED_MAX_OBJ_PER_TRACK; ++nObj )
		{
			sprintf( szPropName, "%sT%iE%i", EVNTED_BASE_STRING, nTrack, nObj );
			
			if( g_pLTServer->GetPropGeneric( szPropName, &gProp ) == LT_OK )
			{
				if( gProp.m_String[0] )
				{
					char *pTimeStamp = strtok( gProp.m_String, "|" );
					char *pCmdString = strtok( LTNULL, "\0" );

					// Make sure we don't have an empty event...

					if( !pTimeStamp || !pCmdString ) 
						continue;

					// Ok we have a command and time stamp, create the struct and add it to the list...

					EVENT_CMD_STRUCT *cmdStruct = debug_new(EVENT_CMD_STRUCT);
					m_lstEventCmds.push_back( cmdStruct );
					
					// TimeStamp is in milliseconds so just convert it...

					cmdStruct->m_fTime = (LTFLOAT)atoi( pTimeStamp ) / 1000.0f;
					
					if( cmdStruct->m_fTime > m_fTotalTime )
						m_fTotalTime = cmdStruct->m_fTime;

					cmdStruct->m_hstrCommand = g_pLTServer->CreateString( pCmdString );
				}
			}
		}
	}

	if( g_pLTServer->GetPropGeneric( "FinishedCommand", &gProp ) == LT_OK )
	{
		if( gProp.m_String[0] )
		{
			m_hstrFinishedCmd = g_pLTServer->CreateString( gProp.m_String );
		}
	}

	if( g_pLTServer->GetPropGeneric( "NumberOfActivations", &gProp ) == LT_OK )
	{
		m_nNumActivations = gProp.m_Long;
	}

	if( g_pLTServer->GetPropGeneric( "Locked", &gProp ) == LT_OK )
	{
		m_bLocked = gProp.m_Bool;
	}

	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CommandObject::OnTrigger
//
//  PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool CommandObject::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Start("START");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Stop("STOP");
	static CParsedMsg::CToken s_cTok_Abort("ABORT");
	static CParsedMsg::CToken s_cTok_Pause("PAUSE");
	static CParsedMsg::CToken s_cTok_Resume("RESUME");
	static CParsedMsg::CToken s_cTok_Lock( "LOCK" );
	static CParsedMsg::CToken s_cTok_Unlock( "UNLOCK" );

	if( (cMsg.GetArg(0) == s_cTok_On) || 
		(cMsg.GetArg(0) == s_cTok_Start))
	{
		// Begin the chain of events...

		if( m_lstEventCmds.size() && !m_bLocked )
		{
			if( !m_Timer.On() )
			{
				m_Timer.Start( m_fTotalTime );
				++m_nNumTimesActivated;
			}
			
			Activate();
		}
	}
	else if( (cMsg.GetArg(0) == s_cTok_Off) ||
			 (cMsg.GetArg(0) == s_cTok_Stop) ||
			 (cMsg.GetArg(0) == s_cTok_Abort))
	{
			m_Timer.Stop();
			Deactivate();
	}
	else if( cMsg.GetArg(0) == s_cTok_Pause )
	{
		if( m_Timer.On() && !m_Timer.Paused() )
		{
			m_Timer.Pause();
			Deactivate();
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Resume )
	{
		if( m_Timer.On() && m_Timer.Paused() )
		{
			m_Timer.Resume();	
			Activate();
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_Lock )
	{
		m_bLocked = true;
	}
	else if( cMsg.GetArg(0) == s_cTok_Unlock )
	{
		m_bLocked = false;
	}
	else
	{
		return GameBaseLite::OnTrigger( hSender, cMsg );
	}

	return true;
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
	EVENT_CMD_LIST::iterator iter;
	EVENT_CMD_STRUCT	*pEvntCmdStruct = LTNULL;

	// Process any command whos time stamp is reached...

	for( iter = m_lstEventCmds.begin(); iter != m_lstEventCmds.end(); ++iter )
	{
		pEvntCmdStruct = *iter;

		// Did we already process this command...

		if( !pEvntCmdStruct->m_bProcessed )
		{
			if( pEvntCmdStruct->m_fTime <= m_Timer.GetElapseTime() )
			{
				// Ok we are supposed to process this one, send it...

				pEvntCmdStruct->m_bProcessed = LTTRUE;

				if( pEvntCmdStruct->m_hstrCommand )
				{
					const char	*pCmd = g_pLTServer->GetStringData( pEvntCmdStruct->m_hstrCommand );

					if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
					{
						g_pCmdMgr->Process( pCmd, this, this );
					}
				}
			}
		}
	}

	// Don't update anymore if we are finished...

	if( m_Timer.Stopped() )
	{
		m_Timer.Stop();

		// Send a finished command if we have one...

		if( m_hstrFinishedCmd )
		{
			const char *pCmd = g_pLTServer->GetStringData( m_hstrFinishedCmd );

			if( pCmd && g_pCmdMgr->IsValidCmd( pCmd ))
			{
				g_pCmdMgr->Process( pCmd, this, this );
			}
		}

		Deactivate();

		// Reset the commands

		for( iter = m_lstEventCmds.begin(); iter != m_lstEventCmds.end(); ++iter )
		{
			pEvntCmdStruct = *iter;
			pEvntCmdStruct->m_bProcessed = LTFALSE;
		}

		// If we have a limited number of activations remove us when the limit is reached...

		if( m_nNumActivations > 0 )
		{
			if (m_nNumTimesActivated >= m_nNumActivations)
			{
				g_pGameServerShell->GetLiteObjectMgr()->RemoveObject(this);
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
	GameBaseLite::Save( pMsg );

	m_Timer.Save( pMsg );
	
	SAVE_FLOAT( m_fTotalTime );
	SAVE_HSTRING( m_hstrFinishedCmd );
	SAVE_bool( m_bLocked );
	SAVE_INT( m_nNumActivations );
	SAVE_INT( m_nNumTimesActivated );

	// Save every Event command struct...

	SAVE_INT( m_lstEventCmds.size() );

	EVENT_CMD_LIST::iterator iter;
	EVENT_CMD_STRUCT	*pEvntCmdStruct = LTNULL;

	for( iter = m_lstEventCmds.begin(); iter != m_lstEventCmds.end(); ++iter )
	{
		pEvntCmdStruct = *iter; 

		SAVE_FLOAT( pEvntCmdStruct->m_fTime );
		SAVE_BOOL( pEvntCmdStruct->m_bProcessed );
		SAVE_HSTRING( pEvntCmdStruct->m_hstrCommand );

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
	GameBaseLite::Load( pMsg );

	m_Timer.Load( pMsg );

	LOAD_FLOAT( m_fTotalTime );
	LOAD_HSTRING( m_hstrFinishedCmd );
	LOAD_bool( m_bLocked );
	LOAD_INT( m_nNumActivations );
	LOAD_INT( m_nNumTimesActivated );
	
	// Free any event commands we already have...

	EVENT_CMD_LIST::iterator iter;
	EVENT_CMD_STRUCT	*pEvntCmdStruct = LTNULL;

	for( iter = m_lstEventCmds.begin(); iter != m_lstEventCmds.end(); ++iter )
	{
		pEvntCmdStruct = *iter;

		FREE_HSTRING( pEvntCmdStruct->m_hstrCommand );
		debug_delete( pEvntCmdStruct );
		pEvntCmdStruct = LTNULL;
	}

	m_lstEventCmds.clear();

	// Load all event commands...

	int nNumEventCmds = 0;
	LOAD_INT( nNumEventCmds );

	for( int i = 0; i < nNumEventCmds; ++i )
	{
		pEvntCmdStruct = debug_new( EVENT_CMD_STRUCT );
		m_lstEventCmds.push_back( pEvntCmdStruct );

		LOAD_FLOAT( pEvntCmdStruct->m_fTime );
		LOAD_BOOL( pEvntCmdStruct->m_bProcessed );
		LOAD_HSTRING( pEvntCmdStruct->m_hstrCommand );
	}

}