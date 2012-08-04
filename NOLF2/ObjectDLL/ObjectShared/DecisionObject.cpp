// ----------------------------------------------------------------------- //
//
// MODULE  : DecisionObject.cpp
//
// PURPOSE : DecisionObject - Implementation
//
// CREATED : 7/19/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DecisionObject.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "MsgIDs.h"

LINKFROM_MODULE( DecisionObject );

#pragma force_active on
BEGIN_CLASS(DecisionObject)
	ADD_REALPROP_FLAG(Radius, 300.0f, PF_RADIUS)
	PROP_DEFINEGROUP(Choice1, PF_GROUP(1))
		ADD_LONGINTPROP_FLAG(Choice1StringID, 0, PF_GROUP(1))
		ADD_STRINGPROP_FLAG(Choice1Cmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice2, PF_GROUP(2))
		ADD_LONGINTPROP_FLAG(Choice2StringID, 0, PF_GROUP(2))
		ADD_STRINGPROP_FLAG(Choice2Cmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice3, PF_GROUP(3))
		ADD_LONGINTPROP_FLAG(Choice3StringID, 0, PF_GROUP(3))
		ADD_STRINGPROP_FLAG(Choice3Cmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice4, PF_GROUP(4))
		ADD_LONGINTPROP_FLAG(Choice4StringID, 0, PF_GROUP(4))
		ADD_STRINGPROP_FLAG(Choice4Cmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice5, PF_GROUP(5))
		ADD_LONGINTPROP_FLAG(Choice5StringID, 0, PF_GROUP(5))
		ADD_STRINGPROP_FLAG(Choice5Cmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE)
	PROP_DEFINEGROUP(Choice6, PF_GROUP(6))
		ADD_LONGINTPROP_FLAG(Choice6StringID, 0, PF_GROUP(6))
		ADD_STRINGPROP_FLAG(Choice6Cmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE)
	ADD_STRINGPROP_FLAG(AbortCmd, "", PF_NOTIFYCHANGE)
    ADD_BOOLPROP(RemoveAfterChoice, LTTRUE)

END_CLASS_DEFAULT_FLAGS_PLUGIN(DecisionObject, BaseClass, NULL, NULL, 0, CDecisionObjectPlugin)
#pragma force_active off

CMDMGR_BEGIN_REGISTER_CLASS( DecisionObject )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( START, 1, NULL, "START" )
	CMDMGR_ADD_MSG( STOP, 1, NULL, "STOP" )
	CMDMGR_ADD_MSG( LOCK, 1, NULL, "LOCK" )
	CMDMGR_ADD_MSG( UNLOCK, 1, NULL, "UNLOCK" )

CMDMGR_END_REGISTER_CLASS( DecisionObject, BaseClass )

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDecisionObjectPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CDecisionObjectPlugin::PreHook_PropChanged( const char *szObjName,
													   const char *szPropName, 
													   const int  nPropType, 
													   const GenericProp &gpPropValue,
													   ILTPreInterface *pInterface,
													   const char	*szModifiers)
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
//	ROUTINE:	DecisionObject::DecisionObject()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

