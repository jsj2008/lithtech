// ----------------------------------------------------------------------- //
//
// MODULE  : DecisionObject.cpp
//
// PURPOSE : DecisionObject - Implementation
//
// CREATED : 7/19/00
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "DecisionObject.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h"
#include "MsgIDs.h"

LINKFROM_MODULE( DecisionObject );

#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_DECISIONOBJECT CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	#define CF_HIDDEN_DECISIONOBJECT CF_HIDDEN

#endif

BEGIN_CLASS(DecisionObject)
	ADD_REALPROP_FLAG(Radius, 300.0f, PF_RADIUS, "Activation radius.")
	PROP_DEFINEGROUP(Choice1, PF_GROUP(1), "One of the choices")
		ADD_STRINGIDPROP_FLAG(Choice1StringID, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "String ID to display for choice.")
		ADD_COMMANDPROP_FLAG(Choice1Cmd, "", PF_GROUP(1) | PF_NOTIFYCHANGE, "Command to run for choice.")
	PROP_DEFINEGROUP(Choice2, PF_GROUP(2), "One of the choices")
		ADD_STRINGIDPROP_FLAG(Choice2StringID, "", PF_GROUP(2) | PF_NOTIFYCHANGE, "String ID to display for choice.")
		ADD_COMMANDPROP_FLAG(Choice2Cmd, "", PF_GROUP(2) | PF_NOTIFYCHANGE, "Command to run for choice.")
	PROP_DEFINEGROUP(Choice3, PF_GROUP(3), "One of the choices")
		ADD_STRINGIDPROP_FLAG(Choice3StringID, "", PF_GROUP(3) | PF_NOTIFYCHANGE, "String ID to display for choice.")
		ADD_COMMANDPROP_FLAG(Choice3Cmd, "", PF_GROUP(3) | PF_NOTIFYCHANGE, "Command to run for choice.")
	PROP_DEFINEGROUP(Choice4, PF_GROUP(4), "One of the choices")
		ADD_STRINGIDPROP_FLAG(Choice4StringID, "", PF_GROUP(4) | PF_NOTIFYCHANGE, "String ID to display for choice.")
		ADD_COMMANDPROP_FLAG(Choice4Cmd, "", PF_GROUP(4) | PF_NOTIFYCHANGE, "Command to run for choice.")
	PROP_DEFINEGROUP(Choice5, PF_GROUP(5), "One of the choices")
		ADD_STRINGIDPROP_FLAG(Choice5StringID, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "String ID to display for choice.")
		ADD_COMMANDPROP_FLAG(Choice5Cmd, "", PF_GROUP(5) | PF_NOTIFYCHANGE, "Command to run for choice.")
	PROP_DEFINEGROUP(Choice6, PF_GROUP(6), "One of the choices")
		ADD_STRINGIDPROP_FLAG(Choice6StringID, "", PF_GROUP(6) | PF_NOTIFYCHANGE, "String ID to display for choice.")
		ADD_COMMANDPROP_FLAG(Choice6Cmd, "", PF_GROUP(6) | PF_NOTIFYCHANGE, "Command to run for choice.")
	ADD_COMMANDPROP_FLAG(AbortCmd, "", PF_NOTIFYCHANGE, "Command to run if dialogue aborted.")
    ADD_BOOLPROP(RemoveAfterChoice, true, "Remove the decision object after choice is made.")

END_CLASS_FLAGS_PLUGIN(DecisionObject, GameBase, CF_HIDDEN_DECISIONOBJECT, CDecisionObjectPlugin, "This object specifies a list of options that are presented to the player. The associated command is executed based on the selection.")


CMDMGR_BEGIN_REGISTER_CLASS( DecisionObject )

	ADD_MESSAGE( ON,		1,	NULL, MSG_HANDLER( DecisionObject, HandleOnMsg ),		"ON", "Tells the Decision Object to display the choices (id strings entered in the editor) in the in game HUD", "msg DecisionObject ON" )
	ADD_MESSAGE( OFF,		1,	NULL, MSG_HANDLER( DecisionObject, HandleOffMsg ),		"OFF", "Aborts the Decision object, removes the choices from the HUD and sends the AbortCommand if one was specified.", "msg DecisionObject OFF" )
	ADD_MESSAGE( LOCK,		1,	NULL, MSG_HANDLER( DecisionObject, HandleLockMsg ),		"LOCK", "Locks the DecisionObject so that it ignores all commands other than UNLOCK and REMOVE", "msg DecisionObject LOCK" )
	ADD_MESSAGE( UNLOCK,	1,	NULL, MSG_HANDLER( DecisionObject, HandleUnlockMsg ),	"UNLOCK", "Unlocks the DecisionObject", "msg DecisionObject UNLOCK" )

