// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayTimer.cpp
//
// PURPOSE : DisplayTimer - Implementation
//
// CREATED : 10/15/99
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "DisplayTimer.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "MsgIDs.h"
#include "VersionMgr.h"
#include "TeamMgr.h"
#include "ServerMissionMgr.h"

LINKFROM_MODULE( DisplayTimer );


BEGIN_CLASS(DisplayTimer)
	ADD_COMMANDPROP_FLAG(StartCommand, "", PF_NOTIFYCHANGE, "This is the command string that is sent when the timer begins its count down.")
	ADD_COMMANDPROP_FLAG(EndCommand, "", PF_NOTIFYCHANGE, "This is the command string that is sent when the timer finishes counting down to 0.")
	ADD_BOOLPROP(RemoveWhenDone, true, "By default the timer is removed when the time runs out.  If you want to reuse a timer set the RemoveWhenDone property to FALSE.")
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST, "This is a dropdown that allows you to set which team the timer is targetting.")
END_CLASS_FLAGS_PLUGIN(DisplayTimer, GameBase, CF_HIDDEN, CDisplayTimerPlugin, "Used as a countdown display object." )


CMDMGR_BEGIN_REGISTER_CLASS( DisplayTimer )

	ADD_MESSAGE( ON,		2,	NULL,	MSG_HANDLER( DisplayTimer, HandleOnMsg ),		"ON <time>", "Tells the DisplayTimer to display on the HUD and how much time to display. StartCommand is sent", "Setting a display meter to count down 5 seconds the command would look like:<BR><BR>msg DisplayTimer (on 5)" )
	ADD_MESSAGE( OFF,		1,	NULL,	MSG_HANDLER( DisplayTimer, HandleOffMsg ),		"OFF", "Stops the timer and sends the EndCommand if one was specified and removes it if RemoveWhenDone is set to true.", "msg DisplayTimer OFF" )
	ADD_MESSAGE( INC,		2,	NULL,	MSG_HANDLER( DisplayTimer, HandleIncMsg ),		"INC <amount>", "Increments the DisplayTimer by the specified amount of time.", "To add 5 seconds to the DisplayTimer the command would look like:<BR><BR>msg DisplayTimer (INC 5)" )
	ADD_MESSAGE( DEC,		2,	NULL,	MSG_HANDLER( DisplayTimer, HandleDecMsg ),		"DEC <amount>", "Decrements the DisplayTimer by the specified amount of time.", "To subtract 5 seconds from the DisplayTimer the command would look like:<BR><BR>msg DisplayTimer (DEC 5)" )
	ADD_MESSAGE( ABORT,		1,	NULL,	MSG_HANDLER( DisplayTimer, HandleAbortMsg ),	"ABORT", "Stops the DisplayTimer and removes it if RemoveWhenDone is set to true, but does not send the EndCommand", "msg DisplayTimer ABORT" )
	ADD_MESSAGE( PAUSE,		1,	NULL,	MSG_HANDLER( DisplayTimer, HandlePauseMsg ),	"PAUSE", "Pauses the countdown", "msg DisplayTimer PAUSE" )
	ADD_MESSAGE( RESUME,	1,	NULL,	MSG_HANDLER( DisplayTimer, HandleResumeMsg ),	"RESUME", "Resumes the countdown", "msg DisplayTimer RESUME" )
	ADD_MESSAGE( TEAM,		2,	NULL,	MSG_HANDLER( DisplayTimer, HandleTeamMsg ),		"TEAM <0, 1, -1>", "Specifies which team the DisplayTimer belongs to", "msg DisplayTimer (TEAM 1)" )

CMDMGR_END_REGISTER_CLASS( DisplayTimer, GameBase )

LTRESULT CDisplayTimerPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	// See if we can handle the property...

	// Handle team...

	if( LTStrIEquals( "Team", szPropName ))
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDisplayMeterPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //


