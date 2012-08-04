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
#include "CommandMgr.h"
#include "MsgIDs.h"

BEGIN_CLASS(DisplayTimer)
	ADD_STRINGPROP(StartCommand, "")
	ADD_STRINGPROP(EndCommand, "")
    ADD_BOOLPROP(RemoveWhenDone, LTTRUE)
END_CLASS_DEFAULT(DisplayTimer, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::DisplayTimer()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DisplayTimer::DisplayTimer() : BaseClass(OT_NORMAL)
{
    m_hstrStartCmd      = LTNULL;
    m_hstrEndCmd        = LTNULL;
    m_bRemoveWhenDone   = LTTRUE;
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
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 DisplayTimer::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER :
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMsg);
		}
		break;

		default : break;
	}

	return BaseClass::ObjectMessageFn(hSender, messageID, hRead);
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

    g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
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

    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::TriggerMsg()
//
//	PURPOSE:	Process cinematic trigger messages
//
// --------------------------------------------------------------------------- //

void DisplayTimer::TriggerMsg(HOBJECT hSender, const char *szMsg)
{
    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if ((_stricmp(parse.m_Args[0], "START") == 0) ||
				(_stricmp(parse.m_Args[0], "ON") == 0))
			{
				if (parse.m_nArgs > 1)
				{
                    HandleStart((LTFLOAT)atof(parse.m_Args[1]));
				}
			}
			else if (_stricmp(parse.m_Args[0], "ADD") == 0)
			{
				if (parse.m_nArgs > 1)
				{
                    HandleAdd((LTFLOAT)atof(parse.m_Args[1]));
				}
			}
			else if ((_stricmp(parse.m_Args[0], "END") == 0) ||
					 (_stricmp(parse.m_Args[0], "OFF") == 0))
			{
				HandleEnd();
			}
			else if (_stricmp(parse.m_Args[0], "KILL") == 0)
			{
				HandleAbort();
			}
			else if (_stricmp(parse.m_Args[0], "PAUSE") == 0)
			{
				HandlePause();
			}
			else if (_stricmp(parse.m_Args[0], "RESUME") == 0)
			{
				HandleResume();
			}
		}
	}
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
        char* pCmd = g_pLTServer->GetStringData(m_hstrStartCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
		}
	}

	m_Timer.Start(fTime);


	// Send message to clients telling them about the DisplayTimer...

	UpdateClients();


	// Update the DisplayTimer...

    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
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

    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
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

    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
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
        char* pCmd = g_pLTServer->GetStringData(m_hstrEndCmd);

		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd);
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

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_DISPLAY_TIMER);
    g_pLTServer->WriteToMessageFloat(hMessage, m_Timer.GetCountdownTime());
    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_Timer.Paused());
    g_pLTServer->EndMessage(hMessage);
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

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_DISPLAY_TIMER);
    g_pLTServer->WriteToMessageFloat(hMessage, 0.0f);
    g_pLTServer->EndMessage(hMessage);

    g_pLTServer->SetNextUpdate(m_hObject, 0.0f);

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

void DisplayTimer::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	SAVE_HSTRING(m_hstrStartCmd);
	SAVE_HSTRING(m_hstrEndCmd);
	SAVE_BOOL(m_bRemoveWhenDone);

	m_Timer.Save(hWrite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayTimer::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DisplayTimer::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	LOAD_HSTRING(m_hstrStartCmd);
	LOAD_HSTRING(m_hstrEndCmd);
	LOAD_BOOL(m_bRemoveWhenDone);

	m_Timer.Load(hRead);

	if (m_Timer.GetDuration() > 0.0f)
	{
		UpdateClients();
	}
}