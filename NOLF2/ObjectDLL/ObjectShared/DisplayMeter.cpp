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
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "MsgIDs.h"

LINKFROM_MODULE( DisplayMeter );

#pragma force_active on
BEGIN_CLASS(DisplayMeter)
	PROP_DEFINEGROUP(Phase1, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(Phase1Value, -1, PF_GROUP(1))
		ADD_STRINGPROP_FLAG(Phase1Cmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Phase2, PF_GROUP(2))
		ADD_LONGINTPROP_FLAG(Phase2Value, -1, PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Phase2Cmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Phase3, PF_GROUP(3))
		ADD_LONGINTPROP_FLAG(Phase3Value, -1, PF_GROUP(3))
		ADD_STRINGPROP_FLAG(Phase3Cmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Phase4, PF_GROUP(4))
		ADD_LONGINTPROP_FLAG(Phase4Value, -1, PF_GROUP(4))
		ADD_STRINGPROP_FLAG(Phase4Cmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Phase5, PF_GROUP(5))
		ADD_LONGINTPROP_FLAG(Phase5Value, -1, PF_GROUP(5))
		ADD_STRINGPROP_FLAG(Phase5Cmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
    ADD_BOOLPROP(RemoveWhenEmpty, LTTRUE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(DisplayMeter, BaseClass, NULL, NULL, 0, CDisplayMeterPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( DisplayMeter )

	CMDMGR_ADD_MSG( SHOW, 2, NULL, "SHOW <initial value>" )
	CMDMGR_ADD_MSG( PLUS, 2, NULL, "PLUS <amount>" )
	CMDMGR_ADD_MSG( MINUS, 2, NULL, "MINUS <amount>" )
	CMDMGR_ADD_MSG( SETVAL, 2, NULL, "SETVAL <value>" )
	CMDMGR_ADD_MSG( HIDE, 1, NULL, "HIDE" )

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
//	ROUTINE:	DisplayMeter::OnTrigger()
//
//	PURPOSE:	Process trigger messages
//
// --------------------------------------------------------------------------- //

bool DisplayMeter::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Show("show");
	static CParsedMsg::CToken s_cTok_Plus("plus");
	static CParsedMsg::CToken s_cTok_Minus("minus");
	static CParsedMsg::CToken s_cTok_SetVal("setval");
	static CParsedMsg::CToken s_cTok_Hide("hide");

	if (cMsg.GetArg(0) == s_cTok_Show)
	{
		if (cMsg.GetArgCount() > 1)
		{
            HandleShow((uint8)atoi(cMsg.GetArg(1)));
		}
		else
			HandleShow(100);
	}
	else if (cMsg.GetArg(0) == s_cTok_Plus)
	{
		if (cMsg.GetArgCount() > 1)
		{
            HandlePlus((uint8)atoi(cMsg.GetArg(1)));
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Minus)
	{
		if (cMsg.GetArgCount() > 1)
		{
            HandleMinus((uint8)atoi(cMsg.GetArg(1)));
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_SetVal)
	{
		if (cMsg.GetArgCount() > 1)
		{
            HandleSet((uint8)atoi(cMsg.GetArg(1)));
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_Hide)
	{
		HandleEnd();
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
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
//    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
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
//    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
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
//    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
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
			const char* pCmd = g_pLTServer->GetStringData(m_PhaseData[newPhase].hstrCmd);
			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
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
//    SetNextUpdate(m_hObject, UPDATE_NEXT_FRAME);
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

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_DISPLAY_METER);
	cMsg.Writeuint8(m_nValue);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
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