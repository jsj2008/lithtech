// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayTimer.cpp
//
// PURPOSE : DisplayTimer - Implementation
//
// CREATED : 10/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DisplayTimer.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "MsgIDs.h"
#include "VersionMgr.h"

LINKFROM_MODULE( DisplayTimer );

#pragma force_active on
BEGIN_CLASS(DisplayTimer)
	ADD_STRINGPROP_FLAG(StartCommand, "", PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(EndCommand, "", PF_NOTIFYCHANGE)
    ADD_BOOLPROP(RemoveWhenDone, LTTRUE)
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST)
END_CLASS_DEFAULT_FLAGS_PLUGIN(DisplayTimer, GameBase, NULL, NULL, 0, CDisplayTimerPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( DisplayTimer )

	CMDMGR_ADD_MSG( START,	2,	NULL,	"START <time>" )
	CMDMGR_ADD_MSG( ON,		2,	NULL,	"ON <time>" )
	CMDMGR_ADD_MSG( ADD,	2,	NULL,	"ADD <amount>" )
	CMDMGR_ADD_MSG( END,	1,	NULL,	"END" )
	CMDMGR_ADD_MSG( OFF,	1,	NULL,	"OFF" )
	CMDMGR_ADD_MSG( KILL,	1,	NULL,	"KILL" )
	CMDMGR_ADD_MSG( PAUSE,	1,	NULL,	"PAUSE" )
	CMDMGR_ADD_MSG( RESUME,	1,	NULL,	"RESUME" )
	CMDMGR_ADD_MSG( TEAM,	2,	NULL,	"TEAM <0, 1, -1>" )

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

	if( _stricmp( "Team", szPropName ) == 0 )
	{
		char szTeam[32] = {0};

		_ASSERT(cMaxStrings > (*pcStrings) + 1);
		strcpy( aszStrings[(*pcStrings)++], "NoTeam" );
		
		for( int i = 0; i < MAX_TEAMS; ++i )
		{
			_ASSERT(cMaxStrings > (*pcStrings) + 1);

			sprintf( szTeam, "Team%i", i );
			strcpy( aszStrings[(*pcStrings)++], szTeam );
		}

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

DisplayTimer::DisplayTimer() : GameBase(OT_NORMAL)
{
    m_hstrStartCmd      = LTNULL;
    m_hstrEndCmd        = LTNULL;
    m_bRemoveWhenDone   = LTTRUE;
	m_nTeamId			= INVALID_TEAM;
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
	FREE_HSTRING(m_hstrStartCmd);
	FREE_HSTRING(m_hstrEndCmd);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 DisplayTimer::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
				ReadProp((ObjectCreateStruct*)pData);
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

void DisplayTimer::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("StartCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrStartCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("EndCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrEndCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("RemoveWhenDone", &genProp) == LT_OK)
	{
		m_bRemoveWhenDone = genProp.m_Bool;
	}

	// Get the team this object belongs to.
	if( IsTeamGameType() )
	{
		if( g_pLTServer->GetPropGeneric( "Team", &genProp ) == LT_OK )
		{
			if( genProp.m_String[0] )
			{
				// The team string should be TeamN, when N is the team id.
				char const szTeam[] = "Team";
				int nLen = strlen( szTeam );
				if( !_strnicmp( genProp.m_String, szTeam, nLen ))
				{
					uint32 nTeamId = atoi( &genProp.m_String[ nLen ] );
					if( nTeamId < MAX_TEAMS )
					{
						m_nTeamId = nTeamId;
					}
				}
			}
		}
	}
	else
	{
		m_nTeamId = INVALID_TEAM;
	}
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

	if (m_Timer.Stopped())
	{
		HandleEnd();
		return;
	}

	// Don't need updates until we time out.
    SetNextUpdate(m_Timer.GetCountdownTime( ));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::OnTrigger()
//
//	PURPOSE:	Process trigger messages
//
// --------------------------------------------------------------------------- //

bool DisplayTimer::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Start("START");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Add("ADD");
	static CParsedMsg::CToken s_cTok_End("END");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Kill("KILL");
	static CParsedMsg::CToken s_cTok_Pause("PAUSE");
	static CParsedMsg::CToken s_cTok_Resume("RESUME");
	static CParsedMsg::CToken s_cTok_Team("TEAM");

	if ((cMsg.GetArg(0) == s_cTok_Start) ||
		(cMsg.GetArg(0) == s_cTok_On))
	{
		if (cMsg.GetArgCount() > 1)
		{
            HandleStart((LTFLOAT)atof(cMsg.GetArg(1)));
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Add)
	{
		if (cMsg.GetArgCount() > 1)
		{
            HandleAdd((LTFLOAT)atof(cMsg.GetArg(1)));
		}
	}
	else if ((cMsg.GetArg(0) == s_cTok_End) ||
			 (cMsg.GetArg(0) == s_cTok_Off))
	{
		HandleEnd();
	}
	else if (cMsg.GetArg(0) == s_cTok_Kill)
	{
		HandleAbort();
	}
	else if (cMsg.GetArg(0) == s_cTok_Pause)
	{
		HandlePause();
	}
	else if (cMsg.GetArg(0) == s_cTok_Resume)
	{
		HandleResume();
	}
	else if (cMsg.GetArg(0) == s_cTok_Team)
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			uint32 nTeamId = atoi( cMsg.GetArg( 1 ));
			if( nTeamId < MAX_TEAMS )
			{
				m_nTeamId = nTeamId;
			}
			else
			{
				m_nTeamId = INVALID_TEAM;
			}

			UpdateClients( );
		}
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleStart()
//
//	PURPOSE:	Handle start message
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleStart(LTFLOAT fTime)
{
	if (fTime <= 0.0f) return;

	if (m_hstrStartCmd)
	{
        const char* pCmd = g_pLTServer->GetStringData(m_hstrStartCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
		}
	}

	m_Timer.Start(fTime);


	// Send message to clients telling them about the DisplayTimer...

	UpdateClients();


	// Update the DisplayTimer...

    SetNextUpdate(UPDATE_NEXT_FRAME);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandlePause()
//
//	PURPOSE:	Handle start message
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandlePause()
{
	m_Timer.Pause();

	// Send message to clients telling them about the DisplayTimer...

	UpdateClients();

	// Update the DisplayTimer...

    SetNextUpdate(UPDATE_NEXT_FRAME);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleResume()
//
//	PURPOSE:	Handle start message
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleResume()
{
	m_Timer.Resume();

	// Send message to clients telling them about the DisplayTimer...

	UpdateClients();


	// Update the DisplayTimer...

    SetNextUpdate(UPDATE_NEXT_FRAME);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleAdd()
//
//	PURPOSE:	Handle add message
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleAdd(LTFLOAT fTime)
{
	// Start the timer if it isn't going (and the timer is positive)...

	if (m_Timer.GetCountdownTime() <= 0.0f)
	{
		if (fTime > 0.0f)
		{
			HandleStart(fTime);
		}

		return;
	}


	// Add/subtract the time from the timer...

	m_Timer.Add(fTime);


	// Send message to clients telling them to update DisplayTimer...

	UpdateClients();


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::HandleEnd()
//
//	PURPOSE:	Handle the DisplayTimer ending
//
// ----------------------------------------------------------------------- //

void DisplayTimer::HandleEnd()
{
	if (m_hstrEndCmd)
	{
        const char* pCmd = g_pLTServer->GetStringData(m_hstrEndCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
		}
	}

	// Tell client to stop the timer...

	HandleAbort();
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

	float fGameTimeEnd = g_pLTServer->GetTime( ) + m_Timer.GetCountdownTime( );

	{
		// Set up the update message
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_DISPLAYTIMER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writefloat(fGameTimeEnd);
		cMsg.Writeuint8((uint8)m_Timer.Paused());
		cMsg.Writeuint8( m_nTeamId );

		// Send the message to all connected clients
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
	
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_DISPLAYTIMER_ID);
		cMsg.Writefloat( fGameTimeEnd );
		cMsg.Writeuint8((uint8)m_Timer.Paused());
		cMsg.Writeuint8( m_nTeamId );

		// Make sure new clients will get the message
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}


}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Handle()
//
//	PURPOSE:	Handle abort message
//
// --------------------------------------------------------------------------- //

void DisplayTimer::HandleAbort()
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

	SAVE_HSTRING(m_hstrStartCmd);
	SAVE_HSTRING(m_hstrEndCmd);
	SAVE_BOOL(m_bRemoveWhenDone);

	m_Timer.Save(pMsg);

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

	LOAD_HSTRING(m_hstrStartCmd);
	LOAD_HSTRING(m_hstrEndCmd);
	LOAD_BOOL(m_bRemoveWhenDone);

	m_Timer.Load(pMsg);

	if( g_pVersionMgr->GetCurrentSaveVersion( ) > CVersionMgr::kSaveVersion__1_2 )
	{
		LOAD_BYTE( m_nTeamId );
	}

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

void DisplayTimer::Start( float fTime )
{
	HandleStart( fTime );
}