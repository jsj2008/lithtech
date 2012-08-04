// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayMeter.cpp
//
// PURPOSE : DisplayMeter - Implementation
//
// CREATED : 7/19/00
//
// (c) 2000-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "DisplayMeter.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "MsgIDs.h"

LINKFROM_MODULE( DisplayMeter );


BEGIN_CLASS(DisplayMeter)
	PROP_DEFINEGROUP(Phase1, PF_GROUP(1), "A collection of properties to describe the first phase of the display meter.")
		ADD_LONGINTPROP_FLAG(Phase1Value, -1, PF_GROUP(1), "This is a value that corresponds with a section of the bar meter that is displayed on the screen. When the value is achieved, the section of the bar is removed.")
		ADD_COMMANDPROP_FLAG(Phase1Cmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "A command to process when the first phase is reached.")
	PROP_DEFINEGROUP(Phase2, PF_GROUP(2), "A collection of properties to describe the second phase of the display meter.")
		ADD_LONGINTPROP_FLAG(Phase2Value, -1, PF_GROUP(2), "This is a value that corresponds with a section of the bar meter that is displayed on the screen. When the value is achieved, the section of the bar is removed.")
		ADD_COMMANDPROP_FLAG(Phase2Cmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE, "A command to process when the second phase is reached.")
	PROP_DEFINEGROUP(Phase3, PF_GROUP(3), "A collection of properties to describe the third phase of the display meter.")
		ADD_LONGINTPROP_FLAG(Phase3Value, -1, PF_GROUP(3), "This is a value that corresponds with a section of the bar meter that is displayed on the screen. When the value is achieved, the section of the bar is removed.")
		ADD_COMMANDPROP_FLAG(Phase3Cmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE, "A command to process when the third phase is reached.")
	PROP_DEFINEGROUP(Phase4, PF_GROUP(4), "A collection of properties to describe the fourth phase of the display meter.")
		ADD_LONGINTPROP_FLAG(Phase4Value, -1, PF_GROUP(4), "This is a value that corresponds with a section of the bar meter that is displayed on the screen. When the value is achieved, the section of the bar is removed.")
		ADD_COMMANDPROP_FLAG(Phase4Cmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE, "A command to process when the fourth phase is reached.")
	PROP_DEFINEGROUP(Phase5, PF_GROUP(5), "A collection of properties to describe the fith phase of the display meter.")
		ADD_LONGINTPROP_FLAG(Phase5Value, -1, PF_GROUP(5), "This is a value that corresponds with a section of the bar meter that is displayed on the screen. When the value is achieved, the section of the bar is removed.")
		ADD_COMMANDPROP_FLAG(Phase5Cmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command to process when the fith phase is reached.")
    ADD_BOOLPROP(RemoveWhenEmpty, true, "This flag sets whether or not the bar meter should be removed from the screen when empty.")
END_CLASS_FLAGS_PLUGIN(DisplayMeter, BaseClass, CF_HIDDEN, CDisplayMeterPlugin, "DisplayTimer was used as a boss meter." )


CMDMGR_BEGIN_REGISTER_CLASS( DisplayMeter )

	ADD_MESSAGE( ON, 2, NULL, MSG_HANDLER( DisplayMeter, HandleOnMsg ), "ON <initial value>", "Displays the meter with the specified value 0 being an empty meter and 100 being full.", "msg DisplayMeter (ON 100)" )
	ADD_MESSAGE( INC, 2, NULL, MSG_HANDLER( DisplayMeter, HandleIncrementMsg ), "INC <amount>", "Increments the value of the meter by the amount specified", "msg DisplayMeter (INC 10)" )
	ADD_MESSAGE( DEC, 2, NULL, MSG_HANDLER( DisplayMeter, HandleDecrementMsg ), "DEC <amount>", "Decrements the value of the meter by the amount specified", "msg DisplayMeter (DEC 10)" )
	ADD_MESSAGE( SETVAL, 2, NULL, MSG_HANDLER( DisplayMeter, HandleSetValMsg ), "SETVAL <value>", "Sets the value of the meter to the amount specified.", "msg DisplayMeter (SETVAL 50)" )
	ADD_MESSAGE( OFF, 1, NULL, MSG_HANDLER( DisplayMeter, HandleOffMsg ), "OFF", "Sets the value to 0, hides the meter, and removes the object if RemoveWhenEmpty was set to true.", "msg DisplayMeter OFF" )

CMDMGR_END_REGISTER_CLASS( DisplayMeter, BaseClass )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDisplayMeterPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CDisplayMeterPlugin::PreHook_PropChanged( const char *szObjName,
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
//	ROUTINE:	DisplayMeter::DisplayMeter()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DisplayMeter::DisplayMeter() : GameBase(OT_NORMAL)
{
    m_bRemoveWhenEmpty  = true;
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

uint32 DisplayMeter::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void DisplayMeter::ReadProp( const GenericPropList *pProps )
{
	char szPropName[128];

	for( uint8 nPhase = 1; nPhase <= DM_MAX_NUMBER_OF_PHASES; ++nPhase )
	{
		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Phase%dValue", nPhase);
		m_PhaseData[nPhase-1].nValue = pProps->GetLongInt( szPropName, -1 );

		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Phase%dCmd", nPhase);
		m_PhaseData[nPhase-1].sCmd = pProps->GetCommand( szPropName, "" );
	}

	m_bRemoveWhenEmpty = pProps->GetBool( "RemoveWhenEmpty", m_bRemoveWhenEmpty );
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

    SetNextUpdate(UPDATE_NEVER);
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
    SetNextUpdate(UPDATE_NEVER);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleOnMsg()
//
//	PURPOSE:	Handle a ON message...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Show( (uint8)atoi(crParsedMsg.GetArg(1)) );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleIncrementMsg()
//
//	PURPOSE:	Handle a INC message...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandleIncrementMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Plus( (uint8)atoi(crParsedMsg.GetArg(1)) );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleDecrementMsg()
//
//	PURPOSE:	Handle a DEC message...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandleDecrementMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Minus( (uint8)atoi(crParsedMsg.GetArg(1)) );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleSetValMsg()
//
//	PURPOSE:	Handle a SETVAL message...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandleSetValMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	Set( (uint8)atoi(crParsedMsg.GetArg(1)) );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::HandleOffMsg()
//
//	PURPOSE:	Handle a OFF message...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	End( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Show()
//
//	PURPOSE:	Show the meter with the specified value...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::Show(uint8 initVal)
{
	if (initVal == 0) return;
	if (initVal > 100) initVal = 100;

	m_nValue = initVal;

	// Send message to clients telling them about the DisplayMeter...
	UpdateClients();


	// Update the DisplayMeter...
//    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Set()
//
//	PURPOSE:	Set the meter to the value...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::Set(uint8 val)
{
	if (val == 0)
	{
		if (m_nValue > 0)
			End();
		return;
	}
	if (val > 100) val = 100;

	if (m_nValue == 0)
	{
		Show(val);
		return;
	}

	m_nValue = val;

	// Send message to clients telling them about the DisplayMeter...
	UpdateClients();

	// Update the DisplayMeter...
//    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Plus()
//
//	PURPOSE:	Add the value to the meter...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::Plus(uint8 val)
{
	if (val == 0) return;
	if (val > 100) val = 100;

	if (m_nValue == 0)
	{
		Show(val);
		return;
	}

	m_nValue += val;
	if (m_nValue > 100) m_nValue = 100;

	// Send message to clients telling them about the DisplayMeter...
	UpdateClients();

	// Update the DisplayMeter...
//    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Minus()
//
//	PURPOSE:	Subtract the value from the meter...
//
// --------------------------------------------------------------------------- //

void DisplayMeter::Minus(uint8 val)
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
		if( !m_PhaseData[newPhase].sCmd.empty() )
		{
			g_pCmdMgr->QueueCommand(m_PhaseData[newPhase].sCmd.c_str(), m_hObject, m_hObject);
		}
	}


	if (newValue == 0)
	{
		End();
		return;
	}

	m_nValue = newValue;

	// Send message to clients telling them about the DisplayMeter...
	UpdateClients();

	// Update the DisplayMeter...
//    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::End()
//
//	PURPOSE:	Handle the DisplayMeter ending
//
// ----------------------------------------------------------------------- //

void DisplayMeter::End()
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

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_DISPLAY_METER);
	cMsg.Writeuint8(m_nValue);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DisplayMeter::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_BOOL(m_bRemoveWhenEmpty);
	SAVE_BYTE(m_nValue);

	for (int i=0; i < DM_MAX_NUMBER_OF_PHASES; i++)
	{
		m_PhaseData[i].Save(pMsg);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DisplayMeter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DisplayMeter::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_BOOL(m_bRemoveWhenEmpty);
	LOAD_BYTE(m_nValue);

	for (int i=0; i < DM_MAX_NUMBER_OF_PHASES; i++)
	{
		m_PhaseData[i].Load(pMsg);
	}

	if (m_nValue > 0)
	{
		UpdateClients();
	}
}