LTRESULT CDisplayTimerPlugin::PreHook_PropChanged( const char *szObjName,
												   const char *szPropName, 
												   const int  nPropType, 
												   const GenericProp &gpPropValue,
												   ILTPreInterface *pInterface,
												   const char *szModifiers )
{
	// Only our commands are marked for change notification so just send it to the CommandMgr..

	if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
												szPropName, 
												nPropType, 
												gpPropValue,
												pInterface,
												szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::DisplayTimer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DisplayTimer::DisplayTimer()
:	GameBase			( OT_NORMAL ),
	m_sStartCmd			( ),
	m_sEndCmd			( ),
	m_bRemoveWhenDone	( true ),
	m_nTeamId			( INVALID_TEAM )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::~DisplayTimer()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

DisplayTimer::~DisplayTimer()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 DisplayTimer::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);

			uint32 dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			// We need to reset our sfx message since values
			// could have changed across save versions.
			
			UpdateClients( );
			
			return dwRet;
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void DisplayTimer::ReadProp(const GenericPropList *pProps)
{
	m_sStartCmd			= pProps->GetCommand( "StartCommand", "" );
	m_sEndCmd			= pProps->GetCommand( "EndCommand", "" );
	m_bRemoveWhenDone	= pProps->GetBool( "RemoveWhenDone", m_bRemoveWhenDone );
	
	// Get the team this object belongs to.
	const char *pszTeam = pProps->GetString( "Team", "" );
	m_nTeamId = TeamStringToTeamId( pszTeam );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void DisplayTimer::InitialUpdate()
{
	m_Timer.SetEngineTimer( SimulationTimer::Instance());

	// Must be triggered on...

	// Make sure our sfx message is told to the client.
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE );

    SetNextUpdate(UPDATE_NEVER);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void DisplayTimer::Update()
{
	// Testing...
    //g_pLTServer->CPrint("Time Left: %.2f", m_Timer.GetCountdownTime());

	if (m_Timer.IsTimedOut())
	{
		End();
		return;
	}

	// Don't need updates until we time out.
    SetNextUpdate((float)m_Timer.GetTimeLeft());
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleOnMsg()
//
//	PURPOSE:	Handle a ON message...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Start( (float)atof(crParsedMsg.GetArg(1)) );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleOffMsg()
//
//	PURPOSE:	Handle a OFF message...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	End();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleAbortMsg()
//
//	PURPOSE:	Handle a ABORT message...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleAbortMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Abort();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandlePauseMsg()
//
//	PURPOSE:	Handle a PAUSE message...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandlePauseMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_Timer.Pause();

	// Send message to clients telling them about the DisplayTimer...

	UpdateClients();

	// Update the DisplayTimer...

    SetNextUpdate(UPDATE_NEXT_FRAME);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleResumeMsg()
//
//	PURPOSE:	Handle a RESUME message...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleResumeMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_Timer.Resume();

	// Send message to clients telling them about the DisplayTimer...

	UpdateClients();


	// Update the DisplayTimer...

    SetNextUpdate(UPDATE_NEXT_FRAME);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleIncMsg()
//
//	PURPOSE:	Handle a INC message...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleIncMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	float fTime = (float)atof( crParsedMsg.GetArg(1) );

	// If we're not incrementing the timer or the timer is not on then don't continue...
	if( (fTime <= 0.0f) || !m_Timer.IsStarted())
		return;

	// Add the time from the timer...
	m_Timer.AddDuration(fTime);

	// Send message to clients telling them to update DisplayTimer...
	UpdateClients();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleDecMsg()
//
//	PURPOSE:	Handle a DEC message...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleDecMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	float fTime = -1 * (float)atof( crParsedMsg.GetArg(1) );

	// If we're not decrementing the timer or the timer is not on then don't continue...
	if( (fTime >= 0.0f) || !m_Timer.IsStarted())
		return;

	// Subtract the time from the timer...
	m_Timer.AddDuration( fTime );

	// Send message to clients telling them to update DisplayTimer...
	UpdateClients();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleTeamMsg()
//
//	PURPOSE:	Handle a TEAM message...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 nTeamId = atoi( crParsedMsg.GetArg( 1 ));
	if( nTeamId < MAX_TEAMS )
	{
		ASSERT( nTeamId < 255 );
		m_nTeamId = ( uint8 )LTMIN( nTeamId, 255 );
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}

	UpdateClients( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::End()
//
//	PURPOSE:	End the DisplayTimer
//
// ----------------------------------------------------------------------- //

void DisplayTimer::End()
{
	if( !m_sEndCmd.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sEndCmd.c_str(), m_hObject, m_hObject );
	}

	// Tell client to stop the timer...

	Abort();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::UpdateClients()
//
//	PURPOSE:	Update the client's time
//
// --------------------------------------------------------------------------- //

void DisplayTimer::UpdateClients()
{
	// Send message to clients telling them about the DisplayTimer...

	double fGameTimeEnd = m_Timer.GetEngineTimer().GetTimerAccumulatedS() + m_Timer.GetTimeLeft( );

	DisplayTimerMsgId eDisplayTimerMsgId;
	switch( m_nTeamId )
	{
	default:
		{
			// The main timer can be in playing or in sudden death.
			if( g_pServerMissionMgr->GetServerGameState() == EServerGameState_Playing )
			{
				eDisplayTimerMsgId = kDisplayTimerMsgId_Main;
			}
			else
			{
				eDisplayTimerMsgId = kDisplayTimerMsgId_SuddenDeath;
			}
		}
		break;
	case 0:
		eDisplayTimerMsgId = kDisplayTimerMsgId_Team0;
		break;
	case 1:
		eDisplayTimerMsgId = kDisplayTimerMsgId_Team1;
		break;
	}

	{
		// Set up the update message
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_DISPLAYTIMER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writedouble(fGameTimeEnd);
		cMsg.Writebool(m_Timer.IsPaused());
		cMsg.Writeuint8( eDisplayTimerMsgId );

		// Send the message to all connected clients
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}
	
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_DISPLAYTIMER_ID);
		cMsg.Writedouble( fGameTimeEnd );
		cMsg.Writebool(m_Timer.IsPaused());
		cMsg.Writeuint8( eDisplayTimerMsgId );

		// Make sure new clients will get the message
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}


}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Abort()
//
//	PURPOSE:	Stop the timer...
//
// --------------------------------------------------------------------------- //

void DisplayTimer::Abort()
{
	// Tell the client to stop the timer...
	m_Timer.Stop( );

	UpdateClients( );

	SetNextUpdate(UPDATE_NEVER);

	if (m_bRemoveWhenDone)
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DisplayTimer::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_STDSTRING( m_sStartCmd );
	SAVE_STDSTRING( m_sEndCmd );
	SAVE_BOOL(m_bRemoveWhenDone);

	m_Timer.Save(*pMsg);

	SAVE_BYTE( m_nTeamId );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DisplayTimer::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_STDSTRING( m_sStartCmd );
	LOAD_STDSTRING( m_sEndCmd );
	LOAD_BOOL(m_bRemoveWhenDone);

	m_Timer.Load( *pMsg );

	LOAD_BYTE( m_nTeamId );

	if (m_Timer.GetDuration() > 0.0f)
	{
		UpdateClients();
	}
}
// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Start
//
//	PURPOSE:	Starts the timer.
//
// --------------------------------------------------------------------------- //

void DisplayTimer::Start( double fTime )
{
	if (fTime < 0.0f) return;

	if( !m_sStartCmd.empty() )
	{
		g_pCmdMgr->QueueCommand(m_sStartCmd.c_str(), m_hObject, m_hObject);
	}

	m_Timer.Start(fTime);


	// Send message to clients telling them about the DisplayTimer...

	UpdateClients();


	// Update the DisplayTimer...

	SetNextUpdate(UPDATE_NEXT_FRAME);
}
