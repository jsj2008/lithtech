// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayMeter.cpp
//
// PURPOSE : DisplayMeter - Implementation
//
// CREATED : 7/19/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DisplayMeter.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "CommandMgr.h"
#include "MsgIDs.h"

BEGIN_CLASS(DisplayMeter)
	PROP_DEFINEGROUP(Phase1, PF_GROUP1)
		ADD_LONGINTPROP_FLAG(Phase1Value, -1, PF_GROUP1)
		ADD_STRINGPROP_FLAG(Phase1Cmd, "", PF_GROUP1)
	PROP_DEFINEGROUP(Phase2, PF_GROUP2)
		ADD_LONGINTPROP_FLAG(Phase2Value, -1, PF_GROUP2)
		ADD_STRINGPROP_FLAG(Phase2Cmd, "", PF_GROUP2)
	PROP_DEFINEGROUP(Phase3, PF_GROUP3)
		ADD_LONGINTPROP_FLAG(Phase3Value, -1, PF_GROUP3)
		ADD_STRINGPROP_FLAG(Phase3Cmd, "", PF_GROUP3)
	PROP_DEFINEGROUP(Phase4, PF_GROUP4)
		ADD_LONGINTPROP_FLAG(Phase4Value, -1, PF_GROUP4)
		ADD_STRINGPROP_FLAG(Phase4Cmd, "", PF_GROUP4)
	PROP_DEFINEGROUP(Phase5, PF_GROUP5)
		ADD_LONGINTPROP_FLAG(Phase5Value, -1, PF_GROUP5)
		ADD_STRINGPROP_FLAG(Phase5Cmd, "", PF_GROUP5)
    ADD_BOOLPROP(RemoveWhenEmpty, LTTRUE)