DecisionObject::DecisionObject() : GameBase(OT_NORMAL)
{
	m_hstrAbortCmd = LTNULL;
	m_bVisible = false;
	m_bLock = false;
	m_bRemoveAfterChoice = true;
	m_fRadius = 300.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::~DecisionObject()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

DecisionObject::~DecisionObject()
{
	FREE_HSTRING(m_hstrAbortCmd);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 DecisionObject::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
			if (pStruct)
			{
				pStruct->m_Flags |= FLAG_FORCECLIENTUPDATE;
			}

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
//	ROUTINE:	DecisionObject::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool DecisionObject::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Start("START");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Stop("STOP");
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_Lock("LOCK");
	static CParsedMsg::CToken s_cTok_Unlock("UNLOCK");

	if ((cMsg.GetArg(0) == s_cTok_Start) ||
		(cMsg.GetArg(0) == s_cTok_On)
		)
	{
		HandleShow();
	}
	else if ((cMsg.GetArg(0) == s_cTok_Stop) ||
		(cMsg.GetArg(0) == s_cTok_Off)
		)
	{
		HandleAbort();
	}
	else if( cMsg.GetArg(0) == s_cTok_Lock )
	{
		m_bLock = true;
	}
	else if( cMsg.GetArg(0) == s_cTok_Unlock )
	{
		m_bLock = false;
	}
	else
		return GameBase::OnTrigger(hSender, cMsg);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void DecisionObject::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;
	char szProp[128];

	for (int i=1; i <= MAX_DECISION_CHOICES; i++)
	{
		sprintf(szProp, "Choice%dStringID", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			m_ChoiceData[i-1].nStringID = genProp.m_Long;
		}

		sprintf(szProp, "Choice%dCmd", i);
        if (g_pLTServer->GetPropGeneric(szProp, &genProp) == LT_OK)
		{
			if (genProp.m_String[0])
			{
                m_ChoiceData[i-1].hstrCmd = g_pLTServer->CreateString(genProp.m_String);
			}
		}

	}

    if (g_pLTServer->GetPropGeneric("AbortCmd", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrAbortCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("RemoveAfterChoice", &genProp ) == LT_OK)
	{
		m_bRemoveAfterChoice = ( genProp.m_Bool ? true : false);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

void DecisionObject::InitialUpdate()
{
	// Must be triggered on...

    SetNextUpdate(UPDATE_NEVER);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

void DecisionObject::Update()
{
    SetNextUpdate(UPDATE_NEVER);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::HandleShow()
//
//	PURPOSE:	Handle show message
//
// --------------------------------------------------------------------------- //

void DecisionObject::HandleShow()
{
	if( m_bLock )
		return;

	// Send message to clients telling them about the DecisionObject...
	Show(true);

}

void DecisionObject::HandleChoose(uint8 nChoice)
{
	if( m_bLock )
		return;

	//special case of closing a open decision window as another one is being opened
	// in this case do not notify the client
	if (nChoice == -1)
	{
		HandleAbort(false);
		return;
	}

	if (nChoice >= MAX_DECISION_CHOICES)
	{
		HandleAbort();
		return;
	}


	if (m_ChoiceData[nChoice].hstrCmd)
	{
		const char* pCmd = g_pLTServer->GetStringData(m_ChoiceData[nChoice].hstrCmd);
		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
		}
	}
	else
	{
		HandleAbort();
		return;
	}

	if (m_bRemoveAfterChoice)
		g_pLTServer->RemoveObject(m_hObject);

	// Send message to clients telling them about the DecisionObject...
	Show(false);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::HandleAbort()
//
//	PURPOSE:	Handle the DecisionObject ending
//
// ----------------------------------------------------------------------- //

void DecisionObject::HandleAbort(bool bNotifyClient)
{
	if( m_bLock )
		return;

	if (bNotifyClient)
	{
		Show(false);
	}
	else
	{
		m_bVisible = false;
	}

	if (m_hstrAbortCmd)
	{
		const char* pCmd = g_pLTServer->GetStringData(m_hstrAbortCmd);
		if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
		{
			g_pCmdMgr->Process(pCmd, m_hObject, m_hObject);
		}
	}


}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::Show()
//
//	PURPOSE:	Update the client's time
//
// --------------------------------------------------------------------------- //

void DecisionObject::Show(bool bShow, bool bForceShow)
{
	if( m_bLock )
		return;

	// Send message to clients telling them about the DecisionObject...

	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_DECISION);

	m_bVisible = bShow;


	if (bShow)
	{
		cMsg.Writeuint8(LTTRUE);
		cMsg.Writeuint8( (bForceShow ? LTTRUE : LTFALSE) );
		for (int i=0; i < MAX_DECISION_CHOICES; i++)
		{
			cMsg.Writeuint32(m_ChoiceData[i].nStringID);
		}
		cMsg.WriteObject(m_hObject);
		cMsg.Writefloat(m_fRadius);
	}
	else
	{
		cMsg.Writeuint8(LTFALSE);
		cMsg.WriteObject(m_hObject);
	}
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void DecisionObject::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;


	for (int i=0; i < MAX_DECISION_CHOICES; i++)
	{
		m_ChoiceData[i].Save(pMsg);
	}

	SAVE_HSTRING(m_hstrAbortCmd);
	SAVE_bool(m_bVisible);
	SAVE_bool(m_bRemoveAfterChoice);
	SAVE_FLOAT(m_fRadius);
	SAVE_bool( m_bLock );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void DecisionObject::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	for (int i=0; i < MAX_DECISION_CHOICES; i++)
	{
		m_ChoiceData[i].Load(pMsg);
	}

	LOAD_HSTRING(m_hstrAbortCmd);

	LTBOOL bVisible;
	LOAD_bool(bVisible);
	LOAD_bool(m_bRemoveAfterChoice);
	LOAD_FLOAT(m_fRadius);
	LOAD_bool( m_bLock );

	if (bVisible)
		Show(true,true);

}