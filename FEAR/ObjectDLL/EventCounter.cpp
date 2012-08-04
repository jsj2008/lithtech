// ----------------------------------------------------------------------- //
//
// MODULE  : EventCounter.cpp
//
// PURPOSE : EventCounter - Implementation
//
// CREATED : 03/22/2000
//
// (c) 2000-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "EventCounter.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"

LINKFROM_MODULE( EventCounter );


BEGIN_CLASS(EventCounter)
	ADD_LONGINTPROP_FLAG(StartingValue, 0, 0, "This is the base value that the object will start with.")
	PROP_DEFINEGROUP(Event1, PF_GROUP(1), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event1Value, -1, PF_GROUP(1), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event1IncToValCmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event1DecToValCmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event2, PF_GROUP(2), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event2Value, -1, PF_GROUP(2), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event2IncToValCmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event2DecToValCmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event3, PF_GROUP(3), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event3Value, -1, PF_GROUP(3), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event3IncToValCmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event3DecToValCmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event4, PF_GROUP(4), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event4Value, -1, PF_GROUP(4), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event4IncToValCmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event4DecToValCmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event5, PF_GROUP(5), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event5Value, -1, PF_GROUP(5), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event5IncToValCmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event5DecToValCmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event6, PF_GROUP(6), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event6Value, -1, PF_GROUP(6), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event6IncToValCmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event6DecToValCmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event7, PF_GROUP(7), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event7Value, -1, PF_GROUP(7), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event7IncToValCmd, "", PF_GROUP(7) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event7DecToValCmd, "", PF_GROUP(7) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event8, PF_GROUP(8), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event8Value, -1, PF_GROUP(8), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event8IncToValCmd, "", PF_GROUP(8) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event8DecToValCmd, "", PF_GROUP(8) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event9, PF_GROUP(9), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event9Value, -1, PF_GROUP(9), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event9IncToValCmd, "", PF_GROUP(9) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event9DecToValCmd, "", PF_GROUP(9) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
	PROP_DEFINEGROUP(Event10, PF_GROUP(10), "This is a subset of properties that define a possible event." )
		ADD_LONGINTPROP_FLAG(Event10Value, -1, PF_GROUP(10), "This is the numeric value at which either the EventIncToValCmd or EventDecToValCmd will be sent.")
		ADD_COMMANDPROP_FLAG(Event10IncToValCmd, "", PF_GROUP(10) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental ascension.")
		ADD_COMMANDPROP_FLAG(Event10DecToValCmd, "", PF_GROUP(10) | PF_NOTIFYCHANGE, "This is the command string that will be sent if the EventValue for this Event has been reached through incremental descent.")
END_CLASS_FLAGS_PLUGIN(EventCounter, GameBase, 0, CEventCounterPlugin, "The EventCounter object is used to send certain commands for specific values of the event counter.  The value of the EventCounter is initialy set through the StartingValue property and then controlled using the INC and DEC messages.")


CMDMGR_BEGIN_REGISTER_CLASS( EventCounter )

	ADD_MESSAGE_ARG_RANGE( INC,			1,	2,	NULL,	MSG_HANDLER( EventCounter, HandleIncMsg ),	"INC <amount>", "Adds the specified amount to the current value of the EventCounter object", "msg EventCounter (INC 1)" )
	ADD_MESSAGE_ARG_RANGE( DEC,			1,	2,	NULL,	MSG_HANDLER( EventCounter, HandleDecMsg ),	"DEC <amount>", "Subtracts the specified amount from the current value of the EventCounter object", "msg EventCounter (DEC 1)" )

	ADD_MESSAGE( LOCK,		1,	NULL,	MSG_HANDLER( EventCounter, HandleLockMsg ),		"LOCK", "Locks the EventCounter so that it ignores all commands other than UNLOCK and REMOVE", "msg EventCounter LOCK" )
	ADD_MESSAGE( UNLOCK,	1,	NULL,	MSG_HANDLER( EventCounter, HandleUnlockMsg ),	"UNLOCK", "Unlocks the EventCounter", "msg EventCounter UNLOCK" )

CMDMGR_END_REGISTER_CLASS( EventCounter, GameBase )

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
    m_bLocked = false;
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

uint32 EventCounter::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&((ObjectCreateStruct*)pData)->m_cProperties);
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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::HandleIncMsg
//
//	PURPOSE:	Handle a INC or INCREMENT message...
//
// ----------------------------------------------------------------------- //

void EventCounter::HandleIncMsg( HOBJECT /*hSender*/, const CParsedMsg& crParsedMsg )
	{
		int nCount = 1;
	if( crParsedMsg.GetArgCount() > 1 )
		{
		nCount = atol( crParsedMsg.GetArg(1) );
		}

		nCount = (nCount < 1 ? 1 : nCount);

		for (int i=0; i < nCount; i++)
		{
			Increment();
		}
	}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::HandleIncMsg
//
//	PURPOSE:	Handle a DEC or DECREMENT message...
//
// ----------------------------------------------------------------------- //

void EventCounter::HandleDecMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
	{
		int nCount = 1;
	if( crParsedMsg.GetArgCount() > 1 )
		{
		nCount = atol( crParsedMsg.GetArg(1) );
		}

		nCount = (nCount < 1 ? 1 : nCount);

		for (int i=0; i < nCount; i++)
		{
			Decrement();
		}
	}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::HandleLockMsg
//
//	PURPOSE:	Handle a LOCK message...
//
// ----------------------------------------------------------------------- //

void EventCounter::HandleLockMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg */)
{
	m_bLocked = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EventCounter::HandleLockMsg
//
//	PURPOSE:	Handle a UNLOCK message...
//
// ----------------------------------------------------------------------- //

void EventCounter::HandleUnlockMsg( HOBJECT /*hSender*/, const CParsedMsg& /*crParsedMsg*/ )
{
	m_bLocked = false;
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
			if( !m_EventData[i].sIncToValCmd.empty() )
					{
				g_pCmdMgr->QueueCommand( m_EventData[i].sIncToValCmd.c_str(), m_hObject, m_hObject );
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
			if( !m_EventData[i].sDecToValCmd.empty() )
					{
				g_pCmdMgr->QueueCommand( m_EventData[i].sDecToValCmd.c_str(), m_hObject, m_hObject );
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

bool EventCounter::ReadProp( const GenericPropList *pProps )
{
	if( !pProps )
		return false;

	char szPropName[128];

	for( uint8 nEvent = 1; nEvent <= EC_MAX_NUMBER_OF_EVENTS; ++nEvent )
		{
		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Event%dValue", nEvent);
		m_EventData[nEvent-1].nValue = pProps->GetLongInt( szPropName, -1 );

		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Event%dIncToValCmd", nEvent);
		m_EventData[nEvent-1].sIncToValCmd = pProps->GetCommand( szPropName, "" );

		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Event%dDecToValCmd", nEvent);
		m_EventData[nEvent-1].sDecToValCmd = pProps->GetCommand( szPropName, "" );
	}

	m_nCurVal = pProps->GetLongInt( "StartingValue", m_nCurVal );

	return true;
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
