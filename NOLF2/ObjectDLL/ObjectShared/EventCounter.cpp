// ----------------------------------------------------------------------- //
//
// MODULE  : EventCounter.cpp
//
// PURPOSE : EventCounter - Implementation
//
// CREATED : 03/22/2000
//
// (c) 2000-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "EventCounter.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"

LINKFROM_MODULE( EventCounter );

#pragma force_active on
BEGIN_CLASS(EventCounter)
	ADD_LONGINTPROP_FLAG(StartingValue, 0, 0)
	PROP_DEFINEGROUP(Event1, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(Event1Value, -1, PF_GROUP(1))
		ADD_STRINGPROP_FLAG(Event1IncToValCmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event1DecToValCmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event2, PF_GROUP(2))
		ADD_LONGINTPROP_FLAG(Event2Value, -1, PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Event2IncToValCmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event2DecToValCmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event3, PF_GROUP(3))
		ADD_LONGINTPROP_FLAG(Event3Value, -1, PF_GROUP(3))
		ADD_STRINGPROP_FLAG(Event3IncToValCmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event3DecToValCmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event4, PF_GROUP(4))
		ADD_LONGINTPROP_FLAG(Event4Value, -1, PF_GROUP(4))
		ADD_STRINGPROP_FLAG(Event4IncToValCmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event4DecToValCmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event5, PF_GROUP(5))
		ADD_LONGINTPROP_FLAG(Event5Value, -1, PF_GROUP(5))
		ADD_STRINGPROP_FLAG(Event5IncToValCmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event5DecToValCmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event6, PF_GROUP(6))
		ADD_LONGINTPROP_FLAG(Event6Value, -1, PF_GROUP(6))
		ADD_STRINGPROP_FLAG(Event6IncToValCmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event6DecToValCmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event6, PF_GROUP(6))
		ADD_LONGINTPROP_FLAG(Event6Value, -1, PF_GROUP(6))
		ADD_STRINGPROP_FLAG(Event6IncToValCmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event6DecToValCmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event7, PF_GROUP(7))
		ADD_LONGINTPROP_FLAG(Event7Value, -1, PF_GROUP(7))
		ADD_STRINGPROP_FLAG(Event7IncToValCmd, "", PF_GROUP(7) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event7DecToValCmd, "", PF_GROUP(7) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event8, PF_GROUP(8))
		ADD_LONGINTPROP_FLAG(Event8Value, -1, PF_GROUP(8))
		ADD_STRINGPROP_FLAG(Event8IncToValCmd, "", PF_GROUP(8) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event8DecToValCmd, "", PF_GROUP(8) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event9, PF_GROUP(9))
		ADD_LONGINTPROP_FLAG(Event9Value, -1, PF_GROUP(9))
		ADD_STRINGPROP_FLAG(Event9IncToValCmd, "", PF_GROUP(9) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event9DecToValCmd, "", PF_GROUP(9) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Event10, PF_GROUP(10))
		ADD_LONGINTPROP_FLAG(Event10Value, -1, PF_GROUP(10))
		ADD_STRINGPROP_FLAG(Event10IncToValCmd, "", PF_GROUP(10) | PF_NOTIFYCHANGE)
		ADD_STRINGPROP_FLAG(Event10DecToValCmd, "", PF_GROUP(10) | PF_NOTIFYCHANGE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(EventCounter, BaseClass, NULL, NULL, 0, CEventCounterPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( EventCounter )

	CMDMGR_ADD_MSG( INC,		2,	NULL,	"INC <amount>" )
	CMDMGR_ADD_MSG( INCREMENT,	2,	NULL,	"INCREMENT <amount>" )
	CMDMGR_ADD_MSG( DEC,		2,	NULL,	"DEC <amount>" )
	CMDMGR_ADD_MSG( DECREMENT,	2,	NULL,	"DECREMENT <amount>" )
	CMDMGR_ADD_MSG( LOCK,		1,	NULL,	"LOCK" )
	CMDMGR_ADD_MSG( UNLOCK,		1,	NULL,	"UNLOCK" )

CMDMGR_END_REGISTER_CLASS( EventCounter, BaseClass )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CEventCounterPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CEventCounterPlugin::PreHook_PropChanged( const char *szObjName,
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
//	ROUTINE:	EventCounter::EventCounter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

EventCounter::EventCounter() : GameBase()
{
    m_bLocked = LTFALSE;
	m_nCurVal = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::~EventCounter()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

EventCounter::~EventCounter()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 EventCounter::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE :
		{
			// Don't eat ticks please...
			SetNextUpdate(UPDATE_NEVER);
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
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::TriggerMsg()
//
//	PURPOSE:	Process EventCounter trigger messages
//
// --------------------------------------------------------------------------- //

bool EventCounter::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Inc("INC");
	static CParsedMsg::CToken s_cTok_Increment("INCREMENT");
	static CParsedMsg::CToken s_cTok_Dec("DEC");
	static CParsedMsg::CToken s_cTok_Decrement("DECREMENT");
	static CParsedMsg::CToken s_cTok_Lock("LOCK");
	static CParsedMsg::CToken s_cTok_Unlock("UNLOCK");

	if ((cMsg.GetArg(0) == s_cTok_Inc) ||
		(cMsg.GetArg(0) == s_cTok_Increment))
	{
		int nCount = 1;
		if (cMsg.GetArgCount() > 1)
		{
			nCount = atol(cMsg.GetArg(1));
		}

		nCount = (nCount < 1 ? 1 : nCount);

		for (int i=0; i < nCount; i++)
		{
			Increment();
		}
	}
	else if ((cMsg.GetArg(0) == s_cTok_Dec) ||
			 (cMsg.GetArg(0) == s_cTok_Decrement))
	{
		int nCount = 1;
		if (cMsg.GetArgCount() > 1)
		{
			nCount = atol(cMsg.GetArg(1));
		}

		nCount = (nCount < 1 ? 1 : nCount);

		for (int i=0; i < nCount; i++)
		{
			Decrement();
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Lock)
	{
        m_bLocked = LTTRUE;
	}
	else if (cMsg.GetArg(0) == s_cTok_Unlock)
	{
        m_bLocked = LTFALSE;
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::Increment
//
//	PURPOSE:	Increment the event counter
//
// ----------------------------------------------------------------------- //

void EventCounter::Increment()
{
	if (m_bLocked) return;

	m_nCurVal++;

	// See if we should process any commands...

	for (int i=0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		if (m_EventData[i].nValue == m_nCurVal)
		{
			if (m_EventData[i].hstrIncToValCmd)
			{
                const char* pCmd = g_pLTServer->GetStringData(m_EventData[i].hstrIncToValCmd);
				if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
				{
					g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::Decrement
//
//	PURPOSE:	Decrement the event counter
//
// ----------------------------------------------------------------------- //

void EventCounter::Decrement()
{
	if (m_bLocked || m_nCurVal <= 0) return;

	m_nCurVal--;

	// See if we should process any commands...

	for (int i=0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		if (m_EventData[i].nValue == m_nCurVal)
		{
			if (m_EventData[i].hstrDecToValCmd)
			{
                const char* pCmd = g_pLTServer->GetStringData(m_EventData[i].hstrDecToValCmd);
				if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
				{
					g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL EventCounter::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;
	char szProp[128];

	for (int i=1; i <= EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		sprintf(szProp, "Event%dValue", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_EventData[i-1].nValue = genProp.m_Long;
		}

		sprintf(szProp, "Event%dIncToValCmd", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
                m_EventData[i-1].hstrIncToValCmd = g_pLTServer->CreateString(genProp.m_String);
			}
		}

		sprintf(szProp, "Event%dDecToValCmd", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
                m_EventData[i-1].hstrDecToValCmd = g_pLTServer->CreateString(genProp.m_String);
			}
		}
	}

    if (g_pLTServer->GetPropGeneric("StartingValue", &genProp) == LT_OK)
	{
		m_nCurVal = genProp.m_Long;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void EventCounter::Save(ILTMessage_Write *pMsg)
{
	SAVE_BOOL(m_bLocked);
	SAVE_INT(m_nCurVal);

	for (int i=0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		m_EventData[i].Save(pMsg);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void EventCounter::Load(ILTMessage_Read *pMsg)
{
	LOAD_BOOL(m_bLocked);
	LOAD_INT(m_nCurVal);

	for (int i=0; i < EC_MAX_NUMBER_OF_EVENTS; i++)
	{
		m_EventData[i].Load(pMsg);
	}
}