CMDMGR_END_REGISTER_CLASS( DecisionObject, GameBase )

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
	switch( nPropType )
	{
	case LT_PT_COMMAND:
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
		break;

	case LT_PT_STRINGID:
		if( m_StringEditMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
		break;
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

DecisionObject::DecisionObject() 
:	GameBase				( OT_NORMAL ),
	m_sAbortCmd				( ),
	m_bVisible				( false ),
	m_bLock					( false ),
	m_bRemoveAfterChoice	( true ),
	m_fRadius				( 300.0f )
{
	
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
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 DecisionObject::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
//	ROUTINE:	DecisionObject::HandleOnMsg
//
//	PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void DecisionObject::HandleOnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_bLock )
		return;

	// Send message to clients telling them about the DecisionObject...
	Show( true );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::HandleOffMsg
//
//	PURPOSE:	Handle a OFF message...
//
// ----------------------------------------------------------------------- //

void DecisionObject::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	HandleAbort( hSender );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::HandleLockMsg
//
//	PURPOSE:	Handle a LOCK message...
//
// ----------------------------------------------------------------------- //

void DecisionObject::HandleLockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bLock = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::HandleUnlockMsg
//
//	PURPOSE:	Handle a UNLOCK message...
//
// ----------------------------------------------------------------------- //

void DecisionObject::HandleUnlockMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_bLock = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DecisionObject::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

void DecisionObject::ReadProp(const GenericPropList *pProps)
{
	char szPropName[128] = {0};

	for( uint32 nDecision = 1; nDecision <= MAX_DECISION_CHOICES; ++nDecision )
	{
		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Choice%dStringID", nDecision );
		m_ChoiceData[nDecision-1].nStringID = IndexFromStringID( pProps->GetStringID(szPropName, "") );
		
		LTSNPrintF( szPropName, ARRAY_LEN(szPropName), "Choice%dCmd", nDecision );
		m_ChoiceData[nDecision-1].sCmd = pProps->GetCommand( szPropName, "" );
	}
	
	m_sAbortCmd = pProps->GetCommand( "AbortCmd", "" );
	m_bRemoveAfterChoice = pProps->GetBool( "RemoveAfterChoice", m_bRemoveAfterChoice );
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


void DecisionObject::HandleChoose(HOBJECT hPlayer, uint8 nChoice)
{
	if( m_bLock )
		return;

	//special case of closing a open decision window as another one is being opened
	// in this case do not notify the client
	if (nChoice == (uint8)-1)
	{
		HandleAbort( hPlayer, false);
		return;
	}

	if (nChoice >= MAX_DECISION_CHOICES)
	{
		HandleAbort( hPlayer );
		return;
	}


	if( !m_ChoiceData[nChoice].sCmd.empty() )
	{
		if( !m_ChoiceData[nChoice].sCmd.empty() )
		{
			g_pCmdMgr->QueueCommand( m_ChoiceData[nChoice].sCmd.c_str(), hPlayer, m_hObject );
		}
	}
	else
	{
		HandleAbort( hPlayer );
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

void DecisionObject::HandleAbort( HOBJECT hPlayer, bool bNotifyClient)
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

	if( !m_sAbortCmd.empty() )
	{
		if( !m_sAbortCmd.empty() )
		{
			g_pCmdMgr->QueueCommand(m_sAbortCmd.c_str(), hPlayer, m_hObject);
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
		cMsg.Writeuint8(true);
		cMsg.Writeuint8( (bForceShow ? true : false) );
		for (int i=0; i < MAX_DECISION_CHOICES; i++)
		{
			cMsg.Writeuint32(m_ChoiceData[i].nStringID);
		}
		cMsg.WriteObject(m_hObject);
		cMsg.Writefloat(m_fRadius);
	}
	else
	{
		cMsg.Writeuint8(false);
		cMsg.WriteObject(m_hObject);
	}
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
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

	SAVE_STDSTRING(m_sAbortCmd);
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

	LOAD_STDSTRING( m_sAbortCmd );

	bool bVisible;
	LOAD_bool(bVisible);
	LOAD_bool(m_bRemoveAfterChoice);
	LOAD_FLOAT(m_fRadius);
	LOAD_bool( m_bLock );

	if (bVisible)
		Show(true,true);

}