END_CLASS_DEFAULT(DisplayMeter, BaseClass, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::DisplayMeter()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DisplayMeter::DisplayMeter() : BaseClass(OT_NORMAL)
{
    m_bRemoveWhenEmpty  = LTTRUE;
	m_nValue			= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::~DisplayMeter()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

DisplayMeter::~DisplayMeter()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 DisplayMeter::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
//	ROUTINE:	DisplayMeter::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 DisplayMeter::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
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
//	ROUTINE:	DisplayMeter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void DisplayMeter::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;
	char szProp[128];

	for (int i=1; i <= DM_MAX_NUMBER_OF_PHASES; i++)
	{
		sprintf(szProp, "Phase%dValue", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_PhaseData[i-1].nValue = genProp.m_Long;
		}

		sprintf(szProp, "Phase%dCmd", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
                m_PhaseData[i-1].hstrCmd = g_pLTServer->CreateString(genProp.m_String);
			}
		}

	}

    if (g_pLTServer->GetPropGeneric("RemoveWhenEmpty", &genProp) == LT_OK)
	{
		m_bRemoveWhenEmpty = genProp.m_Bool;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void DisplayMeter::InitialUpdate()
{
	// Must be triggered on...

    g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void DisplayMeter::Update()
{
    g_pLTServer->SetNextUpdate(m_hObject, 0.0f);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::TriggerMsg()
//
//	PURPOSE:	Process cinematic trigger messages
//
// --------------------------------------------------------------------------- //

void DisplayMeter::TriggerMsg(HOBJECT hSender, const char *szMsg)
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
			if ((_stricmp(parse.m_Args[0], "show") == 0))
			{
				if (parse.m_nArgs > 1)
				{
                    HandleShow((uint8)atoi(parse.m_Args[1]));
				}
				else
					HandleShow(100);
			}
			else if (_stricmp(parse.m_Args[0], "plus") == 0)
			{
				if (parse.m_nArgs > 1)
				{
                    HandlePlus((uint8)atoi(parse.m_Args[1]));
				}
			}
			else if (_stricmp(parse.m_Args[0], "minus") == 0)
			{
				if (parse.m_nArgs > 1)
				{
                    HandleMinus((uint8)atoi(parse.m_Args[1]));
				}
			}
			else if (_stricmp(parse.m_Args[0], "set") == 0)
			{
				if (parse.m_nArgs > 1)
				{
                    HandleSet((uint8)atoi(parse.m_Args[1]));
				}
			}
			else if (_stricmp(parse.m_Args[0], "hide") == 0)
			{
				HandleEnd();
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleShow()
//
//	PURPOSE:	Handle show message
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandleShow(uint8 initVal)
{
	if (initVal == 0) return;
	if (initVal > 100) initVal = 100;

	m_nValue = initVal;

	// Send message to clients telling them about the DisplayMeter...
	UpdateClients();


	// Update the DisplayMeter...
//    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleSet()
//
//	PURPOSE:	Handle set message
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandleSet(uint8 val)
{
	if (val == 0)
	{
		if (m_nValue > 0)
			HandleEnd();
		return;
	}
	if (val > 100) val = 100;

	if (m_nValue == 0)
	{
		HandleShow(val);
		return;
	}

	m_nValue = val;

	// Send message to clients telling them about the DisplayMeter...
	UpdateClients();

	// Update the DisplayMeter...
//    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandlePlus()
//
//	PURPOSE:	Handle plus message
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandlePlus(uint8 val)
{
	if (val == 0) return;
	if (val > 100) val = 100;

	if (m_nValue == 0)
	{
		HandleShow(val);
		return;
	}

	m_nValue += val;
	if (m_nValue > 100) m_nValue = 100;

	// Send message to clients telling them about the DisplayMeter...
	UpdateClients();

	// Update the DisplayMeter...
//    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleMinus()
//
//	PURPOSE:	Handle plus message
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandleMinus(uint8 val)
{
	if (val == 0) return;
	if (val > 100) val = 100;

	int newValue = m_nValue - val;
	if (newValue < 0) newValue = 0;
	int newPhase = -1;
	int testPhase = 0;
	//see if we've entered a new phase
	while (testPhase < DM_MAX_NUMBER_OF_PHASES)
	{
		//if our new value is less than the phase, but our old value is greater than it, we're in a new phase
		if ((int)newValue <= m_PhaseData[testPhase].nValue && (int)m_nValue > m_PhaseData[testPhase].nValue)
		{
			//if we find more than one possible phase use the lowest valued one
			if (newPhase < 0 || m_PhaseData[testPhase].nValue < m_PhaseData[newPhase].nValue)
				newPhase = testPhase;

		}
		testPhase++;
	}

	//if we have entered a new phase, see if it has a command
	if (newPhase >= 0)
	{
		if (m_PhaseData[newPhase].hstrCmd)
		{
			char* pCmd = g_pLTServer->GetStringData(m_PhaseData[newPhase].hstrCmd);
			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd);
			}
		}
	}


	if (newValue == 0)
	{
		HandleEnd();
		return;
	}

	m_nValue = newValue;

	// Send message to clients telling them about the DisplayMeter...
	UpdateClients();

	// Update the DisplayMeter...
//    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleEnd()
//
//	PURPOSE:	Handle the DisplayMeter ending
//
// ----------------------------------------------------------------------- //

void DisplayMeter::HandleEnd()
{
	m_nValue = 0;

	if (m_bRemoveWhenEmpty)
	{
        g_pLTServer->RemoveObject(m_hObject);
	}
	UpdateClients();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::UpdateClients()
//
//	PURPOSE:	Update the client's time
//
// --------------------------------------------------------------------------- //

void DisplayMeter::UpdateClients()
{
	// Send message to clients telling them about the DisplayMeter...

    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_DISPLAY_METER);
    g_pLTServer->WriteToMessageByte(hMessage, m_nValue);
    g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DisplayMeter::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	SAVE_BOOL(m_bRemoveWhenEmpty);
	SAVE_BYTE(m_nValue);

	for (int i=0; i < DM_MAX_NUMBER_OF_PHASES; i++)
	{
		m_PhaseData[i].Save(hWrite);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DisplayMeter::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	LOAD_BOOL(m_bRemoveWhenEmpty);
	LOAD_BYTE(m_nValue);

	for (int i=0; i < DM_MAX_NUMBER_OF_PHASES; i++)
	{
		m_PhaseData[i].Load(hRead);
	}

	if (m_nValue > 0)
	{
		UpdateClients();
	}
}